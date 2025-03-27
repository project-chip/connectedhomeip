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
import logging
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional


@dataclass
class RouteResponse:
    status: int
    headers: Dict[str, str]
    body: Dict[str, Any]


@dataclass
class QueryConfig:
    params: Dict[str, Any]


@dataclass
class Route:
    method: str
    path: str
    response: RouteResponse
    body: Optional[Any] = None
    query: Optional[QueryConfig] = None


@dataclass
class Configuration:
    routing: List[Route] = field(default_factory=list)


def load_configurations(config_path: Path, routing_config_dir: Path) -> Configuration:
    """
    Load and combine configuration files from specified paths.

    Args:
        config_path (Path): Path to the main configuration file.
        routing_config_dir (Path): Directory containing routing configuration files.

    Returns:
        Configuration: A Configuration object containing all merged routing configurations.

    Raises:
        ValueError: If config_path is not a file or routing_config_dir is not a directory.
    """
    if not config_path.is_file():
        raise ValueError(f"'{config_path}' is not a file")

    if not routing_config_dir.is_dir():
        raise ValueError(f"'{routing_config_dir}' is not a directory")

    # Load routing configuration
    routing_config: List[Route] = load_routing_configuration_dir(routing_config_dir)

    # Create and return the Configuration object
    return Configuration(routing=routing_config)


def load_routing_configuration_dir(directory: Path) -> List[Route]:
    """
    Load and merge all routing configuration files from a specified directory.

    Args:
        directory (Path): Directory path containing JSON configuration files.

    Returns:
        List[Route]: A list of Route objects containing all merged routing configurations.

    Raises:
        ValueError: If the specified directory path is not a directory.

    Note:
        - Processes all .json files in the specified directory
        - Continues processing even if one file fails to load, logging the error
    """
    if not directory.is_dir():
        raise ValueError(f"'{directory}' is not a directory")

    all_routes: List[Route] = []

    # Process all JSON files in the directory
    for file_path in directory.glob("*.json"):
        try:
            routes = load_routing_configuration_file(file_path)
            all_routes.extend(routes)
        except Exception as e:
            print(f"Error loading configuration from {file_path}: {str(e)}")
            continue

    return all_routes


def load_routing_configuration_file(file_path: Path) -> List[Route]:
    """
    Load and parse a single routing configuration JSON file.

    Args:
        file_path (Path): Path to the JSON configuration file.

    Returns:
        List[Route]: A list of Route objects parsed from the configuration file.

    Raises:
        FileNotFoundError: If the configuration file doesn't exist.
        json.JSONDecodeError: If the file contains invalid JSON.
        KeyError: If the required 'routes' key is missing in the configuration.

    Format:
        The JSON file should contain:
        {
            "routes": [
                {
                    "method": str,
                    "path": str,
                    "response": {
                        "status": int,
                        "headers": dict[str, str],
                        "body": dict[str, Any]
                    },
                    "body": Any,  # Optional
                    "query": dict[str, Any]  # Optional
                }
            ]
        }
    """
    try:
        with open(file_path, "r") as file:
            config = json.load(file)
        if "routes" not in config:
            logging.error("Missing required 'routes' key in configuration file: %s", file_path)
            raise KeyError("Configuration file must contain 'routes' key")

        logging.debug("Routes configuration loaded successfully from %s", file_path)
        routes = []
        for route_config in config["routes"]:
            # Create RouteResponse object
            response = RouteResponse(
                status=route_config["response"]["status"],
                headers=route_config["response"].get("headers", {}),
                body=route_config["response"].get("body", {}),
            )

            # Create QueryConfig if query params exist
            query = None
            if "query" in route_config:
                query = QueryConfig(params=route_config["query"])

            # Create Route object
            route = Route(
                method=route_config["method"],
                path=route_config["path"],
                response=response,
                body=route_config.get("body"),
                query=query,
            )
            routes.append(route)

        return routes

    except FileNotFoundError:
        logging.error("Configuration file not found: %s", file_path)
        raise
    except json.JSONDecodeError as e:
        logging.error("Invalid JSON in configuration file: %s", file_path)
        raise e
