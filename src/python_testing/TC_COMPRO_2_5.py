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

"""TC-COMPRO-2.5 — Commissioning Proxy cluster: Writable Attributes with DUT as Server.

Verifies that the writable attributes ScanMaxTime and CacheTimeout can be
written, that the written value is reflected on a subsequent read, and that
the original value can be restored.

Test plan reference: TC-COMPRO-2.5 (PROVISIONAL)

Example usage:
    ```bash
    python3 TC_COMPRO_2_5.py \\
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

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_COMPRO_2_5(COMPROBaseTest):

    def desc_TC_COMPRO_2_5(self) -> str:
        return "[TC-COMPRO-2.5] Writable Attributes with DUT as Server"

    def pics_TC_COMPRO_2_5(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads ScanMaxTime attribute and saves as original_scan_max_time",
                     "DUT returns a uint8 value >= 1"),
            TestStep(3, "TH writes ScanMaxTime attribute with value (original_scan_max_time + 1), "
                     "clamped to uint8 range",
                     "DUT returns SUCCESS"),
            TestStep(4, "TH reads ScanMaxTime attribute",
                     "DUT returns the value written in step 3"),
            TestStep(5, "TH restores ScanMaxTime attribute to original_scan_max_time",
                     "DUT returns SUCCESS"),
            TestStep(6, "TH reads CacheTimeout attribute (BGS feature) and saves as original_cache_timeout",
                     "DUT returns a uint16 value >= 1"),
            TestStep(7, "TH writes CacheTimeout attribute with value (original_cache_timeout + 1), "
                     "clamped to uint16 range (BGS feature)",
                     "DUT returns SUCCESS"),
            TestStep(8, "TH reads CacheTimeout attribute (BGS feature)",
                     "DUT returns the value written in step 7"),
            TestStep(9, "TH restores CacheTimeout attribute to original_cache_timeout (BGS feature)",
                     "DUT returns SUCCESS"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_5(self):
        cp = self.cp

        # Step 1 — commissioning is done by the framework
        self.step(1)

        # Determine BGS feature support (used to guard steps 6-9).
        feature_map = await self.read_feature_map()
        has_bgs = self.has_feature_bgs(feature_map)
        logger.info("FeatureMap = 0x%04x  has_bgs=%s", feature_map, has_bgs)

        # ----------------------------------------------------------------
        # Steps 2-5: ScanMaxTime read / write / verify / restore
        # ----------------------------------------------------------------

        # Step 2 — read ScanMaxTime, save original
        self.step(2)
        original_scan_max_time = await self.read_scan_max_time()
        logger.info("ScanMaxTime (original) = %d", original_scan_max_time)
        asserts.assert_greater_equal(original_scan_max_time, 1,
                                     "ScanMaxTime must be >= 1")

        # Step 3 — write ScanMaxTime = original + 1 (clamped to uint8)
        self.step(3)
        new_scan_max_time = min(original_scan_max_time + 1, 0xFF)
        logger.info("Writing ScanMaxTime = %d", new_scan_max_time)
        await self.write_single_attribute(
            attribute_value=cp.Attributes.ScanMaxTime(new_scan_max_time),
            endpoint_id=self.cp_endpoint,
        )

        # Step 4 — read back and verify
        self.step(4)
        read_scan_max_time = await self.read_scan_max_time()
        logger.info("ScanMaxTime (after write) = %d", read_scan_max_time)
        asserts.assert_equal(read_scan_max_time, new_scan_max_time,
                             f"ScanMaxTime must reflect the written value {new_scan_max_time}")

        # Step 5 — restore original value
        self.step(5)
        logger.info("Restoring ScanMaxTime = %d", original_scan_max_time)
        await self.write_single_attribute(
            attribute_value=cp.Attributes.ScanMaxTime(original_scan_max_time),
            endpoint_id=self.cp_endpoint,
        )

        # ----------------------------------------------------------------
        # Steps 6-9: CacheTimeout read / write / verify / restore (BGS only)
        # ----------------------------------------------------------------

        if has_bgs:
            # Step 6 — read CacheTimeout, save original
            self.step(6)
            original_cache_timeout = await self.read_cache_timeout()
            logger.info("CacheTimeout (original) = %d", original_cache_timeout)
            asserts.assert_greater_equal(original_cache_timeout, 1,
                                         "CacheTimeout must be >= 1")

            # Step 7 — write CacheTimeout = original + 1 (clamped to uint16)
            self.step(7)
            new_cache_timeout = min(original_cache_timeout + 1, 0xFFFF)
            logger.info("Writing CacheTimeout = %d", new_cache_timeout)
            await self.write_single_attribute(
                attribute_value=cp.Attributes.CacheTimeout(new_cache_timeout),
                endpoint_id=self.cp_endpoint,
            )

            # Step 8 — read back and verify
            self.step(8)
            read_cache_timeout = await self.read_cache_timeout()
            logger.info("CacheTimeout (after write) = %d", read_cache_timeout)
            asserts.assert_equal(read_cache_timeout, new_cache_timeout,
                                 f"CacheTimeout must reflect the written value {new_cache_timeout}")

            # Step 9 — restore original value
            self.step(9)
            logger.info("Restoring CacheTimeout = %d", original_cache_timeout)
            await self.write_single_attribute(
                attribute_value=cp.Attributes.CacheTimeout(original_cache_timeout),
                endpoint_id=self.cp_endpoint,
            )
        else:
            self.mark_step_range_skipped(6, 9)


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
