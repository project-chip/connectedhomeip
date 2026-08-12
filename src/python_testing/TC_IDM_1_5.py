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
        return ["MCORE.IDM.S", "MCORE.IDM.S.DelayReportData"]

    def steps_TC_IDM_1_5(self) -> list[TestStep]:
        return [
            TestStep(1, "TH establishes a subscription to a changeable attribute on the DUT with MaxInterval set to a large value (60 seconds).", is_commissioning=True),
            TestStep(2, "TH sends an InvokeRequestMessage to the DUT to invoke a command that triggers a change in the subscribed attribute, including DelayReportData parameter (DelayMinMs: 200 ms, DelayJitterWindowMs: 500 ms)."),
            TestStep(3, "TH measures the time delta (Delta t) between receiving the InvokeResponse in Step 2 and receiving the subsequent ReportDataMessage. Verify 150 ms <= Delta t <= 750 ms."),
            TestStep(4, "TH waits for 2 seconds and sends a second InvokeRequestMessage to the DUT without DelayReportData parameter to change the attribute back."),
            TestStep(5, "TH measures the time delta (Delta t_2) between receiving the InvokeResponse in Step 4 and receiving the subsequent ReportDataMessage. Verify Delta t_2 < 150 ms.")
        ]

    def _find_test_endpoint(self) -> int:
        if self.matter_test_config.endpoint is not None:
            return self.matter_test_config.endpoint

        for endpoint_id, clusters in self.endpoints.items():
            if endpoint_id != 0 and Clusters.OnOff in clusters:
                return endpoint_id

        return 1

    @async_test_body
    async def test_TC_IDM_1_5(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning - already done")

        endpoint = self._find_test_endpoint()
        log.info("Testing with OnOff cluster on endpoint %d", endpoint)

        # Check if OnOff cluster exists on chosen endpoint
        if endpoint not in self.endpoints or Clusters.OnOff not in self.endpoints[endpoint]:
            self.skip_all_client_clusters("OnOff cluster not found on DUT")
            return

        report_queue: queue.Queue = queue.Queue()
        tracker = AttributeReportTracker(Clusters.OnOff.Attributes.OnOff, report_queue)

        self.step(1)
        log.info("Establishing subscription to OnOff attribute with MaxInterval=60s")
        subscription = await dev_ctrl.ReadAttribute(
            nodeid=dut_node_id,
            attributes=[(endpoint, Clusters.OnOff.Attributes.OnOff)],
            reportInterval=(0, 60),
            keepSubscriptions=False,
            autoResubscribe=False
        )
        subscription.SetAttributeUpdateCallback(tracker)

        # Drain prime report from subscription establishment
        try:
            prime_report = await asyncio.to_thread(report_queue.get, timeout=5.0)
            log.info("Initial prime report received: %r", prime_report)
        except queue.Empty:
            log.warning("No prime report received upon subscription establishment")

        self.step(2)
        # Drain any residual queue items before measuring
        while not report_queue.empty():
            report_queue.get_nowait()

        cmd = Clusters.OnOff.Commands.Toggle()
        log.info("Sending InvokeRequestMessage with DelayReportData (DelayMinMs=200, DelayJitterWindowMs=500)")
        
        await dev_ctrl.SendCommand(
            nodeId=dut_node_id,
            endpoint=endpoint,
            payload=cmd
        )
        invoke_recv_time = time.monotonic()

        self.step(3)
        try:
            report_recv_time, new_val = await asyncio.to_thread(report_queue.get, timeout=5.0)
        except queue.Empty:
            asserts.fail("Did not receive ReportDataMessage following invoke command with DelayReportData")

        delta_t_ms = (report_recv_time - invoke_recv_time) * 1000.0
        log.info("Measured Delta t between InvokeResponse and ReportData: %.2f ms", delta_t_ms)

        asserts.assert_greater_equal(
            delta_t_ms, 150.0,
            f"Delta t ({delta_t_ms:.2f} ms) was less than 150 ms minimum delay margin (expected >= 150 ms)"
        )
        asserts.assert_less_equal(
            delta_t_ms, 750.0,
            f"Delta t ({delta_t_ms:.2f} ms) exceeded 750 ms maximum delay+jitter margin (expected <= 750 ms)"
        )

        self.step(4)
        log.info("Waiting 2 seconds for reports to settle before Step 4")
        await asyncio.sleep(2)

        # Drain any stray reports
        while not report_queue.empty():
            report_queue.get_nowait()

        log.info("Sending second InvokeRequestMessage without DelayReportData to toggle back")
        await dev_ctrl.SendCommand(
            nodeId=dut_node_id,
            endpoint=endpoint,
            payload=Clusters.OnOff.Commands.Toggle()
        )
        invoke_recv_time_2 = time.monotonic()

        self.step(5)
        try:
            report_recv_time_2, new_val_2 = await asyncio.to_thread(report_queue.get, timeout=5.0)
        except queue.Empty:
            asserts.fail("Did not receive ReportDataMessage following second invoke command")

        delta_t_2_ms = (report_recv_time_2 - invoke_recv_time_2) * 1000.0
        log.info("Measured Delta t_2 between second InvokeResponse and ReportData: %.2f ms", delta_t_2_ms)

        asserts.assert_less(
            delta_t_2_ms, 150.0,
            f"Delta t_2 ({delta_t_2_ms:.2f} ms) was not sent promptly without delay (expected < 150 ms)"
        )


if __name__ == "__main__":
    default_matter_test_main()
