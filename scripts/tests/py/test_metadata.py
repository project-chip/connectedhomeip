# Copyright (c) 2023 Project CHIP Authors
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

import unittest
from metadata import Metadata
from metadata import Metadata_Reader


class TestMetadataReader(unittest.TestCase):
    # build the test Metadata_Reader object
    #defines expected arguments associated with runs
    def setUp(self):

        # build the reader object
        self.reader=Metadata_Reader("/scripts/tests/py", "/scripts/tests/py/env_test.yaml")
        self.actual_runs_metadata = self.reader.get_runs_metadata()
        self.expected_runs_metadata = {}

        self.expected_runs_metadata["/scripts/tests/py/simple_run_args.txt+run1"] = Metadata(app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
                   discriminator=1234, py_script_path="/scripts/tests/py/simple_run_args.txt", run="run1", passcode=20202021)
        
    # gets the metadata associated with each run by parsing the test script and compares it to the expected metadata values
    def test_run_metadata_parser(self):
        for actual_run in self.actual_runs_metadata:
            self.assertEqual(self.actual_runs_metadata[actual_run], self.expected_runs_metadata[actual_run])

    
if __name__=="__main__":
    unittest.main()
            
