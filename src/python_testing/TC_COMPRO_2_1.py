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

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_COMPRO_2_1(COMPROBaseTest):

    def desc_TC_COMPRO_2_1(self) -> str:
        return "[TC-COMPRO-2.1] Attributes with DUT as Server"

    def pics_TC_COMPRO_2_1(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute",
                     "DUT returns a valid FeatureMap bitmap"),
            TestStep(3, "TH reads Transport attribute",
                     "DUT returns a CapabilitiesBitmap value with at least one of the BLE "
                     "(bit 1), WiFiPAF (bit 3) or NTL (bit 4) bits set and no reserved bits "
                     "(bits 0, 2 or 5-7) set"),
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
                     "DUT returns null (no cached results when background scan is not running)"),
            TestStep(10, "TH reads WiFiBand attribute (if present in AttributeList)",
                     "DUT returns a WiFiBandBitmap with only valid bits set"),
            TestStep(11, "TH writes Transport attribute with a value different from step 3",
                     "DUT returns UNSUPPORTED_WRITE (read-only Fixed attribute)"),
            TestStep(12, "TH reads Transport attribute",
                     "DUT returns the same value as step 3 (value unchanged)"),
            TestStep(13, "TH writes MaxSessions attribute with a value different from step 5",
                     "DUT returns UNSUPPORTED_WRITE (read-only Fixed attribute)"),
            TestStep(14, "TH reads MaxSessions attribute",
                     "DUT returns the same value as step 5 (value unchanged)"),
            TestStep(15, "TH writes MaxCachedResults attribute with a value different from step 6 "
                     "(if BGS supported)",
                     "DUT returns UNSUPPORTED_WRITE (read-only Fixed attribute)"),
            TestStep(16, "TH reads MaxCachedResults attribute (if BGS supported)",
                     "DUT returns the same value as step 6 (value unchanged)"),
            TestStep(17, "TH writes WiFiBand attribute with a value different from step 10 "
                     "(if present in AttributeList)",
                     "DUT returns UNSUPPORTED_WRITE (read-only Fixed attribute)"),
            TestStep(18, "TH reads WiFiBand attribute (if present in AttributeList)",
                     "DUT returns the same value as step 10 (value unchanged)"),
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
        has_bgs = self.has_feature_bgs(feature_map)

        # Step 3 — Transport (mandatory).  Defined transport bits per spec are
        # BLE (bit 1), WiFiPAF (bit 3) and NTL (bit 4); bits 0, 2 and 5-7 are
        # reserved.  At least one defined bit must be set and no reserved bits.
        self.step(3)
        transport = await self.read_transport()
        logger.info("Transport = 0x%02x", transport)
        self.assert_transport_value_valid(transport)

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

        max_cached = None
        if has_bgs:
            # Step 6 — MaxCachedResults (BGS feature, uint8, >= 1)
            self.step(6)
            max_cached = await self.read_cp_attribute(cp.Attributes.MaxCachedResults)
            logger.info("MaxCachedResults = %d", max_cached)
            asserts.assert_greater_equal(max_cached, 1,
                                         "MaxCachedResults must be >= 1")
            asserts.assert_less_equal(max_cached, 0xFF,
                                      "MaxCachedResults must fit in uint8")

            # Step 7 — NumCachedResults (BGS feature, uint8, should be 0 at start)
            self.step(7)
            num_cached = await self.read_cp_attribute(cp.Attributes.NumCachedResults)
            logger.info("NumCachedResults = %d", num_cached)
            asserts.assert_equal(num_cached, 0,
                                 "NumCachedResults should be 0 when no background scan is active")

            # Step 8 — CacheTimeout (BGS feature, uint16, >= 1)
            self.step(8)
            cache_timeout = await self.read_cache_timeout()
            logger.info("CacheTimeout = %d", cache_timeout)
            asserts.assert_greater_equal(cache_timeout, 1,
                                         "CacheTimeout must be >= 1")
            asserts.assert_less_equal(cache_timeout, 0xFFFF,
                                      "CacheTimeout must fit in uint16")

            # Step 9 — CachedResults (BGS feature, nullable list)
            self.step(9)
            cached_results = await self.read_cp_attribute(cp.Attributes.CachedResults)
            logger.info("CachedResults = %s", cached_results)
            asserts.assert_equal(cached_results, NullValue,
                                 "CachedResults must be null when no background scan is active")
        else:
            self.mark_step_range_skipped(6, 9)

        # Step 10 — WiFiBand (WiFiBandBitmap).  Conformance is [WI]: optional even
        # when WI is supported, so a WI proxy MAY omit the attribute.  Gate on its
        # actual presence in the AttributeList rather than on the WI feature.  The
        # spec constraint is "desc": WiFiBand indicates the bands supported for
        # PAFTP.  WI does not require the WiFiPAF transport bit, so a value of 0 is
        # valid when WI is advertised without WiFiPAF.  Only require at least one
        # band when the WiFiPAF transport is advertised; always reject undefined
        # bits.
        wifi_band = None
        self.step(10)
        has_wifiband = await self.attribute_guard(endpoint=self.cp_endpoint, attribute=cp.Attributes.WiFiBand)
        if has_wifiband:
            wifi_band = await self.read_wifi_band()
            logger.info("WiFiBand = 0x%04x", wifi_band)
            valid_band_mask = (cp.Bitmaps.WiFiBandBitmap.k2g4 |
                               cp.Bitmaps.WiFiBandBitmap.k5g)
            asserts.assert_equal(wifi_band & ~valid_band_mask, 0,
                                 f"WiFiBand 0x{wifi_band:04x} contains undefined band bits")
            if transport & cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF:
                asserts.assert_not_equal(
                    wifi_band, 0,
                    "WiFiBand must have at least one band bit set when WiFiPAF transport is supported")

        # ------------------------------------------------------------------
        # Steps 11-18 — Transport, MaxSessions, MaxCachedResults and WiFiBand
        # all carry the Fixed (F) quality and are read-only (R access).  A write
        # SHALL be rejected with UNSUPPORTED_WRITE and SHALL NOT change the value;
        # the following read confirms the stored value is unchanged.
        # ------------------------------------------------------------------

        # Step 11 — Transport: write a different value → UNSUPPORTED_WRITE
        self.step(11)
        await self.expect_write_rejected(
            cp.Attributes.Transport(transport ^ int(cp.Bitmaps.CapabilitiesBitmap.kBle)),
            "Transport")

        # Step 12 — Transport unchanged after the rejected write
        self.step(12)
        transport2 = await self.read_transport()
        logger.info("Transport (re-read) = 0x%02x", transport2)
        asserts.assert_equal(transport2, transport,
                             "Transport attribute (Fixed quality) value changed after a rejected write")

        # Step 13 — MaxSessions: write a different value → UNSUPPORTED_WRITE
        self.step(13)
        await self.expect_write_rejected(
            cp.Attributes.MaxSessions((max_sessions % 0xFF) + 1),
            "MaxSessions")

        # Step 14 — MaxSessions unchanged after the rejected write
        self.step(14)
        max_sessions2 = await self.read_cp_attribute(cp.Attributes.MaxSessions)
        logger.info("MaxSessions (re-read) = %d", max_sessions2)
        asserts.assert_equal(max_sessions2, max_sessions,
                             "MaxSessions attribute (Fixed quality) value changed after a rejected write")

        # Steps 15-16 — MaxCachedResults (BGS feature only)
        if has_bgs:
            # Step 15 — write a different value → UNSUPPORTED_WRITE
            self.step(15)
            await self.expect_write_rejected(
                cp.Attributes.MaxCachedResults((max_cached % 0xFF) + 1),
                "MaxCachedResults")

            # Step 16 — MaxCachedResults unchanged after the rejected write
            self.step(16)
            max_cached2 = await self.read_cp_attribute(cp.Attributes.MaxCachedResults)
            logger.info("MaxCachedResults (re-read) = %d", max_cached2)
            asserts.assert_equal(max_cached2, max_cached,
                                 "MaxCachedResults attribute (Fixed quality) value changed after a rejected write")
        else:
            self.mark_step_range_skipped(15, 16)

        # Steps 17-18 — WiFiBand ([WI]: present only if advertised in the
        # AttributeList, as determined in step 10)
        if has_wifiband:
            # Step 17 — write a different value → UNSUPPORTED_WRITE
            self.step(17)
            await self.expect_write_rejected(
                cp.Attributes.WiFiBand(wifi_band ^ int(cp.Bitmaps.WiFiBandBitmap.k2g4)),
                "WiFiBand")

            # Step 18 — WiFiBand unchanged after the rejected write
            self.step(18)
            wifi_band2 = await self.read_wifi_band()
            logger.info("WiFiBand (re-read) = 0x%04x", wifi_band2)
            asserts.assert_equal(wifi_band2, wifi_band,
                                 "WiFiBand attribute (Fixed quality) value changed after a rejected write")
        else:
            self.mark_step_range_skipped(17, 18)


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
