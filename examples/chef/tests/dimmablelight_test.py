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

from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_DIMMABLELIGHT(MatterBaseTest):
    """Tests for chef dimmablelight device."""

    def desc_TC_DIMMABLELIGHT(self) -> str:
        return "[TC_DIMMABLELIGHT] chef dimmablelight functionality test."

    def steps_TC_DIMMABLELIGHT(self):
        return [TestStep(1, "[TC_DIMMABLELIGHT] Commissioning already done.", is_commissioning=True)]

    @async_test_body
    async def test_TC_DIMMABLELIGHT(self):
        # *** STEP 1 ***
        # Commissioning already done.
        self.step(1)


if __name__ == "__main__":
    default_matter_test_main()
