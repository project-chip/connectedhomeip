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
#       --autoApplyImage
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_provider_port:5541
#       --timeout 2100
#     factory-reset: true
#     quiet: false
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_2(SoftwareUpdateBaseTest):

    def matcher_ota_updatestate(self, step_name, start_states, allowed_states, min_interval_sec, final_state=None):
        """
        Generic matcher for OTA UpdateState across multiple steps.

        Args:
            step_name (str): Name of the step, used in logging.
            start_states (list[int]): States that trigger the start of the interval timer.
            allowed_states (list[int]): States allowed during the interval. Any other state is considered unexpected.
            min_interval_sec (float): Minimum duration of the interval in seconds.
            final_state (int, optional): If provided, matcher waits for this state after interval completes.

        Returns:
            matcher_obj (AttributeMatcher): Matcher object to be used in event subscriptions.
            state_sequence (list): List of observed states during the interval.
            unexpected_states (set): Set of unexpected states observed during the interval.
            interval_duration (list): List containing the duration (in seconds) of the interval, or None if not completed.

        Notes:
            tolerance_sec (float): Used to allow for minor timing deviations when checking if the minimum interval has been reached.
        """

        seen_states = set()
        state_sequence = []
        unexpected_states = set()
        final_seen = False
        start_seen = False

        t_start_interval = None
        t_end_interval = None
        interval_duration = [None]
        tolerance_sec = 0.5

        logger.info(f'{step_name}: OTA matcher: start={start_states}, allowed={allowed_states}')

        def matcher(report):
            nonlocal final_seen, t_start_interval, t_end_interval, start_seen
            val = report.value
            if val is None:
                return False

            current_time = time.time()

            # Record state if new
            if val not in seen_states:
                state_sequence.append(val)
                seen_states.add(val)
                logger.info(f'{step_name}: State observed: {val} at {current_time}')

            # First start_state observed
            if val in start_states and not start_seen:
                start_seen = True
                t_start_interval = current_time
                logger.info(f'{step_name}: First start state recorded: {val}')
                logger.info(f'{step_name}: t_start_interval: {t_start_interval}')
                return False

            # Check unexpected states during interval
            if start_seen and t_start_interval is not None and t_end_interval is None:
                if current_time - t_start_interval < min_interval_sec - tolerance_sec:
                    if val not in allowed_states:
                        unexpected_states.add(val)
                        logger.info(f'{step_name}: Unexpected state during interval: {val}')

            # End interval after min time
            if start_seen and t_start_interval is not None and t_end_interval is None:
                if current_time - t_start_interval >= min_interval_sec + tolerance_sec:
                    t_end_interval = current_time
                    interval_duration[0] = t_end_interval - t_start_interval
                    logger.info(f'{step_name}: Interval completed after {min_interval_sec}s')
                    logger.info(f'{step_name}: t_end_interval: {t_end_interval}')
                    logger.info(f'{step_name}: interval_duration: {interval_duration}')

                    return final_state is None

            # Final state check
            if final_state and val == final_state and t_end_interval is not None:
                final_seen = True
                if val not in seen_states:
                    state_sequence.append(val)
                    seen_states.add(val)
                logger.info(f'{step_name}: Final state {val} observed, matcher ending')
                return True

            return False

        matcher_obj = AttributeMatcher.from_callable(
            description=f"{step_name} - Match OTA UpdateState, start={start_states}, allowed={allowed_states}, final={final_state}",
            matcher=matcher
        )

        return matcher_obj, state_sequence, unexpected_states, interval_duration

    def desc_TC_SU_2_2(self) -> str:
        return "[TC-SU-2.2] Handling Different QueryImageResponse Scenarios on Requestor"

    def pics_TC_SU_2_2(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_2(self) -> list[TestStep]:
        # Steps are executed in this order: 0, 2, 3, 4, 7, 5, 1, 6.
        #
        # Steps 2, 3, 7 do not trigger an OTA image transfer — the provider is killed immediately
        # after each verification. Step 4 triggers a download after the 180s delay but the
        # provider is killed right after confirming kDownloading (download aborted, no apply).
        # Step 1 is the single step where the full OTA update is allowed to complete (DUT
        # upgrades to V2). Step 6 runs last so the same V2 image is served as a "same version"
        # update — the DUT has just applied V2 and will reject it. This means only one firmware
        # image (V2) is needed for the entire test.
        return [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'Busy', DelayedActionTime is set to 60 seconds.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'NotAvailable'.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            TestStep(4, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to Busy, Set DelayedActionTime to 3 minutes. On the subsequent QueryImage command, "
                     "TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'.",
                     "Verify that the DUT waits for at least the time mentioned in the DelayedActionTime (3 minutes) before issuing another QueryImage command to the TH/OTA-P. "
                     "Verify that there is a transfer of the software image after the second QueryImageResponse with UpdateAvailable status from the TH/OTA-P to the DUT."),
            TestStep(7, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable', ImageURI field contains an invalid BDX ImageURI.",
                     "Verify that the DUT does not start transferring the software image."),
            TestStep(5, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
                     "ImageURI should have the https url from where the image can be downloaded.",
                     "Verify that the DUT queries the https url and downloads the software image."),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "Set ImageURI to the location where the image is located.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT."),
            TestStep(6, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
                     "Software Version is set to the same version the DUT just applied (V2), which is numerically equal to the current version.",
                     "Verify that the DUT does not start transferring the software image."),
        ]

    @async_test_body
    async def teardown_test(self):
        # Clear provider KVS (self.KVS_PATH) after test to avoid interference with subsequent tests
        self.clear_kvs(kvs_path_prefix=self.KVS_PATH)
        self.terminate_provider()
        super().teardown_test()

    @async_test_body
    async def test_TC_SU_2_2(self):
        self.LOG_FILE_PATH = "provider.log"
        self.KVS_PATH = "/tmp/chip_kvs_provider"
        self.provider_app_path = self.user_params.get('provider_app_path')
        self.ota_image = self.user_params.get('ota_image')

        # Validate provider_app_path
        if not self.provider_app_path or not os.path.exists(self.provider_app_path):
            raise FileNotFoundError(f'Invalid provider_app_path: {self.provider_app_path}.')

        # Validate ota_image
        if not self.ota_image or not os.path.exists(self.ota_image):
            raise FileNotFoundError(f'Invalid ota_image: {self.ota_image}.')

        self.step(0)
        # Controller has already commissioned the requestor

        # Prerequisite #1.0 - Requestor (DUT) info
        controller = self.default_controller
        fabric_id = controller.fabricId
        requestor_node_id = self.dut_node_id

        # Prerequisite #1.0 - Provider info
        provider_node_id = self.dut_node_id + 1
        provider_discriminator = 1111
        provider_setup_pincode = 20202021
        provider_port = self.user_params.get('ota_provider_port', 5541)

        # Pre-define all provider arg sets for reuse across steps
        provider_extra_args_updateAvailable = [
            "-q", "updateAvailable"
        ]

        # ------------------------------------------------------------------------------------
        # Provider commissioning (done once before any step that requires AnnounceOTAProvider).
        # Start the provider for the first time with Busy/60s args (used in Step 2) so that
        # commissioning can happen before Step 2 runs.
        # ------------------------------------------------------------------------------------
        provider_extra_args_busy = [
            "-q", "busy",
            "-t", "60"
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            extra_args=provider_extra_args_busy,
        )

        # Commission Provider (only once — subsequent restarts reuse the same node ID)
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'Provider commissioning response: {resp}')

        await self.create_acl_entry(
            dev_ctrl=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id
        )

        self.step(2)
        # ------------------------------------------------------------------------------------
        # [STEP_2]: Provider already started above with busy/60s args.
        # ------------------------------------------------------------------------------------
        step_number_s2 = "[STEP_2]"
        logger.info(f'{step_number_s2}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Attributes: UpdateState (Busy sequence)
        # ------------------------------------------------------------------------------------
        subscription_attr_state_busy = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_state_busy.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0.5,
            max_interval_sec=0.5,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.0 - Controller sends AnnounceOTAProvider command')
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info(f'{step_number_s2}: Step #2.0 - sent cmd AnnounceOTAProvider.')

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.2 - Track OTA attributes: UpdateState (Busy sequence)
        # Allowed states during 120s interval: Idle, DelayedOnQuery, Querying.
        # Any unexpected states during the 120s interval are asserted.
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s2}: Step #2.1 - Started subscription for UpdateState attribute (Busy sequence). '
            'Waiting for the 120s minimum interval. This step may take several minutes to complete.')

        matcher_busy_state_obj, state_sequence_busy, observed_states_during_interval, interval_duration_ref = self.matcher_ota_updatestate(
            step_name=step_number_s2,
            start_states=[
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery,
            ],
            allowed_states=[
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
            ],
            min_interval_sec=120,
            final_state=None
        )

        subscription_attr_state_busy.await_all_expected_report_matches([matcher_busy_state_obj], timeout_sec=920.0)
        logger.info(f'{step_number_s2}: Step #2.3 - UpdateState (Busy sequence) matcher has completed.')
        subscription_attr_state_busy.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.4 - Verify Busy sequence
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.4 - Full OTA UpdateState (Busy sequence) observed: {state_sequence_busy}')

        interval_duration_busy = interval_duration_ref[0]

        if interval_duration_busy is None:
            asserts.fail(f"Interval did not complete for Busy sequence {interval_duration_busy}.")

        logger.info(f"Interval duration: {interval_duration_busy:.2f}s")
        logger.info(f'{step_number_s2}: Step #2.4 - 120s interval: {interval_duration_busy:.2f}s, '
                    f'unexpected states: {list(observed_states_during_interval)}')

        expected_start = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery

        asserts.assert_true(expected_start in state_sequence_busy,
                            f"Expected start state {expected_start} not found in observed sequence: {state_sequence_busy}")
        asserts.assert_true(interval_duration_busy >= 120, f"Expected interval >= 120s, observed: {interval_duration_busy:.2f}s")
        asserts.assert_equal(list(observed_states_during_interval), [],
                             f"Unexpected states: {list(observed_states_during_interval)}")

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.5 - Closed Provider process.')
        self.current_provider_app_proc.terminate()

        self.step(3)
        # ------------------------------------------------------------------------------------
        # [STEP_3]: Prerequisites - Setup Provider
        # ------------------------------------------------------------------------------------
        step_number_s3 = "[STEP_3]"
        logger.info(f'{step_number_s3}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args_updateNotAvailable = [
            "-q", "updateNotAvailable",
            "-t", "60"
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            extra_args=provider_extra_args_updateNotAvailable,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Attributes: UpdateState (updateNotAvailable sequence)
        # ------------------------------------------------------------------------------------
        subscription_attr_state_updatenotavailable = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_state_updatenotavailable.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0.5,
            max_interval_sec=0.5,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.0 - Controller sends AnnounceOTAProvider command')
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info(f'{step_number_s3}: Step #3.0 - sent cmd AnnounceOTAProvider.')

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.2 - Track OTA attributes: UpdateState (updateNotAvailable sequence)
        # Allowed states during 120s interval: Idle, Querying.
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s3}: Step #3.1 - Started subscription for UpdateState attribute '
            '(updateNotAvailable sequence). '
            'Waiting for the 120s minimum interval. This step may take several minutes to complete.')

        matcher_not_available_state_obj, state_sequence_notavailable, observed_states_during_interval, interval_duration_ref = self.matcher_ota_updatestate(
            step_name=step_number_s3,
            start_states=[
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
            ],
            allowed_states=[
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
            ],
            min_interval_sec=120,
            final_state=None
        )

        subscription_attr_state_updatenotavailable.await_all_expected_report_matches(
            [matcher_not_available_state_obj], timeout_sec=920.0)
        logger.info(f'{step_number_s3}: Step #3.3 - UpdateState (updateNotAvailable sequence) matcher has completed.')
        subscription_attr_state_updatenotavailable.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.4 - Verify updateNotAvailable sequence
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.4 - Full OTA UpdateState (updateNotAvailable sequence) observed: {state_sequence_notavailable}')

        interval_duration_notavailable = interval_duration_ref[0]

        if interval_duration_notavailable is None:
            asserts.fail(f"Interval did not complete for updateNotAvailable sequence {interval_duration_notavailable}.")

        logger.info(f'{step_number_s3}: Step #3.4 - 120s interval: {interval_duration_notavailable:.2f}s, '
                    f'unexpected states: {list(observed_states_during_interval)}')

        expected_start = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying

        asserts.assert_true(expected_start in state_sequence_notavailable,
                            f"Expected start state {expected_start} not found in observed sequence: {state_sequence_notavailable}")
        asserts.assert_true(interval_duration_notavailable >= 120,
                            f"Expected interval >= 120s, observed: {interval_duration_notavailable:.2f}s")
        asserts.assert_equal(list(observed_states_during_interval), [],
                             f"Unexpected states: {list(observed_states_during_interval)}")

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.5 - Closed Provider process.')
        self.current_provider_app_proc.terminate()

        self.step(4)
        # ------------------------------------------------------------------------------------
        # [STEP_4]: Prerequisites - Setup Provider
        # The provider is started with busy/180s args. The provider is killed immediately after
        # confirming kDownloading so the download is aborted and no full OTA update is applied
        # in this step. The full OTA update happens in Step 1.
        # ------------------------------------------------------------------------------------
        step_number_s4 = "[STEP_4]"
        logger.info(f'{step_number_s4}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args_busy_180 = [
            "-q", "busy",
            "-t", "180"
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            extra_args=provider_extra_args_busy_180,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Attributes: UpdateState (Busy, 180s DelayedActionTime)
        # ------------------------------------------------------------------------------------
        subscription_attr_state_busy_180s = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_state_busy_180s.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0.5,
            max_interval_sec=0.5,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s4}: Step #4.0 - Controller sends AnnounceOTAProvider command')
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info(f'{step_number_s4}: Step #4.0 - sent cmd AnnounceOTAProvider.')

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.2 - Track OTA attributes: UpdateState (Busy, 180s DelayedActionTime sequence)
        # Allowed states during 180s interval: Idle, DelayedOnQuery, Querying.
        # After the interval, kDownloading is verified as the final state.
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s4}: Step #4.1 - Started subscription for UpdateState attribute '
            '(Busy, 180s DelayedActionTime sequence). '
            'Waiting for the device to start downloading the image. This step may take several minutes to complete.')

        matcher_busy_state_delayed_180s_obj, state_sequence_busy_180, observed_states_during_interval, interval_duration_ref = self.matcher_ota_updatestate(
            step_name=step_number_s4,
            start_states=[
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery
            ],
            allowed_states=[
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
            ],
            min_interval_sec=180,
            final_state=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        )

        subscription_attr_state_busy_180s.await_all_expected_report_matches(
            [matcher_busy_state_delayed_180s_obj], timeout_sec=980.0)
        logger.info(f'{step_number_s4}: Step #4.3 - UpdateState Busy > Downloading transition (180s) successfully observed.')
        subscription_attr_state_busy_180s.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.4 - Verify Busy, 180s DelayedActionTime sequence
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s4}: Step #4.4 - Full OTA UpdateState (Busy, 180s DelayedActionTime sequence) observed: {state_sequence_busy_180}')

        interval_duration_busy_180 = interval_duration_ref[0]
        logger.info(f'{step_number_s4}: Step #4.4 - 180s interval: {interval_duration_busy_180:.2f}s, '
                    f'unexpected states: {list(observed_states_during_interval)}')

        expected_start_state = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery
        expected_final_state = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading

        asserts.assert_true(expected_start_state in state_sequence_busy_180,
                            f"Expected start state {expected_start_state} not found in observed sequence: {state_sequence_busy_180}")
        asserts.assert_true(expected_final_state in state_sequence_busy_180,
                            f"Expected final state {expected_final_state} not found in observed sequence: {state_sequence_busy_180}")
        asserts.assert_true(interval_duration_busy_180 >= 180,
                            f"Expected interval >= 180s, observed: {interval_duration_busy_180:.2f}s")
        asserts.assert_equal(list(observed_states_during_interval), [],
                             f"Unexpected states: {list(observed_states_during_interval)}")

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.5 - Close Provider Process
        # Kill immediately after download start is confirmed so the download is aborted.
        # The single full OTA update is reserved for Step 1.
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s4}: Step #4.5 - Close Provider Process')
        self.current_provider_app_proc.terminate()

        self.step(7)
        # ------------------------------------------------------------------------------------
        # [STEP_7]: Prerequisites - Setup Provider
        # ------------------------------------------------------------------------------------
        step_number_s7 = "[STEP_7]"
        logger.info(f'{step_number_s7}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args_invalid_bdx = [
            "-i", "bdx://000000000000000X"
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            extra_args=provider_extra_args_invalid_bdx,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.1 - Matcher for OTA event logs
        # Start EventSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Events: StateTransition (should stay Idle due to invalid BDX ImageURI in UpdateAvailable)
        # ------------------------------------------------------------------------------------
        subscription_state_invalid_uri = EventSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id
        )

        await subscription_state_invalid_uri.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1
        )

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s7}: Step #7.0 - Controller sends AnnounceOTAProvider command')
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info(f'{step_number_s7}: Step #7.0 - sent cmd AnnounceOTAProvider.')

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.2 - Track OTA StateTransition event: should stay Idle due to invalid BDX ImageURI.
        #     First event: Idle > Querying
        #     Second event: Querying > Idle
        # ------------------------------------------------------------------------------------

        # Transition 1: Idle > Querying
        event1 = subscription_state_invalid_uri.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition,
            timeout_sec=620
        )
        logger.info(f"{step_number_s7}: Event 1: {event1}")

        self.verify_state_transition_event(
            event1,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        )

        # Transition 2: Querying > Idle
        event2 = subscription_state_invalid_uri.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition,
            timeout_sec=30
        )
        logger.info(f"{step_number_s7}: Event 2: {event2}")

        self.verify_state_transition_event(
            event2,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        )

        subscription_state_invalid_uri.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.4 - Verify NO image transfer occurs due to invalid BDX ImageURI.
        # ------------------------------------------------------------------------------------
        logger.info(f"{step_number_s7}: No image transfer occurred due to invalid BDX URI (expected).")

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s7}: Step #7.5 - Closed Provider.')
        self.current_provider_app_proc.terminate()

        self.step(5)
        # Step #5 - HTTPS image download
        # NOTE: Step skipped (provisional / not implemented in spec)

        self.step(1)
        # ------------------------------------------------------------------------------------
        # [STEP_1]: Prerequisites - Setup Provider
        # The provider is started with updateAvailable args. The provider is kept running until
        # kApplying is observed (BDX transfer complete), then killed. This is the single
        # full OTA update in the entire test — the DUT applies V2 and reboots.
        # ------------------------------------------------------------------------------------
        step_number = "[STEP_1]"
        logger.info(f'{step_number}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')
        logger.info(f'{step_number}: Prerequisite #1.0 - Launched Provider')

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            extra_args=provider_extra_args_updateAvailable,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Attributes: UpdateState and UpdateStateProgress (updateAvailable sequence)
        # ------------------------------------------------------------------------------------
        subscription_attr = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None  # receive all attributes
        )

        await subscription_attr.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=20,
            keepSubscriptions=True
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number}: Step #1.0 - Controller sends AnnounceOTAProvider command')
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info(f'{step_number}: Step #1.0 - sent cmd AnnounceOTAProvider.')

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.2 - Track OTA attributes: UpdateState and UpdateStateProgress
        #   - UpdateState must reach kDownloading
        #   - UpdateStateProgress must have at least one value in range 1-100
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number}: Step #1.1 - Started subscription for UpdateState and UpdateStateProgress attributes. '
            'Waiting for the device to start downloading the image. This step may take several minutes to complete.')

        state_sequence = []
        progress_values = []
        downloading_seen = False
        progress_seen = False

        def matcher_combined(report):
            """
            Combined matcher for Step 1:
            - Validates UpdateState reaches kDownloading
            - UpdateStateProgress has any value 1-100
            """
            nonlocal state_sequence, progress_values, downloading_seen, progress_seen
            val = getattr(report.value, "value", report.value)

            current_time = time.time()

            # UpdateState
            if report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState:
                if val is not None and val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading:
                    if not downloading_seen:
                        downloading_seen = True
                        state_sequence.append(Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
                        logger.info(f'{step_number}: State observed: {val} at {current_time}')

            # UpdateStateProgress
            elif report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                if val is not None and isinstance(val, int) and 1 <= val <= 100:
                    if not progress_seen:
                        progress_seen = True
                        progress_values.append(val)
                        logger.info(f'{step_number}: Progress observed: {val} at {current_time}')

            return downloading_seen and progress_seen

        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number} - Minimal Step 1 matcher: Downloading + progress 1-100",
            matcher=matcher_combined
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.3 - Wait for download to start
        # ------------------------------------------------------------------------------------
        subscription_attr.await_all_expected_report_matches([matcher_combined_obj], timeout_sec=800.0)
        logger.info(f'{step_number}: Step #1.3 - UpdateState (Available sequence) matcher has completed.')
        subscription_attr.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.4 - Verify image transfer from TH/OTA-P to DUT is successfully started.
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number}: Step #1.4 - Full OTA state sequence observed: {state_sequence}')
        logger.info(f'{step_number}: Step #1.4 - Progress values observed: {progress_values}')

        expected_flows = [
            [Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading]
        ]

        if state_sequence in expected_flows:
            logger.info(f'{step_number}: Step #1.4 - OTA flow is valid: {state_sequence}')
        else:
            msg = f"Observed OTA flow: {state_sequence}, Expected one of: {expected_flows}"
            asserts.fail(msg)

        asserts.assert_true(any(1 <= v <= 100 for v in progress_values),
                            f"{step_number}: No valid UpdateStateProgress observed (1-100)")
        logger.info(f'{step_number}: Step #1.4 - UpdateStateProgress has valid value(s) in range 1-100')

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.5 - Wait for kApplying to confirm the BDX transfer is fully
        # complete, then kill the provider. The DUT will finish applying and reboot on its own.
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number}: Step #1.5 - Waiting for kApplying to confirm download complete.')

        subscription_attr_applying = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_applying.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=20,
            keepSubscriptions=True
        )

        def matcher_applying(report):
            val = report.value
            return val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        matcher_applying_obj = AttributeMatcher.from_callable(
            description=f"{step_number} - Wait for kApplying",
            matcher=matcher_applying
        )

        subscription_attr_applying.await_all_expected_report_matches([matcher_applying_obj], timeout_sec=800.0)
        logger.info(f'{step_number}: Step #1.5 - kApplying observed — BDX transfer complete.')
        subscription_attr_applying.cancel()

        logger.info(f'{step_number}: Step #1.5 - Killing provider (download done, DUT applying firmware).')
        self.current_provider_app_proc.terminate()

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.6 - Wait for DUT to reboot after applying V2 firmware.
        # Expire the stale session so the controller reconnects cleanly, then poll until
        # GetConnectedDevice succeeds (DUT is back online).
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number}: Step #1.6 - Expiring stale session and waiting for DUT to reboot.')
        controller.ExpireSessions(requestor_node_id)

        reboot_timeout_sec = 120
        poll_interval_sec = 5
        reconnected = False
        for attempt in range(reboot_timeout_sec // poll_interval_sec):
            await asyncio.sleep(poll_interval_sec)
            try:
                await controller.GetConnectedDevice(requestor_node_id, allowPASE=False)
                reconnected = True
                logger.info(f'{step_number}: Step #1.6 - DUT reconnected after OTA reboot (attempt {attempt + 1}).')
                break
            except Exception:
                logger.info(f'{step_number}: Step #1.6 - Waiting for DUT to come back online (attempt {attempt + 1}/{reboot_timeout_sec // poll_interval_sec})...')

        asserts.assert_true(reconnected, f'{step_number}: DUT did not come back online within {reboot_timeout_sec}s after OTA reboot.')

        self.step(6)
        # ------------------------------------------------------------------------------------
        # [STEP_6]: Prerequisites - Setup Provider
        # The DUT has just applied the V2 firmware in Step 1. By serving the same V2 image here
        # with updateAvailable, the DUT sees it as "same version" and rejects the download.
        # No separate firmware image is needed — the single V2 image (ota_image) is reused,
        # meaning only one firmware image is required for the entire test.
        # ------------------------------------------------------------------------------------
        step_number_s6 = "[STEP_6]"
        logger.info(f'{step_number_s6}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            extra_args=provider_extra_args_updateAvailable,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.1 - Matcher for OTA event logs
        # Start EventSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Events: StateTransition (UpdateState should stay Idle because the offered version equals
        # the version the DUT just installed)
        # ------------------------------------------------------------------------------------
        subscription_state_no_download = EventSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id
        )

        await subscription_state_no_download.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1
        )

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s6}: Step #6.0 - Controller sends AnnounceOTAProvider command')
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info(f'{step_number_s6}: Step #6.0 - sent cmd AnnounceOTAProvider.')

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.2 - Track OTA StateTransition events
        #     First event: Idle > Querying
        #     Second event: Querying > Idle
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s6}: Step #6.1 - Waiting for StateTransition events '
            '(DUT should reject the image because the offered version equals the installed version)')

        # Transition 1: Idle > Querying
        event1 = subscription_state_no_download.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition,
            timeout_sec=30
        )
        logger.info(f"{step_number_s6}: Event 1: {event1}")

        self.verify_state_transition_event(
            event1,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        )

        # Transition 2: Querying > Idle
        event2 = subscription_state_no_download.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition,
            timeout_sec=30
        )
        logger.info(f"{step_number_s6}: Event 2: {event2}")

        self.verify_state_transition_event(
            event2,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        )

        subscription_state_no_download.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.4 - Verify NO image transfer occurs (same version already installed)
        # ------------------------------------------------------------------------------------
        logger.info(f"{step_number_s6}: No image transfer occurred (expected - DUT already on V2).")


if __name__ == "__main__":
    default_matter_test_main()
