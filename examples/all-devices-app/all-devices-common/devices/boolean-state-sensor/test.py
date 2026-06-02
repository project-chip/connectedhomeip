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
import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class BooleanStateSensorCommissioningTest(MatterBaseTest):
    """Simple test that commissions and reads the Boolean State Sensor device."""

    async def read_boolean_state_value(self, endpoint: int):
        """Helper method to read the StateValue attribute from the BooleanState cluster on a given endpoint."""
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.BooleanState,
            attribute=Clusters.BooleanState.Attributes.StateValue
        )

    def desc_TC_BOOL_1_1(self) -> str:
        return "[TC_BOOL_1_1] Boolean State Sensor Commissioning and State Read Test"

    def steps_TC_BOOL_1_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read Boolean State Value on Endpoint 1 and Endpoint 2"),
        ]

    @async_test_body
    async def test_TC_BOOL_1_1(self):
        # Step 1: Commissioning (handled automatically by the test runner)
        self.step(1)
        logger.info("Successfully completed commissioning step.")

        # Step 2: Read Boolean State Value on Endpoint 1 and Endpoint 2
        self.step(2)

        # Read Endpoint 1
        state_value_ep1 = await self.read_boolean_state_value(endpoint=1)
        logger.info(f"Boolean State Value on Endpoint 1: {state_value_ep1}")

        # Read Endpoint 2
        state_value_ep2 = await self.read_boolean_state_value(endpoint=2)
        logger.info(f"Boolean State Value on Endpoint 2: {state_value_ep2}")


if __name__ == "__main__":
    default_matter_test_main()
