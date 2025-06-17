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
#     app: ${ENERGY_MANAGEMENT_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --application evse
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

import logging
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, EventChangeCallback, MatterBaseTest, TestStep,
                                         async_test_body, default_matter_test_main)
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EEVSE_2_7(MatterBaseTest, EEVSEBaseTestHelper):
    """This test case verifies the primary functionality of the Energy EVSE Cluster server
     with the optional SoCReporting feature supported. This test case can also verify the
     interaction between SoCReporting and ChargingPreferences features if ChargingPreferences
     is also supported."""

    def desc_TC_EEVSE_2_7(self) -> str:
        """Returns a description of this test"""
        return "[TC-EEVSE-2.7] Optional SoCReporting feature functionality with DUT as Server"

    def pics_TC_EEVSE_2_7(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["EEVSE.S.F01"]

    def steps_TC_EEVSE_2_7(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the FeatureMap",
                     "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5", "TH reads from the DUT the State",
                     "Value has to be 0x00 (NotPluggedIn)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E00(EVConnected) sent"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EVSE Set SoC Low Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the StateOfCharge",
                     "Value has to be 20% state Of charge"),
            TestStep("7b", "TH reads from the DUT the BatteryCapacity",
                     "Value has to be 70,000,000 (70kWh)"),

        ]

        return steps

    @async_test_body
    async def test_TC_EEVSE_2_7(self):
        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.get_endpoint())

        self.step("2")
        feature_map = await self.read_evse_attribute_expect_success(attribute="FeatureMap")
        logger.info(f"FeatureMap: {feature_map}")
        has_v2x = feature_map & Clusters.EnergyEvse.Bitmaps.Feature.kV2x

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_basic()

        self.step("5")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("6")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVConnected)

        self.step("7")
        await self.send_test_event_trigger_evse_set_soc_low()

        self.step("7a")
        await self.check_evse_attribute("StateOfCharge", 20)

        self.step("7b")
        await self.check_evse_attribute("BatteryCapacity", 70000)


if __name__ == "__main__":
    default_matter_test_main()
