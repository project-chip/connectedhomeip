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

import os
import unittest
from os import path

from metadata import Metadata, MetadataReader


class TestMetadataReader(unittest.TestCase):
    path_under_test = path_under_test = path.join(path.dirname(__file__), "simple_run_args.txt")

    def setUp(self):

        # build the reader object
        self.reader = MetadataReader(path.join(path.dirname(__file__), "env_test.yaml"))
        with open(self.path_under_test, 'w', encoding='utf8') as test_file:
            test_file.writelines(''' 
            # test-runner-runs: run1 
            # test-runner-run/run1: app/all-clusters discriminator KVS storage-path commissioning-method discriminator passcode
            ''')

    def test_parse_single_run(self):

        expected_runs_metadata = []

        expected_runs_metadata.append(Metadata(app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
                                               discriminator=1234, py_script_path=self.path_under_test, run="run1", passcode=20202021))

        self.assertEqual(self.reader.parse_script(self.path_under_test), expected_runs_metadata)

    def tearDown(self):
        if os.path.exists(self.path_under_test):
            os.remove(self.path_under_test)


if __name__ == "__main__":
    unittest.main()