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

# Spec-mandated minimum intervals the DUT must observe before issuing another
# QueryImage command. These are requirements on the DUT, not tuning knobs — they
# apply to any device and are never derived from the test budget.
SPEC_GUARD_S1_SEC = 120
SPEC_GUARD_S2_SEC = 120
# DelayedActionTime this test configures on Step 3's provider, and therefore the interval
# the DUT must observe in that step. Unlike the two constants above this is not a spec
# value: the spec only requires the DUT to wait at least the DelayedActionTime it was
# given, and 3 minutes is the figure the test plan tells the TH/OTA-P to send. It is
# passed to the provider as -t below, so the guard and the provider cannot drift apart.
STEP3_DELAYED_ACTION_TIME_SEC = 180
# Fail Step 5 if UpdateStateProgress makes no progress for this long while waiting for
# kApplying. Per spec, UpdateStateProgress "MAY be updated infrequently" (quality Q) to
# avoid over-reporting, so a real DUT downloading over a slow transport (e.g. Thread,
# BLE) can legitimately go a long time between progress updates. Kept generous so this
# only catches a genuinely dead transfer, not spec-compliant sparse reporting.
DOWNLOAD_STALL_TIMEOUT_SEC = 1800
# Nominal execution reserve kept aside for each not-yet-started step when handing the
# remaining test budget to a wait, so early steps cannot starve later ones.
STEP_RESERVE_SEC = 60
# How long UpdateState must stay kIdle before Step 2 considers the DUT quiescent (its
# session-recovery re-query burst from the provider switch is over and its next-query timer
# is armed). This time is consumed from the 120s guard rather than added to it, since the spec
# measures the minimum from the last QueryImage. See _wait_until_idle_settled.
STEP2_SETTLE_SEC = 20
# Upper bound for observing Step 6's kQuerying→kIdle cycle via subscription reports. The
# cycle completes within ~1s of the announce on any DUT (a same-version query is answered
# and rejected immediately), so this is a generous safety bound, not an expected duration.
# It also bounds the failure case: if the transient kQuerying report is ever coalesced away
# by the subscription, Phase A fails fast at this timeout instead of waiting on the full
# test budget.
STEP6_CYCLE_TIMEOUT_SEC = 120
# Upper bound for the DUT to reach kIdle before a step announces. Only consumed when the DUT
# is not idle already, which is the exceptional case: the preceding step normally leaves it
# quiescent on its periodic query timer.
IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC = 120
# Log line every OTA provider prints on receiving a QueryImage, whatever it then answers: see
# OtaProviderLogic::QueryImage in src/app/clusters/ota-provider/OTAProviderCluster.cpp, which
# logs it before dispatching to the application delegate. Step 4 polls it on the provider's
# stdout as an out-of-band barrier: it is the only way to tell a query the provider actually
# answered (with the invalid ImageURI under test) from one that died in a CASE session left
# over from a previous provider process, since both leave the DUT in the same
# kQuerying→kIdle transition.
PROVIDER_QUERY_RECEIVED_LOG = "OTA Provider received QueryImage"


class TC_SU_2_2(SoftwareUpdateBaseTest):

    # The DUT reboots mid-test in Step 5. The framework's background wildcard
    # subscription (autoResubscribe=False) would silently die there with a stale value
    # cache, and its report traffic competes with the BDX transfer on constrained
    # devices — so opt out of it for this test.
    disable_wildcard_subscription = True

    @property
    def default_timeout(self) -> int:
        # Used only when no --timeout is passed on the command line (CI passes
        # --timeout 2100 via the test-runner args above). Real devices have unbounded
        # download/apply times, so default to a generous budget: every long wait is
        # event-driven and ends early on fast devices, so an oversized budget only
        # costs time when something is genuinely wrong.
        return 7200

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

    def _next_provider_log_path(self):
        """Return a fresh, unique log-file path for the next provider start.

        Every provider (re)start gets its own file so logs are not interleaved across
        restarts. The name embeds a monotonic start counter (so files sort in start order)
        and the current step index (for correlation), e.g. ``provider_logs/provider_03_step2.log``.
        """
        self._provider_start_count += 1
        step_idx = getattr(self, "current_step_index", 0)
        filename = f"provider_{self._provider_start_count:02d}_step{step_idx}.log"
        return os.path.join(self.PROVIDER_LOG_DIR, filename)

    def restart_provider_not_available(self):
        """Restart the OTA provider as a persistent 'updateNotAvailable' instance.

        With ``--persistQueryImageStatus`` the provider answers NotAvailable to EVERY
        QueryImage — it does not flip to updateAvailable after the first response (see
        ``OTAProviderExample::HandleQueryImage``). So it can be left running for the whole step
        without any restart: the DUT's CASE session to it stays valid (no session-recovery
        re-query artifact) and no image is ever served. Commissioning is preserved via the
        persistent KVS, so the DUT keeps talking to the same provider node.
        """
        self.terminate_provider()
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self._provider_setup_pincode,
            discriminator=self._provider_discriminator,
            port=self._provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self._next_provider_log_path(),
            extra_args=["-q", "updateNotAvailable", "--persistQueryImageStatus"],
        )

    def _wait_until_idle_settled(self, subscription, settle_sec, timeout_sec, step_name) -> float:
        """Block until UpdateState has stayed kIdle for ``settle_sec`` with no intervening
        non-idle report — i.e. the DUT has finished any post-provider-switch session-recovery
        churn and is quiescent on its next-query timer.

        Switching the provider between steps (e.g. Busy -> NotAvailable) invalidates the DUT's
        cached CASE session to the old provider, so the DUT's first query afterwards fails and
        it re-CASEs and retries (a burst of kQuerying spread over ~10-15s). This absorbs that
        burst so the 120s guard that follows measures the DUT's real, unperturbed behaviour.
        Fails if the DUT never settles within ``timeout_sec``.

        The caller must flush the report queue before triggering the DUT, so that a report which
        predates the trigger cannot be mistaken for the DUT having completed a query. (A freshly
        started subscription enqueues nothing by itself: its priming report is consumed by
        ReadAttribute() before the handler registers its callback, and this method therefore
        never settles on a DUT that is already idle and quiet when the subscription starts.)

        Returns:
            ``time.time()`` of the last transition into kIdle, i.e. when the DUT's final
            QueryImage exchange completed. TC-SU-2.2 measures the 120s minimum "from the last
            QueryImage command", so the caller must anchor its guard window on this timestamp
            rather than on this method's return, which is ``settle_sec`` later.
        """
        kIdle = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        q = subscription.attribute_queue
        deadline = time.time() + timeout_sec
        # Track the CURRENT UpdateState (unchanged states emit no reports, so absence of reports
        # must not be mistaken for idle). Settle only once the state has been kIdle continuously.
        current_state = None
        idle_since = None
        while True:
            now = time.time()
            if current_state == kIdle and idle_since is not None and now - idle_since >= settle_sec:
                return idle_since
            if now >= deadline:
                asserts.fail(f"{step_name}: DUT did not settle to a stable kIdle within "
                             f"{timeout_sec:.0f}s after the provider switch.")
            try:
                report = q.get(block=True, timeout=min(1.0, deadline - now))
            except queue.Empty:
                continue
            val = report.value
            if val == kIdle:
                if current_state != kIdle:
                    current_state = kIdle
                    idle_since = time.time()
            else:
                current_state = val
                idle_since = None

    async def _announce_until_provider_queried(self, controller, provider_node_id: int, requestor_node_id: int,
                                               timeout_sec: float, step_name: str,
                                               retry_interval_sec: float = 60.0) -> None:
        """Announce the provider, repeating until the provider reports receiving a QueryImage.

        One AnnounceOTAProvider does not guarantee the DUT queries this provider. The requestor
        drops an announce that arrives while UpdateState is not kIdle, and a query it does send
        can still die in a CASE session left over from the previous provider process — every step
        replaces that process — costing a message-layer timeout before the DUT tries again.
        Repeating the announce keeps the step moving in both cases instead of leaving it at the
        mercy of the DUT's own retry policy: DefaultOTARequestorDriver grants a single automatic
        retry per invalid session (kMaxInvalidSessionRetries) and the spec mandates none at all.

        Waiting on the provider's own report of the query, rather than on a DUT state change, is
        what makes the loop terminate for the right reason: it is the only signal that separates a
        query this provider answered from one that never arrived.

        The caller must arm the provider's matcher with PROVIDER_QUERY_RECEIVED_LOG right after
        starting it and before calling this, so a receipt landing between the announce and the
        wait cannot be missed.

        Announces stop as soon as the provider reports the query, so this never perturbs a
        timing guard that follows: any announce it sends is either dropped by a busy DUT or
        answered by the query that ends the loop.
        """
        proc = self.current_provider_app_proc
        t_start = time.time()
        attempt = 0

        while True:
            remaining = timeout_sec - (time.time() - t_start)
            if remaining <= 0:
                asserts.fail(f"{step_name}: the provider received no QueryImage within {timeout_sec:.0f}s "
                             f"of the first announce ({attempt} sent); the DUT never reached it.")
            attempt += 1
            try:
                await self.announce_ota_provider(
                    controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
                logger.info('%s: AnnounceOTAProvider sent (attempt %d).', step_name, attempt)
            except (TimeoutError, ChipDeviceCtrl.ChipStackError) as e:
                # Expected while the DUT is recovering from an aborted transfer: its session to
                # the controller can drop (e.g. under Wi-Fi power-save). Retry on the next pass.
                logger.info('%s: AnnounceOTAProvider failed (DUT transiently unreachable): %s; will retry.',
                            step_name, e)

            if proc.wait_for_output(timeout=min(retry_interval_sec, remaining)):
                logger.info('%s: provider received a QueryImage %.0fs after the first announce.',
                            step_name, time.time() - t_start)
                return

            logger.info('%s: no QueryImage reached the provider in %.0fs (elapsed %.0fs / %.0fs); re-announcing.',
                        step_name, retry_interval_sec, time.time() - t_start, timeout_sec)

    async def _wait_until_idle_before_announce(self, controller, requestor_node_id: int, subscription,
                                               timeout_sec: float, step_name: str) -> None:
        """Block until the DUT's UpdateState is kIdle, so the AnnounceOTAProvider that follows is
        acted on instead of dropped.

        The requestor silently ignores an AnnounceOTAProvider that arrives while UpdateState is
        not kIdle ("State is not kIdle, ignoring the AnnounceOTAProviders"), which would leave the
        step depending on whatever retry the DUT runs on its own rather than on the announce.

        The current value is READ rather than awaited from ``subscription``:
        AttributeSubscriptionHandler.start() registers its callback only after ReadAttribute() has
        already consumed the priming report, so nothing is enqueued for a DUT that is idle when the
        subscription starts — and an unchanged attribute is never reported again, so a pure wait
        would block until it times out. The subscription is used only for the case where the DUT
        still has to transition, which does produce a report.
        """
        kIdle = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        state = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState)
        if state == kIdle:
            logger.info('%s: DUT is already idle — the announce will be acted on.', step_name)
            return

        logger.info('%s: DUT is in %s; waiting up to %.0fs for it to reach kIdle before announcing.',
                    step_name, state, timeout_sec)
        subscription.await_first_value_asserting_no_forbidden(
            target_value=kIdle,
            forbidden_values=set(),
            timeout_sec=timeout_sec,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
        )
        logger.info('%s: DUT reached kIdle — the announce will be acted on.', step_name)

    @async_test_body
    async def test_TC_SU_2_2(self):
        # Each provider (re)start writes to its own log file under this directory instead of
        # appending to a single shared log, so each step's provider activity can be inspected
        # in isolation. See _next_provider_log_path.
        self.PROVIDER_LOG_DIR = "provider_logs"
        self._provider_start_count = 0
        os.makedirs(self.PROVIDER_LOG_DIR, exist_ok=True)
        self.KVS_PATH = "/tmp/chip_kvs_provider"
        self.provider_app_path = self.user_params.get('provider_app_path')
        self.ota_image = self.user_params.get('ota_image')

        # Validate provider_app_path
        if not self.provider_app_path or not os.path.exists(self.provider_app_path):
            raise FileNotFoundError(f'Invalid provider_app_path: {self.provider_app_path}.')

        # Validate ota_image and check if we can proceed with the image
        if not self.ota_image or not os.path.exists(self.ota_image):
            raise FileNotFoundError(f'Invalid ota_image: {self.ota_image}.')

        # All device-dependent waits below are sized from the remaining overall test
        # budget (asyncio.wait_for timeout) instead of per-device constants, since
        # download/apply/recovery durations of a real DUT cannot be known up front.
        self.start_test_budget_clock()

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

        # Stored so restart_provider_not_available() can rebuild the provider on the same node
        # (KVS-persisted commissioning) when switching it to NotAvailable for Step 2.
        self._provider_setup_pincode = provider_setup_pincode
        self._provider_discriminator = provider_discriminator
        self._provider_port = provider_port
        # Validate ota image if is valid and can proceed. The version is kept so Step 5 can
        # confirm the DUT actually came back running this image.
        ota_image_version = await self.check_ota_image_version(
            controller=controller, requestor_node_id=requestor_node_id, ota_image_path=self.ota_image)

        # Pre-define all provider arg sets for reuse across steps
        provider_extra_args_updateAvailable = [
            "-q", "updateAvailable"
        ]

        # ------------------------------------------------------------------------------------
        # Provider commissioning (done once before any step that requires AnnounceOTAProvider).
        # Start the provider for the first time with Busy/60s args (used in Step 2) so that
        # commissioning can happen before Step 2 runs.
        # ------------------------------------------------------------------------------------
        # --persistQueryImageStatus: keep answering Busy (with DelayedActionTime 60) on EVERY
        # query instead of flipping to updateAvailable after the first response. The DUT then
        # re-queries into Busy again (never downloads), so Step 1 needs no provider restart and
        # incurs no session-recovery re-query artifact.
        provider_extra_args_busy = [
            "-q", "busy",
            "-t", "60",
            "--persistQueryImageStatus"
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self._next_provider_log_path(),
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
        # Two-phase queue-based verification:
        #
        #   Phase A — wait for kDelayedOnQuery to confirm the DUT received the Busy/60s
        #             response.  kDownloading before kDelayedOnQuery is an immediate fail.
        #
        #   Phase B — STRICTLY verify the DUT issues no new QueryImage within the 120s spec
        #             minimum. The provider serves Busy on every query (--persistQueryImageStatus)
        #             and is NOT restarted, so the DUT's CASE session stays valid and it simply
        #             stays in kDelayedOnQuery until it re-queries at ~120s (Busy again — never a
        #             download). Any kQuerying inside the guard window is a genuine early re-query
        #             and hard-fails; kDownloading/kApplying also fail.
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
            # Reserve the spec guard windows of Steps 1-3 plus a nominal reserve for
            # each remaining step (2-6).
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=SPEC_GUARD_S1_SEC + SPEC_GUARD_S2_SEC + STEP3_DELAYED_ACTION_TIME_SEC + 5 * STEP_RESERVE_SEC),
        )
        logger.info('%s: Phase A complete — kDelayedOnQuery at %.2f', step_number_s1, t_delayed_on_query_s1)

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Phase B — STRICTLY verify the DUT does not issue another QueryImage within
        # the 120s spec minimum after the Busy response.
        #
        # The provider serves Busy on every query (--persistQueryImageStatus) and is NOT
        # restarted, so the DUT's CASE session stays valid the whole time. The DUT therefore
        # simply stays in kDelayedOnQuery and only re-queries at ~120s (getting Busy again —
        # never a download, no session-recovery churn). Any kQuerying inside the guard window is
        # a genuine early re-query and hard-fails; kDownloading/kApplying also fail. The
        # tolerance zone at the end absorbs the boundary re-query at ~120s.
        # ------------------------------------------------------------------------------------
        kQuerying_s1 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        tolerance_s1_sec = 5.0
        logger.info('%s: Phase B — verifying no re-query for %ss (tolerance %ss) after Busy.',
                    step_number_s1, SPEC_GUARD_S1_SEC, tolerance_s1_sec)
        subscription_attr_state_busy.await_duration_asserting_no_forbidden(
            duration_sec=SPEC_GUARD_S1_SEC,
            forbidden_values={kQuerying_s1, kDownloading_s1, kApplying_s1},
            tolerance_sec=tolerance_s1_sec,
        )
        logger.info('%s: Phase B complete — DUT observed the %ss minimum (no early re-query).',
                    step_number_s1, SPEC_GUARD_S1_SEC)

        subscription_attr_state_busy.cancel()

        self.step(2)
        # ------------------------------------------------------------------------------------
        # [STEP_2]: Prerequisites - Setup a persistent updateNotAvailable provider.
        #
        # With --persistQueryImageStatus the provider answers NotAvailable to EVERY query and is
        # NOT restarted during the measurement, so the DUT's session stays valid and its
        # re-query timing is unperturbed — letting Phase B verify the 120s minimum strictly.
        # Switching from Step 1's Busy provider to this one is the single restart of the step; it
        # invalidates the DUT's cached session, so the DUT's first query here fails and it
        # re-CASEs (a short recovery burst). Phase A absorbs that burst before the guard starts.
        # ------------------------------------------------------------------------------------
        step_number_s2 = "[STEP_2]"
        logger.info('%s: Prerequisite #1.0 - Requestor (DUT), NodeID: %s, FabricId: %s',
                    step_number_s2, requestor_node_id, fabric_id)

        # The restart replaces the provider process only. It invalidates the DUT's CASE session
        # to the provider, which Phase A absorbs; the controller's own session to the DUT is
        # unaffected and is deliberately left in place. Expiring it here would discard a working
        # session and force the subscription below through a full mDNS re-discovery and CASE
        # handshake for no benefit.
        self.restart_provider_not_available()

        # Arm the barrier before the announce below, so that a QueryImage reaching this provider
        # cannot slip past unnoticed. restart_provider_not_available() leaves the match armed on
        # start_provider()'s own "Server initialization complete" wait, so this must follow it.
        self.current_provider_app_proc.arm_output_match(PROVIDER_QUERY_RECEIVED_LOG)

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

        kQuerying_s2 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        kDownloading_s2 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kApplying_s2 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying
        tolerance_s2 = 5.0

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.0 - Controller sends AnnounceOTAProvider command, repeating until the
        # provider confirms it received a QueryImage.
        #
        # Entering this step the DUT is still in kDelayedOnQuery from Step 1, so the first
        # announce is dropped and the DUT reaches this provider on its own next-query timer.
        # Waiting for the provider's confirmation rather than assuming the announce worked is what
        # establishes the step's premise — that a NotAvailable response actually reached the DUT.
        # Without it Phase A would settle on any kIdle, including one the DUT enters without
        # contacting this provider at all (an invalid session it does not retry, or its Busy retry
        # budget running out), and the guard would then measure silence that proves nothing.
        #
        # Flush first: any report the DUT emitted between the subscription starting and the
        # announce predates the trigger, and left in the queue could satisfy Phase A's settle
        # without the DUT having queried at all.
        # ------------------------------------------------------------------------------------
        subscription_attr_state_updatenotavailable.flush_reports()
        logger.info('%s: Step #2.0 - Controller sends AnnounceOTAProvider command', step_number_s2)
        await self._announce_until_provider_queried(
            controller=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=SPEC_GUARD_S2_SEC + STEP3_DELAYED_ACTION_TIME_SEC + 4 * STEP_RESERVE_SEC),
            step_name=step_number_s2,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Phase A — let the DUT settle to a stable kIdle, absorbing any session-recovery
        # re-query burst from the provider switch. The query itself has already been confirmed
        # above, so what remains is to find where the DUT came to rest: after a NotAvailable
        # response it arms its long next-query timer and goes quiet, and once it has been kIdle
        # for STEP2_SETTLE_SEC the recovery is over.
        #
        # The returned timestamp is when the DUT last entered kIdle, i.e. when its last
        # QueryImage exchange completed — the point the spec measures the 120s minimum from.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Phase A — waiting for the DUT to receive NotAvailable and settle to kIdle.', step_number_s2)
        t_last_query_s2 = self._wait_until_idle_settled(
            subscription=subscription_attr_state_updatenotavailable,
            settle_sec=STEP2_SETTLE_SEC,
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=SPEC_GUARD_S2_SEC + STEP3_DELAYED_ACTION_TIME_SEC + 4 * STEP_RESERVE_SEC),
            step_name=step_number_s2,
        )
        subscription_attr_state_updatenotavailable.flush_reports()
        logger.info('%s: Phase A complete — DUT settled on NotAvailable; starting the %ss guard.',
                    step_number_s2, SPEC_GUARD_S2_SEC)

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Phase B — STRICTLY verify the DUT does not issue another QueryImage within
        # the 120s spec minimum of the last (NotAvailable) query. The persistent provider is left
        # running (no restart), so any kQuerying here is a genuine early re-query and hard-fails;
        # kDownloading/kApplying also fail.
        #
        # The guard is measured from the last QueryImage, per TC-SU-2.2, so the time already
        # spent settling counts towards the 120s instead of extending it: waiting the full
        # SPEC_GUARD_S2_SEC here would demand ~120 + STEP2_SETTLE_SEC seconds of silence and
        # fail a DUT that legitimately re-queries just after the spec minimum.
        # ------------------------------------------------------------------------------------
        guard_remaining_s2 = SPEC_GUARD_S2_SEC - (time.time() - t_last_query_s2)
        asserts.assert_greater(
            guard_remaining_s2, tolerance_s2,
            f"{step_number_s2}: settling consumed the whole {SPEC_GUARD_S2_SEC}s guard window "
            f"({guard_remaining_s2:.1f}s left); nothing meaningful is left to verify.")
        logger.info('%s: Phase B — verifying no re-query for %.1fs (tolerance %ss); %ss minus %.1fs already settled.',
                    step_number_s2, guard_remaining_s2, tolerance_s2, SPEC_GUARD_S2_SEC,
                    SPEC_GUARD_S2_SEC - guard_remaining_s2)
        subscription_attr_state_updatenotavailable.await_duration_asserting_no_forbidden(
            duration_sec=guard_remaining_s2,
            forbidden_values={kQuerying_s2, kDownloading_s2, kApplying_s2},
            tolerance_sec=tolerance_s2,
        )
        logger.info('%s: Phase B complete — DUT observed the %ss minimum (no early re-query).',
                    step_number_s2, SPEC_GUARD_S2_SEC)

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
            "-t", str(STEP3_DELAYED_ACTION_TIME_SEC)
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=provider_setup_pincode,
            discriminator=provider_discriminator,
            port=provider_port,
            kvs_path=self.KVS_PATH,
            log_file=self._next_provider_log_path(),
            extra_args=provider_extra_args_busy_180,
        )

        # Arm the barrier before the announce below; start_provider() leaves the match armed on
        # its own "Server initialization complete" wait, so this must follow it.
        self.current_provider_app_proc.arm_output_match(PROVIDER_QUERY_RECEIVED_LOG)

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

        # Announce until this provider confirms it received the query. The provider process was
        # just replaced, so the DUT's first query can die in the session it cached for Step 2's
        # process; re-announcing keeps the step moving without relying on the DUT retrying by
        # itself. Announces stop at the confirmation, well before the guard window below.
        logger.info('%s: Step #3.0 - Controller sends AnnounceOTAProvider command', step_number_s3)
        await self._announce_until_provider_queried(
            controller=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=STEP3_DELAYED_ACTION_TIME_SEC + 3 * STEP_RESERVE_SEC),
            step_name=step_number_s3,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Phase A — wait for kDelayedOnQuery (DUT received Busy/180s).
        # The provider auto-switches to UpdateAvailable after one Busy response.
        # kDownloading before kDelayedOnQuery is an immediate fail.
        # ------------------------------------------------------------------------------------
        kQuerying_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        kDelayedOnQuery_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery
        kDownloading_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        kApplying_s3 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        logger.info('%s: Phase A — waiting for kDelayedOnQuery (DUT received Busy/180s)', step_number_s3)
        t_delayed_on_query_s3 = subscription_attr_state_busy_180s.await_first_value_asserting_no_forbidden(
            target_value=kDelayedOnQuery_s3,
            forbidden_values={kDownloading_s3, kApplying_s3},
            # Reserve the Step 3 DelayedActionTime window plus a nominal reserve for
            # each remaining step (4-6).
            timeout_sec=self.remaining_test_budget_sec(
                reserve_sec=STEP3_DELAYED_ACTION_TIME_SEC + 3 * STEP_RESERVE_SEC),
        )
        logger.info('%s: Phase A complete — kDelayedOnQuery at %.2f, 180s guard window starts',
                    step_number_s3, t_delayed_on_query_s3)

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Phase B — 180s guard window; kQuerying/kDownloading/kApplying are forbidden.
        # After the window elapses, the provider has auto-switched to UpdateAvailable and the
        # DUT's next query will trigger a download.
        #
        # kQuerying is what the step actually verifies the absence of ("the DUT waits for at
        # least the DelayedActionTime before issuing another QueryImage"), so it is checked
        # directly rather than only through its consequence: relying on kDownloading alone would
        # miss an early query whose response never led to a transfer, and would measure the
        # interval to the download rather than to the query itself.
        #
        # Forbidding it here is safe because the window is anchored on kDelayedOnQuery, which
        # the DUT only reaches after it has found this provider and received its response — any
        # session-recovery re-query from the provider switch is already behind us. The provider
        # also runs untouched for the whole window (it is killed only once kDownloading is
        # confirmed), so no further session invalidation can occur inside it.
        # ------------------------------------------------------------------------------------
        tolerance_s3 = 5.0
        min_interval_s3 = STEP3_DELAYED_ACTION_TIME_SEC

        logger.info(
            '%s: Phase B — guarding %ss DelayedActionTime (tolerance %ss). kQuerying/kDownloading/kApplying forbidden. This will take ~3 minutes.', step_number_s3, min_interval_s3, tolerance_s3)

        subscription_attr_state_busy_180s.await_duration_asserting_no_forbidden(
            duration_sec=min_interval_s3,
            forbidden_values={kQuerying_s3, kDownloading_s3, kApplying_s3},
            tolerance_sec=tolerance_s3,
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Phase C — wait for kDownloading to confirm the DUT re-queried after 180s.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Phase C — waiting for kDownloading (DUT re-queried after 180s delay)', step_number_s3)
        t_downloading_s3 = subscription_attr_state_busy_180s.await_first_value_asserting_no_forbidden(
            target_value=kDownloading_s3,
            forbidden_values=set(),
            # Nominal reserve for each remaining step (4-6); no spec guard windows left.
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=3 * STEP_RESERVE_SEC),
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
        # minutes to recover (BDX timeout + retry backoff). Step 4 tolerates any stale
        # StateTransition events left behind by filtering them out in its event loop.

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
            log_file=self._next_provider_log_path(),
            extra_args=provider_extra_args_invalid_bdx,
        )

        # Arm the out-of-band barrier now, before the announce below, so that a QueryImage
        # reaching the provider can never slip past unnoticed between the announce and the
        # check in the event loop. This has to run after start_provider(), which leaves the
        # match armed on its own "Server initialization complete" wait.
        self.current_provider_app_proc.arm_output_match(PROVIDER_QUERY_RECEIVED_LOG)

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
        # BDX recovery time (timeout + retry backoff) is vendor-specific and unbounded by
        # the spec, so the outer limit is the remaining test budget (reserving a nominal
        # slice for Steps 5-6) rather than a guessed constant. The 60s re-announce loop
        # below is the liveness mechanism that picks the DUT up as soon as it recovers.
        event1 = None
        s4_timeout = self.remaining_test_budget_sec(reserve_sec=2 * STEP_RESERVE_SEC)
        t_s4_start = time.time()

        while time.time() - t_s4_start < s4_timeout:
            remaining = s4_timeout - (time.time() - t_s4_start)
            try:
                raw = subscription_state_invalid_uri.get_event_from_queue(block=True, timeout=min(60.0, remaining))
            except queue.Empty:
                # No event for 60 s — DUT may have missed AnnounceOTAProvider while busy.
                # Re-send so the DUT queries as soon as it returns to kIdle.
                logger.info("%s: No event in 60s, re-sending AnnounceOTAProvider (elapsed: %.0fs / %.0fs)",
                            step_number_s4, time.time() - t_s4_start, s4_timeout)
                # While the DUT recovers from Step 3's aborted BDX transfer it is frequently
                # unreachable (its CASE session to the controller drops, e.g. under Wi-Fi
                # power-save), so this re-announce InvokeCommand can time out. That is expected
                # during recovery: swallow it and retry on the next iteration rather than
                # failing the step — the loop is already bounded by the remaining test budget.
                try:
                    await self.announce_ota_provider(
                        controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
                except (TimeoutError, ChipDeviceCtrl.ChipStackError) as e:
                    logger.info("%s: re-announce AnnounceOTAProvider failed (DUT transiently unreachable): %s; "
                                "will retry.", step_number_s4, e)
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
                            f"{step_number_s4}: Idle→Querying transition not found within {s4_timeout:.0f}s "
                            "(remaining test budget exhausted)")

        # --- Transition 2: Querying → Idle ---
        # Apply the same stale-event filtering as Transition 1 — earlier steps may have left
        # residual events in the queue (e.g. kDownloading→kIdle from an aborted BDX session).
        #
        # Additionally, only a transition that follows a QueryImage the provider ACTUALLY
        # RECEIVED proves the DUT rejected the invalid ImageURI. A query that timed out in a
        # CASE session left over from Step 3's provider process produces the very same
        # kQuerying→kIdle, and accepting that one would pass the step without the invalid-URI
        # response ever reaching the DUT. The provider reports receipt out of band on its
        # stdout (PROVIDER_QUERY_RECEIVED_LOG), which is polled here per candidate event.
        event2 = None
        s4_t2_timeout = self.remaining_test_budget_sec(reserve_sec=2 * STEP_RESERVE_SEC)
        t_s4_t2_start = time.time()

        while time.time() - t_s4_t2_start < s4_t2_timeout:
            remaining = s4_t2_timeout - (time.time() - t_s4_t2_start)
            try:
                raw = subscription_state_invalid_uri.get_event_from_queue(block=True, timeout=min(30.0, remaining))
            except queue.Empty:
                # A query that died before reaching the provider leaves the DUT idle on its
                # long periodic timer, and its transition was discarded below, so nothing
                # further arrives unless the DUT is nudged into querying again. Re-announce
                # for the same reason (and with the same tolerance for a transiently
                # unreachable DUT) as the Transition 1 loop above.
                logger.info("%s: No event in 30s, re-sending AnnounceOTAProvider (elapsed: %.0fs / %.0fs)",
                            step_number_s4, time.time() - t_s4_t2_start, s4_t2_timeout)
                try:
                    await self.announce_ota_provider(
                        controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
                except (TimeoutError, ChipDeviceCtrl.ChipStackError) as e:
                    logger.info("%s: re-announce AnnounceOTAProvider failed (DUT transiently unreachable): %s; "
                                "will retry.", step_number_s4, e)
                continue

            if raw.Header.EventId != Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id:
                continue

            evt2 = raw.Data
            if evt2.previousState == kQuerying and evt2.newState == kIdle:
                if not self.current_provider_app_proc.wait_for_output(timeout=0):
                    logger.info("%s: Discarding Querying→Idle — no QueryImage has reached the provider yet, "
                                "so the query ended before it could be answered.", step_number_s4)
                    continue
                event2 = evt2
                logger.info("%s: Event 2 (Querying→Idle): %s", step_number_s4, event2)
                break

            if evt2.newState in s4_forbidden_states:
                asserts.fail(f"{step_number_s4}: DUT entered {evt2.newState} — "
                             "image transfer started despite invalid BDX ImageURI!")
            logger.info("%s: Discarding stale event (transition 2): %s → %s", step_number_s4, evt2.previousState, evt2.newState)

        asserts.assert_true(event2 is not None,
                            f"{step_number_s4}: no Querying→Idle transition following a QueryImage that the provider "
                            f"actually received was observed within {s4_t2_timeout:.0f}s "
                            "(remaining test budget exhausted)")

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
            log_file=self._next_provider_log_path(),
            extra_args=provider_extra_args_updateAvailable,
        )

        # Arm the barrier before the announce below; start_provider() leaves the match armed on
        # its own "Server initialization complete" wait, so this must follow it.
        self.current_provider_app_proc.arm_output_match(PROVIDER_QUERY_RECEIVED_LOG)

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
        # [STEP_5]: Step #5.0 - Controller sends AnnounceOTAProvider command.
        #
        # Two distinct things can keep this step's announce from producing a download, so both are
        # handled: the DUT dropping the announce because it is not idle (see
        # _wait_until_idle_before_announce), and the query it then sends dying in the CASE session
        # cached for Step 4's provider process (see _announce_until_provider_queried). Flush
        # between them so the matcher below sees only post-announce reports.
        # ------------------------------------------------------------------------------------
        await self._wait_until_idle_before_announce(
            controller=controller,
            requestor_node_id=requestor_node_id,
            subscription=subscription_attr,
            timeout_sec=IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC,
            step_name=step_number_s5,
        )
        subscription_attr.flush_reports()

        logger.info('%s: Step #5.0 - Controller sends AnnounceOTAProvider command', step_number_s5)
        await self._announce_until_provider_queried(
            controller=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC),
            step_name=step_number_s5,
        )

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
        subscription_attr.await_all_expected_report_matches(
            [matcher_combined_obj],
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))
        logger.info('%s: Step #5.3 - UpdateState (Available sequence) matcher has completed.', step_number_s5)

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
        #
        # The download duration of a real DUT is unknown, so instead of an absolute timeout
        # this wait is bounded by a progress-stall watchdog: as long as UpdateStateProgress
        # keeps advancing, the wait continues (up to the remaining test budget). Reports
        # already queued from Step 5.3 (kDownloading, early progress) are harmless: they are
        # not the target and merely count as liveness. expected_attribute is required here
        # because the cluster-wide queue also carries UpdateStateProgress ints, which would
        # otherwise compare equal to UpdateStateEnum values.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #5.5 - Waiting for kApplying (progress-stall watchdog: %ss).',
                    step_number_s5, DOWNLOAD_STALL_TIMEOUT_SEC)

        kApplying_s5 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying
        last_progress_s5 = [None]

        def progress_liveness_s5(report):
            if report.attribute != Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                return False
            val = getattr(report.value, "value", report.value)
            if val is None or val == last_progress_s5[0]:
                return False
            last_progress_s5[0] = val
            return True

        subscription_attr.await_first_value_asserting_no_forbidden(
            target_value=kApplying_s5,
            forbidden_values=set(),
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC),
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
            stall_timeout_sec=DOWNLOAD_STALL_TIMEOUT_SEC,
            liveness_matcher=progress_liveness_s5,
        )
        logger.info('%s: Step #5.5 - kApplying observed — BDX transfer complete.', step_number_s5)
        subscription_attr.cancel()

        logger.info('%s: Step #5.5 - Killing provider (download done, DUT applying firmware).', step_number_s5)
        self.current_provider_app_proc.terminate()

        # ------------------------------------------------------------------------------------
        # [STEP_5]: Step #5.6 - Wait for DUT to reboot after applying V2 firmware.
        # Expire the stale session so the controller reconnects cleanly, then poll until
        # GetConnectedDevice succeeds (DUT is back online). Applying firmware on a real DUT
        # (signature check, flash bank copy, reboot, mDNS re-advertisement) has no known
        # upper bound and offers no observable liveness signal, so the polling loop itself
        # is the liveness mechanism and runs until the remaining test budget is exhausted.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #5.6 - Expiring stale session and waiting for DUT to reboot.', step_number_s5)
        controller.ExpireSessions(requestor_node_id)

        reboot_timeout_sec = self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC)
        poll_interval_sec = 5
        reconnect_timeout_ms = 5000
        reconnected = False
        t_reboot_start = time.time()
        attempt = 0
        while time.time() - t_reboot_start < reboot_timeout_sec:
            attempt += 1
            await asyncio.sleep(poll_interval_sec)
            try:
                await controller.GetConnectedDevice(
                    requestor_node_id, allowPASE=False, timeoutMs=reconnect_timeout_ms)
                reconnected = True
                logger.info('%s: Step #5.6 - DUT reconnected after OTA reboot (attempt %s).', step_number_s5, attempt)
                break
            except (TimeoutError, ChipDeviceCtrl.ChipStackError):
                logger.info('%s: Step #5.6 - Waiting for DUT to come back online (attempt %s, elapsed %.0fs / %.0fs)...',
                            step_number_s5, attempt, time.time() - t_reboot_start, reboot_timeout_sec)

        asserts.assert_true(
            reconnected,
            f'{step_number_s5}: DUT did not come back online within {reboot_timeout_sec:.0f}s after OTA reboot '
            '(remaining test budget exhausted).')

        # Allow the DUT to finish post-OTA housekeeping (attribute writes, data-version
        # bumps on the OTA Requestor cluster) before Step 6 establishes a subscription.
        # Without this sleep, the subscription is invalidated immediately by a data-version
        # mismatch (Error 50) triggered by the DUT's own post-apply cluster updates.
        logger.info('%s: Step #5.6 - Waiting 15s for DUT to stabilize after OTA reboot.', step_number_s5)
        await asyncio.sleep(15)

        # Confirm the DUT is really running the new image. Otherwise the only evidence that the
        # apply succeeded is Step 6's indirect one — a DUT still on the old version would treat
        # the same image as an upgrade and download it again — which surfaces as a spurious
        # transfer there rather than as the version mismatch it actually is.
        await self.verify_version_applied_basic_information(
            controller=controller, node_id=requestor_node_id, target_version=ota_image_version)
        logger.info('%s: Step #5.6 - DUT confirmed running software version %s.', step_number_s5, ota_image_version)

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
            log_file=self._next_provider_log_path(),
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
        kApplying_s6 = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying
        s6_forbidden = {kDownloading_s6, kApplying_s6}
        UpdateState_s6 = Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState

        # Confirm the DUT is idle before announcing, then flush the report queue so the strict
        # ordered wait below sees only post-announce reports (a lingering kIdle report would
        # otherwise sit ahead of the real cycle in the FIFO queue).
        await self._wait_until_idle_before_announce(
            controller=controller,
            requestor_node_id=requestor_node_id,
            subscription=subscription_s6,
            timeout_sec=IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC,
            step_name=step_number_s6,
        )
        subscription_s6.flush_reports()

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #6.0 - Controller sends AnnounceOTAProvider command', step_number_s6)
        await self.announce_ota_provider(controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
        logger.info('%s: Step #6.0 - sent cmd AnnounceOTAProvider.', step_number_s6)

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.1 - The DUT (on V2) must query the provider (which offers the same
        # V2), reject the image, and return to kIdle WITHOUT downloading.
        #
        # Strict-order verification over the FIFO report queue. await_first_value_asserting_no_
        # forbidden consumes reports one at a time in arrival order, so:
        #   Phase A: wait for kQuerying (the DUT queries the provider), then
        #   Phase B: wait for kIdle (the DUT returns to idle).
        # Phase B starts only after Phase A has consumed the kQuerying report, so it can only be
        # satisfied by a kIdle that genuinely FOLLOWS the query — the kQuerying→kIdle order is
        # enforced, not merely "both were seen at some point". A pre-query priming/keepalive
        # kIdle cannot satisfy Phase A (only kQuerying does) and was flushed above anyway.
        # kDownloading/kApplying at any point fails immediately (no same-version transfer).
        #
        # Being queue-based (consume-once) rather than history-rescan, this does not hang on a
        # fast requestor: the terminal kIdle is a real value change that is reported and queued,
        # so Phase B consumes it instead of waiting for a fresh kIdle that an unchanged
        # attribute would never re-send. The bounded timeout turns the one remaining edge case —
        # a kQuerying report coalesced away entirely — into a prompt, clear failure rather than
        # an indefinite wait.
        # ------------------------------------------------------------------------------------
        logger.info('%s: Step #6.1 - Phase A — waiting for kQuerying (DUT queries the provider after announce).',
                    step_number_s6)
        subscription_s6.await_first_value_asserting_no_forbidden(
            target_value=kQuerying_s6,
            forbidden_values=s6_forbidden,
            timeout_sec=STEP6_CYCLE_TIMEOUT_SEC,
            expected_attribute=UpdateState_s6,
        )
        logger.info('%s: kQuerying observed (expected).', step_number_s6)

        logger.info('%s: Phase B — waiting for return to kIdle (same-version image rejected, no download).',
                    step_number_s6)
        subscription_s6.await_first_value_asserting_no_forbidden(
            target_value=kIdle_s6,
            forbidden_values=s6_forbidden,
            timeout_sec=STEP6_CYCLE_TIMEOUT_SEC,
            expected_attribute=UpdateState_s6,
        )
        logger.info('%s: kIdle after query observed — cycle completed without download.', step_number_s6)

        subscription_s6.cancel()
        logger.info('%s: Step #6.2 - Query cycle fully completed after announce.', step_number_s6)
        logger.info("%s: No image transfer occurred (expected — DUT already on V2).", step_number_s6)


if __name__ == "__main__":
    default_matter_test_main()
