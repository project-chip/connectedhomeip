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
import queue
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

        logger.info('%s: OTA matcher: start=%s, allowed=%s', step_name, start_states, allowed_states)

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
                logger.info('%s: State observed: %s at %s', step_name, val, current_time)

            # First start_state observed
            if val in start_states and not start_seen:
                start_seen = True
                t_start_interval = current_time
                logger.info('%s: First start state recorded: %s', step_name, val)
                logger.info('%s: t_start_interval: %s', step_name, t_start_interval)
                return False

            # Check unexpected states during interval
            if start_seen and t_start_interval is not None and t_end_interval is None:
                if current_time - t_start_interval < min_interval_sec - tolerance_sec:
                    if val not in allowed_states:
                        unexpected_states.add(val)
                        logger.info('%s: Unexpected state during interval: %s', step_name, val)

            # End interval after min time
            if start_seen and t_start_interval is not None and t_end_interval is None:
                if current_time - t_start_interval >= min_interval_sec + tolerance_sec:
                    t_end_interval = current_time
                    interval_duration[0] = t_end_interval - t_start_interval
                    logger.info('%s: Interval completed after %ss', step_name, min_interval_sec)
                    logger.info('%s: t_end_interval: %s', step_name, t_end_interval)
                    logger.info('%s: interval_duration: %s', step_name, interval_duration)

                    return final_state is None

            # Final state check
            if final_state and val == final_state and t_end_interval is not None:
                final_seen = True
                if val not in seen_states:
                    state_sequence.append(val)
                    seen_states.add(val)
                logger.info('%s: Final state %s observed, matcher ending', step_name, val)
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
        # Steps are executed in order: 0, 1, 2, 3, 4, 5, 6.
        #
        # Steps 1, 2, 4 do not trigger an OTA image transfer — the provider is killed immediately
        # after each verification. Step 3 triggers a download after the 180s delay but the
        # provider is killed right after confirming kDownloading (download aborted, no apply).
        # Step 5 is the single step where the full OTA update is allowed to complete (DUT
        # upgrades to V2). Step 6 runs last so the same V2 image is served as a "same version"
        # update — the DUT has just applied V2 and will reject it. This means only one firmware
        # image (V2) is needed for the entire test.
        return [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'Busy', DelayedActionTime is set to 60 seconds.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'NotAvailable'.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to Busy, Set DelayedActionTime to 3 minutes. On the subsequent QueryImage command, "
                     "TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'.",
                     "Verify that the DUT waits for at least the time mentioned in the DelayedActionTime (3 minutes) before issuing another QueryImage command to the TH/OTA-P. "
                     "Verify that the transfer of the software image has been initiated after the second QueryImageResponse with UpdateAvailable status from the TH/OTA-P to the DUT."
                     "Cancel the transfer after confirming it has started to avoid applying the update in this step (the single full update is reserved for Step 5)."),
            TestStep(4, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable', ImageURI field contains an invalid BDX ImageURI.",
                     "Verify that the DUT does not start transferring the software image."),
            TestStep(5, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
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
        logger.info('Provider commissioning response: %s', resp)

        await self.create_acl_entry(
            dev_ctrl=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id
        )

        self.step(1)
        # ------------------------------------------------------------------------------------
        # [STEP_1]: Provider already started above with busy/60s args.
        #
        # Two-phase queue-based verification replaces the polling-matcher approach:
        #
        #   Phase A — wait for kDelayedOnQuery to confirm the DUT received the Busy/60s
        #             response.  kDownloading before kDelayedOnQuery is an immediate fail.
        #
        #   Phase B — guard a 120s window using await_duration_asserting_no_forbidden().
        #             That method evaluates each report exactly once at dequeue time, so there
        #             is no polling-loop re-evaluation drift that allowed kDownloading to slip
        #             through the 119.5 s–120.5 s dead zone in the old matcher_ota_updatestate
        #             approach.  tolerance_sec=1.0 shrinks the strict window to 119s so states
        #             arriving within the last second of the nominal interval are not flagged.
        #             kDownloading that arrives after the 119s window (i.e. after the DUT
        #             correctly observed the minimum delay) does not cause a failure.
        # ------------------------------------------------------------------------------------
        step_number_s1 = "[STEP_1]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s1, requestor_node_id, fabric_id)

        kDelayedOnQuery_s1 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery
        kDownloading_s1 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kApplying_s1 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        subscription_attr_state_busy = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_state_busy.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #1.0 - Controller sends AnnounceOTAProvider command', step_number_s1)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #1.0 - sent cmd AnnounceOTAProvider.', step_number_s1)

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Phase A — wait for kDelayedOnQuery (DUT received Busy/60s)
        # ------------------------------------------------------------------------------------
        logger.info('%s: Phase A — waiting for kDelayedOnQuery (DUT received Busy/60s from provider)', step_number_s1)
        t_delayed_on_query_s1 = subscription_attr_state_busy.await_first_value_asserting_no_forbidden(
            target_value=kDelayedOnQuery_s1,
            forbidden_values={kDownloading_s1, kApplying_s1},
            timeout_sec=120,
        )
        logger.info('%s: Phase A complete — kDelayedOnQuery at %.2f', step_number_s1, t_delayed_on_query_s1)

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Phase B — 120s guard window; kDownloading/kApplying are forbidden.
        # ------------------------------------------------------------------------------------
        tolerance_s1_sec = 1.0
        logger.info(
            '%s: Phase B — guarding 120s minimum interval (tolerance %ss). kDownloading/kApplying forbidden.', step_number_s1, tolerance_s1_sec)

        subscription_attr_state_busy.await_duration_asserting_no_forbidden(
            duration_sec=120,
            forbidden_values={kDownloading_s1, kApplying_s1},
            tolerance_sec=tolerance_s1_sec,
        )

        elapsed_s1 = time.time() - t_delayed_on_query_s1
        logger.info('%s: Phase B complete — elapsed since kDelayedOnQuery: %.2fs', step_number_s1, elapsed_s1)

        asserts.assert_true(
            elapsed_s1 >= 120,
            f"{step_number_s1}: Elapsed since kDelayedOnQuery was {elapsed_s1:.2f}s, expected >= 120s")

        subscription_attr_state_busy.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #1.5 - Closed Provider process.', step_number_s1)
        self.current_provider_app_proc.terminate()

        self.step(2)
        # ------------------------------------------------------------------------------------
        # [STEP_2]: Prerequisites - Setup Provider
        # ------------------------------------------------------------------------------------
        step_number_s2 = "[STEP_2]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s2, requestor_node_id, fabric_id)

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

        subscription_attr_state_updatenotavailable = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_state_updatenotavailable.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False
        )

        logger.info('%s: Step #2.0 - Controller sends AnnounceOTAProvider command', step_number_s2)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #2.0 - sent cmd AnnounceOTAProvider.', step_number_s2)

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Phase A — wait for kQuerying (DUT sent its first QueryImage).
        # kDownloading/kApplying before kQuerying is an immediate fail.
        # ------------------------------------------------------------------------------------
        kQuerying_s2 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        kDownloading_s2 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kApplying_s2 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        logger.info('%s: Phase A — waiting for kQuerying (first QueryImage sent by DUT)', step_number_s2)
        t_querying_s2 = subscription_attr_state_updatenotavailable.await_first_value_asserting_no_forbidden(
            target_value=kQuerying_s2,
            forbidden_values={kDownloading_s2, kApplying_s2},
            timeout_sec=120,
        )
        logger.info('%s: Phase A complete — kQuerying at %.2f, 120s guard window starts', step_number_s2, t_querying_s2)

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Phase B — 120s guard window; kDownloading/kApplying are forbidden.
        # ------------------------------------------------------------------------------------
        tolerance_s2 = 2.0
        min_interval_s2 = 120

        logger.info(
            '%s: Phase B — guarding %ss minimum interval (tolerance %ss). kDownloading/kApplying forbidden.', step_number_s2, min_interval_s2, tolerance_s2)

        subscription_attr_state_updatenotavailable.await_duration_asserting_no_forbidden(
            duration_sec=min_interval_s2,
            forbidden_values={kDownloading_s2, kApplying_s2},
            tolerance_sec=tolerance_s2,
        )

        elapsed_s2 = time.time() - t_querying_s2
        logger.info('%s: Phase B complete — elapsed since kQuerying: %.2fs', step_number_s2, elapsed_s2)

        asserts.assert_true(
            elapsed_s2 >= min_interval_s2,
            f"{step_number_s2}: Elapsed since kQuerying was {elapsed_s2:.2f}s, expected >= {min_interval_s2}s")

        subscription_attr_state_updatenotavailable.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #2.5 - Closed Provider process.', step_number_s2)
        self.current_provider_app_proc.terminate()

        self.step(3)
        # ------------------------------------------------------------------------------------
        # [STEP_3]: Prerequisites - Setup Provider
        # The provider is started with busy/180s args. The provider is killed immediately after
        # confirming kDownloading so the download is aborted and no full OTA update is applied
        # in this step. The full OTA update happens in Step 5.
        # ------------------------------------------------------------------------------------
        step_number_s3 = "[STEP_3]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s3, requestor_node_id, fabric_id)

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

        subscription_attr_state_busy_180s = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_attr_state_busy_180s.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False
        )

        logger.info('%s: Step #3.0 - Controller sends AnnounceOTAProvider command', step_number_s3)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #3.0 - sent cmd AnnounceOTAProvider.', step_number_s3)

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Phase A — wait for kDelayedOnQuery (DUT received Busy/180s).
        # The provider auto-switches to UpdateAvailable after one Busy response.
        # kDownloading before kDelayedOnQuery is an immediate fail.
        # ------------------------------------------------------------------------------------
        kDelayedOnQuery_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery
        kDownloading_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kApplying_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        logger.info('%s: Phase A — waiting for kDelayedOnQuery (DUT received Busy/180s)', step_number_s3)
        t_delayed_on_query_s3 = subscription_attr_state_busy_180s.await_first_value_asserting_no_forbidden(
            target_value=kDelayedOnQuery_s3,
            forbidden_values={kDownloading_s3, kApplying_s3},
            timeout_sec=120,
        )
        logger.info('%s: Phase A complete — kDelayedOnQuery at %.2f, 180s guard window starts',
                    step_number_s3, t_delayed_on_query_s3)

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Phase B — 180s guard window; kDownloading/kApplying are forbidden.
        # After the window elapses, the provider has auto-switched to UpdateAvailable and the
        # DUT's next query will trigger a download.
        # ------------------------------------------------------------------------------------
        tolerance_s3 = 5.0
        min_interval_s3 = 180

        logger.info(
            '%s: Phase B — guarding %ss DelayedActionTime (tolerance %ss). kDownloading/kApplying forbidden. This will take ~3 minutes.', step_number_s3, min_interval_s3, tolerance_s3)

        subscription_attr_state_busy_180s.await_duration_asserting_no_forbidden(
            duration_sec=min_interval_s3,
            forbidden_values={kDownloading_s3, kApplying_s3},
            tolerance_sec=tolerance_s3,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Phase C — wait for kDownloading to confirm the DUT re-queried after 180s.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Phase C — waiting for kDownloading (DUT re-queried after 180s delay)', step_number_s3)
        t_downloading_s3 = subscription_attr_state_busy_180s.await_first_value_asserting_no_forbidden(
            target_value=kDownloading_s3,
            forbidden_values=set(),
            timeout_sec=120,
        )

        elapsed_s3 = t_downloading_s3 - t_delayed_on_query_s3
        logger.info('%s: Phase C complete — elapsed kDelayedOnQuery → kDownloading: %.2fs', step_number_s3, elapsed_s3)

        asserts.assert_true(
            elapsed_s3 >= min_interval_s3 - tolerance_s3,
            f"{step_number_s3}: DUT re-queried too soon. "
            f"Elapsed: {elapsed_s3:.2f}s, expected >= {min_interval_s3 - tolerance_s3}s.")

        subscription_attr_state_busy_180s.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.6 - Close Provider Process
        # Kill immediately after download start is confirmed so the download is aborted.
        # The single full OTA update is reserved for Step 5.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #3.6 - Close Provider Process (aborting download)', step_number_s3)
        self.current_provider_app_proc.terminate()

        # kIdle wait removed: when the provider is killed mid-BDX the DUT can take many
        # minutes to recover (BDX timeout + retry backoff). Step 5 handles any stale
        # StateTransition events by filtering them in a loop (see Option B comments there).

        self.step(4)
        # ------------------------------------------------------------------------------------
        # [STEP_4]: Prerequisites - Setup Provider
        # ------------------------------------------------------------------------------------
        step_number_s4 = "[STEP_4]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s4, requestor_node_id, fabric_id)

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
        # [STEP_4]: Step #4.1 - Matcher for OTA event logs
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
            min_interval_sec=0,
            max_interval_sec=30
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.2 - Track OTA StateTransition events: Idle→Querying→Idle.
        #
        # Stale-event handling: Step 3 killed the provider during an active BDX download.
        # The DUT may take a long time to recover (BDX timeout + retry backoff), emitting
        # stale StateTransition events (kDownloading→kIdle, kIdle→kQuerying from retries,
        # etc.) that arrive in this subscription's queue before the Step 4 events.
        # Flush buffered events accumulated before the announce so only post-announce
        # events are considered, then loop discarding any remaining stale transitions and
        # re-sending AnnounceOTAProvider every 60 s so the DUT queries as soon as it recovers.
        # ------------------------------------------------------------------------------------
        subscription_state_invalid_uri.flush_events()

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #4.0 - Controller sends AnnounceOTAProvider command', step_number_s4)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #4.0 - sent cmd AnnounceOTAProvider.', step_number_s4)

        kIdle = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        kQuerying = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        kDownloading_s4 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kApplying_s4 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying
        s4_forbidden_states = {kDownloading_s4, kApplying_s4}

        # --- Transition 1: Idle → Querying ---
        event1 = None
        s4_timeout = 900  # 15 min: accommodates DUT recovering from a stuck BDX transfer
        t_s4_start = time.time()

        while time.time() - t_s4_start < s4_timeout:
            remaining = s4_timeout - (time.time() - t_s4_start)
            try:
                raw = subscription_state_invalid_uri.get_event_from_queue(block=True, timeout=min(60.0, remaining))
            except queue.Empty:
                # No event for 60 s — DUT may have missed AnnounceOTAProvider while busy.
                # Re-send so the DUT queries as soon as it returns to kIdle.
                logger.info("%s: No event in 60s, re-sending AnnounceOTAProvider (elapsed: %.0fs / %ss)",
                            step_number_s4, time.time() - t_s4_start, s4_timeout)
                await self.announce_ota_provider(
                    controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
                continue

            if raw.Header.EventId != Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id:
                continue

            evt = raw.Data
            if evt.previousState == kIdle and evt.newState == kQuerying:
                event1 = evt
                logger.info("%s: Event 1 (Idle→Querying): %s", step_number_s4, event1)
                break

            if evt.newState in s4_forbidden_states:
                asserts.fail(f"{step_number_s4}: DUT entered {evt.newState} — "
                             "image transfer started despite invalid BDX ImageURI!")
            logger.info("%s: Discarding stale event: %s → %s", step_number_s4, evt.previousState, evt.newState)

        asserts.assert_true(event1 is not None,
                            f"{step_number_s4}: Idle→Querying transition not found within {s4_timeout}s")

        # --- Transition 2: Querying → Idle ---
        # Apply the same stale-event filtering as Transition 1 — earlier steps may have left
        # residual events in the queue (e.g. kDownloading→kIdle from an aborted BDX session).
        event2 = None
        s4_t2_timeout = 120
        t_s4_t2_start = time.time()

        while time.time() - t_s4_t2_start < s4_t2_timeout:
            remaining = s4_t2_timeout - (time.time() - t_s4_t2_start)
            try:
                raw = subscription_state_invalid_uri.get_event_from_queue(block=True, timeout=min(30.0, remaining))
            except queue.Empty:
                continue

            if raw.Header.EventId != Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id:
                continue

            evt2 = raw.Data
            if evt2.previousState == kQuerying and evt2.newState == kIdle:
                event2 = evt2
                logger.info("%s: Event 2 (Querying→Idle): %s", step_number_s4, event2)
                break

            if evt2.newState in s4_forbidden_states:
                asserts.fail(f"{step_number_s4}: DUT entered {evt2.newState} — "
                             "image transfer started despite invalid BDX ImageURI!")
            logger.info("%s: Discarding stale event (transition 2): %s → %s", step_number_s4, evt2.previousState, evt2.newState)

        asserts.assert_true(event2 is not None,
                            f"{step_number_s4}: Querying→Idle transition not found within {s4_t2_timeout}s")

        subscription_state_invalid_uri.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.4 - Verify NO image transfer occurs due to invalid BDX ImageURI.
        # ------------------------------------------------------------------------------------
        logger.info("%s: No image transfer occurred due to invalid BDX URI (expected).", step_number_s4)

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #4.5 - Closed Provider.', step_number_s4)
        self.current_provider_app_proc.terminate()

        self.step(5)
        # ------------------------------------------------------------------------------------
        # [STEP_5]: Prerequisites - Setup Provider
        # The provider is started with updateAvailable args. The provider is kept running until
        # kApplying is observed (BDX transfer complete), then killed. This is the single
        # full OTA update in the entire test — the DUT applies V2 and reboots.
        # ------------------------------------------------------------------------------------
        step_number_s5 = "[STEP_5]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s5, requestor_node_id, fabric_id)
        logger.info('%s: Prerequisite #1.0 - Launched Provider', step_number_s5)

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
        # [STEP_5]: Step #5.1 - Matcher for OTA records logs
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
        # [STEP_5]: Step #5.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #5.0 - Controller sends AnnounceOTAProvider command', step_number_s5)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #5.0 - sent cmd AnnounceOTAProvider.', step_number_s5)

        # ------------------------------------------------------------------------------------
        # [STEP_5]: Step #5.2 - Track OTA attributes: UpdateState and UpdateStateProgress
        #   - UpdateState must reach kDownloading
        #   - UpdateStateProgress must have at least one value in range 1-100
        # ------------------------------------------------------------------------------------
        logger.info(
            '%s: Step #5.2 - Started subscription for UpdateState and UpdateStateProgress attributes. Waiting for the device to start downloading the image. This step may take several minutes to complete.', step_number_s5)

        state_sequence = []
        progress_values = []
        downloading_seen = False
        progress_seen = False

        def matcher_combined(report):
            """
            Combined matcher for Step 5:

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
                        logger.info('%s: State observed: %s at %s', step_number_s5, val, current_time)

            # UpdateStateProgress
            elif report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                if val is not None and isinstance(val, int) and 1 <= val <= 100:
                    if not progress_seen:
                        progress_seen = True
                        progress_values.append(val)
                        logger.info('%s: Progress observed: %s at %s', step_number_s5, val, current_time)

            return downloading_seen and progress_seen

        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s5} - Step 5 matcher: Downloading + progress 1-100",
            matcher=matcher_combined
        )

        # ------------------------------------------------------------------------------------
        # [STEP_5]: Step #5.3 - Wait for download to start
        # ------------------------------------------------------------------------------------
        # Start the kApplying subscription before waiting for kDownloading/progress so there
        # is no gap between the two subscriptions — kApplying could fire while subscription_attr
        # is still active, and we must not miss it.
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

        subscription_attr.await_all_expected_report_matches([matcher_combined_obj], timeout_sec=800.0)
        logger.info('%s: Step #5.3 - UpdateState (Available sequence) matcher has completed.', step_number_s5)
        subscription_attr.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_5]: Step #5.4 - Verify image transfer from TH/OTA-P to DUT is successfully started.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #5.4 - Full OTA state sequence observed: %s', step_number_s5, state_sequence)
        logger.info('%s: Step #5.4 - Progress values observed: %s', step_number_s5, progress_values)

        expected_flows = [
            [Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading]
        ]

        if state_sequence in expected_flows:
            logger.info('%s: Step #5.4 - OTA flow is valid: %s', step_number_s5, state_sequence)
        else:
            msg = f"Observed OTA flow: {state_sequence}, Expected one of: {expected_flows}"
            asserts.fail(msg)

        asserts.assert_true(any(1 <= v <= 100 for v in progress_values),
                            f"{step_number_s5}: No valid UpdateStateProgress observed (1-100)")
        logger.info('%s: Step #5.4 - UpdateStateProgress has valid value(s) in range 1-100', step_number_s5)

        # ------------------------------------------------------------------------------------
        # [STEP_5]: Step #5.5 - Wait for kApplying to confirm the BDX transfer is fully
        # complete, then kill the provider. The DUT will finish applying and reboot on its own.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #5.5 - Waiting for kApplying to confirm download complete.', step_number_s5)

        def matcher_applying(report):
            val = report.value
            return val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        matcher_applying_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s5} - Wait for kApplying",
            matcher=matcher_applying
        )

        subscription_attr_applying.await_all_expected_report_matches([matcher_applying_obj], timeout_sec=800.0)
        logger.info('%s: Step #5.5 - kApplying observed — BDX transfer complete.', step_number_s5)
        subscription_attr_applying.cancel()

        logger.info('%s: Step #5.5 - Killing provider (download done, DUT applying firmware).', step_number_s5)
        self.current_provider_app_proc.terminate()

        # ------------------------------------------------------------------------------------
        # [STEP_5]: Step #5.6 - Wait for DUT to reboot after applying V2 firmware.
        # Expire the stale session so the controller reconnects cleanly, then poll until
        # GetConnectedDevice succeeds (DUT is back online).
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #5.6 - Expiring stale session and waiting for DUT to reboot.', step_number_s5)
        controller.ExpireSessions(requestor_node_id)

        reboot_timeout_sec = 120
        poll_interval_sec = 5
        reconnect_timeout_ms = 5000
        reconnected = False
        for attempt in range(reboot_timeout_sec // poll_interval_sec):
            await asyncio.sleep(poll_interval_sec)
            try:
                await controller.GetConnectedDevice(
                    requestor_node_id, allowPASE=False, timeoutMs=reconnect_timeout_ms)
                reconnected = True
                logger.info('%s: Step #5.6 - DUT reconnected after OTA reboot (attempt %s).', step_number_s5, attempt + 1)
                break
            except (TimeoutError, ChipDeviceCtrl.ChipStackError):
                logger.info('%s: Step #5.6 - Waiting for DUT to come back online (attempt %s/%s)...',
                            step_number_s5, attempt + 1, reboot_timeout_sec // poll_interval_sec)

        asserts.assert_true(
            reconnected, f'{step_number_s5}: DUT did not come back online within {reboot_timeout_sec}s after OTA reboot.')

        # Allow the DUT to finish post-OTA housekeeping (attribute writes, data-version
        # bumps on the OTA Requestor cluster) before Step 6 establishes a subscription.
        # Without this sleep, the subscription is invalidated immediately by a data-version
        # mismatch (Error 50) triggered by the DUT's own post-apply cluster updates.
        logger.info('%s: Step #5.6 - Waiting 15s for DUT to stabilize after OTA reboot.', step_number_s5)
        await asyncio.sleep(15)

        self.step(6)
        # ------------------------------------------------------------------------------------
        # [STEP_6]: Prerequisites - Setup Provider
        # The DUT has just applied the V2 firmware in Step 5. By serving the same V2 image here
        # with updateAvailable, the DUT sees it as "same version" and rejects the download.
        # No separate firmware image is needed — the single V2 image (ota_image) is reused,
        # meaning only one firmware image is required for the entire test.
        # ------------------------------------------------------------------------------------
        step_number_s6 = "[STEP_6]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s6, requestor_node_id, fabric_id)

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
        # [STEP_6]: Step #6.1 - Subscribe to UpdateState attribute.
        # Attribute subscription is used instead of event subscription because:
        #   1. After the Step 6 OTA reboot the DUT's event buffer contains stale
        #      StateTransition events (kApplying→kIdle etc.) that arrive first.
        #   2. Event subscriptions have no keepalive when no events are generated —
        #      the CASE session goes idle and expires, breaking the re-announce call.
        # Attribute subscriptions always send keepalives (current value every max_interval),
        # keeping the session alive and avoiding stale-event issues entirely.
        # ------------------------------------------------------------------------------------
        subscription_s6 = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        await subscription_s6.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False
        )

        kIdle_s6 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        kDownloading_s6 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kQuerying_s6 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying

        # Wait for the priming kIdle (DUT's current state on subscription start), then reset
        # accumulated history BEFORE sending the announce.  This guarantees that any kQuerying
        # or kIdle that arrives after the announce is a genuine post-announce report and cannot
        # be confused with the pre-announce priming report.
        priming_matcher_s6 = AttributeMatcher.from_callable(
            description=f"{step_number_s6} - initial kIdle priming report",
            matcher=lambda report: report.value == kIdle_s6
        )
        subscription_s6.await_all_expected_report_matches([priming_matcher_s6], timeout_sec=30.0)
        logger.info('%s: Initial kIdle (priming report) observed.', step_number_s6)
        subscription_s6.reset()

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #6.0 - Controller sends AnnounceOTAProvider command', step_number_s6)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #6.0 - sent cmd AnnounceOTAProvider.', step_number_s6)

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.1 - Wait for the DUT to query and reject the same-version image.
        # The DUT is on V2 and the provider offers V2, so the DUT should query (kQuerying)
        # but reject the image and return to kIdle without downloading.
        #
        # Two-phase wait to reliably cover the full kIdle→kQuerying→kIdle cycle:
        #   Phase 1 (720 s): accept kQuerying or kIdle.  kQuerying is transient — the DUT
        #     may complete the entire cycle between subscription polls, so a bare kIdle counts.
        #   Phase 2 (60 s): always runs after Phase 1.  Resets history and waits for kIdle.
        #     This prevents a false pass when Phase 1 fires on a periodic keepalive kIdle
        #     (max_interval_sec=30) that arrives before the DUT starts querying.
        # ------------------------------------------------------------------------------------
        logger.info(
            '%s: Step #6.1 - Waiting for kQuerying→kIdle sequence (DUT should reject the same-version image without downloading)', step_number_s6)

        downloading_seen_s6 = [False]
        querying_seen_s6 = [False]

        def phase1_matcher_s6(report):
            val = report.value
            if val == kDownloading_s6:
                downloading_seen_s6[0] = True
                logger.info('%s: UNEXPECTED kDownloading — DUT started download of same-version image!', step_number_s6)
                return False
            if val == kQuerying_s6:
                querying_seen_s6[0] = True
                logger.info('%s: kQuerying observed (expected)', step_number_s6)
                return True
            if val == kIdle_s6:
                logger.info('%s: kIdle observed (query cycle completed or too fast to capture kQuerying)', step_number_s6)
                return True
            return False

        phase1_matcher_s6_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s6} - post-announce kQuerying or kIdle, no kDownloading",
            matcher=phase1_matcher_s6
        )

        subscription_s6.await_all_expected_report_matches([phase1_matcher_s6_obj], timeout_sec=720.0)

        # Phase 2: reset and wait for kIdle.
        logger.info('%s: Phase 2: awaiting kIdle to confirm query cycle completed without download.', step_number_s6)
        subscription_s6.reset()

        def phase2_matcher_s6(report):
            val = report.value
            if val == kDownloading_s6:
                downloading_seen_s6[0] = True
                logger.info(
                    '%s: UNEXPECTED kDownloading in Phase 2 — DUT started download of same-version image!', step_number_s6)
                return False
            if val == kIdle_s6:
                logger.info('%s: kIdle confirmed — query cycle completed without download.', step_number_s6)
                return True
            return False

        phase2_matcher_s6_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s6} - post-kQuerying kIdle, no kDownloading",
            matcher=phase2_matcher_s6
        )

        subscription_s6.await_all_expected_report_matches([phase2_matcher_s6_obj], timeout_sec=120.0)
        logger.info('%s: Step #6.2 - Query cycle fully completed after announce.', step_number_s6)
        subscription_s6.cancel()

        asserts.assert_true(querying_seen_s6[0],
                            f"{step_number_s6}: kQuerying never observed — DUT did not query the provider after announce.")
        asserts.assert_false(downloading_seen_s6[0],
                             f"{step_number_s6}: DUT started downloading the same-version image (kDownloading seen).")
        logger.info("%s: No image transfer occurred (expected — DUT already on V2).", step_number_s6)


if __name__ == "__main__":
    default_matter_test_main()
