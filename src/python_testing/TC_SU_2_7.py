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
#       --discriminator 123
#       --passcode 2123
#       --secured-device-port 5540
#       --KVS /tmp/chip_kvs_requestor
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --nodeId 123
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_image_expected_version:2
#       --tests test_TC_SU_2_7_1
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 123
#       --passcode 2123
#       --secured-device-port 5540
#       --KVS /tmp/chip_kvs_requestor
#       --requestorCanConsent true
#       --userConsentState deferred
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --nodeId 123
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --tests test_TC_SU_2_7_2
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from time import sleep, time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_7(SoftwareUpdateBaseTest):
    """This test case verifies that the DUT behaves according to the spec when events are generated."""
    # Reference variable for the OTA Software Update Provider cluster.
    TEST_DESCRIPTION = "[TC-SU-2.7] Verifying Events on OTA-R(DUT)"
    PICS = ["MCORE.OTA.Requestor"]
    provider_kvs_path = '/tmp/chip_kvs_provider'
    provider_log = '/tmp/provider_2_7.log'
    ota_prov = Clusters.OtaSoftwareUpdateProvider
    ota_req = Clusters.OtaSoftwareUpdateRequestor
    controller = None
    provider_data = {
        "node_id": 321,
        "discriminator": 321,
        "setup_pincode": 2321
    }
    requestor_setup_pincode = 2123
    requestor_node_id = None

    def all_steps(self) -> list[TestStep]:
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
    async def setup_test(self):
        super().setup_test()
        self.ota_prov = Clusters.OtaSoftwareUpdateProvider
        self.ota_req = Clusters.OtaSoftwareUpdateRequestor
        self.requestor_node_id = self.dut_node_id  # 123 with discriminator 123

    @async_test_body
    async def teardown_test(self):
        self.terminate_provider()
        super().teardown_test()

    def desc_TC_SU_2_7_1(self) -> str:
        return self.TEST_DESCRIPTION

    def desc_TC_SU_2_7_2(self) -> str:
        return self.TEST_DESCRIPTION

    def pics_TC_SU_2_7_1(self):
        """Return the PICS definitions associated with this test."""
        return self.PICS

    def pics_TC_SU_2_7_2(self):
        """Return the PICS definitions associated with this test."""
        return self.PICS

    def steps_TC_SU_2_7_1(self) -> list[TestStep]:
        return self.all_steps()

    def steps_TC_SU_2_7_2(self) -> list[TestStep]:
        return self.all_steps()

    @async_test_body
    async def test_TC_SU_2_7_1(self):
        # Requestor is the DUT
        # Requestor has the flag --autoApply
        self.step(0)
        controller = self.default_controller
        provider_app_path = self.user_params.get('provider_app_path', None)
        ota_image = self.user_params.get('ota_image')
        expected_version = self.user_params.get('ota_image_expected_version')

        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image,
            setup_pincode=self.provider_data['setup_pincode'],
            discriminator=self.provider_data['discriminator'],
            port=5541,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            expected_output='Server initialization complete',
            timeout=10
        )
        await controller.CommissionOnNetwork(
            nodeId=self.provider_data['node_id'],
            setupPinCode=self.provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.provider_data['discriminator']
        )
        await self.set_default_ota_providers_list(controller=controller, provider_node_id=self.provider_data['node_id'], endpoint=0, requestor_node_id=self.requestor_node_id)
        logger.info("About to write acl entries")
        await self.create_acl_entry(dev_ctrl=controller, provider_node_id=self.provider_data['node_id'], requestor_node_id=self.requestor_node_id)

        self.step(1)
        # Craete event subcriber for basicinformation cluster
        basicinformation_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation, expected_event_id=Clusters.BasicInformation.Events.ShutDown.event_id)
        await basicinformation_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=420)
        # Create event subscriber for StateTransition
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=420)
        await self.announce_ota_provider(controller, self.provider_data['node_id'], self.requestor_node_id)
        # Register event, should change to Querying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=5)
        logger.info(f"Event report {event_report}")
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kIdle,
                                           self.ota_req.Enums.UpdateStateEnum.kQuerying, expected_target_version=NullValue)

        # Event for Downloading
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=10)
        logger.info(f"Event report for Downloading {event_report}")
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                           self.ota_req.Enums.UpdateStateEnum.kDownloading, expected_target_version=expected_version)

        # Event for Applying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report for Applying {event_report}")
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kDownloading,
                                           self.ota_req.Enums.UpdateStateEnum.kApplying, expected_target_version=expected_version)

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
            self.requestor_node_id,
            events=[(0, state_transition_event, urgent)],
            fabricFiltered=True
        )
        logger.info(f"StateTransition Gathered {events_response}")
        # Only UpdateAppliedEvent should be in the list
        if len(events_response) == 0:
            asserts.fail("Failed to read the Version Applied Event")
        # Verify StateTransitionEvent
        event_report = events_response[0].Data
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kApplying,
                                           self.ota_req.Enums.UpdateStateEnum.kIdle)

        logger.info(f"UpdateAppliedEvent response: {events_response}")
        self.terminate_provider()

        self.step(2)
        ota_image = self.user_params.get('ota_image')
        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image,
            setup_pincode=self.provider_data['setup_pincode'],
            discriminator=self.provider_data['discriminator'],
            port=5541, extra_args=['--queryImageStatus', 'busy', '--delayedQueryActionTimeSec', '10'],
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=20
        )

        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(controller, self.provider_data['node_id'], self.requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=10)
        self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        # self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
        #                                    expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnQuery, expected_reason=self.ota_req.Enums.ChangeReasonEnum.kDelayByProvider)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        logger.info(f"About close the provider app with proc {self.current_provider_app_proc}")
        self.terminate_provider()

        self.step(3)
        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image,
            setup_pincode=self.provider_data['setup_pincode'],
            discriminator=self.provider_data['discriminator'],
            port=5541,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            expected_output='Server initialization complete',
            timeout=10
        )
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*6)
        # This step we need to Kill the provider PID before the announcement
        logger.info("Killing the provider process")
        self.current_provider_app_proc.kill()
        await self.announce_ota_provider(controller, self.provider_data['node_id'], self.requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event response after killing app: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event response : {event_report}")
        self.verify_state_transition_event(event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kIdle, expected_reason=self.ota_req.Enums.ChangeReasonEnum.kFailure)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        self.terminate_provider()
        self.restart_requestor(controller)

        # # This step is executed with TC_2_7_2
        self.skip_step(4)

        self.step(5)
        ota_image = self.user_params.get('ota_image')
        expected_version = self.user_params.get('ota_image_expected_version')
        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image,
            setup_pincode=self.provider_data['setup_pincode'],
            discriminator=self.provider_data['discriminator'],
            port=5541,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=60
        )
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        error_download_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.DownloadError.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*7)
        await error_download_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)
        await self.announce_ota_provider(controller, self.provider_data['node_id'], self.requestor_node_id)
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
        self.current_provider_app_proc.kill()
        time_middle = time()
        elapsed_time = time_middle - time_start
        logger.info(f"Elapsed time since announce {elapsed_time}")
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
        asserts.assert_equal(download_event_report.softwareVersion, expected_version,
                             f"Expected Software version {expected_version}, found {download_event_report.softwareVersion}")
        asserts.assert_greater(download_event_report.bytesDownloaded, 0, "Download was 0 bytes")
        asserts.assert_greater(download_event_report.progressPercent, 0, "Download progress was 0")
        asserts.assert_equal(download_event_report.platformCode, NullValue,
                             f"Null value not found at platformCode {download_event_report.platformCode}")
        # Cancel Handlers
        error_download_event_handler.reset()
        await error_download_event_handler.cancel()
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        self.terminate_provider()

        self.step(6)
        ota_image = self.user_params.get('ota_image')
        expected_version = self.user_params.get('ota_image_expected_version')
        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image,
            setup_pincode=self.provider_data['setup_pincode'],
            discriminator=self.provider_data['discriminator'],
            port=5541, extra_args=['--applyUpdateAction', 'awaitNextAction', '--delayedApplyActionTimeSec', '3'],
            kvs_path=self.provider_kvs_path,
            log_file='/tmp/provider_log_2_7_3.log',
            timeout=10
        )
        # EventHandlers
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        basicinformation_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation, expected_event_id=Clusters.BasicInformation.Events.ShutDown.event_id)
        await basicinformation_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*8)
        await update_state_attr_handler.start(dev_ctrl=controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(controller, self.provider_data['node_id'], self.requestor_node_id)
        # To avoid possible transiton between Querying and Idle use AttributeHandler
        update_state_match = AttributeMatcher.from_callable(
            "Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)
        await update_state_attr_handler.cancel()
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*12)
        # Download complete then Applying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kApplying)

        # Verification of the testStep DelayedOnApply
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=5)
        logger.info(f"Event report: {event_report}")
        self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kApplying, expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply,
                                           expected_reason=self.ota_req.Enums.ChangeReasonEnum.kDelayByProvider, expected_target_version=NullValue)

        # The following actions is to be able to catch the Event on time for VersionApplied Event
        # Wait until Apply
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report: {event_report}")
        self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply, expected_new_state=self.ota_req.Enums.UpdateStateEnum.kApplying,
                                           expected_reason=self.ota_req.Enums.ChangeReasonEnum.kSuccess, expected_target_version=expected_version)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        # Wait for Restart after ShutdownEvent
        shutdown_event = basicinformation_handler.wait_for_event_report(Clusters.BasicInformation.Events.ShutDown, timeout_sec=60)
        logger.info(f"Shutting down {shutdown_event}")
        basicinformation_handler.reset()
        await basicinformation_handler.cancel()

        self.step(7)
        await asyncio.sleep(5)
        urgent = 1
        version_applied_event = Clusters.OtaSoftwareUpdateRequestor.Events.VersionApplied
        events_response = await controller.ReadEvent(
            self.requestor_node_id,
            events=[(0, version_applied_event, urgent)],
            fabricFiltered=True
        )
        logger.info(f"Events gathered {events_response}")
        # Only UpdateAppliedEvent should be in the list
        if len(events_response) == 0:
            asserts.fail("Failed to read the Version Applied Event")

        version_applied_event_data = events_response[0].Data

        logger.info(f"UpdateAppliedEvent response: {events_response}")
        asserts.assert_equal(expected_version, version_applied_event_data.softwareVersion,
                             f"Software version from VersionAppliedEvent is not {expected_version}")
        asserts.assert_is_not_none(version_applied_event_data.productID, "Product ID from VersionApplied Event is None")
        await self.verify_version_applied_basic_information(controller=controller, node_id=self.requestor_node_id, target_version=expected_version)

    @async_test_body
    async def test_TC_SU_2_7_2(self):
        # Launch the requestor with flags:
        #       --requestorCanConsent true
        #       --userConsentState deferred
        # These flags are needed by the provider to work properly to trigger DelayedOnUserConsent
        # Retrieve the data from the args
        controller = self.default_controller
        provider_app_path = self.user_params.get('provider_app_path', None)
        ota_image = self.user_params.get('ota_image')
        self.step(0)
        self.skip_step(1)
        self.skip_step(2)
        self.skip_step(3)
        self.step(4)
        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image,
            setup_pincode=self.provider_data['setup_pincode'],
            discriminator=self.provider_data['discriminator'],
            port=5541, extra_args=['-u', 'deferred', '-c'],
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            expected_output='Server initialization complete',
            timeout=10
        )
        await controller.CommissionOnNetwork(
            nodeId=self.provider_data['node_id'],
            setupPinCode=self.provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.provider_data['discriminator']
        )
        logger.info("About to write acl entries")
        await self.create_acl_entry(dev_ctrl=controller, provider_node_id=self.provider_data['node_id'], requestor_node_id=self.requestor_node_id)
        logger.info("About to writa ota providers")
        await self.set_default_ota_providers_list(controller=controller, provider_node_id=self.provider_data['node_id'], endpoint=0, requestor_node_id=self.requestor_node_id)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*6)
        await self.announce_ota_provider(controller, self.provider_data['node_id'], self.requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        self.verify_state_transition_event(event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        self.verify_state_transition_event(event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnUserConsent)
        state_transition_event_handler.reset()
        await state_transition_event_handler.cancel()
        self.skip_step(5)
        self.skip_step(6)
        self.skip_step(7)


if __name__ == "__main__":
    default_matter_test_main()
