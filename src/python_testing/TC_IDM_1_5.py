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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
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

from mobly import asserts
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.clusters import Command as ClusterCommand
from matter.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class AttributeReportTracker:
    """Callback tracker that records received attribute reports with monotonic timestamps."""

    def __init__(self, expected_attribute, output_queue: queue.Queue):
        self._expected_attribute = expected_attribute
        self._output_queue = output_queue

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        if path.AttributeType == self._expected_attribute:
            recv_time = time.monotonic()
            val = transaction.GetAttribute(path)
            log.info("Received attribute report for %s: %r at timestamp %.6f", path.AttributeType, val, recv_time)
            self._output_queue.put((recv_time, val))


class TC_IDM_1_5(IDMBaseTest):
    """[TC-IDM-1.5] DelayReportData Invoke Request Action [DUT as Server]"""

    def desc_TC_IDM_1_5(self) -> str:
        return "[TC-IDM-1.5] DelayReportData Invoke Request Action [DUT as Server]"

    def pics_TC_IDM_1_5(self) -> list[str]:
        return ["MCORE.IDM.S"]

    def steps_TC_IDM_1_5(self) -> list[TestStep]:
        return [
            TestStep(1, "TH establishes a subscription to a changeable attribute on the DUT with MaxInterval set to a large value (3600 seconds).", is_commissioning=True),
            TestStep(2, "TH records start time t_invoke_sent and sends an InvokeRequestMessage to the DUT to invoke a command that triggers a change in the subscribed attribute, including DelayReportData parameter (DelayMinMs: 1000 ms, DelayJitterWindowMs: 1000 ms)."),
            TestStep(3, "TH measures the time delta (Delta t) between sending the InvokeRequestMessage (t_invoke_sent) in Step 2 and receiving the subsequent ReportDataMessage. Verify Delta t >= DelayMinMs (1000 ms)."),
            TestStep(4, "TH sends an InvokeRequestMessage to the DUT to invoke the ArmFailSafe command (ExpiryLengthSeconds set to 0, Breadcrumb set to 0) to disarm the fail-safe and clean up.")
        ]

    @async_test_body
    async def test_TC_IDM_1_5(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning - already done")

        # DUT must be a non-ICD device (!ICDM.S)
        if self.check_pics("ICDM.S"):
            log.info("DUT is an ICD device (ICDM.S is enabled); skipping test as it requires !ICDM.S")
            self.skip_step(1)
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            return

        # GeneralCommissioning cluster is mandatory on Endpoint 0 for all Matter devices
        endpoint = 0
        target_attribute = Clusters.GeneralCommissioning.Attributes.Breadcrumb

        report_queue: queue.Queue = queue.Queue()
        tracker = AttributeReportTracker(target_attribute, report_queue)

        self.step(1)
        log.info("Establishing subscription to Breadcrumb attribute on Endpoint 0 with MaxInterval=3600s")
        sub_start_time = time.monotonic()
        subscription = await dev_ctrl.ReadAttribute(
            nodeid=dut_node_id,
            attributes=[(endpoint, target_attribute)],
            reportInterval=(0, 3600),
            keepSubscriptions=False,
            autoResubscribe=False
        )
        subscription.SetAttributeUpdateCallback(tracker)

        # Synchronously await and drain prime report from subscription establishment
        try:
            prime_report = await asyncio.to_thread(report_queue.get, timeout=5.0)
            log.info("Initial prime report received: %r", prime_report)
        except queue.Empty:
            asserts.fail("No prime report received upon subscription establishment")

        # Give DUT 1.0s to process StatusResponse ACK and enter quiet active state
        log.info("Waiting 1 second for subscription to settle before Step 2")
        await asyncio.sleep(1.0)

        sub_elapsed_sec = time.monotonic() - sub_start_time
        remaining_sub_window_sec = 3600.0 - sub_elapsed_sec

        delay_min_ms = 1000
        delay_jitter_window_ms = 1000
        min_required_sub_window_sec = (delay_min_ms + delay_jitter_window_ms) / 1000.0 + 5.0

        log.info("Subscription established in %.2f s; remaining window: %.2f s (required >= %.2f s)",
                 sub_elapsed_sec, remaining_sub_window_sec, min_required_sub_window_sec)

        asserts.assert_greater_equal(
            remaining_sub_window_sec, min_required_sub_window_sec,
            f"Subscription setup took too long ({sub_elapsed_sec:.2f} s); not enough time remaining "
            f"({remaining_sub_window_sec:.2f} s) to safely measure DelayReportData"
        )

        self.step(2)
        # Drain any residual queue items before measuring
        while not report_queue.empty():
            report_queue.get_nowait()

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=1)
        log.info("Sending InvokeRequestMessage with DelayReportData (DelayMinMs=%d, DelayJitterWindowMs=%d)",
                 delay_min_ms, delay_jitter_window_ms)

        invoke_sent_time = time.monotonic()
        await dev_ctrl.SendCommand(
            nodeId=dut_node_id,
            endpoint=endpoint,
            payload=cmd,
            delayReportData=ClusterCommand.DelayReportData(delayMinMs=delay_min_ms, delayJitterWindowMs=delay_jitter_window_ms),
        )

        self.step(3)
        wait_timeout_sec = (delay_min_ms + delay_jitter_window_ms) / 1000.0 + 5.0
        try:
            report_recv_time, new_val = await asyncio.to_thread(report_queue.get, timeout=wait_timeout_sec)
        except queue.Empty:
            asserts.fail(f"Did not receive ReportDataMessage within {wait_timeout_sec:.2f}s following invoke command with DelayReportData")

        delta_t_ms = (report_recv_time - invoke_sent_time) * 1000.0
        log.info("Measured Delta t between sending InvokeRequest and receiving ReportData: %.2f ms (DelayMinMs=%d, Jitter=%d)",
                 delta_t_ms, delay_min_ms, delay_jitter_window_ms)

        min_expected_delta_ms = float(delay_min_ms)

        asserts.assert_greater_equal(
            delta_t_ms, min_expected_delta_ms,
            f"Delta t ({delta_t_ms:.2f} ms) was less than {min_expected_delta_ms:.2f} ms minimum delay"
        )

        self.step(4)
        # Cleanup: Disarm fail-safe and reset Breadcrumb
        log.info("Cleaning up: disarming fail-safe and resetting Breadcrumb to 0")
        await dev_ctrl.SendCommand(
            nodeId=dut_node_id,
            endpoint=endpoint,
            payload=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )


if __name__ == "__main__":
    default_matter_test_main()
