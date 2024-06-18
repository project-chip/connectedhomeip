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

import os
import tempfile
import unittest
from typing import List

from metadata import Metadata, MetadataReader


class TestMetadataReader(unittest.TestCase):

    test_file_content = ''' 
    # test-runner-runs: run1 
    # test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
    # test-runner-run/run1/app-args: --discriminator 1234 --trace-to json:${TRACE_APP}.json
    # test-runner-run/run1/script-args: --commissioning-method on-network --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
    # test-runner-run/run1/factoryreset: True
    # test-runner-run/run1/quiet: True
    '''

    env_file_content = '''
    ALL_CLUSTERS_APP: out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app
    CHIP_LOCK_APP: out/linux-x64-lock-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-lock-app
    ENERGY_MANAGEMENT_APP: out/linux-x64-energy-management-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-energy-management-app
    TRACE_APP: out/trace_data/app-{SCRIPT_BASE_NAME}
    TRACE_TEST_JSON: out/trace_data/test-{SCRIPT_BASE_NAME}
    TRACE_TEST_PERFETTO: out/trace_data/test-{SCRIPT_BASE_NAME}
    '''

    expected_metadata = Metadata(
        script_args="--commissioning-method on-network --trace-to json:out/trace_data/test-{SCRIPT_BASE_NAME}.json --trace-to perfetto:out/trace_data/test-{SCRIPT_BASE_NAME}.perfetto",
        py_script_path="",
        app_args="--discriminator 1234 --trace-to json:out/trace_data/app-{SCRIPT_BASE_NAME}.json",
        run="run1",
        app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
        factoryreset=True,
        quiet=True
    )

    def generate_temp_file(self, directory: str, file_content: str) -> str:
        fd, temp_file_path = tempfile.mkstemp(dir=directory)
        with os.fdopen(fd, 'w') as fp:
            fp.write(file_content)
        return temp_file_path

    def test_run_arg_generation(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_file = self.generate_temp_file(temp_dir, self.test_file_content)
            env_file = self.generate_temp_file(temp_dir, self.env_file_content)

            reader = MetadataReader(env_file)
            self.maxDiff = None

            self.expected_metadata.py_script_path = temp_file
            actual = reader.parse_script(temp_file)[0]
            self.assertEqual(self.expected_metadata, actual)


if __name__ == "__main__":
    unittest.main()
