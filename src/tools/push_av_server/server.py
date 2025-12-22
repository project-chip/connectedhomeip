import argparse
import asyncio
import contextlib
import datetime
import ipaddress
import json
import logging
import os.path
import pathlib
import random
import re
import signal
import socket
import ssl
import string
import subprocess
import sys
import tempfile
import xml.etree.ElementTree
from enum import Enum
from pathlib import Path
from typing import Awaitable, Callable, Literal, Optional, Tuple

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives.asymmetric.types import CertificateIssuerPrivateKeyTypes, CertificatePublicKeyTypes
from cryptography.x509.oid import NameOID, ExtendedKeyUsageOID
from fastapi import APIRouter, FastAPI, HTTPException, Request, Response
from fastapi.responses import FileResponse, HTMLResponse, JSONResponse, RedirectResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel
from zeroconf import ServiceInfo, Zeroconf

log = logging.getLogger(__name__)

module_dir_path = os.path.dirname(os.path.realpath(__file__))
templates_path = os.path.join(module_dir_path, "templates")
static_path = os.path.join(module_dir_path, "static")
'''
The initialisation segments must have .init extension as per CMAF-Ingest requirements.
https://dashif.org/Ingest/#interface-2-naming
'''
VALID_EXTENSIONS = ["mpd", "m3u8", "m4s", "init"]


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

            log.info(f"CA Hierarchy loaded from disk: {self.name}")
        elif self.root_key_path.exists() or self.root_cert_path.exists():
            # Only one of the two file exists, bailing out
            log.error("root certificate partially exist on disk, stopping early")
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

            log.info(f"CA Hierarchy generated: {self.name}")

    def _save_cert(
        self,
        name: str,
        cert: x509.Certificate,
        key: Optional[CertificateIssuerPrivateKeyTypes],
        bundle_root: bool,
    ) -> tuple[Optional[Path], Path]:
        """
        Private method that help with saving certificate and key to the hierarchy folder.
        This tool isn't meant to be used in production, but instead to help with development
        and as such have the goal to make the CA hierarchy as available as possible, which in
        turn make it very unsecure.
        """
        cert_path = self.directory / f"{name}.pem"
        key_path = self.directory / f"{name}.key" if key else None

        if key and key_path:
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
        duration: datetime.timedelta,
        ip_address: Optional[str] = None
    ) -> x509.Certificate:
        """
        Generate and sign a certificate.
        """
        # Use ip_address for Common Name if provided, otherwise use dns
        common_name = ip_address if ip_address else dns

        # Sign certificate
        subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "TC_PAVS"),
                x509.NameAttribute(NameOID.COMMON_NAME, common_name),
            ]
        )

        extended_key_usage = [ExtendedKeyUsageOID.CLIENT_AUTH] if self.kind == "client" else [
            ExtendedKeyUsageOID.SERVER_AUTH]

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
            san_names: list[x509.DNSName | x509.IPAddress] = [x509.DNSName(dns)]
            if ip_address:
                san_names.append(x509.IPAddress(ipaddress.ip_address(ip_address)))
            builder.add_extension(
                x509.SubjectAlternativeName(san_names),
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

        log.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)

        return (key_path, cert_bundle_path, False)

    def gen_keypair(self, dns: str,
                    override=False,
                    duration: datetime.timedelta = datetime.timedelta(hours=1),
                    ip_address: Optional[str] = None) -> tuple[Path, Path, bool]:
        """
        Generate a private key as well as the associated certificate signed by this CA
        hierarchy. Returns the path to the key, cert, and whether it was reused or not.
        """

        # If we don't always override, first check if an existing keypair already exists
        if not override:
            cert_path = self.directory / f"{dns}.pem"
            key_path = self.directory / f"{dns}.key"

            if cert_path.exists() and key_path.exists():
                cert = x509.load_pem_x509_certificate(cert_path.read_bytes())

                if datetime.datetime.now() > cert.not_valid_after:
                    # We only reuse the certificate/key if the cert is still valid
                    return (key_path, cert_path, True)

        # Generate private key
        key = rsa.generate_private_key(public_exponent=65537, key_size=2048)

        # Sign certificate
        cert = self._sign_cert(dns, key.public_key(), duration, ip_address=ip_address)

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(dns, cert, key, bundle_root=True)

        if key_path is None:
            raise ValueError("Key path should always be set")

        log.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)

        return (key_path, cert_bundle_path, False)


class SignClientCertificate(BaseModel):
    """Request model to sign a client certificate"""
    csr: str


class TrackNameRequest(BaseModel):
    """Request model to update track name for a stream"""
    track_name: str


class SupportedIngestInterface(str, Enum):
    cmaf = "cmaf-ingest"  # Interface 1
    dash = "dash"  # Interface 2, DASH version
    hls = "hls"  # Interface 2, HLS version


class UploadError(BaseModel):
    session_id: Optional[int]
    file_path: str
    reasons: list[str]


class Session(BaseModel):
    id: int  # TODO Ignore and use the index in the list instead?

    uploaded_segments: list[Tuple[str, str]] = []
    uploaded_manifests: list[Tuple[str, str]] = []
    complete: bool = False


class Stream(BaseModel):
    # Configuration of the PushAv stream
    id: int
    strict_mode: bool = True
    interface: SupportedIngestInterface
    track_name: Optional[str] = None

    # Keep track of the various sessions encountered
    sessions: list[Session] = []
    errors: list[UploadError] = []

    # Utilities

    def save_to_disk(self, wd: WorkingDirectory):
        p = wd.path("streams", str(self.id), "stream.json")
        with open(p, 'w', encoding='utf-8') as f:
            json.dump(self.model_dump(), f, ensure_ascii=False, indent=4)

    def new_session(self) -> Session:
        session_id = len(self.sessions) + 1
        session = Session(id=session_id)
        self.sessions.append(session)
        return session

    def last_in_progress_session(self) -> Optional[Session]:
        if len(self.sessions) == 0:
            return None

        last_session = self.sessions[-1]
        if not last_session.complete:
            return last_session

        return None


class PushAvServer:

    templates = Jinja2Templates(directory=templates_path)

    def __init__(self, wd: WorkingDirectory, device_hierarchy: CAHierarchy, strict_mode: bool):
        self.wd = wd
        self.device_hierarchy = device_hierarchy
        self.strict_mode = strict_mode
        self.router = APIRouter()

        # In-memory map to track camera streams
        self.streams = self._list_streams()

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

        self.router.add_api_route("/streams/{stream_id}/{file_path:path}.{ext}", self.handle_upload, methods=["PUT"])

        self.router.add_api_route("/streams/{stream_id}/{file_path:path}", self.segment_download, methods=["GET"])
        self.router.add_api_route("/streams/{stream_id}/trackName", self.update_track_name, methods=["POST"], status_code=202)
        self.router.add_api_route("/certs", self.list_certs, methods=["GET"], status_code=200)
        self.router.add_api_route("/certs/{hierarchy}/{name}", self.certificate_details, methods=["GET"], status_code=200)
        self.router.add_api_route("/certs/{name}/keypair", self.create_client_keypair, methods=["POST"])
        self.router.add_api_route("/certs/{name}/sign", self.sign_client_certificate, methods=["POST"])

    # Utilities

    def _read_stream_details(self, stream_id: int):
        # TODO Remove
        p = self.wd.path("streams", str(stream_id), "details.json")

        try:
            with open(p, 'r') as file:
                return json.load(file)
        except FileNotFoundError:
            raise HTTPException(404, detail="Stream doesn't exists")
        except Exception as e:
            raise HTTPException(500, f"An unexpected error occurred: {e}")

    def _list_streams(self):
        streams: dict[str, Stream] = {}

        for stream_path in self.wd.path("streams").iterdir():
            if stream_path.is_dir():
                stream_file = stream_path / "stream.json"
                if stream_file.exists():
                    with open(stream_file, 'r', encoding='utf-8') as f:
                        stream_data = json.load(f)
                        streams[stream_path.name] = Stream.model_validate(stream_data)
        return streams

    @contextlib.contextmanager
    def _open_stream(self, stream_id: int):
        """Context manager helper to save a stream after use.

        Note that any exceptions raised within the context will prevent streams from being saved to disk.
        """
        stream_id_str = str(stream_id)
        yield self.streams[stream_id_str]

        # TODO Look if we need the following try/catch block if the default behavior of the framework
        # is good enough for debugging purposes.
        # except Exception as e:
        #    raise HTTPException(status_code=500, detail=f"Failed to write stream details: {e}")
        self.streams[stream_id_str].save_to_disk(self.wd)

    # UI website

    def index(self):
        return RedirectResponse("/ui/streams")

    def ui_streams_list(self, request: Request):
        s = self.list_streams()
        return self.templates.TemplateResponse(
            request=request, name="streams_list.jinja2", context={"streams": s["streams"]}
        )

    # TODO Change what we show in here.
    # Stream listing page should only show the number of sessions and errors.
    # Stream details should provide the ffprobe of the index.mpd, the session history details (including errors?),
    # the stream configuration, and the per-file details (should reconsider if it's actually useful for media)
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
        elif file_path == 'details.json':
            context['type'] = 'details'
            context['details'] = self._read_stream_details(stream_id)
        else:
            context['type'] = 'media'
            context['probe'] = self.ffprobe_check(stream_id, file_path)
            context['pretty_probe'] = json.dumps(context['probe'], sort_keys=True, indent=4)

        return self.templates.TemplateResponse(request=request, name="streams_details.jinja2", context=context)

    def ui_certificates_list(self, request: Request):
        return self.templates.TemplateResponse(
            request=request, name="certificates_list.jinja2", context={"certs": self.list_certs()}
        )

    def ui_certificates_details(self, request: Request, hierarchy: str, name: str):
        context = self.certificate_details(hierarchy, name)
        context["certs"] = self.list_certs()

        return self.templates.TemplateResponse(request=request, name="certificates_details.jinja2", context=context)

    # APIs

    def create_stream(self, interface: SupportedIngestInterface = SupportedIngestInterface.cmaf):
        # Find the last registered stream
        dirs = [d for d in pathlib.Path(self.wd.path("streams")).iterdir() if d.is_dir()]
        last_stream = int(dirs[-1].name) if len(dirs) > 0 else 0
        stream_id = last_stream + 1
        stream_id_str = str(stream_id)

        # Initialize entry in stream files map
        stream = Stream(
            id=stream_id,
            strict_mode=self.strict_mode,
            interface=interface,
        )
        self.streams[stream_id_str] = stream

        self.wd.mkdir("streams", str(stream_id))
        self.streams[stream_id_str].save_to_disk(self.wd)

        return stream  # TODO Update TH to use sessions instead

    def list_streams(self):
        return {"streams": self.streams.values()}

    async def handle_upload(self, stream_id: int, file_path: str, ext: str, req: Request):
        """
            Handle file upload for a given stream.

            Validate the file name based on the extension and path format.
            Always save the uploaded file to disk for further analysis.
            If strict mode is enabled, return bad requests with the errors if any.
        """
        with self._open_stream(stream_id) as stream:
            file_path_with_ext = f"{file_path}.{ext}"
            session = stream.last_in_progress_session()
            body = await req.body()

            # Validate the incoming file upload (path and extension)
            errors = []

            if ext == "mpd":
                # DASH manifest files
                if (stream.interface != SupportedIngestInterface.dash):
                    errors.append("Unsupported manifest object extension")

                if session is None:
                    session = stream.new_session()

                session.uploaded_segments.append((file_path_with_ext, file_path_with_ext + ".crt"))

                # TODO Validate the path is always session_name/index.mpd

                root = xml.etree.ElementTree.fromstring(body)
                mpd_type = root.attrib.get('type')

                if mpd_type == "dynamic" and len(session.uploaded_segments) > 0:
                    errors.append("Dynamic MPD cannot be uploaded after segments have been uploaded")

                if mpd_type == "static" and len(session.uploaded_segments) == 0:
                    errors.append("Static MPD cannot be uploaded before segments have been uploaded")

                if mpd_type == "static":
                    session.complete = True
            elif ext == "m3u8":
                # HLS manifest files
                if stream.interface != SupportedIngestInterface.hls:
                    errors.append("Unsupported manifest object extension")

                if session is None:
                    session = stream.new_session()

                session.uploaded_segments.append((file_path_with_ext, file_path_with_ext + ".crt"))

                # TODO Lifecycle validation for HLS manifests
            elif ext == "m4s" or ext == "init":
                # Segmented video files

                if session is not None:
                    session.uploaded_segments.append((file_path_with_ext, file_path_with_ext + ".crt"))
                else:
                    errors.append("No active session when uploading " + file_path_with_ext)

                # The Track's init segment is uploaded as `session_name/track_name/track_name.init`
                #
                # `/session_1/index.mpd` - Initial upload. Has `MPD@type="dynamic"`.
                # `/session_1/video1/video1.init`
                # `/session_1/audio1/audio1.init`
                # `/session_1/video1/segment_1001.m4s`
                # `/session_1/audio1/segment_1001.m4s`
                # `/session_1/video1/segment_1002.m4s`
                # `/session_1/audio1/segment_1002.m4s`
                # `/session_1/video1/segment_1003.m4s`
                # `/session_1/audio1/segment_1003.m4s`
                # `/session_1/index.mpd` - Final upload. Has `MPD@type="static"`.

                # TODO Make sure this is correct. Most likely not at this point.

                path_regex = r"^session_\d+/(?P<trackName>[^/]+)/segment_\d+$"
                if ext == "init":
                    path_regex = r"^session_\d+/(?P<trackName>.init$"
                path_regex = re.compile(path_regex)

                match = path_regex.match(file_path)
                if not match:
                    errors.append("Path does not adhere to Matter's path format")
                else:
                    # Validate if the trackName is same as the one assigned during transport allocation.
                    # https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/PushAVStreamTransport.adoc#685-trackname-field
                    track_name_in_path = match.group("trackName")
                    track_name = stream.track_name

                    # TODO Need to find out where the number after the symbolic track name is coming from
                    if track_name and track_name != track_name_in_path:
                        errors.append("Track name mismatch: "
                                      f"{track_name_in_path} != {track_name}, "
                                      "must match TrackName provided in ContainerOptions")
            else:
                errors.append(f"Invalid extension: {ext}, valid extensions are {', '.join(VALID_EXTENSIONS)}")

            # Validation complete, now saving data to disk

            if len(errors) > 0:
                session_id = session.id if session else None
                stream.errors.append(UploadError(session_id=session_id, file_path=file_path_with_ext, reasons=errors))

            file_local_path = self.wd.mkdir("streams", str(stream_id), file_path_with_ext, is_file=True)

            cert_details = req.scope["extensions"]["ssl"]["client_certificate"]

            # TODO If file already exists, come up with a way to version it instead of overwriting it.
            with open(file_local_path.with_suffix(file_local_path.suffix + ".crt"), "w") as f:
                f.write(json.dumps(cert_details))

            with open(file_local_path, "wb") as f:
                f.write(body)

            # And finally return the appropriate response to the camera

            if stream.strict_mode and len(errors) > 0:
                return JSONResponse(
                    status_code=400,
                    content={"errors": errors}
                )
            else:
                return Response(status_code=202)

    def ffprobe_check(self, stream_id: int, file_path: str):

        p = self.wd.path("streams", str(stream_id), file_path)

        if not p.exists():
            raise HTTPException(404, detail="Media file doesn't exists")

        cmd = [
            "ffprobe", "-allowed_extensions", "init,m4s",
            "-show_streams", "-show_format", "-output_format", "json",
            str(p.absolute())
            ]

        print(cmd)
        # ffprobe -show_streams -show_format -output_format json /Users/francoismonniot/.pavstest/streams/1/index.mpd

        proc = subprocess.run(
            cmd,
            capture_output=True
        )

        if proc.returncode != 0:
            # TODO Add more details (maybe stderr) to the response
            raise HTTPException(500, "ffprobe failed to analyze the media file")

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

    # Seems unused in the current TH tests
    # TODO Verify in spec how a track name updated should be handled mid-stream
    async def update_track_name(self, stream_id: int, track_request: TrackNameRequest):
        """Updates the track_name for a given stream_id."""
        with self._open_stream(stream_id) as stream:
            stream.track_name = track_request.track_name

    def sign_client_certificate(
        self, name: str, req: SignClientCertificate, override: bool = True
    ):
        (key, cert, created) = self.device_hierarchy.gen_cert(name, req.csr, override)

        # TODO Verify that key is always None, and if true, get rid of it
        return {key, cert, created}


class PushAvContext:
    """Hold the context for a full Push AV Server including temporary disk, CA hierarchies and web server"""

    def __init__(self, host: Optional[str], port: Optional[int], working_directory: Optional[str], dns: Optional[str], server_ip: Optional[str], strict_mode: bool):
        self.directory = WorkingDirectory(working_directory)
        self.host = host
        self.port = port
        self.dns = "localhost" if dns is None else f"{dns}._http._tcp.local."
        self.strict_mode = strict_mode

        # Create CA hierarchies (for webserver and devices)
        self.device_hierarchy = CAHierarchy(self.directory.mkdir("certs", "device"), "device", "client")
        self.server_hierarchy = CAHierarchy(self.directory.mkdir("certs", "server"), "server", "server")
        (self.server_key_file, self.server_cert_file, _) = self.server_hierarchy.gen_keypair(self.dns, ip_address=server_ip)

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

        logger = logging.getLogger("hypercorn.error")
        self.app = FastAPI()
        self.app.mount("/static", StaticFiles(directory=static_path), name="static")
        pas = PushAvServer(self.directory, self.device_hierarchy, strict_mode)
        self.app.include_router(pas.router)

        @self.app.exception_handler(HTTPException)
        async def http_exception_handler(request: Request, exc: HTTPException):
            logger.error(
                f"HTTPExecption: {exc.status_code} {exc.detail}"
            )
            return JSONResponse(
                status_code=exc.status_code,
                content={"detail": exc.detail}
            )

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.cleanup()

    async def start(self, shutdown_trigger: Optional[Callable[..., Awaitable]] = None):
        """
        Start the PUSH AV server. Note that method do not check if a server is already running.
        """
        # Advertise over mDNS
        if self.svc_info:
            log.info("Advertising the service as %s", self.svc_info)
            self.zeroconf.register_service(self.svc_info)

        # Start the web server
        from hypercorn.asyncio import serve
        from hypercorn.config import Config
        bind = (self.host or "127.0.0.1") + ":" + (str(self.port or 8000))
        config = Config.from_mapping(
            bind=bind,
            quic_bind=bind,
            alpn_protocols=["h2"],
            keyfile=self.server_key_file,
            certfile=self.server_cert_file,
            ca_certs=self.device_hierarchy.root_cert_path,
            verify_mode=ssl.CERT_OPTIONAL
        )

        try:
            await serve(self.app, config, shutdown_trigger=shutdown_trigger)

        finally:
            if self.svc_info:
                self.zeroconf.unregister_service(self.svc_info)

    def cleanup(self):
        self.directory.cleanup()


if __name__ == "__main__":
    logging.basicConfig(
        format="%(asctime)s|%(name)-8s|%(levelname)-5s|%(message)s",
        level=logging.DEBUG,
        datefmt="%H:%M:%S",
    )
    logging.getLogger("hpack").setLevel(logging.WARNING)

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
    parser.add_argument("--server-ip", help="The IP address of the server to include in the SSL certificate.")
    parser.add_argument("--strict-mode", action='store_true',
                        help="When enabled, upload must happen on the path described by the Matter specification")

    args = parser.parse_args()

    with PushAvContext(args.host, args.port, args.working_directory, args.dns, args.server_ip, args.strict_mode) as ctx:

        shutdown_event = asyncio.Event()

        def _signal_handler():
            print("SIGINT received. Shutting down web server.")
            shutdown_event.set()

        with asyncio.Runner() as runner:
            runner.get_loop().add_signal_handler(signal.SIGINT, _signal_handler)
            runner.run(ctx.start(shutdown_trigger=shutdown_event.wait))
