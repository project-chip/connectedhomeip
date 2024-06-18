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


class TC_MWOCTRL_2_5(MatterBaseTest):

    async def read_mwoctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.MicrowaveOvenControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def set_cook_time_expect_success(self, endpoint, value):
        commands = Clusters.Objects.MicrowaveOvenControl.Commands
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(cookTime=value), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

    async def send_add_more_time_command_expect_response(self, endpoint, value, expectedError=Status.Success):
        commands = Clusters.Objects.MicrowaveOvenControl.Commands
        try:
            await self.send_single_cmd(cmd=commands.AddMoreTime(timeToAdd=value), endpoint=endpoint)
            asserts.assert_equal(Status.Success, expectedError, "No error occured but unexpected response received.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expectedError, "Unexpected error response received.")

    async def read_and_check_cook_time_value(self, endpoint, value):
        attributes = Clusters.MicrowaveOvenControl.Attributes
        cooktime = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CookTime)
        asserts.assert_equal(cooktime, value, "Cooktime value not as expected")

    def desc_TC_MWOCTRL_2_5(self) -> str:
        return "[TC-MWOCTRL-2.1] Primary functionality with DUT as Server"

    def steps_TC_MWOCTRL_2_5(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Set the CookTime attribute to 60",
                        "Verify DUT responds w/ status SUCCESS(0x00)."
                     ),
            TestStep(3, "Send the AddMoreTime command with a value of 30.",
                        "Verify DUT responds w/ status SUCCESS(0x00)."
                     ),
            TestStep(4, "Read the CookTime attribute and verify it is 90",
                        "Verify DUT responds w/ a value of 90."),
            TestStep(5, "Read the MaxCookTime attribute",
                        "Save the response as MaxCookTime"),
            TestStep(6, "Send the AddMoreTime command with a value of MaxCookTime+1.",
                        "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)."
                     ),
            TestStep(7, "Manually set the device into a state where it will respond with INVALID_IN_STATE for the AddMoreTime command."),
            TestStep(8, "Send the AddMoreTime command with a value of 30.",
                        "Verify DUT responds w/ status INVALID_IN_STATE(0xCB)."
                     ),
        ]
        return steps

    def pics_TC_MWOCTRL_2_5(self) -> list[str]:
        pics = [
            "MWOCTRL.S",
            "MWOCTRL.SC.ADD_MORE_TIME"
        ]
        return pics

    @async_test_body
    async def test_TC_MWOCTRL_2_5(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.MicrowaveOvenControl.Attributes

        self.step(2)
        await self.set_cook_time_expect_success(endpoint, 60)

        self.step(3)
        await self.send_add_more_time_command_expect_response(endpoint, 30)

        self.step(4)
        await self.read_and_check_cook_time_value(endpoint, 90)

        self.step(5)
        maxCookTime = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxCookTime)

        self.step(6)
        await self.send_add_more_time_command_expect_response(endpoint, maxCookTime+1, Status.ConstraintError)

        if self.check_pics("MWOCTRL.S.M.ManualInvalidAddTime"):
            self.step(7)
            input("Press Enter when done.\n")

            self.step(8)
            await self.send_add_more_time_command_expect_response(endpoint, 30, Status.InvalidInState)
        else:
            self.skip_step(7)
            self.skip_step(8)


if __name__ == "__main__":
    default_matter_test_main()
