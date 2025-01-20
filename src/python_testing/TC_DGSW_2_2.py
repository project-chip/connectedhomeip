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


class TC_DGSW_2_2(MatterBaseTest):

    @staticmethod
    def is_valid_uint64_value(value):
        return isinstance(value, int) and 0 <= value <= 0xFFFFFFFFFFFFFFFF

    @staticmethod
    def is_valid_octet_string(value):
        return isinstance(value, (bytes, bytearray))

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
        asserts.assert_true(
            self.is_valid_uint64_value(event_data.id),
            "The 'Id' field must be a uint64 type"
        )

        # Validate 'Name' field: Ensure it is a string
        asserts.assert_true(
            isinstance(event_data.name, str),
            "The 'Name' field must be a string type"
        )

        # Validate 'FaultRecording' field: Ensure it is an octet string (bytes or bytearray)
        asserts.assert_true(
            self.is_valid_octet_string(event_data.faultRecording),
            "The 'FaultRecording' field must be an octet string (bytes or bytearray)"
        )

    def desc_TC_DGSW_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGSW-2.2] Attributes with Server as DUT"

    def pics_TC_DGSW_2_2(self) -> list[str]:
        return ["DGSW.S"]

    def steps_TC_DGSW_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the SoftwareFault event(s) from the DUT"),
        ]
        return steps

    @async_test_body
    async def test_TC_DGSW_2_2(self):

        endpoint = self.get_endpoint(default=0)

        # STEP 1: Commission DUT (already done)
        self.step(1)

        # Create and start an EventChangeCallback to subscribe for events
        events_callback = EventChangeCallback(Clusters.SoftwareDiagnostics)
        await events_callback.start(
            self.default_controller,     # The controller
            self.dut_node_id,            # DUT's node id
            endpoint                     # The endpoint on which we expect Wi-Fi events
        )

        # STEP 2: DUT sends an event report to TH. TH reads a list of SoftwareFault structs from DUT.
        self.step(2)

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
