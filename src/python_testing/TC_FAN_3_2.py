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
# test-runner-runs:
#   run1:
#     app: ${AIR_PURIFIER_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --timeout 600
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import random
from typing import Any

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.interaction_model import Status
from chip.testing.matter_asserts import is_valid_uint_value
from chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, AttributeValue,
                                         async_test_body, default_matter_test_main)
from mobly import asserts


logger = logging.getLogger(__name__)


class Uint8Type(int):
    pass


class TC_FAN_3_2(MatterBaseTest):
    def desc_TC_FAN_3_2(self) -> str:
        return "[TC-FAN-3.2] Optional speed functionality with DUT as Server"

    def steps_TC_FAN_3_2(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH checks the DUT for support of the MultiSpeed feature.",
                         "If the MultiSpeed feature is unsupported, fail the test."),
                TestStep(3, "[FC] TH reads from the DUT the initial SpeedMax attribute value and store.",
                         "Verify that the DUT response contains a uint8 with value between 1 and 100 inclusive."),
                TestStep(4, "[FC] TH reads from the DUT the initial SpeedSetting attribute value and store.", "Verify that the DUT response contains a uint8 with value between 0 and the value of SpeedMax inclusive."),
                TestStep(5, "[FC] TH subscribes to the DUT's FanControl cluster.", "Enables the TH to receive attribute updates."),
                TestStep(6, "[FC] TH writes to the DUT a SpeedSetting value less than or equal to the SpeedMax value.",
                         "Device shall return either SUCCESS or INVALID_VALUE. If the write operation returned SUCCESS, verify that the SpeedSetting and SpeedCurrent attributes are updated with the new value. If the write operation returned INVALID_VALUE, verify that the SpeedSetting and SpeedCurrent attributes are the same as the initial SpeedSetting value"),
                ]

    async def read_setting(self, attribute: Any) -> Any:
        """
        Asynchronously reads a specified attribute from the FanControl cluster at a given endpoint.

        Args:
            endpoint (int): The endpoint identifier for the fan device.
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
        write_status_success = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_status.name})")
        return write_status, value

    async def verify_setting(self, attribute, type, range):
        # Read attribute value
        value = await self.read_setting(attribute)

        # Verify response is of expected type
        if type is Uint8Type:
            asserts.assert_true(is_valid_uint_value(value),
                                f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")
        else:
            asserts.assert_is_instance(value, type,
                                       f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")

        # Verify response is valid (value is within expected range)
        asserts.assert_in(value, range, f"[FC] {attribute.__name__} result ({value}) is invalid")

        return value    

    def pics_TC_FAN_3_2(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_2(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attributes = cluster.Attributes

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH checks the DUT for support of the MultiSpeed feature
        # If the MultiSpeed feature is unsupported, fail the test
        self.step(2)
        feature_map = await self.read_setting(attributes.FeatureMap)
        self.supports_multispeed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if not self.supports_multispeed:
            asserts.fail("[FC] This test case is only valid if the MultiSpeed feature is supported by the DUT.")

        # *** STEP 3 ***
        # TH reads from the DUT the initial SpeedMax attribute value and store
        # Verify that the DUT response contains a uint8 with value between 1 and 100 inclusive
        self.step(3)
        speed_max = await self.verify_setting(attributes.SpeedMax, Uint8Type, range(1, 101))

        # *** STEP 4 ***
        # TH reads from the DUT the initial SpeedSetting attribute value and store
        # Verify that the DUT response contains a uint8 with value between 0 and the value of SpeedMax inclusive
        self.step(4)
        speed_setting_init = await self.verify_setting(attributes.SpeedSetting, Uint8Type, range(0, speed_max + 1))
        
        logging.info(f"[FC] Initial settings - SpeedMax: {speed_max}, SpeedSetting: {speed_setting_init}")

        # *** STEP 5 ***
        # TH subscribes to the DUT's FanControl cluster
        self.step(5)
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, self.endpoint)

        # *** STEP 6 ***
        # TH writes to the DUT a SpeedSetting value less than or equal to the SpeedMax value
        # Device shall return either SUCCESS or INVALID_VALUE
        # If the write operation returned SUCCESS, verify that the SpeedSetting and SpeedCurrent attributes
        # are updated with the new value
        # If the write operation returned INVALID_VALUE, verify that the SpeedSetting and SpeedCurrent attributes
        # are the same as the initial SpeedSetting value
        self.step(6)
        write_status, value = await self.write_setting(attributes.SpeedSetting, random.randint(1, speed_max))
        if write_status == Status.Success:
            expected_values = [
                AttributeValue(
                    endpoint_id=self.endpoint,
                    attribute=attributes.SpeedSetting,
                    value=value),
                AttributeValue(
                    endpoint_id=self.endpoint,
                    attribute=attributes.SpeedCurrent,
                    value=value)
                ]
            self.attribute_subscription.await_all_final_values_reported(
                expected_final_values=expected_values, timeout_sec=1)
        elif write_status == Status.InvalidInState:
            speed_setting_current = await self.read_setting(attributes.SpeedSetting)
            speed_current_current = await self.read_setting(attributes.SpeedCurrent)
            asserts.assert_equal(speed_setting_current, speed_setting_init, f"[FC] Current SpeedSetting ({speed_setting_current}) must be equal to initial SpeedSetting ({speed_setting_init}).")
            asserts.assert_equal(speed_current_current, speed_setting_init, f"[FC] Current SpeedCurrent ({speed_current_current}) must be equal to initial SpeedSetting ({speed_setting_init}).")

if __name__ == "__main__":
    default_matter_test_main()
