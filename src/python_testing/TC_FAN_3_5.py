#
#    Copyright (c) 2023 Project CHIP Authors
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

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_3_5(MatterBaseTest):

    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_percent_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.PercentCurrent)

    async def read_speed_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedCurrent)

    async def read_speed_max(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedMax)

    async def write_percent_setting(self, endpoint, percent_setting):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.PercentSetting(percent_setting))])
        asserts.assert_equal(result[0].Status, Status.Success, "PercentSetting write failed")

    async def write_speed_setting(self, endpoint, speed_setting):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.SpeedSetting(speed_setting))])
        asserts.assert_equal(result[0].Status, Status.Success, "SpeedSetting write failed")

    async def send_step_command(self, endpoint,
                                direction: Clusters.Objects.FanControl.Enums.StepDirectionEnum,
                                wrap: bool = False, lowestOff: bool = False, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.FanControl.Commands.Step(direction=direction, wrap=wrap, lowestOff=lowestOff), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            pass

    def pics_TC_FAN_3_5(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_5(self):
        if not self.check_pics("FAN.S.F04"):
            logger.info("Test skipped because PICS FAN.S.F04 is not set")
            return

        endpoint = self.user_params.get("endpoint", 1)

        # Part 1

        self.print_step(1, "Commissioning, already done")

        # Part 2

        self.print_step("2a", "writes to the DUT the PercentSetting attribute with 50")
        await self.write_percent_setting(endpoint=endpoint, percent_setting=50)

        time.sleep(1)

        self.print_step("2b", "Read from the DUT the PercentCurrent attribute and store")
        percent_current = await self.read_percent_current(endpoint=endpoint)

        self.print_step("2c", "TH sends Step command to DUT with Direction set to Increase")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kIncrease)

        time.sleep(1)

        self.print_step("2d", "Read from the DUT the PercentCurrent attribute and check its higher than the stored value")
        percent_current_after = await self.read_percent_current(endpoint=endpoint)
        asserts.assert_greater(percent_current_after, percent_current, "PercentCurrent did not increase")

        # Part 3

        self.print_step("3a", "writes to the DUT the PercentSetting attribute with 50")
        await self.write_percent_setting(endpoint=endpoint, percent_setting=50)

        time.sleep(1)

        self.print_step("3b", "Read from the DUT the PercentCurrent attribute and store")
        percent_current = await self.read_percent_current(endpoint=endpoint)

        self.print_step("3c", "TH sends Step command to DUT with Direction set to Decrease")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kDecrease)

        self.print_step("3d", "Read from the DUT the PercentCurrent attribute and check its lower than the stored value")
        percent_current_after = await self.read_percent_current(endpoint=endpoint)
        asserts.assert_less(percent_current_after, percent_current, "PercentCurrent did not decrease")

        # Part 4
        self.print_step("4a", "TH reads from the DUT the SpeedMax attribute")
        speed_max = await self.read_speed_max(endpoint=endpoint)

        self.print_step("4b", "TH writes to the DUT the SpeedSetting attribute with the value of SpeedMax")
        await self.write_speed_setting(endpoint=endpoint, speed_setting=speed_max)

        time.sleep(1)

        self.print_step("4c", "TH sends Step command to DUT with Direction set to Increase and Wrap set to False")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kIncrease, wrap=False)

        time.sleep(1)

        self.print_step("4d", "Read from the DUT the SpeedCurrent attribute and check its equal to SpeedMax")
        speed_current = await self.read_speed_current(endpoint=endpoint)
        asserts.assert_equal(speed_current, speed_max, "SpeedCurrent incremented past SpeedMax when wrap was false")

        # Part 5
        self.print_step("5a", "TH writes to the DUT the SpeedSetting attribute with the value of SpeedMax")
        await self.write_speed_setting(endpoint=endpoint, speed_setting=speed_max)

        time.sleep(1)

        self.print_step("5b", "TH sends Step command to DUT with Direction set to Increase and Wrap set to True")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kIncrease, wrap=True)

        time.sleep(1)

        self.print_step("5c", "Read from the DUT the SpeedCurrent attribute and check its equal to 1")
        speed_current = await self.read_speed_current(endpoint=endpoint)
        asserts.assert_equal(speed_current, 1, "SpeedCurrent did not wrap to 1 when wrap was true")

        # Part 6
        self.print_step("6a", "TH writes to the DUT the SpeedSetting attribute with the value of SpeedMax")
        await self.write_speed_setting(endpoint=endpoint, speed_setting=speed_max)

        time.sleep(1)

        self.print_step("6b", "TH sends Step command to DUT with Direction set to Increase, Wrap set to True and LowestOff set to True")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kIncrease, wrap=True, lowestOff=True)

        time.sleep(1)

        self.print_step("6c", "Read from the DUT the SpeedCurrent attribute and check its equal to 0")
        speed_current = await self.read_speed_current(endpoint=endpoint)
        asserts.assert_equal(speed_current, 0, "SpeedCurrent did not wrap to 0 when wrap was true and lowestOff was true")

        # Part 7
        self.print_step("7a", "TH writes to the DUT the SpeedSetting attribute with the value of 1")
        await self.write_speed_setting(endpoint=endpoint, speed_setting=1)

        time.sleep(1)

        self.print_step("7b", "TH sends Step command to DUT with Direction set to Decrease, Wrap set to False and LowestOff set to False")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kDecrease, wrap=False, lowestOff=False)

        time.sleep(1)

        self.print_step("7c", "Read from the DUT the SpeedCurrent attribute and check its equal to 1")
        speed_current = await self.read_speed_current(endpoint=endpoint)
        asserts.assert_equal(speed_current, 1, "SpeedCurrent decremented past 1 when wrap was false and lowestOff was false")

        # Part 8
        self.print_step("8b", "TH writes to the DUT the SpeedSetting attribute with the value of 1")
        await self.write_speed_setting(endpoint=endpoint, speed_setting=1)

        time.sleep(1)

        self.print_step("8c", "TH sends Step command to DUT with Direction set to Decrease, Wrap set to True and LowestOff set to False")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kDecrease, wrap=True, lowestOff=False)

        time.sleep(1)

        self.print_step("8d", "Read from the DUT the SpeedCurrent attribute and check its equal to SpeedMax")
        speed_current = await self.read_speed_current(endpoint=endpoint)
        asserts.assert_equal(speed_current, speed_max,
                             "SpeedCurrent did not wrap to SpeedMax when wrap was true and lowestOff was false")

        # Part 9
        self.print_step("9b", "TH writes to the DUT the SpeedSetting attribute with the value of 0")
        await self.write_speed_setting(endpoint=endpoint, speed_setting=0)

        time.sleep(1)

        self.print_step("9c", "TH sends Step command to DUT with Direction set to Decrease, Wrap set to True and LowestOff set to True")
        await self.send_step_command(endpoint=endpoint, direction=Clusters.Objects.FanControl.Enums.StepDirectionEnum.kDecrease, wrap=True, lowestOff=True)

        time.sleep(1)

        self.print_step("9d", "Read from the DUT the SpeedCurrent attribute and check its equal to SpeedMax")
        speed_current = await self.read_speed_current(endpoint=endpoint)
        asserts.assert_equal(speed_current, speed_max,
                             "SpeedCurrent did not wrap to SpeedMax when wrap was true and lowestOff was true")


if __name__ == "__main__":
    default_matter_test_main()
