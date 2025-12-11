#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import fnmatch
import glob as g
import logging
import os
import re
import subprocess
import sys

import click
import coloredlogs
import yaml

log = logging.getLogger(__name__)

# Function to load --app argument environment variables from a file


def load_env_from_yaml(file_path):
    """
    Load environment variables from the specified YAML file.

    The YAML file contains key-value pairs that define --app environment variables
    required for the test scripts to run. These variables configurations needed during the test execution.

    This function reads the YAML file and sets the environment variables
    in the current process's environment using os.environ.

    Args:
        file_path (str): The path to the YAML file containing the environment variables.
    """
    with open(file_path) as f:
        for key, value in yaml.full_load(f).items():
            os.environ[key] = value


@click.command()
@click.option(
    "--search-directory",
    type=str,
    default="src/python_testing",
    help="Directory to search for Python scripts.",
)
@click.option(
    "--env-file",
    type=str,
    default="/tmp/test_env.yaml",
    help="Path to the environment variables file.",
)
@click.option(
    "--keep-going",
    is_flag=True,
    help="Run ALL the test, report a final status of what passed/failed.",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="If true, just print out what will be run but do not execute.",
)
@click.option(
    "--glob",
    multiple=True,
    help="Glob the tests to pick. Use `!` to negate the glob. Glob FILTERS out non-matching (i.e. you can use it to restrict more and more, but not to add)",
)
@click.option(
    "--regex",
    multiple=True,
    help="Regex the tests to pick. Use `!` to negate the expression. Expressions FILTERS out non-matching (i.e. you can use it to restrict more and more, but not to add)",
)
def main(search_directory, env_file, keep_going, dry_run: bool, glob: list[str], regex: list[str]):
    # Determine the root directory of the CHIP project
    chip_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

    # Load environment variables from the specified file
    load_env_from_yaml(env_file)

    # Define the base command to run tests
    base_command = os.path.join(chip_root, "scripts/tests/run_python_test.py")

    with open(os.path.join(chip_root, "src/python_testing/test_metadata.yaml")) as f:
        metadata = yaml.full_load(f)
    excluded_patterns = {item["name"] for item in metadata["not_automated"]}

    # Get all .py files in the directory
    all_python_files = g.glob(os.path.join(search_directory, "*.py"))

    for pattern in glob:
        if pattern.startswith('!'):
            def match(p): return not fnmatch.fnmatch(p, f"*{pattern[1:]}*")
        else:
            def match(p): return fnmatch.fnmatch(p, f"*{pattern}*")
        all_python_files = [path for path in all_python_files if match(path)]

    for pattern in regex:
        if pattern.startswith('!'):
            r = re.compile(pattern[1:])
            def match(p): return not r.search(p)
        else:
            r = re.compile(pattern)
            def match(p): return r.search(p) is not None
        all_python_files = [path for path in all_python_files if match(path)]

    # Filter out the files matching the excluded patterns
    python_files = [file for file in all_python_files if os.path.basename(file) not in excluded_patterns]

    # Run each script with the base command
    failed_scripts = []
    for script in python_files:
        try:
            full_command = f"{base_command} --load-from-env {env_file} --script {script}"
            if dry_run:
                print(f"DRY-RUN(skip): {full_command}", flush=True)  # Flush print to stdout immediately
            else:
                print(f"Running command: {full_command}", flush=True)  # Flush print to stdout immediately
                subprocess.run(full_command, shell=True, check=True)
        except Exception:
            if keep_going:
                failed_scripts.append(script)
            else:
                raise

    if failed_scripts:
        log.error("FAILURES detected:")
        for s in failed_scripts:
            log.error("   - %s", s)
        sys.exit(1)


if __name__ == "__main__":
    coloredlogs.install(level="INFO")
    main(auto_envvar_prefix="CHIP")
