"""DUT_R commissioning regression tests without a live Matter stack or devices."""

import asyncio
import os
import sys
import unittest
from types import SimpleNamespace
from unittest.mock import AsyncMock, Mock, call, create_autospec, patch

from mobly import signals

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.commissioning import PairingStatus
from matter.testing.matter_testing import MatterBaseTest

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import TC_PROXRTestBase as proxr  # noqa: E402 isort:skip
from TC_PROXR_2_3 import TC_PROXR_2_3  # noqa: E402 isort:skip
from TC_PROXR_2_4 import TC_PROXR_2_4  # noqa: E402 isort:skip


def reflector():
    device = proxr.ProximityRangingTestBase()
    device.user_params = {"dut_r_node_id": "2", "dut_r_discriminator": "3841", "dut_r_passcode": "20202021"}
    device.is_pics_sdk_ci_only = False
    device.dut_node_id = 1
    device.get_endpoint = Mock(return_value=1)
    device.wait_for_user_input = Mock()
    device.matter_test_config = SimpleNamespace(
        dut_node_ids=[1], commissioning_method=None, tc_version_to_simulate=None, tc_user_response_to_simulate=None,
        wifi_ssid="shared-test-ssid", wifi_passphrase="shared-test-passphrase",
        thread_operational_dataset=b"test-dataset", thread_ba_host="::1", thread_ba_port=49191)
    device.default_controller = create_autospec(ChipDeviceCtrl.ChipDeviceController, instance=True)
    device.default_controller.nodeId = 112233
    device.default_controller.ReadAttribute.return_value = {
        7: {Clusters.Descriptor: {Clusters.Descriptor.Attributes.ServerList: [Clusters.ProximityRanging.id]}}}
    return device


class TestReflectorSetup(unittest.TestCase):
    def test_setup_coerces_integers_without_prompting(self):
        device = reflector()
        self.assertEqual(device.setup_reflector_device(), 2)
        self.assertEqual(device._reflector_discriminator, 3841)
        self.assertEqual(device._reflector_passcode, 20202021)
        self.assertIsNone(device.reflector_endpoint)
        device.wait_for_user_input.assert_not_called()

    def test_second_node_id_and_conflicting_override(self):
        device = reflector()
        device.matter_test_config.dut_node_ids = [1, 8]
        with self.assertRaises(signals.TestFailure):
            device.setup_reflector_device()
        del device.user_params["dut_r_node_id"]
        self.assertEqual(device.setup_reflector_device(), 8)

    def test_missing_and_colliding_node_ids(self):
        for node_id in (None, 1, 112233, 0, 0xFFFFFFFFFFFFFFFF):
            with self.subTest(node_id=node_id):
                device = reflector()
                device.user_params["dut_r_node_id"] = node_id
                with self.assertRaises(signals.TestFailure):
                    device.setup_reflector_device()

    def test_invalid_integer_parameters(self):
        for name, value in (("dut_r_discriminator", "bad"), ("dut_r_discriminator", 4096),
                            ("dut_r_discriminator", 1.5), ("dut_r_discriminator", True),
                            ("dut_r_passcode", 12345678), ("dut_r_endpoint", 65535)):
            with self.subTest(name=name, value=value):
                device = reflector()
                device.user_params[name] = value
                with self.assertRaises(signals.TestFailure):
                    device.setup_reflector_device()

    def test_physical_pairing_credentials_are_required(self):
        for name in ("dut_r_discriminator", "dut_r_passcode"):
            with self.subTest(name=name):
                device = reflector()
                del device.user_params[name]
                with self.assertRaises(signals.TestFailure):
                    device.setup_reflector_device()

    def test_ci_uses_pics_and_avoids_controller_node(self):
        device = reflector()
        device.is_pics_sdk_ci_only = True
        device.default_controller.nodeId = 2
        device.user_params = {"th_reflector_app_path": "reflector-app"}
        with patch.object(proxr.os.path, "exists", return_value=True), \
                patch.object(proxr.tempfile, "TemporaryDirectory"), patch.object(proxr, "Subprocess") as process:
            self.assertEqual(device.setup_reflector_device(), 3)
            process.return_value.start.assert_called_once()
            self.assertEqual(device.reflector_endpoint, 1)
            device.teardown_reflector_device()
            process.return_value.terminate.assert_called_once()
        device.wait_for_user_input.assert_not_called()
        device.default_controller.UnpairDevice.assert_not_awaited()

    def test_properties_are_shared(self):
        for test_class in (TC_PROXR_2_3, TC_PROXR_2_4):
            for name in ("dut_r_discriminator", "dut_r_passcode", "dut_r_endpoint", "dut_r_commissioning_method"):
                self.assertIs(getattr(test_class, name), getattr(proxr.ProximityRangingTestBase, name))


class TestReflectorCommissioning(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.device = reflector()
        self.device.setup_reflector_device()
        self.controller = self.device.default_controller
        self.commission = AsyncMock(return_value=PairingStatus())
        patcher = patch.object(proxr, "commission_device", self.commission)
        patcher.start()
        self.addCleanup(patcher.stop)

    async def test_on_network_reset_tc_prompt_and_discovery_order(self):
        self.device.matter_test_config.tc_version_to_simulate = 0
        self.device.matter_test_config.tc_user_response_to_simulate = 0
        order = Mock()
        order.attach_mock(self.device.wait_for_user_input, "prompt")
        order.attach_mock(self.controller, "controller")
        await self.device.commission_reflector()
        self.assertEqual(order.mock_calls[0], call.prompt(prompt_msg="Put DUT_R into commissioning mode and press ENTER."))
        self.assertEqual(self.controller.mock_calls[0:2], [call.ResetCommissioningParameters(), call.SetTCAcknowledgements(0, 0)])
        self.assertEqual(self.controller.mock_calls[-2], call.ResetCommissioningParameters())
        self.assertEqual(self.controller.ResetCommissioningParameters.call_count, 2)
        self.controller.CommissionOnNetwork.assert_awaited_once_with(
            nodeId=2, setupPinCode=20202021,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=3841)
        self.assertEqual(self.device.reflector_endpoint, 7)
        self.assertTrue(self.device._reflector_commissioned)
        self.commission.assert_not_awaited()

    async def test_ci_never_prompts_and_uses_on_network(self):
        self.device.is_pics_sdk_ci_only = True
        self.device.user_params["dut_r_commissioning_method"] = "ble-thread"
        await self.device.commission_reflector()
        self.device.wait_for_user_input.assert_not_called()
        self.controller.CommissionOnNetwork.assert_awaited_once()

    async def test_ble_transports_only_receive_their_own_network(self):
        for method in ("ble-wifi", "ble-thread"):
            with self.subTest(method=method):
                self.device.user_params["dut_r_commissioning_method"] = method
                await self.device.commission_reflector()
                info = self.commission.await_args.args[3]
                if method == "ble-wifi":
                    self.assertEqual(info.wifi_ssid, "shared-test-ssid")
                    self.assertEqual(info.wifi_passphrase, "shared-test-passphrase")
                    self.assertIsNone(info.thread_operational_dataset)
                else:
                    self.assertIsNone(info.wifi_ssid)
                    self.assertIsNone(info.wifi_passphrase)
                    self.assertEqual(info.thread_operational_dataset, b"test-dataset")
                self.controller.EstablishPASESessionBLE.assert_awaited_with(
                    setupPinCode=20202021, discriminator=3841, nodeId=2)

    async def test_wifi_overrides_are_atomic(self):
        self.device.user_params.update(dut_r_commissioning_method="ble-wifi", dut_r_wifi_ssid="override")
        with self.assertRaises(signals.TestFailure):
            await self.device.commission_reflector()
        self.device.wait_for_user_input.assert_not_called()
        self.controller.ResetCommissioningParameters.assert_not_called()
        self.device.user_params["dut_r_wifi_passphrase"] = "override-passphrase"
        await self.device.commission_reflector()
        info = self.commission.await_args.args[3]
        self.assertEqual((info.wifi_ssid, info.wifi_passphrase), ("override", "override-passphrase"))

    async def test_ip_does_not_inherit_network_credentials(self):
        self.device.user_params.update(dut_r_commissioning_method="on-network-ip", dut_r_ip_addr="::1")
        await self.device.commission_reflector()
        info = self.commission.await_args.args[3]
        self.assertIsNone(info.wifi_ssid)
        self.assertIsNone(info.wifi_passphrase)
        self.assertIsNone(info.thread_operational_dataset)
        self.controller.EstablishPASESessionIP.assert_awaited_once_with(ipaddr="::1", setupPinCode=20202021, nodeId=2)

    async def test_meshcop_preserves_long_discriminator_and_coerces_port(self):
        self.device.user_params.update(dut_r_commissioning_method="thread-meshcop", dut_r_thread_ba_port="49192")
        await self.device.commission_reflector()
        self.controller.CommissionThreadMeshcop.assert_awaited_once_with(
            nodeId=2, setupPinCode=20202021, discriminator=3841, borderAgentIPAddr="::1",
            borderAgentPort=49192, threadOperationalDataset=b"test-dataset")
        self.commission.assert_not_awaited()

    async def test_invalid_transport_configuration_fails_before_prompt(self):
        original_params = self.device.user_params.copy()
        for params in ({"dut_r_commissioning_method": "unsupported"},
                       {"dut_r_commissioning_method": "on-network-ip"},
                       {"dut_r_commissioning_method": "ble-thread", "dut_r_thread_operational_dataset": "not-bytes"},
                       {"dut_r_commissioning_method": "thread-meshcop", "dut_r_thread_ba_port": 65536}):
            with self.subTest(params=params):
                self.device.user_params = original_params | params
                with self.assertRaises(signals.TestFailure):
                    await self.device.commission_reflector()
                self.device.wait_for_user_input.assert_not_called()

    async def test_runner_commissioned_second_node_is_not_recommissioned(self):
        self.device.matter_test_config.dut_node_ids = [1, 2]
        self.device.matter_test_config.commissioning_method = "on-network"
        self.device.user_params = {}
        self.device.setup_reflector_device()
        await self.device.commission_reflector()
        self.assertEqual(self.device.reflector_endpoint, 7)
        self.device.wait_for_user_input.assert_not_called()
        self.controller.CommissionOnNetwork.assert_not_awaited()
        self.assertFalse(self.device._reflector_commissioned)
        self.device.teardown_reflector_device()
        self.controller.UnpairDevice.assert_not_awaited()

    async def test_runner_commissioning_primary_does_not_skip_reflector(self):
        self.device.matter_test_config.commissioning_method = "on-network"
        self.device.setup_reflector_device()
        await self.device.commission_reflector()
        self.controller.CommissionOnNetwork.assert_awaited_once()
        self.assertTrue(self.device._reflector_commissioned)

    async def test_reset_after_pase_failure(self):
        self.device.user_params["dut_r_commissioning_method"] = "ble-thread"
        self.controller.EstablishPASESessionBLE.side_effect = RuntimeError("PASE failed")
        with self.assertRaisesRegex(RuntimeError, "PASE failed"):
            await self.device.commission_reflector()
        self.assertEqual(self.controller.ResetCommissioningParameters.call_count, 2)
        self.assertFalse(self.device._reflector_commissioned)
        self.commission.assert_not_awaited()

    async def test_reset_after_on_network_failure(self):
        self.controller.CommissionOnNetwork.side_effect = RuntimeError("pairing failed")
        with self.assertRaisesRegex(RuntimeError, "pairing failed"):
            await self.device.commission_reflector()
        self.assertEqual(self.controller.ResetCommissioningParameters.call_count, 2)
        self.assertFalse(self.device._reflector_commissioned)

    async def test_reset_after_helper_failure(self):
        self.device.user_params["dut_r_commissioning_method"] = "ble-wifi"
        self.commission.return_value = PairingStatus(exception=RuntimeError("pairing failed"))
        with self.assertRaises(signals.TestFailure):
            await self.device.commission_reflector()
        self.assertEqual(self.controller.ResetCommissioningParameters.call_count, 2)
        self.assertFalse(self.device._reflector_commissioned)

    async def test_discovery_failure_keeps_cleanup_ownership(self):
        self.controller.ReadAttribute.side_effect = RuntimeError("read failed")
        with self.assertRaisesRegex(RuntimeError, "read failed"):
            await self.device.commission_reflector()
        self.assertTrue(self.device._reflector_commissioned)
        self.assertEqual(self.controller.ResetCommissioningParameters.call_count, 2)

    async def test_endpoint_ambiguity_and_override(self):
        self.controller.ReadAttribute.return_value[8] = self.controller.ReadAttribute.return_value[7]
        with self.assertRaises(signals.TestFailure):
            await self.device.discover_reflector_endpoint()
        self.device.reflector_endpoint = 8
        await self.device.discover_reflector_endpoint()
        self.assertEqual(self.device._proxr_endpoint(2, None), 8)
        self.assertEqual(self.device._proxr_endpoint(1, None), 1)
        self.assertEqual(self.device._proxr_endpoint(2, 9), 9)
        self.device.reflector_endpoint = 1
        with self.assertRaises(signals.TestFailure):
            await self.device.discover_reflector_endpoint()

    async def test_missing_server_is_rejected(self):
        self.controller.ReadAttribute.return_value = {0: {Clusters.Descriptor: {Clusters.Descriptor.Attributes.ServerList: []}}}
        with self.assertRaises(signals.TestFailure):
            await self.device.discover_reflector_endpoint()


class TestReflectorCleanup(unittest.TestCase):
    def setUp(self):
        self.device = reflector()
        self.device.setup_reflector_device()
        self.device.event_loop = asyncio.new_event_loop()
        self.addCleanup(self.device.event_loop.close)

    def test_only_successfully_commissioned_physical_device_is_unpaired(self):
        self.device.teardown_reflector_device()
        self.device.default_controller.UnpairDevice.assert_not_awaited()
        self.device._reflector_commissioned = True
        self.device.teardown_reflector_device()
        self.device.teardown_reflector_device()
        self.device.default_controller.UnpairDevice.assert_awaited_once_with(2)

    def test_unpair_failure_still_releases_resources(self):
        self.device._reflector_commissioned = True
        self.device.default_controller.UnpairDevice.side_effect = RuntimeError("unpair failed")
        process, storage = Mock(), Mock()
        self.device.reflector, self.device._reflector_storage = process, storage
        with self.assertRaisesRegex(RuntimeError, "unpair failed"):
            self.device.teardown_reflector_device()
        process.terminate.assert_called_once()
        storage.cleanup.assert_called_once()

    def test_both_classes_always_call_base_teardown(self):
        for test_class in (TC_PROXR_2_3, TC_PROXR_2_4):
            with self.subTest(test_class=test_class.__name__):
                instance = object.__new__(test_class)
                instance.teardown_reflector_device = Mock(side_effect=RuntimeError("unpair failed"))
                with patch.object(MatterBaseTest, "teardown_class") as teardown:
                    with self.assertRaisesRegex(RuntimeError, "unpair failed"):
                        instance.teardown_class()
                    teardown.assert_called_once()


class TestReflectorSummaryRedaction(unittest.TestCase):
    def test_sensitive_values_are_redacted(self):
        for key in ("dut_r_wifi_ssid", "dut_r_wifi_passphrase", "thread_operational_dataset", "dut_r_thread_operational_dataset"):
            with self.subTest(key=key):
                self.assertEqual(MatterBaseTest._format_summary_value(None, key, b"test-secret"), "***REDACTED***")
        self.assertEqual(MatterBaseTest._format_summary_value(None, "dut_r_endpoint", 7), "7")


if __name__ == "__main__":
    unittest.main()
