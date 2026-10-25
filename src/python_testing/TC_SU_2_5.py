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
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --string-arg provider_app_pipe_out:/tmp/provider_app_pipe_out_2_5
#       --string-arg provider_app_pipe:/tmp/provider_app_pipe_2_5
#       --int-arg ota_provider_port:5541
#       --timeout 2100
#     factory-reset: true
#     app-ready-pattern: Server initialization complete
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Nominal execution reserve kept aside for each not-yet-started step when handing the
# remaining test budget to a wait, so early steps cannot starve later ones.
STEP_RESERVE_SEC = 60
# Log line every OTA provider prints on receiving a QueryImage, whatever it then answers: see
# OtaProviderLogic::QueryImage in src/app/clusters/ota-provider/OTAProviderCluster.cpp, which
# logs it before dispatching to the application delegate. Steps 2 and 3 arm this match and
# _announce_until_provider_queried polls the provider stdout: it is the only way to tell a
# query this provider answered from one that died in a CASE session left over from a previous
# provider process, since both leave the DUT in the same kQuerying→kIdle transition.
PROVIDER_QUERY_RECEIVED_LOG = "OTA Provider received QueryImage"


class TC_SU_2_5(MatterTestCommissionedDevice, SoftwareUpdateBaseTest):
    "This test case verifies that the DUT behaves according to the spec when it is applying the software update."
    provider_kvs_path = None
    provider_log = None
    current_requestor_app_pid = None
    ota_prov = Clusters.OtaSoftwareUpdateProvider
    ota_req = Clusters.OtaSoftwareUpdateRequestor
    controller = None
    provider_node_id = 321
    provider_discriminator = 321
    provider_setup_pincode = 2321
    requestor_node_id = None
    disable_wildcard_subscription = True

    @property
    def default_timeout(self) -> int:
        # Used only when no --timeout is passed on the command line (CI passes
        # --timeout 2100 via the test-runner args above). Real devices have unbounded
        # download/apply times, so default to a generous budget: every long wait is
        # event-driven and ends early on fast devices, so an oversized budget only
        # costs time when something is genuinely wrong.
        return 7200

    @async_test_body
    async def teardown_test(self):
        await self.clear_ota_providers(self.controller, self.requestor_node_id)
        self.terminate_provider()
        self.clear_kvs(kvs_path_prefix=self.provider_kvs_path)
        super().teardown_test()

    @async_test_body
    async def setup_test(self):

        # Start budget
        self.start_test_budget_clock()

        # Set up the Provider configuration and values for Step 1.
        self.ota_image = self.user_params.get('ota_image')

        self.provider_app_path = self.user_params.get('provider_app_path')
        self.ota_provider_port = self.user_params.get('ota_provider_port', 5541)
        self.provider_kvs_path = self.user_params.get('provider_kvs_path', '/tmp/chip_kvs_provider')
        self.provider_log = self.user_params.get('provider_log_path', '/tmp/provider_log_2_5.log')
        self.provider_app_pipe_out = self.user_params.get('provider_app_pipe_out', '/tmp/provider_app_pipe_out_2_5')
        self.provider_app_pipe = self.user_params.get('provider_app_pipe', '/tmp/provider_app_pipe_2_5')
        if not self.provider_kvs_path.startswith('/tmp'):
            asserts.fail("Provider KVS path must be placed in the /tmp directory.")

        if not self.provider_app_path:
            asserts.fail("Missing provider app path. Specify using --string-arg provider_app_path:<provider_app_path>")

        if not self.ota_image:
            asserts.fail("Missing ota image path. Specify using --string-arg ota_image:<ota_image>")

        self.requestor_node_id = self.dut_node_id  # 123 with discriminator 123
        self.controller = self.default_controller

        # Named-pipe output arguments.
        self.provider_pipe_arguments = ['--app-pipe-out', self.provider_app_pipe_out, '--app-pipe', self.provider_app_pipe]
        logger.info("PROVIDER PIPE OUT %s, PROVIDER PIPE %s", self.provider_app_pipe_out, self.provider_app_pipe)
        # Extra arguments required for Step 1.
        delayed_apply_action_time_s1 = 60
        extra_arguments = ['--applyUpdateAction', 'awaitNextAction',
                           '--delayedApplyActionTimeSec', str(delayed_apply_action_time_s1)] + self.provider_pipe_arguments

        # Check the OTA image and running software version. This fails if the image is unsuitable for updating the device;
        # otherwise, it returns the target version.
        self.expected_software_version = await self.check_ota_image_version(
            controller=self.controller, requestor_node_id=self.requestor_node_id, ota_image_path=self.ota_image)

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.ota_provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            extra_args=extra_arguments,
        )
        logger.info("About to start commissioning")
        await self.controller.CommissionOnNetwork(
            nodeId=self.provider_node_id,
            setupPinCode=self.provider_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.provider_discriminator
        )
        logger.info("Creating ACL entries")
        await self.create_acl_entry(dev_ctrl=self.controller,
                                    provider_node_id=self.provider_node_id, requestor_node_id=self.requestor_node_id)
        logger.info("Writing OTA Providers")
        await self.set_default_ota_providers_list(controller=self.controller, provider_node_id=self.provider_node_id, requestor_node_id=self.requestor_node_id, endpoint=0)
        super().setup_test()

    def desc_TC_SU_2_5(self) -> str:
        return " [TC-SU-2.5] Handling Different ApplyUpdateResponse Scenarios on Requestor"

    def pics_TC_SU_2_5(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_5(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done"),
            TestStep(1, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\", DelayedActionTime is set to 1 minute. Terminate the TH/OTA-P after the DUT changes the UpdateState to kDelayedOnApply.", "Verify that the DUT waits for the minimum interval defined by spec which is 2 minutes before re-sending the ApplyUpdateRequest to the OTA-P and the DUT UpdateState changes to kApplying."
                     "Verify that the DUT does not apply the software update within this time."),
            TestStep(2, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\", DelayedActionTime is set to 3 minutes. Terminate the TH/OTA-P after the DUT changes the UpdateState to kDelayedOnApply.", "Verify that the DUT waits for 3 minutes before sending the ApplyUpdateRequest to the OTA-P and the DUT UpdateState changes to kApplying."
                     "Verify that the DUT does not apply the software update within this time."),
            TestStep(3, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"Discontinue\".", "Verify that the DUT clears its previously downloaded software image, and resets the UpdateState Attribute to Idle."
                     "Verify that the DUT does not send the NotifyUpdateApplied within a reasonable time."
                     "Verify the SoftwareVersion attribute from the Basic Information cluster of the DUT to be the same as it was previously."),
        ]

    async def _wait_for_idle_after_software_update(self, update_state_handler):
        # Wait for the DUT to return to Idle after the Provider is canceled.

        update_state_match = AttributeMatcher.from_callable(
            "UpdateState is Idle",
            lambda report: report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState and report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_handler.await_all_expected_report_matches([update_state_match], timeout_sec=600)
        update_state_handler.cancel()

    @async_test_body
    async def test_TC_SU_2_5(self):

        # Commissioning is already complete.
        self.step(0)

        self.step(1)
        # In this step, the DUT should download the full OTA image without applying the update.
        # The transitions are as follows: kIdle -> kQuerying -> kDownloading.
        # After downloading the full OTA image, the DUT enters kDelayedOnApply and remains there for 120 seconds.
        # The Provider process is terminated once the DUT enters kDelayedOnApply.
        # The DUT must not install the software update during those 120 seconds.
        # The DUT must not apply the update because the Provider was terminated and cannot send another ApplyUpdateResponse.
        # After the assertions, wait for the DUT to return to kIdle before starting the next step.

        tolerance_delay_sec = 3
        step_number_s1 = "[STEP 1] "
        spec_wait_time = 120
        # Read the DUT's current software version, which should remain unchanged throughout the test execution.
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=self.controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=self.requestor_node_id)

        subscription_attr_cluster = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None  # Receive all attributes.
        )

        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

        await self._start_subscription_bounded(
            subscription_attr_cluster, step_number_s1,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        await self._start_subscription_bounded(
            software_version_attr_handler, step_number_s1,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        # Announce the provider
        logger.info('%s: Step #1.0 - Controller sends AnnounceOTAProvider command', step_number_s1)
        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)

        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after AnnounceOTAProvider: %s", step_number_s1, pipe_data)

        downloading_seen = False
        progress_seen = False
        step_number_matcher = step_number_s1

        def matcher_combined(report):
            """
            Combined matcher for Step 1 and 2:

            - Validates that UpdateState reaches kDownloading.
            - Validates that UpdateStateProgress has a value from 1 to 100.
            """
            nonlocal downloading_seen, progress_seen
            val = getattr(report.value, "value", report.value)
            current_time = time.time()
            # UpdateState
            if report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState:
                if val is not None and val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading:
                    if not downloading_seen:
                        downloading_seen = True
                        logger.info('%s: State observed: %s at %s', step_number_matcher, val, current_time)
            # UpdateStateProgress
            elif report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                # logger.info("Report matcher_combined %d progress", report.value)
                if val is not None and isinstance(val, int) and 1 <= val <= 100:
                    if not progress_seen:
                        progress_seen = True
                        logger.info('%s: Progress observed: %s at %s', step_number_matcher, val, current_time)
            return downloading_seen and progress_seen

        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s1} - Step 1 matcher: Waiting for the DUT to start the download",
            matcher=matcher_combined
        )
        # Wait for the download to start.
        logger.info('%s: Step #1.0 - Waiting for the OTA download to start', step_number_s1)
        subscription_attr_cluster.await_all_expected_report_matches(
            [matcher_combined_obj],
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))

        logger.info('%s: Step #1.0 - OTA Download in progress waiting for kDelayedOnApply', step_number_s1)
        # Create a liveness matcher to wait for the kDelayedOnApply
        progress_liveness_last_progress = [None]

        def progress_liveness(report):
            if report.attribute != Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                return False
            val = getattr(report.value, "value", report.value)
            if val is None or val == progress_liveness_last_progress[0]:
                return False
            progress_liveness_last_progress[0] = val
            return True

        # Wait for the device to reach kDelayedOnApply (configured value: 60 seconds; specification value: 120 seconds).
        time_for_kDelayed_apply_s1 = subscription_attr_cluster.await_first_value_asserting_no_forbidden(
            target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
            forbidden_values=set(),
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=STEP_RESERVE_SEC),
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
            liveness_matcher=progress_liveness
        )
        logger.info('%s: Step #1.0 - kDelayedOnApply observed after the download completed', step_number_s1)
        await asyncio.sleep(0.1)
        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after kDelayedOnApply %s", step_number_s1, pipe_data)
        # Assert the values from the Provider named pipes
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestActionResponse'],
                             Clusters.OtaSoftwareUpdateProvider.Enums.ApplyUpdateActionEnum.kAwaitNextAction, "Error on TH: the action from the Provider is not AwaitNextAction")
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestCount'],
                             1, "Error on TH: Only one request should be sent from the Provider to DUT (Requestor).")

        # The Provider must be terminated just before it tries to send the second ApplyUpdateRequest.
        logger.info("%s Terminating the Provider to prevent the device from completing the update", step_number_s1)
        self.terminate_provider()

        # Read the last StateTransitionEvent at this point of the test and make sure newState is kDelayedOnApply
        # During the period of 120 seconds the device must not change to kApplying or update the DUT SoftwareVersion
        state_transition_event_obj = Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition
        state_transition_s1_delayed_before = await self.controller.ReadEvent(
            self.requestor_node_id,
            events=[(0, state_transition_event_obj, 1)],
            fabricFiltered=True,
        )
        # Read the last event that should contain the kDelayedOnApply
        kdelayed_on_apply_s1 = state_transition_s1_delayed_before[-1]
        asserts.assert_equal(kdelayed_on_apply_s1.Data.newState,
                             Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply, "Event newState is not KDelayedOnApply")
        logger.info("%s Latest StateTransitionEvent %s with event number %d", step_number_s1,
                    kdelayed_on_apply_s1, kdelayed_on_apply_s1.Header.EventNumber)
        # Save the time from the kDelayedOnApply Event after confirming that newState is kDelayedOnApply
        event_s1_kDelayed_timestamp = kdelayed_on_apply_s1.Header.Timestamp

        # The device should remain in UpdateState: kDelayedOnApply for 120 seconds and must NOT Apply the software update after
        # the 60-second delayedApplyActionTimeSec interval.
        software_version_match = AttributeMatcher.from_callable(
            f"Software Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        # Guard the software version attribute
        logger.info("%s Starting software-version monitoring for 120 seconds", step_number_s1)
        software_version_attr_handler.wait_all_final_values_reported_persisted(
            expected_matchers=[software_version_match], timeout_sec=spec_wait_time)
        logger.info("%s Completed software-version monitoring with no changes for 120 seconds", step_number_s1)

        # Read the events immediately.
        # After this period of time we want to verify if kApplying was triggered after the 120-second timeframe.
        # Even if the StateTransitionEvent is triggered we should compare it with the Timestamp to verify it was really triggered after the 120 seconds.

        state_transition_s1_delayed_after = await self.read_cluster_events(state_transition_event_obj, kdelayed_on_apply_s1.Header.EventNumber)
        software_version_attr_handler.reset()
        software_version_attr_handler.cancel()

        # Read the filtered events list. There must be at least one event.
        # Verify this condition by comparing the Data.newState and then comparing the Header.Timestamp to make sure the transition is correct and is after the timeframe of 120 seconds.
        wait_for_kapply = True
        if len(state_transition_s1_delayed_after) >= 1:
            # Find the event which has kApplying from the subsequent events
            event_s1_kApply = None
            for event in state_transition_s1_delayed_after:
                if event.Data.newState == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying:
                    # Looking for StateTransitionEvent with kApplying
                    event_s1_kApply = event
                    logger.info("%s : Event found for kApplying %s", step_number_s1, event_s1_kApply)

            if event_s1_kApply is None:
                logger.warning("%s No StateTransitionEvents found with kApplying", step_number_s1)
                asserts.fail(f"{step_number_s1} Gathered events do not contain a kApplying transition {state_transition_s1_delayed_after}.")

            # Verify the transition is correct kDelayedOnApply to kApplying
            asserts.assert_equal(event_s1_kApply.Data.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
                                 f"Event does not have the expected newState kApplying {event_s1_kApply.Data} ")
            asserts.assert_equal(event_s1_kApply.Data.previousState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
                                 f"Event does not have the expected previousState kDelayedOnApply {event_s1_kApply.Data} ")

            # Verify the event sequence
            s1_diff_sequence = event_s1_kApply.Header.EventNumber - kdelayed_on_apply_s1.Header.EventNumber
            asserts.assert_equal(s1_diff_sequence, 1,
                                 "More StateTransitionEvents found than expected from kDelatedOnApply to kApplying")

            # Compare the times to check if the difference is over 120 seconds.
            total_time_from_events_s1 = (event_s1_kApply.Header.Timestamp - event_s1_kDelayed_timestamp)/1000
            logger.info("%s Total time taken from kDelayedOnApply to kApplying %d , adding a tolerance of 3 seconds",
                        step_number_s1, total_time_from_events_s1)
            # Some devices might have some delays, this add some tolerance for this possible issues.
            total_time_from_events_s1 += tolerance_delay_sec

            asserts.assert_greater_equal(total_time_from_events_s1, spec_wait_time,
                                         "Time taken between kDelayedOnApply and kApplying is lower than 120 seconds")
            logger.info("%s State transition from kDelayedOnApply to kApplying detected, time taken for this transition is %.1f seconds",
                        step_number_s1, total_time_from_events_s1)
            logger.info("%s Event found for kApplying after waiting 120 seconds", step_number_s1)
            wait_for_kapply = False

        # At this point it is always over 120 seconds.
        if wait_for_kapply:
            logger.info("%s Did not find the kApplying at UpdateState event waiting for kApplying UpdateState", step_number_s1)
            # If the StateTransition to kApply is not yet visible wait DUT to reach the state
            time_for_kApplying_s1 = subscription_attr_cluster.await_first_value_asserting_no_forbidden(
                target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
                forbidden_values={Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
                                  Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading},
                timeout_sec=STEP_RESERVE_SEC,
                expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
            )

            time_taken_from_delay_to_apply_s1 = time_for_kApplying_s1 - time_for_kDelayed_apply_s1
            logger.info("%s Time taken from kDelayedOnApply to kApplying: %s seconds",
                        step_number_s1, time_taken_from_delay_to_apply_s1)

        logger.info("%s Waiting for the device to return to kIdle after kApplying because the Provider was terminated.", step_number_s1)
        # The Requestor does not receive the second ApplyUpdateResponse because the Provider is terminated before resending it.
        await self._wait_for_idle_after_software_update(update_state_handler=subscription_attr_cluster)
        subscription_attr_cluster.reset()
        subscription_attr_cluster.cancel()

        # Verify that the software version remains unchanged because the second ApplyUpdateResponse was not sent.
        new_software_version_s1 = await self.verify_version_applied_basic_information(controller=self.controller, node_id=self.requestor_node_id, target_version=current_sw_version)
        asserts.assert_equal(new_software_version_s1, current_sw_version,
                             f"Software versions must match Start: {current_sw_version}, End:{new_software_version_s1}")
        logger.info("%s Software version is unchanged and the DUT has returned to kIdle. Start version: %d; end version: %d",
                    step_number_s1, current_sw_version, new_software_version_s1)

        self.step(2)
        step_number_s2 = "[STEP 2] "
        delayed_apply_action_time_s2 = 180
        extra_arguments = ['--applyUpdateAction', 'awaitNextAction',
                           '--delayedApplyActionTimeSec', str(delayed_apply_action_time_s2)] + self.provider_pipe_arguments
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.ota_provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            extra_args=extra_arguments,
        )

        self.current_provider_app_proc.arm_output_match(PROVIDER_QUERY_RECEIVED_LOG)

        # Software Version attribute handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )
        #  OTA Requestor attribute handler
        subscription_attr_cluster = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None
        )

        await self._start_subscription_bounded(
            subscription_attr_cluster, step_number_s2,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        await self._start_subscription_bounded(
            software_version_attr_handler, step_number_s2,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        # Actions before announcing the Provider.
        await self._wait_until_idle_before_announce(
            controller=self.controller,
            requestor_node_id=self.requestor_node_id,
            subscription=subscription_attr_cluster,
            # timeout_sec=IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC,
            timeout_sec=120,
            step_name=step_number_s2,
        )

        await self._announce_until_provider_queried(
            controller=self.controller,
            provider_node_id=self.provider_node_id,
            requestor_node_id=self.requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC),
            step_name=step_number_s2,
        )

        # Reset the variables used by matcher_combined in Step 1 before reusing it in Step 2.
        downloading_seen = False
        progress_seen = False
        step_number_matcher = step_number_s2
        progress_liveness_last_progress = [None]

        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s2} - Step 2 matcher: Waiting for the DUT to see the download",
            matcher=matcher_combined
        )

        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after AnnounceOTAProvider: %s", step_number_s2, pipe_data)

        # Wait for the DUT to start the download and see progress
        logger.info('%s: Step #2.0 - Waiting for the OTA Download to start', step_number_s2)
        subscription_attr_cluster.await_all_expected_report_matches(
            [matcher_combined_obj],
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))

        logger.info("%s Waiting for the DelayedApplyAction interval of %s seconds.", step_number_s2, delayed_apply_action_time_s2)

        # Wait for the device to reach kDelayedOnApply (configured DelayedActionTime: 180 seconds).
        time_for_kdelayed_apply_s2 = subscription_attr_cluster.await_first_value_asserting_no_forbidden(
            target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
            forbidden_values=set(),
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=STEP_RESERVE_SEC),
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
            liveness_matcher=progress_liveness
        )
        # Avoid race condition
        await asyncio.sleep(0.1)

        # Read the named pipe
        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after kDelayedOnApply %s", step_number_s2, pipe_data)
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestActionResponse'],
                             Clusters.OtaSoftwareUpdateProvider.Enums.ApplyUpdateActionEnum.kAwaitNextAction, "Action from the provider is not AwaitNextAction")
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestCount'],
                             1, "Only one request should be sent from the Provider")

        # The Provider must be terminated just before it tries to send the second ApplyUpdateRequest.
        logger.info("%s Terminating the Provider to prevent the device from completing the update", step_number_s2)
        self.terminate_provider()

        # Read the last StateTransitionEvent at this point of the test and make sure newState is kDelayedOnApply
        # During the period of 180 seconds the device must not change to KApplying or update the software version of the DUT
        state_transition_s2_delayed_before = await self.controller.ReadEvent(
            self.requestor_node_id,
            events=[(0, state_transition_event_obj, 1)],
            fabricFiltered=True,
        )
        # Read the last event that should contain the kDelayedOnApply
        state_transition_kdelayedonapply_s2 = state_transition_s2_delayed_before[-1]
        asserts.assert_equal(state_transition_kdelayedonapply_s2.Data.newState,
                             Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply, "Event newState is not KDelayedOnApply")
        # After confirming the kDelayedOnApply event, save its timestamp for comparison
        event_s2_kDelayed_timestamp = state_transition_kdelayedonapply_s2.Header.Timestamp
        logger.info("%s Latest StateTransitionEvent %s with event number %d", step_number_s2,
                    state_transition_kdelayedonapply_s2, state_transition_kdelayedonapply_s2.Header.EventNumber)

        # The device should remain in kDelayedOnApply and must not apply the software update during this 180-second interval.
        software_version_match = AttributeMatcher.from_callable(
            f"Software Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        software_version_attr_handler.wait_all_final_values_reported_persisted(
            expected_matchers=[software_version_match], timeout_sec=delayed_apply_action_time_s2)

        # Read the events immediately.
        # After this period of time we want to verify if kApplying was triggered within the 180-second timeframe.
        # Even if the StateTransitionEvent is triggered we should compare it with the Timestamp to verify it was really triggered after the 180 seconds.
        state_transition_s2_delayed_after = await self.read_cluster_events(state_transition_event_obj, state_transition_kdelayedonapply_s2.Header.EventNumber)
        # Read the filtered events list. There must be at least one event.
        # Verify this condition by comparing the Data.newState and then comparing the Header.Timestamp to make sure the transition is correct and was done after 180 seconds.
        wait_for_kapply = True
        if len(state_transition_s2_delayed_after) >= 1:
            # Find the event which has kApplying from the subsequent events
            event_s2_kApply = None
            for event in state_transition_s2_delayed_after:
                if event.Data.newState == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying:
                    # Looking for StateTransitionEvent with kApplying
                    event_s2_kApply = event
                    logger.info("%s : Event found for kApplying %s", step_number_s2, event_s2_kApply)

            if event_s2_kApply is None:
                logger.warning("%s No StateTransitionEvents found with kApplying", step_number_s2)
                asserts.fail(f"{step_number_s2} Gathered events do not contain a kApplying transition. {state_transition_s2_delayed_after}")

            # Compare the event states
            asserts.assert_equal(event_s2_kApply.Data.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
                                 f"Event does not have the expected newState kApplying {event_s2_kApply.Data} ")
            asserts.assert_equal(event_s2_kApply.Data.previousState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
                                 f"Event does not have the expected previousState kDelayedOnApply {event_s2_kApply.Data} ")

            # Verify the event sequence
            s2_diff_sequence = event_s2_kApply.Header.EventNumber - state_transition_kdelayedonapply_s2.Header.EventNumber
            asserts.assert_equal(s2_diff_sequence, 1,
                                 "More StateTransitionEvents found that expected from kDelatedOnApply to kApplying")

            # Compare the times to check if the difference is over 180 seconds.
            total_time_from_events_s2 = (event_s2_kApply.Header.Timestamp - event_s2_kDelayed_timestamp)/1000
            logger.info("%s Total time taken from kDelayedOnApply to kApplying %d , adding a tolerance of 3 seconds",
                        step_number_s2, total_time_from_events_s2)
            # Some devices might have some delays, this add some tolerance for this possible issues.
            total_time_from_events_s2 += tolerance_delay_sec
            asserts.assert_greater_equal(total_time_from_events_s2, delayed_apply_action_time_s2,
                                         "Time taken between kDelayedOnApply and kApplying is lower than 180 seconds")
            logger.info("%s State transition from kDelayedOnApply to kApplying detected, time taken for this transition is %.1f seconds",
                        step_number_s2, total_time_from_events_s2)
            wait_for_kapply = False

        # If the StateTransition to kApply is not yet visible wait DUT to reach the state
        if wait_for_kapply:
            logger.info("%s Did not find the kApplying at UpdateState event waiting for kApplying UpdateState", step_number_s2)
            time_to_kapplying_s2 = subscription_attr_cluster.await_first_value_asserting_no_forbidden(
                target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
                forbidden_values={Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
                                  Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading},
                timeout_sec=STEP_RESERVE_SEC,
                expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
            )
            time_taken_from_delay_to_apply_s2 = time_to_kapplying_s2 - time_for_kdelayed_apply_s2

            logger.info("%s Time taken from kDelayedOnApply to kApplying: %s seconds",
                        step_number_s2, time_taken_from_delay_to_apply_s2)
            asserts.assert_greater_equal(time_taken_from_delay_to_apply_s2, delayed_apply_action_time_s2,
                                         "Time for kApplying is lower than 180 seconds")
        software_version_attr_handler.cancel()

        # The Requestor does not receive the second ApplyUpdateResponse because the Provider is terminated before resending it.
        await self._wait_for_idle_after_software_update(update_state_handler=subscription_attr_cluster)
        subscription_attr_cluster.reset()
        subscription_attr_cluster.cancel()

        # Verify that the software version remains unchanged because the second ApplyUpdateResponse was not sent.
        new_software_version_s2 = await self.verify_version_applied_basic_information(controller=self.controller, node_id=self.requestor_node_id, target_version=current_sw_version)
        asserts.assert_equal(new_software_version_s2, current_sw_version,
                             f"Software versions must match Start: {current_sw_version}, End:{new_software_version_s2}")
        logger.info("%s Software version is unchanged and the DUT has returned to kIdle. Start version: %d; end version: %d",
                    step_number_s2, current_sw_version, new_software_version_s2)

        self.step(3)
        step_number_s3 = "[STEP 3]"
        extra_arguments = ['--applyUpdateAction', 'discontinue'] + self.provider_pipe_arguments

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.ota_provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            extra_args=extra_arguments,
        )

        self.current_provider_app_proc.arm_output_match(PROVIDER_QUERY_RECEIVED_LOG)

        # Software Version attribute handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )
        #  OTA Requestor attribute handler
        subscription_attr_cluster = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None
        )

        await self._start_subscription_bounded(
            subscription_attr_cluster, step_number_s3,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        await self._start_subscription_bounded(
            software_version_attr_handler, step_number_s3,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        # Actions before announcing the Provider.
        await self._wait_until_idle_before_announce(
            controller=self.controller,
            requestor_node_id=self.requestor_node_id,
            subscription=subscription_attr_cluster,
            # timeout_sec=IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC,
            timeout_sec=120,
            step_name=step_number_s3,
        )

        await self._announce_until_provider_queried(
            controller=self.controller,
            provider_node_id=self.provider_node_id,
            requestor_node_id=self.requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC),
            step_name=step_number_s3,
        )

        # Wait for the DUT to start downloading.
        logger.info("%s Waiting for the DUT to reach the kDownloading state.", step_number_s3)
        update_state_match = AttributeMatcher.from_callable(
            "Waiting UpdateState is Downloading",
            lambda report: report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState and report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        subscription_attr_cluster.await_all_expected_report_matches([update_state_match], timeout_sec=600)

        # This can only be tested in CI or locally because real devices might not expose this path.
        if self.is_pics_sdk_ci_only:
            logger.info("%s Verifying OTA image information.", step_number_s3)
            # The state is kDownloading; let it run for a few seconds to produce data to check.
            await asyncio.sleep(3)
            # Verify the default download path and file size.
            # The file /tmp/test.bin should exist and be larger than 0 bytes.
            ota_file_data = self.get_downloaded_ota_image_info()
            logger.info("Downloaded ota image data %s", ota_file_data)
            asserts.assert_equal(True, ota_file_data['exists'], f"File was not downloaded  at {ota_file_data['path']}")
            asserts.assert_greater(ota_file_data['size'], 0, "Downloaded file is still at 0")

        # Device is downloading the image
        progress_seen = False
        download_completed = False

        def check_ota_download_matcher(report):
            """Check UpdateStateProgress and confirm that the image has finished downloading when the
                value reaches NullValue.
            Args:
                report : Report value
            """
            nonlocal progress_seen, download_completed
            attribute = report.attribute
            value = report.value
            # UpdateStateProgress
            if attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                if value is not None and isinstance(value, int) and 1 <= value <= 100:
                    if not progress_seen:
                        progress_seen = True
                if progress_seen and value == NullValue:
                    download_completed = True
            return download_completed

        logger.info("%s Waiting for the OTA download to complete.", step_number_s3)
        download_progress_attr_matcher_obj = AttributeMatcher.from_callable(
            description="Waiting Download to Complete ", matcher=check_ota_download_matcher)
        subscription_attr_cluster.await_all_expected_report_matches(
            [download_progress_attr_matcher_obj], timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))

        logger.info("%s OTA download completed; reading the ApplyUpdateRequestActionResponse configured on the Provider", step_number_s3)
        # Use named pipes to confirm the ApplyUpdateAction
        await asyncio.sleep(0.1)
        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after kDownload and kDiscontinue enabled %s", step_number_s3, pipe_data)
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestActionResponse'],
                             Clusters.OtaSoftwareUpdateProvider.Enums.ApplyUpdateActionEnum.kDiscontinue, "Action from the provider is not kDiscontinue")
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestCount'],
                             1, "Only one request should be sent from the Provider")

        # The DUT does not apply the update and returns to kIdle because the action is set to Discontinue.
        logger.info("%s Waiting for the DUT to return to kIdle because ApplyUpdateAction is Discontinue", step_number_s3)
        update_state_match = AttributeMatcher.from_callable(
            "Waiting UpdateState is Idle",
            lambda report: report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState and report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        subscription_attr_cluster.await_all_expected_report_matches(
            [update_state_match], self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))

        subscription_attr_cluster.cancel()

        if self.is_pics_sdk_ci_only:
            ota_file_data = self.get_downloaded_ota_image_info()
            logger.info("Downloaded ota image data %s", ota_file_data)
            asserts.assert_equal(ota_file_data['exists'], False, f"Downloaded file is still present {ota_file_data['path']}")
            asserts.assert_equal(ota_file_data['size'], 0, "File size is greater than 0")

        update_state_progress = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress, self.controller, self.requestor_node_id, 0)
        asserts.assert_equal(update_state_progress, NullValue, "Progress is not Null")
        logger.info("%s Current UpdateStateProgress is %s, as expected", step_number_s3, update_state_progress)
        # Verify that the software version is unchanged from the start of the test.
        new_software_version_s3 = await self.verify_version_applied_basic_information(self.controller, self.requestor_node_id, current_sw_version)
        asserts.assert_equal(new_software_version_s3, current_sw_version,
                             f"Software versions must match Start: {current_sw_version}, End:{new_software_version_s3}")
        logger.info("%s Software version is unchanged and the DUT has returned to kIdle. Start version: %d; end version: %d",
                    step_number_s3, current_sw_version, new_software_version_s3)


if __name__ == "__main__":
    default_matter_test_main()
