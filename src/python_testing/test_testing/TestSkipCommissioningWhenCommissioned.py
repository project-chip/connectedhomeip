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

# Checks that the runner skips the commissioning step when the DUT is already commissioned
# on the controller's fabric. run1 declares fresh-dut so it always commissions from scratch;
# run2 reuses run1's controller storage and app KVS and must skip. Both runs pass
# --commissioning-method.
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
#       --bool-arg expect_commissioning_skipped:false
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     fresh-dut: true
#     quiet: true
#   run2:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --bool-arg expect_commissioning_skipped:true
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""
Runner-level check for skipping commissioning on an already commissioned DUT.

The runner records its decision in matter_test_config.commissioning_skipped; each run
declares what it expects, and the test also proves the DUT is usable by reading an
attribute over CASE.
"""

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TestSkipCommissioningWhenCommissioned(MatterBaseTest):

    @async_test_body
    async def test_runner_decision_matches_expectation(self):
        expected = self.user_params.get("expect_commissioning_skipped", False)
        asserts.assert_equal(
            self.matter_test_config.commissioning_skipped, expected,
            "Runner commissioning decision differs from what this run expects")

        vendor_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID, endpoint=0)
        asserts.assert_true(vendor_id > 0, "DUT must be reachable over CASE after the runner's decision")


if __name__ == "__main__":
    default_matter_test_main()
