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

import time
import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_VCC_4_2(MatterBaseTest):
    async def read_vcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VCC_4_2(self) -> str:
        return "[TC-VCC-4.2] DefaultOpenDuration functionality with DUT as Server"

    def steps_TC_VCC_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read DefaultOpenDuration attribute"),
            TestStep(3, "Send Open command"),
            TestStep(4, "Read OpenDuration attribute"),
            TestStep(5, "Read RemainingDuration attribute"),
            TestStep(6, "Wait for 5 seconds"),
            TestStep(7, "Read RemainingDuration attribute"),
            TestStep(8, "Send Close command"),
            TestStep(9, "Read OpenDuration attribute"),
            TestStep(10, "Read RemainingDuration attribute"),
        ]
        return steps

    def pics_TC_VCC_4_2(self) -> list[str]:
        pics = [
            "VCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VCC_4_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        defaultOpenDuration = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)

        self.step(3)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(openDuration=60), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(4)
        open_duration_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)
        asserts.assert_true(open_duration_dut is not NullValue, "OpenDuration is null")
        asserts.assert_equal(open_duration_dut, defaultOpenDuration, "OpenDuration is not the expected value")

        self.step(5)
        remaining_duration_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)
        asserts.assert_true(remaining_duration_dut is not NullValue, "RemainingDuration is null")
        asserts.assert_greater_equal(remaining_duration_dut, (defaultOpenDuration - 5),
                                     "RemainingDuration is not in the expected range")
        asserts.assert_less_equal(remaining_duration_dut, defaultOpenDuration, "RemainingDuration is not in the expected range")

        self.step(6)
        time.sleep(5)

        self.step(7)
        remaining_duration_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)
        asserts.assert_true(remaining_duration_dut is not NullValue, "RemainingDuration is null")
        asserts.assert_greater_equal(remaining_duration_dut, (defaultOpenDuration - 10),
                                     "RemainingDuration is not in the expected range")
        asserts.assert_less_equal(remaining_duration_dut, (defaultOpenDuration - 5),
                                  "RemainingDuration is not in the expected range")

        self.step(8)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(9)
        open_duration_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)
        asserts.assert_true(open_duration_dut is NullValue, "OpenDuration is not null")

        self.step(10)
        remaining_duration_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)
        asserts.assert_true(remaining_duration_dut is NullValue, "RemainingDuration is not null")


if __name__ == "__main__":
    default_matter_test_main()
