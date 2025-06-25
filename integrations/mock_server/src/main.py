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

This module serves as the entry point for a configurable mock HTTPS server designed
for API testing. It provides a secure, flexible way to simulate API endpoints with
configurable responses.

The server supports:
- Custom routing configurations via JSON files
- HTTPS with TLS certificate support
- Configurable port binding
- Multiple concurrent connections
- Dynamic response configuration

Usage:
    python main.py [--port PORT] [--config CONFIG_FILE]
                   [--routing-config-dir ROUTE_DIR]
                   [--cert CERT_FILE] [--key KEY_FILE]

Arguments:
    --port PORT                 Port number to listen on (default: 8443)
    --config CONFIG_FILE        Path to main configuration file (default: config.json)
    --routing-config-dir DIR    Directory containing route configurations
                               (default: config.json)
    --cert CERT_FILE           Path to SSL certificate file (default: server.crt)
    --key KEY_FILE             Path to SSL private key file (default: server.key)

Example:
    python main.py --port 8443 --config ./config/main.json
                  --routing-config-dir ./config/routes
                  --cert ./certs/server.crt --key ./certs/server.key

Configuration:
    The server requires two types of configuration:
    1. Main configuration file (--config):
       Contains server-wide settings and default behaviors

    2. Route configuration directory (--routing-config-dir):
       Contains JSON files defining endpoint behaviors, responses,
       and matching criteria

Security:
    - HTTPS only, no HTTP support
    - Requires valid SSL certificate and private key
    - TLS configuration uses server's default security settings

Notes:
    - Server runs until interrupted (Ctrl+C)
    - All endpoints return JSON by default
    - Logs to stdout with DEBUG level
    - Supports concurrent requests via threading
"""


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Mock HTTP/HTTPS Server for API testing")
    parser.add_argument("--port", type=int, default=8443, help="Set the server port")
    parser.add_argument("--config", type=str, default="config.json", help="Path to the common config file")
    parser.add_argument("--routing-config-dir", type=str, default="config.json", help="Path to the common config file")
    parser.add_argument("--cert", type=str, default="server.crt", help="SSL Certificate file")
    parser.add_argument("--key", type=str, default="server.key", help="SSL Private Key file")

    args = parser.parse_args()
    run_server(args.port, Path(args.config), Path(args.routing_config_dir), Path(args.cert), Path(args.key))
