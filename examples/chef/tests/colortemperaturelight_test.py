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

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_COLORTEMPERATURELIGHT(MatterBaseTest):
    """Tests for chef colortemperaturelight device."""

    _COLORTEMPERATURELIGHT_ENDPOINT = 1

    async def _read_remaining_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._COLORTEMPERATURELIGHT_ENDPOINT, cluster=Clusters.Objects.ColorControl, attribute=Clusters.Objects.ColorControl.Attributes.RemainingTime)

    def desc_TC_COLORTEMPERATURELIGHT(self) -> str:
        return "[TC_COLORTEMPERATURELIGHT] chef colortemperaturelight functionality test."

    def steps_TC_COLORTEMPERATURELIGHT(self):
        return [TestStep(1, "[TC_COLORTEMPERATURELIGHT] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_COLORTEMPERATURELIGHT] Test move-color-temperature command.")]

    @async_test_body
    async def test_TC_COLORTEMPERATURELIGHT(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        # Check no transition in progress.
        asserts.assert_equal(await self._read_remaining_time(), 0)
        await self.send_single_cmd(
            cmd=Clusters.Objects.ColorControl.Commands.MoveColorTemperature(
                moveMode=Clusters.Objects.ColorControl.Enums.MoveModeEnum.kUp,
                rate=1,  # 1 step per second
                colorTemperatureMinimumMireds=0,  # default to ColorTempPhysicalMinMireds
                colorTemperatureMaximumMireds=0,  # default to ColorTempPhysicalMaxMireds
                optionsMask=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
                optionsOverride=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
            ),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._COLORTEMPERATURELIGHT_ENDPOINT,
        )  # Start transition
        # Check transition started
        asserts.assert_greater(await self._read_remaining_time(), 0)
        await self.send_single_cmd(
            cmd=Clusters.Objects.ColorControl.Commands.StopMoveStep(
                optionsMask=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
                optionsOverride=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
            ),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._COLORTEMPERATURELIGHT_ENDPOINT,
        )  # Stop transition
        # Check transition stopped
        asserts.assert_equal(await self._read_remaining_time(), 0)


if __name__ == "__main__":
    default_matter_test_main()
