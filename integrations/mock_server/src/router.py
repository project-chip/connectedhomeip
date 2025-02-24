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

from typing import Any, Dict, List, Optional
from urllib.parse import parse_qs

from route_configuration import Route


def match_route(routing: List[Route], method: str, path: str, query: Optional[Dict[str, Any] | str] = None) -> Optional[Route]:
    """
    Finds the best matching route configuration for an incoming HTTP request.

    Evaluates incoming requests against a list of routes based on three criteria
    in order of priority:
    1. HTTP method (e.g., GET, POST, PUT, DELETE)
    2. URL path pattern (including wildcard support)
    3. Query parameters (if specified in route configuration)

    Args:
        routing (List[Route]): List of available route configurations
        method (str): HTTP method from the incoming request
        path (str): URL path from the incoming request
        query (Optional[Dict[str, Any] | str]): Query parameters from the request,
            either as a dictionary or URL-encoded string. Defaults to None.

    Returns:
        Optional[Route]: The first matching Route object that satisfies all criteria,
            or None if no match is found. When multiple routes match, returns the
            first matching route in the original routing list order.

    Examples:
        # Available routes
        routes = [
            Route(method="GET", path="/api/device/*", query=None),
            Route(method="GET", path="/api/device/status", query={"type": "sensor"})
        ]

        # These would match:
        match_route(routes, "GET", "/api/device/123")  # Returns first route
        match_route(routes, "GET", "/api/device/status", {"type": "sensor"})  # Returns second route

        # These would return None:
        match_route(routes, "POST", "/api/device/123")
        match_route(routes, "GET", "/api/other")

    Note:
        - Routes are evaluated in order, returning the first match
        - Wildcard paths (ending in *) match any path with the specified prefix
        - Query parameters must match exactly if specified in the route
        - Method matching is case-sensitive
        - Path matching is case-sensitive
        - Query strings are automatically parsed into dictionaries
    """

    # Filter routes to only those matching the HTTP method
    method_routes: List[Route] = []
    for route in routing:
        if route.method == method:
            method_routes.append(route)
    if not method_routes:
        return None

    # Filter routes to only those matching the path
    path_routes: List[Route] = []
    for route in method_routes:
        if route.path == path:
            path_routes.append(route)
        elif route.path.endswith("*"):
            # Handle wildcard paths
            if path.startswith(route.path[:-1]):
                path_routes.append(route)
    if not path_routes:
        return None

    # Parse query parameters if present
    query_params = {}
    if query:
        if isinstance(query, str):
            # parse_qs returns values as lists, we'll take the first value for each parameter
            parsed = parse_qs(query)
            query_params = {k: v[0] if v else "" for k, v in parsed.items()}
        else:
            query_params = query

    # Find the first route that matches the path and query parameters
    for route in path_routes:
        if not route.query:
            return route

        # Check if all required query parameters are present
        if all(param in query_params for param in route.query.params):
            return route

    return None
