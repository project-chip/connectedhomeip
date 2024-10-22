import logging
import ssl
from typing import Optional, Union
import tempfile
import os.path
import datetime
import argparse
import pathlib
from pathlib import Path
import random
import string
import json
import socket
import sys
import subprocess

from zeroconf import ServiceInfo, Zeroconf

import uvicorn
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives.asymmetric.types import (
    CertificatePublicKeyTypes,
    CertificateIssuerPrivateKeyTypes,
)


from fastapi import FastAPI, Request, HTTPException, Response
from fastapi.responses import HTMLResponse
from pydantic import BaseModel

# Monkey patch uvicorn to make the underlying transport available to us.
# That will let us access the ssl context and get the client certificate information.
from uvicorn.protocols.http.h11_impl import H11Protocol

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

    def __init__(self, base: Path, name: str) -> None:
        self.name = name
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
                    datetime.datetime.now(datetime.timezone.utc)
                    + datetime.timedelta(days=1)
                )
                .add_extension(
                    x509.BasicConstraints(ca=True, path_length=None), critical=True
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
        key_path = self.directory / f"{name}.key"

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
        self, dns: str, public_key: CertificatePublicKeyTypes
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

        extended_key_usage = [x509.ExtendedKeyUsageOID.CLIENT_AUTH] if self.name == "device" else [
            x509.ExtendedKeyUsageOID.SERVER_AUTH]

        return (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(self.root_cert.subject)
            .public_key(public_key)
            .serial_number(x509.random_serial_number())
            .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
            .not_valid_after(
                datetime.datetime.now(datetime.timezone.utc)
                + datetime.timedelta(hours=1)
            )
            .add_extension(
                x509.SubjectAlternativeName(
                    [
                        # Describe what sites we want this certificate for.
                        # TODO Is it needed when we already have it in the CN field ?
                        x509.DNSName(dns),
                    ]
                ),
                critical=False,
            )
            .add_extension(
                x509.BasicConstraints(ca=False, path_length=None),
                critical=True,
            )
            .add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=True,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=False,
                    crl_sign=False,
                    encipher_only=False,
                    decipher_only=False,
                ),
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
            .sign(self.root_key, hashes.SHA256())
        )

    def gen_cert(self, dns: str, csr: str, override=False) -> tuple[Path, Path, bool]:
        """
        Generate a certificate signed by this CA hierarchy using the provided CSR.
        Returns the path to the key, cert, and whether it was reused or not.
        """
        signing_request = x509.load_pem_x509_csr(csr)
        signing_request.public_key()

        # If we don't always override, first check if an existing keypair already exists
        if not override:
            cert_path = self.directory / f"{dns}.pem"
            key_path = self.directory / f"{dns}.key"

            if cert_path.exists() and key_path.exists():
                return (key_path, cert_path, True)

        # Sign certificate
        cert = self._sign_cert(dns, csr.public_key())

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(
            dns, cert, None, bundle_root=True
        )

        logging.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)

        return (key_path, cert_bundle_path, False)

    def gen_keypair(self, dns: str, override=False) -> tuple[Path, Path, bool]:
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
        cert = self._sign_cert(dns, key.public_key())

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(dns, cert, key, bundle_root=True)

        logging.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)

        return (key_path, cert_bundle_path, False)


app = FastAPI()
# Couldn't find how to do DI with state created in __main__ without using
# global variables, so using those.
wd: WorkingDirectory = None
device_hierarchy: CAHierarchy = None


@app.get("/", response_class=HTMLResponse)
def root():
    # TODO Include the UI here

    with open("index.html", "r") as f:
        return f.read()


@app.post("/streams", status_code=201)
def create_stream():
    # Find the last registered stream
    dirs = [d for d in pathlib.Path(wd.path("streams")).iterdir() if d.is_dir()]
    last_stream = int(dirs[-1].name) if len(dirs) > 0 else 0
    print(f"last_stream={last_stream}")
    stream_id = last_stream + 1

    wd.mkdir("streams", str(stream_id))

    return {"stream_id": stream_id}


@app.get("/streams")
def list_streams():
    dirs = [d for d in pathlib.Path(wd.path("streams")).iterdir() if d.is_dir()]

    # TODO Files can be multiple directory deep, need to reconstruct the tree here
    streams = [{"id": d.name, "files": [f.name for f in d.iterdir()]} for d in dirs]

    return {"streams": streams}


# TODO app.put("/streams/{stream_id}")
# To conform to the ingest spec, support of that path with discarding the body
# and not doing anything.

@app.put("/streams/{stream_id}/{file_path:path}", status_code=202)
async def segment_upload(file_path: str, stream_id: int, req: Request):
    """Extract the parsed version of a client certificate.
    See https://docs.python.org/3/library/ssl.html#ssl.SSLSocket.getpeercert
    for the exact content.
    """
    cert_details = req.scope["transport"].get_extra_info("ssl_object").getpeercert()

    logging.debug(f"segment_upload. stream_id={stream_id} file_path:{file_path}")

    if not wd.path("streams", str(stream_id)).exists():
        raise HTTPException(404, detail="Stream doesn't exists")

    dst = wd.mkdir("streams", str(stream_id), file_path, is_file=True)

    with open(dst.with_suffix(dst.suffix + ".crt"), "w") as f:
        f.write(json.dumps(cert_details))

    with open(dst, "wb") as f:
        async for chunk in req.stream():
            f.write(chunk)

    return Response(status_code=202)


@app.get("/streams/probe/{stream_id}/{file_path:path}")
def ffprobe_check(file_path: str, stream_id: int):

    p = wd.path("streams", str(stream_id), file_path)

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


# TODO app.post("streams/convert/{stream_id}")
# Will execute a ffmpeg conversion of the uploaded stream to a regular mp4
# for easier consumption. Need to double check first if Firefox/Chrome can
# read DASH media with CMAF tracks. If they can no need to actually convert
# to a new format.


@app.get("/certs", status_code=200)
def list_certs():
    server = [f.name for f in pathlib.Path(wd.path("server")).iterdir()]
    device = [f.name for f in pathlib.Path(wd.path("device")).iterdir()]

    return {"server": server, "device": device}


@app.post("/certs/{name}/keypair")
def create_client_keypair(name: str, override: bool = True):
    (key, cert, created) = device_hierarchy.gen_keypair(name, override)

    return {key, cert, created}


class SignClientCertificate(BaseModel):
    csr: str


@app.post("/certs/{name}/issue")
def sign_client_certificate(
    name: str, req: SignClientCertificate, override: bool = True
):
    (key, cert, created) = device_hierarchy.gen_cert(name, req.csr, override)

    return {key, cert, created}


def run(host: Optional[str], port: Optional[int], working_directory: Optional[str], dns: Optional[str]):
    global wd, device_hierarchy
    # For the Test Harness implementation, will need to find a way to return a reference
    # to the server running in the background. This function currently doesn't return.
    with WorkingDirectory(working_directory) as directory:

        # Sharing state with the various endpoints
        wd = directory

        dns = "localhost" if dns is None else f"{dns}._http._tcp_.local."

        # Create CA hierarchies (for webserver and devices)
        device_hierarchy = CAHierarchy(directory.mkdir("certs", "device"), "device")
        server_hierarchy = CAHierarchy(directory.mkdir("certs", "server"), "server")
        (server_key_file, server_cert_file, _) = server_hierarchy.gen_keypair(dns, override=True)

        # mDNS configuration. Registration only happen if the dns isn't localhost.
        zeroconf = Zeroconf()
        svc_info = None

        if dns != "localhost":
            svc_info = ServiceInfo(
                "_http._tcp.local.",
                name=dns,
                addresses=[socket.inet_aton("127.0.0.1")],
                port=1234,
            )
            # Advertise over mDNS
            logging.info("Advertising the service as %s", svc_info)
            zeroconf.register_service(svc_info)

        # Streams holder
        wd.mkdir("streams")

        # Setup the web server
        try:
            uvicorn.run(
                app,
                host=host,
                port=port,
                ssl_keyfile=server_key_file,
                ssl_certfile=server_cert_file,
                ssl_cert_reqs=ssl.CERT_OPTIONAL,
                ssl_ca_certs=device_hierarchy.root_cert_path,
            )
        finally:
            if dns != "localhost":
                zeroconf.unregister_service(svc_info)

        directory.print_tree()


# TODO UI
# html page to provide a good way to see uploaded content (file "browser" + video player)

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

    run(args.host, args.port, args.working_directory, args.dns)
