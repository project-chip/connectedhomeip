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


class TC_FANCONTROL(MatterBaseTest):
    """Tests for FanControl cluster."""

    def desc_TC_FANCONTROL(self) -> str:
        return "[TC_FANCONTROL] FanControl cluster functionality test."

    def steps_TC_FANCONTROL(self):
        return [
            TestStep(1, "Commissioning already done.", is_commissioning=True),
            TestStep(2, "Read initial speed and percents."),
            TestStep(3, "Increase Speed by 1 and read speeds and percents."),
            TestStep(4, "Increase Speed by 1 again and read speeds and percents."),
            TestStep(5, "Decrease Speed by 1 and read speeds and percents.")
        ]

    async def _read_fan_control_attribute(self, endpoint, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.FanControl,
            attribute=attribute
        )

    @async_test_body
    async def test_TC_FANCONTROL(self):
        endpoint = self.user_params.get(
            "fan_endpoint", self.matter_test_config.endpoint)
        if endpoint is None:
            endpoint = 1
        logger.info(f"Testing FanControl on endpoint {endpoint}")

        self.step(1)
        # Commissioning already done.

        # Read FeatureMap to check for MultiSpeed support
        feature_map = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.FeatureMap)
        has_multi_speed = (
            feature_map & Clusters.Objects.FanControl.Bitmaps.Feature.kMultiSpeed) != 0
        logger.info(
            f"FeatureMap: {feature_map}, MultiSpeed supported: {has_multi_speed}")

        self.step(2)
        # Read initial speed and percents
        speed_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedSetting)
        speed_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedCurrent)
        percent_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentSetting)
        percent_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentCurrent)

        logger.info(f"Initial SpeedSetting: {speed_setting}")
        logger.info(f"Initial SpeedCurrent: {speed_current}")
        logger.info(f"Initial PercentSetting: {percent_setting}")
        logger.info(f"Initial PercentCurrent: {percent_current}")

        asserts.assert_equal(
            speed_setting, 0, "Initial SpeedSetting should be 0")
        asserts.assert_equal(
            speed_current, 0, "Initial SpeedCurrent should be 0")
        if has_multi_speed:
            asserts.assert_equal(percent_setting, 0,
                                 "Initial PercentSetting should be 0")
            asserts.assert_equal(percent_current, 0,
                                 "Initial PercentCurrent should be 0")

        self.step(3)
        # Increase Speed by 1
        await self.send_single_cmd(
            cmd=Clusters.Objects.FanControl.Commands.Step(
                direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kIncrease,
                wrap=False,
                lowestOff=False
            ),
            endpoint=endpoint
        )

        speed_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedSetting)
        speed_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedCurrent)
        percent_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentSetting)
        percent_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentCurrent)

        logger.info(f"SpeedSetting: {speed_setting}")
        logger.info(f"SpeedCurrent: {speed_current}")
        logger.info(f"PercentSetting: {percent_setting}")
        logger.info(f"PercentCurrent: {percent_current}")

        asserts.assert_equal(
            speed_setting, 1, "SpeedSetting should be 1 after increase")
        asserts.assert_equal(
            speed_current, 1, "SpeedCurrent should be 1 after increase")
        if has_multi_speed:
            asserts.assert_equal(percent_setting, 10,
                                 "PercentSetting should be 10 after increase")
            asserts.assert_equal(percent_current, 10,
                                 "PercentCurrent should be 10 after increase")

        self.step(4)
        # Increase Speed by 1 again
        await self.send_single_cmd(
            cmd=Clusters.Objects.FanControl.Commands.Step(
                direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kIncrease,
                wrap=False,
                lowestOff=False
            ),
            endpoint=endpoint
        )

        speed_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedSetting)
        speed_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedCurrent)
        percent_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentSetting)
        percent_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentCurrent)

        logger.info(f"SpeedSetting: {speed_setting}")
        logger.info(f"SpeedCurrent: {speed_current}")
        logger.info(f"PercentSetting: {percent_setting}")
        logger.info(f"PercentCurrent: {percent_current}")

        asserts.assert_equal(
            speed_setting, 2, "SpeedSetting should be 2 after 2nd increase")
        asserts.assert_equal(
            speed_current, 2, "SpeedCurrent should be 2 after 2nd increase")
        if has_multi_speed:
            asserts.assert_equal(
                percent_setting, 20, "PercentSetting should be 20 after 2nd increase")
            asserts.assert_equal(
                percent_current, 20, "PercentCurrent should be 20 after 2nd increase")

        self.step(5)
        # Decrease Speed by 1
        await self.send_single_cmd(
            cmd=Clusters.Objects.FanControl.Commands.Step(
                direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kDecrease,
                wrap=False,
                lowestOff=False
            ),
            endpoint=endpoint
        )

        speed_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedSetting)
        speed_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.SpeedCurrent)
        percent_setting = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentSetting)
        percent_current = await self._read_fan_control_attribute(endpoint, Clusters.Objects.FanControl.Attributes.PercentCurrent)

        logger.info(f"SpeedSetting: {speed_setting}")
        logger.info(f"SpeedCurrent: {speed_current}")
        logger.info(f"PercentSetting: {percent_setting}")
        logger.info(f"PercentCurrent: {percent_current}")

        asserts.assert_equal(
            speed_setting, 1, "SpeedSetting should be 1 after decrease")
        asserts.assert_equal(
            speed_current, 1, "SpeedCurrent should be 1 after decrease")
        if has_multi_speed:
            asserts.assert_equal(percent_setting, 10,
                                 "PercentSetting should be 10 after decrease")
            asserts.assert_equal(percent_current, 10,
                                 "PercentCurrent should be 10 after decrease")


if __name__ == "__main__":
    default_matter_test_main()
