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

"""TC-COMPRO-2.6 — Commissioning Proxy cluster: Error Handling with DUT as Server.

Verifies that the Commissioning Proxy server correctly rejects all invalid
command scenarios:

  Steps 5, 7, 9:  Commands sent over a PASE session (non-CASE) must be
                  rejected with UNSUPPORTED_ACCESS.
  Step 6:         ProxyConnectRequest with more than one transport bit set
                  must be rejected with INVALID_COMMAND.
  Step 8:         ProxyMessageRequest for a non-existent SessionId must be
                  rejected with NOT_FOUND.
  Step 10:        ProxyConnectRequest to establish a valid session (session_a).
  Step 11:        A second concurrent ProxyMessageRequest for session_a, sent
                  while the first is still in-flight, must be rejected with BUSY.
  Step 12:        ProxyDisconnectRequest(session_a) must return SUCCESS.

Test plan reference: TC-COMPRO-2.6

Example — automated with remote ED RPi:
    ```bash
    python3 TC_COMPRO_2_6.py \\
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
"""

import asyncio
import logging

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main
from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

logger = logging.getLogger(__name__)

# Timeout for ProxyConnectRequest (seconds)
CONNECT_TIMEOUT_S = 120

# Minimal 8-byte Matter message used for negative-path ProxyMessageRequest tests.
_MINIMAL_MATTER_MSG = bytes(8)


class TC_COMPRO_2_6(COMPROBaseTest):

    @property
    def default_timeout(self) -> int:
        # Step 10 ProxyConnect: up to proxy_connect_timeout (default 120 s) + margin
        # Step 11 BUSY test: ResponseTimeout=30 s + 5 s IM margin + 2 s sleep
        # PASE window operations + other steps: ~30 s
        return 300

    def desc_TC_COMPRO_2_6(self) -> str:
        return "[TC-COMPRO-2.6] Error Handling with DUT as Server"

    def pics_TC_COMPRO_2_6(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT (proxy) to TH", is_commissioning=True),
            TestStep(2, "Ensure ED is commissionable (advertising via WiFiPAF)"),
            TestStep(3, "TH reads Transport attribute", "Store as valid_transports"),
            TestStep(4, "TH reads WiFiBand attribute (if WI supported)", "Store as valid_bands"),
            TestStep(5, "TH sends ProxyConnectRequest over a non-CASE (PASE) session",
                     "DUT returns UNSUPPORTED_ACCESS"),
            TestStep(6, "TH sends ProxyConnectRequest with more than one transport bit set",
                     "DUT returns INVALID_COMMAND"),
            TestStep(7, "TH sends ProxyMessageRequest over a non-CASE (PASE) session",
                     "DUT returns UNSUPPORTED_ACCESS"),
            TestStep(8, "TH sends ProxyMessageRequest with SessionId=0xFFFE (no active session)",
                     "DUT returns NOT_FOUND"),
            TestStep(9, "TH sends ProxyDisconnectRequest over a non-CASE (PASE) session",
                     "DUT returns UNSUPPORTED_ACCESS"),
            TestStep(10, "TH sends ProxyConnectRequest to establish a valid session",
                     "DUT returns ProxyConnectResponse with SUCCESS; save session_id as session_a "
                     "(0x0001–0xFFFE)"),
            TestStep(11, "TH sends two concurrent ProxyMessageRequests for session_a; "
                     "the second is sent while the first is still in-flight",
                     "The second ProxyMessageRequest is rejected with BUSY"),
            TestStep(12, "TH sends ProxyDisconnectRequest(SessionId=session_a)",
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

        # Step 1 — commissioning done by the framework
        self.step(1)

        # Step 2 — make ED commissionable (needed for step 10)
        self.step(2)
        await self.ensure_ed_commissionable(
            ed,
            manual_prompt=(
                f"Make the End Device commissionable via WiFiPAF "
                f"(discriminator={ed_discriminator}, passcode={ed_passcode}). "
                "Press Enter when ready."
            ),
        )

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

        single_transport = self.pick_single_transport_bit(valid_transports)
        single_band = self.pick_single_transport_bit(valid_bands) if has_wi else None

        # ----------------------------------------------------------------
        # Open a commissioning window on the DUT to obtain a PASE session.
        # This PASE node ID is used for steps 5, 7, and 9.
        # ----------------------------------------------------------------
        window_params = await self.open_commissioning_window()
        pase_node_id = self.dut_node_id + 1
        await self.default_controller.FindOrEstablishPASESession(
            setupCode=window_params.commissioningParameters.setupQRCode,
            nodeId=pase_node_id,
        )
        logger.info("PASE session established (pase_node_id=0x%016x)", pase_node_id)

        # ----------------------------------------------------------------
        # Step 5: ProxyConnectRequest over PASE — UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(5)
        logger.info("Step 5: ProxyConnectRequest over PASE (expect UNSUPPORTED_ACCESS)")
        try:
            await self.default_controller.SendCommand(
                nodeId=pase_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=single_transport,
                    discriminator=ed_discriminator,
                    vendorId=0,
                    productId=0,
                    timeout=30,
                    wiFiBand=single_band,
                ),
            )
            asserts.fail("Expected UNSUPPORTED_ACCESS but ProxyConnectRequest over PASE succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 f"Expected UNSUPPORTED_ACCESS for PASE ProxyConnectRequest, got {e.status}")
            logger.info("Step 5: correctly got UNSUPPORTED_ACCESS")

        # ----------------------------------------------------------------
        # Step 6: ProxyConnectRequest with multi-bit transport — INVALID_COMMAND
        # Use both defined transport bits so the value always has >1 bit set.
        # ----------------------------------------------------------------
        self.step(6)
        multi_transport = (int(cp.Bitmaps.CapabilitiesBitmap.kBle) |
                           int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF))
        logger.info("Step 6: ProxyConnectRequest multi-transport=0x%02x (expect INVALID_COMMAND)",
                    multi_transport)
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=multi_transport,
                    discriminator=ed_discriminator,
                    vendorId=0,
                    productId=0,
                    timeout=30,
                ),
                interactionTimeoutMs=10000,
            )
            asserts.fail("Expected INVALID_COMMAND but ProxyConnectRequest with multi-bit transport succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 f"Expected INVALID_COMMAND for multi-bit transport, got {e.status}")
            logger.info("Step 6: correctly got INVALID_COMMAND")

        # ----------------------------------------------------------------
        # Step 7: ProxyMessageRequest over PASE — UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(7)
        logger.info("Step 7: ProxyMessageRequest over PASE (expect UNSUPPORTED_ACCESS)")
        try:
            await self.default_controller.SendCommand(
                nodeId=pase_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyMessageRequest(
                    sessionId=0x0001,
                    responseTimeout=10,
                    message=_MINIMAL_MATTER_MSG,
                ),
            )
            asserts.fail("Expected UNSUPPORTED_ACCESS but ProxyMessageRequest over PASE succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 f"Expected UNSUPPORTED_ACCESS for PASE ProxyMessageRequest, got {e.status}")
            logger.info("Step 7: correctly got UNSUPPORTED_ACCESS")

        # ----------------------------------------------------------------
        # Step 8: ProxyMessageRequest with non-existent SessionId — NOT_FOUND
        # ----------------------------------------------------------------
        self.step(8)
        logger.info("Step 8: ProxyMessageRequest sessionId=0xFFFE (expect NOT_FOUND)")
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyMessageRequest(
                    sessionId=0xFFFE,
                    responseTimeout=10,
                    message=_MINIMAL_MATTER_MSG,
                ),
                interactionTimeoutMs=15000,
            )
            asserts.fail("Expected NOT_FOUND but ProxyMessageRequest with invalid sessionId succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND for non-existent sessionId, got {e.status}")
            logger.info("Step 8: correctly got NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 9: ProxyDisconnectRequest over PASE — UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(9)
        logger.info("Step 9: ProxyDisconnectRequest over PASE (expect UNSUPPORTED_ACCESS)")
        try:
            await self.default_controller.SendCommand(
                nodeId=pase_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyDisconnectRequest(sessionId=0x0001),
            )
            asserts.fail("Expected UNSUPPORTED_ACCESS but ProxyDisconnectRequest over PASE succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 f"Expected UNSUPPORTED_ACCESS for PASE ProxyDisconnectRequest, got {e.status}")
            logger.info("Step 9: correctly got UNSUPPORTED_ACCESS")

        # Close the commissioning window now that PASE steps are complete.
        logger.info("Revoking commissioning window")
        await self.send_single_cmd(
            Clusters.AdministratorCommissioning.Commands.RevokeCommissioning(),
            endpoint=0,
            timedRequestTimeoutMs=5000,
        )

        # ----------------------------------------------------------------
        # Step 10: ProxyConnectRequest — establish session_a
        # ----------------------------------------------------------------
        self.step(10)
        logger.info("Step 10: ProxyConnectRequest (transport=0x%02x discriminator=%d)",
                    single_transport, ed_discriminator)
        connect_response = await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyConnectRequest(
                address=NullValue,
                transport=single_transport,
                discriminator=ed_discriminator,
                vendorId=0,
                productId=0,
                timeout=proxy_connect_timeout,
                wiFiBand=single_band,
            ),
            interactionTimeoutMs=None,
        )
        session_a = connect_response.sessionId
        asserts.assert_true(
            0x0001 <= session_a <= 0xFFFE,
            f"session_a {session_a:#06x} must be in range 0x0001–0xFFFE")
        logger.info("Step 10: session_a = %d (0x%04x)", session_a, session_a)

        # ----------------------------------------------------------------
        # Step 11: BUSY — two concurrent ProxyMessageRequests for session_a.
        #
        # _first_msg sends with ResponseTimeout=30 and hangs waiting for the
        # commissione.e reply (up to 30 s).  _second_msg waits 2 s (ensuring
        # the first is in-flight) then sends an identical request; the DUT
        # MUST reject it with BUSY.  asyncio.gather waits for both to finish.
        # ----------------------------------------------------------------
        self.step(11)

        async def _first_msg():
            try:
                return await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyMessageRequest(
                        sessionId=session_a,
                        responseTimeout=30,
                        message=_MINIMAL_MATTER_MSG,
                    ),
                    interactionTimeoutMs=35000,
                )
            except InteractionModelError as e:
                return e

        async def _second_msg():
            await asyncio.sleep(2)  # let the first request reach the DUT and go in-flight
            try:
                await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyMessageRequest(
                        sessionId=session_a,
                        responseTimeout=30,
                        message=_MINIMAL_MATTER_MSG,
                    ),
                    interactionTimeoutMs=5000,
                )
                asserts.fail("Expected BUSY for concurrent ProxyMessageRequest but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Busy,
                                     f"Expected BUSY for duplicate in-flight ProxyMessageRequest, got {e.status}")
                logger.info("Step 11: second ProxyMessageRequest correctly got BUSY")
                return e

        first_result, _second_result = await asyncio.gather(_first_msg(), _second_msg())
        logger.info("Step 11: first ProxyMessageRequest resolved with %s",
                    first_result.status if isinstance(first_result, InteractionModelError)
                    else "SUCCESS")

        # ----------------------------------------------------------------
        # Step 12: ProxyDisconnectRequest(session_a) — SUCCESS
        # ----------------------------------------------------------------
        self.step(12)
        logger.info("Step 12: ProxyDisconnectRequest(sessionId=%d)", session_a)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyDisconnectRequest(sessionId=session_a),
        )
        logger.info("Step 12: ProxyDisconnectRequest succeeded for session_a=%d", session_a)

        # Cleanup
        await self.ensure_ed_not_commissionable(
            ed,
            manual_prompt=(
                "Test complete. Power off or factory-reset the End Device so it stops "
                "advertising. Press Enter when done."
            ),
        )


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
