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

    INITIAL_OCCUPIED_HEATING_SETPOINT = 5500
    TANK_VOLUME = 150
    INITIAL_TANK_PERCENTAGE = 50
    MIN_BOOST_DURATION = 60

    def desc_TC_WATERHEATER(self) -> str:
        return "[TC_WATERHEATER] chef waterheater functionality test."

    def steps_TC_WATERHEATER(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "Test identify cluster."),
                TestStep(3, "Test water heater mode cluster."),
                TestStep(4, "Set up PwRPC connection."),
                TestStep(5, "PwRPC tests: Tests that use at least 1 PwRPC call.")]

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
        asserts.assert_equal(val, self.INITIAL_OCCUPIED_HEATING_SETPOINT,
                             f"OccupiedHeatingSetpoint initial value should be {self.INITIAL_OCCUPIED_HEATING_SETPOINT}")

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
        # 1. Use Pigweed to set ThermostatRunningState to Heat.
        self._write_thermostat_running_state_pwrpc(device, cluster.Bitmaps.RelayStateBitmap.kHeat)

        # 2. Read ThermostatRunningState and check it is Heat.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.ThermostatRunningState)
        asserts.assert_equal(val, cluster.Bitmaps.RelayStateBitmap.kHeat,
                             f"ThermostatRunningState should be {cluster.Bitmaps.RelayStateBitmap.kHeat}")

    async def water_heater_management_test(self, device):
        cluster = Clusters.Objects.WaterHeaterManagement
        attributes = cluster.Attributes

        # 1. HeaterTypes (read-only attribute) - verify its default value.
        expected_heater_types = cluster.Bitmaps.WaterHeaterHeatSourceBitmap.kImmersionElement1 | cluster.Bitmaps.WaterHeaterHeatSourceBitmap.kHeatPump
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.HeaterTypes)
        asserts.assert_equal(val, expected_heater_types, f"HeaterTypes initial value should be {expected_heater_types}")

        # 2. TankVolume (read-only attribute) - verify its default value.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TankVolume)
        asserts.assert_equal(val, self.TANK_VOLUME, "TankVolume initial value should be 150")

        # 3. TankPercentage (read-only attribute) - verify its default value.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.TankPercentage)
        asserts.assert_equal(val, self.INITIAL_TANK_PERCENTAGE, "TankPercentage initial value should be 50")

        # 4. BoostState (read-only attribute) - verify it starts as kInactive (0).
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.BoostState)
        asserts.assert_equal(val, cluster.Enums.BoostStateEnum.kInactive, "BoostState initial value should be kInactive")

        # 5. Boost command:
        # - Call Boost with duration < MIN_BOOST_DURATION, should fail with ConstraintError.
        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.Boost(boostInfo=cluster.Structs.WaterHeaterBoostInfoStruct(
                    duration=self.MIN_BOOST_DURATION - 1)),
                endpoint=self.ENDPOINT
            )
            asserts.fail("Boost with duration < 60 should have failed")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Should be ConstraintError")

        # - Call Boost with duration >= MIN_BOOST_DURATION, should succeed.
        await self.send_single_cmd(
            cmd=cluster.Commands.Boost(boostInfo=cluster.Structs.WaterHeaterBoostInfoStruct(duration=self.MIN_BOOST_DURATION)),
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
        # Expected: 435625
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.EstimatedHeatRequired)
        asserts.assert_equal(val, 435625, f"EstimatedHeatRequired should be 435625, got {val}")

        # 8. EstimatedHeatRequired calculation (Zero case):
        self._write_thermostat_local_temperature_pwrpc(device, 3000)  # 30C
        # OccupiedHeatingSetpoint is 2500.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.EstimatedHeatRequired)
        asserts.assert_equal(val, 0, "EstimatedHeatRequired should be 0 when temperature is above setpoint")

    async def water_heater_mode_test(self):
        cluster = Clusters.Objects.WaterHeaterMode
        attributes = cluster.Attributes

        # 0. FeatureMap (read-only attribute) - verify its default value.
        val = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.FeatureMap)
        asserts.assert_equal(val, 0, "WaterHeaterMode FeatureMap initial value should be 0")

        # 1. Read SupportedModes
        supported_modes = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.SupportedModes)

        expected_supported_modes = [
            cluster.Structs.ModeOptionStruct(
                label="Off", mode=0, modeTags=[cluster.Structs.ModeTagStruct(value=cluster.Enums.ModeTag.kOff)]),
            cluster.Structs.ModeOptionStruct(
                label="Manual", mode=1, modeTags=[cluster.Structs.ModeTagStruct(value=cluster.Enums.ModeTag.kManual)]),
        ]
        asserts.assert_equal(supported_modes, expected_supported_modes, "SupportedModes should match expected")

        # 2. Read CurrentMode
        current_mode = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentMode)

        # 3. Change to Mode Manual
        await self.send_single_cmd(
            cmd=cluster.Commands.ChangeToMode(newMode=1),
            endpoint=self.ENDPOINT
        )

        # 4. Verify CurrentMode is Manual
        current_mode = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentMode)
        asserts.assert_equal(current_mode, 1, "CurrentMode should be 1 (Manual)")

        # 5. Change to Mode 0 (Off)
        await self.send_single_cmd(
            cmd=cluster.Commands.ChangeToMode(newMode=0),
            endpoint=self.ENDPOINT
        )

        # 6. Verify CurrentMode is Off
        current_mode = await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT, cluster=cluster, attribute=attributes.CurrentMode)
        asserts.assert_equal(current_mode, 0, "CurrentMode should be 0 (Off)")

        # 7. Try an unsupported mode and verify it returns UnsupportedMode
        response = await self.send_single_cmd(
            cmd=cluster.Commands.ChangeToMode(newMode=2),
            endpoint=self.ENDPOINT
        )
        asserts.assert_not_equal(response.status, Status.Success, "Status should not be Success.")

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

        # Step 3: Test water heater mode cluster.
        self.step(3)
        await self.water_heater_mode_test()

        # Step 4: Set up PwRPC connection.
        self.step(4)
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

        # Step 5: PwRPC tests. All tests using PwRPC calls.
        self.step(5)
        with device_connection as device:
            await self.thermostat_test(device)
            await self.water_heater_management_test(device)


if __name__ == "__main__":
    default_matter_test_main()
