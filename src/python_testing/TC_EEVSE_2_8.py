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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${EVSE_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

import matter.clusters as Clusters
from matter.clusters.Types import Nullable
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

cluster = Clusters.EnergyEvse


class TC_EEVSE_2_8(MatterBaseTest, EEVSEBaseTestHelper):
    """This test case verifies the primary functionality of the Energy EVSE
    Cluster server with the optional PlugAndCharge feature supported."""

    def desc_TC_EEVSE_2_8(self) -> str:
        """Returns a description of this test"""
        return "[TC-EEVSE-2.8] Optional PlugAndCharge feature functionality with DUT as Server"

    def pics_TC_EEVSE_2_8(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEVSE.S.F02"]

    def steps_TC_EEVSE_2_8(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("3", "TH reads from the DUT the VehicleID attribute.",
                     "Verify that the DUT response contains a string value, or null."),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Set VehicleID Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5", "TH reads from the DUT the VehicleID attribute.",
                     "Verify that the DUT response contains a string value with length <= 32 characters."),
        ]

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kPlugAndCharge))
    async def test_TC_EEVSE_2_8(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        value = await self.read_evse_attribute_expect_success(attribute="VehicleID")
        asserts.assert_is_instance(value, (Nullable, str), "VehicleID must be null or a string")

        self.step("4")
        await self.send_test_event_trigger_evse_set_vehicleid()

        self.step("5")
        value = await self.read_evse_attribute_expect_success(attribute="VehicleID")
        asserts.assert_is_instance(value, str, "VehicleID must be a string")
        asserts.assert_less_equal(len(value), 32, "VehicleID length must be less than or equal to 32 characters")


if __name__ == "__main__":
    default_matter_test_main()
