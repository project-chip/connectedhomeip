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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
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

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TestInvokeReturnCodes(MatterBaseTest):
    """
    Validates that the invoke action correctly refuses commands
    on invalid endpoints.
    """

    @async_test_body
    async def test_invalid_endpoint_command(self):
        self.print_step(0, "Commissioning - already done")

        self.print_step(1, "Find an invalid endpoint id")
        root_parts = await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            endpoint=0,
        )
        endpoints = set(root_parts)
        invalid_endpoint_id = 1
        while invalid_endpoint_id in endpoints:
            invalid_endpoint_id += 1

        self.print_step(
            2,
            "Attempt to invoke SoftwareDiagnostics::ResetWatermarks on an invalid endpoint",
        )
        try:
            await self.send_single_cmd(
                cmd=Clusters.SoftwareDiagnostics.Commands.ResetWatermarks(),
                endpoint=invalid_endpoint_id,
            )
            asserts.fail("Unexpected command success on an invalid endpoint")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.UnsupportedEndpoint, "Unexpected error returned"
            )


if __name__ == "__main__":
    default_matter_test_main()
