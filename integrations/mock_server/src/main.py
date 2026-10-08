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

import argparse
from pathlib import Path

from server import run_server

"""
Mock HTTP/HTTPS Server for API Testing

This module serves as the entry point for a configurable mock server designed
for API testing. It provides a flexible way to simulate API endpoints with
configurable responses, supporting both HTTPS and HTTP modes.

The server supports:
- Custom routing configurations via JSON files
- HTTPS with TLS certificate support (default)
- HTTP mode for reverse tunnel access (e.g., Android via adb)
- Configurable port binding
- Multiple concurrent connections
- Dynamic response configuration

Usage:
    python main.py [--port PORT] [--config CONFIG_FILE]
                   [--routing-config-dir ROUTE_DIR]
                   [--cert CERT_FILE] [--key KEY_FILE]
                   [--http]

Arguments:
    --port PORT                 Port number to listen on (default: 8443)
    --config CONFIG_FILE        Path to main configuration file (default: config.json)
    --routing-config-dir DIR    Directory containing route configurations
                               (default: config.json)
    --cert CERT_FILE           Path to SSL certificate file (default: server.crt)
    --key KEY_FILE             Path to SSL private key file (default: server.key)
    --http                     Run in HTTP mode without TLS (for reverse tunnel)

Example:
    # HTTPS mode (default)
    python main.py --port 8443 --config ./config/main.json
                  --routing-config-dir ./config/routes
                  --cert ./certs/server.crt --key ./certs/server.key

    # HTTP mode (for Android reverse tunnel via adb)
    python main.py --port 8080 --config ./config/main.json
                  --routing-config-dir ./config/routes --http

Configuration:
    The server requires two types of configuration:
    1. Main configuration file (--config):
       Contains server-wide settings and default behaviors

    2. Route configuration directory (--routing-config-dir):
       Contains JSON files defining endpoint behaviors, responses,
       and matching criteria

Security:
    - HTTPS mode: Requires valid SSL certificate and private key
    - HTTP mode: No encryption, use only for local testing with reverse tunnels
    - TLS configuration uses server's default security settings

Notes:
    - Server runs until interrupted (Ctrl+C)
    - All endpoints return JSON by default
    - Logs to stdout with DEBUG level
    - Supports concurrent requests via threading
    - HTTP mode is useful for Android apps accessing localhost via:
      adb reverse tcp:<device_port> tcp:<host_port>
"""


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Mock HTTP/HTTPS Server for API testing")
    parser.add_argument("--port", type=int, default=8443, help="Set the server port")
    parser.add_argument("--config", type=str, default="config.json", help="Path to the common config file")
    parser.add_argument("--routing-config-dir", type=str, default="config.json", help="Path to the common config file")
    parser.add_argument("--cert", type=str, default="server.crt", help="SSL Certificate file")
    parser.add_argument("--key", type=str, default="server.key", help="SSL Private Key file")
    parser.add_argument("--http", action="store_true", help="Run server in HTTP mode (no TLS) for reverse tunnel access")

    args = parser.parse_args()
    run_server(
        args.port,
        Path(args.config),
        Path(args.routing_config_dir),
        Path(args.cert) if not args.http else None,
        Path(args.key) if not args.http else None,
        use_https=not args.http
    )
