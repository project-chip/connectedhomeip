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
monitoring of CachedResults and NumCachedResults attributes.

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

from matter.clusters.Types import NullValue
from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

# How long to poll for a subscription update after making the ED commissionable/not
SUBSCRIPTION_POLL_TIMEOUT_SEC = 40


class TC_COMPRO_2_3(COMPROBaseTest):

    def desc_TC_COMPRO_2_3(self) -> str:
        return "TC-COMPRO-2.3: Proxy Background Scan feature functionality"

    def pics_TC_COMPRO_2_3(self) -> list[str]:
        return ["COMPRO.S", "COMPRO.S.F01"]

    def steps_TC_COMPRO_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "Ensure ED is NOT commissionable"),
            TestStep(3, "TH reads Transport and WiFiBand attributes",
                     "Store as valid_transports and valid_bands"),
            TestStep(4, "TH subscribes to CachedResults and NumCachedResults (min=0, max=30 s)",
                     "Subscription established successfully"),
            TestStep(5, "TH sends ProxyBackGroundScanStartRequest(Transport, Timeout=0, WiFiBands)",
                     "DUT responds with SUCCESS"),
            TestStep(6, "TH monitors subscription for 40 s while ED is NOT commissionable",
                     "NumCachedResults remains 0; CachedResults remains null/empty"),
            TestStep(7, "Ensure ED IS commissionable"),
            TestStep(8, "TH monitors subscription until NumCachedResults >= 1 (up to 40 s)",
                     "NumCachedResults >= 1; CachedResults has at least one entry with "
                     "non-zero Transport, valid Discriminator, VendorID, ProductID fields"),
            TestStep(9, "TH reads CacheTimeout attribute", "Store as cache_timeout"),
            TestStep(10, "Ensure ED is NOT commissionable"),
            TestStep(11, "TH monitors subscription until CachedResults clears (up to cache_timeout + 10 s)",
                     "NumCachedResults drops to 0; CachedResults becomes null or empty"),
            TestStep(12, "TH sends ProxyBackGroundScanStopRequest(Transport, WiFiBands)",
                     "DUT responds with SUCCESS"),
            TestStep(13, "TH terminates subscription"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_3(self):
        cp = self.cp

        # Step 1 — commissioning done by framework
        self.step(1)

        # Guard: skip entire test if BackgroundScan feature not present
        feature_map = await self.read_feature_map()
        if not self.has_feature_bgs(feature_map):
            logger.info("BackgroundScan feature not supported — skipping TC-COMPRO-2.3")
            self.skip_all_remaining_steps(2)
            return

        has_wi = self.has_feature_wi(feature_map)
        ed = self._ed_fixture_from_params()

        # Step 2 — ensure ED not commissionable
        self.step(2)
        await self.ensure_ed_not_commissionable(ed)

        # Step 3 — read transport/band attributes
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("valid_transports = 0x%02x", valid_transports)
        valid_bands: int = 0
        if has_wi:
            valid_bands = await self.read_wifi_band()
            logger.info("valid_bands = 0x%04x", valid_bands)

        # Step 4 — subscribe to CachedResults and NumCachedResults
        self.step(4)
        subscription = await self.default_controller.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[
                (self.cp_endpoint, cp.Attributes.NumCachedResults),
                (self.cp_endpoint, cp.Attributes.CachedResults),
            ],
            reportInterval=(0, 30),
            keepSubscriptions=False,
        )
        logger.info("Subscription to NumCachedResults and CachedResults established")

        # Step 5 — send ProxyBackGroundScanStartRequest
        self.step(5)
        start_cmd = cp.Commands.ProxyBackGroundScanStartRequest(
            transport=valid_transports,
            timeout=0,   # 0 = infinite
            wiFiBands=valid_bands if has_wi else None,
        )
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=start_cmd,
        )
        logger.info("ProxyBackGroundScanStartRequest sent successfully")

        # Step 6 — confirm cache stays empty while ED not commissionable
        self.step(6)
        await asyncio.sleep(SUBSCRIPTION_POLL_TIMEOUT_SEC)
        # Read current attribute values directly to verify they haven't changed
        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        logger.info("After %d s (no ED): NumCachedResults=%d CachedResults=%s",
                    SUBSCRIPTION_POLL_TIMEOUT_SEC, num_cached, cached_results)
        asserts.assert_equal(num_cached, 0,
                             "NumCachedResults must be 0 when no ED is commissionable")
        is_null_or_empty = (cached_results is NullValue) or (cached_results == [])
        asserts.assert_true(is_null_or_empty,
                            "CachedResults must be null/empty when no ED is commissionable")

        # Step 7 — make ED commissionable
        self.step(7)
        await self.ensure_ed_commissionable(ed)

        # Step 8 — wait for cache to fill
        self.step(8)
        logger.info("Waiting up to %d s for CachedResults to populate...", SUBSCRIPTION_POLL_TIMEOUT_SEC)
        await self._wait_for_num_cached_results_nonzero(timeout_sec=SUBSCRIPTION_POLL_TIMEOUT_SEC)

        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        logger.info("NumCachedResults=%d", num_cached)
        asserts.assert_greater_equal(num_cached, 1,
                                     "NumCachedResults must be >= 1 after ED becomes commissionable")
        asserts.assert_not_equal(cached_results, NullValue,
                                 "CachedResults must not be null after ED becomes commissionable")
        asserts.assert_greater_equal(len(cached_results), 1,
                                     "CachedResults must have at least one entry")

        # Validate first result entry
        result = cached_results[0]
        asserts.assert_not_equal(result.transport, 0,
                                 "CachedResult entry Transport must be non-zero")
        asserts.assert_less_equal(result.discriminator, 4095,
                                  "CachedResult entry Discriminator must be <= 4095")
        if has_wi:
            asserts.assert_is_not_none(result.wiFiBand,
                                       "CachedResult entry WiFiBand must be present when WI feature active")

        # Step 9 — read CacheTimeout
        self.step(9)
        cache_timeout = await self.read_cache_timeout()
        logger.info("CacheTimeout = %d s", cache_timeout)

        # Step 10 — make ED not commissionable (triggers TTL expiry)
        self.step(10)
        await self.ensure_ed_not_commissionable(ed)

        # Step 11 — wait for cache to expire
        self.step(11)
        expiry_deadline = cache_timeout + 10
        logger.info("Waiting up to %d s for CachedResults to clear...", expiry_deadline)
        await self._wait_for_cache_to_clear(timeout_sec=expiry_deadline)

        num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
        cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
        logger.info("NumCachedResults=%d after cache expiry", num_cached)
        asserts.assert_equal(num_cached, 0,
                             "NumCachedResults must return to 0 after cache timeout")
        is_null_or_empty = (cached_results is NullValue) or (cached_results == [])
        asserts.assert_true(is_null_or_empty,
                            "CachedResults must be null/empty after cache timeout")

        # Step 12 — stop background scan
        self.step(12)
        stop_cmd = cp.Commands.ProxyBackGroundScanStopRequest(
            transport=valid_transports,
            wiFiBands=valid_bands if has_wi else None,
        )
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=stop_cmd,
        )
        logger.info("ProxyBackGroundScanStopRequest sent successfully")

        # Step 13 — terminate subscription
        self.step(13)
        subscription.Shutdown()
        logger.info("Subscription shut down")

    # ------------------------------------------------------------------
    # Internal polling helpers
    # ------------------------------------------------------------------

    async def _wait_for_num_cached_results_nonzero(self, timeout_sec: int):
        """Poll NumCachedResults until it is >= 1 or timeout expires."""
        cp = self.cp
        poll_interval = 2
        elapsed = 0
        while elapsed < timeout_sec:
            val = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
            if val >= 1:
                logger.info("NumCachedResults = %d after %d s", val, elapsed)
                return
            await asyncio.sleep(poll_interval)
            elapsed += poll_interval
        asserts.fail(f"NumCachedResults did not become >= 1 within {timeout_sec} s")

    async def _wait_for_cache_to_clear(self, timeout_sec: int):
        """Poll NumCachedResults until it is 0 or timeout expires."""
        cp = self.cp
        poll_interval = 2
        elapsed = 0
        while elapsed < timeout_sec:
            val = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
            if val == 0:
                logger.info("NumCachedResults cleared after %d s", elapsed)
                return
            await asyncio.sleep(poll_interval)
            elapsed += poll_interval
        asserts.fail(f"NumCachedResults did not clear within {timeout_sec} s")


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
