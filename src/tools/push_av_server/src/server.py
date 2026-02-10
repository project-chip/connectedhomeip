"""
Main entry point for the Push AV Server.
"""

import argparse
import asyncio
import logging
import signal
import socket
import ssl
from typing import Awaitable, Callable, Optional

from api import PushAvServer
from certificates import CAHierarchy
from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles
from streams import StreamService
from utils import WorkingDirectory, static_path
from zeroconf import ServiceInfo, Zeroconf

log = logging.getLogger(__name__)


class PushAvContext:
    """
    Hold the context for a full Push AV Server including temporary disk,
    CA hierarchies and web server.
    """

    def __init__(
        self,
        host: Optional[str],
        port: Optional[int],
        working_directory: Optional[str],
        dns: Optional[str],
        server_ip: Optional[str],
        strict_mode: bool
    ):
        self.directory = WorkingDirectory(working_directory)
        self.host = host
        self.port = port
        self.dns = "localhost" if dns is None else f"{dns}._http._tcp.local."
        self.strict_mode = strict_mode

        # Create CA hierarchies (for webserver and devices)
        self.device_hierarchy = CAHierarchy(
            self.directory.mkdir("certs", "device"), "device", "client"
        )
        self.server_hierarchy = CAHierarchy(
            self.directory.mkdir("certs", "server"), "server", "server"
        )
        (self.server_key_file, self.server_cert_file, _) = self.server_hierarchy.gen_keypair(
            self.dns, ip_address=server_ip
        )

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

        # Create stream service
        self.stream_service = StreamService(self.directory)

        # Setup FastAPI app
        logger = logging.getLogger("hypercorn.error")
        self.app = FastAPI()
        self.app.mount("/static", StaticFiles(directory=static_path), name="static")

        # Include API router
        pas = PushAvServer(self.stream_service, self.device_hierarchy, strict_mode)
        self.app.include_router(pas.router)

        # Setup exception handler
        @self.app.exception_handler(Exception)
        async def http_exception_handler(request, exc):
            if isinstance(exc, HTTPException):
                # Re-raise HTTPException for FastAPI's default handler to process
                raise exc
            logger.error(f"Exception: {exc}")
            return JSONResponse(
                status_code=500,
                content={"detail": str(exc)}
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
        """Clean up resources."""
        self.directory.cleanup()


def main():
    """Main entry point for the Push AV Server."""
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
    parser.add_argument(
        "--server-ip",
        help="The IP address of the server to include in the SSL certificate."
    )
    parser.add_argument(
        "--strict-mode",
        action='store_true',
        help="When enabled, upload must happen on the path described by the Matter specification"
    )

    args = parser.parse_args()

    with PushAvContext(
        args.host,
        args.port,
        args.working_directory,
        args.dns,
        args.server_ip,
        args.strict_mode
    ) as ctx:
        shutdown_event = asyncio.Event()

        def _signal_handler():
            print("SIGINT received. Shutting down web server.")
            shutdown_event.set()

        with asyncio.Runner() as runner:
            runner.get_loop().add_signal_handler(signal.SIGINT, _signal_handler)
            runner.run(ctx.start(shutdown_trigger=shutdown_event.wait))


if __name__ == "__main__":
    main()
