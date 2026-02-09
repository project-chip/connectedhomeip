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
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_4_1(SoftwareUpdateBaseTest):
    """[TC-SU-4.1] Verifying Cluster Attributes on OTA-R(DUT)"""

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
            TestStep(
                1,
                "TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. "
                "TH2 is set as the default Provider for the fabric.",
                "Verify that the write operation for the attribute works and DUT does not respond with any errors.",
            ),
            TestStep(
                2,
                "TH sends a read request to read the DefaultOTAProviders Attribute on the first fabric to the DUT.",
                "Verify that the attribute value is set to TH2 as the default OTA provider for the fabric.",
            ),
            TestStep(
                3,
                "TH sends a write request for the DefaultOTAProviders Attribute on the second fabric to the DUT. "
                "TH3 is set as the default Provider for the fabric.",
                "Verify that the write operation for the attribute works and DUT does not respond with any errors.",
            ),
            TestStep(
                4,
                "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                "Verify that the attribute value is set to TH2 as the default OTA provider for the first fabric and TH3 for the second fabric.",
            ),
            TestStep(
                5,
                "TH sends a write request for the DefaultOTAProviders Attribute on the first fabric to the DUT. "
                "TH4 is the first Provider location and TH2 is the second Provider location in the same write request on the first fabric. "
                "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                "Verify that the write operation fails with CONSTRAINT_ERROR status code 0x87.\n"
                "Verify that the attribute value is set to TH3 as the default OTA provider for the second fabric and TH4 for the first fabric.",
            ),
            TestStep(
                6,
                "TH sends a write request for the DefaultOTAProviders Attribute with an empty provider list on the second fabric to the DUT. "
                "TH sends a read request to read the DefaultOTAProviders Attribute on the first and second fabric to the DUT.",
                "Verify that the write operation for the attribute works and DUT does not respond with any errors.\n"
                "Verify that the attribute value is set to TH4 as the default OTA provider for the first fabric and none for the second fabric.",
            ),
            TestStep(
                7,
                "TH sends a read request to read the UpdatePossible attribute from the DUT.",
                "Verify that the attribute value is set to True when there is an update possible.",
            ),
            TestStep(
                8,
                "TH sends a read request to read the UpdateState Attribute from the DUT.",
                "Verify that the attribute value is set to one of the following values.\n"
                "Unknown, Idle, Querying, DelayedOnQuery, Downloading, Applying, "
                "DelayedOnApply, RollingBack, DelayedOnUserConsent.",
            ),
        ]

    async def teardown_test(self):
        # Clear provider KVS after test to avoid interference with subsequent tests
        self.clear_kvs()
        await super().teardown_test()

    @async_test_body
    async def test_TC_SU_4_1(self):
        self.step(0)

        # Providers (Node IDs) used in DefaultOTAProviders.
        provider1_fabric1_node_id = 1  # TH2 in the test plan
        provider1_fabric2_node_id = 3  # TH3 in the test plan
        provider2_fabric1_node_id = 4  # TH4 in the test plan

        # TH (default_controller) acts as the Fabric1 admin/controller in this test.
        admin_fabric1 = self.default_controller
        requestor_node_id = self.dut_node_id

        self.step(1)

        logger.info(f"Step #1 - DUT NodeID (OTA Requestor): {requestor_node_id}")
        logger.info(f"Step #1 - admin_fabric1 FabricID: {admin_fabric1.fabricId}")

        # Read current DefaultOTAProviders value on Fabric1 (via admin_fabric1)
        actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #1 - DefaultOTAProviders read via admin_fabric1: {actual_otap_info}")

        # Write DefaultOTAProviders on Fabric1: set provider1_fabric1 (TH2)
        await self.set_default_ota_providers_list(
            controller=admin_fabric1,
            provider_node_id=provider1_fabric1_node_id,
            requestor_node_id=requestor_node_id,
        )
        logger.info("Step #1 - DefaultOTAProviders written on Fabric1")

        self.step(2)

        expected_provider1_fabric1_node_id = provider1_fabric1_node_id

        actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #2 - DefaultOTAProviders read via admin_fabric1: {actual_otap_info}")

        asserts.assert_true(len(actual_otap_info) > 0, "DefaultOTAProviders list is empty on Fabric1")

        actual_provider_f1 = actual_otap_info[0]
        logger.info(f"Step #2 - First provider entry (Fabric1): {actual_provider_f1}")

        fabric1_index = actual_provider_f1.fabricIndex
        logger.info(f"Step #2 - Captured fabric1_index from DUT: {fabric1_index}")

        asserts.assert_equal(
            actual_provider_f1.providerNodeID,
            expected_provider1_fabric1_node_id,
            "Mismatch in providerNodeID (Fabric1)",
        )
        asserts.assert_equal(actual_provider_f1.endpoint, 0, "Mismatch in endpoint (Fabric1)")
        asserts.assert_true(actual_provider_f1.fabricIndex > 0, "Expected a valid fabricIndex from DUT (Fabric1)")
        logger.info("Step #2 - DefaultOTAProviders (Fabric1) matches expected values.")

        self.step(3)

        # Establish Fabric2 admin/controller (TH3 in the test plan).
        # NOTE: fabricId values are not the same as fabricIndex and are not used for ProviderLocation.fabricIndex.
        admin_fabric2_fabric_id = admin_fabric1.fabricId + 1

        ca_fabric2 = self.certificate_authority_manager.NewCertificateAuthority()
        fabric_admin_fabric2 = ca_fabric2.NewFabricAdmin(vendorId=0xFFF1, fabricId=admin_fabric2_fabric_id)
        admin_fabric2 = fabric_admin_fabric2.NewController(
            nodeId=provider1_fabric2_node_id, useTestCommissioner=True
        )

        logger.info(f"Step #3 - admin_fabric2 NodeID: {admin_fabric2.nodeId}")
        logger.info(f"Step #3 - admin_fabric2 FabricID: {admin_fabric2.fabricId}")

        params = await self.open_commissioning_window(admin_fabric1, requestor_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        logger.info(f"Step #3 - Commissioning window opened: {vars(params)}")

        logger.info("Step #3 - Commissioning DUT with admin_fabric2")
        resp = await admin_fabric2.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=long_discriminator,
        )
        logger.info(f"Step #3 - admin_fabric2 commissioning response: {resp}")

        # Write DefaultOTAProviders on Fabric2: set provider1_fabric2 (TH3)
        await self.set_default_ota_providers_list(
            controller=admin_fabric2,
            provider_node_id=provider1_fabric2_node_id,
            requestor_node_id=requestor_node_id,
        )
        logger.info("Step #3 - DefaultOTAProviders written on Fabric2")

        self.step(4)

        # Verify on Fabric2 (via admin_fabric2)
        expected_provider1_fabric2_node_id = provider1_fabric2_node_id

        f2_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric2,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #4 - DefaultOTAProviders read via admin_fabric2: {f2_otap_info}")

        asserts.assert_true(len(f2_otap_info) > 0, "DefaultOTAProviders list is empty on Fabric2")

        actual_provider_f2 = f2_otap_info[0]
        fabric2_index = actual_provider_f2.fabricIndex
        logger.info(f"Step #4 - First provider entry (Fabric2): {actual_provider_f2}")
        logger.info(f"Step #4 - Captured fabric2_index from DUT: {fabric2_index}")

        asserts.assert_equal(
            actual_provider_f2.providerNodeID,
            expected_provider1_fabric2_node_id,
            "Mismatch in providerNodeID (Fabric2)",
        )
        asserts.assert_equal(actual_provider_f2.endpoint, 0, "Mismatch in endpoint (Fabric2)")
        asserts.assert_true(actual_provider_f2.fabricIndex > 0, "Expected a valid fabricIndex from DUT (Fabric2)")
        logger.info("Step #4 - DefaultOTAProviders (Fabric2) matches expected values.")

        # Verify Fabric1 is still provider1_fabric1 (TH2) when read via admin_fabric1
        f1_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #4 - DefaultOTAProviders read via admin_fabric1: {f1_otap_info}")

        asserts.assert_true(len(f1_otap_info) > 0, "DefaultOTAProviders list is empty on Fabric1 after Fabric2 write")

        actual_provider_f1 = f1_otap_info[0]
        asserts.assert_equal(
            actual_provider_f1.providerNodeID,
            expected_provider1_fabric1_node_id,
            "Mismatch in providerNodeID (Fabric1) after Fabric2 write",
        )
        asserts.assert_equal(actual_provider_f1.endpoint, 0, "Mismatch in endpoint (Fabric1) after Fabric2 write")
        asserts.assert_equal(
            actual_provider_f1.fabricIndex,
            fabric1_index,
            "Mismatch in fabricIndex (Fabric1) after Fabric2 write",
        )
        logger.info("Step #4 - DefaultOTAProviders (Fabric1) still matches expected values.")

        self.step(5)

        # Step 5 writes a list with 2 entries on Fabric1 (violating list constraints -> CONSTRAINT_ERROR).
        # List entries are expected to be processed in order; therefore the first entry (TH4) is expected to remain.
        provider2_fabric1_for_write = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=provider2_fabric1_node_id,
            endpoint=0,
        )
        provider1_fabric1_for_write = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=provider1_fabric1_node_id,
            endpoint=0,
        )

        providers_list = [provider2_fabric1_for_write, provider1_fabric1_for_write]
        logger.info(f"Step #5 - Providers list (Fabric1 write): {providers_list}")

        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=providers_list)

        pre_write_val = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #5 - BEFORE Write - Fabric1 DefaultOTAProviders: {pre_write_val}")

        resp = await admin_fabric1.WriteAttribute(
            nodeId=requestor_node_id,
            attributes=[(0, attr)],
        )
        logger.info(f"Step #5 - Write DefaultOTAProviders response: {resp}")

        asserts.assert_equal(
            resp[0].Status,
            Status.ConstraintError,
            "Expected write to fail with ConstraintError, got different status",
        )
        logger.info("Step #5 - Write operation correctly failed with CONSTRAINT_ERROR")

        post_write_val = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #5 - AFTER Write - Fabric1 DefaultOTAProviders: {post_write_val}")

        asserts.assert_equal(
            len(post_write_val),
            1,
            "DefaultOTAProviders list should contain only one provider after failed write on Fabric1",
        )
        asserts.assert_equal(
            post_write_val[0].providerNodeID,
            provider2_fabric1_node_id,
            "Expected TH4 (first entry) to remain on Fabric1",
        )
        asserts.assert_equal(post_write_val[0].endpoint, 0, "Expected endpoint 0 for remaining provider on Fabric1")
        asserts.assert_equal(
            post_write_val[0].fabricIndex,
            fabric1_index,
            "Expected remaining provider to remain on Fabric1",
        )

        # Verify Fabric2 still has provider1_fabric2 (TH3), unaffected by Fabric1 write
        fabric2_val = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric2,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #5 - Fabric2 DefaultOTAProviders read via admin_fabric2: {fabric2_val}")

        asserts.assert_equal(len(fabric2_val), 1, "Fabric2 should still have exactly one provider after failed write on Fabric1")
        asserts.assert_equal(
            fabric2_val[0].providerNodeID,
            provider1_fabric2_node_id,
            "Fabric2 provider should remain TH3 after ConstraintError on Fabric1",
        )
        asserts.assert_equal(fabric2_val[0].endpoint, 0, "Fabric2 provider endpoint should remain 0")
        asserts.assert_equal(
            fabric2_val[0].fabricIndex,
            fabric2_index,
            "Fabric2 provider should remain on Fabric2",
        )
        logger.info("Step #5 - Fabric2 DefaultOTAProviders unchanged as expected")

        self.step(6)

        providers_list_empty = []
        logger.info(f"Step #6 - Providers list empty (Fabric2 write): {providers_list_empty}")

        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=providers_list_empty)

        resp = await admin_fabric2.WriteAttribute(
            nodeId=requestor_node_id,
            attributes=[(0, attr)],
        )
        logger.info(f"Step #6 - Write DefaultOTAProviders response on Fabric2: {resp}")

        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders (empty list) on Fabric2")
        logger.info("Step #6 - Write operation succeeded with empty list on Fabric2")

        # Verify Fabric2 DefaultOTAProviders is now empty
        f2_after_clear = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric2,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #6 - Fabric2 DefaultOTAProviders after clear: {f2_after_clear}")

        asserts.assert_equal(len(f2_after_clear), 0, "DefaultOTAProviders on Fabric2 should be empty after clear")
        asserts.assert_equal(f2_after_clear, [], "DefaultOTAProviders on Fabric2 should be [] after clear")

        # Verify Fabric1 remains TH4 (from step 5 ordering expectation)
        f1_after_clear_f2 = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders,
        )
        logger.info(f"Step #6 - Fabric1 DefaultOTAProviders after Fabric2 clear: {f1_after_clear_f2}")

        asserts.assert_equal(len(f1_after_clear_f2), 1, "DefaultOTAProviders on Fabric1 should contain one provider")
        asserts.assert_equal(
            f1_after_clear_f2[0].providerNodeID,
            provider2_fabric1_node_id,
            "DefaultOTAProviders on Fabric1 should remain TH4",
        )
        asserts.assert_equal(f1_after_clear_f2[0].endpoint, 0, "Expected endpoint 0 on Fabric1 remaining provider")
        asserts.assert_equal(
            f1_after_clear_f2[0].fabricIndex,
            fabric1_index,
            "Expected Fabric1 remaining provider to still be on Fabric1",
        )
        logger.info("Step #6 - Fabric1 DefaultOTAProviders remains TH4 as expected.")

        self.step(7)

        update_possible = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdatePossible,
        )
        logger.info(f"Step #7 - UpdatePossible read via admin_fabric1: {update_possible}")

        asserts.assert_true(update_possible, "Expected UpdatePossible to be True")

        self.step(8)

        update_state = await self.read_single_attribute_check_success(
            dev_ctrl=admin_fabric1,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.UpdateState,
        )
        logger.info(f"Step #8 - UpdateState read via admin_fabric1: {update_state}")

        valid_states = [
            self.cluster_otar.Enums.UpdateStateEnum.kUnknown,
            self.cluster_otar.Enums.UpdateStateEnum.kIdle,
            self.cluster_otar.Enums.UpdateStateEnum.kQuerying,
            self.cluster_otar.Enums.UpdateStateEnum.kDelayedOnQuery,
            self.cluster_otar.Enums.UpdateStateEnum.kDownloading,
            self.cluster_otar.Enums.UpdateStateEnum.kApplying,
            self.cluster_otar.Enums.UpdateStateEnum.kDelayedOnApply,
            self.cluster_otar.Enums.UpdateStateEnum.kRollingBack,
            self.cluster_otar.Enums.UpdateStateEnum.kDelayedOnUserConsent,
        ]

        asserts.assert_true(update_state in valid_states, f"Unexpected UpdateState value: {update_state}")
        logger.info("Step #8 - UpdateState value is valid.")


if __name__ == "__main__":
    default_matter_test_main()
