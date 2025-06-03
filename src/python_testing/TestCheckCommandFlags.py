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

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

""" Command flags test for error path returns via the UnitTesting cluster.
"""


class TestCheckCommandFlags(MatterBaseTest):

    @async_test_body
    async def test_invalid_transport_type_for_large_payload(self):

        self.print_step(0, "Commissioning - already done")
        cmd = Clusters.UnitTesting.Commands.TestCheckCommandFlags()

        # Send command requiring large payload session over a regular MRP
        # session to receive InvalidTransportType status code.
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=1, timedRequestTimeoutMs=1000)
            asserts.fail("Unexpected success for Command requiring LargePayload session. Expecting InvalidTransportType error")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidTransportType,
                                 "Failed to verify INVALID_TRANSPORT_TYPE for a LargePayload message")

        # Send the command requiring large payload session over a TCP session to
        # receive Success status code.
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=1, timedRequestTimeoutMs=1000,
                                       payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")

        logging.info("Test completed")


if __name__ == "__main__":
    default_matter_test_main()
