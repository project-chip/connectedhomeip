#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main


class TC_WebRTCP_2_6(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_6(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.6] Validate CameraAVStreamManagement cluster presence on endpoint"

    def steps_TC_WebRTCP_2_6(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH reads the descriptor cluster on the endpoint containing the WebRTC Transport Provider cluster"),
            TestStep(2, "TH verifies CameraAVStreamManagement cluster (ID 0x0551) is present in the server cluster list"),
        ]

    def pics_TC_WebRTCP_2_6(self) -> list[str]:
        """
        Return the list of PICS applicable to this test case.
        """
        return [
            "WEBRTCP.S",           # WebRTC Transport Provider Server
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WebRTCP_2_6(self):
        """
        Executes the test steps for validating CameraAVStreamManagement cluster presence.
        """

        self.step("precondition")
        # Commission DUT - already done
        webrtc_endpoint = self.get_endpoint()

        self.step(1)
        # Read the descriptor cluster on the endpoint containing the WebRTC Transport Provider cluster
        server_list = await self.read_single_attribute_check_success(
            endpoint=webrtc_endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

        # Verify WebRTC Transport Provider cluster is present (this should be true since we found this endpoint)
        webrtc_cluster_id = Clusters.WebRTCTransportProvider.id
        asserts.assert_in(webrtc_cluster_id, server_list,
                          f"WebRTC Transport Provider cluster (0x{webrtc_cluster_id:04X}) not found in server list")

        self.step(2)
        # Verify CameraAVStreamManagement cluster (ID 0x0551) is present in the server cluster list
        camera_avsm_cluster_id = Clusters.CameraAvStreamManagement.id
        asserts.assert_in(camera_avsm_cluster_id, server_list,
                          f"CameraAVStreamManagement cluster (0x{camera_avsm_cluster_id:04X}) not found on the same endpoint as WebRTC Transport Provider cluster")

        # Log success for clarity
        self.print_step(
            2, f"✓ CameraAVStreamManagement cluster (0x{camera_avsm_cluster_id:04X}) found on endpoint {webrtc_endpoint}")
        self.print_step(
            2, f"✓ Both WebRTC Transport Provider and CameraAVStreamManagement clusters are present on endpoint {webrtc_endpoint}")


if __name__ == "__main__":
    default_matter_test_main()
