import argparse
import datetime
import json
import logging
import os.path
import pathlib
import random
import socket
import ssl
import string
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Literal, Optional, Union
import multiprocessing

import uvicorn
from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives.asymmetric.types import CertificateIssuerPrivateKeyTypes, CertificatePublicKeyTypes
from cryptography.x509.oid import NameOID
from fastapi import FastAPI, HTTPException, Request, Response, APIRouter
from fastapi.responses import FileResponse, HTMLResponse, RedirectResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel
# Monkey patch uvicorn to make the underlying transport available to us.
# That will let us access the ssl context and get the client certificate information.
from uvicorn.protocols.http.h11_impl import H11Protocol
from zeroconf import ServiceInfo, Zeroconf

http_tools_protocol_old__should_upgrade = H11Protocol._should_upgrade


def http_tools_protocol_new__should_upgrade(self):
    http_tools_protocol_old__should_upgrade(self)
    self.scope["transport"] = self.transport


H11Protocol._should_upgrade = http_tools_protocol_new__should_upgrade

# End monkey patch


class WorkingDirectory:
    """
    Collection of utilities to add convention to the files used by this program.
    """

    tmp = None

    def __init__(self, directory: Optional[str] = None) -> None:

        if directory is None:
            self.tmp = tempfile.TemporaryDirectory(prefix="TC_PAVS_1_0")
        else:
            d = pathlib.Path(directory)
            d.mkdir(parents=True, exist_ok=True)
            self.directory = d

    def __enter__(self):
        return self

    def __exit__(self, exc, value, tb):
        self.cleanup()

    def cleanup(self):
        if self.tmp:
            self.tmp.cleanup()

    def root_dir(self) -> Path:
        return Path(self.tmp.name) if self.tmp else self.directory

    def path(self, *paths: str) -> Path:
        return Path(os.path.join(self.root_dir(), *paths))

    def mkdir(self, *paths: str, is_file=False) -> Path:
        """
        Create a directory using the given path rooted in the working directory.
        If a file is provided, the directory up to that file will be created instead.
        Returns the full path.
        """
        p = self.path(*paths)

        # Let's create the parent directories exist
        p2 = pathlib.Path(p)
        if is_file:
            p2 = p2.parent

        p2.mkdir(parents=True, exist_ok=True)

        return p

    def print_tree(self):
        # TODO Convert this helper to build a HTML representation for use in the UI

        def tree(dir_path: pathlib.Path, prefix: str = ""):
            """A recursive generator, given a directory Path object
            will yield a visual tree structure line by line
            with each line prefixed by the same characters
            """
            # prefix components:
            space = "    "
            branch = "│   "
            # pointers:
            tee = "├── "
            last = "└── "

            contents = list(dir_path.iterdir())
            # contents each get pointers that are ├── with a final └── :
            pointers = [tee] * (len(contents) - 1) + [last]
            for pointer, path in zip(pointers, contents):
                is_dir = path.is_dir()
                yield prefix + pointer + path.name + ("/" if is_dir else "")
                if path.is_dir():  # extend the prefix and recurse:
                    extension = branch if pointer == tee else space
                    # i.e. space because last, └── , above so no more |
                    yield from tree(path, prefix=prefix + extension)

        root = self.root_dir()
        print(root)
        for line in tree(pathlib.Path(root)):
            print(line)


class CAHierarchy:
    """
    Utilities to manage a CA hierarchy on disk.
    """

    default_ca_duration = datetime.timedelta(days=365.25*20)

    client_key_usage_cert = x509.KeyUsage(
        digital_signature=True,
        content_commitment=False,
        key_encipherment=True,
        data_encipherment=False,
        key_agreement=False,
        key_cert_sign=False,
        crl_sign=False,
        encipher_only=False,
        decipher_only=False,
    )
    server_key_usage_cert = x509.KeyUsage(
        digital_signature=True,
        content_commitment=False,
        key_encipherment=False,
        data_encipherment=False,
        key_agreement=False,
        key_cert_sign=False,
        crl_sign=False,
        encipher_only=False,
        decipher_only=False,
    )

    def __init__(self, base: Path, name: str, kind: Literal['server', 'client']) -> None:
        self.name = name
        self.kind = kind
        self.directory = base

        self.root_cert_path = self.directory / "root.pem"
        self.root_key_path = self.directory / "root.key"

        if self.root_key_path.exists() and self.root_cert_path.exists():
            # Root certificate already exists, re-using them
            self.root_cert = x509.load_pem_x509_certificate(
                self.root_cert_path.read_bytes()
            )
            self.root_key = serialization.load_pem_private_key(
                self.root_key_path.read_bytes(), None
            )

            logging.info(f"CA Hierarchy loaded from disk: {self.name}")
        elif self.root_key_path.exists() or self.root_cert_path.exists():
            # Only one of the two file exists, bailing out
            logging.error("root certificate partially exist on disk, stopping early")
            sys.exit(1)
        else:
            # Start generating the root certificate
            self.root_key = rsa.generate_private_key(
                public_exponent=65537, key_size=2048
            )
            rand_suffix = "".join(
                random.choices(string.ascii_letters + string.digits, k=16)
            )
            root_cert_subject = x509.Name(
                [
                    x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                    x509.NameAttribute(
                        NameOID.COMMON_NAME, "TC_PAVS root " + rand_suffix
                    ),
                ]
            )
            self.root_cert = (
                x509.CertificateBuilder()
                .subject_name(root_cert_subject)
                .issuer_name(root_cert_subject)
                .public_key(self.root_key.public_key())
                .serial_number(x509.random_serial_number())
                .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
                .not_valid_after(
                    datetime.datetime.now(datetime.timezone.utc) + self.default_ca_duration
                )
                .add_extension(
                    # We make it so that our root can only issue leaf certificates, no intermediate here.
                    x509.BasicConstraints(ca=True, path_length=0), critical=True
                )
                .add_extension(
                    x509.KeyUsage(
                        digital_signature=True,
                        content_commitment=False,
                        key_encipherment=False,
                        data_encipherment=False,
                        key_agreement=False,
                        key_cert_sign=True,
                        crl_sign=True,
                        encipher_only=False,
                        decipher_only=False,
                    ),
                    critical=True,
                )
                .add_extension(
                    x509.SubjectKeyIdentifier.from_public_key(
                        self.root_key.public_key()
                    ),
                    critical=False,
                )
                .sign(self.root_key, hashes.SHA256())
            )

            self._save_cert("root", self.root_cert, self.root_key, False)

            logging.info(f"CA Hierarchy generated: {self.name}")

    def _save_cert(
        self,
        name: str,
        cert: x509.Certificate,
        key: Union[CertificateIssuerPrivateKeyTypes, None],
        bundle_root: bool,
    ) -> tuple[str, str]:
        """
        Private method that help with saving certificate and key to the hierarchy folder.
        This tool isn't meant to be used in production, but instead to help with development
        and as such have the goal to make the CA hierarchy as available as possible, which in
        turn make it very unsecure.
        """
        cert_path = self.directory / f"{name}.pem"
        key_path = self.directory / f"{name}.key" if key else None

        if key:
            with open(key_path, "wb") as f:
                f.write(
                    key.private_bytes(
                        encoding=serialization.Encoding.PEM,
                        format=serialization.PrivateFormat.TraditionalOpenSSL,
                        encryption_algorithm=serialization.NoEncryption(),
                    )
                )

        with open(cert_path, "wb") as f:
            f.write(cert.public_bytes(serialization.Encoding.PEM))

            if bundle_root:
                f.write(b"\n")
                f.write(self.root_cert.public_bytes(serialization.Encoding.PEM))

        return (key_path, cert_path)

    def _sign_cert(
        self,
        dns: str,
        public_key: CertificatePublicKeyTypes,
        duration: datetime.timedelta
    ) -> x509.Certificate:
        """
        Generate and sign a certificate.
        """
        # Sign certificate
        subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "TC_PAVS"),
                x509.NameAttribute(NameOID.COMMON_NAME, dns),
            ]
        )

        extended_key_usage = [x509.ExtendedKeyUsageOID.CLIENT_AUTH] if self.kind == "client" else [
            x509.ExtendedKeyUsageOID.SERVER_AUTH]

        builder = (x509.CertificateBuilder()
                   .subject_name(subject)
                   .issuer_name(self.root_cert.subject)
                   .public_key(public_key)
                   .serial_number(x509.random_serial_number())
                   .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
                   .not_valid_after(
            datetime.datetime.now(datetime.timezone.utc) + duration
        )
            .add_extension(
                x509.BasicConstraints(ca=False, path_length=None),
                critical=False,
        )
            .add_extension(
                self.client_key_usage_cert if self.kind == "client" else self.server_key_usage_cert,
                critical=True,
        )
            .add_extension(
                x509.ExtendedKeyUsage(extended_key_usage),
                critical=False,
        )
            .add_extension(
                x509.SubjectKeyIdentifier.from_public_key(public_key),
                critical=False,
        )
            .add_extension(
                x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(
                    self.root_cert.extensions.get_extension_for_class(
                        x509.SubjectKeyIdentifier
                    ).value
                ),
                critical=False,
        )
            .add_extension(x509.CRLDistributionPoints([x509.DistributionPoint(
                full_name=[x509.UniformResourceIdentifier("http://not.a.valid.website.com/some/path/to/a.crl")],
                relative_name=None,
                reasons=None,
                crl_issuer=None
            )]), critical=False)
        )

        if self.kind == 'server':
            builder.add_extension(
                x509.SubjectAlternativeName([x509.DNSName(dns)]),
                critical=False,
            )

        return builder.sign(self.root_key, hashes.SHA256())

    def gen_cert(self, dns: str, csr: str, override=False, duration: datetime.timedelta = datetime.timedelta(hours=1)) -> tuple[Path, Path, bool]:
        """
        Generate a certificate signed by this CA hierarchy using the provided CSR.
        Returns the path to the key, cert, and whether it was reused or not.
        """
        signing_request = x509.load_pem_x509_csr(csr.encode('utf-8'))
        signing_request.public_key()

        # If we don't always override, first check if an existing keypair already exists
        if not override:
            cert_path = self.directory / f"{dns}.pem"
            key_path = self.directory / f"{dns}.key"

            if cert_path.exists() and key_path.exists():
                return (key_path, cert_path, True)

        # Sign certificate
        cert = self._sign_cert(dns, signing_request.public_key(), duration)

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(
            dns, cert, None, bundle_root=True
        )

        logging.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)

        return (key_path, cert_bundle_path, False)

    def gen_keypair(self, dns: str, override=False, duration: datetime.timedelta = datetime.timedelta(hours=1)) -> tuple[Path, Path, bool]:
        """
        Generate a private key as well as the associated certificate signed by this CA
        hierarchy. Returns the path to the key, cert, and whether it was reused or not.
        """

        # If we don't always override, first check if an existing keypair already exists
        if not override:
            cert_path = self.directory / f"{dns}.pem"
            key_path = self.directory / f"{dns}.key"

            if cert_path.exists() and key_path.exists():
                return (key_path, cert_path, True)

        # Generate private key
        key = rsa.generate_private_key(public_exponent=65537, key_size=2048)

        # Sign certificate
        cert = self._sign_cert(dns, key.public_key(), duration)

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(dns, cert, key, bundle_root=True)

        logging.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)

        return (key_path, cert_bundle_path, False)


class SignClientCertificate(BaseModel):
    """Request model to sign a client certificate"""
    csr: str


class PushAvServer:

    templates = Jinja2Templates(directory="templates")

    def __init__(self, wd: WorkingDirectory, device_hierarchy: CAHierarchy):
        self.wd = wd
        self.device_hierarchy = device_hierarchy
        self.router = APIRouter()

        # UI
        self.router.add_api_route("/", self.index, methods=["GET"], response_class=RedirectResponse)
        self.router.add_api_route("/ui/streams", self.ui_streams_list, methods=["GET"], response_class=HTMLResponse)
        self.router.add_api_route("/ui/streams/{stream_id}/{file_path:path}", self.ui_streams_details, methods=["GET"])
        self.router.add_api_route("/ui/certificates", self.ui_certificates_list, methods=["GET"], response_class=HTMLResponse)
        self.router.add_api_route("/ui/certificates/{hierarchy}/{name}",
                                  self.ui_certificates_details, methods=["GET"], response_class=HTMLResponse)

        # HTTP APIs
        self.router.add_api_route("/streams", self.create_stream, methods=["POST"], status_code=201)
        self.router.add_api_route("/streams", self.list_streams, methods=["GET"])
        self.router.add_api_route("/streams/probe/{stream_id}/{file_path:path}", self.ffprobe_check, methods=["GET"])
        self.router.add_api_route("/streams/{stream_id}", self.manifest_upload, methods=["PUT"])
        self.router.add_api_route("/streams/{stream_id}/{file_path:path}", self.segment_upload, methods=["PUT"], status_code=202)
        self.router.add_api_route("/streams/{stream_id}/{file_path:path}", self.segment_download, methods=["GET"])
        self.router.add_api_route("/certs", self.list_certs, methods=["GET"], status_code=200)
        self.router.add_api_route("/certs/{hierarchy}/{name}", self.certificate_details, methods=["GET"], status_code=200)
        self.router.add_api_route("/certs/{name}/keypair", self.create_client_keypair, methods=["POST"])
        self.router.add_api_route("/certs/{name}/sign", self.sign_client_certificate, methods=["POST"])

    # UI website

    def index(self):
        return RedirectResponse("/ui/streams")

    def ui_streams_list(self, request: Request):
        s = self.list_streams()
        return self.templates.TemplateResponse(
            request=request, name="streams_list.html", context={"streams": s["streams"]}
        )

    def ui_streams_details(self, request: Request, stream_id: int, file_path: str):
        context = {}
        context['streams'] = self.list_streams()['streams']
        context['stream_id'] = stream_id
        context['file_path'] = file_path

        if file_path.endswith('.crt'):
            context['type'] = 'cert'
            p = self.wd.path("streams", str(stream_id), file_path)
            with open(p, "r") as f:
                context['cert'] = json.load(f)
        else:
            context['type'] = 'media'
            context['probe'] = self.ffprobe_check(stream_id, file_path)
            context['pretty_probe'] = json.dumps(context['probe'], sort_keys=True, indent=4)

        return self.templates.TemplateResponse(request=request, name="streams_details.html", context=context)

    def ui_certificates_list(self, request: Request):
        return self.templates.TemplateResponse(
            request=request, name="certificates_list.html", context={"certs": self.list_certs()}
        )

    def ui_certificates_details(self, request: Request, hierarchy: str, name: str):
        context = self.certificate_details(hierarchy, name)
        context["certs"] = self.list_certs()

        return self.templates.TemplateResponse(request=request, name="certificates_details.html", context=context)

    # APIs

    def create_stream(self):
        # Find the last registered stream
        dirs = [d for d in pathlib.Path(self.wd.path("streams")).iterdir() if d.is_dir()]
        last_stream = int(dirs[-1].name) if len(dirs) > 0 else 0
        stream_id = last_stream + 1

        self.wd.mkdir("streams", str(stream_id))

        return {"stream_id": stream_id}

    def list_streams(self):
        dirs = [d for d in pathlib.Path(self.wd.path("streams")).iterdir() if d.is_dir()]

        def stream_files(dir: Path):
            return [f.relative_to(dir) for f in dir.glob("**/*") if f.is_file()]

        streams = [{"id": d.name, "files": stream_files(d)} for d in dirs]

        return {"streams": streams}

    async def manifest_upload(self, stream_id: int, req: Request):
        """The DASH manifest is uploaded onto the base path without any file path"""

        # Here we assume that no camera will upload an index.mpd file on their own.
        # That is something that may not be true, in which case we would have to add
        # another layer of abstraction on the file system where we can store the mpd
        # file and the camera direct uploads.
        return await self.segment_upload("index.mpd", stream_id, req)

    async def segment_upload(self, file_path: str, stream_id: int, req: Request):
        """Extract the parsed version of a client certificate.
        See https://docs.python.org/3/library/ssl.html#ssl.SSLSocket.getpeercert
        for the exact content.
        """
        cert_details = req.scope["transport"].get_extra_info("ssl_object").getpeercert()

        logging.debug(f"segment_upload. stream_id={stream_id} file_path:{file_path}")

        if not self.wd.path("streams", str(stream_id)).exists():
            raise HTTPException(404, detail="Stream doesn't exists")

        dst = self.wd.mkdir("streams", str(stream_id), file_path, is_file=True)

        with open(dst.with_suffix(dst.suffix + ".crt"), "w") as f:
            f.write(json.dumps(cert_details))

        with open(dst, "wb") as f:
            async for chunk in req.stream():
                f.write(chunk)

        return Response(status_code=202)

    def ffprobe_check(self, stream_id: int, file_path: str):

        p = self.wd.path("streams", str(stream_id), file_path)

        if not p.exists():
            return HTTPException(404, detail="Stream doesn't exists")

        proc = subprocess.run(
            ["ffprobe", "-show_streams", "-show_format", "-output_format", "json", str(p.absolute())],
            capture_output=True
        )

        if proc.returncode != 0:
            # TODO Add more details (maybe stderr) to the response
            return HTTPException(500)

        return json.loads(proc.stdout)

    async def segment_download(self, file_path: str, stream_id: int):
        return FileResponse(self.wd.path("streams", str(stream_id), file_path))

    def list_certs(self):
        server = [f.name for f in pathlib.Path(self.wd.path("certs", "server")).iterdir()]
        device = [f.name for f in pathlib.Path(self.wd.path("certs", "device")).iterdir()]

        return {"server": server, "device": device}

    def certificate_details(self, hierarchy: str, name: str):
        data = pathlib.Path(self.wd.path("certs", hierarchy, name)).read_bytes()
        type = "key" if name.endswith(".key") else "cert"

        key = None
        cert = None
        if type == "key":
            key = serialization.load_pem_private_key(data, None)
            key = {
                "key_size": key.key_size,
                "private_key": key.private_bytes(
                    encoding=serialization.Encoding.PEM,
                    format=serialization.PrivateFormat.TraditionalOpenSSL,
                    encryption_algorithm=serialization.NoEncryption(),
                ),
                "public_key": key.public_key().public_bytes(
                    encoding=serialization.Encoding.PEM,
                    format=serialization.PublicFormat.PKCS1,
                ),
            }
        else:
            cert = x509.load_pem_x509_certificate(data)
            cert = {
                "public_cert": cert.public_bytes(serialization.Encoding.PEM),
                "serial_number": hex(cert.serial_number),
                "not_valid_before": cert.not_valid_before_utc,
                "not_valid_after": cert.not_valid_after_utc,
                # public_key? fingerprint?
                "issuer": cert.issuer.rfc4514_string(),
                "subject": cert.subject.rfc4514_string(),
                "extensions": [str(ext) for ext in cert.extensions]
            }

        return {"type": type, "key": key, "cert": cert}

    def create_client_keypair(self, name: str, override: bool = True):
        (key, cert, created) = self.device_hierarchy.gen_keypair(name, override)

        return {key, cert, created}

    def sign_client_certificate(
        self, name: str, req: SignClientCertificate, override: bool = True
    ):
        (key, cert, created) = self.device_hierarchy.gen_cert(name, req.csr, override)

        return {key, cert, created}


class PushAvContext:
    """Hold the context for a full Push AV Server including temporary disk, CA hierarchies and web server"""

    def __init__(self, host: Optional[str], port: Optional[int], working_directory: Optional[str], dns: Optional[str]):
        self.directory = WorkingDirectory(working_directory)
        self.host = host
        self.port = port
        self.dns = "localhost" if dns is None else f"{dns}._http._tcp_.local."
        self.proc: multiprocessing.Process | None = None

        # Create CA hierarchies (for webserver and devices)
        self.device_hierarchy = CAHierarchy(self.directory.mkdir("certs", "device"), "device", "client")
        self.server_hierarchy = CAHierarchy(self.directory.mkdir("certs", "server"), "server", "server")
        (self.server_key_file, self.server_cert_file, _) = self.server_hierarchy.gen_keypair(self.dns, override=True)

        # mDNS configuration. Registration only happen if the dns isn't localhost.
        self.zeroconf = Zeroconf()
        self.svc_info = None

        if self.dns != "localhost":
            self.svc_info = ServiceInfo(
                "_http._tcp.local.",
                name=self.dns,
                addresses=[socket.inet_aton("127.0.0.1")],
                port=1234,
            )

        # Streams holder
        self.directory.mkdir("streams")

        self.app = FastAPI()
        self.app.mount("/static", StaticFiles(directory="static"), name="static")
        pas = PushAvServer(self.directory, self.device_hierarchy)
        self.app.include_router(pas.router)

    def start_in_background(self):
        if self.proc:
            logging.warning("Attempting to start a server when one is already running, no new server is being started.")
            return

        # Advertise over mDNS
        if self.svc_info:
            logging.info("Advertising the service as %s", self.svc_info)
            self.zeroconf.register_service(self.svc_info)

        def background_job():
            # Start the web server
            try:
                uvicorn.run(
                    self.app,
                    host=self.host,
                    port=self.port,
                    ssl_keyfile=self.server_key_file,
                    ssl_certfile=self.server_cert_file,
                    ssl_cert_reqs=ssl.CERT_OPTIONAL,
                    ssl_ca_certs=self.device_hierarchy.root_cert_path,
                )
            finally:
                if self.svc_info:
                    self.zeroconf.unregister_service(self.svc_info)

        # Spawning the function results in python not being able to pickle the full context
        # (most notably cryptography's rust bindings). So instead we force use forks as the
        # way to create processes.
        multiprocessing.set_start_method('fork')
        self.proc = multiprocessing.Process(target=background_job, daemon=True)
        self.proc.start()

    def terminate(self):
        self.proc.terminate()
        self.directory.cleanup()


if __name__ == "__main__":
    logging.basicConfig(
        format="%(asctime)s|%(name)-8s|%(levelname)-5s|%(message)s",
        level=logging.DEBUG,
        datefmt="%H:%M:%S",
    )

    parser = argparse.ArgumentParser(
        prog="push_av_tool.py",
        description="Tooling to help test Matter's Push AV capabilities",
    )

    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", default=1234)
    parser.add_argument(
        "--working-directory",
        help="Where to store content like certificates or uploaded streams. "
        "Default to a temporary directory.",
    )
    parser.add_argument(
        "--dns", help="A mDNS record to adversise, or none if left empty."
    )

    args = parser.parse_args()

    ctx = PushAvContext(args.host, args.port, args.working_directory, args.dns)
    ctx.start_in_background()
    print(ctx.proc)
    ctx.proc.join()
    ctx.terminate()
