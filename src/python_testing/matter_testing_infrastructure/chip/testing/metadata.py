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

import logging
import re
from dataclasses import dataclass
from io import StringIO
from typing import Any, Dict, List, Optional

import yaml


# TODO #35787: Remove support for non-YAML format
def cast_to_bool(value: Any) -> bool:
    """Convert True/true/False/false strings to bool."""
    if isinstance(value, str):
        return value.strip().lower() == "true"
    return bool(value)


@dataclass
class Metadata:
    py_script_path: str
    run: str
    app: str = ""
    app_args: Optional[str] = None
    app_ready_pattern: Optional[str] = None
    script_args: Optional[str] = None
    factory_reset: bool = False
    factory_reset_app_only: bool = False
    script_gdb: bool = False
    quiet: bool = True


class NamedStringIO(StringIO):
    def __init__(self, content, name):
        super().__init__(content)
        self.name = name


def extract_runs_args(py_script_path: str) -> Dict[str, Dict[str, str]]:
    """Extract the run arguments from the CI test arguments blocks."""

    found_ci_args_section = False
    done_ci_args_section = False

    runs_def_ptrn = re.compile(r'^\s*#\s*test-runner-runs:\s*(?P<run_id>.*)$')
    arg_def_ptrn = re.compile(
        r'^\s*#\s*test-runner-run/(?P<run_id>[a-zA-Z0-9_]+)/(?P<arg_name>[a-zA-Z0-9_\-]+):\s*(?P<arg_val>.*)$')

    runs_arg_lines: Dict[str, Dict[str, str]] = {}

    ci_args_section_lines = []
    with open(py_script_path, 'r', encoding='utf8') as py_script:
        for line_idx, line in enumerate(py_script.readlines()):
            line = line.strip()
            line_num = line_idx + 1

            # Append empty line to the line capture, so during YAML parsing
            # line numbers will match the original file.
            ci_args_section_lines.append("")

            # Detect the single CI args section, to skip the lines otherwise.
            if not done_ci_args_section and line.startswith("# === BEGIN CI TEST ARGUMENTS ==="):
                found_ci_args_section = True
                continue
            elif found_ci_args_section and line.startswith("# === END CI TEST ARGUMENTS ==="):
                done_ci_args_section = True
                found_ci_args_section = False
                continue

            if found_ci_args_section:
                # Update the last line in the line capture.
                ci_args_section_lines[-1] = " " + line.lstrip("#")

            runs_match = runs_def_ptrn.match(line)
            args_match = arg_def_ptrn.match(line)

            if not found_ci_args_section and (runs_match or args_match):
                logging.warning(f"{py_script_path}:{line_num}: Found CI args outside of CI TEST ARGUMENTS block")
                continue

            if runs_match:
                for run in runs_match.group("run_id").strip().split():
                    runs_arg_lines[run] = {}
                    runs_arg_lines[run]['run'] = run

            elif args_match:
                runs_arg_lines[args_match.group("run_id")][args_match.group("arg_name")] = args_match.group("arg_val")

    if not runs_arg_lines:
        try:
            runs = yaml.safe_load(NamedStringIO("\n".join(ci_args_section_lines), py_script_path))
            for run, args in runs.get("test-runner-runs", {}).items():
                runs_arg_lines[run] = {}
                runs_arg_lines[run]['run'] = run
                runs_arg_lines[run].update(args)
        except yaml.YAMLError as e:
            logging.error(f"Failed to parse CI arguments YAML: {e}")

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
            if not isinstance(arg_val, str):
                continue
            # We do not expect to recurse (like ${FOO_${BAR}}) so just expand once
            for name, value in self.env.items():
                arg_val = arg_val.replace(f'${{{name}}}', value)
            metadata_dict[arg] = arg_val.strip()

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
        runs_args = extract_runs_args(py_script_path)

        for run, attr in runs_args.items():
            self.__resolve_env_vals__(attr)
            runs_metadata.append(Metadata(
                py_script_path=py_script_path,
                run=run,
                app=attr.get("app", ""),
                app_args=attr.get("app-args"),
                app_ready_pattern=attr.get("app-ready-pattern"),
                script_args=attr.get("script-args"),
                factory_reset=cast_to_bool(attr.get("factoryreset", False)),
                quiet=cast_to_bool(attr.get("quiet", True))
            ))

        return runs_metadata
