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
#     app: ${ALL_DEVICES_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --device commissioning-proxy:5 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --in-test-commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 5
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --string-arg ed_app_path:${ED_APP}
#       --int-arg ed_discriminator:3841 ed_passcode:20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""TC-COMPRO-2.6 — Commissioning Proxy cluster: Error Handling with DUT as Server.

For the test rig topology, the Python wheel requirement and how to run this
suite, see ``support_modules/compro_support.py``.

Verifies the error responses of the Commissioning Proxy server for invalid
command scenarios.  The procedure is split into two phases so the session type
in use at each step is unambiguous:

  Phase 1 — DUT NOT commissioned to TH (PASE), steps 1-5:
    Step 1:  DUT starts uncommissioned and in commissioning mode; ED is
             commissionable.
    Step 2:  TH establishes a PASE session with the DUT (no commissioning).
    Steps 3-5: The three commands the spec requires to be executed via a CASE
             session (ProxyConnectRequest, ProxyMessageRequest,
             ProxyDisconnectRequest) are rejected with UNSUPPORTED_ACCESS over
             the PASE session.

  Phase 2 — DUT commissioned to TH (CASE), steps 6-13:
    Step 6:  Commission the DUT to the TH; from here all commands are over CASE.
    Steps 7-8: Read Transport / WiFiBand.
    Step 9:  ProxyConnectRequest with more than one transport bit set →
             INVALID_COMMAND.
    Step 10: ProxyMessageRequest for a non-existent SessionID → NOT_FOUND.
    Step 11: ProxyConnectRequest to establish a valid session (session_a).
    Step 12: A second concurrent ProxyMessageRequest for session_a, sent while
             the first is still in-flight → BUSY.
    Step 13: ProxyDisconnectRequest(session_a) → SUCCESS.

The scan commands (ProxyScanRequest and the BackgroundScan commands) are O-only
in the command table — they carry no CASE-session requirement — so they are not
expected to be rejected over PASE and are not exercised in phase 1.

This test MUST run with ``--in-test-commissioning-method`` (not
``--commissioning-method``) against a factory-reset / uncommissioned DUT, so the
PASE session in phase 1 exists before any fabric is provisioned.

Test plan reference: TC-COMPRO-2.6

Example — automated with a serial-driven ED RPi:
    ```bash
    python3 TC_COMPRO_2_6.py \\
        --in-test-commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 5 \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_serial_port:/dev/ttyUSB0 \\
        --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --int-arg ed_discriminator:3841 ed_passcode:20202021
    ```
"""

import asyncio
import logging

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest

from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout for ProxyConnectRequest (seconds)
CONNECT_TIMEOUT_S = 120

# Minimal placeholder message (8 zero bytes) for negative-path ProxyMessageRequest
# tests.  This is NOT a well-formed Matter frame; it is sufficient only because the
# DUT rejects the request (UNSUPPORTED_ACCESS / NOT_FOUND / BUSY) before parsing it.
_MINIMAL_MATTER_MSG = bytes(8)


class TC_COMPRO_2_6(COMPROBaseTest):

    # The DUT starts UNCOMMISSIONED (PASE-first): there is no operational node to
    # subscribe to at setup_test time, so opt out of the framework's background
    # wildcard subscription — otherwise it spends ~45 s timing out on operational
    # mDNS resolution before step 1 even begins.
    disable_wildcard_subscription = True

    @property
    def default_timeout(self) -> int:
        # Step 6 commissions the DUT (~30-60 s).
        # Step 11 ProxyConnect: up to proxy_connect_timeout (default 120 s) + margin.
        # Step 12 BUSY test: ResponseTimeout=10 s + IM margin + 2 s sleep.
        # PASE + remaining steps: ~30 s.
        return 400

    def desc_TC_COMPRO_2_6(self) -> str:
        return "[TC-COMPRO-2.6] Error Handling with DUT as Server"

    def pics_TC_COMPRO_2_6(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "Ensure the DUT is uncommissioned and in commissioning mode, "
                     "and that the ED is commissionable"),
            TestStep(2, "TH establishes a PASE session with the DUT and does not proceed "
                     "with commissioning",
                     "PASE session establishment succeeds"),
            TestStep(3, "Over the PASE session, TH sends ProxyConnectRequest",
                     "DUT returns UNSUPPORTED_ACCESS"),
            TestStep(4, "Over the PASE session, TH sends ProxyMessageRequest",
                     "DUT returns UNSUPPORTED_ACCESS"),
            TestStep(5, "Over the PASE session, TH sends ProxyDisconnectRequest",
                     "DUT returns UNSUPPORTED_ACCESS"),
            TestStep(6, "Commission DUT to TH; all remaining steps are over CASE",
                     "DUT is commissioned onto the TH fabric; CASE session can be established",
                     is_commissioning=True),
            TestStep(7, "TH reads Transport attribute",
                     "Store as valid_transports; value has at least one of the BLE (bit 1), "
                     "WiFiPAF (bit 3) or NTL (bit 4) bits set and no reserved bits set"),
            TestStep(8, "TH reads WiFiBand attribute (if WI supported)", "Store as valid_bands"),
            TestStep(9, "TH sends ProxyConnectRequest with more than one transport bit set",
                     "DUT returns INVALID_COMMAND"),
            TestStep(10, "TH sends ProxyMessageRequest with SessionID=0xFFFE (no active session)",
                     "DUT returns NOT_FOUND"),
            TestStep(11, "TH sends ProxyConnectRequest to establish a valid session",
                     "DUT returns ProxyConnectResponse with SUCCESS; save session_id as session_a "
                     "(0x0001–0xFFFE)"),
            TestStep(12, "TH sends two concurrent ProxyMessageRequests for session_a; "
                     "the second is sent while the first is still in-flight",
                     "The second ProxyMessageRequest is rejected with BUSY"),
            TestStep(13, "TH sends ProxyDisconnectRequest(SessionID=session_a)",
                     "DUT returns SUCCESS"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_6(self):
        cp = self.cp
        params = getattr(self, 'user_params', {}) or {}
        ed = self._ed_fixture_from_params()

        ed_discriminator = int(params.get('ed_discriminator', 3841))
        ed_passcode = int(params.get('ed_passcode', 20202021))
        proxy_connect_timeout = int(params.get('proxy_connect_timeout', CONNECT_TIMEOUT_S))

        # ================================================================
        # Phase 1 — DUT NOT commissioned to TH: CASE session requirement
        # ================================================================

        # Step 1 — DUT uncommissioned/in commissioning mode; make ED commissionable
        # (needed for the valid session established in step 11).
        self.step(1)
        await self.ensure_ed_commissionable(
            ed,
            manual_prompt=(
                "Ensure the DUT (proxy) is factory-reset / uncommissioned and in "
                "commissioning mode. Make the End Device commissionable via WiFiPAF "
                f"(discriminator={ed_discriminator}, passcode={ed_passcode}). "
                "Press Enter when ready."
            ),
        )

        # Step 2 — establish a bare PASE session to the uncommissioned DUT.
        self.step(2)
        pase_node_id = await self.establish_pase_to_dut()
        logger.info("Step 2: PASE session established (pase_node_id=0x%016x)", pase_node_id)

        # Read Transport over the PASE session (attribute reads are not
        # CASE-restricted) so the PASE ProxyConnectRequest in step 3 carries a
        # transport the DUT actually supports, and its rejection is on access
        # grounds only.
        transport_read = await self.default_controller.ReadAttribute(
            pase_node_id,
            [(self.cp_endpoint, cp.Attributes.Transport)],
        )
        pase_transport = self.pick_single_transport_bit(
            transport_read[self.cp_endpoint][cp][cp.Attributes.Transport])

        # ----------------------------------------------------------------
        # Step 3: ProxyConnectRequest over PASE — UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(3)
        logger.info("Step 3: ProxyConnectRequest over PASE (expect UNSUPPORTED_ACCESS)")
        await self.expect_command_rejected(
            cp.Commands.ProxyConnectRequest(
                address=NullValue,
                transport=pase_transport,
                discriminator=ed_discriminator,
                vendorID=0,
                productID=0,
                timeout=30,
            ),
            Status.UnsupportedAccess,
            "Step 3 ProxyConnectRequest over PASE",
            node_id=pase_node_id)

        # ----------------------------------------------------------------
        # Step 4: ProxyMessageRequest over PASE — UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(4)
        logger.info("Step 4: ProxyMessageRequest over PASE (expect UNSUPPORTED_ACCESS)")
        await self.expect_command_rejected(
            cp.Commands.ProxyMessageRequest(
                sessionID=0x0001,
                responseTimeout=10,
                message=_MINIMAL_MATTER_MSG,
            ),
            Status.UnsupportedAccess,
            "Step 4 ProxyMessageRequest over PASE",
            node_id=pase_node_id)

        # ----------------------------------------------------------------
        # Step 5: ProxyDisconnectRequest over PASE — UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(5)
        logger.info("Step 5: ProxyDisconnectRequest over PASE (expect UNSUPPORTED_ACCESS)")
        await self.expect_command_rejected(
            cp.Commands.ProxyDisconnectRequest(sessionID=0x0001),
            Status.UnsupportedAccess,
            "Step 5 ProxyDisconnectRequest over PASE",
            node_id=pase_node_id)

        # ================================================================
        # Phase 2 — DUT commissioned to TH: error handling over CASE
        # ================================================================

        # Step 6 — commission the DUT by reusing the on-network PASE from step 2
        # (do NOT expire it; commission_dut_in_test runs commissioning over it).
        self.step(6)
        await self.commission_dut_in_test()
        logger.info("Step 6: DUT commissioned; CASE available (dut_node_id=0x%016x)",
                    self.dut_node_id)

        # Step 7 — read Transport; verify at least one defined transport bit
        # (BLE/WiFiPAF/NTL) is set and no reserved bits are set.
        self.step(7)
        valid_transports = await self.read_transport()
        logger.info("Step 7: valid_transports = 0x%02x", valid_transports)
        self.assert_transport_value_valid(valid_transports)

        # Step 8 — read WiFiBand (conditional on WI feature)
        feature_map = await self.read_feature_map()
        has_wi = self.has_feature_wi(feature_map)
        valid_bands: int = 0
        if has_wi:
            self.step(8)
            valid_bands = await self.read_wifi_band()
            logger.info("Step 8: valid_bands = 0x%04x", valid_bands)
        else:
            self.skip_step(8)

        single_band = self.pick_single_transport_bit(valid_bands) if has_wi else None

        # For step 11 (valid session), select the transport that matches the
        # actual ED transport type.  pick_single_transport_bit() returns the
        # lowest set bit (kBle=0x02 when both are advertised), which would cause
        # the server to reject a wiFiBand argument with INVALID_COMMAND.
        ed_transport_type = (ed.ed_transport if ed is not None
                             else params.get('ed_transport', 'wifipaf'))
        proxy_transport = self.pick_proxy_transport(valid_transports, ed_transport_type)
        proxy_wifi_band = (single_band
                           if proxy_transport == int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF)
                           else None)

        # ----------------------------------------------------------------
        # Step 9: ProxyConnectRequest with multi-bit transport — INVALID_COMMAND
        # Use both defined transport bits so the value always has >1 bit set.
        # ----------------------------------------------------------------
        self.step(9)
        multi_transport = (int(cp.Bitmaps.CapabilitiesBitmap.kBle) |
                           int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF))
        logger.info("Step 9: ProxyConnectRequest multi-transport=0x%02x (expect INVALID_COMMAND)",
                    multi_transport)
        await self.expect_command_rejected(
            cp.Commands.ProxyConnectRequest(
                address=NullValue,
                transport=multi_transport,
                discriminator=ed_discriminator,
                vendorID=0,
                productID=0,
                timeout=30,
            ),
            Status.InvalidCommand,
            "Step 9 ProxyConnectRequest with a multi-bit transport",
            timeout_ms=10000)

        # ----------------------------------------------------------------
        # Step 10: ProxyMessageRequest with non-existent SessionID — NOT_FOUND
        # ----------------------------------------------------------------
        self.step(10)
        logger.info("Step 10: ProxyMessageRequest sessionID=0xFFFE (expect NOT_FOUND)")
        await self.expect_command_rejected(
            cp.Commands.ProxyMessageRequest(
                sessionID=0xFFFE,
                responseTimeout=10,
                message=_MINIMAL_MATTER_MSG,
            ),
            Status.NotFound,
            "Step 10 ProxyMessageRequest with a non-existent sessionID",
            timeout_ms=15000)

        # ----------------------------------------------------------------
        # Step 11: ProxyConnectRequest — establish session_a
        # ----------------------------------------------------------------
        self.step(11)
        logger.info("Step 11: ProxyConnectRequest (transport=0x%02x discriminator=%d)",
                    proxy_transport, ed_discriminator)
        connect_response = await self.send_cp_command(
            cp.Commands.ProxyConnectRequest(
                address=NullValue,
                transport=proxy_transport,
                discriminator=ed_discriminator,
                vendorID=0,
                productID=0,
                timeout=proxy_connect_timeout,
                wiFiBand=proxy_wifi_band,
            ),
            # ProxyConnect is async: the CP holds the invoke open until the
            # transport session establishes. The invoke wait is governed by
            # interactionTimeoutMs, so it must cover proxy_connect_timeout;
            # otherwise the invoke times out (~10.5s MRP default) before a
            # slower-but-valid connect completes.
            timeout_ms=proxy_connect_timeout * 1000 + 10000)
        session_a = connect_response.sessionID
        asserts.assert_true(
            0x0001 <= session_a <= 0xFFFE,
            f"session_a {session_a:#06x} must be in range 0x0001–0xFFFE")
        logger.info("Step 11: session_a = %d (0x%04x)", session_a, session_a)

        # ----------------------------------------------------------------
        # Step 12: BUSY — two concurrent ProxyMessageRequests for session_a.
        #
        # _first_msg sends with a short ResponseTimeout and hangs waiting for
        # the commissionee reply.  The ResponseTimeout must be shorter than the
        # transport's keep-alive/ack window so the message resolves with TIMEOUT
        # while the proxy session stays open (letting step 13 disconnect it);
        # otherwise the transport's ack timer could close the session first.
        # _second_msg waits 2 s (ensuring the first is in-flight) then sends an
        # identical request; the DUT MUST reject it with BUSY.  asyncio.gather
        # waits for both to finish.
        # ----------------------------------------------------------------
        self.step(12)

        async def _first_msg():
            try:
                return await self.send_cp_command(
                    cp.Commands.ProxyMessageRequest(
                        sessionID=session_a,
                        responseTimeout=10,
                        message=_MINIMAL_MATTER_MSG,
                    ),
                    timeout_ms=35000)
            except InteractionModelError as e:
                return e

        async def _second_msg():
            await asyncio.sleep(2)  # let the first request reach the DUT and go in-flight
            return await self.expect_command_rejected(
                cp.Commands.ProxyMessageRequest(
                    sessionID=session_a,
                    responseTimeout=10,
                    message=_MINIMAL_MATTER_MSG,
                ),
                Status.Busy,
                "Step 12 duplicate in-flight ProxyMessageRequest",
                timeout_ms=5000)

        first_result, _second_result = await asyncio.gather(_first_msg(), _second_msg())
        logger.info("Step 12: first ProxyMessageRequest resolved with %s",
                    first_result.status if isinstance(first_result, InteractionModelError)
                    else "SUCCESS")

        # ----------------------------------------------------------------
        # Step 13: ProxyDisconnectRequest(session_a) — SUCCESS
        # ----------------------------------------------------------------
        self.step(13)
        logger.info("Step 13: ProxyDisconnectRequest(sessionID=%d)", session_a)
        await self.send_cp_command(
            cp.Commands.ProxyDisconnectRequest(sessionID=session_a))
        logger.info("Step 13: ProxyDisconnectRequest succeeded for session_a=%d", session_a)

        # Cleanup
        await self.ensure_ed_not_commissionable(
            ed,
            manual_prompt=(
                "Test complete. Power off or factory-reset the End Device so it stops "
                "advertising. Press Enter when done."
            ),
        )


if __name__ == "__main__":
    default_matter_test_main()
