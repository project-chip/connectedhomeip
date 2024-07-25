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

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --endpoint endpoint


class TC_MWOCTRL_2_1(MatterBaseTest):

    async def read_mwoctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.MicrowaveOvenControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def set_cook_time_expect_success(self, endpoint, value):
        commands = Clusters.Objects.MicrowaveOvenControl.Commands
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(cookTime=value), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

    async def set_bad_cook_time_value_expect_failure(self, endpoint, value):
        commands = Clusters.Objects.MicrowaveOvenControl.Commands
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(cookTime=value), endpoint=endpoint)
            asserts.assert_fail("Expected an exception but received none.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Expected a CONSTRAINT_ERROR but got a different response.")

    async def read_and_check_cook_time_value(self, endpoint, value):
        attributes = Clusters.MicrowaveOvenControl.Attributes
        cooktime = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CookTime)
        asserts.assert_equal(cooktime, value, "Cooktime value not as expected")

    def desc_TC_MWOCTRL_2_1(self) -> str:
        return "[TC-MWOCTRL-2.1] Primary functionality with DUT as Server"

    def steps_TC_MWOCTRL_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the MaxCookTime attribute and check limits",
                     "Verify that the DUT response contains an elapsed-s value between 1 and 86400 inclusive. Save value as MaxCookTime."
                     ),
            TestStep(3, "Read the CookTime attribute and check limits",
                     "Verify that the DUT response contains an elapsed-s value between 1 and MaxCookTime inclusive."
                     ),
            TestStep(4, "Set the CookTime attribute to 60", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(5, "Read the CookTime attribute and check for 60",
                     "Verify that the DUT response contains the CookTime value 60."),
            TestStep(6, "Set the CookTime attribute to 1", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(7, "Read the CookTime attribute and check for 1",
                     "Verify that the DUT response contains the CookTime value 1."),
            TestStep(8, "Set the CookTime attribute to MaxCookTime", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(9, "Read the CookTime attribute and check for MaxCookTime",
                     "Verify that the DUT response contains the CookTime value MaxCookTime."),
            TestStep(10, "Read the WattRating attribute, if supported", "Verify that the DUT response contains a uint16 value."),
            TestStep(11, "Set the CookTime attribute to 0", "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(12, "Set the CookTime attribute to MaxCookTime+1", "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
        ]
        return steps

    def pics_TC_MWOCTRL_2_1(self) -> list[str]:
        pics = [
            "MWOCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_MWOCTRL_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.MicrowaveOvenControl.Attributes

        self.step(2)
        maxCookTime = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxCookTime)
        asserts.assert_greater_equal(maxCookTime, 1, "maxCookTime is less than 1")
        asserts.assert_less_equal(maxCookTime, 86400, "maxCookTime is greater than 86400")

        self.step(3)
        cookTime = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CookTime)
        asserts.assert_greater_equal(cookTime, 1, "cookTime is less than 1")
        asserts.assert_less_equal(cookTime, maxCookTime, "cookTime is greater than maxCookTime")

        self.step(4)
        newCookTime = 60
        await self.set_cook_time_expect_success(endpoint, newCookTime)

        self.step(5)
        await self.read_and_check_cook_time_value(endpoint, newCookTime)

        self.step(6)
        await self.set_cook_time_expect_success(endpoint, 1)

        self.step(7)
        await self.read_and_check_cook_time_value(endpoint, 1)

        self.step(8)
        await self.set_cook_time_expect_success(endpoint, maxCookTime)

        self.step(9)
        await self.read_and_check_cook_time_value(endpoint, maxCookTime)

        self.step(10)
        if self.pics_guard(self.check_pics("MWOCTRL.S.F01")):
            await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.WattRating)

        self.step(11)
        await self.set_bad_cook_time_value_expect_failure(endpoint, 0)

        self.step(12)
        await self.set_bad_cook_time_value_expect_failure(endpoint, maxCookTime+1)


if __name__ == "__main__":
    default_matter_test_main()
