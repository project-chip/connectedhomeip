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
from unittest import mock

from mobly import signals

from matter.testing import global_stash
from matter.testing.CommissioningPreTest import CommissionDeviceTest
from matter.testing.matter_test_config import MatterTestConfig
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


class TestMatterBaseTestSetupValidation(unittest.TestCase):
    """setup_test() must validate declared PIXITs before any DUT interaction.

    These guard the wiring of _validate_test_parameters() into setup_test():
    the call must run, and it must run before the ACL-capture / background
    wildcard-subscription branches so a misconfigured run fails fast without
    touching the DUT.
    """

    def _make_instance(self, *, provide_required: bool, force_dut_expected: bool):
        class _SetupValidationTest(MatterBaseTest):
            # requires_dut = False keeps the background wildcard subscription out of
            # setup_test, so this exercises validation without needing a real DUT.
            requires_dut = False

            @pixit("smoke_pixit", str, "Required smoke pixit", required=True)
            def test_needs_pixit(self):
                pass

        instance = _SetupValidationTest.__new__(_SetupValidationTest)
        instance.is_commissioning = False
        # force_dut_expected makes the ACL-capture branch eligible without providing a
        # usable event_loop; if setup_test reached it, it would raise there (not in
        # validation), which is exactly the regression these tests must catch.
        instance._dut_confirmed_available = force_dut_expected
        # matter_test_config is a read-only property backed by the global stash via
        # user_params, mirroring how the framework wires the config onto a test.
        user_params = {"matter_test_config": global_stash.stash_globally(MatterTestConfig())}
        if provide_required:
            user_params["smoke_pixit"] = "value"
        instance.user_params = user_params
        instance.current_test_info = type("TestInfo", (), {"name": "test_needs_pixit"})()
        return instance

    def test_setup_test_fails_when_required_pixit_missing(self):
        """A missing required PIXIT fails in validation, ahead of the DUT branch."""
        instance = self._make_instance(provide_required=False, force_dut_expected=True)
        with self.assertRaises(signals.TestFailure) as ctx:
            instance.setup_test()
        msg = str(ctx.exception)
        self.assertIn("missing required PIXIT", msg)
        self.assertIn("smoke_pixit", msg)
        self.assertNotIn("AccessControl", msg)

    def test_setup_test_passes_when_required_pixit_present(self):
        """With the PIXIT supplied and no DUT expected, setup_test completes cleanly."""
        instance = self._make_instance(provide_required=True, force_dut_expected=False)
        instance.setup_test()
        self.assertEqual(instance.current_step_index, 0)
        self.assertFalse(instance.failed)

    def test_setup_test_fails_when_pixit_wrong_type(self):
        """A present PIXIT with the wrong Python type fails in validation, not at the DUT."""
        class _TypeValidationTest(MatterBaseTest):
            requires_dut = False

            @pixit("smoke_pixit", str, "Required smoke pixit", required=True)
            @pixit("smoke_timeout", int, "Timeout seconds", required=True)
            def test_needs_pixit(self):
                pass

        instance = _TypeValidationTest.__new__(_TypeValidationTest)
        instance.is_commissioning = False
        instance._dut_confirmed_available = True
        instance.user_params = {
            "matter_test_config": global_stash.stash_globally(MatterTestConfig()),
            "smoke_pixit": "value",
            "smoke_timeout": "30",
        }
        instance.current_test_info = type("TestInfo", (), {"name": "test_needs_pixit"})()
        with self.assertRaises(signals.TestFailure) as ctx:
            instance.setup_test()
        msg = str(ctx.exception)
        self.assertIn("PIXIT parameter type error", msg)
        self.assertIn("smoke_timeout", msg)
        self.assertNotIn("AccessControl", msg)


class TestMatterBaseTestBaselineCapture(unittest.TestCase):
    """Regression coverage for pre-test DUT baseline probing."""

    def _make_instance(self, *, commissioning_method=None, in_test_commissioning_method=None,
                       dut_confirmed=False, pre_test_commissioning_complete=False):
        class _CommissioningTest(MatterBaseTest):
            pass

        instance = _CommissioningTest.__new__(_CommissioningTest)
        instance.is_commissioning = False
        instance._original_acl = None
        instance._original_fabrics = None
        instance._dut_confirmed_available = dut_confirmed
        instance.event_loop = mock.Mock()
        instance._capture_original_acl = mock.Mock()
        instance._capture_original_fabrics = mock.Mock()

        config = MatterTestConfig(commissioning_method=commissioning_method,
                                  in_test_commissioning_method=in_test_commissioning_method,
                                  dut_node_ids=[0x12344321])
        config._pre_test_commissioning_complete = pre_test_commissioning_complete
        controller = mock.Mock()
        instance.user_params = {
            "matter_test_config": global_stash.stash_globally(config),
            "default_controller": global_stash.stash_globally(controller),
        }
        return instance, controller

    def test_configured_commissioning_skips_unconfirmed_default_node_lookup(self):
        """Do not start CASE discovery before configured framework commissioning."""
        instance, controller = self._make_instance(commissioning_method="ble-thread")

        instance._capture_dut_baseline()

        controller.GetConnectedDevice.assert_not_called()
        instance.event_loop.run_until_complete.assert_not_called()

    def test_in_test_commissioning_skips_unconfirmed_default_node_lookup(self):
        """Do not start CASE discovery before commissioning performed in the test body."""
        instance, controller = self._make_instance(in_test_commissioning_method="ble-thread")

        instance._capture_dut_baseline()

        controller.GetConnectedDevice.assert_not_called()
        instance.event_loop.run_until_complete.assert_not_called()

    def test_no_commissioning_method_retains_short_baseline_probe(self):
        instance, controller = self._make_instance()

        instance._capture_dut_baseline()

        controller.GetConnectedDevice.assert_called_once_with(
            nodeId=0x12344321, allowPASE=False, timeoutMs=500)
        instance._capture_original_acl.assert_called_once_with()
        instance._capture_original_fabrics.assert_called_once_with()

    def test_confirmed_dut_retains_full_baseline_capture(self):
        instance, controller = self._make_instance(in_test_commissioning_method="ble-thread", dut_confirmed=True)

        instance._capture_dut_baseline()

        controller.GetConnectedDevice.assert_called_once_with(
            nodeId=0x12344321, allowPASE=False, timeoutMs=5000)
        instance._capture_original_acl.assert_called_once_with()
        instance._capture_original_fabrics.assert_called_once_with()

    def test_completed_framework_commissioning_retains_full_baseline_capture(self):
        instance, controller = self._make_instance(commissioning_method="ble-thread",
                                                   pre_test_commissioning_complete=True)

        instance._capture_dut_baseline()

        self.assertTrue(instance._dut_confirmed_available)
        controller.GetConnectedDevice.assert_called_once_with(
            nodeId=0x12344321, allowPASE=False, timeoutMs=5000)
        instance._capture_original_acl.assert_called_once_with()
        instance._capture_original_fabrics.assert_called_once_with()

    def test_successful_framework_commissioning_is_shared_with_test_class(self):
        config = MatterTestConfig(commissioning_method="ble-thread")
        instance = CommissionDeviceTest.__new__(CommissionDeviceTest)
        instance.event_loop = mock.Mock()
        instance.event_loop.run_until_complete.return_value = True
        instance.dut_node_ids = []
        instance.setup_payloads = []
        instance.commissioning_info = mock.sentinel.commissioning_info
        instance.user_params = {
            "matter_test_config": global_stash.stash_globally(config),
            "default_controller": global_stash.stash_globally(mock.Mock()),
        }

        with mock.patch("matter.testing.CommissioningPreTest.commission_devices",
                        new=mock.Mock(return_value=mock.sentinel.commissioning)):
            instance.test_run_commissioning()

        self.assertTrue(config._pre_test_commissioning_complete)


if __name__ == "__main__":
    unittest.main()
