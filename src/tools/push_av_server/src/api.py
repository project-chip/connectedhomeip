"""
FastAPI router and endpoints for the Push AV Server.
"""

import datetime
import json
import logging
import pathlib
import subprocess

from certificates import CAHierarchy
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from fastapi import APIRouter, HTTPException, Query, Request, Response
from fastapi.responses import FileResponse, HTMLResponse, JSONResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from models import SignClientCertificate, SupportedIngestInterface, TrackNameRequest
from streams import StreamService
from utils import templates_path
from validation import MatterCMAFUploadValidator

log = logging.getLogger(__name__)


class PushAvServer:
    """Main FastAPI server for Push AV media ingestion and validation."""

    def __init__(self, stream_service: StreamService, device_hierarchy: CAHierarchy, strict_mode: bool):
        self.stream_service = stream_service
        self.device_hierarchy = device_hierarchy
        self.strict_mode = strict_mode
        self.validator = MatterCMAFUploadValidator()
        self.router = APIRouter()
        self.templates = Jinja2Templates(directory=templates_path)

        self._setup_routes()

    def _setup_routes(self):
        """Setup all API routes."""
        # UI routes
        self.router.add_api_route("/", self.index, methods=["GET"], response_class=RedirectResponse)
        self.router.add_api_route("/ui/streams", self.ui_streams_list, methods=["GET"], response_class=HTMLResponse)
        self.router.add_api_route("/ui/streams/{stream_id}", self.ui_streams_details, methods=["GET"])
        self.router.add_api_route("/ui/streams/{stream_id}/{file_path:path}", self.ui_streams_file_details, methods=["GET"])
        self.router.add_api_route("/ui/certificates", self.ui_certificates_list, methods=["GET"], response_class=HTMLResponse)
        self.router.add_api_route("/ui/certificates/{hierarchy}/{name}",
                                  self.ui_certificates_details, methods=["GET"], response_class=HTMLResponse)

        # HTTP API routes
        self.router.add_api_route("/streams", self.create_stream, methods=["POST"], status_code=201)
        self.router.add_api_route("/streams", self.list_streams, methods=["GET"])
        self.router.add_api_route("/streams/probe/{stream_id}/{file_path:path}", self.ffprobe_check, methods=["GET"])
        self.router.add_api_route("/streams/{stream_id}/{file_path:path}.{ext}", self.handle_upload, methods=["PUT"])
        self.router.add_api_route("/streams/{stream_id}/{file_path:path}", self.segment_download, methods=["GET"])
        self.router.add_api_route("/streams/{stream_id}/trackName", self.update_track_name, methods=["POST"], status_code=202)

        # Certificate routes
        self.router.add_api_route("/certs", self.list_certs, methods=["GET"], status_code=200)
        self.router.add_api_route("/certs/{hierarchy}/{name}", self.certificate_details, methods=["GET"], status_code=200)
        self.router.add_api_route("/certs/{name}/keypair", self.create_client_keypair, methods=["POST"])
        self.router.add_api_route("/certs/{name}/sign", self.sign_client_certificate, methods=["POST"])

    # UI endpoints
    def index(self):
        """Redirect to streams list."""
        return RedirectResponse("/ui/streams")

    def ui_streams_list(self, request: Request):
        """Render streams list UI."""
        s = self.list_streams()
        return self.templates.TemplateResponse(
            request=request, name="streams_list.jinja2", context={"streams": s["streams"]}
        )

    def ui_streams_details(self, request: Request, stream_id: int):
        """Render stream details UI."""
        stream = self.stream_service.get_stream(stream_id)
        if stream is None:
            raise HTTPException(status_code=400, detail="Stream ID doesn't exist")
        return self.templates.TemplateResponse(request=request, name="streams_details.jinja2", context={'stream': stream})

    def ui_streams_file_details(self, request: Request, stream_id: int, file_path: str):
        """Render file details UI."""
        context = {}
        context['streams'] = self.list_streams()['streams']
        context['stream_id'] = stream_id
        context['file_path'] = file_path

        if file_path.endswith('.crt'):
            context['type'] = 'cert'
            p = self.stream_service.wd.path("streams", str(stream_id), file_path)
            with open(p, "r") as f:
                context['cert'] = json.load(f)
        else:
            context['type'] = 'media'
            context['probe'] = self.ffprobe_check(stream_id, file_path)
            context['pretty_probe'] = json.dumps(context['probe'], sort_keys=True, indent=4)

        return self.templates.TemplateResponse(request=request, name="streams_file_details.jinja2", context=context)

    def ui_certificates_list(self, request: Request):
        """Render certificates list UI."""
        return self.templates.TemplateResponse(
            request=request, name="certificates_list.jinja2", context={"certs": self.list_certs()}
        )

    def ui_certificates_details(self, request: Request, hierarchy: str, name: str):
        """Render certificate details UI."""
        context = self.certificate_details(hierarchy, name)
        context["certs"] = self.list_certs()
        return self.templates.TemplateResponse(request=request, name="certificates_details.jinja2", context=context)

    # Stream API endpoints
    def create_stream(self, interface: SupportedIngestInterface = Query(default=SupportedIngestInterface.cmaf)):
        """Create a new stream."""
        stream_id = self.stream_service.get_next_stream_id()
        return self.stream_service.create_stream(stream_id, interface, self.strict_mode)

    def list_streams(self):
        """List all streams."""
        return {"streams": self.stream_service.list_streams()}

    async def handle_upload(self, stream_id: int, file_path: str, ext: str, req: Request):
        """
        Handle file upload for a given stream.
        Validate the file name based on the extension and path format.
        Always save the uploaded file to disk for further analysis.
        If strict mode is enabled, return bad requests with the errors if any.
        """
        log.debug(f"Upload started: stream={stream_id}, file={file_path}.{ext}")

        with self.stream_service.open_stream(stream_id) as stream:
            if stream is None:
                raise HTTPException(status_code=400, detail="Stream ID doesn't exist")

            file_path_with_ext = f"{file_path}.{ext}"
            session = stream.last_in_progress_session()
            body = await req.body()

            # Validate the incoming file upload
            errors, session = self.validator.validate_upload(
                stream, session, file_path, ext, body, dict(req.headers)
            )

            # Save the file to disk
            file_local_path = self.stream_service.wd.mkdir("streams", str(stream_id), file_path_with_ext, is_file=True)

            # If file already exists, create versioned backup
            if file_local_path.exists():
                timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
                backup_path = file_local_path.with_stem(f"{file_local_path.stem}.{timestamp}")
                file_local_path.rename(backup_path)
                log.info(f"Backed up existing file to {backup_path}")

            # Save certificate details if available
            cert_details = req.scope["extensions"]["ssl"].get('client_certificate', None)
            if cert_details:
                with open(file_local_path.with_suffix(file_local_path.suffix + ".crt"), "w") as f:
                    f.write(json.dumps(cert_details))
            else:
                errors.append("File upload did not happen with SSL context")

            # Save the file to disk
            with open(file_local_path, "wb") as f:
                f.write(body)

            session_id = session.id if session else None

            if len(errors) > 0:
                self.stream_service.add_error_upload(stream, session_id, file_path_with_ext, errors)
            else:
                self.stream_service.add_valid_upload(stream, session_id, file_path_with_ext)

            if stream.strict_mode and len(errors) > 0:
                log.warning(f"Upload validation failed: {errors}")
                return JSONResponse(status_code=400, content={"errors": errors})

            log.info(f"Upload successful: stream={stream_id}, file={file_path}.{ext}, errors={errors}, strict={stream.strict_mode}")
            return Response(status_code=202)

    def ffprobe_check(self, stream_id: int, file_path: str):
        """Analyze media file using ffprobe."""
        p = self.stream_service.wd.path("streams", str(stream_id), file_path)
        if not p.exists():
            raise HTTPException(404, detail="Media file doesn't exists")

        cmd = [
            "ffprobe", "-allowed_extensions", "init,m4s",
            "-show_streams", "-show_format", "-output_format", "json",
            str(p.absolute())
        ]

        proc = subprocess.run(cmd, capture_output=True)

        if proc.returncode != 0:
            stderr_text = proc.stderr.decode('utf-8', errors='replace')
            raise HTTPException(
                500,
                detail={
                    "message": "ffprobe failed to analyze the media file",
                    "stderr": stderr_text,
                    "command": " ".join(cmd)
                }
            )

        return json.loads(proc.stdout)

    async def segment_download(self, stream_id: int, file_path: str):
        """Download a media segment."""
        return FileResponse(self.stream_service.wd.path("streams", str(stream_id), file_path))

    async def update_track_name(self, stream_id: int, track_request: TrackNameRequest):
        """Update the track_name for a given stream_id."""
        stream = self.stream_service.get_stream(stream_id)
        if stream is None:
            raise HTTPException(status_code=400, detail="Stream ID doesn't exist")

        stream.track_name = track_request.track_name
        self.stream_service.update_stream(stream)

    # Certificate API endpoints
    def list_certs(self):
        """List all certificates."""
        server = [f.name for f in pathlib.Path(self.stream_service.wd.path("certs", "server")).iterdir()]
        device = [f.name for f in pathlib.Path(self.stream_service.wd.path("certs", "device")).iterdir()]
        return {"server": server, "device": device}

    def certificate_details(self, hierarchy: str, name: str):
        """Get certificate details."""
        data = pathlib.Path(self.stream_service.wd.path("certs", hierarchy, name)).read_bytes()
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
                "issuer": cert.issuer.rfc4514_string(),
                "subject": cert.subject.rfc4514_string(),
                "extensions": [str(ext) for ext in cert.extensions]
            }

        return {"type": type, "key": key, "cert": cert}

    def create_client_keypair(self, name: str, override: bool = True):
        """Create a client keypair."""
        (key, cert, created) = self.device_hierarchy.gen_keypair(name, override)
        return {"key": key, "cert": cert, "created": created}

    def sign_client_certificate(self, name: str, req: SignClientCertificate, override: bool = True):
        """Sign a client certificate."""
        (key, cert, created) = self.device_hierarchy.gen_cert(name, req.csr, override)
        return {"key": key, "cert": cert, "created": created}
