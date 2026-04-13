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
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_MICROWAVEOVEN(MatterBaseTest):
    """Tests for chef microwave oven device."""

    _MICROWAVEOVEN_ENDPOINT = 1
    _MODE_DEFROST = 1

    def desc_TC_MICROWAVEOVEN(self) -> str:
        return "[TC_MICROWAVEOVEN] chef microwave oven functionality test."

    def steps_TC_MICROWAVEOVEN(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "Stop microwave if running."),
                TestStep(3, "Start microwave in defrost mode for 1 minute and verify state."),
                TestStep(4, "Add more time and verify cookTime.")]

    async def _read_operational_state(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._MICROWAVEOVEN_ENDPOINT,
            cluster=Clusters.Objects.OperationalState,
            attribute=Clusters.Objects.OperationalState.Attributes.OperationalState)

    async def _read_cook_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._MICROWAVEOVEN_ENDPOINT,
            cluster=Clusters.Objects.MicrowaveOvenControl,
            attribute=Clusters.Objects.MicrowaveOvenControl.Attributes.CookTime)

    async def _read_current_mode(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._MICROWAVEOVEN_ENDPOINT,
            cluster=Clusters.Objects.MicrowaveOvenMode,
            attribute=Clusters.Objects.MicrowaveOvenMode.Attributes.CurrentMode)

    async def _read_countdown_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._MICROWAVEOVEN_ENDPOINT,
            cluster=Clusters.Objects.OperationalState,
            attribute=Clusters.Objects.OperationalState.Attributes.CountdownTime)

    @async_test_body
    async def test_TC_MICROWAVEOVEN(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        # Step 2: Stop operational cycle if running.
        op_state = await self._read_operational_state()
        if op_state != Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kStopped:
            await self.send_single_cmd(
                cmd=Clusters.Objects.OperationalState.Commands.Stop(),
                endpoint=self._MICROWAVEOVEN_ENDPOINT)

        self.step(3)
        await self.test_start_microwave_in_defrost_mode()

        self.step(4)
        await self.test_add_time()

    async def test_start_microwave_in_defrost_mode(self):
        # Step 3: Start microwave in defrost mode for 1 minute
        await self.send_single_cmd(
            cmd=Clusters.Objects.MicrowaveOvenControl.Commands.SetCookingParameters(
                cookMode=self._MODE_DEFROST,
                cookTime=60,
                startAfterSetting=True),
            endpoint=self._MICROWAVEOVEN_ENDPOINT)

        # Verify cookTime is 60.
        cook_time = await self._read_cook_time()
        asserts.assert_equal(cook_time, 60, f"Microwave Oven Control Cook Time should be 60 seconds, got {cook_time}")

        # Verify currentMode is Defrost.
        current_mode = await self._read_current_mode()
        asserts.assert_equal(current_mode, self._MODE_DEFROST,
                             f"Microwave Oven Mode should be Defrost ({self._MODE_DEFROST}), got {current_mode}")

        # Verify operationalState is Running.
        op_state = await self._read_operational_state()
        asserts.assert_equal(
            op_state, Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kRunning, "Operational State should be Running")

        # Verify countdownTime is greater than 0.
        countdown_time = await self._read_countdown_time()
        asserts.assert_greater(countdown_time, 0, "Operational State countdown time should be greater than 0")

        # Cleanup: Stop microwave if running.
        op_state = await self._read_operational_state()
        if op_state != Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kStopped:
            await self.send_single_cmd(
                cmd=Clusters.Objects.OperationalState.Commands.Stop(),
                endpoint=self._MICROWAVEOVEN_ENDPOINT)

    async def test_add_time(self):
        # Step 4: Add more time and verify cookTime.
        # Start microwave with 60 seconds
        await self.send_single_cmd(
            cmd=Clusters.Objects.MicrowaveOvenControl.Commands.SetCookingParameters(
                cookTime=60,
                startAfterSetting=True),
            endpoint=self._MICROWAVEOVEN_ENDPOINT)

        # Add 10 seconds
        await self.send_single_cmd(
            cmd=Clusters.Objects.MicrowaveOvenControl.Commands.AddMoreTime(timeToAdd=10),
            endpoint=self._MICROWAVEOVEN_ENDPOINT)

        # Verify cookTime is 70.
        cook_time = await self._read_cook_time()
        asserts.assert_equal(cook_time, 70, f"Microwave Oven Control Cook Time should be 70 seconds, got {cook_time}")

        # Cleanup: Stop microwave if running.
        op_state = await self._read_operational_state()
        if op_state != Clusters.Objects.OperationalState.Enums.OperationalStateEnum.kStopped:
            await self.send_single_cmd(
                cmd=Clusters.Objects.OperationalState.Commands.Stop(),
                endpoint=self._MICROWAVEOVEN_ENDPOINT)


if __name__ == "__main__":
    default_matter_test_main()
