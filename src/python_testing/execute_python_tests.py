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

import argparse
import glob
import os
import subprocess

import yaml

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
    for key, value in yaml.full_load(open(file_path, 'r')).items():
        os.environ[key] = value


def main(search_directory, env_file):
    # Determine the root directory of the CHIP project
    chip_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

    # Load environment variables from the specified file
    load_env_from_yaml(env_file)

    # Define the base command to run tests
    base_command = os.path.join(chip_root, "scripts/tests/run_python_test.py")

    metadata = yaml.full_load(open(os.path.join(chip_root, "src/python_testing/test_metadata.yaml")))
    excluded_patterns = set([item["name"] for item in metadata["not_automated"]])

    # Get all .py files in the directory
    all_python_files = glob.glob(os.path.join(search_directory, "*.py"))

    # Filter out the files matching the excluded patterns
    python_files = [file for file in all_python_files if os.path.basename(file) not in excluded_patterns]

    # Run each script with the base command
    for script in python_files:
        full_command = f"{base_command} --load-from-env {env_file} --script {script}"
        print(f"Running command: {full_command}", flush=True)  # Flush print to stdout immediately
        subprocess.run(full_command, shell=True, check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run Python test scripts.")
    parser.add_argument("--search-directory", type=str, default="src/python_testing",
                        help="Directory to search for Python scripts.")
    parser.add_argument("--env-file", type=str, default="/tmp/test_env.yaml", help="Path to the environment variables file.")

    args = parser.parse_args()
    main(args.search_directory, args.env_file)
