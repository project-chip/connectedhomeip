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

"""
Generates a gni file containing all data_model files (generally XML and JSON files) 
that are typically used by matter_testing_infrastructure.

These files are to be bundled with whl packages of the matter_testing_infrastructure
so that methods requiring data model files work just by installing the python
package without requiring a full chip SDK checkout.
"""
import os

import jinja2

# Set chip_root to be dynamically based on the script's location
chip_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../.."))

# Directories to search for .xml and .json files relative to chip_root
directories = [
    os.path.join(chip_root, "data_model/1.3/clusters/"),
    os.path.join(chip_root, "data_model/1.3/device_types/"),
    os.path.join(chip_root, "data_model/1.4/clusters/"),
    os.path.join(chip_root, "data_model/1.4/device_types/"),
    os.path.join(chip_root, "data_model/master/clusters/"),
    os.path.join(chip_root, "data_model/master/device_types/"),
]

# Template for generating the GNI file content with proper indentation
GNI_TEMPLATE = """\
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

import("//build_overrides/chip.gni")

data_model_XMLS = [
{% for name in file_list %}
    "{{ name }}",
{% endfor %}
]
"""

# Function to find and collect all .xml and .json files


def get_data_model_file_names():
    file_list = []
    for directory in directories:
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith(".xml") or file.endswith(".json"):
                    # Replace absolute path with `${chip_root}` for GNI compatibility
                    relative_path = os.path.join("${chip_root}", os.path.relpath(root, chip_root), file)
                    file_list.append(relative_path)
    # Sort files alphabetically
    file_list.sort()
    return file_list

# Main function to generate the data_model_xmls.gni file


def generate_gni_file():
    # Step 1: Find all files and create the sorted file list
    file_list = get_data_model_file_names()

    # Step 2: Render the template with the file list
    environment = jinja2.Environment(trim_blocks=True, lstrip_blocks=True)
    template = environment.from_string(GNI_TEMPLATE)
    output_content = template.render(file_list=file_list)

    # Step 3: Dynamically generate the output file path
    # Get the script's directory (where this script is located)
    script_dir = os.path.dirname(os.path.realpath(__file__))  # Directory of the current script

    # Step 4: Ensure we are in the correct `src/python_testing/` directory
    base_dir = os.path.abspath(os.path.join(script_dir, "../.."))  # Go up two levels to src/python_testing/
    # Now append `matter_testing_infrastructure`
    output_dir = os.path.join(base_dir, "python_testing", "matter_testing_infrastructure")
    output_file = os.path.join(output_dir, "data_model_xmls.gni")

    # Step 5: Write the rendered content to the output file
    os.makedirs(output_dir, exist_ok=True)  # Ensure the output directory exists
    with open(output_file, "wt") as f:
        f.write(output_content)
    print(f"{output_file} has been generated successfully.")


# Run the function to generate the .gni file
if __name__ == "__main__":
    generate_gni_file()
