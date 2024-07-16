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

import os
import glob
import subprocess

# Define the base command
base_command = "scripts/run_in_python_env.sh out/venv './scripts/tests/run_python_test.py --load-from-env /tmp/test_env.yaml --script"

# Define the directory to search for Python scripts
search_directory = "src/python_testing"

# Define the files and patterns to exclude
excluded_patterns = [
    "MinimalRepresentation.py", 
    "TC_ACL_2_2.py", 
    "TC_BOOLCFG*.py",  # Use wildcard pattern to exclude matching files
    "TC_CNET_4_4.py", 
    "TC_DGGEN_3_2.py", 
    "TC_EEVSE_Utils.py", 
    "TC_EnergyReporting_Utils.py", 
    "TC_OpstateCommon.py", 
    "TC_TMP_2_1.py",
    "TC_VALCC*.py", # Use wildcard pattern to exclude matching files
    "TC_pics_checker.py",
    "TestCommissioningTimeSync.py",
    "TestConformanceSupport.py",
    "TestIdChecks.py",
    "TestMatterTestingSupport.py",
    "TestSpecParsingSupport.py",
    "TestTimeSyncTrustedTimeSource.py",
    "basic_composition_support.py",
    "conformance_support.py",
    "drlk_2_x_common.py",
    "global_attribute_ids.py",
    "hello_external_runner.py",
    "hello_test.py",
    "matter_testing_support.py",
    "pics_support.py",
    "spec_parsing_support.py",
    "taglist_and_topology_test_support.py",
    "test_plan_support.py",
    "test_plan_table_generator.py"
]

# Get all .py files in the directory
all_python_files = glob.glob(os.path.join(search_directory, "*.py"))

# Filter out the files matching the excluded patterns
python_files = []
for file in all_python_files:
    exclude = False
    for pattern in excluded_patterns:
        if glob.fnmatch.fnmatch(os.path.basename(file), pattern):
            exclude = True
            break
    if not exclude:
        python_files.append(file)

# Run each script with the base command
for script in python_files:
    full_command = f"{base_command} {script}'"
    print(f"Running command: {full_command}")
    subprocess.run(full_command, shell=True)
