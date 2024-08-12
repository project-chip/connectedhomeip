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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_MICROWAVE_OVEN_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

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

    async def set_power_setting_expect_success(self, endpoint, value):
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.MicrowaveOvenControl.Commands.SetCookingParameters(powerSetting=value), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Error while trying to set the power setting.")

    async def set_power_setting_expect_failure(self, endpoint, value):
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.MicrowaveOvenControl.Commands.SetCookingParameters(powerSetting=value), endpoint=endpoint)
            asserts.assert_fail("Expected an exception but received none.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "Expected ConstraintError but received a different response")

    async def read_and_check_power_setting_value(self, endpoint, value):
        powerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.MicrowaveOvenControl.Attributes.PowerSetting)
        asserts.assert_equal(powerValue, value, "PowerSetting was not correctly set")

    def desc_TC_MWOCTRL_2_2(self) -> str:
        return "[TC-MWOCTRL-2.2] Secondary functionality with DUT as Server"

    def steps_TC_MWOCTRL_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Set MinPowerValue variable"),
            TestStep(3, "Read the MinPower attribute"),
            TestStep(4, "Set the MaxPowerValue variable"),
            TestStep(5, "Read the MaxPower attribute"),
            TestStep(6, "Set the PowerStepValue variable"),
            TestStep(7, "Read the PowerStep attribute"),
            TestStep(8, "Read the PowerSetting attribute"),
            TestStep(9, "Send the SetCookingParameters command"),
            TestStep(10, "Read and verify the PowerSetting attribute"),
            TestStep(11, "Set the PowerSetting attribute to the minimum value"),
            TestStep(12, "Read and verify the PowerSetting attribute"),
            TestStep(13, "Set the PowerSetting attribute to the maximum value"),
            TestStep(14, "Read and verify the PowerSetting attribute"),
            TestStep(15, "Set PowerSetting to an invalid value"),
            TestStep(16, "If PowerStep=1, exit test case."),
            TestStep(17, "Set PowerSetting to a value that is not an integer multiple of PowerStep"),
        ]
        return steps

    def pics_TC_MWOCTRL_2_2(self) -> list[str]:
        pics = [
            "MWOCTRL.S",
            "MWOCTRL.S.F00",
        ]
        return pics

    @async_test_body
    async def test_TC_MWOCTRL_2_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.MicrowaveOvenControl.Attributes
        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        features = Clusters.MicrowaveOvenControl.Bitmaps.Feature
        is_pwrlmits_feature_supported = feature_map & features.kPowerNumberLimits
        is_pwrnum_feature_supported = feature_map & features.kPowerAsNumber

        if not is_pwrnum_feature_supported:
            logging.info("PWRNUM is not supported so skipping remaining test steps")
            self.skip_all_remaining_steps(2)
            return

        logging.info("The PWRNUM feature is supported so continuing with remaining test steps")

        self.step(2)
        minPowerValue = 10

        self.step(3)
        if is_pwrlmits_feature_supported:
            minPowerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MinPower)
            asserts.assert_greater_equal(minPowerValue, 1, "MinPower is less than 1")
            asserts.assert_less_equal(minPowerValue, 99, "MinPower is less than 1")
        logging.info("MinPower is %s" % minPowerValue)

        self.step(4)
        maxPowerValue = 100

        self.step(5)
        if is_pwrlmits_feature_supported:
            maxPowerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxPower)
            asserts.assert_greater(maxPowerValue, minPowerValue, "MaxPower is less than MinPower")
            asserts.assert_less(maxPowerValue, 100, "MaxPower is greater than 100")
        logging.info("MaxPower is %s" % maxPowerValue)

        self.step(6)
        powerStepValue = 10

        self.step(7)
        if is_pwrlmits_feature_supported:
            powerStepValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerStep)
            asserts.assert_greater_equal(powerStepValue, 1, "PowerStep is less than 1")
            asserts.assert_less_equal(powerStepValue, maxPowerValue, "PowerStep is greater than MaxPower")
            asserts.assert_true((maxPowerValue - minPowerValue) % powerStepValue ==
                                0, "PowerStep is not correct for MaxPower - MinPower")
        logging.info("PowerStep is %s" % powerStepValue)

        self.step(8)
        powerValue = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.PowerSetting)
        asserts.assert_greater_equal(powerValue, minPowerValue, "PowerSetting is less than the minimum.")
        asserts.assert_less_equal(powerValue, maxPowerValue, "PowerSetting is greater than the maxium")
        asserts.assert_true((powerValue-minPowerValue) % powerStepValue == 0, "PowerSetting is not a multiple of power step")

        self.step(9)
        logging.info("minPowerValue is %s" % minPowerValue)
        logging.info("maxPowerValue is %s" % maxPowerValue)
        logging.info("powerStepValue is %s" % powerStepValue)
        logging.info("powerValue is %s" % powerValue)
        newPowerValue = (powerValue-minPowerValue) % (maxPowerValue-minPowerValue)+powerStepValue+minPowerValue
        logging.info("newPowerValue is %s" % newPowerValue)
        await self.set_power_setting_expect_success(endpoint, newPowerValue)

        self.step(10)
        await self.read_and_check_power_setting_value(endpoint, newPowerValue)

        self.step(11)
        await self.set_power_setting_expect_success(endpoint, minPowerValue)

        self.step(12)
        await self.read_and_check_power_setting_value(endpoint, minPowerValue)

        self.step(13)
        await self.set_power_setting_expect_success(endpoint, maxPowerValue)

        self.step(14)
        await self.read_and_check_power_setting_value(endpoint, maxPowerValue)

        self.step(15)
        newPowerValue = maxPowerValue+1
        await self.set_power_setting_expect_failure(endpoint, newPowerValue)

        self.step(16)
        if powerStepValue == 1:
            self.skip_step(17)
            return

        self.step(17)
        newPowerValue = minPowerValue + powerStepValue / 2
        logging.info("-------> MinPower = %d", minPowerValue)
        logging.info("-------> PowerStep = %d", powerStepValue)
        logging.info("-------> newPowerValue = %d", newPowerValue)
        await self.set_power_setting_expect_failure(endpoint, newPowerValue)


if __name__ == "__main__":
    default_matter_test_main()
