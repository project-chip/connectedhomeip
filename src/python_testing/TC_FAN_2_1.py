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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
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
        return "[TC-FAN-3.1] Mandatory functionality with DUT as Server"

    def steps_TC_FAN_2_1(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH writes to the DUT the Auto (5) FanMode attribute value."),
                TestStep(3, "[FC] TH reads from the DUT the FanMode attribute.",
                         "Verify that the DUT response contains an enum8 with a value between 0 and 5, excluding 4."),
                TestStep(4, "[FC] TH reads from the DUT the FanModeSequence attribute.",
                         "Verify that the DUT response contains an enum8 with value between 0 and 5 inclusive. Verify that when FanMode is Auto (5), the FanModeSequence supports Auto mode."),
                TestStep(5, "[FC] TH writes to the DUT the Low (1) FanMode attribute value."),
                TestStep(6, "[FC] TH reads from the DUT the PercentSetting attribute.",
                         "Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive."),
                TestStep(7, "[FC] TH reads from the DUT the PercentCurrent attribute.",
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
            asserts.assert_true(is_valid_uint_value(value),
                                f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")
        else:
            asserts.assert_is_instance(value, type,
                                       f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")

        # Verify response is valid (value is within expected range)
        asserts.assert_in(value, range, f"[FC] {attribute.__name__} result ({value}) is invalid")

        return value

    async def verify_auto_conformance(self, fan_mode, fan_mode_sequence):
        fm_enum = Clusters.FanControl.Enums.FanModeEnum
        fms_enum = Clusters.FanControl.Enums.FanModeSequenceEnum

        # FanModeSequence values that support Auto FanMode
        fms_auto_values = [fms_enum.kOffLowMedHighAuto, fms_enum.kOffLowHighAuto, fms_enum.kOffHighAuto]

        # If FanMode is Auto (5), verify FanModeSequence supports auto FanMode
        if fan_mode == fm_enum.kAuto:
            asserts.assert_in(fan_mode_sequence, fms_auto_values,
                              f"[FC] FanModeSequence value ({fan_mode_sequence}:{fan_mode_sequence.name}) must support Auto FanMode")

    def pics_TC_FAN_2_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_2_1(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attribute = cluster.Attributes
        fm_enum = cluster.Enums.FanModeEnum
        fms_enum = cluster.Enums.FanModeSequenceEnum
        self.timeout_sec = 0.01  # Timeout given for item retreival from the attribute queue

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH writes to the DUT the Auto (5) FanMode attribute value
        self.step(2)
        await self.write_setting(attribute.FanMode, fm_enum.kAuto)

        # TH reads from the DUT the FanMode attribute
        # Verify that the DUT response contains an enum8 with a value between 0 and 5, excluding 4
        self.step(3)
        fan_mode = await self.verify_setting(attribute.FanMode, fm_enum, [0, 1, 2, 3, 5])

        # TH reads from the DUT the FanModeSequence attribute
        # Verify that the DUT response contains an enum8 with value between 0 and 5 inclusive.
        self.step(4)
        fan_mode_sequence = await self.verify_setting(attribute.FanModeSequence, fms_enum, range(0, 6))

        # Verify that when FanMode is Auto (5), the FanModeSequence supports Auto mode
        await self.verify_auto_conformance(fan_mode, fan_mode_sequence)

        # TH writes to the DUT the Low (1) FanMode attribute value
        self.step(5)
        await self.write_setting(attribute.FanMode, fm_enum.kLow)

        # TH reads from the DUT the PercentSetting attribute
        # Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive.
        self.step(6)
        await self.verify_setting(attribute.PercentSetting, Uint8Type, range(0, 101))

        # TH reads from the DUT the PercentCurrent attribute
        # Verify that the DUT response contains a uint8 with value between 0 and 100 inclusive.
        self.step(7)
        await self.verify_setting(attribute.PercentCurrent, Uint8Type, range(0, 101))


if __name__ == "__main__":
    default_matter_test_main()
