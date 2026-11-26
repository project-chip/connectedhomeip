#
#    Copyright (c) 2026 Project CHIP Authors
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
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import queue
import time
from typing import Any

from mobly import asserts
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.clusters import Command as ClusterCommand
from matter.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class AttributeReportTracker:
    """Callback tracker that records received attribute reports with monotonic timestamps."""

    def __init__(self, expected_attribute: type[Clusters.ClusterObjects.ClusterAttributeDescriptor],
                 output_queue: queue.Queue[tuple[float, Any]]) -> None:
        """Initializes the attribute report tracker.

        Args:
            expected_attribute: The cluster attribute descriptor type to filter and track.
            output_queue: Queue where received (timestamp, value) tuples are enqueued.
        """
        self._expected_attribute = expected_attribute
        self._output_queue = output_queue

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction) -> None:
        """Subscription callback invoked on attribute report updates.

        Args:
            path: The attribute path reported.
            transaction: The active subscription transaction.
        """
        if path.AttributeType == self._expected_attribute:
            recv_time = time.monotonic()
            val = transaction.GetAttribute(path)
            log.info("Received attribute report for %s: %r at timestamp %.6f", path.AttributeType, val, recv_time)
            self._output_queue.put((recv_time, val))


class TC_IDM_1_5(IDMBaseTest):
    """[TC-IDM-1.5] DelayReportData Invoke Request Action [DUT as Server]"""

    @pics("MCORE.IDM.S")
    @async_test_body
    async def test_TC_IDM_1_5(self) -> None:
        """[TC-IDM-1.5] DelayReportData Invoke Request Action [DUT as Server]"""
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        # Step 1: Check if this device supports ICD, skip all remaining steps if it does
        self.step(1, "Check if this device supports ICD, skip all remaining steps if it does.", is_commissioning=True)
        ep0_servers = await self.get_descriptor_server_list(dev_ctrl, ep=0)
        if Clusters.IcdManagement.id in ep0_servers:
            log.info("DUT supports ICD (IcdManagement cluster present on EP0); skipping remaining steps as test requires a non-ICD device")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            return

        # GeneralCommissioning cluster on Endpoint 0 is mandatory for all Matter devices
        endpoint = 0
        target_attribute = Clusters.GeneralCommissioning.Attributes.Breadcrumb

        report_queue: queue.Queue = queue.Queue()
        tracker = AttributeReportTracker(target_attribute, report_queue)

        delay_min_ms = 1000
        delay_jitter_window_ms = 1000
        min_delay_window_sec = (delay_min_ms + delay_jitter_window_ms) / 1000.0
        mrp_timeout_sec = self.get_mrp_retransmission_timeout_sec(dev_ctrl)
        # In Step 2: Full end-to-end window requires 2x MRP worst-case retransmissions (Invoke leg + ReportData leg)
        min_required_sub_window_sec = min_delay_window_sec + (2 * mrp_timeout_sec)
        # In Step 4: After command is delivered, waiting only for delay + 1x MRP (ReportData leg)
        report_wait_timeout_sec = min_delay_window_sec + mrp_timeout_sec
        log.info("Computed MRP retransmission timeout: %.2f s (required sub window: %.2f s, report wait timeout: %.2f s)",
                 mrp_timeout_sec, min_required_sub_window_sec, report_wait_timeout_sec)
        min_interval_floor_sec = 0
        max_interval_ceiling_sec = 3600

        subscription = None
        failsafe_armed = False
        try:
            # Step 2: Establish subscription to Breadcrumb with MinIntervalFloor = 0s, MaxIntervalCeiling = 3600s
            self.step(
                2, "TH establishes a subscription to the Breadcrumb attribute of the GeneralCommissioning cluster on Endpoint 0 of the DUT with MinIntervalFloor set to 0 seconds and MaxIntervalCeiling set to 3600 seconds. TH synchronously awaits the initial prime report from the subscription establishment, records the timestamp t_prime, and verifies the negotiated MaxInterval from the SubscribeResponseMessage is within [0, 3600] seconds and leaves sufficient window for DelayReportData.")
            log.info("Establishing subscription to Breadcrumb attribute on Endpoint 0 with MinIntervalFloor=%ds, MaxIntervalCeiling=%ds",
                     min_interval_floor_sec, max_interval_ceiling_sec)
            # Record t_prime before sending SubscribeRequest; the publisher's MaxInterval window
            # cannot begin earlier, giving a conservative remaining-window estimate.
            t_prime = time.monotonic()
            subscription = await dev_ctrl.ReadAttribute(
                nodeId=dut_node_id,
                attributes=[(endpoint, target_attribute)],
                reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
                keepSubscriptions=False,
                autoResubscribe=False
            )
            subscription.SetAttributeUpdateCallback(tracker)

            _, negotiated_max_interval_sec = subscription.GetReportingIntervalsSeconds()
            log.info("Negotiated subscription intervals: MaxInterval = %ds", negotiated_max_interval_sec)
            asserts.assert_greater_equal(
                negotiated_max_interval_sec, min_interval_floor_sec,
                f"Negotiated MaxInterval ({negotiated_max_interval_sec}s) must be >= MinIntervalFloor ({min_interval_floor_sec}s)"
            )
            asserts.assert_less_equal(
                negotiated_max_interval_sec, max_interval_ceiling_sec,
                f"Negotiated MaxInterval ({negotiated_max_interval_sec}s) exceeded MaxIntervalCeiling ({max_interval_ceiling_sec}s)"
            )

            # Synchronously verify initial prime report was received during subscription establishment
            prime_attrs = subscription.GetAttributes()
            asserts.assert_in(endpoint, prime_attrs, "Endpoint 0 not found in subscription prime report")
            asserts.assert_in(Clusters.GeneralCommissioning, prime_attrs[endpoint],
                              "GeneralCommissioning cluster not found in subscription prime report")
            asserts.assert_in(target_attribute, prime_attrs[endpoint][Clusters.GeneralCommissioning],
                              "Breadcrumb attribute not found in subscription prime report")
            prime_val = prime_attrs[endpoint][Clusters.GeneralCommissioning][target_attribute]
            log.info("Initial prime report received: Breadcrumb = %r at timestamp %.6f", prime_val, time.monotonic())

            sub_elapsed_sec = time.monotonic() - t_prime
            remaining_sub_window_sec = float(negotiated_max_interval_sec) - sub_elapsed_sec

            log.info("Time elapsed since subscribe request: %.2f s; remaining window: %.2f s "
                     "(min delay window: %.2f s, worst-case MRP window: %.2f s)",
                     sub_elapsed_sec, remaining_sub_window_sec, min_delay_window_sec, min_required_sub_window_sec)

            asserts.assert_greater(
                remaining_sub_window_sec, min_delay_window_sec,
                f"Timing precondition failed: remaining subscription window ({remaining_sub_window_sec:.2f} s, "
                f"from negotiated MaxInterval={negotiated_max_interval_sec} s, subscribe elapsed={sub_elapsed_sec:.2f} s) "
                f"must exceed DelayMinMs + DelayJitterWindowMs ({min_delay_window_sec:.2f} s) so MaxInterval does not clamp DelayReportData"
            )

            if remaining_sub_window_sec < min_required_sub_window_sec:
                log.warning(
                    "Remaining subscription window (%.2f s, negotiated MaxInterval=%d s) is smaller than the worst-case "
                    "2x MRP retransmission window (%.2f s). Continuing test; if MRP retransmissions delay message delivery "
                    "past MaxInterval, a periodic report may interfere with the measurement.",
                    remaining_sub_window_sec, negotiated_max_interval_sec, min_required_sub_window_sec
                )

            # Step 3: Send InvokeRequestMessage for ArmFailSafe with DelayReportData
            self.step(3, "TH records start time t_invoke_sent and sends an InvokeRequestMessage to the DUT to invoke the ArmFailSafe command (ExpiryLengthSeconds set to 900, Breadcrumb set to 1) on the GeneralCommissioning cluster (Endpoint 0) with DelayReportData (DelayMinMs: 1000 ms, DelayJitterWindowMs: 1000 ms).")
            # Drain any residual queue items before measuring
            while not report_queue.empty():
                report_queue.get_nowait()

            cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=1)
            log.info("Sending InvokeRequestMessage for ArmFailSafe with DelayReportData (DelayMinMs=%d, DelayJitterWindowMs=%d)",
                     delay_min_ms, delay_jitter_window_ms)

            t_invoke_sent = time.monotonic()
            resp = await dev_ctrl.SendCommand(
                nodeId=dut_node_id,
                endpoint=endpoint,
                payload=cmd,
                delayReportData=ClusterCommand.DelayReportData(delayMinMs=delay_min_ms, delayJitterWindowMs=delay_jitter_window_ms),
            )
            asserts.assert_is_instance(resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse,
                                       "DUT response should be ArmFailSafeResponse")
            asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                                 f"ArmFailSafe failed with errorCode: {resp.errorCode}")
            failsafe_armed = True

            # Step 4: Measure delta t and verify delta t >= DelayMinMs
            self.step(4, "TH measures the time delta (Delta t) between sending the InvokeRequestMessage (t_invoke_sent) in Step 3 and receiving the subsequent ReportDataMessage containing the Breadcrumb attribute change report. Verify Delta t >= DelayMinMs (1000 ms).")
            try:
                report_recv_time, new_val = await asyncio.to_thread(report_queue.get, timeout=report_wait_timeout_sec)
            except queue.Empty:
                elapsed_since_prime_sec = time.monotonic() - t_prime
                max_interval_note = (
                    f" Negotiated MaxInterval ({negotiated_max_interval_sec}s) elapsed during the wait "
                    f"(elapsed since prime: {elapsed_since_prime_sec:.2f}s); MRP retransmissions on a periodic report "
                    f"may have delayed the attribute report."
                    if elapsed_since_prime_sec >= negotiated_max_interval_sec
                    else ""
                )
                asserts.fail(
                    f"Did not receive ReportDataMessage within {report_wait_timeout_sec:.2f}s following invoke command "
                    f"with DelayReportData.{max_interval_note}"
                )

            asserts.assert_equal(new_val, 1, f"Expected Breadcrumb report value to be 1, got {new_val}")

            delta_t_ms = (report_recv_time - t_invoke_sent) * 1000.0
            elapsed_since_prime_sec = report_recv_time - t_prime
            log.info("Measured Delta t between sending InvokeRequest (t_invoke_sent) and receiving ReportData: %.2f ms "
                     "(DelayMinMs=%d, Jitter=%d, elapsed since prime=%.2f s / MaxInterval=%d s)",
                     delta_t_ms, delay_min_ms, delay_jitter_window_ms, elapsed_since_prime_sec, negotiated_max_interval_sec)
            if elapsed_since_prime_sec >= negotiated_max_interval_sec:
                log.warning(
                    "Elapsed time since prime report (%.2f s) reached negotiated MaxInterval (%d s); "
                    "periodic ceiling timing overlapped the DelayReportData window.",
                    elapsed_since_prime_sec, negotiated_max_interval_sec
                )

            min_expected_delta_ms = float(delay_min_ms)
            asserts.assert_greater_equal(
                delta_t_ms, min_expected_delta_ms,
                f"Delta t ({delta_t_ms:.2f} ms) was less than {min_expected_delta_ms:.2f} ms minimum delay "
                f"(elapsed since prime: {elapsed_since_prime_sec:.2f} s, negotiated MaxInterval: {negotiated_max_interval_sec} s)"
            )

            # Step 5: Disarm fail-safe and reset Breadcrumb
            self.step(5, "TH sends an InvokeRequestMessage to the DUT to invoke the ArmFailSafe command (ExpiryLengthSeconds set to 0, Breadcrumb set to 0) to disarm the fail-safe and clean up.")
            log.info("Cleaning up: disarming fail-safe and resetting Breadcrumb to 0")
            cleanup_resp = await dev_ctrl.SendCommand(
                nodeId=dut_node_id,
                endpoint=endpoint,
                payload=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
            )
            asserts.assert_is_instance(cleanup_resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse,
                                       "DUT response should be ArmFailSafeResponse")
            asserts.assert_equal(cleanup_resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                                 f"ArmFailSafe cleanup failed with errorCode: {cleanup_resp.errorCode}")
            failsafe_armed = False
        finally:
            if subscription is not None:
                try:
                    subscription.Shutdown()
                except Exception:
                    log.exception("Cleanup: failed to shutdown subscription")

            if failsafe_armed:
                try:
                    await dev_ctrl.SendCommand(
                        nodeId=dut_node_id,
                        endpoint=endpoint,
                        payload=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
                    )
                except Exception:
                    log.exception("Cleanup: failed to disarm fail-safe during error recovery")


if __name__ == "__main__":
    default_matter_test_main()
