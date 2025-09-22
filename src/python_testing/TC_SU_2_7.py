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
#       --passcode 2123
#       --secured-device-port 5540
#       --KVS /tmp/chip_kvs_requestor
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 2123
#       --vendor-id 65521
#       --product-id 32769
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from os import environ, getcwd, kill, setpgrp
from signal import SIGTERM
from subprocess import Popen, run
from time import sleep, time

import psutil
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.apps import OtaImagePath, OTAProviderSubprocess
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
    current_requestor_app_pid = None
    current_provider_app_proc = None
    current_requestor_app_proc = None

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

    async def _verify_version_applied_basic_information(self, controller, node_id: int, target_version):

        basicinfo_softwareversion = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=node_id)
        asserts.assert_equal(basicinfo_softwareversion, target_version,
                             f"Version from basic info cluster is not {target_version}, current cluster version is {basicinfo_softwareversion}")

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

        # Read Updated OTAProviders
        after_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=self.ota_req,
            attribute=self.ota_req.Attributes.DefaultOTAProviders
        )
        logger.info(f"OTA Providers List: {after_otap_info}")

    async def _announce_ota_provider(self, controller, provider_node_id,  requestor_node_id, reason: Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum = Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable):
        cmd_announce_ota_provider = self.ota_req.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=0xFFF1,
            announcementReason=reason,
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

    def _launch_app(self, app_name_path: str, env_app_name: str, base_params: list = [], extra_params: list = [], log: str = "provider.log") -> int:
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
        params.insert(0, 'nohup')
        str_cmd = " ".join(params)
        logger.info(f"CMD {str_cmd}")
        process = Popen(args=str_cmd, shell=True, stdout=open(log, 'a'),
                        stderr=open(f"{getcwd()}/{log}.err", 'a'), preexec_fn=setpgrp)
        logger.info(f"Launched app {env_app_name} with pid {process.pid}")
        # Wait app to startup
        sleep(3)
        return process

    def _launch_provider_app(self, version: int = 2, extra_args: list = []):
        """Launch the provider app with different configurations.
        """
        logger.info(f"LAUNCHING PROVIDER APP WITH VERSION {version}")

        # Image to launch
        ota_app = ""
        if environ.get("OTA_PROVIDER_APP") is not None:
            ota_app = environ.get("OTA_PROVIDER_APP")
        else:
            ota_app = f"{getcwd()}/out/debug/chip-ota-provider-app"

        # Ota image
        ota_image = ""
        if environ.get(f"SU_OTA_REQUESTOR_V{version}") is not None:
            ota_image = environ.get(f"SU_OTA_REQUESTOR_V{version}")
        else:
            ota_image = f"{getcwd()}/chip-ota-requestor-app_v{version}.min.ota"

        ota_image_path = OtaImagePath(path=ota_image)

        proc = OTAProviderSubprocess(
            ota_app,
            storage_dir='/tmp',
            port=5541,
            discriminator=321,
            passcode=2321,
            ota_source=ota_image_path,
            extra_args=extra_args,
            f_stdout=open('/tmp/provider.log', 'a+b'))
        proc.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.current_provider_app_proc = proc
        logger.info(f"PROC {proc}")

    def _launch_requestor_app(self, extra_params: list = []):
        logger.info("LAUNCHING REQUESTOR APP")
        proc = self._launch_app(
            app_name_path='out/debug/chip-ota-requestor-app',
            env_app_name="OTA_REQUESTOR_APP",
            base_params=[
                "--discriminator 123",
                '--passcode 2123',
                "--secured-device-port 5540",
                "--KVS /tmp/chip_kvs_requestor",
            ],
            extra_params=extra_params,
            log="requestor.log"
        )
        self.current_requestor_app_proc = proc
        return proc

    def _verify_event_transition_status(self, event_report, previous_state, new_state, target_version=None, reason=None):
        logger.info(f"Verifying the event {event_report}")
        asserts.assert_equal(event_report.previousState, previous_state, f"Previous state was not {previous_state}")
        asserts.assert_equal(event_report.newState,  new_state, f"New state is not {new_state}")
        if target_version is not None:
            asserts.assert_equal(event_report.targetSoftwareVersion,  target_version, f"Target version is not {target_version}")
        if reason is not None:
            asserts.assert_equal(event_report.reason,  reason, f"Reason is not {reason}")

    def _kill_provider_process(self):
        if self.current_provider_app_proc is not None:
            logger.info(f"Killing provider with pid {self.current_provider_app_proc.get_pid()}")
            self.current_provider_app_proc.kill()
            run("rm -rf /tmp/chip_kvs_provider*", shell=True)
            self.current_provider_app_proc = None

    def _terminate_provider_process(self):
        if self.current_provider_app_proc is not None:
            logger.info(f"Terminating provider with pid {self.current_provider_app_proc.get_pid()}")
            self.current_provider_app_proc.terminate()
            run("rm -rf /tmp/chip_kvs_provider*", shell=True)
            self.current_provider_app_proc = None
            # Wait SIGTERM
            sleep(2)

    def _terminate_requestor_process(self):
        if self.current_requestor_app_proc is not None:
            self.current_requestor_app_proc.terminate()
            self.current_requestor_app_proc = None
        else:
            kill(self.current_requestor_app_pid, SIGTERM)
            self.current_requestor_app_pid = None
        run("rm -rf /tmp/chip_kvs_requestor*", shell=True)
        # Wait SIGTERM
        sleep(2)

    @async_test_body
    async def teardown_test(self):
        self._terminate_provider_process()
        self._terminate_requestor_process()

    @async_test_body
    async def test_TC_SU_2_7(self):

        #### Apps to run for this test case ###
        # Terminal 1: ./out/debug/chip-ota-requestor-app --discriminator 123 --passcode 2123 --secured-device-port 5540 --KVS /tmp/chip_kvs_requestor -a
        # Terminal 2: ./out/debug/chip-ota-provider-app --filepath firmware_requestor_v2.min.ota  --discriminator 321 --passcode 2321 --secured-device-port 5541
        # Terminal 3: python3 src/python_testing/TC_SU_2_7.py --commissioning-method on-network --passcode 2123 --discriminator 123 --endpoint 0 --nodeId 123
        ###
        # Define varaibles to use in test case
        update_software_version = 2
        self.current_requestor_app_pid = self._get_pid_by_name("chip-ota-requestor-app")
        if self.current_requestor_app_pid is None:
            asserts.fail("Failed to find the PID for chip-ota-requestor-app")
        logger.info(f"Test started with provider PID {self.current_requestor_app_pid}")

        # Requestor is the DUT
        admin_node_id = self.default_controller.nodeId
        controller = self.default_controller
        requestor_node_id = self.dut_node_id  # 123 with discriminator 123
        requestor_setup_pincode = 2123

        # Provider
        provider_data = {
            "node_id": 321,
            "discriminator": 321,
            "setup_pincode": 2321
        }

        logger.info(f"Admin nodeId: {admin_node_id}")
        logger.info(f"Requestor NodeId: {requestor_node_id}")
        logger.info(f"Provider NodeId: {provider_data['node_id']}")

        # Launch ProviderApp
        self.step(0)
        self._launch_provider_app()
        # Commission Provider
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        # write the rules into the provider

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)

        self.step(1)
        # Craete event subcriber for basicinformation cluster
        basicinformation_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation, expected_event_id=Clusters.BasicInformation.Events.ShutDown.event_id)
        await basicinformation_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)
        # Create event subscriber for StateTransition
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*6)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        # Register event, should change to Querying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=5)
        logger.info(f"Event report {event_report}")
        self._verify_event_transition_status(event_report, self.ota_req.Enums.UpdateStateEnum.kIdle,
                                             self.ota_req.Enums.UpdateStateEnum.kQuerying, target_version=NullValue)

        # Event for Downloading
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=10)
        logger.info(f"Event report after QueryImage {event_report}")
        self._verify_event_transition_status(event_report, self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                             self.ota_req.Enums.UpdateStateEnum.kDownloading, target_version=update_software_version)

        # Event for Applying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report for Complete transfer {event_report}")
        self._verify_event_transition_status(event_report, self.ota_req.Enums.UpdateStateEnum.kDownloading,
                                             self.ota_req.Enums.UpdateStateEnum.kApplying, target_version=update_software_version)

        # Wait until the device restarts
        bi_event_report = basicinformation_handler.wait_for_event_report(Clusters.BasicInformation.Events.ShutDown, timeout_sec=60)
        logger.info(f"Shutting down event: {bi_event_report}")
        basicinformation_handler.reset()
        await basicinformation_handler.cancel()
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        await asyncio.sleep(5)
        # After restart read the events from the start (Startup)
        urgent = 1
        state_transition_event = self.ota_req.Events.StateTransition
        events_response = await controller.ReadEvent(
            requestor_node_id,
            events=[(0, state_transition_event, urgent)],
            fabricFiltered=True
        )
        logger.info(f"StateTransition Gathered {events_response}")
        # Only UpdateAppliedEvent should be in the list
        if len(events_response) == 0:
            asserts.fail("Failed to read the Version Applied Event")
        # Verify StateTransitionEvent
        event_report = events_response[0].Data
        self._verify_event_transition_status(event_report, self.ota_req.Enums.UpdateStateEnum.kApplying,
                                             self.ota_req.Enums.UpdateStateEnum.kIdle)

        logger.info(f"UpdateAppliedEvent response: {events_response}")

        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])
        self._terminate_requestor_process()
        controller.ExpireSessions(nodeid=requestor_node_id)
        # Cleaned up so version is reset

        self.step(2)
        self._launch_requestor_app(extra_params=["-a"])
        await controller.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=requestor_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=requestor_node_id
        )
        self._launch_provider_app(extra_args=['--delayedQueryActionTimeSec', '60', '--queryImageStatus', 'busy'])
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*6)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=10)
        self._verify_event_transition_status(event_report=event_report, previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
                                             new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        self._verify_event_transition_status(event_report=event_report, previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                             new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnQuery, reason=self.ota_req.Enums.ChangeReasonEnum.kDelayByProvider)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        logger.info(f"About close the provider app with proc {self.current_provider_app_proc}")
        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(3)
        self._launch_provider_app()
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*6)
        # This step we need to Kill the provider PID before the announcement
        self._kill_provider_process()
        sleep(1)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event response after killing app: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event response : {event_report}")
        self._verify_event_transition_status(event_report, previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                             new_state=self.ota_req.Enums.UpdateStateEnum.kIdle, reason=self.ota_req.Enums.ChangeReasonEnum.kFailure)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(4)
        # Need to launch the requestor with -c true
        self._terminate_requestor_process()
        controller.ExpireSessions(nodeid=requestor_node_id)
        self._launch_requestor_app(extra_params=["-c true", "-u deferred"])
        await controller.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=requestor_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=requestor_node_id
        )
        self._launch_provider_app(extra_args=['-u', 'deferred', '-c'])
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*6)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        self._verify_event_transition_status(event_report, previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
                                             new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        self._verify_event_transition_status(event_report, previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                             new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnUserConsent)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(5)
        self._terminate_requestor_process()
        controller.ExpireSessions(nodeid=requestor_node_id)
        self._launch_requestor_app()
        await controller.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=requestor_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=requestor_node_id
        )

        update_software_version = 3
        self._launch_provider_app(version=update_software_version)
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        error_download_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.DownloadError.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*12)
        await error_download_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        time_start = time()
        # Block waiting for Download
        logger.info("About to wait for StateTransition Events")
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report Downloading {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report Downloading {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDownloading)
        # Wait some time to let it download some data and then Kill the current process and remove kvs files
        logger.info("Wait 10 seconds to allow download some data")
        sleep(10)
        self._kill_provider_process()
        time_middle = time()
        elapsed_time = time_middle - time_start
        logger.info(f"Elapsed time since amnounce {elapsed_time}")
        # wait to until the 5 minutes
        minimun_wait_time = (60*5)-elapsed_time
        logger.info(f"Script will wait for {minimun_wait_time} to match the minimum 5 minutes wait ")
        sleep(minimun_wait_time)
        logger.info("Completed waiting for 5 minutes")
        # Somehow here the provider app should be terminated
        # Download error should not come out in less than 5 minutes
        download_event_report = error_download_event_handler.wait_for_event_report(
            self.ota_req.Events.DownloadError, timeout_sec=60*10)
        logger.info(f"Download error Event: {download_event_report}")
        asserts.assert_equal(download_event_report.softwareVersion, update_software_version,
                             f"Expected Software version {update_software_version}, found {download_event_report.softwareVersion}")
        asserts.assert_greater(download_event_report.bytesDownloaded, 0, "Download was 0 bytes")
        asserts.assert_greater(download_event_report.progressPercent, 0, "Download progress was 0")
        asserts.assert_equal(download_event_report.platformCode, NullValue,
                             f"Null value not found at platformCode {download_event_report.platformCode}")
        # Cancel Handlers
        error_download_event_handler.reset()
        await error_download_event_handler.cancel()
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(6)
        # Again the process to update the cluster but we check the Delayed on Apply
        # Autoapply flag must be disabled in requestor in order to work
        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])
        self._terminate_requestor_process()
        controller.ExpireSessions(nodeid=requestor_node_id)
        self._launch_requestor_app(extra_params=["-a"])
        await controller.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=requestor_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=requestor_node_id
        )
        update_software_version = 4
        provider_data['node_id'] = 322
        self._launch_provider_app(version=update_software_version, extra_args=[
                                  '--applyUpdateAction', 'awaitNextAction', '--delayedApplyActionTimeSec', '5'])
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )
        # EventHandlers
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)
        basicinformation_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation, expected_event_id=Clusters.BasicInformation.Events.ShutDown.event_id)
        await basicinformation_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self._write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        await self._announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report Querying {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report Downloading {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDownloading)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kApplying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=5)
        logger.info(f"Event report: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply,
                             f"Event status is not {self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply}")
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report for Complete transfer {event_report}")
        self._verify_event_transition_status(event_report, self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply,
                                             self.ota_req.Enums.UpdateStateEnum.kApplying, target_version=update_software_version)
        # Wait for Restart or ShutdownEvent
        shutdown_event = basicinformation_handler.wait_for_event_report(Clusters.BasicInformation.Events.ShutDown, timeout_sec=60)
        logger.info(f"Shutting down {shutdown_event}")
        await state_transition_event_handler.cancel()
        await basicinformation_handler.cancel()

        self.step(7)
        await asyncio.sleep(5)
        urgent = 1
        version_applied_event = Clusters.OtaSoftwareUpdateRequestor.Events.VersionApplied
        events_response = await controller.ReadEvent(
            requestor_node_id,
            events=[(0, version_applied_event, urgent)],
            fabricFiltered=True
        )
        logger.info(f"Events gathered {events_response}")
        # Only UpdateAppliedEvent should be in the list
        if len(events_response) == 0:
            asserts.fail("Failed to read the Version Applied Event")

        version_applied_event_data = events_response[0].Data

        logger.info(f"UpdateAppliedEvent response: {events_response}")
        asserts.assert_equal(update_software_version, version_applied_event_data.softwareVersion,
                             f"Software version from VersionAppliedEvent is not {update_software_version}")
        asserts.assert_is_not_none(version_applied_event_data.productID, "Product ID from VersionApplied Event is None")
        await self._verify_version_applied_basic_information(controller=controller, node_id=requestor_node_id, target_version=update_software_version)


if __name__ == "__main__":
    default_matter_test_main()
