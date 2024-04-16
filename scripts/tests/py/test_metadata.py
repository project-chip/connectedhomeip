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
from os import path
import tempfile
from typing import List

from metadata import Metadata, MetadataReader


class TestMetadataReader(unittest.TestCase):

    def setUp(self):
        # build the reader object
        self.reader = MetadataReader(path.join(path.dirname(__file__), "env_test.yaml"))

    def assertMetadataParse(self, file_content: str, expected: List[Metadata]):
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as fp:
            fp.write(file_content)
            fp.close()
            for e in expected:
                e.py_script_path = fp.name
            actual = self.reader.parse_script(fp.name)
            self.assertEqual(actual, expected)

    def test_parse_single_run(self):
        self.assertMetadataParse(''' 
            # test-runner-runs: run1 
            # test-runner-run/run1: app/all-clusters discriminator passcode
            ''',
                                 [
                                     Metadata(app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
                                              discriminator=1234, run="run1", passcode=20202021)
                                 ]
                                 )


if __name__ == "__main__":
    unittest.main()
