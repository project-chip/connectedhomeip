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
import json
import urllib.parse
from typing import List, Optional, Type

from route_configuration import Configuration, Route
from router import match_route


def createMockServerHandler(config: Configuration) -> Type[http.server.BaseHTTPRequestHandler]:
    """
    Creates a custom HTTP request handler class configured with predefined routes and responses.

    This factory function generates a new HTTP request handler class that processes incoming
    HTTP requests according to the provided routing configuration. The handler supports
    route matching based on HTTP method, path, and query parameters, returning predefined
    responses for matched routes.

    Args:
        config (Configuration): A Configuration object containing route definitions.
            Each route should specify:
            - HTTP method (GET, POST, PUT, DELETE)
            - URL path pattern
            - Response details (status code, headers, body)
            - Optional query parameters for matching

    Returns:
        Type[http.server.BaseHTTPRequestHandler]: A new handler class that:
            - Processes standard HTTP methods (GET, POST, PUT, DELETE)
            - Matches incoming requests against configured routes
            - Returns JSON or plain text responses based on configuration
            - Provides 404 responses for unmatched routes

    Example Configuration:
        {
            "routing": [
                {
                    "method": "GET",
                    "path": "/api/users",
                    "response": {
                        "status": 200,
                        "headers": {"Content-Type": "application/json"},
                        "body": {"users": []}
                    }
                }
            ]
        }

    Note:
        - The handler automatically sets appropriate Content-Type headers
        - JSON responses are automatically encoded to UTF-8
        - Non-JSON responses are converted to strings before encoding
        - All responses include standard HTTP headers and status codes
    """

    class MockServerHandler(http.server.BaseHTTPRequestHandler):
        def _set_headers(self, status_code=200, headers=None) -> None:
            self.send_response(status_code)
            if headers:
                for key, value in headers.items():
                    self.send_header(key, value)
            self.end_headers()

        def do_GET(self) -> None:
            self.handle_request()

        def do_POST(self) -> None:
            self.handle_request()

        def do_PUT(self) -> None:
            self.handle_request()

        def do_DELETE(self) -> None:
            self.handle_request()

        def handle_request(self) -> None:
            parsed_path: urllib.parse.ParseResult = urllib.parse.urlparse(self.path)
            path: str = parsed_path.path
            query_params: dict[str, list[str]] = urllib.parse.parse_qs(parsed_path.query)

            # Find the matching route from the configuration
            routes: List[Route] = config.routing
            route: Optional[Route] = match_route(routes, self.command, path, query_params)

            if not route:
                # No matching route found; return a 404 error response
                self._set_headers(404, {"Content-Type": "application/json"})
                self.wfile.write(json.dumps({}).encode("utf-8"))
                return

            # Use the static response defined in the configuration
            self._set_headers(route.response.status, route.response.headers)
            if route.response.headers.get("Content-Type") == "application/json":
                self.wfile.write(json.dumps(route.response.body).encode("utf-8"))
            else:
                self.wfile.write(str(route.response.body).encode("utf-8"))

    return MockServerHandler
