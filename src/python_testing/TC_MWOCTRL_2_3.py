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


class TC_MWOCTRL_2_3(MatterBaseTest):

    async def read_mwoctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.MicrowaveOvenControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_MWOCTRL_2_3(self) -> str:
        return "[TC-MWOCTRL-2.3] PWRNUM functionality with DUT as Server"

    def steps_TC_MWOCTRL_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the MinPower attribute"),
            TestStep(3, "Read the MaxPower attribute"),
            TestStep(4, "Read the PowerStep attribute"),
            TestStep(5, "Read the PowerSetting attribute"),
            TestStep(6, "Send the SetCookingParameters command"),
            TestStep(7, "Read and verify the PowerSetting attribute"),
            TestStep(8, "Cause constraint error response"),
        ]
        return steps

    def pics_TC_MWOCTRL_2_3(self) -> list[str]:
        pics = [
            "MWOCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_MWOCTRL_2_3(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.MicrowaveOvenControl.Attributes
        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        features = Clusters.MicrowaveOvenControl.Bitmaps.Feature
        commands = Clusters.Objects.MicrowaveOvenControl.Commands

        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        supports_pwrnum_and_limits = feature_map & (features.kPowerAsNumber or features.kPowerNumberLimits)

        if not supports_pwrnum_and_limits:
            logging.info("Device does not support PWRNUM and PWRLMTS so skipping the rest of the tests.")
            self.skip_all_remaining_steps(2)
            return

        self.step(2)
        minPowerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MinPower)
        logging.info("MinPower is %s" % minPowerValue)
        asserts.assert_true(minPowerValue >= 1, "MinPower is less than 1")

        self.step(3)
        maxPowerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxPower)
        logging.info("MaxPower is %s" % maxPowerValue)
        asserts.assert_true(maxPowerValue >= 1, "MaxPower is less than MinPower")
        asserts.assert_true(maxPowerValue <= 100, "MaxPower is greater than 100")

        self.step(4)
        powerStepValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerStep)
        logging.info("PowerStep is %s" % powerStepValue)
        asserts.assert_true(powerStepValue >= 1, "PowerStep is less than 1")
        asserts.assert_true(powerStepValue <= maxPowerValue, "PowerStep is greater than MaxPower")

        self.step(5)
        powerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerSetting)
        logging.info("PowerSetting is %s" % powerValue)
        asserts.assert_true(powerValue >= minPowerValue, "PowerSetting is less than MinPower")
        asserts.assert_true(powerValue <= maxPowerValue, "PowerSetting is greater than MaxPower")
        asserts.assert_true((powerValue-minPowerValue) % powerStepValue == 0, "PowerSetting is not a multiple of 10")

        self.step(6)
        newPowerValue = (powerValue-minPowerValue) % (maxPowerValue-minPowerValue)+powerStepValue+minPowerValue
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(powerSetting=newPowerValue), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(7)
        powerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerSetting)
        asserts.assert_true(powerValue == newPowerValue, "PowerSetting was not correctly set")

        self.step(8)
        newPowerValue = maxPowerValue+1
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(powerSetting=newPowerValue), endpoint=endpoint)
            asserts.assert_fail("Expected an exception but received none.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Expected ConstraintError but received a different response.")


if __name__ == "__main__":
    default_matter_test_main()
