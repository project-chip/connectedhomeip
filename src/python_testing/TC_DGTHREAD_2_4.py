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
#    run1:
#        app: ${ALL_CLUSTERS_APP}
#        app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#        script-args: >
#          --storage-path admin_storage.json
#          --commissioning-method on-network
#          --discriminator 1234
#          --passcode 20202021
#          --trace-to json:${TRACE_TEST_JSON}.json
#          --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#        factory-reset: true
#        quiet: true
# === END CI TEST ARGUMENTS ===
#

from mobly import asserts

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_THREADND_2_4(MatterBaseTest):
    """
    [TC-DGTHREAD-2.4] ResetCounts Command Verification with Server as DUT

    This test case verifies the ResetCounts command and subsequent OverrunCount attribute behavior.
    """

    async def send_reset_counts_command(self, endpoint):
        """Sends the ResetCounts command to the DUT."""
        cluster = Clusters.Objects.ThreadNetworkDiagnostics
        await self.send_single_cmd(cluster.Commands.ResetCounts(), endpoint=endpoint)

    async def read_thread_diagnostics_attribute_expect_success(self, endpoint, attribute):
        """
        Convenience method to read a single ThreadNetworkDiagnostics attribute,
        ensuring success.
        """
        cluster = Clusters.Objects.ThreadNetworkDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_THREADND_2_4(self) -> str:
        """Returns a description of this test."""
        return "[TC-DGWIFI-2.4] ResetCounts Command Verification with Server as DUT"

    def pics_TC_THREADND_2_4(self) -> list[str]:
        return ["DGWIFI.S", "DGWIFI.S.F.ERRCNT"]

    def steps_TC_THREADND_2_4(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep("2", "TH sends ResetCounts Command to DUT"),
            TestStep("3", "TH reads OverrunCount attribute from DUT"),
        ]
        return steps

    @async_test_body
    async def test_TC_THREADND_2_4(self):
        endpoint = self.get_endpoint(default=0)

        # STEP 1: Commission DUT (already done)
        self.step("1")
        # Typically, we assume commissioning was performed by harness scripts.
        attributes = Clusters.ThreadNetworkDiagnostics.Attributes

        # STEP 2: Send ResetCounts command
        self.step("2")
        await self.send_reset_counts_command(endpoint)

        # STEP 3: Verify OverrunCount attribute
        self.step("3")
        overrun_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.OverrunCount)
        if overrun_count is not None:
            # Verify that the OverrunCount is set to Zero
            asserts.assert_true(overrun_count == 0, "OverrunCount should be set to Zero")


if __name__ == "__main__":
    default_matter_test_main()
