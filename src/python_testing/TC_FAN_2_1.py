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
from chip.clusters import ClusterObjects as ClusterObjects
from chip.interaction_model import Status
from chip.testing.matter_asserts import is_valid_uint_value
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class Uint8Type(int):
    pass


class TC_FAN_2_1(MatterBaseTest):
    def desc_TC_FAN_2_1(self) -> str:
        return "[TC-FAN-2.1] Mandatory functionality with DUT as Server"

    def steps_TC_FAN_2_1(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH checks for support of the Auto feature.",
                         "Save result for future use."),
                TestStep(3, "[FC] TH reads from the DUT the FanModeSequence attribute.",
                         "Verify that the DUT response contains a FanModeSequenceEnum with value between 0 and 5 inclusive. If Auto is not supported, verify that the FanModeSequence attribute is a valid sequence (non-Auto). If Auto is supported, verify that the FanModeSequence attribute is a valid sequence (Auto or non-Auto)."),
                TestStep(4, "[FC] TH reads from the DUT the FanMode attribute value.",
                         "Verify that the DUT response contains a FanModeEnum with a value between 0 and 5, excluding 4. Verify that if the FanMode attribute is set to Auto, the Auto feature is supported."),
                TestStep(5, "[FC] TH reads from the DUT the PercentSetting attribute.",
                         "Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive."),
                TestStep(6, "[FC] TH reads from the DUT the PercentCurrent attribute.",
                         "Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive."),
                ]

    async def read_setting(self, attribute: Any) -> Any:
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def write_setting(self, attribute, value) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        write_status = result[0].Status
        write_status_success = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_status.name})")
        return write_status

    async def verify_setting(self, attribute, type, range):
        # Read attribute value
        value = await self.read_setting(attribute)

        # Verify response is of expected type
        if type is Uint8Type:
            asserts.assert_true(is_valid_uint_value(value, bit_count=8),
                                f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")
        else:
            asserts.assert_is_instance(value, type,
                                       f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")

        # Verify response is valid (value is within expected range)
        asserts.assert_in(value, range, f"[FC] {attribute.__name__} result ({value}) is out of range")

        return value

    def pics_TC_FAN_2_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_2_1(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attribute = cluster.Attributes
        feature = cluster.Bitmaps.Feature
        fm_enum = cluster.Enums.FanModeEnum
        fms_enum = cluster.Enums.FanModeSequenceEnum
        non_auto_values = [fms_enum.kOffHigh, fms_enum.kOffLowHigh, fms_enum.kOffLowMedHigh]
        auto_values = [fms_enum.kOffHighAuto, fms_enum.kOffLowHighAuto, fms_enum.kOffLowMedHighAuto]

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH checks for support of the Auto feature
        self.step(2)
        feature_map = await self.read_setting(attribute.FeatureMap)
        supports_auto = bool(feature_map & feature.kAuto)
        logging.info(f"[FC] DUT supports Auto FanMode feature: {supports_auto}")

        # *** STEP 3 ***
        # TH reads from the DUT the FanModeSequence attribute
        # Verify that the DUT response contains a FanModeEnum with value between 0 and 5 inclusive.
        self.step(3)
        fan_mode_sequence = await self.verify_setting(attribute.FanModeSequence, fms_enum, range(0, 6))

        if not supports_auto:
            # If Auto is not supported, verify that the FanModeSequence
            # attribute is a valid sequence (non-Auto)
            asserts.assert_in(fan_mode_sequence, non_auto_values,
                              f"[FC] FanModeSequence ({fan_mode_sequence}:{fan_mode_sequence.name}) must be a valid FanMode sequence (non-Auto).")
        else:
            # If Auto is supported, verify that the FanModeSequence
            # attribute is a valid sequence (Auto or non-Auto)
            asserts.assert_in(fan_mode_sequence, non_auto_values + auto_values,
                              f"[FC] FanModeSequence ({fan_mode_sequence}:{fan_mode_sequence.name}) must be a valid FanMode sequence (Auto or non-Auto).")

        # *** STEP 4 ***
        # TH reads from the DUT the FanMode attribute value
        # Verify that the DUT response contains an enum8 with
        # a value between 0 and 5, excluding 4
        self.step(4)
        fan_mode = await self.verify_setting(attribute.FanMode, fm_enum, [0, 1, 2, 3, 5])

        # Verify that if the FanMode attribute is set to Auto,
        # the Auto feature is supported
        is_fan_mode_conformant = not ((fan_mode == fm_enum.kAuto) and not supports_auto)
        asserts.assert_true(is_fan_mode_conformant,
                            f"[FC] FanMode attribute ({fan_mode}:{fan_mode.name}) is set to Auto, but the DUT does not support the Auto feature.")

        # *** STEP 5 ***
        # TH reads from the DUT the PercentSetting attribute
        # Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive.
        self.step(5)
        await self.verify_setting(attribute.PercentSetting, Uint8Type, range(0, 101))

        # *** STEP 6 ***
        # TH reads from the DUT the PercentCurrent attribute
        # Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive.
        self.step(6)
        await self.verify_setting(attribute.PercentCurrent, Uint8Type, range(0, 101))


if __name__ == "__main__":
    default_matter_test_main()
