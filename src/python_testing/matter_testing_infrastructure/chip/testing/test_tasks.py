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

from tasks import Subprocess


class TestSubprocess(unittest.TestCase):

    def test_expected_output(self):
        p = Subprocess("python3", "-c", "print('Hello, World!')")
        p.start(expected_output="Hello, World!", timeout=1)
        p.terminate()

    def test_expected_output_timeout(self):
        p = Subprocess("python3", "--version")
        with self.assertRaises(TimeoutError):
            p.start(expected_output="Python 1.0.0", timeout=1)
        p.terminate()


if __name__ == "__main__":
    unittest.main()
