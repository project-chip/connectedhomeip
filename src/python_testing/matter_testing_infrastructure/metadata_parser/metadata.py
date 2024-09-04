#!/usr/bin/python3
# Copyright (c) 2024 Project CHIP Authors
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

import re
import sys
from dataclasses import dataclass
from typing import Any, Dict, List

import yaml


def bool_from_str(value: str) -> bool:
    """Convert True/true/False/false strings to bool."""
    return value.strip().lower() == "true"


@dataclass
class Metadata:
    py_script_path: str
    run: str
    app: str
    app_args: str
    script_args: str
    script_start_delay: int = 0
    factoryreset: bool = False
    factoryreset_app_only: bool = False
    script_gdb: bool = False
    quiet: bool = True

    def copy_from_dict(self, attr_dict: Dict[str, Any]) -> None:
        """
        Sets the value of the attributes from a dictionary.

        Attributes:

        attr_dict:
         Dictionary that stores attributes value that should
         be transferred to this class.
        """
        if "app" in attr_dict:
            self.app = attr_dict["app"]

        if "run" in attr_dict:
            self.run = attr_dict["run"]

        if "app-args" in attr_dict:
            self.app_args = attr_dict["app-args"]

        if "script-args" in attr_dict:
            self.script_args = attr_dict["script-args"]

        if "script-start-delay" in attr_dict:
            self.script_start_delay = int(attr_dict["script-start-delay"])

        if "py_script_path" in attr_dict:
            self.py_script_path = attr_dict["py_script_path"]

        if "factoryreset" in attr_dict:
            self.factoryreset = bool_from_str(attr_dict["factoryreset"])

        if "factoryreset_app_only" in attr_dict:
            self.factoryreset_app_only = bool_from_str(attr_dict["factoryreset_app_only"])

        if "script_gdb" in attr_dict:
            self.script_gdb = bool_from_str(attr_dict["script_gdb"])

        if "quiet" in attr_dict:
            self.quiet = bool_from_str(attr_dict["quiet"])


def extract_runs_arg_lines(py_script_path: str) -> Dict[str, Dict[str, str]]:
    """Extract the run arguments from the CI test arguments blocks."""

    found_ci_args_section = False
    done_ci_args_section = False

    runs_def_ptrn = re.compile(r'^\s*#\s*test-runner-runs:\s*(?P<run_id>.*)$')
    arg_def_ptrn = re.compile(
        r'^\s*#\s*test-runner-run/(?P<run_id>[a-zA-Z0-9_]+)/(?P<arg_name>[a-zA-Z0-9_\-]+):\s*(?P<arg_val>.*)$')

    runs_arg_lines: Dict[str, Dict[str, str]] = {}

    with open(py_script_path, 'r', encoding='utf8') as py_script:
        for line_idx, line in enumerate(py_script.readlines()):
            line = line.strip()
            line_num = line_idx + 1

            # Detect the single CI args section, to skip the lines otherwise.
            if not done_ci_args_section and line.startswith("# === BEGIN CI TEST ARGUMENTS ==="):
                found_ci_args_section = True
            elif found_ci_args_section and line.startswith("# === END CI TEST ARGUMENTS ==="):
                done_ci_args_section = True
                found_ci_args_section = False

            runs_match = runs_def_ptrn.match(line)
            args_match = arg_def_ptrn.match(line)

            if not found_ci_args_section and (runs_match or args_match):
                print(f"WARNING: {py_script_path}:{line_num}: Found CI args outside of CI TEST ARGUMENTS block!", file=sys.stderr)
                continue

            if runs_match:
                for run in runs_match.group("run_id").strip().split():
                    runs_arg_lines[run] = {}
                    runs_arg_lines[run]['run'] = run
                    runs_arg_lines[run]['py_script_path'] = py_script_path

            elif args_match:
                runs_arg_lines[args_match.group("run_id")][args_match.group("arg_name")] = args_match.group("arg_val")

    return runs_arg_lines


class MetadataReader:
    """
    A class to parse run arguments from the test scripts and
    resolve them to environment specific values.
    """

    def __init__(self, env_yaml_file_path: str):
        """
        Reads the YAML file and Constructs the environment object

        Parameters:

        env_yaml_file_path:
         Path to the environment file that contains the YAML configuration.
        """
        with open(env_yaml_file_path) as stream:
            self.env: Dict[str, str] = yaml.safe_load(stream)

    def __resolve_env_vals__(self, metadata_dict: Dict[str, str]) -> None:
        """
        Resolves the argument defined in the test script to environment values.
        For example, if a test script defines "all_clusters" as the value for app
        name, we will check the environment configuration to see what raw value is
        associated with the "all_cluster" variable and set the value for "app" option
        to this raw value.

        Parameter:

        metadata_dict:
         Dictionary where each key represent a particular argument and its value represent
         the value for that argument defined in the test script.
        """
        for arg, arg_val in metadata_dict.items():
            # We do not expect to recurse (like ${FOO_${BAR}}) so just expand once
            for name, value in self.env.items():
                arg_val = arg_val.replace(f'${{{name}}}', value)
            metadata_dict[arg] = arg_val

    def parse_script(self, py_script_path: str) -> List[Metadata]:
        """
        Parses a script and returns a list of metadata object where
        each element of that list representing run arguments associated
        with a particular run.

        Parameter:

        py_script_path:
         path to the python test script

        Return:

        List[Metadata]
         List of Metadata object where each Metadata element represents
         the run arguments associated with a particular run defined in
         the script file.
        """
        runs_metadata: List[Metadata] = []
        runs_arg_lines = extract_runs_arg_lines(py_script_path)

        for run, attr in runs_arg_lines.items():
            self.__resolve_env_vals__(attr)

            metadata = Metadata(
                py_script_path=attr.get("py_script_path", ""),
                run=run,
                app=attr.get("app", ""),
                app_args=attr.get("app_args", ""),
                script_args=attr.get("script_args", ""),
                script_start_delay=int(attr.get("script_start_delay", 0)),
                factoryreset=bool(attr.get("factoryreset", False)),
                factoryreset_app_only=bool(attr.get("factoryreset_app_only", False)),
                script_gdb=bool(attr.get("script_gdb", False)),
                quiet=bool(attr.get("quiet", True))
            )
            metadata.copy_from_dict(attr)
            runs_metadata.append(metadata)

        return runs_metadata
