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
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --secured-device-port 5541
#       --KVS /tmp/chip_kvs_requestor
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --vendor-id 65521
#       --product-id 32769
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
# from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_2(MatterBaseTest):
    """

    """
    cluster_otap = Clusters.OtaSoftwareUpdateProvider
    cluster_otar = Clusters.OtaSoftwareUpdateRequestor

    async def write_acl(self, controller, acl):
        """
        Writes the Access Control List (ACL) to the DUT device using the specified controller.

        Args:
            controller: The Matter controller (e.g., th1, th4) that will perform the write operation.
            acl (list): List of AccessControlEntryStruct objects defining the ACL permissions to write.

        Raises:
            AssertionError: If writing the ACL attribute fails (status is not Status.Success).
        """
        result = await controller.WriteAttribute(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Attributes.Acl(acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

    def desc_TC_SU_2_2(self) -> str:
        return "[TC-SU-2.2] Handling Different QueryImageResponse Scenarios on Requestor"

    def pics_TC_SU_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.S",      # Pics
        ]
        return pics

    def steps_TC_SU_2_2(self) -> list[TestStep]:
        # TODO: In progress
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "Set ImageURI to the location where the image is located..",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT."),
            # TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to 'Busy', DelayedActionTime is set to 60 seconds.",
            #          "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec " \
            #          "which is 2 minutes (120 seconds) from the last QueryImage command."),
            # TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to 'NotAvailable'.",
            #          "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec " \
            #          "which is 2 minutes (120 seconds) from the last QueryImage command."),
            # TestStep(4, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to Busy, Set DelayedActionTime to 3 minutes. On the subsequent QueryImage command, " \
            #          "TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'.",
            #          "Verify that the DUT waits for at least the time mentioned in the DelayedActionTime (3 minutes) before issuing another QueryImage command to the TH/OTA-P."
            #          "Verify that there is a transfer of the software image after the second QueryImageResponse with UpdateAvailable status from the TH/OTA-P to the DUT."),
            # TestStep(5, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
            #          "ImageURI should have the https url from where the image can be downloaded.",
            #          "Verify that the DUT queries the https url and downloads the software image."),
            # TestStep(6, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
            #          "Software Version should be set to the same or an older (numerically lower) version.",
            #          "Verify that the DUT does not start transferring the software image."),
            # TestStep(7, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to 'UpdateAvailable', ImageURI field contains an invalid BDX ImageURI.",
            #          "Verify that the DUT does not start transferring the software image."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_2(self):
        # TODO: In progress
        # ------------------------------------------------------------------------------------
        # Manual Setup
        # ------------------------------------------------------------------------------------
        # 1. Launch OTA Requestor (TH1 / DUT) from Terminal:
        #     ./out/debug/chip-ota-requestor-app \
        #         --discriminator 1234 \
        #         --passcode 20202021 \
        #         --secured-device-port 5541 \
        #         --autoApplyImage \
        #         --KVS /tmp/chip_kvs_requestor
        # 2. Run Python test with commission ):
        #     python3 src/python_testing/TC_SU_2_2.py \
        #         --commissioning-method on-network \
        #         --discriminator 1234 \
        # ------------------------------------------------------------------------------------

        self.step(0)

        # Read the Steps

        self.step(1)
        logger.info("Step #1 - DUT sends QueryImage command to TH/OTA-P")

        # Establishing TH1 controller - DUT is TH1, NodeID=2, Fabric=1
        th1 = self.default_controller
        th1_node_id = self.dut_node_id
        th1_fabric_id = th1.fabricId
        logger.info(f'Step #1 - DUT NodeID (OTA Requestor): {th1_node_id}')
        logger.info(f'Step #1 - TH1 FabricID (Should be Fabric 1): {th1_fabric_id}')

        # 1.1 Send QueryImage command from DUT (requestor) to TH (provider)

        # 1.2 Verify that QueryImageResponse QueryStatus is UpdateAvailable

        # 1.3 Verify that software image transfer from TH/OTA-P to DUT is successful


if __name__ == "__main__":
    default_matter_test_main()
