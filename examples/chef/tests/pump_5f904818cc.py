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
from typing import Any

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
# from chip.interaction_model import Status
# from chip.testing.matter_asserts import is_valid_uint_value
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_PUMP(MatterBaseTest):
    """Tests for chef pump device."""

    # Always receive attribute update reports, regardless of the time delta between them.
    _SUBSCRIPTION_MIN_INTERVAL_SEC = 0

    # Set this to a large value so liveliness updates aren't received during test.
    _SUBSCRIPTION_MAX_INTERVAL_SEC = 3600

    def desc_TC_PUMP(self) -> str:
        return "[TC_PUMP] Mandatory functionality with chef pump device as server"

    def steps_TC_PUMP(self):
        return [TestStep(1, "[PUMP] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[PUMP] Assert initial attribute values are expected."),
                TestStep(3, "[PUMP] Subscribe to all required attributes.")]

    @async_test_body
    async def test_TC_PUMP(self):
        # *** STEP 1 ***
        self.step(1)

        # ** STEP 2 ***
        self.step(2)
        onOff = await self.read_single_attribute_check_success(
            endpoint=1, cluster=Clusters.Objects.OnOff, attribute=Clusters.Objects.OnOff.Attributes.OnOff)
        asserts.assert_equal(onOff, False)
        level = await self.read_single_attribute_check_success(
            endpoint=1, cluster=Clusters.Objects.LevelControl, attribute=Clusters.Objects.LevelControl.Attributes.CurrentLevel)
        asserts.assert_equal(level, 1)

        # ** STEP 3 ***
        self.step(3)
        on_off_sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(1, Clusters.Objects.OnOff.Attributes.OnOff)],
            reportInterval=(self._SUBSCRIPTION_MIN_INTERVAL_SEC, self._SUBSCRIPTION_MAX_INTERVAL_SEC),
            keepSubscriptions=False,
        )
        level_control_sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(1, Clusters.Objects.OnOff.Attributes.OnOff)],
            reportInterval=(self._SUBSCRIPTION_MIN_INTERVAL_SEC, self._SUBSCRIPTION_MAX_INTERVAL_SEC),
            keepSubscriptions=True,  # Keep previous subscriptions.
        )


if __name__ == "__main__":
    default_matter_test_main()
