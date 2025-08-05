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
import asyncio

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
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
        return True

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
        # 1. Launch OTA Provider (TH2) from Terminal 1:
        #     ./out/debug/chip-ota-provider-app --filepath firmware_v2.ota
        # 2. Manually commission the OTA Provider using Node ID 1 (Terminal 2):
        #     ./out/chip-tool/chip-tool pairing onnetwork 1 20202021
        # 3. Launch OTA Requestor (TH1 / DUT) from Terminal 3:
        #     ./out/debug/chip-ota-requestor-app \
        #         --discriminator 1234 \
        #         --passcode 20202021 \
        #         --secured-device-port 5541 \
        #         --autoApplyImage \
        #         --KVS /tmp/chip_kvs_requestor
        # 4. Run Python test with commission Provisioner/Requestor (Terminal 2):
        #     python3 src/python_testing/TC_SU_2_2.py \
        #         --commissioning-method on-network \
        #         --discriminator 1234 \
        #         --passcode 20202021 \
        #         --vendor-id 65521 \
        #         --product-id 32769 \
        #         --nodeId 2
        #     python3 src/python_testing/TC_SU_2_2.py \
        #         --commissioning-method on-network \
        #   --discriminator 1234 \
        #   --passcode 20202021 \
        #   --vendor-id 65521 \
        #   --product-id 32769 \
        #   --nodeId 2

        # ------------------------------------------------------------------------------------

        self.step(0)

        # Read the Steps
        self.step(1)
        # 1.0 Establishing TH1 controller - DUT is TH1, NodeID=2, Fabric=1
        th1 = self.default_controller
        th1_node_id = self.dut_node_id
        th1_fabric_id = th1.fabricId
        logger.info(f'Step #1.0 - DUT NodeID (OTA Requestor): {th1_node_id}')
        logger.info(f'Step #1.0 - TH1 FabricID (Should be Fabric 1): {th1_fabric_id}')

        # Read the actual value of DefaultOTAProviders attribute on the DUT (TH1, NodeID=2)
        actual_otap_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #1.0 - Read actaul DefaultOTAProviders value on DUT (TH1): {actual_otap_info}')

        # TH2 is the OTA Provider (NodeID=1) for fabric 1
        provider_th2_for_fabric1 = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=1,   # TH2 is the OTA Provider (NodeID=1)
            endpoint=0,
            fabricIndex=1       # Fabric ID from TH1 (controller writing to DUT)
        )

        # DefaultOTAProviders attribute with the provider list
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=[provider_th2_for_fabric1])

        # Write the DefaultOTAProviders attribute to the DUT (TH1)
        resp = await self.write_single_attribute(
            attribute_value=attr,
            endpoint_id=0
        )
        logger.info(f'Step #1.0 - Write DefaultOTAProviders response: {resp}')
        # Verify write succeeded (response code 0)
        asserts.assert_equal(resp, Status.Success, "Failed to write DefaultOTAProviders attribute")

        # 1.1 Establishing TH2 controller - TH2, NodeID=1, Fabric=1
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=th1.fabricId + 1)
        th2 = th2_fabric_admin.NewController(nodeId=1, useTestCommissioner=True)

        th2_node_id = th2.nodeId
        th2_fabric_id = th2.fabricId
        logger.info(f'Step #1.1 - TH2 (PROVIDER) NodeID: {th2_node_id}')
        logger.info(f'Step #1.1 - TH2 (PROVIDER) FabricID: {th2_fabric_id}')

        params = await self.open_commissioning_window(th1, th1_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        # setup_qr_code = params.commissioningParameters.setupQRCode
        logger.info(f'Step #1.1: Commissioning window opened: {vars(params)}')

        logger.info('Step #1.1 - Commissioning DUT with TH2...')
        resp = await th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=long_discriminator)
        logger.info(f'Step #1.1 - TH2 Commissioning response: {resp}')

        # ACL on DUT to allow access from TH2 (Provider)
        admin_acl_for_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th2_node_id],
            targets=[]
        )

        resp = await self.write_acl(th1, [admin_acl_for_provider])
        logger.info(f'Step #1.1 - Wrote ACL on DUT (TH1) to allow access to Provider (TH2): {resp}')

        # ACL on Provider to allow access from DUT (Requestor)
        admin_acl_for_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th1_node_id],
            targets=[]
        )

        resp = await self.write_acl(th2, [admin_acl_for_requestor])
        logger.info(f'Step #1.1 - Wrote ACL on Provider (TH2) to allow access from DUT (TH1): {resp}')

        # 1.2 TH2 (Provider) sends AnnounceOTAProvider command to TH1 (DUT/Requestor)
        logger.info("Step #1.2 - DUT sends QueryImage command to TH/OTA-P")
        cmd = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=1,
            vendorID=0xFFF1,
            announcementReason=Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUrgentUpdateAvailable,
            metadataForNode=None,
            endpoint=0
        )
        logger.info(f"Step #1.2 - cmd AnnounceOTAProvider: {cmd}")

        resp = await self.send_single_cmd(
            dev_ctrl=th2,
            node_id=th1_node_id,
            cmd=cmd

        )
        logger.info(f"Step #1.2 - Sent AnnounceOTAProvider to DUT, response: {resp}")

        # TODO: Need to make it work from here
        logger.info("Step #1.3 - ArmFailSafe")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=60,
            breadcrumb=1
        )
        resp = await self.send_single_cmd(
            dev_ctrl=th1,
            node_id=th1_node_id,
            cmd=cmd,
            endpoint=0
        )
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        logger.info("Step #1.3 - Wait")
        await asyncio.sleep(5)
        # ******
        logger.info("Step #1.4 - DUT send QueryImage to Provider and wait for response")
        logger.info(f"Step #1.4 - Info requestor(th1): {vars(th1)}")
        # product_id = await self.read_single_attribute(th1, endpoint=0, cluster="BasicInformation", attribute="productID")
        # logger.info(f"ProductId del requestor: {th1.productId}")

        cmd_query_image = Clusters.OtaSoftwareUpdateProvider.Commands.QueryImage(
            vendorID=0xFFF1,
            productID=0x8000,
            softwareVersion=0,
            protocolsSupported=[Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kBDXSynchronous],
            hardwareVersion=None,
            location=None,
            requestorCanConsent=None,
            metadataForProvider=None
        )

        logger.info("Step #1.4 - Send QueryImage from DUT to Provider")
        resp = await self.send_single_cmd(
            dev_ctrl=th1,
            node_id=th2_node_id,
            cmd=cmd_query_image,
            endpoint=0
        )
        logger.info(f"Step #1.4 - Response from Provider: {resp}")

        # 1.5 Verify that QueryImageResponse QueryStatus is UpdateAvailable
        # 1.6 Verify that software image transfer from TH/OTA-P to DUT is successful


if __name__ == "__main__":
    default_matter_test_main()
