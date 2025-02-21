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

import json
import shutil
import tempfile
import unittest
from pathlib import Path
from typing import List

from route_configuration import QueryConfig, Route, RouteResponse, load_configurations
from router import match_route


class TestConfigLoader(unittest.TestCase):
    def setUp(self):
        # Create a temporary directory for our test files
        self.test_dir = tempfile.mkdtemp()
        self.config_dir = Path(self.test_dir) / "routes"
        self.config_dir.mkdir()

        # Create an empty config.json
        self.empty_config = Path(self.test_dir) / "config.json"
        self.empty_config.write_text("{}")

        # Create two route configuration files
        simple_route = {
            "routes": [
                {
                    "method": "GET",
                    "path": "/test",
                    "response": {
                        "status": 200,
                        "headers": {"Content-Type": "application/json"},
                        "body": {"message": "ok"},
                    },
                }
            ]
        }

        route_with_query = {
            "routes": [
                {
                    "method": "GET",
                    "path": "/api/data",
                    "query": {},
                    "response": {
                        "status": 200,
                        "headers": {"Content-Type": "application/json"},
                        "body": {"version": "1.0", "data": "sample"},
                    },
                }
            ]
        }

        # Write the route configurations to files
        (self.config_dir / "simple_route.json").write_text(json.dumps(simple_route))
        (self.config_dir / "data_route.json").write_text(json.dumps(route_with_query))

    def tearDown(self):
        # Clean up temporary files and directory
        shutil.rmtree(self.test_dir)

    def test_load_valid_config(self):
        # Test loading empty config file
        loaded_config = load_configurations(self.empty_config, self.config_dir)

        # Verify we loaded both routes
        self.assertEqual(len(loaded_config.routing), 2)

        # Verify simple route
        simple_route = match_route(loaded_config.routing, "GET", "/test", None)
        self.assertIsNotNone(simple_route)
        self.assertIsNotNone(simple_route.response)  # type: ignore
        self.assertEqual(simple_route.response.status, 200)  # type: ignore
        self.assertEqual(simple_route.response.body["message"], "ok")  # type: ignore

        # Verify route with query parameters
        query_route = match_route(loaded_config.routing, "GET", "/api/data", "{}")
        self.assertIsNotNone(query_route)
        self.assertEqual(query_route.response.status, 200)  # type: ignore
        self.assertEqual(query_route.response.body["version"], "1.0")  # type: ignore


class TestRouter(unittest.TestCase):
    routes: List[Route] = []

    def setUp(self):
        self.routes = [
            Route(
                method="GET",
                path="/api/data",
                response=RouteResponse(status=200, headers={}, body={"message": "Hello, World!"}),
            ),
            Route(
                method="GET",
                path="/api/query",
                query=QueryConfig(params={"key": "value"}),
                response=RouteResponse(status=200, headers={}, body={"message": "Query matched"}),
            ),
            Route(
                method="GET",
                path="/api/*",
                response=RouteResponse(status=200, headers={}, body={"message": "Wildcard matched"}),
            ),
            Route(
                method="POST",
                path="/api/echo",
                body={"regex": ".*hello.*"},
                response=RouteResponse(status=200, headers={}, body={"message": "Echo"}),
            ),
        ]

    def test_exact_route_match(self):
        route = match_route(self.routes, "GET", "/api/data", {})
        self.assertIsNotNone(route)
        self.assertEqual(route.response.body, {"message": "Hello, World!"})  # type: ignore

    def test_wildcard_route_match(self):
        route = match_route(self.routes, "GET", "/api/anything", {})
        self.assertIsNotNone(route)
        self.assertEqual(route.response.body, {"message": "Wildcard matched"})  # type: ignore

    def test_query_parameter_matching(self):
        route = match_route(self.routes, "GET", "/api/query", {"key": ["value"]})
        self.assertIsNotNone(route)
        self.assertEqual(route.response.body, {"message": "Query matched"})  # type: ignore

    def test_body_regex_match(self):
        route = match_route(self.routes, "POST", "/api/echo", "This is a hello message")
        self.assertIsNotNone(route)
        self.assertEqual(route.response.body, {"message": "Echo"})  # type: ignore

    def test_no_match(self):
        route = match_route(self.routes, "DELETE", "/nonexistent", {})
        self.assertIsNone(route)


if __name__ == "__main__":
    unittest.main()
