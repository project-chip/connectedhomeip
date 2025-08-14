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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, AttributeChangeCallback
from chip.testing.matter_testing import ClusterAttributeChangeAccumulator, AttributeMatcher

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
            node_id:

        Raises:
            AssertionError: If writing the ACL attribute fails (status is not Status.Success).
        """

    async def write_acl(self, controller, node_id, acl):
        result = await controller.WriteAttribute(
            node_id,
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
        # 1. Launch OTA Provider from Terminal 1:
        #     ./out/debug/chip-ota-provider-app --filepath firmware_v1.ota \
        #         --discriminator 1111
        #         --passcode 20202021
        #         --secured-device-port 5540
        # 2. Launch OTA Requestor (TH1 / DUT) from Terminal 2:
        #     ./out/debug/chip-ota-requestor-app \
        #         --discriminator 1234 \
        #         --passcode 20202021 \
        #         --secured-device-port 5541 \
        #         --autoApplyImage \
        #         --KVS /tmp/chip_kvs_requestor
        # 3. Run Python test with commission Provisioner/Requestor from Terminal 3:
        #     python3 src/python_testing/TC_SU_2_2.py \
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
        # ------------------------------------------------------------------------------------
        # Step 1.0: DUT is already commissioned via test args (DUT/TH1 = OTA Requestor)
        # ------------------------------------------------------------------------------------
        # 1.0 Establishing TH1 controller - DUT is TH1, NodeID=2, Fabric=1
        controller = self.default_controller
        requestor_node_id = self.dut_node_id  # 2
        fabric_id = controller.fabricId
        logger.info(f'Step #1.0 - Requestor (DUT) NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_node_id = 1
        provider_discriminator = 1111
        provider_setupPinCode = 20202021
        logger.info(f'Step #1.0 - Provider NodeID: {provider_node_id}, FabricId: {fabric_id}')

        # ------------------------------------------------------------------------------------
        # Step 1.1 - Open commissioning window on DUT (via TH1)
        # ------------------------------------------------------------------------------------

        params = await self.open_commissioning_window(controller, requestor_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        # setup_qr_code = params.commissioningParameters.setupQRCode
        logger.info(f'Step #1.1: Commissioning window opened: {vars(params)}')

        # ------------------------------------------------------------------------------------
        # Step # 1.2 - Commissioning TH2 (OTA Provider) using TH1 controller
        # ------------------------------------------------------------------------------------

        logger.info('Step #1.2 - Commissioning DUT with TH2')
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'Step #1.2 - TH2 Commissioning response: {resp}')

        # ------------------------------------------------------------------------------------
        # Step # 1.3 - Setting ACLs
        # ------------------------------------------------------------------------------------
        fabric_index = controller.fabricId

        logger.info(f"Step #1.3 - Setting ACLs under FabricIndex {fabric_index}")
        logger.info(f"Step #1.3 - Requestor (DUT) NodeID: {requestor_node_id}")
        logger.info(f"Step #1.3 - Provider NodeID: {provider_node_id}")

        # ACL on Requestor to allow Provider to operate OTA Requestor

        # Provider can send commands to OTA Requestor
        acl_operate_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateRequestor.id)]
        )

        # Provider can view attributes to OTA Requestor
        acl_view_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[]
        )

        # Provider can administer (modify ACLs etc.) on Requestor - optional
        acl_admin_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateRequestor.id)]
        )

        # Group all three ACL entries and write them on Requestor
        acls_on_th1 = [acl_admin_provider, acl_view_provider, acl_operate_provider]
        resp = await self.write_acl(controller, requestor_node_id, acls_on_th1)
        logger.info(f'Step #1.3.1 - Wrote ACLs on Requestor to allow access from Provider: {resp}')

        # ACL to allow Requestor to operate OTA Provider

        # can send commands to OTA Provider
        acl_operate_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateProvider.id
            )]
        )

        # can view attributes to OTA Provider
        acl_view_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[]
        )

        # TH1 can administer to OTA Provider
        acl_admin_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[requestor_node_id],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateProvider.id
            )]
        )

        # Group all ACL entries and write them on TH2 (Provider)
        acls_on_th2 = [acl_admin_requestor, acl_view_requestor, acl_operate_requestor]
        resp = await self.write_acl(controller, provider_node_id, acls_on_th2)
        logger.info(f'Step #1.3.2 - Wrote ACLs on Provider to allow access from Requestor: {resp}')

        # ------------------------------------------------------------------------------------
        # Step #1.4 - Read the current OTA providers on the Requestor,
        # create a ProviderLocation structure pointing to Provider as the OTA provider,
        # and write this updated provider list back to the DUT (TH1)
        # ------------------------------------------------------------------------------------

        # Read the actual value of DefaultOTAProviders attribute on the Requestor (NodeID=2)
        actual_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders)
        logger.info(f'Step #1.4 - Read actaul DefaultOTAProviders value on DUT (TH1): {actual_otap_info}')

        # Create the ProviderLocation struct the OTA Provider (NodeID=1, Fabric=1)
        provider_for_fabric = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=provider_node_id,        # TH2 is the OTA Provider (NodeID=1)
            endpoint=0,
            fabricIndex=controller.fabricId           # FabricId from TH1 (the DUT)
        )
        logger.info(f'Step #1.4 - ProviderLocation to write: {provider_for_fabric}')

        # Create the DefaultOTAProviders attribute with the OTA provider list
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=[provider_for_fabric])
        logger.info(f'Step #1.4 - Attribute to write: {attr}')

        # Write the DefaultOTAProviders attribute to the Requestor
        resp = await controller.WriteAttribute(
            attributes=[(0, attr)],
            nodeid=requestor_node_id,
        )
        logger.info(f'Step #1.4 - Write DefaultOTAProviders response: {resp}')
        # Verify write succeeded (response code 0)
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")

        # ------------------------------------------------------------------------------------
        # Step # 1.5 - Provider sends AnnounceOTAProvider command to TH1 Requestor
        # ------------------------------------------------------------------------------------

        logger.info("Step #1.5.1 - TH2 (Provider) sends AnnounceOTAProvider command to TH1 (DUT)")
        cmd_announce = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=0x0000000000000001,  # Provider
            vendorID=0xFFF1,
            announcementReason=Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kSimpleAnnouncement,
            metadataForNode=None,
            endpoint=0
        )
        logger.info(f"Step #1.5 - cmd AnnounceOTAProvider: {cmd_announce}")

        resp_announce = await self.send_single_cmd(
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT/TH1 NodeID
            cmd=cmd_announce
        )
        logging.info(f"Step #1.5 - AnnounceOTAProvider response: {resp_announce}.")

        # ------------------------------------------------------------------------------------
        # Step # 1.6 - Verify image transfer from TH/OTA-P to DUT is successful
        # by UpdateStateProgress
        # ------------------------------------------------------------------------------------

        # logger.info("Step #1.6 - Create an accumulator for the UpdateStateProgress attribute")

        # # Create accumulator for UpdateStateProgress attribute
        # accumulator = ClusterAttributeChangeAccumulator(
        #     expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
        #     expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress
        # )

        # # Start subscription (async)
        # await accumulator.start(
        #     dev_ctrl=controller,
        #     node_id=requestor_node_id,
        #     endpoint=0,
        #     fabric_filtered=False,
        #     min_interval_sec=5,
        #     max_interval_sec=10,
        #     keepSubscriptions=True
        # )

        # # Track last known progress
        # last_progress = {"value": None}

        # def matcher_final(report):
        #     val = report.value
        #     if val is not None:
        #         last_progress["value"] = val
        #         if val == 100:
        #             logger.info("Step #1.6 - Final progress reached 100%, update completed.")
        #             return True
        #         return False
        #     else:
        #         # Null received — can mean finish or failure
        #         if last_progress["value"] is not None and last_progress["value"] >= 99:
        #             logger.info("Step #1.6 - Final progress Null after >=99%, update completed.")
        #             return True
        #         else:
        #             raise AssertionError("Progress became Null before reaching 99%, update failed.")

        # matcher_final_obj = AttributeMatcher.from_callable(
        #     description="Final progress: 100 or Null after >=99%",
        #     matcher=matcher_final
        # )

        # try:
        #     await accumulator.await_all_expected_report_matches([matcher_final_obj], timeout_sec=1200.0)
        #     logger.info("Step #1.6 - Update completed successfully.")
        # except Exception as e:
        #     raise AssertionError(f"No final progress received in time or failed: {e}")

        # # Cancel subscription
        # await accumulator.cancel()

        # ------------------------------------------------------------------------------------
        # Step # 1.7 - Verify image transfer from TH/OTA-P to DUT is successful
        # by UpdateState
        # ------------------------------------------------------------------------------------

        logger.info("Step #1.6 - Create an accumulator for the UpdateState attribute")

        # Create accumulator for UpdateState attribute
        accumulator = ClusterAttributeChangeAccumulator(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscription (async) to receive periodic updates of UpdateState
        # This keeps the subscription active and sets update intervals between 5–10 seconds
        await accumulator.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=1,
            keepSubscriptions=True
        )

        # Track last known state to print changes and save full sequence
        last_state = None
        state_sequence = []  # List to store the full OTA state flow

        def matcher_update_state(report):
            """
            Matcher function to track OTA UpdateState.
            Prints only when state changes and saves the full sequence.
            Handles normal OTA flow: Downloading -> Applying -> Idle
            """
            nonlocal last_state, state_sequence
            val = report.value
            if val is None:
                return False

            # Only log when the value changes
            if last_state != val:
                logger.info(f"1.6 - UpdateState changed: {val}")
                last_state = val
                state_sequence.append(val)
            else:
                return False  # Ignore duplicates, sequence not advanced

            # Return True when Idle is reached (end of expected OTA sequence)
            return val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle

        # Create matcher object for the accumulator
        matcher_update_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState transitions: Downloading > Applying > Idle",
            matcher=matcher_update_state
        )

        try:
            # Wait until the final state (Idle) is reached or timeout (20 min)
            await accumulator.await_all_expected_report_matches([matcher_update_state_obj], timeout_sec=1200.0)
            logger.info("Step #1.6 - Update completed successfully.")
        except Exception as e:
            # Handle OTA errors and timeout (logs like Transfer timed out or retransmission errors)
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel subscription to stop receiving updates
            await accumulator.cancel()

        # After subscription ends, validate the full OTA state sequence
        logger.info(f"Step #1.6 - Full OTA state sequence observed: {state_sequence}")

        # Assert that the expected flow happened at least once
        expected_flow = [
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        ]
        # Assert that the observed OTA state sequence matches exactly the expected flow
        asserts.assert_equal(
            state_sequence,
            expected_flow,
            msg=f"OTA flow did not match expected sequence. Observed: {state_sequence}, Expected: {expected_flow}"
        )


if __name__ == "__main__":
    default_matter_test_main()
