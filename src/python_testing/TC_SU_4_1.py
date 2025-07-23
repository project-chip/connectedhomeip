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

import logging
import random
from typing import Optional

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_4_1(MatterBaseTest):
    """

    """
    cluster_otap = Clusters.OtaSoftwareUpdateProvider
    cluster_otar = Clusters.OtaSoftwareUpdateRequestor

    def desc_TC_SU_4_1(self) -> str:
        return "[TC-SU-4.1] Verifying Cluster Attributes on OTA-R(DUT)"

    def pics_TC_SU_4_1(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.S",      # Pics
        ]
        return pics

    async def OpenCommissioningWindow(self, th: ChipDeviceCtrl, expectedErrCode: Optional[Clusters.AdministratorCommissioning.Enums.StatusCode] = None) -> CommissioningParameters:
        if expectedErrCode == 0x00:
            params = await th.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        else:
            ctx = asserts.assert_raises(ChipStackError)
            with ctx:
                await th.OpenCommissioningWindow(
                    nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=10000, discriminator=self.discriminator, option=1)
            errcode = ctx.exception.chip_error
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                'Unexpected error code returned from CommissioningComplete')

    async def CommissionAttempt(self, setupPinCode: int, thnum: int, th):
        await th.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminator)

    def steps_TC_SU_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. TH2 is set as the default Provider for the fabric.",
                     "Verify that the write operation for the attribute works and DUT does not respond with any errors."),
            TestStep(2, "TH sends a read request to read the DefaultOTAProviders Attribute on the first fabric to the DUT.",
                     "Verify that the attribute value is set to TH2 as the default OTA provider for the fabric."),
            TestStep(3, "TH sends a write request for the DefaultOTAProviders Attribute on the second fabric to the DUT. TH3 is set as the default Provider for the fabric.",
                     "Verify that the write operation for the attribute works and DUT does not respond with any errors."),
            TestStep(4, "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT."
                     "Verify that the attribute value is set to TH2 as the default OTA provider for the first fabric and TH3 for the second fabric.")
            # TestStep(5, "TH..."),
            # TestStep(6, "TH..."),
            # TestStep(7, "TH..."),
            # TestStep(8, "TH..."),
            # TestStep(9, "TH..."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_4_1(self):
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
        #     python3 src/python_testing/TC_SU_4_1.py \
        #         --commissioning-method on-network \
        #         --discriminator 1234 \
        #         --passcode 20202021 \
        #         --vendor-id 65521 \
        #         --product-id 32769 \
        #         --nodeId 2
        # ------------------------------------------------------------------------------------

        self.step(0)

        # Read the Steps

        self.step(1)

        # Establishing TH1 controller - DUT is TH1, NodeID=2, Fabric=1
        th1 = self.default_controller
        th1_node_id = self.dut_node_id
        th1_fabric_id = th1.fabricId
        logger.info(f'Step #1 - DUT NodeID (OTA Requestor): {th1_node_id}')
        logger.info(f'Step #1 - TH1 FabricID (Should be Fabric 1): {th1_fabric_id}')

        # Read the actual value of DefaultOTAProviders attribute on the DUT (TH1, NodeID=2)
        actual_otap_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #1 - Read actaul DefaultOTAProviders value on DUT (TH1): {actual_otap_info}')

        # TH2 is the OTA Provider (NodeID=1) for fabric 1
        provider_th2_for_fabric1 = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=1,   # TH2 is the OTA Provider (NodeID=1)
            endpoint=0,
            fabricIndex=1       # Fabric ID from TH1 (controller writing to DUT)
        )

        # Create Providers list and Add TH2 to Providers list
        providers_list = [provider_th2_for_fabric1]
        logger.info(f'Step #1 - Providers list updated with provider "TH2 for fabric 1": {providers_list}')

        # DefaultOTAProviders attribute with the provider list
        attr = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=providers_list)

        # Write the DefaultOTAProviders attribute to the DUT (TH1)
        resp = await self.write_single_attribute(
            attribute_value=attr,
            endpoint_id=0
        )
        logger.info(f'Step #1 - Write DefaultOTAProviders response: {resp}')

        # Verify write succeeded (response code 0)
        asserts.assert_equal(resp, 0, "Failed to write DefaultOTAProviders attribute")

        self.step(2)
        # Verify DefaultOTAProviders attribute on the DUT (TH1) after write (using TH1 on Fabric 1)
        actual_otap_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #2 - DefaultOTAProviders value read from DUT (TH1): {actual_otap_info}')

        # Verify that the provider list is not empty (expecting at least one provider)
        asserts.assert_true(len(actual_otap_info) > 0, "DefaultOTAProviders list is empty")

        # Get the first provider from the list for verification
        actual_provider = actual_otap_info[0]
        logger.info(f'Step #2 - Read DefaultOTAProviders value after write on DUT (TH1): {actual_provider}')

        # Verify the actual provider matches the expected OTA Provider (TH2)
        # TH1 = DUT (NodeID=2), TH2 = OTA Provider (NodeID=1)
        asserts.assert_equal(actual_provider.providerNodeID, provider_th2_for_fabric1.providerNodeID, "Mismatch in providerNodeID")
        asserts.assert_equal(actual_provider.endpoint, provider_th2_for_fabric1.endpoint, "Mismatch in endpoint")
        asserts.assert_equal(actual_provider.fabricIndex, provider_th2_for_fabric1.fabricIndex, "Mismatch in fabricIndex")
        logger.info("Step #2 - DefaultOTAProviders attribute matches expected values.")

        self.step(3)

        # Establishing TH3 controller - TH3, NodeID=3, Fabric=2
        th3_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th3_fabric_admin = th3_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=th1.fabricId + 1)
        th3 = th3_fabric_admin.NewController(nodeId=3, useTestCommissioner=True)

        th3_node_id = th3.nodeId
        th3_fabric_id = th3.fabricId
        logger.info(f'Step #3 - TH3 NodeID: {th3_node_id}')
        logger.info(f'Step #3 - TH3 FabricID: {th3_fabric_id}')

        params = await self.open_commissioning_window(th1, th1_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        setup_qr_code = params.commissioningParameters.setupQRCode
        logger.info(f'Step #3: Commissioning window opened: {vars(params)}')

        logger.info('Step #3 - Commissioning DUT with TH3...')
        resp = await th3.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=long_discriminator)
        logger.info(f'Step #3 - TH3 Commissioning response: {resp}')

        # Use TH3 controller (already commissioned) to write DefaultOTAProviders on fabric 2
        # TH3 is the OTA Provider (NodeID=3) for fabric 2
        provider_th3_for_fabric2 = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=th3.nodeId,  # TH3 is the OTA Provider (NodeID=3)
            endpoint=0,
            fabricIndex=th3.fabricId    # Fabric ID from TH3
        )

        # Update attribute with new providers list  (TH3 for fabric 2)
        attr = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=[provider_th3_for_fabric2])

        # Write updated DefaultOTAProviders attribute to DUT (TH1)
        resp = await th3.WriteAttribute(
            attributes=[(0, attr)],
            nodeid=self.dut_node_id,
        )
        logger.info(f'Step #3 - Write DefaultOTAProviders response: {resp}')
        asserts.assert_equal(resp[0].Status, 0, "Failed to write DefaultOTAProviders attribute")

        self.step(4)

        # Read the actual value of DefaultOTAProviders attribute on the TH3
        th3_actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=th3,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)

        # Get the first provider from the list for verification
        actual_provider = th3_actual_otap_info[0]
        logger.info(f'Step #4 - Read DefaultOTAProviders attribute on DUT using TH3: {th3_actual_otap_info}')

        # Verify the actual provider matches the expected OTA Provider (TH3) (NodeID=3)
        asserts.assert_equal(actual_provider.providerNodeID, provider_th3_for_fabric2.providerNodeID, "Mismatch in providerNodeID")
        asserts.assert_equal(actual_provider.endpoint, provider_th3_for_fabric2.endpoint, "Mismatch in endpoint")
        asserts.assert_equal(actual_provider.fabricIndex, provider_th3_for_fabric2.fabricIndex, "Mismatch in fabricIndex")
        logger.info("Step #4 - DefaultOTAProviders attribute matches expected values.")

        # Also verify from TH1 (fabric 1) still have TH2 as is OTA provider
        th1_actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)

        actual_provider = th1_actual_otap_info[0]
        logger.info(f'Step #4 - Read DefaultOTAProviders attribute on DUT using TH1: {th1_actual_otap_info}')

        # Verify the actual provider matches the expected OTA Provider (TH2) (NodeID=1)
        asserts.assert_equal(actual_provider.providerNodeID, provider_th2_for_fabric1.providerNodeID,
                             "Mismatch in providerNodeID (fabric 1)")
        asserts.assert_equal(actual_provider.endpoint, provider_th2_for_fabric1.endpoint, "Mismatch in endpoint (fabric 1)")
        asserts.assert_equal(actual_provider.fabricIndex, provider_th2_for_fabric1.fabricIndex,
                             "Mismatch in fabricIndex (fabric 1)")
        logger.info("Step #4 - DefaultOTAProviders attribute matches expected values from TH1 view.")

        # self.step(5)
        # self.step(6)
        # self.step(7)
        # self.step(8)
        # self.step(9)
if __name__ == "__main__":
    default_matter_test_main()
