#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Unit tests for MatterBaseTest.pixit() default resolution.

These tests exercise the PIXIT integration on MatterBaseTest, which pulls in
the compiled `matter` wheel (matter.tlv, matter.clusters, ...). They therefore
run in the REPL test environment (out/venv) rather than as a lightweight
pigweed unit test alongside the pure-Python pixit tests in
matter_testing_infrastructure/matter/testing/test_pixit.py.
"""

import unittest

from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit


class TestMatterBaseTestPixit(unittest.TestCase):
    """Tests for MatterBaseTest.pixit() default resolution."""

    def _make_test_instance(self, test_method_name: str):
        class PixitTestClass(MatterBaseTest):
            @pixit("path", str, "Required path", required=True, default="/decorator/default")
            def test_required(self):
                pass

            @pixit("timeout", int, "Timeout", required=False, default=30)
            def test_optional(self):
                pass

        instance = PixitTestClass.__new__(PixitTestClass)
        instance.user_params = {}
        instance.current_test_info = type("TestInfo", (), {"name": test_method_name})()
        return instance

    def test_required_pixit_ignores_decorator_default(self):
        """Required PIXITs must not mask missing values with a decorator default."""
        instance = self._make_test_instance("test_required")
        self.assertIsNone(instance.pixit("path"))
        self.assertEqual(instance.pixit("path", default="method-default"), "method-default")

    def test_optional_pixit_uses_decorator_default(self):
        """Optional PIXITs fall back to the decorator default when unset."""
        instance = self._make_test_instance("test_optional")
        self.assertEqual(instance.pixit("timeout"), 30)


if __name__ == "__main__":
    unittest.main()
