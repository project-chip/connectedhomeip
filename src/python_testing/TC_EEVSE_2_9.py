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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.event_attribute_reporting import EventChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EEVSE_2_9(MatterBaseTest, EEVSEBaseTestHelper):
    """This test case verifies the primary functionality of the Energy EVSE Cluster server 
    with the optional RFID feature supported."""

    def desc_TC_EEVSE_2_9(self) -> str:
        """Returns a description of this test"""
        return "[TC-EEVSE-2.9] Optional RFID feature functionality with DUT as Server"

    def pics_TC_EEVSE_2_9(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEVSE.S", "EEVSE.S.F03"]

    def steps_TC_EEVSE_2_9(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("3", "Set up a subscription to all EnergyEVSE cluster events"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EVSE Trigger RFID Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5", "TH checks its subscription to RFID from DUT has yielded one new event within 5 seconds",
                     "Verify the data of the RFID event received by TH contains a UID with a maximum of 10 bytes."),
        ]

        return steps

    @async_test_body
    async def test_TC_EEVSE_2_9(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.get_endpoint())

        self.step("4")
        await self.send_test_event_trigger_evse_trigger_rfid()

        self.step("5")
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.Rfid)

        uid = event_data.uid
        asserts.assert_true(
            uid is not NullValue and len(uid) <= 10,
            f"RFID event received with UID: {uid}")

        logger.info(f"RFID event received with UID: 0x{bytes(uid).hex()}")


if __name__ == "__main__":
    default_matter_test_main()
