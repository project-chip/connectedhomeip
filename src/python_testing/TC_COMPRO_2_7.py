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

"""TC-COMPRO-2.7 — Commissioning Proxy cluster: Multi-Session Support with DUT as Server.

Verifies that the Commissioning Proxy server correctly handles simultaneous
commissioning sessions up to the MaxSessions limit.

  Steps 2–4:  Read MaxSessions, Transport, and WiFiBand attributes.
  Step 5:     Ensure max_sessions EDs are commissionable simultaneously.
  Step 6:     Loop: for each ED N from 1 to max_sessions, send
              ProxyConnectRequest and verify a unique SessionID in 0x0001–0xFFFE.
  Step 7:     One additional ProxyConnectRequest (all slots occupied) must be
              rejected with RESOURCE_EXHAUSTED.
  Step 8:     If max_sessions > 1, verify all session IDs from step 6 are distinct.
  Step 9:     ProxyDisconnectRequest for every session from step 6 must succeed.

Test plan reference: TC-COMPRO-2.7 (PROVISIONAL)

Example — automated with remote ED RPi (max_sessions == 1):
    ```bash
    python3 TC_COMPRO_2_7.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 1 \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_ssh_host:192.168.1.10 \\
        --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --int-arg ed_discriminator:3841 ed_passcode:20202021
    ```

For max_sessions > 1 supply additional EDs via ed2_*, ed3_*, ... arguments:
    ```
    --string-arg ed2_app_path:/home/ubuntu/apps/chip-lighting-app \\
        ed2_ssh_host:192.168.1.11 'ed2_extra_args:--wifi --wifipaf freq_list=2437' \\
    --int-arg ed2_discriminator:3842 ed2_passcode:20202021
    ```
"""

import logging
import typing

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, EDFixture, commission_if_needed

from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

CONNECT_TIMEOUT_S = 120


class TC_COMPRO_2_7(COMPROBaseTest):

    @property
    def default_timeout(self) -> int:
        # Step 6 connect loop: max_sessions × (proxy_connect_timeout + margin)
        # Step 7 quick rejection + step 9 disconnects + other steps: ~60 s
        return 600

    def desc_TC_COMPRO_2_7(self) -> str:
        return "[TC-COMPRO-2.7] Multi-Session Support with DUT as Server"

    def pics_TC_COMPRO_2_7(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_7(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT (proxy) to TH", is_commissioning=True),
            TestStep(2, "TH reads MaxSessions attribute",
                     "Value is a uint8 ≥ 1; store as max_sessions"),
            TestStep(3, "TH reads Transport attribute", "Store as valid_transports"),
            TestStep(4, "TH reads WiFiBand attribute (if WI feature supported)",
                     "Store as valid_bands"),
            TestStep(5, "Ensure at least max_sessions EDs are commissionable simultaneously"),
            TestStep(6, "For each ED N from 1 to max_sessions, TH sends ProxyConnectRequest",
                     "Each response has SUCCESS and a unique SessionID in 0x0001–0xFFFE; "
                     "store as session_N"),
            TestStep(7, "TH sends ProxyConnectRequest with any valid parameters "
                     "(all max_sessions slots are now occupied)",
                     "DUT returns RESOURCE_EXHAUSTED"),
            TestStep(8, "If max_sessions > 1: verify all session_N values from step 6 are distinct",
                     "All session_N values are different from each other"),
            TestStep(9, "TH sends ProxyDisconnectRequest for each session_N from step 6",
                     "Each disconnect returns SUCCESS"),
        ]

    def _ed_fixture_for_index(self, n: int) -> typing.Optional[EDFixture]:
        """Build an EDFixture for ED #n (1-based) from user params.

        ED #1 uses the standard ed_* params; ED #2+ use ed2_*, ed3_*, etc.
        Returns None if the relevant app_path param is absent (operator prompted).
        """
        params = getattr(self, 'user_params', {}) or {}
        prefix = 'ed' if n == 1 else f'ed{n}'
        app_path = params.get(f'{prefix}_app_path')
        if not app_path:
            return None
        return EDFixture(
            app_path=app_path,
            discriminator=int(params.get(f'{prefix}_discriminator', 3840 + n - 1)),
            passcode=int(params.get(f'{prefix}_passcode', 20202021)),
            ssh_host=params.get(f'{prefix}_ssh_host'),
            ssh_user=params.get(f'{prefix}_ssh_user', 'ubuntu'),
            extra_args=params.get(f'{prefix}_extra_args', ''),
            ed_transport=params.get(f'{prefix}_transport', params.get('ed_transport', 'wifipaf')),
        )

    def _discriminator_for_index(self, n: int) -> int:
        params = getattr(self, 'user_params', {}) or {}
        prefix = 'ed' if n == 1 else f'ed{n}'
        return int(params.get(f'{prefix}_discriminator', 3840 + n - 1))

    @async_test_body
    async def test_TC_COMPRO_2_7(self):
        cp = self.cp
        params = getattr(self, 'user_params', {}) or {}
        proxy_connect_timeout = int(params.get('proxy_connect_timeout', CONNECT_TIMEOUT_S))

        # Step 1 — commissioning done by the framework
        self.step(1)

        # Step 2 — read MaxSessions
        self.step(2)
        max_sessions = int(await self.read_max_sessions())
        asserts.assert_greater_equal(max_sessions, 1,
                                     f"MaxSessions must be ≥ 1, got {max_sessions}")
        logger.info("max_sessions = %d", max_sessions)

        # Step 3 — read Transport
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("valid_transports = 0x%02x", valid_transports)

        # Step 4 — read WiFiBand (conditional on WI feature)
        feature_map = await self.read_feature_map()
        has_wi = self.has_feature_wi(feature_map)
        valid_bands: int = 0
        if has_wi:
            self.step(4)
            valid_bands = await self.read_wifi_band()
            logger.info("valid_bands = 0x%04x", valid_bands)
        else:
            self.skip_step(4)

        # For ProxyConnectRequest commands, select the transport that matches the
        # actual ED transport type so wiFiBand is not sent with a non-WiFiPAF
        # transport (which would cause INVALID_COMMAND).
        ed_transport_type = params.get('ed_transport', 'wifipaf')
        proxy_transport = self.pick_proxy_transport(valid_transports, ed_transport_type)
        proxy_wifi_band = (self.pick_single_transport_bit(valid_bands)
                           if has_wi and proxy_transport == int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF)
                           else None)

        # Step 5 — ensure max_sessions EDs commissionable simultaneously
        self.step(5)
        ed_fixtures: list[typing.Optional[EDFixture]] = []
        for n in range(1, max_sessions + 1):
            disc_n = self._discriminator_for_index(n)
            ed_n = self._ed_fixture_for_index(n)
            ed_fixtures.append(ed_n)
            await self.ensure_ed_commissionable(
                ed_n,
                manual_prompt=(
                    f"Ensure ED #{n} is commissionable via WiFiPAF "
                    f"(discriminator={disc_n}). Press Enter when ready."
                ),
            )

        # ------------------------------------------------------------------
        # Step 6 — connect EDs 1..max_sessions; collect session IDs
        # ------------------------------------------------------------------
        self.step(6)
        active_sessions: list[int] = []

        for n in range(1, max_sessions + 1):
            disc_n = self._discriminator_for_index(n)
            logger.info("Step 6 [ED #%d]: ProxyConnectRequest (discriminator=%d)", n, disc_n)
            resp = await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=proxy_transport,
                    discriminator=disc_n,
                    vendorId=0,
                    productId=0,
                    timeout=proxy_connect_timeout,
                    wiFiBand=proxy_wifi_band,
                ),
                interactionTimeoutMs=None,
            )
            sess_n = resp.sessionID
            asserts.assert_true(
                0x0001 <= sess_n <= 0xFFFE,
                f"session_{n} {sess_n:#06x} must be in range 0x0001–0xFFFE")
            logger.info("Step 6 [ED #%d]: session_%d = %d (0x%04x)", n, n, sess_n, sess_n)
            active_sessions.append(sess_n)

        # ------------------------------------------------------------------
        # Step 7 — one more ProxyConnectRequest → RESOURCE_EXHAUSTED
        # All max_sessions slots are occupied; DUT rejects at the session-count
        # check before any transport scan, so no additional ED is needed.
        # ------------------------------------------------------------------
        self.step(7)
        # The overflow request must itself be valid so RESOURCE_EXHAUSTED (session
        # table full) is the only possible rejection reason. The discriminator must
        # stay within the spec range (0..4095); a value above it would be rejected
        # with INVALID_COMMAND by the discriminator-range check, which runs before
        # the session-count gate. 4095 is the max valid value and is clear of the
        # 3840-based discriminators used to fill the sessions in step 6.
        overflow_disc = int(params.get('overflow_discriminator', 4095))
        logger.info("Step 7: ProxyConnectRequest (discriminator=%d, expect RESOURCE_EXHAUSTED)",
                    overflow_disc)
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=proxy_transport,
                    discriminator=overflow_disc,
                    vendorId=0,
                    productId=0,
                    timeout=30,
                    wiFiBand=proxy_wifi_band,
                ),
                interactionTimeoutMs=10000,
            )
            asserts.fail("Expected RESOURCE_EXHAUSTED but ProxyConnectRequest succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Expected RESOURCE_EXHAUSTED when session table is full, got {e.status}")
            logger.info("Step 7: correctly got RESOURCE_EXHAUSTED")

        # ------------------------------------------------------------------
        # Step 8 — verify all session IDs distinct (only meaningful when > 1)
        # ------------------------------------------------------------------
        if max_sessions > 1:
            self.step(8)
            asserts.assert_equal(
                len(active_sessions), len(set(active_sessions)),
                f"SessionIds from step 6 are not all unique: {active_sessions}")
            logger.info("Step 8: all %d session IDs are distinct: %s", max_sessions, active_sessions)
        else:
            self.skip_step(8)

        # ------------------------------------------------------------------
        # Step 9 — disconnect all active sessions → SUCCESS
        # ------------------------------------------------------------------
        self.step(9)
        for sess_id in active_sessions:
            logger.info("Step 9: ProxyDisconnectRequest(sessionID=%d)", sess_id)
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyDisconnectRequest(sessionID=sess_id),
            )
            logger.info("Step 9: disconnected session %d", sess_id)

        # Cleanup
        for n, ed in enumerate(ed_fixtures, start=1):
            await self.ensure_ed_not_commissionable(
                ed,
                manual_prompt=(
                    f"Test complete. Stop ED #{n} from advertising. Press Enter when done."
                ),
            )


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
