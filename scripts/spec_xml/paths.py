#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os


def get_chip_root():
    """
    Returns the CHIP root directory, trying the environment variable first
    and falling back if necessary.
    """
    chip_root = os.getenv('PW_PROJECT_ROOT')
    if chip_root:
        return chip_root
    try:
        return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
    except Exception as e:
        raise EnvironmentError(
            "Unable to determine CHIP root directory. Please ensure the environment is activated."
        ) from e


def get_data_model_path():
    return os.path.join(get_chip_root(), 'data_model')


def get_spec_xml_output_path():
    """
    Returns the path to the output directory for generated XML files.
    """
    chip_root = get_chip_root()
    output_dir = os.path.join(chip_root, 'out', 'spec_xml')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)  # Automatically create the directory if it doesn't exist
    return output_dir


def get_cluster_documentation_file_path():
    """
    Returns the path to the documentation file.
    """
    chip_root = get_chip_root()
    return os.path.join(chip_root, 'docs', 'ids_and_codes', 'spec_clusters.md')


def get_device_types_documentation_file_path():
    """
    Returns the path to the documentation file.
    """
    chip_root = get_chip_root()
    return os.path.join(chip_root, 'docs', 'ids_and_codes', 'spec_device_types.md')


def get_python_testing_path():
    """
    Returns the path to the python_testing directory.
    """
    chip_root = get_chip_root()
    python_testing_path = os.path.join(chip_root, 'src', 'python_testing')
    if not os.path.exists(python_testing_path):
        raise FileNotFoundError(f"Python testing directory does not exist: {python_testing_path}")
    return python_testing_path
