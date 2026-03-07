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

import logging
import time

from attributes_service import attributes_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_WATERVALVE(MatterBaseTest):
    """Tests for chef watervalve device."""

    ENDPOINT = 1

    _PW_RPC_SOCKET_ADDR = "0.0.0.0:33000"
    _PW_RPC_BAUD_RATE = 115200

    def desc_TC_WATERVALVE(self) -> str:
        return "[TC_WATERVALVE] chef watervalve functionality test."

    def steps_TC_WATERVALVE(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "Test identify cluster."),
                TestStep(3, "Set up PwRPC connection."),
                TestStep(4, "Test remaining clusters (Flow Measurement and Valve Configuration and Control).")]

    async def _read_identify_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.Identify,
            attribute=Clusters.Objects.Identify.Attributes.IdentifyTime)

    async def flow_measurement_test(self):
        cluster = Clusters.Objects.FlowMeasurement
        attributes = cluster.Attributes

        measured_value = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.MeasuredValue)
        asserts.assert_equal(measured_value, NullValue, "MeasuredValue should be NULL")

        min_measured_value = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.MinMeasuredValue)
        asserts.assert_equal(min_measured_value, 0, "MinMeasuredValue should be 0")

        max_measured_value = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.MaxMeasuredValue)
        asserts.assert_equal(max_measured_value, 100, "MaxMeasuredValue should be 100")

        tolerance = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.Tolerance)
        asserts.assert_equal(tolerance, 0, "Tolerance should be 0")

    def _write_current_level_pwrpc(self, device, level: int):
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_uint8=level),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=self.ENDPOINT,
                cluster=Clusters.Objects.ValveConfigurationAndControl.id,
                attribute_id=Clusters.Objects.ValveConfigurationAndControl.Attributes.CurrentLevel.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_INT8U_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    async def valve_configuration_and_control_test(self, device):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        attributes = cluster.Attributes

        # 4.2.1: Check attributes are all NULL initially.
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.OpenDuration), NullValue, "OpenDuration should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.DefaultOpenDuration), NullValue, "DefaultOpenDuration should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.AutoCloseTime), NullValue, "AutoCloseTime should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.RemainingDuration), NullValue, "RemainingDuration should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentState), NullValue, "CurrentState should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetState), NullValue, "TargetState should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentLevel), NullValue, "CurrentLevel should be NULL")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetLevel), NullValue, "TargetLevel should be NULL")

        # 4.2.1 (again): Send an Open command with openDuration=15 and targetLevel=50.
        await self.send_single_cmd(
            cmd=cluster.Commands.Open(openDuration=15, targetLevel=50),
            endpoint=self.ENDPOINT
        )

        # 4.2.2: Check openDuration attribute value is 15.
        open_duration = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.OpenDuration)
        asserts.assert_equal(open_duration, 15, "OpenDuration should be 15")

        # 4.2.3: Check AutoCloseTime and RemainingDuration are non-NULL and > 0.
        auto_close_time = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.AutoCloseTime)
        asserts.assert_is_not_none(auto_close_time, "AutoCloseTime should not be NULL")
        asserts.assert_greater(auto_close_time, 0, "AutoCloseTime should be > 0")

        remaining_duration = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.RemainingDuration)
        asserts.assert_is_not_none(remaining_duration, "RemainingDuration should not be NULL")
        asserts.assert_greater(remaining_duration, 0, "RemainingDuration should be > 0")

        # 4.2.4: Check current state is kTransitioning (2)
        current_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentState)
        asserts.assert_equal(current_state, cluster.Enums.ValveStateEnum.kTransitioning, "CurrentState should be kTransitioning")

        # 4.2.5: Check target state is kOpen (1)
        target_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetState)
        asserts.assert_equal(target_state, cluster.Enums.ValveStateEnum.kOpen, "TargetState should be kOpen")

        # 4.3.6: Check target level is 50.
        target_level = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetLevel)
        asserts.assert_equal(target_level, 50, "TargetLevel should be 50")

        # 4.3.7: Send Pw command to update currentLevel to 50.
        self._write_current_level_pwrpc(device, 50)

        # 4.3.8: Read current level and check it is 50.
        current_level = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentLevel)
        asserts.assert_equal(current_level, 50, "CurrentLevel should be 50")

        # 4.3.9: Read target level and check it is NULL.
        target_level = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetLevel)
        asserts.assert_equal(target_level, NullValue, "TargetLevel should be NULL after reaching target")

        # 4.3.10: Read current state and check it is kOpen (1).
        current_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentState)
        asserts.assert_equal(current_state, cluster.Enums.ValveStateEnum.kOpen, "CurrentState should be kOpen")

        # 4.3.11: Read target state and check it is NULL.
        target_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetState)
        asserts.assert_equal(target_state, NullValue, "TargetState should be NULL after reaching target")

        # 4.3.12: Now send Close command.
        await self.send_single_cmd(
            cmd=cluster.Commands.Close(),
            endpoint=self.ENDPOINT
        )

        # 4.3.13: Check openDuration is NULL, RemainingDuration is NULL, AutoCloseTime is NULL.
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.OpenDuration), NullValue, "OpenDuration should be NULL after Close")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.RemainingDuration), NullValue, "RemainingDuration should be NULL after Close")
        asserts.assert_equal(await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.AutoCloseTime), NullValue, "AutoCloseTime should be NULL after Close")

        # 4.3.14: Check current state is kTransitioning (2) and Check target state is kClosed (0), and targetLevel is 0.
        current_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentState)
        asserts.assert_equal(current_state, cluster.Enums.ValveStateEnum.kTransitioning, "CurrentState should be kTransitioning")

        target_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetState)
        asserts.assert_equal(target_state, cluster.Enums.ValveStateEnum.kClosed, "TargetState should be kClosed")

        target_level = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetLevel)
        asserts.assert_equal(target_level, 0, "TargetLevel should be 0 for Close")

        # 4.3.15: Send Pw command to update currentLevel to 0.
        self._write_current_level_pwrpc(device, 0)

        # 4.3.16: Read current level and check it is 0.
        current_level = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentLevel)
        asserts.assert_equal(current_level, 0, "CurrentLevel should be 0")

        # 4.3.17: Read target level and check it is NULL.
        target_level = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TargetLevel)
        asserts.assert_equal(target_level, NullValue, "TargetLevel should be NULL after reaching 0")

        # 4.3.18: Read current state and check it is kClosed (0).
        current_state = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentState)
        asserts.assert_equal(current_state, cluster.Enums.ValveStateEnum.kClosed, "CurrentState should be kClosed")

    @async_test_body
    async def test_TC_WATERVALVE(self):
        self.step(1)
        # Commissioning already done.

        self.step(2)
        asserts.assert_equal(await self._read_identify_time(), 0)
        await self.send_single_cmd(
            cmd=Clusters.Objects.Identify.Commands.Identify(identifyTime=5),
            endpoint=self.ENDPOINT
        )
        identify_time = await self._read_identify_time()
        asserts.assert_greater(identify_time, 0)
        asserts.assert_less_equal(identify_time, 5)

        self.step(3)
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

        self.step(4)
        with device_connection as device:
            await self.flow_measurement_test()
            await self.valve_configuration_and_control_test(device)


if __name__ == "__main__":
    default_matter_test_main()
