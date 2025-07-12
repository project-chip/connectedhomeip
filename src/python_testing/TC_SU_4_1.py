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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
import logging

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_4_1(MatterBaseTest):
    def desc_TC_SU_4_1(self) -> str:
        return "[TC-SU-4.1] Verifying Cluster Attributes on OTA-R(DUT)"

    def pics_TC_SU_4_1(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.S",      # Pics
        ]
        return pics

    def steps_TC_SU_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, """TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. 
                     TH2 is set as the default Provider for the fabric."""),
            # TestStep(2, "TH..."),
            # TestStep(3, "TH..."),
            # TestStep(4, "TH..."),
            # TestStep(5, "TH..."),
            # TestStep(6, "TH..."),
            # TestStep(7, "TH..."),
            # TestStep(8, "TH..."),
            # TestStep(9, "TH..."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_4_1(self):
        cluster_otap = Clusters.OtaSoftwareUpdateProvider
        cluster_otar = Clusters.OtaSoftwareUpdateRequestor

        self.step(0)

    # Read the Steps
        self.step(1)

        # self.step(2)
        # self.step(3)
        # self.step(4)
        # self.step(5)
        # self.step(6)
        # self.step(7)
        # self.step(8)
        # self.step(9)
if __name__ == "__main__":
    default_matter_test_main()
