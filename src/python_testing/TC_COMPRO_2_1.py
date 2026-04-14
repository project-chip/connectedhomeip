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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""TC-COMPRO-2.1 — Commissioning Proxy cluster: Attributes with DUT as Server.

Verifies that all mandatory and conditional attributes of the Commissioning Proxy
cluster are present and within spec-defined constraints.

Test plan reference: TC-COMPRO-2.1 (PROVISIONAL)

Example usage:
    ```bash
    python3 TC_COMPRO_2_1.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path /path/to/paa-trust-store \\
        --endpoint 1
    ```
"""

import logging

from matter.clusters.Types import NullValue
from mobly import asserts
from support_modules.compro_support import COMPRO_ENDPOINT, COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_COMPRO_2_1(COMPROBaseTest):

    def desc_TC_COMPRO_2_1(self) -> str:
        return "TC-COMPRO-2.1 [PROVISIONAL]: Attributes with DUT as Server"

    def pics_TC_COMPRO_2_1(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute",
                     "DUT returns a valid FeatureMap bitmap"),
            TestStep(3, "TH reads Transport attribute",
                     "DUT returns a CapabilitiesBitmap value with at least one bit set"),
            TestStep(4, "TH reads ScanMaxTime attribute",
                     "DUT returns a uint8 value >= 1"),
            TestStep(5, "TH reads MaxSessions attribute",
                     "DUT returns a uint8 value >= 1"),
            TestStep(6, "TH reads MaxCachedResults attribute (if BGS supported)",
                     "DUT returns a uint8 value >= 1"),
            TestStep(7, "TH reads NumCachedResults attribute (if BGS supported)",
                     "DUT returns a uint8 value of 0 at startup"),
            TestStep(8, "TH reads CacheTimeout attribute (if BGS supported)",
                     "DUT returns a uint16 value >= 1"),
            TestStep(9, "TH reads CachedResults attribute (if BGS supported)",
                     "DUT returns null or an empty list"),
            TestStep(10, "TH reads WiFiBand attribute (if WI supported)",
                     "DUT returns a WiFiBandBitmap with only valid bits set"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_1(self):
        cp = self.cp

        # Step 1 — commissioning is done by the framework
        self.step(1)

        # Step 2 — FeatureMap
        self.step(2)
        feature_map = await self.read_feature_map()
        logger.info("FeatureMap = 0x%04x", feature_map)
        has_wi = self.has_feature_wi(feature_map)
        has_bgs = self.has_feature_bgs(feature_map)

        # Step 3 — Transport (mandatory)
        self.step(3)
        transport = await self.read_transport()
        logger.info("Transport = 0x%02x", transport)
        asserts.assert_not_equal(transport, 0,
                                 "Transport attribute must have at least one capability bit set")
        # Only defined bits: kBle=0x02, kWiFiPAF=0x08; verify no undefined bits
        valid_transport_mask = (cp.Bitmaps.CapabilitiesBitmap.kBle |
                                cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF)
        asserts.assert_equal(transport & ~valid_transport_mask, 0,
                             f"Transport 0x{transport:02x} contains undefined capability bits")

        # Step 4 — ScanMaxTime (mandatory, uint8, >= 1)
        self.step(4)
        scan_max_time = await self.read_cp_attribute(cp.Attributes.ScanMaxTime)
        logger.info("ScanMaxTime = %d", scan_max_time)
        asserts.assert_greater_equal(scan_max_time, 1,
                                     "ScanMaxTime must be >= 1")
        asserts.assert_less_equal(scan_max_time, 0xFF,
                                  "ScanMaxTime must fit in uint8")

        # Step 5 — MaxSessions (mandatory, uint8, >= 1)
        self.step(5)
        max_sessions = await self.read_cp_attribute(cp.Attributes.MaxSessions)
        logger.info("MaxSessions = %d", max_sessions)
        asserts.assert_greater_equal(max_sessions, 1,
                                     "MaxSessions must be >= 1")
        asserts.assert_less_equal(max_sessions, 0xFF,
                                  "MaxSessions must fit in uint8")

        # Step 6 — MaxCachedResults (BGS feature, uint8, >= 1)
        self.step(6)
        if not has_bgs:
            self.skip_step(6)
        else:
            max_cached = await self.read_cp_attribute(cp.Attributes.MaxCachedResults)
            logger.info("MaxCachedResults = %d", max_cached)
            asserts.assert_greater_equal(max_cached, 1,
                                         "MaxCachedResults must be >= 1")
            asserts.assert_less_equal(max_cached, 0xFF,
                                      "MaxCachedResults must fit in uint8")

        # Step 7 — NumCachedResults (BGS feature, uint8, should be 0 at start)
        self.step(7)
        if not has_bgs:
            self.skip_step(7)
        else:
            num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
            logger.info("NumCachedResults = %d", num_cached)
            asserts.assert_equal(num_cached, 0,
                                 "NumCachedResults should be 0 when no background scan is active")

        # Step 8 — CacheTimeout (BGS feature, uint16, >= 1)
        self.step(8)
        if not has_bgs:
            self.skip_step(8)
        else:
            cache_timeout = await self.read_cache_timeout()
            logger.info("CacheTimeout = %d", cache_timeout)
            asserts.assert_greater_equal(cache_timeout, 1,
                                         "CacheTimeout must be >= 1")
            asserts.assert_less_equal(cache_timeout, 0xFFFF,
                                      "CacheTimeout must fit in uint16")

        # Step 9 — CachedResults (BGS feature, nullable list)
        self.step(9)
        if not has_bgs:
            self.skip_step(9)
        else:
            cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
            logger.info("CachedResults = %s", cached_results)
            # At startup, must be null or empty
            is_null_or_empty = (cached_results is NullValue) or (cached_results == [])
            asserts.assert_true(is_null_or_empty,
                                "CachedResults must be null or empty at startup")

        # Step 10 — WiFiBand (WI feature, WiFiBandBitmap)
        self.step(10)
        if not has_wi:
            self.skip_step(10)
        else:
            wifi_band = await self.read_wifi_band()
            logger.info("WiFiBand = 0x%04x", wifi_band)
            valid_band_mask = (cp.Bitmaps.WiFiBandBitmap.k2g4 |
                               cp.Bitmaps.WiFiBandBitmap.k5g)
            asserts.assert_not_equal(wifi_band, 0,
                                     "WiFiBand must have at least one band bit set")
            asserts.assert_equal(wifi_band & ~valid_band_mask, 0,
                                 f"WiFiBand 0x{wifi_band:04x} contains undefined band bits")


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
