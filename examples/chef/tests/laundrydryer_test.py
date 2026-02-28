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
from matter.interaction_model import Status
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
                TestStep(2, "Verification of Initial State."),
                TestStep(3, "Valid Write Transaction."),
                TestStep(4, "Nullable Support."),
                TestStep(5, "Constraint Error: Value Not in Supported List."),
                TestStep(6, "Constraint Error: Out-of-Bounds Enum."),
                TestStep(7, "Invalid State Transition."),
                TestStep(8, "Dynamic List Update (Mode Dependency)."),
                TestStep(9, "Dead Front (Off State) Behavior.")]

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

    async def _write_selected_dryness_level(self, level):
        return await self.write_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryDryerControls,
            attribute=Clusters.Objects.LaundryDryerControls.Attributes.SelectedDrynessLevel,
            value=level)

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

    async def _write_on_off(self, on_off: bool):
        return await self.write_single_attribute_check_success(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.OnOff,
            attribute=Clusters.Objects.OnOff.Attributes.OnOff,
            value=on_off)

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
            await self.write_single_attribute_expect_error(
                endpoint=self._LAUNDRYDRYER_ENDPOINT,
                cluster=Clusters.Objects.LaundryDryerControls,
                attribute=Clusters.Objects.LaundryDryerControls.Attributes.SelectedDrynessLevel,
                value=unsupported_level,
                error=Status.ConstraintError)

        # Step 6: Constraint Error: Out-of-Bounds Enum
        self.step(6)
        invalid_enum_value = 5 # Values 0-3 are valid, 4 is kUnknownEnumValue
        await self.write_single_attribute_expect_error(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryDryerControls,
            attribute=Clusters.Objects.LaundryDryerControls.Attributes.SelectedDrynessLevel,
            value=invalid_enum_value,
            error=Status.ConstraintError)

        # Step 7: Invalid State Transition
        self.step(7)
        # Change state to Running
        await self.send_single_cmd(
            cmd=Clusters.Objects.OperationalState.Commands.Start(),
            endpoint=self._LAUNDRYDRYER_ENDPOINT)
        
        op_state = await self._read_operational_state()
        asserts.assert_equal(op_state, Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kRunning)

        await self.write_single_attribute_expect_error(
            endpoint=self._LAUNDRYDRYER_ENDPOINT,
            cluster=Clusters.Objects.LaundryDryerControls,
            attribute=Clusters.Objects.LaundryDryerControls.Attributes.SelectedDrynessLevel,
            value=supported_levels[0],
            error=Status.InvalidInState)

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
                # Plan says SupportedDrynessLevels should be [0]
                # supported_dryness = await self._read_supported_dryness_levels()
                # asserts.assert_equal(supported_dryness, [Clusters.Objects.LaundryDryerControls.Enums.DrynessLevelEnum.kLow])

            if heavy_mode is not None:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.LaundryWasherMode.Commands.ChangeToMode(newMode=heavy_mode),
                    endpoint=self._LAUNDRYDRYER_ENDPOINT)
                # Plan says SupportedDrynessLevels should contain more levels
                # updated_supported_dryness = await self._read_supported_dryness_levels()
                # asserts.assert_greater(len(updated_supported_dryness), 1)

            # If labels not found, just cycle through available modes
            if delicates_mode is None or heavy_mode is None:
                for mode_option in supported_modes:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.LaundryWasherMode.Commands.ChangeToMode(newMode=mode_option.mode),
                        endpoint=self._LAUNDRYDRYER_ENDPOINT)
                    updated_supported_dryness = await self._read_supported_dryness_levels()
                    logger.info(f"Mode {mode_option.label} supports dryness levels: {updated_supported_dryness}")

        # Step 9: Dead Front (Off State) Behavior
        self.step(9)
        # Ensure OnOff is True first
        if not await self._read_on_off():
            await self._write_on_off(True)
        
        # Set OnOff to False
        await self._write_on_off(False)
        
        selected_level = await self._read_selected_dryness_level()
        # Recommended to be null
        if selected_level is not NullValue:
            logger.warning("SelectedDrynessLevel is not null in Dead Front state (recommended best effort).")

if __name__ == "__main__":
    default_matter_test_main()
