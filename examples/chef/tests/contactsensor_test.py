#
#    Copyright (c) 2025 Project CHIP Authors
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

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_CONTACTSENSOR(MatterBaseTest):
    """Tests for chef contact sensor device."""

    _CONTACTSENSOR_ENDPOINT = 1

    async def _read_state_value(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._CONTACTSENSOR_ENDPOINT,
            cluster=Clusters.Objects.BooleanState,
            attribute=Clusters.Objects.BooleanState.Attributes.StateValue
        )

    def desc_TC_CONTACTSENSOR(self) -> str:
        return "[TC_CONTACTSENSOR] chef contact sensor functionality test."

    def steps_TC_CONTACTSENSOR(self):
        return [
            TestStep(1, "[TC_CONTACTSENSOR] Commissioning already done.", is_commissioning=True),
            TestStep(2, "[TC_CONTACTSENSOR] Test boolean state value.")
        ]

    @async_test_body
    async def test_TC_CONTACTSENSOR(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        # Read the StateValue attribute from the Boolean State cluster
        state_value = await self._read_state_value()

        # Verify that it returns a boolean value
        asserts.assert_true(isinstance(state_value, bool), "StateValue must be a boolean.")
        logger.info(f"Contact Sensor StateValue: {state_value}")


if __name__ == "__main__":
    default_matter_test_main()
