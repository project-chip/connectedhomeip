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
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing.event_attribute_reporting import EventSubscriptionHandler
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

from mobly import asserts

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_7(MatterBaseTest):
    """This test case verifies that the DUT behaves according to the spec when events are generated."""

    # Reference variable for the OTA Software Update Provider cluster.
    # cluster_otap = Clusters.OtaSoftwareUpdateProvider

    async def write_acl(self, controller, node_id, endpoint, acls):
        result = await controller.WriteAttribute(
            node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl(acls))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        logger.info(f"Status of write ACL: {result}")
        return True

    def desc_TC_SU_2_7(self) -> str:
        return "[TC-SU-2.7] Verifying Events on OTA-R(DUT)"

    def pics_TC_SU_2_7(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",
        ]
        return pics

    def steps_TC_SU_2_7(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "Perform a software update on the DUT.",
                     "Verify that the OTA-Subscriber receives a StateTransition event notification for all the state changes i.e. Querying, Downloading, Applying, Idle (optional)."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to \"Busy\"",
                     "Verify that the OTA-Subscriber receives a StateTransition event notification for the state change to DelayedOnQuery."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P does not respond back to DUT.",
                     "Verify that the OTA-Subscriber receives a StateTransition event notification for the state change to Idle."),
            TestStep(4, "DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT. OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.",
                     "Verify that the OTA-Subscriber receives a StateTransition event notification for the state change to DelayedOnUserConsent."),
            TestStep(5, "Force an error during the download of the OTA image to the DUT. Wait for the Idle timeout which should be no less than 5 minutes.", "Verify that the OTA-Subscriber receives a StateTransition event notification for the state change to Idle."
                     "Verify that the OTA-Subscriber receives a DownloadError event notification on BDX Idle timeout."
                     "Verify that the data in this event has the following."
                     "SoftwareVersion - Set to the value of the SoftwareVersion being downloaded."
                     "BytesDownloaded - Number of bytes that have been downloaded."
                     "ProgressPercent - Nearest Integer percent value reflecting how far within the transfer the failure occurred. IF the total length of the transfer is unknown, the value can be NULL."
                     "PlatformCode - Internal product-specific error code or NULL."),
            TestStep(6, "After the OTA image is transferred, DUT sends ApplyUpdateRequest to the OTA-P. OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\".",
                     "Verify that the OTA-Subscriber receives a StateTransition event notification for the state change to DelayedOnApply."),
            TestStep(7, "DUT successfully finishes applying a software update, and the new software image version is being executed on the DUT. OTA-Subscriber sends a read request to read the VersionApplied event from the DUT.", "Verify that the VersionApplied event is generated whenever a new version starts executing after being applied due to a software update."
                     "Verify that the data in this event has the following."
                     "SoftwareVersion - Same as the one available in the SoftwareVersion attribute of the Basic Information Cluster for the newly executing version."
                     "ProductID - Same as what is available in the ProductID attribute of the Basic Information Cluster."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_7(self):

        #### Apps to run for this test case ###
        # Terminal 1: ./out/debug/chip-ota-requestor-app --discriminator 1234 --passcode 20202021 --secured-device-port 5541 --autoApplyImage --KVS /tmp/chip_kvs_requestor
        # Terminal 2:  ./out/debug/chip-ota-provider-app --filepath firmware_requestor_v2.ota  --discriminator 1111
        ###

        # Clusters to use
        ota_prov = Clusters.OtaSoftwareUpdateProvider
        ota_req = Clusters.OtaSoftwareUpdateRequestor
        basic_information = Clusters.BasicInformation

        # Define varaibles to use in test case
        update_software_version = '2'
        upate_sorftware_version_str = '2.0'

        # Requestor is the DUT
        admin_node_id = self.default_controller.nodeId
        controller = self.default_controller
        requestor_node_id = self.dut_node_id  # 123 with discriminator 123
        th1_fabric_id = controller.fabricId

        # Provider
        provider_node_id = 321
        provider_discriminator = 321
        provider_setup_pincode = 20202021

        logger.info(f"Admin nodeId: {admin_node_id}")
        logger.info(f"TH1 -> FabricId: {th1_fabric_id} DUT NodeId: {requestor_node_id}")
        logger.info(f"TH2 -> FabricId: {th1_fabric_id} TH2 NodeId: {provider_node_id}")

        # open commissioning window for provider
        params = await self.open_commissioning_window(controller, requestor_node_id)

        # commission TH2 for
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )

        self.step(0)

        ### Configure ACLs Rules ###
        # This works #
        # ACL to Write,Read,Administer into Provider
        acl_admin_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=ota_prov.id
            )],
        )

        # Provider can operate (modify ACLs etc.) on Provider - optional
        acl_operate_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[
                Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=0,
                    cluster=ota_prov.id,
                )
            ]
        )

        acl_entries = [acl_admin_provider,  acl_operate_provider]
        resp_acl_th1 = await self.write_acl(controller, provider_node_id, 0, acl_entries)
        logger.info(f" Wrote ACLs to gain access to Requestor Cluster: {resp_acl_th1}")

        # out/chip-tool accesscontrol write acl '[
        # {"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null},
        # {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}
        # ]'
        # 0xDEADBEEF 0
        # # ACL for Requestor can send commands to OTA provider
        # This does not work
        # acl_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
        #     privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
        #     authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
        #     subjects=[admin_node_id],
        #     targets=NullValue
        # )

        # # TH1 can administer to OTA Provider
        # acl_operate = Clusters.AccessControl.Structs.AccessControlEntryStruct(
        #     privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
        #     authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
        #     subjects=[],
        #     targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
        #         endpoint=NullValue,
        #         cluster=ota_prov.id
        #     )]
        # )

        # acls = [acl_admin, acl_operate]
        # resp_acl_th2 = await self.write_acl(controller, requestor_node_id, 0, acls)
        # logger.info(f" Wrote ACLs on Requestor to allow access from Provider: {resp_acl_th2}")

        current_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=ota_req,
            attribute=ota_req.Attributes.DefaultOTAProviders
        )
        logger.info(f"OTA Providers: {current_otap_info}")

        # Create Provider Location into Requestor
        provider_location_struct = ota_req.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=0,
            fabricIndex=th1_fabric_id
        )

        # Create the OTA Provider Attribute
        ota_providers_attr = ota_req.Attributes.DefaultOTAProviders(value=[provider_location_struct])

        # Write the Attribute
        resp = await controller.WriteAttribute(
            attributes=[(0, ota_providers_attr)],
            nodeid=requestor_node_id,
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write Default OTA Providers Attribute")

        # Updated OTAProviders
        after_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=ota_req,
            attribute=ota_req.Attributes.DefaultOTAProviders
        )
        logger.info(f"OTA Providers: {after_otap_info}")

        # Software update steps defined in the MatterSpecs at 11.20.3 diagram.
        self.step(1)

        # Create event subscriber
        events_subscriber_requestor = EventSubscriptionHandler(expected_cluster=ota_req)
        await events_subscriber_requestor.start(controller, requestor_node_id, 0)

        cmd_announce_ota_provider = ota_req.Commands.AnnounceOTAProvider(
            providerNodeID=requestor_node_id,
            vendorID=0xFFF1,
            announcementReason=ota_req.Enums.AnnouncementReasonEnum.kUrgentUpdateAvailable,
            metadataForNode=None,
            endpoint=0
        )
        logger.info("Sending AnnounceOTA Provider Command")
        cmd_resp = await self.send_single_cmd(
            cmd=cmd_announce_ota_provider,
            dev_ctrl=controller
        )

        # Register event
        event_report = events_subscriber_requestor.wait_for_event_report(ota_req.Events.StateTransition, timeout_sec=60)
        # Start on idle to then change to Querying
        asserts.assert_equal(event_report.previousState, ota_req.Enums.UpdateStateEnum.kIdle, "Previous state was not Idle")
        asserts.assert_equal(event_report.newState,  ota_req.Enums.UpdateStateEnum.kQuerying, "New state is not KQueryng")
        logger.info(f"Event report {event_report}")

        self.step(2)

        # cmd_query_image = ota_prov.Commands.QueryImage(
        #     vendorID=0xFFF1,
        #     productID=0x8001,
        #     softwareVersion=2,
        #     protocolsSupported=[ota_prov.Enums.DownloadProtocolEnum.kBDXSynchronous],
        #     hardwareVersion=None,
        #     location=None,
        #     requestorCanConsent=None,
        #     metadataForProvider=None
        # )
        events_subscriber_requestor = EventSubscriptionHandler(expected_cluster=ota_req)
        await events_subscriber_requestor.start(controller, requestor_node_id, 0)
        logger.info("Sending QueryImage Command")
        # cmd_query_image_rsp = await self.send_single_cmd(
        #     cmd=cmd_query_image,
        #     node_id=provider_node_id,
        #     dev_ctrl=controller,
        #     endpoint=0
        # )
        # logger.info(f"Response from Query image: {cmd_query_image_rsp}")

        event_report = events_subscriber_requestor.wait_for_event_report(ota_req.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report after QueryImage {event_report}")

        self.step(3)

        self.step(4)

        self.step(5)

        self.step(6)

        self.step(7)
        events_subscriber_requestor = EventSubscriptionHandler(expected_cluster=ota_req)
        await events_subscriber_requestor.start(controller, requestor_node_id, 0)
        event_report = events_subscriber_requestor.wait_for_event_report(ota_req.Events.VersionApplied, timeout_sec=60)
        version_applied_event = event_report.softwareVersion

        basicinfo_softwareversion = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=basic_information,
            attribute=basic_information.Attributes.softwareVersion
        )

        # basicinfo_softwareversion_str = await self.read_single_attribute_check_success(
        #     dev_ctrl=controller,
        #     cluster=basic_information,
        #     attribute=basic_information.Attributes.softwareVersionString
        # )

        asserts.assert_equal(version_applied_event, basicinfo_softwareversion, "Versions are not the same")


if __name__ == "__main__":
    default_matter_test_main()
