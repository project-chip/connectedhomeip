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


class TC_DIMMABLELIGHT(MatterBaseTest):
    """Tests for chef dimmablelight device."""

    _DIMMABLELIGHT_ENDPOINT = 1
    _MAX_BRIGHTNESS_LEVEL = 254
    _MID_BRIGHTNESS_LEVEL = 127
    _MIN_BRIGHTNESS_LEVEL = 1

    async def _read_on_off(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.OnOff, attribute=Clusters.Objects.OnOff.Attributes.OnOff)

    async def _read_current_level(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.LevelControl, attribute=Clusters.Objects.LevelControl.Attributes.CurrentLevel)

    def desc_TC_DIMMABLELIGHT(self) -> str:
        return "[TC_DIMMABLELIGHT] chef dimmablelight functionality test."

    def steps_TC_DIMMABLELIGHT(self):
        return [TestStep(1, "[TC_DIMMABLELIGHT] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_DIMMABLELIGHT] Test level control."),
                TestStep(3, "[TC_DIMMABLELIGHT] Test toggle.")]

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
        for level in [self._MIN_BRIGHTNESS_LEVEL, self._MID_BRIGHTNESS_LEVEL, self._MAX_BRIGHTNESS_LEVEL]:
            await self.send_single_cmd(
                cmd=Clusters.Objects.LevelControl.Commands.MoveToLevel(level=level),
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


if __name__ == "__main__":
    default_matter_test_main()
