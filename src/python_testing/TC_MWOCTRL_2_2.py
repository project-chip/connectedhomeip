#
#    Copyright (c) 2024 Project CHIP Authors
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
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --endpoint endpoint


class TC_MWOCTRL_2_2(MatterBaseTest):

    async def read_mwoctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.MicrowaveOvenControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_MWOCTRL_2_2(self) -> str:
        return "[TC-MWOCTRL-2.2] Secondary functionality with DUT as Server"

    def steps_TC_MWOCTRL_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the PowerSetting attribute"),
            TestStep(3, "Send the SetCookingParameters command"),
            TestStep(4, "Read and verify the PowerSetting attribute"),
            TestStep(5, "Cause constraint error response"),
        ]
        return steps

    def pics_TC_MWOCTRL_2_2(self) -> list[str]:
        pics = [
            "MWOCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_MWOCTRL_2_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.MicrowaveOvenControl.Attributes
        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        features = Clusters.MicrowaveOvenControl.Bitmaps.Feature
        commands = Clusters.Objects.MicrowaveOvenControl.Commands

        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        only_pwrnum_supported = feature_map == features.kPowerAsNumber

        if not only_pwrnum_supported:
            logging.info("More than PWRNUM is supported so skipping remaining test steps")
            self.skip_all_remaining_steps(2)
            return

        logging.info("Only the PWRNUM feature is supported so continuing with remaining test steps")
        self.step(2)
        powerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerSetting)
        asserts.assert_greater_equal(powerValue, 10, "PowerSetting is less than 10")
        asserts.assert_less_equal(powerValue, 100, "PowerSetting is greater than 100")
        asserts.assert_true(powerValue % 10 == 0, "PowerSetting is not a multiple of 10")

        self.step(3)
        newPowerValue = (powerValue+10) % 100
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(powerSetting=newPowerValue), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(4)
        powerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerSetting)
        asserts.assert_true(powerValue == newPowerValue, "PowerSetting was not correctly set")

        self.step(5)
        newPowerValue = 125
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(powerSetting=newPowerValue), endpoint=endpoint)
            asserts.assert_fail("Expected an exception but received none.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Expected ConstraintError but received a different error.")


if __name__ == "__main__":
    default_matter_test_main()
