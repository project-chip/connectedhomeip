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

from attributes_service import attributes_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_WATERHEATER(MatterBaseTest):
    """Tests for chef waterheater device."""

    ENDPOINT = 1

    _PW_RPC_SOCKET_ADDR = "0.0.0.0:33000"
    _PW_RPC_BAUD_RATE = 115200

    def desc_TC_WATERHEATER(self) -> str:
        return "[TC_WATERHEATER] chef waterheater functionality test."

    def steps_TC_WATERHEATER(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "Test identify cluster."),
                TestStep(3, "Set up PwRPC connection."),
                TestStep(4, "PwRPC tests.")]

    async def _read_identify_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.Identify,
            attribute=Clusters.Objects.Identify.Attributes.IdentifyTime)

    def _write_thermostat_local_temperature_pwrpc(self, device, value: int):
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_int16=value),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=self.ENDPOINT,
                cluster=Clusters.Objects.Thermostat.id,
                attribute_id=Clusters.Objects.Thermostat.Attributes.LocalTemperature.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_INT16S_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    def _write_thermostat_running_state_pwrpc(self, device, value: int):
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_uint16=value),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=self.ENDPOINT,
                cluster=Clusters.Objects.Thermostat.id,
                attribute_id=Clusters.Objects.Thermostat.Attributes.ThermostatRunningState.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_BITMAP16_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    async def thermostat_test(self, device):
        cluster = Clusters.Objects.Thermostat
        attributes = cluster.Attributes

        # Test SetpointRaiseLower command
        # 1. Read OccupiedHeatingSetpoint (0x0012) via Client (Initial Value: 5500).
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.OccupiedHeatingSetpoint)
        asserts.assert_equal(val, 5500, "OccupiedHeatingSetpoint initial value should be 5500")

        # 2. Send SetpointRaiseLower command with Amount = 70 (Increases by 7.0 C).
        await self.send_single_cmd(
            cmd=cluster.Commands.SetpointRaiseLower(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=70),
            endpoint=self.ENDPOINT
        )

        # 3. Read OccupiedHeatingSetpoint. Value must be 6200.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.OccupiedHeatingSetpoint)
        asserts.assert_equal(val, 6200, "OccupiedHeatingSetpoint should be 6200 after raise by 70")

        # Tests LocalTemperature
        # 1. Use Pigweed to set LocalTemperature to 3000 (30 C).
        self._write_thermostat_local_temperature_pwrpc(device, 3000)

        # 2. Read LocalTemperature and check it is 30 C.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.LocalTemperature)
        asserts.assert_equal(val, 3000, "LocalTemperature should be 3000")

        # Tests ThermostatRunningState
        # 1. Use Pigweed to set ThermostatRunningState to 1 (Heat).
        self._write_thermostat_running_state_pwrpc(device, 1)

        # 2. Read ThermostatRunningState and check it is 1.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.ThermostatRunningState)
        asserts.assert_equal(val, 1, "ThermostatRunningState should be 1")

    async def water_heater_management_test(self, device):
        cluster = Clusters.Objects.WaterHeaterManagement
        attributes = cluster.Attributes

        # 1. HeaterTypes (read-only attribute) - verify its default value.
        # mHeaterTypes = kImmersionElement1 | kHeatPump = (1 << 0) | (1 << 3) = 9
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.HeaterTypes)
        asserts.assert_equal(val, 5, "HeaterTypes initial value should be 9")

        # 2. TankVolume (read-only attribute) - verify its default value.
        # mTankVolume = 150
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TankVolume)
        asserts.assert_equal(val, 150, "TankVolume initial value should be 150")

        # 3. TankPercentage (read-only attribute) - verify its default value.
        # mTankPercentage = 50
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TankPercentage)
        asserts.assert_equal(val, 50, "TankPercentage initial value should be 50")

        # 4. BoostState (read-only attribute) - verify it starts as kInactive (0).
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.BoostState)
        asserts.assert_equal(val, cluster.Enums.BoostStateEnum.kInactive, "BoostState initial value should be kInactive")

        # 5. Boost command:
        # - Call Boost with duration < 60, should fail with ConstraintError.
        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.Boost(duration=30),
                endpoint=self.ENDPOINT
            )
            asserts.fail("Boost with duration < 60 should have failed")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Should be ConstraintError")

        # - Call Boost with duration >= 60, should succeed.
        await self.send_single_cmd(
            cmd=cluster.Commands.Boost(duration=100),
            endpoint=self.ENDPOINT
        )

        # - Verify BoostState is now kActive (1).
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.BoostState)
        asserts.assert_equal(val, cluster.Enums.BoostStateEnum.kActive, "BoostState should be kActive after Boost command")

        # 6. CancelBoost command:
        # - Call CancelBoost, should succeed.
        await self.send_single_cmd(
            cmd=cluster.Commands.CancelBoost(),
            endpoint=self.ENDPOINT
        )

        # - Verify BoostState is now kInactive (0).
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.BoostState)
        asserts.assert_equal(val, cluster.Enums.BoostStateEnum.kInactive,
                             "BoostState should be kInactive after CancelBoost command")

        # 7. EstimatedHeatRequired calculation:
        # Use Pigweed to set Thermostat local temperature
        self._write_thermostat_local_temperature_pwrpc(device, 2000)  # 20C

        # Set OccupiedHeatingSetpoint to 2500 (25C)
        await self.write_single_attribute(
            attribute_value=Clusters.Objects.Thermostat.Attributes.OccupiedHeatingSetpoint(2500),
            endpoint_id=self.ENDPOINT
        )

        # Now read EstimatedHeatRequired.
        # Expected: 43562
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.EstimatedHeatRequired)
        asserts.assert_equal(val, 43562, f"EstimatedHeatRequired should be 43562, got {val}")

        # 8. EstimatedHeatRequired calculation (Zero case):
        self._write_thermostat_local_temperature_pwrpc(device, 3000)  # 30C
        # OccupiedHeatingSetpoint is 2500.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.EstimatedHeatRequired)
        asserts.assert_equal(val, 0, "EstimatedHeatRequired should be 0 when temperature is above setpoint")

    @async_test_body
    async def test_TC_WATERHEATER(self):
        # Step 1: Commissioning already done.
        self.step(1)

        # Step 2: Test identify cluster.
        self.step(2)
        identify_time = await self._read_identify_time()
        asserts.assert_equal(identify_time, 0, "IdentifyTime should be 0")
        await self.send_single_cmd(
            cmd=Clusters.Objects.Identify.Commands.Identify(
                identifyTime=5),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.ENDPOINT,
        )
        identify_time = await self._read_identify_time()
        asserts.assert_greater(identify_time, 0)
        asserts.assert_less_equal(identify_time, 5)

        # Step 3: Set up PwRPC connection.
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

        # Step 4: PwRPC tests. All tests using PwRPC calls.
        self.step(4)
        with device_connection as device:
            await self.thermostat_test(device)
            await self.water_heater_management_test(device)


if __name__ == "__main__":
    default_matter_test_main()
