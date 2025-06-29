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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import random
from typing import Any

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_asserts import assert_valid_map8
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_2_4(MatterBaseTest):
    def desc_TC_FAN_2_4(self) -> str:
        return "[TC-FAN-2.4] Optional wind attributes with DUT as Server"

    def steps_TC_FAN_2_4(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH reads from the DUT the WindSupport attribute.",
                         "Verify that the WindSupport attribute value is a map8 bitmap. Verify that the WindSupport attribute's value is between 1 and 3 inclusive."),
                TestStep(3, "[FC] TH reads from the DUT the WindSetting attribute.",
                         "Verify that the WindSetting attribute value is a map8 bitmap. Verify that the WindSetting attribute's value is between 0 and 3 inclusive"),
                TestStep(4, "[FC] TH checks that WindSetting is conformant with WindSupport.",
                         "Verify that all bits set in WindSetting are also set in WindSupport."),
                TestStep(5, "[FC] TH writes a valid bit from WindSupport to WindSetting.",
                         "Device shall return SUCCESS."),
                TestStep(6, "[FC] TH reads the WindSetting attribute.",
                         "Verify that the proper bit was set from the previous step."),
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

    async def write_setting(self, attribute, value) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        write_status = result[0].Status
        write_status_success = (write_status == Status.Success)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of SUCCESS ({write_status.name}), value: {value}")
        return write_status

    @staticmethod
    def get_random_wind_setting(wind_support: int, wind_support_range: range) -> int:
        """
        Returns a random valid WinddSetting based on the given WindSupport bitmask.
        """
        valid_wind_setting_values = [i for i in wind_support_range if (i & wind_support) == i]
        return random.choice(valid_wind_setting_values)

    def pics_TC_FAN_2_4(self) -> list[str]:
        return ["FAN.S.F03"]

    @run_if_endpoint_matches(has_feature(Clusters.FanControl, Clusters.FanControl.Bitmaps.Feature.kWind))
    async def test_TC_FAN_2_4(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        valid_wind_support_range = range(1, 4)
        valid_wind_setting_range = range(0, 4)

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH reads from the DUT the WindSupport attribute
        self.step(2)
        wind_support = await self.read_setting(attr.WindSupport)

        # Verify that the WindSupport attribute value is a map8 bitmap
        assert_valid_map8(wind_support, "WindSupport")

        # Verify that the WindSupport attribute's value is between 1 and 3 inclusive
        asserts.assert_in(wind_support, valid_wind_support_range,
                          f"[FC] WindSupport attribute value ({wind_support}) is not between 1 and 3 inclusive")

        # *** STEP 3 ***
        # TH reads from the DUT the WindSetting attribute
        self.step(3)
        wind_setting = await self.read_setting(attr.WindSetting)

        # Verify that the WindSetting attribute value is a map8 bitmap
        assert_valid_map8(wind_setting, "WindSetting")

        # Verify that the WindSetting attribute's value is between 0 and 3 inclusive
        asserts.assert_in(wind_setting, valid_wind_setting_range,
                          f"[FC] WindSetting attribute value ({wind_setting}) is not between 0 and 3 inclusive")

        # *** STEP 4 ***
        # TH checks that WindSetting is conformant with WindSupport
        # Verify that all bits set in WindSetting are also set in WindSupport
        self.step(4)
        is_wind_conformant = (wind_setting & wind_support) == wind_setting
        asserts.assert_true(is_wind_conformant, "[FC] WindSetting contains unsupported bits; it is not conformant with WindSupport")

        # *** STEP 5 ***
        # TH writes a valid bit from WindSupport to WindSetting
        # Device shall return SUCCESS
        self.step(5)
        wind_setting_write = self.get_random_wind_setting(wind_support, valid_wind_support_range)
        await self.write_setting(attr.WindSetting, wind_setting_write)

        # *** STEP 6 ***
        # TH reads the WindSetting attribute
        # Verify that the proper bit was set from the previous step
        self.step(6)
        wind_setting_read = await self.read_setting(attr.WindSetting)
        asserts.assert_equal(wind_setting_read, wind_setting_write,
                             f"[FC] WindSetting attribute value ({wind_setting_read}) does not match the expected value ({wind_setting_write})")


if __name__ == "__main__":
    default_matter_test_main()
