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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_LAUNDRYDRYER(MatterBaseTest):
    """Tests for chef laundry dryer device."""

    _LAUNDRYDRYER_ENDPOINT = 1

    def desc_TC_LAUNDRYDRYER(self) -> str:
        return "[TC_LAUNDRYDRYER] chef laundry dryer functionality test."

    def steps_TC_LAUNDRYDRYER(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_LAUNDRY_DRYER_CONTROLS] Verification of Initial State."),
                TestStep(3, "[TC_OPERATIONAL_STATE] Stop operational cycle if running."),
                TestStep(4, "[TC_LAUNDRY_DRYER_CONTROLS] Nullable Support."),
                TestStep(5, "[TC_LAUNDRY_DRYER_CONTROLS] Constraint Error: Value Not in Supported List."),
                TestStep(6, "[TC_LAUNDRY_DRYER_CONTROLS] Constraint Error: Out-of-Bounds Enum."),
                TestStep(7, "[TC_OPERATIONAL_STATE] Operational State Start Stop."),
                TestStep(8, "[TC_LAUNDRY_WASHER_MODE] Dynamic List Update (Mode Dependency)."),
                TestStep(9, "[TC_ON_OFF] Dead Front (Off State) Behavior."),
                TestStep(10, "[TC_IDENTIFY] Test Identify."),
                TestStep(11, "[TC_TEMPERATURE_CONTROL] Test Temperature Control.")]

    async def _read_supported_dryness_levels(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryDryerControls,
            attribute=Clusters.Objects.LaundryDryerControls.Attributes.SupportedDrynessLevels)

    async def _read_selected_dryness_level(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryDryerControls,
            attribute=Clusters.Objects.LaundryDryerControls.Attributes.SelectedDrynessLevel)

    async def _write_selected_dryness_level(self, level, expect_success=True):
        return await self.write_single_attribute(
            attribute_value=Clusters.Objects.LaundryDryerControls.Attributes.SelectedDrynessLevel(value=level),
            endpoint_id=self._LAUNDRYDRYER_ENDPOINT,
            expect_success=expect_success)

    async def _read_operational_state(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.OperationalState,
            attribute=Clusters.Objects.OperationalState.Attributes.OperationalState)

    async def _read_on_off(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.OnOff,
            attribute=Clusters.Objects.OnOff.Attributes.OnOff)

    async def _read_identify_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.Identify,
            attribute=Clusters.Objects.Identify.Attributes.IdentifyTime)

    async def _read_laundry_washer_current_mode(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryWasherMode,
            attribute=Clusters.Objects.LaundryWasherMode.Attributes.CurrentMode)

    async def _read_supported_temperature_levels(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.TemperatureControl,
            attribute=Clusters.Objects.TemperatureControl.Attributes.SupportedTemperatureLevels)

    async def _read_selected_temperature_level(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.TemperatureControl,
            attribute=Clusters.Objects.TemperatureControl.Attributes.SelectedTemperatureLevel)

    async def _send_set_temperature_level_command(self, level):
        return await self.send_single_cmd(
            cmd=Clusters.Objects.TemperatureControl.Commands.SetTemperature(targetTemperatureLevel=level),
            endpoint=self._LAUNDRYDRYER_ENDPOINT)

    async def _send_on_off_command(self, on: bool):
        command = Clusters.Objects.OnOff.Commands.On() if on else Clusters.Objects.OnOff.Commands.Off()
        return await self.send_single_cmd(
            cmd=command,
            endpoint=self._LAUNDRYDRYER_ENDPOINT)

    @async_test_body
    async def test_TC_LAUNDRYDRYER(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        # Step 2: Verification of Initial State
        self.step(2)
        supported_levels = await self._read_supported_dryness_levels()
        asserts.assert_greater(len(supported_levels), 0, "SupportedDrynessLevels should not be empty")

        selected_level = await self._read_selected_dryness_level()
        if selected_level is not NullValue:
            asserts.assert_in(selected_level, supported_levels, "SelectedDrynessLevel should be in SupportedDrynessLevels")

        # Step 3: Valid Write Transaction
        self.step(3)
        # Ensure we are in a state that allows changes (Stopped)
        # OperationalStateEnum: kStopped = 0x00
        op_state = await self._read_operational_state()
        if op_state != Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kStopped:
            await self.send_single_cmd(
                cmd=Clusters.Objects.OperationalState.Commands.Stop(),
                endpoint=self._LAUNDRYDRYER_ENDPOINT)

        target_level = supported_levels[0]
        await self._write_selected_dryness_level(target_level)
        asserts.assert_equal(await self._read_selected_dryness_level(), target_level, "SelectedDrynessLevel should match target")

        # Step 4: Nullable Support
        self.step(4)
        await self._write_selected_dryness_level(NullValue)
        asserts.assert_equal(await self._read_selected_dryness_level(), NullValue, "SelectedDrynessLevel should be null")

        # Step 5: Constraint Error: Value Not in Supported List
        self.step(5)
        # Find a level that is not supported. DrynessLevelEnum has 0, 1, 2, 3.
        all_levels = [Clusters.Objects.LaundryDryerControls.Enums.DrynessLevelEnum.kLow,
                      Clusters.Objects.LaundryDryerControls.Enums.DrynessLevelEnum.kNormal,
                      Clusters.Objects.LaundryDryerControls.Enums.DrynessLevelEnum.kExtra,
                      Clusters.Objects.LaundryDryerControls.Enums.DrynessLevelEnum.kMax]
        unsupported_level = next((l for l in all_levels if l not in supported_levels), None)

        if unsupported_level is not None:
            status = await self._write_selected_dryness_level(unsupported_level, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError, "Expected ConstraintError for unsupported level")

        # Step 6: Constraint Error: Out-of-Bounds Enum
        self.step(6)
        invalid_enum_value = 5 # Values 0-3 are valid, 4 is kUnknownEnumValue
        status = await self._write_selected_dryness_level(invalid_enum_value, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError, "Expected ConstraintError for invalid enum value")

        # Step 7: Operational State Start Stop
        self.step(7)
        # Change state to Running
        await self.send_single_cmd(
            cmd=Clusters.Objects.OperationalState.Commands.Start(),
            endpoint=self._LAUNDRYDRYER_ENDPOINT)

        op_state = await self._read_operational_state()
        asserts.assert_equal(op_state, Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kRunning)

        # Reset to Stopped for next tests
        await self.send_single_cmd(
            cmd=Clusters.Objects.OperationalState.Commands.Stop(),
            endpoint=self._LAUNDRYDRYER_ENDPOINT)

        # Step 8: Dynamic List Update (Mode Dependency)
        self.step(8)
        # Change LaundryWasherMode
        supported_modes = await self.read_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryWasherMode,
            attribute=Clusters.Objects.LaundryWasherMode.Attributes.SupportedModes)

        if len(supported_modes) > 1:
            initial_supported_dryness = await self._read_supported_dryness_levels()

            # Try to find Delicates and Heavy labels
            delicates_mode = next((m.mode for m in supported_modes if "Delicate" in m.label), None)
            heavy_mode = next((m.mode for m in supported_modes if "Heavy" in m.label), None)

            if delicates_mode is not None:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.LaundryWasherMode.Commands.ChangeToMode(newMode=delicates_mode),
                    endpoint=self._LAUNDRYDRYER_ENDPOINT)
                asserts.assert_equal(await self._read_laundry_washer_current_mode(), delicates_mode, "Mode should be Delicate")
                # Check if it changes (optional but good)
                updated_dryness = await self._read_supported_dryness_levels()
                logger.info(f"Delicate mode supported dryness levels: {updated_dryness}")

            if heavy_mode is not None:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.LaundryWasherMode.Commands.ChangeToMode(newMode=heavy_mode),
                    endpoint=self._LAUNDRYDRYER_ENDPOINT)
                asserts.assert_equal(await self._read_laundry_washer_current_mode(), heavy_mode, "Mode should be Heavy")
                updated_dryness = await self._read_supported_dryness_levels()
                logger.info(f"Heavy mode supported dryness levels: {updated_dryness}")

            # If labels not found, just cycle through available modes
            if delicates_mode is None or heavy_mode is None:
                for mode_option in supported_modes:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.LaundryWasherMode.Commands.ChangeToMode(newMode=mode_option.mode),
                        endpoint=self._LAUNDRYDRYER_ENDPOINT)
                    asserts.assert_equal(await self._read_laundry_washer_current_mode(), mode_option.mode, f"Mode should be {mode_option.label}")
                    updated_supported_dryness = await self._read_supported_dryness_levels()
                    logger.info(f"Mode {mode_option.label} supports dryness levels: {updated_supported_dryness}")

        # Step 9: Dead Front (Off State) Behavior
        self.step(9)
        # Ensure OnOff is True first
        if not await self._read_on_off():
            await self._send_on_off_command(True)

        # Set OnOff to False using Off command
        await self._send_on_off_command(False)

        selected_level = await self._read_selected_dryness_level()
        # Recommended to be null
        if selected_level is not NullValue:
            logger.warning("SelectedDrynessLevel is not null in Dead Front state (recommended best effort).")

        # Step 10: Test Identify
        self.step(10)
        asserts.assert_equal(await self._read_identify_time(), 0)
        await self.send_single_cmd(
            cmd=Clusters.Objects.Identify.Commands.Identify(identifyTime=5),
            endpoint=self._LAUNDRYDRYER_ENDPOINT
        )
        identify_time = await self._read_identify_time()
        asserts.assert_greater(identify_time, 0)
        asserts.assert_less_equal(identify_time, 5)

        # Step 11: Test Temperature Control
        self.step(11)
        # Step 11.1: Discover Capabilities
        supported_temp_levels = await self._read_supported_temperature_levels()
        asserts.assert_greater(len(supported_temp_levels), 0, "SupportedTemperatureLevels should not be empty")
        logger.info(f"Supported temperature levels: {supported_temp_levels}")

        # Step 11.2: Read Initial State
        initial_temp_level = await self._read_selected_temperature_level()
        logger.info(f"Initial temperature level: {initial_temp_level}")

        # Step 11.3: Execute Valid Change
        # Try to change to a different level if possible
        target_temp_level = (initial_temp_level + 1) % len(supported_temp_levels)
        await self._send_set_temperature_level_command(target_temp_level)

        current_temp_level = await self._read_selected_temperature_level()
        asserts.assert_equal(current_temp_level, target_temp_level, "SelectedTemperatureLevel should match target")

        # Step 11.4: Execute Invalid Change
        invalid_temp_level = len(supported_temp_levels) + 1
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.TemperatureControl.Commands.SetTemperature(targetTemperatureLevel=invalid_temp_level),
                endpoint=self._LAUNDRYDRYER_ENDPOINT
            )
            asserts.fail("Expected ConstraintError for invalid temperature level but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Expected ConstraintError for invalid temperature level")

        # Verify it remained unchanged
        current_temp_level = await self._read_selected_temperature_level()
        asserts.assert_equal(current_temp_level, target_temp_level, "SelectedTemperatureLevel should remain unchanged after invalid write")

if __name__ == "__main__":
    default_matter_test_main()
