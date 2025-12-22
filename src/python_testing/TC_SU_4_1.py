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
#       --KVS /tmp/chip_kvs_requestor
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

import logging

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_4_1(SoftwareUpdateBaseTest):

    # Reference variable for the OTA Software Update Requestor cluster.
    cluster_otar = Clusters.OtaSoftwareUpdateRequestor

    def desc_TC_SU_4_1(self) -> str:
        return "[TC-SU-4.1] Verifying Cluster Attributes on OTA-R(DUT)"

    def pics_TC_SU_4_1(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_4_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. TH2 is set as the default Provider for the fabric.",
                     "Verify that the write operation for the attribute works and DUT does not respond with any errors."),
            TestStep(2, "TH sends a read request to read the DefaultOTAProviders Attribute on the first fabric to the DUT.",
                     "Verify that the attribute value is set to TH2 as the default OTA provider for the fabric."),
            TestStep(3, "TH sends a write request for the DefaultOTAProviders Attribute on the second fabric to the DUT. TH3 is set as the default Provider for the fabric.",
                     "Verify that the write operation for the attribute works and DUT does not respond with any errors."),
            TestStep(4, "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                     "Verify that the attribute value is set to TH2 as the default OTA provider for the first fabric and TH3 for the second fabric."),
            TestStep(5, "TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. "
                     "TH4 is the first Provider location and TH2 is the second Provider location in the same write request on the first fabric. "
                     "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                     "Verify that the write operation fails with CONSTRAINT_ERROR status code 0x87.\n"
                     "Verify that the attribute value is set to TH3 as the default OTA provider for the second fabric and either of TH2 or TH4 for the first fabric."),
            TestStep(6, "TH sends a write request for the DefaultOTAProviders Attribute with an empty provider list on the second fabric to the DUT. "
                     "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                     "Verify that the write operation for the attribute works and DUT does not respond with any errors.\n"
                     "Verify that the attribute value is set to TH4 as the default OTA provider for the first fabric and none for the second fabric."),
            TestStep(7, "TH sends a read request to read the UpdatePossible attribute from the DUT.",
                     "Verify that the attribute value is set to True when there is an update possible."),
            TestStep(8, "TH sends a read request to read the UpdateState Attribute from the DUT.",
                     "Verify that the attribute value is set to one of the following values.\n"
                     "Unknown, Idle, Querying, DelayedOnQuery, Downloading, Applying, DelayedOnApply, RollingBack, DelayedOnUserConsent."),
        ]

    async def teardown_test(self):
        # Clear provider KVS after test to avoid interference with subsequent tests
        self.clear_kvs()
        await super().teardown_test()

    @async_test_body
    async def test_TC_SU_4_1(self):
        self.step(0)

        # OTA Providers (Node IDs) setup
        th2_node_id = 1                     # nodeId=1, Fabric 1
        th3_node_id = th2_node_id + 2       # nodeId=3, Fabric 2
        th4_node_id = th3_node_id + 1       # nodeId=4, Fabric 1

        self.step(1)
        # Establishing TH controller - DUT (OTA Requestor) NodeID=2, TH on Fabric=1
        # NOTE: In the test verification this controller corresponds to "TH" (alpha commissioner)
        th = self.default_controller
        th_node_id = self.dut_node_id
        th_fabric_id = th.fabricId
        logger.info(f'Step #1 - DUT NodeID (OTA Requestor): {th_node_id}')
        logger.info(f'Step #1 - TH FabricID (Should be Fabric 1): {th_fabric_id}')

        # Read the actual value of DefaultOTAProviders attribute on the DUT (TH, NodeID=2)
        actual_otap_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #1 - Read actual DefaultOTAProviders value on DUT (TH): {actual_otap_info}')

        # TH2 is the OTA Provider (NodeID=1) for fabric 1
        # Write DefaultOTAProviders to DUT using base class helper function
        await self.set_default_ota_providers_list(
            controller=th,
            provider_node_id=th2_node_id,   # TH2 is the OTA Provider (NodeID=1)
            requestor_node_id=th_node_id
        )
        logger.info("Step #1 - DefaultOTAProviders attribute written successfully to DUT (TH)")

        self.step(2)
        th2_fabric_id = th.fabricId  # Both in Fabric 1
        # Reference object for verifying DefaultOTAProviders (TH2 as OTA Provider, NodeID=1, Fabric 1)
        provider_th2_for_fabric1 = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=th2_node_id,
            endpoint=0,
            fabricIndex=th2_fabric_id
        )

        # Verify DefaultOTAProviders attribute on the DUT (TH) after write (using TH on Fabric 1)
        actual_otap_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #2 - DefaultOTAProviders value read from DUT (TH): {actual_otap_info}')

        # Verify that the provider list is not empty (expecting at least one provider)
        asserts.assert_true(len(actual_otap_info) > 0, "DefaultOTAProviders list is empty")

        # Get the first provider from the list for verification
        actual_provider = actual_otap_info[0]
        logger.info(f'Step #2 - Read DefaultOTAProviders value after write on DUT (TH): {actual_provider}')

        # Verify the actual provider matches the expected OTA Provider (TH2)
        # DUT as OTA Requestor (NodeID=2); TH2 as OTA Provider (NodeID=1)
        asserts.assert_equal(actual_provider.providerNodeID, provider_th2_for_fabric1.providerNodeID, "Mismatch in providerNodeID")
        asserts.assert_equal(actual_provider.endpoint, provider_th2_for_fabric1.endpoint, "Mismatch in endpoint")
        asserts.assert_equal(actual_provider.fabricIndex, provider_th2_for_fabric1.fabricIndex, "Mismatch in fabricIndex")
        logger.info("Step #2 - DefaultOTAProviders attribute matches expected values.")

        self.step(3)

        # Establishing TH3 controller - TH3, NodeID=3, Fabric=2
        th3_fabric_id = th.fabricId + 1
        th3_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th3_fabric_admin = th3_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=th3_fabric_id)
        th3 = th3_fabric_admin.NewController(nodeId=th3_node_id, useTestCommissioner=True)

        logger.info(f'Step #3 - TH3 NodeID: {th3.nodeId}')
        logger.info(f'Step #3 - TH3 FabricID: {th3.fabricId}')

        params = await self.open_commissioning_window(th, th_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        logger.info(f'Step #3: Commissioning window opened: {vars(params)}')

        logger.info('Step #3 - Commissioning DUT with TH3')
        resp = await th3.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=long_discriminator)
        logger.info(f'Step #3 - TH3 Commissioning response: {resp}')

        # Use TH3 controller (already commissioned) to write DefaultOTAProviders on fabric 2
        # NOTE: In the test verification this controller corresponds to "TH3" (beta commissioner)
        # TH3 is the OTA Provider (NodeID=3) for fabric 2
        await self.set_default_ota_providers_list(
            controller=th3,            # TH3 controller → Fabric 2
            provider_node_id=th3.nodeId,
            requestor_node_id=self.dut_node_id
        )
        logger.info("Step #3 - DefaultOTAProviders attribute written successfully to DUT (TH3)")

        self.step(4)

        # Reference object for verifying DefaultOTAProviders (TH3, NodeId 3,  Fabric 2)
        provider_th3_for_fabric2 = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=th3.nodeId,
            endpoint=0,
            fabricIndex=th3.fabricId
        )

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

        # Also verify from TH (fabric 1) still have TH2 as is OTA provider
        th1_actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)

        actual_provider = th1_actual_otap_info[0]
        logger.info(f'Step #4 - Read DefaultOTAProviders attribute on DUT using TH: {th1_actual_otap_info}')

        # Verify the actual provider matches the expected OTA Provider (TH2) (NodeID=1)
        asserts.assert_equal(actual_provider.providerNodeID, provider_th2_for_fabric1.providerNodeID,
                             "Mismatch in providerNodeID (fabric 1)")
        asserts.assert_equal(actual_provider.endpoint, provider_th2_for_fabric1.endpoint, "Mismatch in endpoint (fabric 1)")
        asserts.assert_equal(actual_provider.fabricIndex, provider_th2_for_fabric1.fabricIndex,
                             "Mismatch in fabricIndex (fabric 1)")
        logger.info("Step #4 - DefaultOTAProviders attribute matches expected values from TH view.")

        self.step(5)
        # TH4 is the OTA Provider (NodeID=4) for fabric 1
        th4_fabric_id = th2_fabric_id  # Same Fabric 1
        provider_th4_for_fabric1 = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=th4_node_id,
            endpoint=0,
            fabricIndex=th4_fabric_id
        )

        # NOTE:
        # According to Bug #40294 and Matter specification sections 7.3.3 (Write Path Data Process)
        # and 10.6.4.3.1 (Lists), when writing a list attribute with multiple providers from the same fabric:
        # - The write may fail with a ConstraintError if any entry violates constraints.
        # - Only one valid entry in the list is guaranteed to be preserved ; which entry is preserved
        #   is not deterministic, as observed in current Matter SDK behavior (Bug #40294).
        # Use WriteAttribute() instead of set_default_ota_providers_list(), which only supports a single provider.
        # This is expected behavior for list attributes and should be verified in the test.

        # Create Providers list and Add TH4 and TH2 for fabric 1 to Providers list
        providers_list = [provider_th4_for_fabric1, provider_th2_for_fabric1]
        logger.info(f'Step #5 - Providers list updated with provider "TH4 and TH2 for fabric 1": {providers_list}')

        # Update attribute with providers list  (TH2 and TH4 for fabric 1)
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=providers_list)

        # Read DefaultOTAProviders attribute before write (TH on Fabric 1)
        pre_write_val = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #5b - BEFORE Write - TH DefaultOTAProviders: {pre_write_val}')

        # Write DefaultOTAProviders using TH (fabric 1) - expecting ConstraintError
        resp = await th.WriteAttribute(
            nodeId=self.dut_node_id,
            attributes=[(0, attr)]
        )
        logger.info(f'Step #5- Write DefaultOTAProviders response: {resp}')
        asserts.assert_equal(resp[0].Status, Status.ConstraintError,
                             "Expected write to fail with ConstraintError, got different status")
        logger.info("Step #5 - Write operation correctly failed with CONSTRAINT_ERROR")

        # Read DefaultOTAProviders attribute after write (TH on Fabric 1)
        post_write_val = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #5b - AFTER Write - TH DefaultOTAProviders: {post_write_val}')

        # Verify that exactly one provider remains (it is either TH2 or TH4)
        asserts.assert_equal(len(post_write_val), 1, "DefaultOTAProviders list should contain only one provider after failed write")
        asserts.assert_true(
            post_write_val[0] in [provider_th2_for_fabric1, provider_th4_for_fabric1],
            "Remaining provider must be either TH2 or TH4 after ConstraintError"
        )

        self.step(6)
        # Create Empty Providers list
        providers_list_empty = []
        logger.info(f'Step #6 - Providers list updated with provider empty: {providers_list_empty}')

        # Update attribute with empty providers list (TH3 for fabric 2)
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=providers_list_empty)

        # Write updated DefaultOTAProviders attribute to DUT using TH3 (Fabric 2)
        resp = await th3.WriteAttribute(
            nodeId=self.dut_node_id,
            attributes=[(0, attr)],
        )
        logger.info(f'Step #6- Write DefaultOTAProviders response on TH3 Fabric 2: {resp}')
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")
        logger.info("Step #6 - Write operation correctly performed with empty list")

        # Verify DefaultOTAProviders attribute on the DUT after write (TH3 on Fabric 2)
        th3_actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=th3,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)

        actual_provider = th3_actual_otap_info

        # Verify DefaultOTAProviders attribute on Fabric 2 is empty
        asserts.assert_true(len(actual_provider) == 0, "DefaultOTAProviders list is not empty")
        asserts.assert_equal(actual_provider, [], "DefaultOTAProviders on TH3 (Fabric 2) should be empty")
        logger.info(f'Step #6 - Fabric 2 DefaultOTAProviders attribute is empty as expected: {actual_provider}')

        # NOTE:
        # - Writing an empty list succeeds and clears the providers for that fabric.
        # - Other fabrics are not affected.

        # Verify DefaultOTAProviders attribute on Fabric 1 after write is still TH4 (using TH on Fabric 1)
        th_actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #6 - Read DefaultOTAProviders attribute on DUT using TH (Fabric 1): {th_actual_otap_info}')

        # Verify Fabric 1 still has exactly one provider
        asserts.assert_equal(len(th_actual_otap_info), 1, "DefaultOTAProviders list on Fabric 1 should contain one provider")

        # Remaining provider must be unchanged (either TH2 or TH4)
        asserts.assert_equal(
            th_actual_otap_info[0],
            provider_th4_for_fabric1,
            "DefaultOTAProviders on Fabric 1 should remain set to TH4"
        )
        logger.info("Step #6 - Fabric 1 DefaultOTAProviders remains set to TH4.")

        self.step(7)
        # Step #7: Verify UpdatePossible attribute.
        # This step only requires confirming that UpdatePossible is True when an update is possible.

        # Read UpdatePossible from TH (Fabric 1)
        update_possible_th = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdatePossible)
        logger.info(f'Step #7 - Read UpdatePossible attribute on DUT using TH (fabric 1): {update_possible_th}')

        # Verify UpdatePossible attribute is True
        asserts.assert_true(update_possible_th, "Expected UpdatePossible to be True")

        self.step(8)
        # Read UpdateState from TH (fabric 1)
        th_update_state = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdateState
        )
        logger.info(f'Step #8 - TH UpdateState (Fabric 1): {th_update_state}')

        # Verify the UpdateState is one of the valid states
        valid_states = [
            self.cluster_otar.Enums.UpdateStateEnum.kUnknown,
            self.cluster_otar.Enums.UpdateStateEnum.kIdle,
            self.cluster_otar.Enums.UpdateStateEnum.kQuerying,
            self.cluster_otar.Enums.UpdateStateEnum.kDelayedOnQuery,
            self.cluster_otar.Enums.UpdateStateEnum.kDownloading,
            self.cluster_otar.Enums.UpdateStateEnum.kApplying,
            self.cluster_otar.Enums.UpdateStateEnum.kDelayedOnApply,
            self.cluster_otar.Enums.UpdateStateEnum.kRollingBack,
            self.cluster_otar.Enums.UpdateStateEnum.kDelayedOnUserConsent
        ]

        asserts.assert_true(th_update_state in valid_states,
                            f"Unexpected UpdateState value: {th_update_state}")
        logger.info("Step #8 - UpdateState values are valid.")

        # NOTE: Step 9 skipped per test plan refinement.
        # Fabric separation and image download behavior will be validated in TC-SU-2.1 and TC-SU-2.2 tests.


if __name__ == "__main__":
    default_matter_test_main()
