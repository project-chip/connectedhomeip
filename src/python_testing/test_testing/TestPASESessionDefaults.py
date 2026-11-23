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

"""Unit tests for controller defaults used by PASE callers across the test framework."""

import asyncio
import unittest
from types import MethodType, SimpleNamespace
from unittest.mock import AsyncMock, MagicMock, Mock, call, patch

from matter.ChipDeviceCtrl import BareChipDeviceController, CallbackContext, ChipDeviceControllerBase, DeviceProxyWrapper
from matter.FabricAdmin import FabricAdmin
from matter.testing import global_stash
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_test_config import MatterTestConfig
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import generate_mobly_test_config

NODE_ID = 0x12344321
SETUP_CODE = "34970112332"
BORDER_AGENT = ("fd5b:35de:ad5b:beef:2ecf:67ff:fed7:841d", 49154)
DATASET = bytes.fromhex("041000112233445566778899aabbccddeeff")


def make_controller(controller_type: type[ChipDeviceControllerBase] = ChipDeviceControllerBase):
    controller = object.__new__(controller_type)
    controller._isActive = False
    controller.CheckIsActive = Mock()
    controller.SetDefaultThreadMeshCoPConfig(None)
    controller.devCtrl = None
    controller._dmLib = Mock()
    controller._dmLib.pychip_GetDeviceBeingCommissioned.side_effect = [
        SimpleNamespace(is_success=False), SimpleNamespace(is_success=True)]
    controller._pase_establishment_context = CallbackContext(asyncio.Lock())
    controller._enablePairingCompleteCallback = Mock()

    def call_async(function):
        function().raise_on_error()
        controller._pase_establishment_context.future.set_result(None)

    controller._ChipStack = SimpleNamespace(
        Call=Mock(side_effect=lambda function: function()),
        CallAsync=AsyncMock(side_effect=call_async),
        CallAsyncWithResult=AsyncMock(side_effect=lambda function, timeout: function()))
    controller.EstablishPASESession = AsyncMock()
    return controller


class TestPASESessionDefaults(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.controller = make_controller()
        self.meshcop = self.controller._dmLib.pychip_DeviceController_EstablishPASESessionThreadMeshcop
        self.set_dataset = self.controller._dmLib.pychip_DeviceController_SetThreadOperationalDataset

    async def test_no_defaults_uses_generic_pase(self):
        await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        self.controller.EstablishPASESession.assert_awaited_once_with(SETUP_CODE, NODE_ID)
        self.meshcop.assert_not_called()
        self.set_dataset.assert_not_called()

    async def test_defaults_pass_dataset_to_meshcop_establishment(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)

        result = await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID, timeoutMs=1000)

        self.meshcop.assert_called_once_with(
            None, BORDER_AGENT[0].encode(), BORDER_AGENT[1], SETUP_CODE.encode(), NODE_ID, DATASET, len(DATASET))
        self.set_dataset.assert_not_called()
        self.assertIsInstance(result, DeviceProxyWrapper)
        self.assertEqual(result._proxyType, DeviceProxyWrapper.DeviceProxyType.COMMISSIONEE)
        self.controller.EstablishPASESession.assert_not_awaited()
        self.assertTrue(all(args.args[1] == 1000
                            for args in self.controller._ChipStack.CallAsyncWithResult.await_args_list))

    async def test_queued_meshcop_requests_keep_their_own_datasets(self):
        other_node_id = NODE_ID + 1
        other_agent = ("::1", 49155)
        other_dataset = bytes.fromhex("0410ffeeddccbbaa99887766554433221100")
        lookups = asyncio.Queue()

        def get_device(_controller, node_id, _device):
            lookups.put_nowait(node_id)
            return SimpleNamespace(is_success=any(args.args[4] == node_id for args in self.meshcop.call_args_list))

        self.controller._dmLib.pychip_GetDeviceBeingCommissioned.side_effect = get_device

        async with asyncio.TaskGroup() as requests:
            async with self.controller._pase_establishment_context:
                self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
                first = requests.create_task(self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID))
                self.assertEqual(await asyncio.wait_for(lookups.get(), timeout=5), NODE_ID)

                self.controller.SetDefaultThreadMeshCoPConfig(other_agent, other_dataset)
                second = requests.create_task(self.controller.FindOrEstablishPASESession(SETUP_CODE, other_node_id))
                self.assertEqual(await asyncio.wait_for(lookups.get(), timeout=5), other_node_id)
                self.meshcop.assert_not_called()
                self.set_dataset.assert_not_called()

        self.assertIsInstance(first.result(), DeviceProxyWrapper)
        self.assertIsInstance(second.result(), DeviceProxyWrapper)
        self.assertEqual(self.meshcop.call_args_list, [
            call(None, BORDER_AGENT[0].encode(), BORDER_AGENT[1], SETUP_CODE.encode(), NODE_ID, DATASET, len(DATASET)),
            call(None, other_agent[0].encode(), other_agent[1], SETUP_CODE.encode(), other_node_id, other_dataset, len(other_dataset)),
        ])

    async def test_bare_controller_uses_default_thread_dataset(self):
        controller = make_controller(BareChipDeviceController)
        controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)

        result = await controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        self.assertIsInstance(result, DeviceProxyWrapper)
        controller._dmLib.pychip_DeviceController_EstablishPASESessionThreadMeshcop.assert_called_once_with(
            None, BORDER_AGENT[0].encode(), BORDER_AGENT[1], SETUP_CODE.encode(), NODE_ID, DATASET, len(DATASET))

    async def test_explicit_border_agent_overrides_defaults_and_preserves_callers_dataset(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        explicit_agent = ("::1", 49155)

        await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID, threadMeshCoPConfig=explicit_agent)

        self.meshcop.assert_called_once_with(
            None, explicit_agent[0].encode(), explicit_agent[1], SETUP_CODE.encode(), NODE_ID, None, 0)
        self.set_dataset.assert_not_called()
        self.controller.EstablishPASESession.assert_not_awaited()

    async def test_existing_session_is_reused_without_changing_dataset(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        self.controller._dmLib.pychip_GetDeviceBeingCommissioned.side_effect = [SimpleNamespace(is_success=True)]

        self.assertIsInstance(await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID), DeviceProxyWrapper)

        self.set_dataset.assert_not_called()
        self.meshcop.assert_not_called()
        self.controller.EstablishPASESession.assert_not_awaited()

    async def test_clear_defaults_restores_generic_pase(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        self.controller.SetDefaultThreadMeshCoPConfig(None)

        await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        self.controller.EstablishPASESession.assert_awaited_once_with(SETUP_CODE, NODE_ID)
        self.meshcop.assert_not_called()
        self.set_dataset.assert_not_called()

    async def test_defaults_do_not_leak_to_another_controller(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        other_controller = make_controller()

        await other_controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        other_controller.EstablishPASESession.assert_awaited_once_with(SETUP_CODE, NODE_ID)
        other_controller._dmLib.pychip_DeviceController_EstablishPASESessionThreadMeshcop.assert_not_called()

    async def test_dataset_failure_prevents_establishment(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        self.meshcop.return_value.raise_on_error.side_effect = ValueError("invalid dataset")

        with self.assertRaisesRegex(ValueError, "invalid dataset"):
            await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        self.controller._dmLib.pychip_GetDeviceBeingCommissioned.assert_called_once()
        self.controller.EstablishPASESession.assert_not_awaited()

    async def test_meshcop_failure_does_not_fall_back_to_generic_pase(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        self.meshcop.side_effect = TimeoutError("MeshCoP timed out")

        with self.assertRaisesRegex(TimeoutError, "MeshCoP timed out"):
            await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        self.controller.EstablishPASESession.assert_not_awaited()

    async def test_invalid_defaults_do_not_replace_valid_configuration(self):
        self.controller.SetDefaultThreadMeshCoPConfig(BORDER_AGENT, DATASET)
        for agent, dataset in [(BORDER_AGENT, None), (BORDER_AGENT, b""), (("", 49154), DATASET),
                               (("::1", 0), DATASET), (("::1", 65536), DATASET)]:
            with self.subTest(agent=agent, dataset=dataset), self.assertRaises(ValueError):
                self.controller.SetDefaultThreadMeshCoPConfig(agent, dataset)

        await self.controller.FindOrEstablishPASESession(SETUP_CODE, NODE_ID)

        self.meshcop.assert_called_once_with(
            None, BORDER_AGENT[0].encode(), BORDER_AGENT[1], SETUP_CODE.encode(), NODE_ID, DATASET, len(DATASET))


class TestPASESessionFrameworkConfig(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.config = MatterTestConfig()
        self.config.thread_ba_host, self.config.thread_ba_port = BORDER_AGENT
        self.config.thread_operational_dataset = DATASET
        self.test = MagicMock(spec=MatterBaseTest)
        self.test.is_commissioning = False
        self.test.matter_test_config = self.config
        self.test._extra_controllers = []
        self.test._extra_cas = []
        self.test._configure_pase_session = MethodType(MatterBaseTest._configure_pase_session, self.test)
        self.controller = make_controller()
        self.controller._is_default_controller = False
        self.controller._skip_cleanup_tracking = False
        self.controller.fabricAdmin = None

    def test_meshcop_from_either_cli_option_configures_defaults(self):
        for method, in_test_method in [(None, "thread-meshcop"), ("thread-meshcop", None),
                                       ("ble-thread", "thread-meshcop")]:
            with self.subTest(method=method, in_test_method=in_test_method):
                controller = Mock(spec=ChipDeviceControllerBase)
                self.config.commissioning_method = method
                self.config.in_test_commissioning_method = in_test_method

                self.test._configure_pase_session(controller)

                controller.SetDefaultThreadMeshCoPConfig.assert_called_once_with(BORDER_AGENT, DATASET)

    def test_commissioning_pretest_uses_commissioning_method(self):
        self.test.is_commissioning = True
        self.config.commissioning_method = "ble-thread"
        self.config.in_test_commissioning_method = "thread-meshcop"
        controller = Mock(spec=ChipDeviceControllerBase)

        self.test._configure_pase_session(controller)

        controller.SetDefaultThreadMeshCoPConfig.assert_called_once_with(None)

    def test_other_transports_clear_stale_meshcop_defaults(self):
        for method, in_test_method in [(None, None), ("ble-thread", None), (None, "on-network"),
                                       ("thread-meshcop", "ble-wifi")]:
            with self.subTest(method=method, in_test_method=in_test_method):
                controller = Mock(spec=ChipDeviceControllerBase)
                self.config.commissioning_method = method
                self.config.in_test_commissioning_method = in_test_method

                self.test._configure_pase_session(controller)

                controller.SetDefaultThreadMeshCoPConfig.assert_called_once_with(None)

    def test_missing_border_agent_is_rejected(self):
        self.config.in_test_commissioning_method = "thread-meshcop"
        for host, port in [(None, None), (None, 49154), ("::1", None)]:
            with self.subTest(host=host, port=port):
                self.config.thread_ba_host, self.config.thread_ba_port = host, port
                with self.assertRaisesRegex(ValueError, "requires both border agent host and port"):
                    self.test._configure_pase_session(self.controller)

    def test_missing_dataset_is_rejected(self):
        self.config.in_test_commissioning_method = "thread-meshcop"
        self.config.thread_operational_dataset = None

        with self.assertRaisesRegex(ValueError, "requires a Thread operational dataset"):
            self.test._configure_pase_session(self.controller)

    def test_new_controllers_receive_defaults_and_remain_tracked_for_cleanup(self):
        self.config.in_test_commissioning_method = "thread-meshcop"

        MatterBaseTest._on_new_controller_created(self.test, self.controller)

        self.assertEqual(self.controller._defaultThreadMeshCoPConfig, (BORDER_AGENT, DATASET))
        self.assertEqual(self.test._extra_controllers, [self.controller])

    def test_controllers_excluded_from_cleanup_still_receive_defaults(self):
        self.config.in_test_commissioning_method = "thread-meshcop"
        self.controller._skip_cleanup_tracking = True

        MatterBaseTest._on_new_controller_created(self.test, self.controller)

        self.assertEqual(self.controller._defaultThreadMeshCoPConfig, (BORDER_AGENT, DATASET))
        self.assertEqual(self.test._extra_controllers, [])

    async def test_composition_setup_uses_meshcop_without_changing_the_pase_caller(self):
        self.config.in_test_commissioning_method = "thread-meshcop"
        self.config.manual_code = [SETUP_CODE]
        self.config.dut_node_ids = [NODE_ID]
        self.controller._Cluster = Mock()

        async def wait_for_case(**_kwargs):
            await asyncio.Future()

        self.controller.GetConnectedDevice = AsyncMock(side_effect=wait_for_case)
        self.controller.Read = AsyncMock(return_value=SimpleNamespace(attributes={}, tlvAttributes={}))
        mobly_config = generate_mobly_test_config(self.config)

        with patch.dict(global_stash._GLOBAL_DATA), patch.object(FabricAdmin, "_new_controller_hook", None):
            mobly_config.user_params["default_controller"] = global_stash.stash_globally(self.controller)
            mobly_config.user_params["matter_test_config"] = global_stash.stash_globally(self.config)
            composition = BasicCompositionTests(mobly_config)
            with patch("matter.testing.matter_testing.ClusterMapper"), \
                    patch.object(composition, "_build_cq_excluded_ids", return_value=frozenset()):
                composition.setup_class()

            await composition.setup_class_helper()

        self.controller._dmLib.pychip_DeviceController_EstablishPASESessionThreadMeshcop.assert_called_once_with(
            None, BORDER_AGENT[0].encode(), BORDER_AGENT[1], SETUP_CODE.encode(), NODE_ID, DATASET, len(DATASET))
        self.controller.Read.assert_awaited_once_with(NODE_ID, [()])


if __name__ == "__main__":
    unittest.main()
