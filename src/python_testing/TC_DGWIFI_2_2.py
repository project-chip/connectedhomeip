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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from chip.testing.matter_testing import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGWIFI_2_2(MatterBaseTest):

    @staticmethod
    def is_valid_disconnection_event_data(event_data):
        """
        Check for Disconnection event data. Verify 'reasonCode' field is int16u.
        """
        return hasattr(event_data, "reasonCode") and (0 <= event_data.reasonCode <= 0xFFFF)

    @staticmethod
    def is_valid_association_failure_data(event_data):
        # Check for the `associationFailureCause` attribute and if its value
        # is within the range of the defined enum (0..3).
        cause_valid = (
            hasattr(event_data, "associationFailureCause")
            and event_data.associationFailureCause in Clusters.Objects.WiFiNetworkDiagnostics.Enums.AssociationFailureCauseEnum._value2member_map_
        )

        # Check for the `status` attribute and validate it's within the typical 802.11 range (0..65535).
        # In practice, you'd compare it against known 802.11 status codes (Table 9‑50).
        status_valid = (
            hasattr(event_data, "status")
            and 0 <= event_data.status <= 0xFFFF
        )

        return cause_valid and status_valid

    @staticmethod
    def is_valid_connection_status_data(event_data):
        # Check if the `connectionStatus` attribute is present
        # and if it's within the range of the defined enum.
        return (
            hasattr(event_data, "connectionStatus")
            and event_data.connectionStatus in Clusters.Objects.WiFiNetworkDiagnostics.Enums.ConnectionStatusEnum._value2member_map_
        )

    #
    # Methods to cause/triggers in your environment
    #
    async def send_wifi_disconnection_test_event_trigger(self):
        # Send test event trigger to programmatically cause a Wi-Fi disconnection in the DUT.
        await self.send_test_event_triggers(eventTrigger=0x0036000000000000)

    async def send_wifi_association_failure_test_event_trigger(self):
        # Send test event trigger to programmatically cause repeated association failures on the DUT.
        await self.send_test_event_triggers(eventTrigger=0x0036000000000001)

    async def send_wifi_reconnection_test_event_trigger(self):
        # Send test event trigger to programmatically reconnect the DUT to Wi-Fi.
        await self.send_test_event_triggers(eventTrigger=0x0036000000000002)

    #
    # Test description & PICS
    #
    def desc_TC_DGWIFI_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGWIFI-2.2] Wi-Fi Diagnostics Event Tests (Server as DUT)"

    def pics_TC_DGWIFI_2_2(self) -> list[str]:
        return ["DGWIFI.S"]

    def steps_TC_DGWIFI_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning (already done)", is_commissioning=True),
            TestStep(2, "Trigger Wi-Fi disconnection -> verify Disconnection event"),
            TestStep(3, "Trigger repeated association failures -> verify AssociationFailure event"),
            TestStep(4, "Reconnect Wi-Fi -> verify ConnectionStatus event"),
        ]
        return steps

    # ---------------------------
    # Main Test Routine
    # ---------------------------
    @async_test_body
    async def test_TC_DGWIFI_2_2(self):
        endpoint = self.get_endpoint(default=0)

        #
        # STEP 1: Commission DUT (already done)
        #
        self.step(1)

        #
        # Create and start an EventChangeCallback to subscribe for events
        #
        events_callback = EventChangeCallback(Clusters.WiFiNetworkDiagnostics)
        await events_callback.start(
            self.default_controller,     # The controller
            self.dut_node_id,            # DUT's node id
            endpoint                     # The endpoint on which we expect Wi-Fi events
        )

        #
        # STEP 2: Cause a Wi-Fi Disconnection -> wait for Disconnection event
        #
        self.step(2)
        await self.send_wifi_disconnection_test_event_trigger()

        # Wait (block) for the Disconnection event to arrive
        event_data = events_callback.wait_for_event_report(
            Clusters.WiFiNetworkDiagnostics.Events.Disconnection
        )

        # Validate the Disconnection event fields
        asserts.assert_true(
            self.is_valid_disconnection_event_data(event_data),
            f"Invalid Disconnection event data: {event_data}"
        )

        #
        # STEP 3: Cause repeated association failures -> wait for AssociationFailure event
        #
        self.step(3)
        await self.send_wifi_association_failure_test_event_trigger()

        event_data = events_callback.wait_for_event_report(
            Clusters.WiFiNetworkDiagnostics.Events.AssociationFailure
        )

        # Validate the AssociationFailure event fields
        asserts.assert_true(
            self.is_valid_association_failure_data(event_data),
            f"Invalid AssociationFailure event data: {event_data}"
        )

        #
        # STEP 4: Reconnect Wi-Fi -> wait for ConnectionStatus event
        #
        self.step(4)
        await self.send_wifi_reconnection_test_event_trigger()

        event_data = events_callback.wait_for_event_report(
            Clusters.WiFiNetworkDiagnostics.Events.ConnectionStatus
        )

        # Validate the ConnectionStatus event fields
        asserts.assert_true(
            self.is_valid_connection_status_data(event_data),
            f"Invalid ConnectionStatus event data: {event_data}"
        )


if __name__ == "__main__":
    default_matter_test_main()
