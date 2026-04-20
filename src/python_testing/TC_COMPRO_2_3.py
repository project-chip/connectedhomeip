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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${COMMISSIONING_PROXY_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --string-arg ed_app_path:${ED_APP}
#       --int-arg ed_discriminator:3841 ed_passcode:20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""TC-COMPRO-2.3 — Commissioning Proxy cluster: Background Scan feature functionality.

Tests ProxyBackGroundScanStartRequest / ProxyBackGroundScanStopRequest with subscription
monitoring of CachedResults and NumCachedResults attributes.  Also covers the Timeout
parameter and negative transport/band validation.

Test plan reference: TC-COMPRO-2.3
PICS requirement: COMPRO.S.F01 (BackgroundScan feature)

Example usage:
    ```bash
    python3 TC_COMPRO_2_3.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path /path/to/paa-trust-store \\
        --endpoint 1 \\
        --string-arg ed_app_path:/path/to/ed-app ed_ssh_host:192.168.1.10 \\
        --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --int-arg ed_discriminator:3841 ed_passcode:20202021
    ```
    If ``ed_app_path`` is omitted the test pauses and prompts the operator to
    make the ED commissionable/not commissionable at each relevant step.
"""

import asyncio
import logging
import queue
import time

from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_COMPRO_2_3(COMPROBaseTest):

    @property
    def default_timeout(self) -> int:
        # Step 7 sleeps 40 s; step 12 waits up to cache_timeout (≤120 s) + 10 s;
        # steps 9/14 poll up to 40 s each; step 22 sleeps 20 s.
        # Worst-case sequential total ≈ 270 s; 360 s gives comfortable headroom.
        return 360

    def desc_TC_COMPRO_2_3(self) -> str:
        return "[TC-COMPRO-2.3] Proxy Background Scan feature functionality"

    def pics_TC_COMPRO_2_3(self) -> list[str]:
        return ["COMPRO.S", "COMPRO.S.F01"]

    def steps_TC_COMPRO_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "Ensure ED is NOT commissionable"),
            TestStep(3, "TH reads Transport attribute",
                     "Store as valid_transports"),
            TestStep(4, "TH reads WiFiBand attribute (if WI supported)",
                     "Store as valid_bands"),
            TestStep(5, "TH subscribes to CachedResults and NumCachedResults (min=0, max=30 s)",
                     "Subscription established successfully"),
            TestStep(6, "TH sends ProxyBackGroundScanStartRequest(Transport=valid_transports, "
                     "Timeout=0, WiFiBands=valid_bands if WI supported)",
                     "DUT responds with SUCCESS"),
            TestStep(7, "TH monitors for 40 s while ED is NOT commissionable",
                     "NumCachedResults == 0; CachedResults is null"),
            TestStep(8, "Ensure ED IS commissionable"),
            TestStep(9, "TH monitors until NumCachedResults >= 1 (up to 40 s)",
                     "NumCachedResults >= 1; CachedResults has at least one entry with "
                     "non-zero Transport, valid Discriminator, VendorID, ProductID"),
            TestStep(10, "TH reads CacheTimeout attribute",
                     "Store as cache_timeout"),
            TestStep(11, "Ensure ED is NOT commissionable"),
            TestStep(12, "TH monitors until cache clears (up to cache_timeout + 10 s)",
                     "NumCachedResults drops to 0; CachedResults becomes null"),
            TestStep(13, "Ensure ED IS commissionable again"),
            TestStep(14, "TH monitors until NumCachedResults >= 1 again (up to 40 s)",
                     "NumCachedResults >= 1 (ED re-appears in background scan cache)"),
            TestStep(15, "TH sends ProxyBackGroundScanStopRequest(Transport=valid_transports, "
                     "WiFiBands=valid_bands if WI supported)",
                     "DUT responds with SUCCESS"),
            TestStep(16, "TH reads NumCachedResults and CachedResults immediately after stop",
                     "NumCachedResults == 0; CachedResults is null (proxy clears cache on stop)"),
            TestStep(17, "TH sends ProxyBackGroundScanStopRequest with a transport not in the "
                     "original start request (if a valid alternative transport is available)",
                     "DUT returns SUCCESS (stopping a transport with no active scan is not an error)"),
            TestStep(18, "TH terminates subscriptions"),
            TestStep(19, "TH sends ProxyBackGroundScanStartRequest with an unsupported Transport bit",
                     "DUT returns INVALID_TRANSPORT_TYPE"),
            TestStep(20, "TH sends ProxyBackGroundScanStartRequest with WiFiPAF transport and a "
                     "WiFiBand not in valid_bands (if WI supported)",
                     "DUT returns INVALID_TRANSPORT_TYPE"),
            TestStep(21, "TH sends ProxyBackGroundScanStartRequest with Timeout=10",
                     "DUT responds with SUCCESS"),
            TestStep(22, "TH verifies NumCachedResults == 0 after 20 s",
                     "NumCachedResults == 0; CachedResults is null "
                     "(background scan auto-stopped after Timeout)"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_3(self):
        cp = self.cp

        # Step 1 — commissioning done by framework
        self.step(1)

        # Guard: skip entire test if BackgroundScan feature not present.
        # (PICS COMPRO.S.F01 also guards this, but we check explicitly so the
        # test is self-contained when run without a PICS file.)
        feature_map = await self.read_feature_map()
        if not self.has_feature_bgs(feature_map):
            logger.info("BackgroundScan feature not supported — skipping TC-COMPRO-2.3")
            self.skip_remaining_steps(2)
            return
        has_wi = self.has_feature_wi(feature_map)
        ed = self._ed_fixture_from_params()

        # Step 2 — ensure ED not commissionable
        self.step(2)
        await self.ensure_ed_not_commissionable(ed)

        # Step 3 — read Transport attribute
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("valid_transports = 0x%02x", valid_transports)

        # Step 4 — read WiFiBand (conditional on WI feature).
        # Read feature_map before this step so we know whether to run or skip.
        valid_bands: int = 0
        if has_wi:
            self.step(4)
            valid_bands = await self.read_wifi_band()
            logger.info("valid_bands = 0x%04x", valid_bands)
        else:
            self.skip_step(4)

        # Step 5 — subscribe to CachedResults and NumCachedResults.
        # AttributeSubscriptionHandler is the standard SDK class for subscription-based
        # monitoring (matter.testing.event_attribute_reporting).  start() calls
        # ReadAttribute with reportInterval and wires SetAttributeUpdateCallback
        # internally; reports are pushed by the DUT and queued in handler.attribute_queue.
        self.step(5)
        handler = AttributeSubscriptionHandler(expected_cluster=cp)
        await handler.start(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.cp_endpoint,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False,
        )
        logger.info("Subscription established")

        # ------------------------------------------------------------------
        # Local helpers — closed over handler / cp.
        # handler.attribute_queue holds AttributeValue(endpoint_id, attribute, value)
        # items pushed by the DUT.  queue.Queue is thread-safe; blocking get() is the
        # standard SDK pattern (see event_attribute_reporting.py, TC_RR_1_1.py).
        # ------------------------------------------------------------------

        def _monitor_sub_empty(duration_sec: int):
            """Drain reports for duration_sec s; fail if NumCachedResults ever != 0."""
            deadline = time.time() + duration_sec
            while True:
                remaining = deadline - time.time()
                if remaining <= 0:
                    break
                try:
                    item = handler.attribute_queue.get(block=True, timeout=remaining)
                    if item.attribute == cp.Attributes.NumCachedResults:
                        logger.info("Sub report (step 7): NumCachedResults=%d", item.value)
                        asserts.assert_equal(
                            item.value, 0,
                            f"NumCachedResults={item.value} while ED is not commissionable")
                except queue.Empty:
                    break  # No report within the window — correct

        def _wait_for_num_cached(condition_fn, timeout_sec: int, label: str):
            """Block until a NumCachedResults report satisfies condition_fn."""
            deadline = time.time() + timeout_sec
            while True:
                remaining = deadline - time.time()
                if remaining <= 0:
                    asserts.fail(f"{label}: NumCachedResults condition not met "
                                 f"within {timeout_sec} s")
                try:
                    item = handler.attribute_queue.get(block=True, timeout=remaining)
                    if item.attribute == cp.Attributes.NumCachedResults:
                        logger.info("Sub report (%s): NumCachedResults=%d", label, item.value)
                        if condition_fn(item.value):
                            return item.value
                except queue.Empty:
                    asserts.fail(f"{label}: NumCachedResults condition not met "
                                 f"within {timeout_sec} s")

        # Step 6 — send ProxyBackGroundScanStartRequest (Timeout=0: infinite)
        self.step(6)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyBackGroundScanStartRequest(
                transport=valid_transports,
                timeout=0,
                wiFiBands=valid_bands if has_wi else None,
            ),
        )
        logger.info("ProxyBackGroundScanStartRequest sent (Timeout=0, infinite)")

        # Step 7 — verify cache stays empty for 40 s via subscription monitoring.
        # Any subscription report with NumCachedResults != 0 fails the step immediately.
        # A direct read at the end confirms the final state.
        self.step(7)
        logger.info("Monitoring subscription for 40 s (ED not commissionable)...")
        _monitor_sub_empty(40)
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        logger.info("After 40 s (no ED): NumCachedResults=%d", num_cached)
        asserts.assert_equal(num_cached, 0,
                             "NumCachedResults must be 0 when no ED is commissionable")
        asserts.assert_true(
            (cached_results is NullValue) or (cached_results == []),
            "CachedResults must be null when no ED is commissionable")

        # Step 8 — make ED commissionable
        self.step(8)
        await self.ensure_ed_commissionable(ed)

        # Step 9 — wait for background scan cache to populate via subscription.
        # The subscription report signals the exact moment the DUT updates the cache.
        self.step(9)
        logger.info("Waiting up to 40 s for NumCachedResults >= 1 via subscription...")
        _wait_for_num_cached(lambda v: v >= 1, 40, "step 9")
        # Direct read for detailed entry validation (NumCachedResults + CachedResults).
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        asserts.assert_greater_equal(num_cached, 1,
                                     "NumCachedResults must be >= 1 after ED becomes commissionable")
        asserts.assert_not_equal(cached_results, NullValue,
                                 "CachedResults must not be null after ED becomes commissionable")
        asserts.assert_greater_equal(len(cached_results), 1,
                                     "CachedResults must have at least one entry")
        result = cached_results[0]
        asserts.assert_not_equal(result.transport, 0,
                                 "CachedResult entry Transport must be non-zero")
        asserts.assert_less_equal(result.discriminator, 4095,
                                  "CachedResult entry Discriminator must be <= 4095")
        if has_wi:
            asserts.assert_is_not_none(result.wiFiBand,
                                       "CachedResult entry WiFiBand must be present when WI feature active")

        # Step 10 — read CacheTimeout
        self.step(10)
        cache_timeout = await self.read_cache_timeout()
        logger.info("CacheTimeout = %d s", cache_timeout)

        # Step 11 — make ED not commissionable to trigger TTL expiry
        self.step(11)
        await self.ensure_ed_not_commissionable(ed)

        # Step 12 — wait for cache to expire via subscription.
        # We receive the moment the DUT drops NumCachedResults to 0.
        self.step(12)
        expiry_deadline = cache_timeout + 10
        logger.info("Waiting up to %d s for cache to expire via subscription...", expiry_deadline)
        _wait_for_num_cached(lambda v: v == 0, expiry_deadline, "step 12")
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        asserts.assert_equal(num_cached, 0, "NumCachedResults must be 0 after cache timeout")
        asserts.assert_true(
            (cached_results is NullValue) or (cached_results == []),
            "CachedResults must be null after cache timeout")

        # Step 13 — make ED commissionable again to verify continuous background scan
        self.step(13)
        await self.ensure_ed_commissionable(ed)

        # Step 14 — wait for cache to re-populate via subscription
        self.step(14)
        logger.info("Waiting up to 40 s for NumCachedResults >= 1 (ED re-appears)...")
        _wait_for_num_cached(lambda v: v >= 1, 40, "step 14")
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        asserts.assert_greater_equal(num_cached, 1,
                                     "NumCachedResults must be >= 1 after ED re-appears")

        # Step 15 — stop the background scan
        self.step(15)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyBackGroundScanStopRequest(
                transport=valid_transports,
                wiFiBands=valid_bands if has_wi else None,
            ),
        )
        logger.info("ProxyBackGroundScanStopRequest sent")

        # Step 16 — verify cache clears immediately after stop
        self.step(16)
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        asserts.assert_equal(num_cached, 0,
                             "NumCachedResults must be 0 immediately after stop")
        asserts.assert_true(
            (cached_results is NullValue) or (cached_results == []),
            "CachedResults must be null immediately after stop")

        # Step 17 — stop with a transport not in the original start request.
        # Requires a valid (supported) transport that was absent from the step-6 start.
        # Since the start used all of valid_transports, this step is only exercisable
        # when multiple transport types are supported simultaneously (e.g., both kWiFiPAF
        # and kBle appear in valid_transports).  Skip until that is the case.
        self.skip_step(17)

        # Step 18 — terminate subscription
        self.step(18)
        handler.cancel()
        logger.info("Subscription cancelled")

        # Step 19 — negative: unsupported transport → INVALID_TRANSPORT_TYPE.
        # Find the first defined transport bit absent from valid_transports.
        # The supported set is derived from feature flags (same as the Transport attribute);
        # kBle will pass here once Feature::kBleInterface is enabled.
        defined_transports = [
            int(cp.Bitmaps.CapabilitiesBitmap.kBle),
            int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF),
        ]
        unsupported_transport = next(
            (b for b in defined_transports if not (valid_transports & b)), None)
        if unsupported_transport is None:
            self.skip_step(19)
            logger.info("Step 19 skipped: all defined transport bits are in valid_transports")
        else:
            self.step(19)
            logger.info("Using unsupported transport bit 0x%02x for negative test", unsupported_transport)
            try:
                await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyBackGroundScanStartRequest(
                        transport=unsupported_transport,
                        timeout=0,
                    ),
                    interactionTimeoutMs=10000,
                )
                asserts.fail("Expected INVALID_TRANSPORT_TYPE but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidTransportType,
                                     f"Expected INVALID_TRANSPORT_TYPE, got {e.status}")
                logger.info("Got expected INVALID_TRANSPORT_TYPE for unsupported transport")

        # Step 20 — negative: WI feature + band not in valid_bands → INVALID_TRANSPORT_TYPE.
        # Try k5g first (common case: proxy only supports 2.4 GHz).
        if not has_wi:
            self.skip_step(20)
        else:
            defined_bands = [
                int(cp.Bitmaps.WiFiBandBitmap.k5g),
                int(cp.Bitmaps.WiFiBandBitmap.k2g4),
            ]
            invalid_band = next((b for b in defined_bands if not (valid_bands & b)), None)
            if invalid_band is None:
                self.skip_step(20)
                logger.info("Step 20 skipped: all defined WiFiBand bits are in valid_bands")
            else:
                self.step(20)
                logger.info("Using invalid WiFiBand bit 0x%04x for negative test", invalid_band)
                try:
                    await self.default_controller.SendCommand(
                        nodeId=self.dut_node_id,
                        endpoint=self.cp_endpoint,
                        payload=cp.Commands.ProxyBackGroundScanStartRequest(
                            transport=valid_transports,
                            timeout=0,
                            wiFiBands=invalid_band,
                        ),
                        interactionTimeoutMs=10000,
                    )
                    asserts.fail("Expected INVALID_TRANSPORT_TYPE but command succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.InvalidTransportType,
                                         f"Expected INVALID_TRANSPORT_TYPE, got {e.status}")
                    logger.info("Got expected INVALID_TRANSPORT_TYPE for invalid WiFiBand")

        # Step 21 — start background scan with Timeout=10 (auto-stops after 10 s).
        # The ED is still commissionable from step 13; results may populate then
        # auto-clear when the Timeout fires, testing both the cache-fill and
        # cache-clear-on-stop paths in a single window.
        self.step(21)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyBackGroundScanStartRequest(
                transport=valid_transports,
                timeout=10,
                wiFiBands=valid_bands if has_wi else None,
            ),
        )
        logger.info("ProxyBackGroundScanStartRequest sent (Timeout=10 s)")

        # Step 22 — after 20 s (2× Timeout) the scan must have auto-stopped and the
        # cache must be clear.
        self.step(22)
        logger.info("Waiting 20 s for Timeout to fire and cache to clear...")
        await asyncio.sleep(20)
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        asserts.assert_equal(num_cached, 0,
                             "NumCachedResults must be 0 after Timeout expired "
                             "(background scan auto-stopped)")
        asserts.assert_true(
            (cached_results is NullValue) or (cached_results == []),
            "CachedResults must be null after Timeout expired")

        # Cleanup
        await self.ensure_ed_not_commissionable(ed)



if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
