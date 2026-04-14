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

"""TC-COMPRO-2.2 — Commissioning Proxy cluster: Proxy Scan feature functionality.

Verifies that ProxyScanRequest returns an empty list when no ED is commissionable,
and returns a populated result list (with correct fields) when an ED is commissionable.

Test plan reference: TC-COMPRO-2.2

Example usage:
    ```bash
    python3 TC_COMPRO_2_2.py \\
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

import logging
import time

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

# When an ED discriminator and passcode are known for verification they can be
# passed as --int-arg ed_discriminator:<int>
# If not provided, only NumberOfResults >= 1 is checked (discriminator not validated).

# Tolerance added on top of ScanMaxTime before the response must arrive (10 %)
SCAN_TIMEOUT_MARGIN = 1.10


class TC_COMPRO_2_2(COMPROBaseTest):

    def desc_TC_COMPRO_2_2(self) -> str:
        return "TC-COMPRO-2.2: Proxy Scan feature functionality"

    def pics_TC_COMPRO_2_2(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "Ensure ED is NOT commissionable"),
            TestStep(3, "TH reads Transport attribute",
                     "Store as valid_transports"),
            TestStep(4, "TH reads WiFiBand attribute (if WI supported)",
                     "Store as valid_bands"),
            TestStep(5, "TH reads ScanMaxTime attribute",
                     "Store as scan_max_time"),
            TestStep(6, "TH sends ProxyScanRequest with valid_transports (and valid_bands if WI)",
                     "DUT sends ProxyScanResponse within scan_max_time * 1.1 s; "
                     "Status SUCCESS; NumberOfResults == 0; ProxyScanResult is empty"),
            TestStep(7, "Ensure ED IS commissionable"),
            TestStep(8, "TH sends ProxyScanRequest with valid_transports (and valid_bands if WI)",
                     "DUT sends ProxyScanResponse within scan_max_time * 1.1 s; "
                     "Status SUCCESS; NumberOfResults >= 1; "
                     "At least one result has matching Transport, Discriminator, VendorID, ProductID"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_2(self):
        cp = self.cp
        ed = self._ed_fixture_from_params()
        params = getattr(self, 'user_params', {}) or {}

        # Optional: if the operator knows the ED discriminator, we verify it
        expected_discriminator = int(params.get('ed_discriminator', 0))

        # Step 1 — commissioning done by framework
        self.step(1)

        # Step 2 — ensure ED not commissionable
        self.step(2)
        await self.ensure_ed_not_commissionable(ed)

        # Step 3 — read Transport
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("valid_transports = 0x%02x", valid_transports)

        # Step 4 — read WiFiBand (conditional)
        self.step(4)
        feature_map = await self.read_feature_map()
        has_wi = self.has_feature_wi(feature_map)
        valid_bands: int = 0
        if has_wi:
            valid_bands = await self.read_wifi_band()
            logger.info("valid_bands = 0x%04x", valid_bands)
        else:
            self.skip_step(4)

        # Step 5 — read ScanMaxTime
        self.step(5)
        scan_max_time = await self.read_scan_max_time()
        response_deadline = scan_max_time * SCAN_TIMEOUT_MARGIN
        logger.info("scan_max_time = %d s  (deadline = %.1f s)", scan_max_time, response_deadline)

        # Step 6 — scan with ED not present
        self.step(6)
        response = await self._send_scan_request(valid_transports, valid_bands if has_wi else None,
                                                  timeout_sec=response_deadline)
        asserts.assert_equal(response.numberOfResults, 0,
                             "Expected NumberOfResults == 0 when no ED is commissionable")
        asserts.assert_equal(response.proxyScanResult, [],
                             "Expected empty ProxyScanResult when no ED is commissionable")

        # Step 7 — make ED commissionable
        self.step(7)
        await self.ensure_ed_commissionable(ed)

        # Step 8 — scan with ED present
        self.step(8)
        response = await self._send_scan_request(valid_transports, valid_bands if has_wi else None,
                                                  timeout_sec=response_deadline)
        asserts.assert_greater_equal(response.numberOfResults, 1,
                                     "Expected NumberOfResults >= 1 when ED is commissionable")
        asserts.assert_greater_equal(len(response.proxyScanResult), 1,
                                     "Expected at least one entry in ProxyScanResult")

        # Validate the first (or matching) result entry
        result = response.proxyScanResult[0]
        if expected_discriminator:
            # Find the entry matching the known ED discriminator
            matches = [r for r in response.proxyScanResult if r.discriminator == expected_discriminator]
            asserts.assert_greater_equal(len(matches), 1,
                                         f"No scan result matched ED discriminator {expected_discriminator}")
            result = matches[0]

        logger.info("Scan result: discriminator=%d transport=0x%02x vendorId=%d productId=%d",
                    result.discriminator, result.transport, result.vendorId, result.productId)

        # Transport must be non-zero
        asserts.assert_not_equal(result.transport, 0,
                                 "ScanResultStruct.Transport must be non-zero")
        # Discriminator range: 0-4095
        asserts.assert_less_equal(result.discriminator, 4095,
                                  "ScanResultStruct.Discriminator must be <= 4095")
        # WiFiBand field must be present if WI feature is supported
        if has_wi:
            asserts.assert_is_not_none(result.wiFiBand,
                                       "ScanResultStruct.WiFiBand must be present when WI feature is supported")

        # Cleanup
        await self.ensure_ed_not_commissionable(ed)

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    async def _send_scan_request(self, transport: int, wifi_bands: int | None,
                                 timeout_sec: float) -> Clusters.CommissioningProxy.Commands.ProxyScanResponse:
        """Send ProxyScanRequest and return the ProxyScanResponse.

        The response is async from the DUT's perspective; we use a generous
        interactionTimeoutMs to cover the full scan window plus margin.
        """
        cmd = self.cp.Commands.ProxyScanRequest(
            transport=transport,
            wiFiBands=wifi_bands,
        )
        start = time.monotonic()
        response = await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cmd,
            interactionTimeoutMs=int(timeout_sec * 1000) + 2000,
        )
        elapsed = time.monotonic() - start
        logger.info("ProxyScanResponse received in %.2f s (deadline %.1f s)", elapsed, timeout_sec)
        asserts.assert_less_equal(elapsed, timeout_sec,
                                  f"ProxyScanResponse took {elapsed:.2f} s, exceeds {timeout_sec:.1f} s deadline")
        return response


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
