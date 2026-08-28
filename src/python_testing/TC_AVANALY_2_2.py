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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --bool-arg PIXIT.CANBEMADEBUSY:False
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_AVANALY_2_2(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_2(self) -> str:
        return "[TC-AVANALY-2.2] Validate writeable attributes with Server as DUT"

    def steps_TC_AVANALY_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads the TrackingEnabled attribute. Verify that it is a bool with a value of false. Store in aTrackingEnabled"),
            TestStep(3, "TH writes to TrackingEnabled the value of !aTrackingEnabled. Verify success response."),
            TestStep(4, "TH reads the TrackingEnabled attribute. Verify that it is set to !aTrackingEnabled,",
                        "then reset the TrackingEnabled attribute to its original value.")
        ]

    def pics_TC_AVANALY_2_2(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_2(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        tracking_enabled_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.TrackingEnabled)
        asserts.assert_false(tracking_enabled_dut, "TrackingEnabled should be false on startup")

        # Wrap in try .. finally to make sure that even if there is an error, we revert the attribute value
        try:
            self.step(3)
            result = await self.write_single_attribute(attributes.TrackingEnabled(not tracking_enabled_dut),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write TrackingEnabled")

            self.step(4)
            tracking_enabled_dut_new = await self.read_avanaly_attribute_expect_success(endpoint, attributes.TrackingEnabled)

            asserts.assert_equal(tracking_enabled_dut_new, not tracking_enabled_dut,
                                 "Value does not match what was written in step 3")
                             
        finally:
            result = await self.write_single_attribute(attributes.TrackingEnabled(tracking_enabled_dut),
                                                       endpoint_id=endpoint)

if __name__ == "__main__":
    default_matter_test_main()
