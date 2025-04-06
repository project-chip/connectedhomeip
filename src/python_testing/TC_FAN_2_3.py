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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${AIR_PURIFIER_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace_file json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from typing import Any

import chip.clusters as Clusters
from chip.testing.matter_asserts import assert_valid_uint8
from matter_testing_infrastructure.chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                                                       default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_2_3(MatterBaseTest):
    def desc_TC_FAN_2_3(self) -> str:
        return "[TC-FAN-2.3] Optional rock attributes with DUT as Server"

    def steps_TC_FAN_2_3(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH checks the DUT for support of the Rock feature (RCK).",
                         "If the DUT does not support the feature, the test is skipped."),
                TestStep(3, "[FC] TH reads from the DUT the RockSupport attribute.",
                         "Verify that the RockSupport attribute value is of uint8 type. Verify that the RockSupport attribute's value is between 1 and 3 inclusive."),
                TestStep(4, "[FC] TH reads from the DUT the RockSetting attribute.",
                         "Verify that the RockSetting attribute value is of uint8 type. Verify that the RockSetting attribute's value is between 0 and 3 inclusive"),
                TestStep(5, "[FC] TH checks that RockSetting is conformant with RockSupport.",
                         "Verify that all bits set in RockSetting are also set in RockSupport."),
                ]

    async def read_setting(self, attribute: Any) -> Any:
        """
        Asynchronously reads a specified attribute from the FanControl cluster at a given endpoint.

        Args:
            attribute (Any): The attribute to be read.

        Returns:
            Any: The value of the specified attribute if the read operation is successful.

        Raises:
            AssertionError: If the read operation fails.
        """
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    def pics_TC_FAN_2_3(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_2_3(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        rock_feature = cluster.Bitmaps.Feature.kRocking

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH checks the DUT for support of the Rock feature (RCK)
        #  - If the DUT does not support the feature, the test is skipped
        self.step(2)
        supports_wind = await self.feature_guard(self.endpoint, cluster, rock_feature)
        if not supports_wind:
            logger.info("[FC] The Rock (RCK) feature must be supported by the DUT for this test, skipping test.")
            self.skip_all_remaining_steps()

        # *** STEP 3 ***
        # TH reads from the DUT the RockSupport attribute
        self.step(3)
        rock_support = await self.read_setting(attr.RockSupport)

        # Verify that the RockSupport attribute value is of uint8 type
        assert_valid_uint8(rock_support, "RockSupport")

        # Verify that the RockSupport attribute's value is between 1 and 7 inclusive
        asserts.assert_in(rock_support, range(
            1, 8), f"[FC] RockSupport attribute value ({rock_support}) is not between 1 and 7 inclusive")

        # *** STEP 4 ***
        # TH reads from the DUT the RockSetting attribute
        self.step(4)
        rock_setting = await self.read_setting(attr.RockSetting)

        # Verify that the RockSetting attribute value is of uint8 type
        assert_valid_uint8(rock_setting, "RockSetting")

        # Verify that the RockSetting attribute's value is between 0 and 7 inclusive
        asserts.assert_in(rock_setting, range(
            0, 8), f"[FC] RockSetting attribute value ({rock_setting}) is not between 0 and 7 inclusive")

        # *** STEP 5 ***
        # TH checks that RockSetting is conformant with RockSupport
        # Verify that all bits set in RockSetting are also set in RockSupport
        self.step(5)
        is_rock_conformant = (rock_setting & rock_support) == rock_setting
        asserts.assert_true(is_rock_conformant, "[FC] RockSetting contains unsupported bits; it is not conformant with RockSupport")


if __name__ == "__main__":
    default_matter_test_main()
