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
#       --KVS /tmp/chip_kvs_requestor
#       --trace-to json:${TRACE_APP}.json
#       --autoApplyImage
#       --requestorCanConsent true
#       --userConsentState deferred
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_image_expected_version:2
#       --int-arg ota_image_download_timeout:360
#       --timeout 1800
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from time import time

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
    provider_port = None
    provider_kvs_path = None
    provider_log = None
    ota_prov = Clusters.OtaSoftwareUpdateProvider
    ota_req = Clusters.OtaSoftwareUpdateRequestor
    controller = None
    provider_node_id = 321
    provider_discriminator = 321
    provider_setup_pincode = 2321
    requestor_node_id = None
    ota_image_download_timeout = 0

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.ota_prov = Clusters.OtaSoftwareUpdateProvider
        self.ota_req = Clusters.OtaSoftwareUpdateRequestor
        self.requestor_node_id = self.dut_node_id
        self.controller = self.default_controller

        self.ota_image = self.user_params.get('ota_image')
        self.expected_software_version = self.user_params.get('ota_image_expected_version')
        self.provider_app_path = self.user_params.get('provider_app_path')
        self.provider_port = self.user_params.get('ota_provider_port', 5541)
        self.provider_kvs_path = self.user_params.get('provider_kvs_path', '/tmp/chip_kvs_provider')
        self.provider_log = self.user_params.get('provider_log_path', '/tmp/provider_2_7.log')
        # On average the ota image build for the CI is 1.8 MB which takes 4-6 min to download. Adjust time if needed.
        self.ota_image_download_timeout = self.user_params.get('ota_image_download_timeout', 60*6)
        logger.info(f"Image download timeout is set to {self.ota_image_download_timeout} seconds")

        if not self.provider_kvs_path.startswith('/tmp'):
            asserts.fail("Provider KVS path must be placed in the /tmp directory.")

        if self.ota_image_download_timeout <= 0:
            asserts.fail("Invalid value for --int-arg ota_image_download_timeout:<seconds> value provided, must be equal or greater than 1.")

        if not self.expected_software_version:
            asserts.fail("Missing OTA image software version. Speficy using --int-arg ota_image_expected_version:<ota_image_expected_version>")

        if not self.provider_app_path:
            asserts.fail("Missing provider app path . Speficy using --string-arg provider_app_path:<provider_app_path>")

        if not self.ota_image:
            asserts.fail("Missing ota image path . Speficy using --string-arg ota_image:<ota_image>")

        if self.matter_test_config.timeout is None or self.matter_test_config.timeout <= 0:
            asserts.fail(
                "Test timeout parameter must be defined and  greater than 0. A good timeout can be 1800 seconds or 30 minutes [ --timeout 1800 ]")

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=10
        )
        await self.controller.CommissionOnNetwork(
            nodeId=self.provider_node_id,
            setupPinCode=self.provider_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.provider_discriminator
        )
        await self.set_default_ota_providers_list(controller=self.controller, provider_node_id=self.provider_node_id, endpoint=0, requestor_node_id=self.requestor_node_id)
        logger.info("About to write acl entries")
        await self.create_acl_entry(dev_ctrl=self.controller, provider_node_id=self.provider_node_id, requestor_node_id=self.requestor_node_id)

    @async_test_body
    async def teardown_test(self):
        await self.clear_ota_providers(self.controller, self.requestor_node_id)
        self.terminate_provider()
        self.clear_kvs(kvs_path_prefix=self.provider_kvs_path)
        super().teardown_test()

    def desc_TC_SU_2_7(self) -> str:
        return "[TC-SU-2.7] Verifying Events on OTA-R(DUT)"

    def pics_TC_SU_2_7(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_7(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "Perform a software update on the DUT."
                     "DUT successfully finishes applying a software update, and the new software image version is being executed on the DUT. OTA-Subscriber sends a read request to read the VersionApplied event from the DUT.",
                     "Verify that the OTA-Subscriber receives a StateTransition event notification for all the state changes i.e. Querying, Downloading, Applying, Idle (optional)."
                     "Verify that the VersionApplied event is generated whenever a new version starts executing after being applied due to a software update."),
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
        ]

    @async_test_body
    async def test_TC_SU_2_7(self):
        # Requestor is the DUT
        # Requestor has the flag --autoApply
        self.step(0)
        controller = self.default_controller
        # # Set the time interval based on the ota_image_download_timeout
        max_interval = self.ota_image_download_timeout + 60

        self.step(1)
        # Create event subscriber for StateTransition
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await update_state_attr_handler.start(dev_ctrl=controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=max_interval)
        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)
        # Register event, should change to Querying
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report {event_report}")
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kIdle,
                                           self.ota_req.Enums.UpdateStateEnum.kQuerying, expected_target_version=NullValue)

        # Event for Downloading
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report for Downloading {event_report}")
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                           self.ota_req.Enums.UpdateStateEnum.kDownloading, expected_target_version=self.expected_software_version)

        # Event for Applying
        event_report = state_transition_event_handler.wait_for_event_report(
            self.ota_req.Events.StateTransition, timeout_sec=self.ota_image_download_timeout)
        logger.info(f"Event report for Applying {event_report}")
        self.verify_state_transition_event(event_report, self.ota_req.Enums.UpdateStateEnum.kDownloading,
                                           self.ota_req.Enums.UpdateStateEnum.kApplying, expected_target_version=self.expected_software_version)
        state_transition_event_handler.cancel()
        Clusters.OtaSoftwareUpdateRequestor.Attributes.FeatureMap
        # kIdle ( After update is Optional) in this case we only wait state to be Idle
        # Just wait the device to be kIdle to avoid unexpected states in following Steps.
        update_state_match = AttributeMatcher.from_callable(
            "Update state is kIdle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=600)
        update_state_attr_handler.cancel()

        # Verify VersionAppliedEvent
        await self.verify_version_applied_basic_information(controller=controller, node_id=self.requestor_node_id, target_version=self.expected_software_version)
        # Once the Device is kIdle read the events list and we should expect the VersionApplied Event to be on the list.
        urgent = 1
        version_applied_event = Clusters.OtaSoftwareUpdateRequestor.Events.VersionApplied
        events_response = await self.controller.ReadEvent(
            self.requestor_node_id,
            events=[(0, version_applied_event, urgent)],
            fabricFiltered=True
        )
        logger.info(f"Events gathered {events_response}")
        # Only UpdateAppliedEvent should be in the list
        if len(events_response) == 0:
            asserts.fail("Failed to read events")
        version_applied_event_data = None
        # Reads for the VersionAppliedEvent
        for event in events_response:
            logger.info(event)
            if event.Header.EventId == self.ota_req.Events.VersionApplied.event_id:
                logger.info("Version AppliedEvent Found")
                version_applied_event_data = event.Data
        asserts.assert_is_not_none(version_applied_event, "Failed to read the VersionAppliedEvent")
        # Need to read the events and filter by VersionAppliedEvent
        asserts.assert_equal(self.expected_software_version, version_applied_event_data.softwareVersion,
                             f"Software version from VersionAppliedEvent is not {self.expected_software_version}")
        asserts.assert_is_not_none(version_applied_event_data.productID, "Product ID from VersionApplied Event is None")

        self.terminate_provider()
        self.restart_requestor(restore=True)

        self.step(2)
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port, extra_args=['--delayedQueryActionTimeSec', '10', '--queryImageStatus', 'busy'],
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=20
        )

        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*2)
        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=10)
        self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnQuery, expected_reason=self.ota_req.Enums.ChangeReasonEnum.kDelayByProvider)
        state_transition_event_handler.cancel()
        logger.info(f"About close the provider app with proc {self.current_provider_app_proc}")
        self.terminate_provider()

        self.step(3)
        self.start_provider(
            ota_image_path=self.ota_image,
            provider_app_path=self.provider_app_path,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=10
        )
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*3)
        # This step we need to Kill the provider PID before the announcement
        logger.info("Killing the provider process")
        self.current_provider_app_proc.kill()
        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60*2)
        logger.info(f"Event response after killing app: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event response : {event_report}")
        self.verify_state_transition_event(event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                           expected_new_state=self.ota_req.Enums.UpdateStateEnum.kIdle, expected_reason=self.ota_req.Enums.ChangeReasonEnum.kFailure)
        state_transition_event_handler.cancel()
        self.terminate_provider()
        self.restart_requestor()

        self.step(4)
        # This pics_guard should be replaced when we can read this value directly from the DUT.
        if self.pics_guard('MCORE.OTA.RequestorConsent'):
            self.start_provider(
                provider_app_path=self.provider_app_path,
                ota_image_path=self.ota_image,
                setup_pincode=self.provider_setup_pincode,
                discriminator=self.provider_discriminator,
                port=self.provider_port, extra_args=['-u', 'deferred', '-c'],
                kvs_path=self.provider_kvs_path,
                log_file=self.provider_log,
                timeout=10
            )
            state_transition_event_handler = EventSubscriptionHandler(
                expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
            await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=90)
            await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)
            event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
            self.verify_state_transition_event(event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
                                               expected_new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)
            event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
            self.verify_state_transition_event(event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
                                               expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnUserConsent)
            state_transition_event_handler.cancel()
            self.terminate_provider()
            self.restart_requestor()

        self.step(5)
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=20
        )
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=60*7)

        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)
        time_start = time()
        # Block waiting for Download
        logger.info("About to wait for StateTransition Events")
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report Querying {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kQuerying)
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report Downloading {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kDownloading)
        # Wait some time to let it download some data and then Kill the current process
        state_transition_event_handler.cancel()
        # Check for DownloadError
        error_download_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.DownloadError.event_id)
        await error_download_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=5000)
        logger.info("Wait 3 seconds to allow download some data")
        await asyncio.sleep(3)
        self.current_provider_app_proc.kill()
        time_middle = time()
        elapsed_time = time_middle - time_start
        logger.info(f"Elapsed time since announce {elapsed_time}")
        # wait to until the 5 minutes
        minimun_wait_time = (60*5)-elapsed_time
        logger.info(f"Script will wait for {minimun_wait_time} to match the minimum 5 minutes wait ")
        await asyncio.sleep(minimun_wait_time)
        logger.info("Completed waiting for 5 minutes")
        # After waiting 5 minutes the device must try trigger the DownloadError Event this may take a while.
        download_event_report = error_download_event_handler.wait_for_event_report(
            self.ota_req.Events.DownloadError, timeout_sec=600)
        logger.info(f"Download error Event: {download_event_report}")
        asserts.assert_equal(download_event_report.softwareVersion, self.expected_software_version,
                             f"Expected Software version {self.expected_software_version}, found {download_event_report.softwareVersion}")
        asserts.assert_greater(download_event_report.bytesDownloaded, 0, "Download was 0 bytes")
        asserts.assert_greater(download_event_report.progressPercent, 0, "Download progress was 0")
        asserts.assert_equal(download_event_report.platformCode, NullValue,
                             f"Null value not found at platformCode {download_event_report.platformCode}")
        error_download_event_handler.cancel()
        self.terminate_provider()

        self.step(6)
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port, extra_args=['--applyUpdateAction', 'awaitNextAction', '--delayedApplyActionTimeSec', '3'],
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=10
        )
        # EventHandlers and AttributeSubscriptionHandler
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await update_state_attr_handler.start(dev_ctrl=controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)
        # To avoid possible transiton between Querying and Idle use AttributeHandler
        update_state_match = AttributeMatcher.from_callable(
            "Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=max_interval)
        # Download complete then Applying
        event_report = state_transition_event_handler.wait_for_event_report(
            self.ota_req.Events.StateTransition, timeout_sec=self.ota_image_download_timeout)
        logger.info(f"Event report: {event_report}")
        asserts.assert_equal(event_report.newState, self.ota_req.Enums.UpdateStateEnum.kApplying)

        # Verification of the testStep DelayedOnApply
        event_report = state_transition_event_handler.wait_for_event_report(self.ota_req.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report: {event_report}")
        self.verify_state_transition_event(event_report=event_report, expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kApplying, expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDelayedOnApply,
                                           expected_reason=self.ota_req.Enums.ChangeReasonEnum.kDelayByProvider, expected_target_version=NullValue)
        update_state_attr_handler.cancel()
        state_transition_event_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
