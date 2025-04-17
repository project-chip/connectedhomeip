#
#    Copyright (c) 2022 Project CHIP Authors
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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CLDIM_4_2(MatterBaseTest):
    def desc_TC_CLDIM_4_2(self) -> str:
        return "[TC-CLDIM-4.2] {C_STEP} Command Input Sanity Check with DUT as Server"

    def steps_TC_CLDIM_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Positioning feature is not supported, skip remaining steps"),
            TestStep(3, "Send Step command with invalid Direction"),
            TestStep(4, "Send Step command with invalid Speed"),
            TestStep(5, "Send Step command with NumberOfSteps = 0"),
        ]
        return steps

    def pics_TC_CLDIM_4_2(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_4_2(self):
        endpoint = self.get_endpoint(default=1)
        attributes = Clusters.ClosureDimension.Attributes

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2a: Read FeatureMap attribute
        self.step("2a")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.ClosureDimension,
            attribute=attributes.FeatureMap
        )

        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning

        # STEP 2b: If Positioning feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("3")
            return

        # STEP 3: Send Step command with invalid Direction
        self.step(3)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=2,  # Invalid Direction
                    NumberOfSteps=1
                ),
                endpoint=endpoint
            )

            asserts.fail("Expected ConstraintError for invalid Direction")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned for invalid Direction")

        # STEP 4: Send Step command with invalid Speed
        self.step(4)
        if feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                        NumberOfSteps=1,
                        Speed=4,  # Invalid Speed
                    ),
                    endpoint=endpoint
                )

                asserts.fail("Expected ConstraintError for invalid Speed")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned for invalid Speed")

        # STEP 5: Send Step command with NumberOfSteps = 0
        self.step(5)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                    NumberOfSteps=0
                ),
                endpoint=endpoint
            )

            asserts.fail("Expected ConstraintError for NumberOfSteps = 0")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned for NumberOfSteps = 0")


if __name__ == "__main__":
    default_matter_test_main()
