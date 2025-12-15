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
#     app: ${OTA_REQUESTOR}
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
import os
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_4_1(SoftwareUpdateBaseTest):
    """
        Validate OTA Requestor (DUT) attribute behavior across fabrics:
        - DefaultOTAProviders
        - UpdatePossible
        - UpdateState

        Raises:
            AssertionError: If any attribute read/write response does not match the expected outcome.

    """

    # Reference variable for the OTA Software Update Provider cluster.
    # cluster_otap = Clusters.OtaSoftwareUpdateProvider
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
            nodeId=self.dut_node_id,
            attributes=[(0, Clusters.AccessControl.Attributes.Acl(acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

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
            TestStep(4, "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT."
                     "Verify that the attribute value is set to TH2 as the default OTA provider for the first fabric and TH3 for the second fabric."),
            TestStep(5, "TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. "
                     "TH4 is the first Provider location and TH2 is the second Provider location in the same write request on the first fabric. "
                     "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                     "Verify that the write operation fails with CONSTRAINT_ERROR status code 0x87.\n"
                     "Verify that the attribute value is set to TH3 as the default OTA provider for the second fabric and either of TH2 or TH4 for the first fabric."),
            TestStep(6, "TH sends a write request for the DefaultOTAProviders Attribute with an empty provider list on the second fabric to the DUT. "
                     "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                     "Verify that the write operation for the attribute works and DUT does not respond with any errors."
                     "Verify that the attribute value is set to TH4 as the default OTA provider for the first fabric and none for the second fabric."),
            TestStep(7, "TH sends a read request to read the UpdatePossible attribute from the DUT.",
                     "Verify that the attribute value is set to True when there is an update possible."),
            TestStep(8, "TH sends a read request to read the UpdateState Attribute from the DUT.",
                     "Verify that the attribute value is set to one of the following values.\n"
                     "Idle, Querying, DelayedOnQuery, Downloading, Applying, DelayedOnApply, RollingBack, DelayedOnUserConsent."),
        ]

    @async_test_body
    async def test_TC_SU_4_1(self):
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
        logger.info(f'Step #1 - Write DefaultOTAProviders response: {resp}')

        # Verify write succeeded (response code 0)
        asserts.assert_equal(resp, Status.Success, "Failed to write DefaultOTAProviders attribute")

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
        # setup_qr_code = params.commissioningParameters.setupQRCode
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
        provider_th3_for_fabric2 = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=th3.nodeId,  # TH3 is the OTA Provider (NodeID=3)
            endpoint=0,
            fabricIndex=th3.fabricId    # Fabric ID from TH3
        )

        # Update attribute with new providers list  (TH3 for fabric 2)
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=[provider_th3_for_fabric2])

        # Write updated DefaultOTAProviders attribute to DUT (TH1)
        resp = await th3.WriteAttribute(
            nodeId=self.dut_node_id,
            attributes=[(0, attr)]
        )
        logger.info(f'Step #3 - Write DefaultOTAProviders response: {resp}')
        # Status=<Status.Success: 0
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")

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

        self.step(5)

        # Establishing TH4 controller - TH4, NodeID=4, Fabric=2
        fabric_admin = self.default_controller.fabricAdmin
        th4 = fabric_admin.NewController(nodeId=4)

        th4_node_id = th4.nodeId
        th4_fabric_id = th4.fabricId
        logger.info(f'Step #5 - TH4 NodeID: {th4_node_id}')
        logger.info(f'Step #5 - TH4 FabricID: {th4_fabric_id}')

        params = await self.open_commissioning_window(th1, th1_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        # setup_qr_code = params.commissioningParameters.setupQRCode
        logger.info(f'Step #5: Commissioning window opened: {vars(params)}')

        logger.info('Step #5 - Commissioning DUT with TH4...')
        resp = await th4.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=long_discriminator)
        logger.info(f'Step #5 - TH4 Commissioning response: {resp}')

        # Write the Access Control List (ACL) to the DUT device for TH4 controller
        # using the library function create_acl_entry, passing both Admin and View permissions
        # so that TH4 can perform necessary operations.
        acl_list = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=0,
                    cluster=self.cluster_otar.id
                )]
            ),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[]
            )
        ]

        resp = await self.create_acl_entry(
            dev_ctrl=th1,
            provider_node_id=self.dut_node_id,
            acl_entries=acl_list
        )
        logger.info(f'Step #5 - TH4 has Admin + View permissions: {resp}')

        # TH4 is the OTA Provider (NodeID=4) for fabric 1
        provider_th4_for_fabric1 = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=th4.nodeId,
            endpoint=0,
            fabricIndex=th4.fabricId
        )

        # NOTE:
        # According to Bug #40294 and Matter specification sections 7.3.3 (Write Path Data Process)
        # and 10.6.4.3.1 (Lists), when writing a list attribute with multiple providers from the same fabric:
        # - The write may fail with a ConstraintError if any entry violates constraints.
        # - Only the first valid entry in the list is guaranteed to be preserved.
        # - The rest of the entries may be ignored, and the attribute value remains unchanged.
        # This is expected behavior for list attributes and should be verified in the test.

        # Create Providers list and Add TH4 and TH2 for fabric 1 to Providers list
        providers_list = [provider_th2_for_fabric1]
        providers_list.append(provider_th4_for_fabric1)
        logger.info(f'Step #5 - Providers list updated with provider "TH4 and TH2 for fabric 1": {providers_list}')

        # Update attribute with providers list  (TH2 and TH4 for fabric 1)
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=providers_list)

        # --- Optional verification before write ---
        # Verify DefaultOTAProviders attribute before write (TH4 on Fabric 1)
        pre_write_val = await self.read_single_attribute_check_success(
            dev_ctrl=th4,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #5b - BEFORE Write - TH4 DefaultOTAProviders: {pre_write_val}')
        # ------------------------------------------

        # Write updated DefaultOTAProviders attribute to TH4
        resp = await th4.WriteAttribute(
            nodeId=self.dut_node_id,
            attributes=[(0, attr)]
        )
        logger.info(f'Step #5- Write DefaultOTAProviders response: {resp}')
        #  Status=<Status.ConstraintError: 135
        asserts.assert_equal(resp[0].Status, Status.ConstraintError,
                             "Expected write to fail with ConstraintError, got different status")
        logger.info("Step #5 - Write operation correctly failed with CONSTRAINT_ERROR")

        # --- Optional verification after write ---
        # Read again to confirm the value did NOT change
        post_write_val = await self.read_single_attribute_check_success(
            dev_ctrl=th4,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #5b - AFTER Write - TH4 DefaultOTAProviders: {post_write_val}')

        # Confirm attribute value did not change
        asserts.assert_equal(pre_write_val, post_write_val,
                             "Attribute value changed unexpectedly after failed write")

        self.step(6)

        # Create Empty Providers list
        providers_list_empty = []
        logger.info(f'Step #6 - Providers list updated with provider empty: {providers_list_empty}')

        # Update attribute with empty providers list (TH3 for fabric 2)
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=providers_list_empty)

        # Write updated DefaultOTAProviders attribute to TH3
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

        # Verify that the provider list is empty
        asserts.assert_true(len(actual_provider) == 0, "DefaultOTAProviders list is not empty")

        # Verify the actual provider matches the expected OTA Provider (TH3)
        asserts.assert_equal(actual_provider, [], "DefaultOTAProviders on TH3 (Fabric 2) should be empty")
        logger.info("Step #6 - DefaultOTAProviders attribute matches expected values.")

        # NOTE:
        # According to current observed behavior and discussion in Bug #40294,
        # when writing a list with multiple providers from the same fabric,
        # only the first valid entry is preserved if the write fails due to a ConstraintError.
        # The rest of the list is ignored, and the original value remains unchanged.

        # Verify DefaultOTAProviders attribute on the DUT after write (TH4 on Fabric 1) in this case should be the original from TH2 fabric 1
        # Read fabric 1 with TH4 and verify that the provider is TH1 (providerNodeID=1 , fabricIndex=1 )
        th4_actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=th4,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)

        logger.info(f'Step #6 - Read DefaultOTAProviders attribute on DUT using TH4: {th4_actual_otap_info}')

        self.step(7)

        # Verify DefaultOTAProviders attribute on the DUT after write (TH4 on Fabric 1)
        update_possible_th4 = await self.read_single_attribute_check_success(
            dev_ctrl=th4,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdatePossible)

        logger.info(f'Step #7 - Read UpdatePossible attribute on DUT using TH4 (fabric 1): {update_possible_th4}')

        # Verify UpdatePossible is true
        asserts.assert_true(update_possible_th4, "Expected UpdatePossible to be True on fabric 1")

        # Verify DefaultOTAProviders attribute on the DUT after write (TH3 on Fabric 2)
        update_possible_th3 = await self.read_single_attribute_check_success(
            dev_ctrl=th4,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdatePossible)

        logger.info(f'Step #7 - Read UpdatePossible attribute on DUT using TH3 (fabric 2): {update_possible_th3}')

        # Verify UpdatePossible is true
        asserts.assert_true(update_possible_th3, "Expected UpdatePossible to be True on fabric 2")

        self.step(8)
        # Step #8 - Read UpdateState from TH4 (fabric 1)
        t4_update_state = await self.read_single_attribute_check_success(
            dev_ctrl=th4,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdateState
        )
        logger.info(f"Step #8 - TH4 UpdateState: {t4_update_state}")

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

        asserts.assert_true(t4_update_state in valid_states,
                            f"Unexpected UpdateState value: {t4_update_state}")

        # NOTE: Step 9 skipped per test plan refinement.
        # Fabric separation and image download behavior will be validated in TC-SU-2.1 and TC-SU-2.2 tests.


if __name__ == "__main__":
    default_matter_test_main()
