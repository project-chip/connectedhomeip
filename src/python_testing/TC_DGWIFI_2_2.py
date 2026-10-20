#
#    Copyright (c) 2026 Project CHIP Authors
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
#     app-args: --discriminator 1234 --KVS kvs1 --enable-key 000102030405060708090a0b0c0d0e0f --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging
from typing import Any

from mobly import asserts

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, pics, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# Test event trigger codes from WiFiDiagnosticsTestEventTriggerHandler.h, also
# defined as PIXIT.DGWIFI.TEST_EVENT_TRIGGER values in the test plan.
TRIGGER_DISCONNECTION = 0x0036000000000000
TRIGGER_ASSOCIATION_FAILURE = 0x0036000000000001
TRIGGER_CONNECTION_STATUS = 0x0036000000000002


class TC_DGWIFI_2_2(MatterTestCommissionedDevice):

    def _assert_defined_enum(self, value: Any, enum_type: type, description: str):
        """Asserts the value is a defined value of the enum, rejecting the unknown sentinel."""
        matter_asserts.assert_valid_enum(value, description, enum_type)
        # Raw values outside the defined enum values decode to the kUnknownEnumValue
        # sentinel, which is an instance of the enum type and passes the check above.
        asserts.assert_not_equal(value, enum_type.kUnknownEnumValue,
                                 f"{description} is not a defined {enum_type.__name__} value.")

    @pics("DGWIFI.S")
    @run_if_endpoint_matches(has_cluster(Clusters.WiFiNetworkDiagnostics))
    async def test_TC_DGWIFI_2_2(self):
        """[TC-DGWIFI-2.2] Event Functionality with Server as DUT"""

        endpoint = self.get_endpoint()
        events = Clusters.WiFiNetworkDiagnostics.Events
        enums = Clusters.WiFiNetworkDiagnostics.Enums
        event_pics = ("DGWIFI.S.E00", "DGWIFI.S.E01", "DGWIFI.S.E02")

        self.step(1, "Commission DUT to TH if not already commissioned")

        self.step(2, "TH reads the TestEventTriggersEnabled attribute from the General Diagnostics cluster. "
                     "TH subscribes to the WiFiNetworkDiagnostics cluster events on the DUT. "
                     "The DUT is triggered to simulate a disconnection via de-authentication or "
                     "dis-association. TH waits for the Disconnection event.",
                  expectation="Verify that the TestEventTriggersEnabled attribute has a value of 1 (True), "
                              "that TH receives the Disconnection event, and that the ReasonCode field "
                              "is a uint16 value.")
        if any(self.check_pics(p) for p in event_pics):
            await self.check_test_event_triggers_enabled()
        # Simulated disconnections should not affect the Matter link, but let the subscription
        # re-establish itself in case a DUT implementation briefly drops it.
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.WiFiNetworkDiagnostics)
        await events_callback.start(self.default_controller, self.dut_node_id, endpoint, autoResubscribe=True)
        if self.pics_guard(self.check_pics("DGWIFI.S.E00")):
            events_callback.reset()
            await self.send_test_event_triggers(eventTrigger=TRIGGER_DISCONNECTION)
            event_data = events_callback.wait_for_event_type_report(events.Disconnection, timeout_sec=10.0)
            matter_asserts.assert_valid_uint16(event_data.reasonCode, "ReasonCode")
            logger.info("Disconnection: reasonCode=%s", event_data.reasonCode)

        self.step(3, "The DUT is triggered to simulate exhausting all internal retries when "
                     "attempting to connect to a Wi-Fi access point. TH waits for the "
                     "AssociationFailure event.",
                  expectation="Verify TH receives the AssociationFailure event, that the AssociationFailureCause "
                              "field is a value specified by the AssociationFailureCause enum, and that the "
                              "Status field is a uint16 value.")
        if self.pics_guard(self.check_pics("DGWIFI.S.E01")):
            events_callback.reset()
            await self.send_test_event_triggers(eventTrigger=TRIGGER_ASSOCIATION_FAILURE)
            event_data = events_callback.wait_for_event_type_report(events.AssociationFailure, timeout_sec=10.0)
            self._assert_defined_enum(event_data.associationFailureCause,
                                      enums.AssociationFailureCauseEnum, "AssociationFailureCause")
            matter_asserts.assert_valid_uint16(event_data.status, "Status")
            logger.info("AssociationFailure: associationFailureCause=%s status=%s",
                        event_data.associationFailureCause, event_data.status)

        self.step(4, "The DUT is triggered to simulate a Wi-Fi disconnection and reconnection. "
                     "TH waits for the ConnectionStatus event.",
                  expectation="Verify TH receives the ConnectionStatus event and that the ConnectionStatus "
                              "field is a value specified by the ConnectionStatus enum.")
        if self.pics_guard(self.check_pics("DGWIFI.S.E02")):
            events_callback.reset()
            await self.send_test_event_triggers(eventTrigger=TRIGGER_CONNECTION_STATUS)
            event_data = events_callback.wait_for_event_type_report(events.ConnectionStatus, timeout_sec=10.0)
            self._assert_defined_enum(event_data.connectionStatus,
                                      enums.ConnectionStatusEnum, "ConnectionStatus")
            logger.info("ConnectionStatus: connectionStatus=%s", event_data.connectionStatus)


if __name__ == "__main__":
    default_matter_test_main()
