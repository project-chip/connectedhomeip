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

import unittest
from metadata import Metadata
from metadata import Metadata_Reader
from os import path


class TestMetadataReader(unittest.TestCase):

    def setUp(self):

        # build the reader object
        self.reader=Metadata_Reader(path.join(path.dirname(__file__),"env_test.yaml"))
            

    def test_parse_single_run(self):

        expected_runs_metadata = {}
        path_under_test=path.join(path.dirname(__file__),"simple_run_args.txt")
        
        expected_runs_metadata[path_under_test] = Metadata(app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
                   discriminator=1234, py_script_path=path_under_test, run="run1", passcode=20202021)
        
                
        for run in expected_runs_metadata:
            self.assertEqual(self.reader.parse_script(run)[0], expected_runs_metadata[run])
    
    
if __name__=="__main__":
    unittest.main()
    
