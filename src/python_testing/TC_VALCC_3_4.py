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

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_VALCC_3_4(MatterBaseTest):
    async def read_valcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VALCC_3_4(self) -> str:
        return "[TC-VALCC-3.4] LevelStep behavior with DUT as Server"

    def steps_TC_VALCC_3_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read AttributeList attribute"),
            TestStep(3, "Verify LevelStep is supported"),
            TestStep(4, "Read LevelStep attribute"),
            TestStep(5, "Verify the supported level values using Open Command"),
            TestStep(6, "Send Close command"),
        ]
        return steps

    def pics_TC_VALCC_3_4(self) -> list[str]:
        pics = [
            "VALCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VALCC_3_4(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        attribute_list = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        if attributes.LevelStep.attribute_id not in attribute_list:
            logging.info("LevelStep not supported skipping test case")

            # Skipping all remainig steps
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")

            return

        else:
            logging.info("Test step skipped")

        self.step(4)
        levelStep = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.LevelStep)

        self.step(5)

        for levelValue in range(1, 100):
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(targetLevel=levelValue), endpoint=endpoint)

                if levelValue % levelStep != 0:
                    asserts.fail("Received Success response when an CONSTRAINT_ERROR was expected")

            except InteractionModelError as e:
                if levelValue % levelStep != 0:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         "Unexpected error returned when an CONSTRAINT_ERROR was expected")
                else:
                    asserts.fail("Unexpected error returned")
                pass

        self.step(6)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass


if __name__ == "__main__":
    default_matter_test_main()
