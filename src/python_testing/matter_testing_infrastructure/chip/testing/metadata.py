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
from dataclasses import dataclass
from io import StringIO
from typing import Dict, List, Optional

import yaml


@dataclass
class Metadata:
    py_script_path: str
    run: str
    app: str = ""
    app_args: Optional[str] = None
    app_ready_pattern: Optional[str] = None
    app_stdin_pipe: Optional[str] = None
    script_args: Optional[str] = None
    factory_reset: bool = False
    factory_reset_app_only: bool = False
    script_gdb: bool = False
    quiet: bool = False


class NamedStringIO(StringIO):
    def __init__(self, content, name):
        super().__init__(content)
        self.name = name


def extract_runs_args(py_script_path: str) -> Dict[str, Dict[str, str]]:
    """Extract the run arguments from the CI test arguments blocks."""

    found_ci_args_section = False
    runs_arg_lines: Dict[str, Dict[str, str]] = {}

    ci_args_section_lines = []

    with open(py_script_path, 'r', encoding='utf8') as py_script:
        for line in py_script.readlines():
            line = line.strip()
            ci_args_section_lines.append("")  # Preserve line numbers for YAML parsing

            if line.startswith("# === BEGIN CI TEST ARGUMENTS ==="):
                found_ci_args_section = True
                continue
            if line.startswith("# === END CI TEST ARGUMENTS ==="):
                break

            if found_ci_args_section:
                ci_args_section_lines[-1] = " " + line.lstrip("#")

    if ci_args_section_lines:
        try:
            runs = yaml.safe_load(NamedStringIO("\n".join(ci_args_section_lines), py_script_path))
            for run, args in runs.get("test-runner-runs", {}).items():
                runs_arg_lines[run] = args

            # Capture skip-default-flags (if defined)
            skip_flags = runs.get("skip-default-flags", [])
            for run in runs_arg_lines:
                runs_arg_lines[run]["skip-default-flags"] = skip_flags

        except yaml.YAMLError as e:
            logging.error(f"Failed to parse CI arguments YAML: {e}")

    return runs_arg_lines


class MetadataReader:
    """
    Parses test script arguments and merges them with defaults from env_test.yaml.
    """

    def __init__(self, env_yaml_file_path: str):
        """Loads default arguments from env_test.yaml."""
        with open(env_yaml_file_path, 'r', encoding='utf8') as stream:
            env_yaml = yaml.safe_load(stream) or {}

        self.default_args = env_yaml.get("default-arguments", {})

    def __resolve_env_vals__(self, metadata_dict: Dict[str, str]) -> None:
        """Resolves ${VAR} placeholders using default arguments."""
        for arg, arg_val in metadata_dict.items():
            if isinstance(arg_val, str):
                for name, value in self.default_args.items():
                    arg_val = arg_val.replace(f'${{{name}}}', value)
                metadata_dict[arg] = arg_val.strip()

    def parse_script(self, py_script_path: str) -> List[Metadata]:
        """
        Parses a test script and merges run arguments with defaults.
        
        - Uses defaults from env_test.yaml.
        - Applies script overrides.
        - Respects skip-default-flags.

        Returns:
            List[Metadata]: List of parsed metadata objects.
        """
        runs_metadata: List[Metadata] = []
        script_args = extract_runs_args(py_script_path)

        for run, script_run_args in script_args.items():
            # Extract skip-default-flags (if present)
            skip_default_flags = script_run_args.pop("skip-default-flags", [])

            # Start with defaults
            combined_args = {k: v.copy() if isinstance(v, dict) else v for k, v in self.default_args.items()}

            # Apply script args, respecting skip-default-flags
            for key, value in script_run_args.items():
                if key in skip_default_flags:
                    combined_args[key] = value  # Take from script YAML block only
                else:
                    combined_args.setdefault(key, value)  # Keep default if not overridden

            self.__resolve_env_vals__(combined_args)  # Resolve ${VAR} placeholders

        runs_metadata.append(Metadata(
            py_script_path=py_script_path,
            run=run,
            app=combined_args.get("app", ""),
            app_args=combined_args.get("app-args"),
            app_ready_pattern=combined_args.get("app-ready-pattern"),
            app_stdin_pipe=combined_args.get("app-stdin-pipe"),
            script_args=combined_args.get("script-args"),
            factory_reset=combined_args.get("factory-reset", False),
            quiet=combined_args.get("quiet", True),
        ))

        return runs_metadata
