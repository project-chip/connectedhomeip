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
#     app: ${all-clusters}
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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_DGSW_2_2(MatterBaseTest):

    async def send_software_fault_test_event_trigger(self):
        await self.send_test_event_triggers(eventTrigger=0x0034000000000000)

    def validate_soft_fault_event_data(self, event_data):
        """
        Validates the SoftFault event data according to the test plan and specification.
        This method checks:
          - `Id` field: Must be of type uint64
          - `Name` field: Vendor-specific string
          - `FaultRecording` field: Vendor-specific payload in octet string format (bytes/bytearray)
        """

        # Validate 'Id' field: Ensure it is a uint64 type
        matter_asserts.assert_valid_uint64(event_data.id, "Id")

        # Validate 'Name' field: Ensure it is a string
        matter_asserts.assert_is_string(event_data.name, "Name")

        # Validate 'FaultRecording' field: Ensure it is an octet string (bytes or bytearray)
        matter_asserts.assert_is_octstr(event_data.faultRecording, "FaultRecording")

    def desc_TC_DGSW_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGSW-2.2] Event Functionality with Server as DUT"

    def pics_TC_DGSW_2_2(self) -> list[str]:
        return ["DGSW.S.E00"]

    def steps_TC_DGSW_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH subscribes to the SoftwareDiagnostics cluster in the DUT to receive SoftwareFault events. "
                     "The DUT is triggered to emit a SoftwareFault event. Wait for the SoftwareFault event to arrive.",
                     "Validate the SoftwareFault event fields."
                     "The Id field of the struct is mandatory and shall be set with software thread ID that last software fault occurred."
                     "Name field shall be set to vendor specific name strings that last software fault occurred."
                     "FaultRecording field shall be set by a vendor specific payload in octstr format."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SoftwareDiagnostics))
    async def test_TC_DGSW_2_2(self):

        endpoint = self.get_endpoint()

        # STEP 1: Commission DUT (already done)
        self.step(1)

        # STEP 2: DUT sends an event report to TH. TH reads a list of SoftwareFault structs from DUT.
        self.step(2)

        # Create and start an EventSubscriptionHandler to subscribe for events
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.SoftwareDiagnostics)
        await events_callback.start(
            self.default_controller,     # The controller
            self.dut_node_id,            # DUT's node id
            endpoint                     # The endpoint on which we expect SoftwareDiagnostics events
        )

        # Trigger a SoftwareFault event on the DUT
        await self.send_software_fault_test_event_trigger()

        # Wait (block) for the SoftwareFault event to arrive
        event_data = events_callback.wait_for_event_report(
            Clusters.SoftwareDiagnostics.Events.SoftwareFault
        )

        # Validate the SoftwareFault event fields
        self.validate_soft_fault_event_data(event_data)


if __name__ == "__main__":
    default_matter_test_main()
