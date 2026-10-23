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

# TODO: Allow to use relative imports or imports from matter.testing package. Then,
#       rename "tasks" module to "subprocess", because it would be more descriptive.
#       Unfortunately, current way of importing clashes with the subprocess module
#       from the Python standard library.
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

    def test_wait_for_armed_output(self):
        p = Subprocess("python3", "-c", "print('armed line')")
        p.arm_output_match("armed line")
        p.start()
        self.assertTrue(p.wait_for_output(timeout=5))
        p.terminate()

    def test_wait_for_armed_output_timeout(self):
        p = Subprocess("python3", "-c", "print('some other line')")
        p.arm_output_match("armed line")
        p.start()
        self.assertFalse(p.wait_for_output(timeout=1))
        p.terminate()

    def test_arm_output_match_discards_previous_match(self):
        p = Subprocess("python3", "-c", "print('Hello, World!')")
        p.start(expected_output="Hello, World!", timeout=5)
        # start() leaves its own match set; re-arming must not report it as a fresh match.
        p.arm_output_match("never printed")
        self.assertFalse(p.wait_for_output(timeout=0))
        p.terminate()


if __name__ == "__main__":
    unittest.main()
