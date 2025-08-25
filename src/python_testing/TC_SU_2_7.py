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
import signal
from os import environ, getcwd, kill
from subprocess import PIPE, Popen
from time import sleep

import psutil
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_7(MatterBaseTest):
    """This test case verifies that the DUT behaves according to the spec when events are generated."""

    # Reference variable for the OTA Software Update Provider cluster.
    # cluster_otap = Clusters.OtaSoftwareUpdateProvider

    ota_prov = Clusters.OtaSoftwareUpdateProvider
    ota_req = Clusters.OtaSoftwareUpdateRequestor
    current_provider_app_pid = None
    current_requestor_app_pid = None

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

    def _get_pid_by_name(self, process_name):
        """
        Finds the PID of a process by its name.
        Returns the PID if found, None otherwise.
        """
        for proc in psutil.process_iter(['pid', 'name']):
            if proc.info['name'] == process_name:
                return proc.info['pid']
        return None

    async def _verify_version_applied_basic_information(self, controller, target_version):

        basicinfo_softwareversion = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

        # asserts.assert_equal(version_applied_event, target_version, f"Version fom event is not {update_software_version}")
        asserts.assert_equal(basicinfo_softwareversion, target_version,
                             f"Version from basic info cluster is not {target_version}")

    async def _write_acl_rules(self, controller, endpoint: int, node_id):
        logger.info("Configure ACL Entries")
        admin_node_id = controller.nodeId
        logging.info(f"Admin node id is {admin_node_id}")
        logging.info(f"FabricId value: {controller.fabricId}")

        acl_attr_base = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.AccessControl,
            attribute=Clusters.AccessControl.Attributes.Acl,
            node_id=node_id,
        )
        logger.info(f"Requestor base acl {acl_attr_base}")

        acl_entries = [
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                fabricIndex=controller.fabricId,
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[admin_node_id],
                targets=NullValue
            ),
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                fabricIndex=controller.fabricId,
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=NullValue,
                    cluster=self.ota_prov.id,
                    deviceType=NullValue
                )]
            )
        ]

        all_acls = acl_attr_base + acl_entries

        acl_attr = Clusters.Objects.AccessControl.Attributes.Acl(value=all_acls)
        resp = await controller.WriteAttribute(node_id, [(endpoint, acl_attr)])
        asserts.assert_equal(resp[0].Status, Status.Success, "ACL write failed.")
        logger.info("ACL permissions configured successfully.")

    async def write_acl(self, controller, node_id, endpoint, acls):
        result = await controller.WriteAttribute(
            node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl(acls))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        logger.info(f"Status of write ACL: {result}")
        return True

    async def _write_ota_providers(self, controller, provider_node_id, endpoint: int = 0):

        current_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=self.ota_req,
            attribute=self.ota_req.Attributes.DefaultOTAProviders
        )
        logger.info(f"OTA Providers: {current_otap_info}")

        # Create Provider Location into Requestor
        provider_location_struct = self.ota_req.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=endpoint,
            fabricIndex=controller.fabricId
        )

        # Create the OTA Provider Attribute
        ota_providers_attr = self.ota_req.Attributes.DefaultOTAProviders(value=[provider_location_struct])

        # Write the Attribute
        resp = await controller.WriteAttribute(
            attributes=[(endpoint, ota_providers_attr)],
            nodeid=self.dut_node_id,
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write Default OTA Providers Attribute")

    async def _announce_ota_provider(self, controller, provider_node_id,  requestor_node_id):
        cmd_announce_ota_provider = self.ota_req.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=0xFFF1,
            announcementReason=self.ota_req.Enums.AnnouncementReasonEnum.kUpdateAvailable,
            metadataForNode=None,
            endpoint=0
        )
        logger.info("Sending AnnounceOTA Provider Command")
        cmd_resp = await self.send_single_cmd(
            cmd=cmd_announce_ota_provider,
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
        )
        logger.info(f"Announce command sent {cmd_resp}")
        return cmd_resp

    def _launch_app(self, app_name_path: str, env_app_name: str, base_params: list = [], extra_params: list = []) -> int:
        """_summary_

        Args:
            app_name_path (str): _description_
            env_app_name (str): _description_
            base_params (list, optional): _description_. Defaults to [].
            extra_params (list, optional): _description_. Defaults to [].

        Returns:
            int: PID of the process
        """

        base_path = getcwd()
        app_path = ''
        # Verify if the env variable for the ota provider is available
        if environ.get(env_app_name):
            app_path = environ.get(env_app_name)
        else:
            app_path = f"{base_path}/{app_name_path}"
        params = base_params + extra_params
        params.insert(0, app_path)
        print(" ".join(params))
        process = Popen(params, shell=True, stdout=PIPE, stderr=PIPE)
        return process

    def _launch_provider_app(self, extra_params: list = []):
        """Launch the provider app with different configurations.
        """
        pid = self._launch_app(
            app_name_path='out/debug/chip-ota-provider-app',
            env_app_name="OTA_PROVIDER_APP",
            base_params=[
                '--filepath firmware_requestor_v2.min.ota',
                '--discriminator 321',
                '--secured-device-port 5541', "--passcode 2321",
            ],
            extra_params=extra_params
        )
        sleep(5)
        return pid

    def _launch_requestor_app(self, extra_params: list = []):
        pid = self._launch_app(
            app_name_path='out/debug/chip-ota-requestor-app',
            env_app_name="OTA_REQUESTOR_APP",
            base_params=[
                "--discriminator 123",
                '--passcode 2123',
                "--secured-device-port 5540",
                "--KVS /tmp/chip_kvs_requestor",
            ],
            extra_params=extra_params,
        )
        return pid

    def _kill_apps(self):
        logger.info("Killing apps PROVIDER and REQUESTOR")
        kill(self.current_provider_app_pid, signal.SIGTERM)
        kill(self.current_provider_app_pid, signal.SIGTERM)
        # Clean the kvs files

    @async_test_body
    async def test_TC_SU_2_7(self):

        #### Apps to run for this test case ###
        # Terminal 1: ./out/debug/chip-ota-requestor-app --discriminator 123 --passcode 2123 --secured-device-port 5541 --autoApplyImage --KVS /tmp/chip_kvs_requestor --autoApplyImage
        # Terminal 2: ./out/debug/chip-ota-provider-app --filepath firmware_requestor_v2.min.ota  --discriminator 321 --passcode 2321
        # Terminal 3: python3 src/python_testing/TC_SU_2_7.py --commissioning-method on-network --passcode 2123 --discriminator 123 --endpoint 0 --nodeId 123
        ###
        # Define varaibles to use in test case
        update_software_version = 2
        self.current_provider_app_pid = self._get_pid_by_name("chip-ota-provider-app")
        logger.info(f"Test started with provider PID {self.current_provider_app_pid}")

        # Requestor is the DUT
        admin_node_id = self.default_controller.nodeId
        controller = self.default_controller
        requestor_node_id = self.dut_node_id  # 123 with discriminator 123

        # Provider
        provider_data = {
            "node_id": 321,
            "discriminator": 321,
            "setup_pincode": 2321
        }

        logger.info(f"Admin nodeId: {admin_node_id}")
        logger.info(f"Requestor NodeId: {requestor_node_id}")
        logger.info(f"Provider NodeId: {provider_data['node_id']}")

        # commission TH2 for
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        self.step(0)

        # write the rules into the provider
        await self._write_acl_rules(controller=controller, endpoint=0, node_id=provider_data['node_id'])

        acl_attr = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.AccessControl,
            attribute=Clusters.AccessControl.Attributes.Acl,
            node_id=provider_data['node_id'],
        )
        logger.info(f"AFTER ACL Entries {acl_attr}")

        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)

        # Read Updated OTAProviders
        after_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=self.ota_req,
            attribute=self.ota_req.Attributes.DefaultOTAProviders
        )
        logger.info(f"OTA Providers List: {after_otap_info}")

        # Software update steps defined in the MatterSpecs at 11.20.3 diagram.
        self.step(1)

        # Create event subscriber
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)
        update_applied_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.VersionApplied.event_id)
        await update_applied_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        # Register event
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=5)

        # Start on idle to then change to Querying
        asserts.assert_equal(event_report.previousState, self.ota_req.Enums.UpdateStateEnum.kIdle, "Previous state was not Idle")
        asserts.assert_equal(event_report.newState,  self.ota_req.Enums.UpdateStateEnum.kQuerying, "New state is not KQueryng")
        asserts.assert_equal(event_report.targetSoftwareVersion,  NullValue, "target is not Null")
        logger.info(f"Event report {event_report}")

        # Event for Downloading
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=10)
        logger.info(f"Event report after QueryImage {event_report}")
        asserts.assert_equal(event_report.previousState, self.ota_req.Enums.UpdateStateEnum.kQuerying,
                             "Previous state was not Querying")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDownloading,
                             "Current state is not Downloading")
        asserts.assert_equal(event_report.targetSoftwareVersion,  update_software_version,
                             f"Target version is not {update_software_version} from {event_report.targetSoftwareVersion}")

        # # Event for Applying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*6)
        logger.info(f"Event report for Complete transfer {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kApplying, "Current state is not Applying")
        asserts.assert_equal(event_report.previousState, self.ota_req.Enums.UpdateStateEnum.kDownloading,
                             "Current state is not Applying")
        asserts.assert_equal(event_report.targetSoftwareVersion,  update_software_version,
                             f"Target version is not {update_software_version} from {event_report.targetSoftwareVersion}")

        self.step(2)
        self._kill_apps()
        # self._launch_provider_app(extra_params=["--delayedQueryActionTimeSec 60","--queryImageStatus  busy"])
        # self._launch_requestor_app(extra_params=[])
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying,
                             "Current state was not Querying")
        logger.info(f"Event report after QueryImage {event_report}")
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDelayedOnQuery,
                             "Current state is not DelayedOnQuery")
        asserts.assert_equal(event_report.reason, self.ota_req.Enums.ChangeReasonEnum.kDelayByProvider,
                             "Current state is not DelayedOnQuery")
        logger.info(f"Event report after 2 {event_report}")

        self.step(3)
        self._kill_apps()
        # self._launch_provider_app(extra_params=[])
        # self._launch_requestor_app(extra_params=["--autoApplyImage"])
        logger.info(f"Killing ota-provider-app PID {self.current_provider_app_pid}")
        kill(self.current_provider_app_pid, signal.SIGTERM)
        sleep(2)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*1)
        # logger.info(f"First event {event_report}")
        # asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying,"Current state is not Idle")
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        # logger.info(f"Second event {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kIdle, "Current state is not Idle")
        asserts.assert_equal(event_report.reason, self.ota_req.Enums.ChangeReasonEnum.kFailure, "Current reason is not failure")

        self.step(4)
        # Launch the provider app with user consent flag true and  deferred to trigger the event
        self._kill_apps()
        # self._launch_provider_app(extra_params=["-u deferred", "-c"])
        # self._launch_requestor_app(extra_params=["-c true", "-u deferred"])
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDelayedOnUserConsent)

        self.step(5)
        self._kill_apps()
        # self._launch_provider_app()
        # self._launch_requestor_app()
        error_download_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.DownloadError.event_id)
        await error_download_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = error_download_event_handler.wait_for_event_report(self.ota_req.Events.DownloadError, timeout_sec=60*5)
        asserts.assert_equal(event_report.softwareversion, update_software_version)
        # asserts.assert_equal(event_report.bytesdownloaded, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        # asserts.assert_equal(event_report.progresspercent, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        asserts.assert_equal(event_report.platformcode, NullValue)

        self.step(6)
        # fails
        # Again the process to update the cluster but we check the Delayed on Apply
        # Delayed on apply
        # self._launch_provider_app(extra_params["--applyUpdateAction awaitNextAction","--delayedApplyActionTimeSec 10"])
        # self._launch_requestor_app()
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDownloading)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply)

        self.step(7)
        # Update is completed at this point, listen for
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*2)
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kApplying)
        event_report_version_applied = update_applied_event_handler.wait_for_event_report(
            self.ota_req.Events.VersionApplied, timeout_sec=60*2)
        self._verify_version_applied_basic_information(controller=controller, target_version=update_software_version)
        asserts.assert_equal(event_report_version_applied.softwareVersion, update_software_version,
                             f"Version fom event is not {update_software_version}")


if __name__ == "__main__":
    default_matter_test_main()
