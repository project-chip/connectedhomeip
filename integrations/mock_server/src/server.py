# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import http.server
import logging
import socketserver
import ssl
from pathlib import Path

from handler import createMockServerHandler
from route_configuration import Configuration, load_configurations

log = logging.getLogger(__name__)


def run_server(
    port: int,
    config_path: Path,
    routing_config_dir: Path,
    cert_path: Path | None,
    key_path: Path | None,
    use_https: bool = True
) -> None:
    """
    Starts an HTTP or HTTPS server with mock endpoints defined by configuration files.

    Initializes and runs a threaded server that handles requests according to
    configured routes. The server can use TLS for secure communication (HTTPS)
    or run in plain HTTP mode for reverse tunnel scenarios.

    Args:
        port (int): Port number on which the server will listen
        config_path (Path): Path to the main configuration file
        routing_config_dir (Path): Directory containing additional routing configuration files
        cert_path (Path | None): Path to the SSL/TLS certificate file (required for HTTPS)
        key_path (Path | None): Path to the SSL/TLS private key file (required for HTTPS)
        use_https (bool): Whether to use HTTPS (True) or HTTP (False). Defaults to True.

    Returns:
        None

    Raises:
        ssl.SSLError: If there are issues with the SSL/TLS certificate or key
        OSError: If the port is already in use or permission is denied
        ValueError: If configuration files are invalid or missing
        KeyboardInterrupt: When the server is stopped using Ctrl+C

    Example:
        # HTTPS mode
        run_server(
            port=8443,
            config_path=Path("config/main.json"),
            routing_config_dir=Path("config/routes"),
            cert_path=Path("certs/server.crt"),
            key_path=Path("certs/server.key"),
            use_https=True
        )

        # HTTP mode (for reverse tunnel)
        run_server(
            port=8080,
            config_path=Path("config/main.json"),
            routing_config_dir=Path("config/routes"),
            cert_path=None,
            key_path=None,
            use_https=False
        )

    Note:
        - Server runs until interrupted by keyboard (Ctrl+C)
        - Logs are written to stdout with DEBUG level
        - Server binds to all available network interfaces ("")
        - Uses ThreadingHTTPServer for concurrent request handling
        - HTTP mode is useful for Android reverse tunnel access via adb
    """

    logging.basicConfig(level=logging.DEBUG, format="[%(levelname)s] %(message)s")

    if not config_path.is_file():
        raise ValueError(f"'{config_path}' is not a file")

    if not routing_config_dir.is_dir():
        raise ValueError(f"'{routing_config_dir}' is not a directory")

    if use_https:
        if cert_path is None or not cert_path.is_file():
            raise ValueError(f"'{cert_path}' is not a file")

        if key_path is None or not key_path.is_file():
            raise ValueError(f"'{key_path}' is not a file")

    config: Configuration = load_configurations(config_path, routing_config_dir)
    server_address: socketserver._AfInetAddress = ("", port)

    theMockServerHandler = createMockServerHandler(config)
    httpd = http.server.ThreadingHTTPServer(server_address, theMockServerHandler)

    log.info("Server starting on port %s", port)

    if use_https:
        context: ssl.SSLContext = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(certfile=cert_path, keyfile=key_path)
        with context.wrap_socket(httpd.socket, server_side=True) as httpd.socket:
            log.info("Server started on port %s (HTTPS)", port)
            log.info("HTTPS enabled with cert: %s and key: %s", cert_path, key_path)
            try:
                httpd.serve_forever()
            except KeyboardInterrupt:
                log.info("Server is shutting down due to keyboard interrupt.")
                httpd.server_close()
    else:
        log.info("Server started on port %s (HTTP)", port)
        log.warning("Running in HTTP mode - no TLS encryption")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            log.info("Server is shutting down due to keyboard interrupt.")
            httpd.server_close()
