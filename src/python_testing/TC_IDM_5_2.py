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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.conformance import ConformanceException
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main
from matter.testing.spec_parsing import dm_from_spec_version

log = logging.getLogger(__name__)


class TC_IDM_5_2(IDMBaseTest, BasicCompositionTests):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def steps_TC_IDM_5_2(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH sends a Timed Request Message with the timeout value set. (Example - 200 milliseconds).",
                     "On the TH verify the DUT sends a status response back to TH."),
            TestStep(2, "TH sends a Timed Request Message(Timed Write Transaction) with the timeout value set. (Example - 200 milliseconds)." +
                     "Wait for the status response message to be received. Send the Write Request Message to the DUT.",
                     "On the TH verify DUT sends back a Write Response after performing the write action. Verify by sending a ReadRequest that the Write action was performed correctly."),
            TestStep(3, "TH sends a Timed Request Message(Timed Invoke Transaction) with the timeout value set. (Example - 200 milliseconds)" +
                     "Wait for the status response message to be received. Wait for 5 seconds(Timer has expired) and then send the Invoke Request Message to the DUT.",
                     "If the device being certified is Matter release 1.4 or later, timeout error should be returned. If the device being certified is Matter release 1.3 or earlier, verify the DUT sends back a Status response with either timeout or unsupported access error."),
            TestStep(4, "TH sends a Timed Request Message(Timed Write Transaction) with the timeout value set. (Example - 200 milliseconds)." +
                     "Wait for the status response message to be received.  Wait for 5 seconds(Timer has expired) and then send the Write Request Message to the DUT.",
                     "If the device being certified is Matter release 1.4 or later, timeout error should be returned. If the device being certified is Matter release 1.3 or earlier, verify the DUT sends back a Status response with either timeout or unsupported access error."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.LevelControl))
    async def test_TC_IDM_5_2(self):
        self.step(0)
        self.endpoint = self.get_endpoint()

        # Test Setup with robust endpoint/cluster discovery
        await self.setup_class_helper(allow_pase=False)

        # Step 1: TH sends a Timed Request Message with timeout (e.g., 200ms) for an invoke command
        self.step(1)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.endpoint,
            payload=Clusters.LevelControl.Commands.MoveToLevel(19),
            timedRequestTimeoutMs=200,
        )

        # Step 2: TH sends a Timed Request Message (Timed Write Transaction) with timeout,
        # waits for status response, then sends Write Request
        TIMED_REQUEST_TIMEOUT_MS = 500
        self.step(2)
        expected_value = 21
        write_result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            attributes=[(self.endpoint, Clusters.LevelControl.Attributes.OnLevel(expected_value))],
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify write was successful
        asserts.assert_equal(write_result[0].Status, Status.Success,
                             f"Step 2: Timed write should succeed, got {write_result[0].Status}")

        # Read back to verify the write was performed
        read_result = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(self.endpoint, Clusters.LevelControl.Attributes.OnLevel)]
        )

        new_value = read_result[self.endpoint][Clusters.LevelControl][Clusters.LevelControl.Attributes.OnLevel]
        asserts.assert_equal(new_value, expected_value,
                             f"Step 2: Read back value after timed write should be {expected_value}, got {new_value}")

        # Step 3: TH sends a Timed Request Message (Timed Invoke Transaction) with timeout,
        # waits for status response, waits 5 seconds (timer expired), then sends Invoke Request
        SPEC_VERSION_1_4 = 0x00010400
        self.step(3)
        spec_version = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.SpecificationVersion)
        try:
            dm_from_spec_version(spec_version)
        except ConformanceException:
            asserts.fail(f'Unknown SpecificationVersion {spec_version:08X}')
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.endpoint,
                payload=Clusters.LevelControl.Commands.MoveToLevel(23),
                timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
                busyWaitMs=TIMED_REQUEST_TIMEOUT_MS * 2  # Wait longer than timeout to trigger TIMEOUT error
            )
            asserts.fail("Step 3: Command should timeout but succeeded unexpectedly")
        except InteractionModelError as e:
            if spec_version >= SPEC_VERSION_1_4:  # Matter release 1.4 or later
                asserts.assert_equal(e.status, Status.Timeout,
                                     f"SendCommand should return TIMEOUT, got {e.status}")
            else:  # Matter release 1.3 or earlier
                asserts.assert_in(e.status, [Status.Timeout, Status.UnsupportedAccess],
                                  f"SendCommand should return TIMEOUT or UNSUPPORTED_ACCESS, got {e.status}")

        # Step 4: TH sends a Timed Request Message (Timed Write Transaction) with timeout,
        # waits for status response, waits 5 seconds (timer expired), then sends Write Request
        self.step(4)
        try:
            write_result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                attributes=[(self.endpoint, Clusters.LevelControl.Attributes.OnLevel(25))],
                timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
                busyWaitMs=TIMED_REQUEST_TIMEOUT_MS * 2  # Wait longer than timeout to trigger TIMEOUT error
            )
            asserts.fail("Step 4: Write should timeout but succeeded unexpectedly")
        except InteractionModelError as e:
            if spec_version >= SPEC_VERSION_1_4:  # Matter release 1.4 or later
                asserts.assert_equal(e.status, Status.Timeout,
                                     f"WriteAttribute should return TIMEOUT, got {e.status}")
            else:  # Matter release 1.3 or earlier
                asserts.assert_in(e.status, [Status.Timeout, Status.UnsupportedAccess],
                                  f"WriteAttribute should return TIMEOUT or UNSUPPORTED_ACCESS, got {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
