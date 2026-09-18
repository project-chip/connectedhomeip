#!/usr/bin/env -S python3 -B
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

"""Unit tests for the per-test teardown of MatterBaseTest.

A test class overrides teardown_test freely, and the framework has per-test teardown of its
own to run as well. That is why the framework's teardown hangs off _teardown_test, mobly's
proxy around teardown_test: an override that forgets to call super(), or that raises, must
not be able to take the framework's cleanup with it.

The tests drive _teardown_test directly, which needs neither a DUT nor a Matter stack. The
order is a property of the class hierarchy alone, and a hierarchy is all these build.
"""

import tempfile
import unittest
from types import SimpleNamespace

from mobly import config_parser

from matter.testing.matter_testing import MatterBaseTest


class RecordingTest(MatterBaseTest):
    """Records the teardown it runs, and has nothing of its own to tear down.

    What stands in for the framework's teardown is the shutdown of the wildcard
    subscription, since that is the part of it a test class can actually be made to skip.
    """

    def __init__(self, *args):
        super().__init__(*args)
        self.order = []
        self.wildcard_subscription_handler = SimpleNamespace(
            shutdown=lambda: self.order.append("framework"))


class RecordingMixin:
    """Stands in for a mixin that brings a test class per-test cleanup of its own."""

    def teardown_test(self):
        self.order.append("mixin")
        super().teardown_test()


class TeardownWithMixin(RecordingMixin, RecordingTest):

    def teardown_test(self):
        self.order.append("own")
        super().teardown_test()


class TeardownWithoutSuper(RecordingMixin, RecordingTest):

    def teardown_test(self):
        self.order.append("own")


class TeardownRaises(RecordingTest):

    def teardown_test(self):
        self.order.append("own")
        raise RuntimeError("teardown_test failed")


class TestMatterBaseTestTeardown(unittest.TestCase):

    def setUp(self):
        self.log_path = tempfile.TemporaryDirectory()
        self.addCleanup(self.log_path.cleanup)

    def instance(self, test_class: type[RecordingTest]) -> RecordingTest:
        config = config_parser.TestRunConfig()
        config.log_path = self.log_path.name
        instance = test_class(config)
        # Set up by mobly when it starts a test case. The teardown path only logs the name.
        instance.current_test_info = SimpleNamespace(name="test_x")
        return instance

    def test_framework_teardown_runs_without_an_override(self):
        instance = self.instance(RecordingTest)
        instance._teardown_test("test_x")
        self.assertEqual(instance.order, ["framework"])

    def test_framework_teardown_runs_after_every_override(self):
        instance = self.instance(TeardownWithMixin)
        instance._teardown_test("test_x")
        self.assertEqual(instance.order, ["own", "mixin", "framework"])

    def test_framework_teardown_runs_without_super(self):
        # The mixin is skipped, which is between it and whoever wrote the override, but the
        # framework's own teardown is not.
        instance = self.instance(TeardownWithoutSuper)
        instance._teardown_test("test_x")
        self.assertEqual(instance.order, ["own", "framework"])

    def test_framework_teardown_runs_when_an_override_raises(self):
        instance = self.instance(TeardownRaises)
        with self.assertRaises(RuntimeError):
            instance._teardown_test("test_x")
        self.assertEqual(instance.order, ["own", "framework"])


if __name__ == "__main__":
    unittest.main()
