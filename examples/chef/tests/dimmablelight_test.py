#
#    Copyright (c) 2025 Project CHIP Authors
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

import logging

from attributes_service import attributes_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_DIMMABLELIGHT(MatterBaseTest):
    """Tests for chef dimmablelight device."""

    _DIMMABLELIGHT_ENDPOINT = 1
    _MAX_BRIGHTNESS_LEVEL = 254
    _MID_BRIGHTNESS_LEVEL = 127
    _MIN_BRIGHTNESS_LEVEL = 1

    _PW_RPC_SOCKET_ADDR = "0.0.0.0:33000"
    _PW_RPC_BAUD_RATE = 115200

    async def _read_on_off(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._DIMMABLELIGHT_ENDPOINT, cluster=Clusters.Objects.OnOff, attribute=Clusters.Objects.OnOff.Attributes.OnOff)

    async def _read_current_level(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._DIMMABLELIGHT_ENDPOINT, cluster=Clusters.Objects.LevelControl, attribute=Clusters.Objects.LevelControl.Attributes.CurrentLevel)

    async def _read_identify_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._DIMMABLELIGHT_ENDPOINT, cluster=Clusters.Objects.Identify, attribute=Clusters.Objects.Identify.Attributes.IdentifyTime)

    def _read_on_off_pwrpc(self, device):
        result = device.rpcs.chip.rpc.Attributes.Read(
            endpoint=self._DIMMABLELIGHT_ENDPOINT,
            cluster=Clusters.Objects.OnOff.id,
            attribute_id=Clusters.Objects.OnOff.Attributes.OnOff.attribute_id,
            type=attributes_service_pb2.AttributeType.ZCL_BOOLEAN_ATTRIBUTE_TYPE
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")
        return result.response.data_bool

    def _write_on_off_pwrpc(self, device, onOff: bool):
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_bool=onOff),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=self._DIMMABLELIGHT_ENDPOINT,
                cluster=Clusters.Objects.OnOff.id,
                attribute_id=Clusters.Objects.OnOff.Attributes.OnOff.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_BOOLEAN_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    def _read_current_level_pwrpc(self, device):
        result = device.rpcs.chip.rpc.Attributes.Read(
            endpoint=self._DIMMABLELIGHT_ENDPOINT,
            cluster=Clusters.Objects.LevelControl.id,
            attribute_id=Clusters.Objects.LevelControl.Attributes.CurrentLevel.attribute_id,
            type=attributes_service_pb2.AttributeType.ZCL_INT8U_ATTRIBUTE_TYPE
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")
        return result.response.data_uint8

    def _write_current_level_pwrpc(self, device, level: int):
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_uint8=level),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=self._DIMMABLELIGHT_ENDPOINT,
                cluster=Clusters.Objects.LevelControl.id,
                attribute_id=Clusters.Objects.LevelControl.Attributes.CurrentLevel.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_INT8U_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    def desc_TC_DIMMABLELIGHT(self) -> str:
        return "[TC_DIMMABLELIGHT] chef dimmablelight functionality test."

    def steps_TC_DIMMABLELIGHT(self):
        return [TestStep(1, "[TC_DIMMABLELIGHT] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_DIMMABLELIGHT] Test level control."),
                TestStep(3, "[TC_DIMMABLELIGHT] Test toggle."),
                TestStep(4, "[TC_DIMMABLELIGHT] Test identify."),
                TestStep(5, "[TC_DIMMABLELIGHT] Set up PwRPC connection."),
                TestStep(6, "[TC_DIMMABLELIGHT] Test PwRPC on/off and level control.")]

    @async_test_body
    async def test_TC_DIMMABLELIGHT(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.On(),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._DIMMABLELIGHT_ENDPOINT,
        )
        asserts.assert_equal(await self._read_on_off(), True)
        for level in [self._MID_BRIGHTNESS_LEVEL, self._MIN_BRIGHTNESS_LEVEL, self._MAX_BRIGHTNESS_LEVEL]:
            await self.send_single_cmd(
                cmd=Clusters.Objects.LevelControl.Commands.MoveToLevel(
                    level=level),
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self._DIMMABLELIGHT_ENDPOINT,
            )
            asserts.assert_equal(await self._read_current_level(), level)

        self.step(3)
        before = await self._read_on_off()
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.Toggle(),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._DIMMABLELIGHT_ENDPOINT,
        )
        asserts.assert_equal(await self._read_on_off(), not before)

        self.step(4)
        asserts.assert_equal(await self._read_identify_time(), 0)
        await self.send_single_cmd(
            cmd=Clusters.Objects.Identify.Commands.Identify(
                identifyTime=5),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._DIMMABLELIGHT_ENDPOINT,
        )
        identify_time = await self._read_identify_time()
        asserts.assert_greater(identify_time, 0)
        asserts.assert_less_equal(identify_time, 5)

        self.step(5)
        device_connection = create_device_serial_or_socket_connection(
            device="",
            baudrate=self._PW_RPC_BAUD_RATE,
            token_databases=[],
            socket_addr=self._PW_RPC_SOCKET_ADDR,
            compiled_protos=[attributes_service_pb2],
            rpc_logging=True,
            channel_id=rpc.DEFAULT_CHANNEL_ID,
            hdlc_encoding=True,
            device_tracing=False,
        )

        self.step(6)
        with device_connection as device:
            # Test onOff
            self._write_on_off_pwrpc(device, True)
            asserts.assert_equal(self._read_on_off_pwrpc(device), True)
            self._write_on_off_pwrpc(device, False)
            asserts.assert_equal(self._read_on_off_pwrpc(device), False)

            # Test Level control
            for level in [self._MID_BRIGHTNESS_LEVEL, self._MIN_BRIGHTNESS_LEVEL, self._MAX_BRIGHTNESS_LEVEL]:
                self._write_current_level_pwrpc(device, level)
                asserts.assert_equal(
                    self._read_current_level_pwrpc(device), level)


if __name__ == "__main__":
    default_matter_test_main()
