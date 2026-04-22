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
#       --string-arg ed_app_path:${ED_APP} wifi_ssid:MyNetwork wifi_password:MyPassword
#       --int-arg ed_discriminator:3841 ed_passcode:20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""TC-COMPRO-2.4 — Commissioning Proxy cluster: Proxy Connect, Message and Disconnect.

The Python TH drives the full commissioning tunnel:

  Steps 1–6:  Commission DUT; make ED commissionable; read attributes; send
              ProxyConnectRequest; tunnel PASE + commissioning via
              ProxyMessageRequest/Response until ED is on the fabric.
  Step 7:     ProxyMessageRequest with ResponseTimeout=0 — DUT MUST respond
              immediately without waiting for the commissionee reply.
  Steps 8–9:  ProxyDisconnect with invalid and valid SessionIds.
  Step 10:    ProxyConnectRequest with Timeout=1 (ED not commissionable) — DUT
              MUST return TIMEOUT after the timeout fires.
  Steps 11–12: ProxyConnectRequest negative transport/band validation.
  Step 13:    ProxyMessageRequest with a non-existent SessionId — DUT MUST return
              NOT_FOUND.
  Step 14:    ProxyDisconnectRequest(SessionId=0xFFFF) to cancel an in-flight
              ProxyConnectRequest — DUT MUST return SUCCESS (or INVALID_IN_STATE
              on a race).

ED control modes
-----------------
Automated (recommended for CI / RPi lab):
  Provide ``ed_app_path`` (and optionally ``ed_ssh_host`` for a remote RPi).  The test
  starts and stops the ED app automatically.

Manual (physical hardware, WiFiPAF-only device):
  Omit ``ed_app_path``.  The test will pause at step 2 and prompt the operator to make
  the ED commissionable (e.g. power-cycle or press the commissioning button).  At the
  end of the test, the operator is prompted to confirm the ED has stopped advertising.

User-defined arguments (all via --string-arg / --int-arg NAME:VALUE):
  wifi_ssid              — Wi-Fi SSID to provision on the ED (required for step 6)
  wifi_password          — Wi-Fi password (default: empty string)
  ed_discriminator       — ED commissionable discriminator (default: 3841)
  ed_passcode            — ED PASE passcode (default: 20202021)
  ed_app_path            — path to the ED binary; if omitted the test prompts the operator
  ed_ssh_host            — IP/hostname of a remote host on which to run the ED binary over SSH
  ed_ssh_user            — SSH username for ed_ssh_host (default: ubuntu)
  ed_extra_args          — extra CLI args forwarded to the ED app
                           (e.g. "--wifi --wifipaf freq_list=2437")
  proxy_connect_timeout  — ProxyConnectRequest timeout in seconds (default: 120)

Test plan reference: TC-COMPRO-2.4

Example — automated with remote ED RPi:
    ```bash
    python3 TC_COMPRO_2_4.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 1 \\
        --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_ssh_host:192.168.1.10 \\
        --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --int-arg ed_discriminator:3841 ed_passcode:20202021
    ```

Example — manual with physical WiFiPAF-only hardware:
    ```bash
    python3 TC_COMPRO_2_4.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 1 \\
        --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
        --int-arg ed_discriminator:3841 ed_passcode:20202021
    ```
"""

import asyncio
import logging

from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout for ProxyConnectRequest (seconds)
CONNECT_TIMEOUT_S = 120

# Node ID assigned to the ED during proxy commissioning
ED_NODE_ID = 0x1001

# Minimal Matter MRP message header bytes (8 zero bytes) used for the
# ResponseTimeout=0 and non-existent-session negative tests.  The DUT
# with ResponseTimeout=0 forwards these bytes to the ED without waiting
# for a reply, so the content does not need to be a fully valid frame.
_MINIMAL_MATTER_MSG = bytes(8)


class TC_COMPRO_2_4(COMPROBaseTest):

    @property
    def default_timeout(self) -> int:
        # Step 5 ProxyConnect: up to proxy_connect_timeout (default 120 s) + margin
        # Step 6 CommissionViaProxy: PASE + commissioning steps ~30–60 s
        # Step 10 ProxyConnect Timeout=1: ~2 s
        # Step 14 asyncio.gather: cancel after ~2 s, connect resolves ~3 s later
        # Overhead for remaining steps: ~30 s
        return 300

    def desc_TC_COMPRO_2_4(self) -> str:
        return "[TC-COMPRO-2.4] Proxy Connect, Message and Disconnect feature functionality"

    def pics_TC_COMPRO_2_4(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT (proxy) to TH", is_commissioning=True),
            TestStep(2, "Ensure ED IS commissionable (advertising via WiFiPAF)"),
            TestStep(3, "TH reads Transport attribute", "Store as valid_transports"),
            TestStep(4, "TH reads WiFiBand attribute (if WI supported)", "Store as valid_bands"),
            TestStep(5, "TH sends ProxyConnectRequest with exactly one transport bit set",
                     "DUT returns ProxyConnectResponse with SUCCESS and a valid SessionId "
                     "(0x0001–0xFFFE); save as current_session_id"),
            TestStep(6, "TH acts as Commissioner and performs full commissioning flow by "
                     "tunneling PASE and commissioning traffic through the DUT",
                     "Commissioning procedure completes; ED is commissioned onto the fabric"),
            TestStep(7, "TH sends ProxyMessageRequest(SessionId=current_session_id, "
                     "ResponseTimeout=0, Message=<valid frame>)",
                     "DUT returns ProxyMessageResponse immediately with "
                     "SessionId=current_session_id; Message may be null"),
            TestStep(8, "TH sends ProxyDisconnectRequest(SessionId=<non-existent>)",
                     "DUT returns NOT_FOUND"),
            TestStep(9, "TH sends ProxyDisconnectRequest(SessionId=current_session_id)",
                     "DUT returns SUCCESS"),
            TestStep(10, "TH sends ProxyConnectRequest with Timeout=1 (ED not commissionable)",
                     "DUT returns TIMEOUT after approximately 1 second"),
            TestStep(11, "TH sends ProxyConnectRequest with an unsupported Transport bit",
                     "DUT returns INVALID_TRANSPORT_TYPE"),
            TestStep(12, "TH sends ProxyConnectRequest with WiFiPAF transport and a "
                     "WiFiBand not in valid_bands (if WI supported)",
                     "DUT returns INVALID_TRANSPORT_TYPE"),
            TestStep(13, "TH sends ProxyMessageRequest with a non-existent SessionId",
                     "DUT returns NOT_FOUND"),
            TestStep(14, "TH sends ProxyConnectRequest(Timeout=30), then immediately sends "
                     "ProxyDisconnectRequest(SessionId=0xFFFF) to cancel the pending connect",
                     "ProxyDisconnectRequest returns SUCCESS (or INVALID_IN_STATE on race); "
                     "ProxyConnectRequest resolves with an error"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_4(self):
        cp = self.cp
        params = getattr(self, 'user_params', {}) or {}
        ed = self._ed_fixture_from_params()

        ed_discriminator = int(params.get('ed_discriminator', 3841))
        ed_passcode = int(params.get('ed_passcode', 20202021))
        proxy_connect_timeout = int(params.get('proxy_connect_timeout', CONNECT_TIMEOUT_S))
        wifi_ssid = params.get('wifi_ssid')
        wifi_password = params.get('wifi_password', '')

        # Step 1 — commissioning of the DUT done by the framework
        self.step(1)

        # Step 2 — make ED commissionable.
        # The ED only needs to advertise via WiFiPAF; it does NOT need to be
        # reachable over Ethernet from the TH.
        self.step(2)
        await self.ensure_ed_commissionable(
            ed,
            manual_prompt=(
                f"Make the End Device commissionable via WiFiPAF "
                f"(discriminator={ed_discriminator}, passcode={ed_passcode}). "
                "The device does NOT need an Ethernet/IP connection — it will be "
                "discovered by the DUT over WiFiPAF. Press Enter when ready."
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

        # ----------------------------------------------------------------
        # Step 5: ProxyConnectRequest — exactly one transport bit from valid_transports
        # ----------------------------------------------------------------
        self.step(5)
        single_transport = self.pick_single_transport_bit(valid_transports)
        single_band = self.pick_single_transport_bit(valid_bands) if has_wi else None
        logger.info("Sending ProxyConnectRequest (transport=0x%02x discriminator=%d)",
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
            # interactionTimeoutMs is passed as c_uint16 in the SDK binding (max 65535 ms).
            # Values > 65535 wrap silently, so pass None to let the SDK auto-compute the
            # exchange timeout rather than risk underflowing for large proxy_connect_timeout.
            interactionTimeoutMs=None,
        )
        current_session_id = connect_response.sessionId
        asserts.assert_true(
            0x0001 <= current_session_id <= 0xFFFE,
            f"SessionId {current_session_id:#06x} must be in range 0x0001–0xFFFE")
        logger.info("ProxyConnectResponse: sessionId=%d", current_session_id)

        # ----------------------------------------------------------------
        # Step 6: Commission ED through the proxy tunnel opened in step 5.
        # The SDK routes PASE + commissioning packets via
        # ProxyMessageRequest/Response on current_session_id.
        # ----------------------------------------------------------------
        self.step(6)
        if wifi_ssid:
            self.default_controller.SetWiFiCredentials(wifi_ssid, wifi_password)

        logger.info("Commissioning ED via proxy (sessionId=%d nodeId=0x%04x discriminator=%d)",
                    current_session_id, ED_NODE_ID, ed_discriminator)
        await self.default_controller.CommissionViaProxy(
            proxyNodeId=self.dut_node_id,
            proxySessionId=current_session_id,
            remoteNodeId=ED_NODE_ID,
            discriminator=ed_discriminator,
            setupPinCode=ed_passcode,
            proxyEndpoint=self.cp_endpoint,
        )
        logger.info("ED commissioned successfully via proxy (nodeId=0x%04x)", ED_NODE_ID)

        # ----------------------------------------------------------------
        # Step 7: ProxyMessageRequest with ResponseTimeout=0.
        # The proxy session (current_session_id) is still live — we have
        # not yet sent ProxyDisconnectRequest.  ResponseTimeout=0 instructs
        # the DUT to forward the message to the ED and return immediately
        # without waiting for a Commissionee reply.
        # ----------------------------------------------------------------
        self.step(7)
        msg_response = await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyMessageRequest(
                sessionId=current_session_id,
                responseTimeout=0,
                message=_MINIMAL_MATTER_MSG,
            ),
            interactionTimeoutMs=5000,
        )
        asserts.assert_equal(
            msg_response.sessionId, current_session_id,
            "ProxyMessageResponse sessionId must match the request sessionId")
        logger.info("ProxyMessageResponse received immediately (ResponseTimeout=0): "
                    "sessionId=%d message=%s",
                    msg_response.sessionId,
                    "null" if msg_response.message is NullValue else f"{len(msg_response.message)} bytes")

        # ----------------------------------------------------------------
        # Step 8: ProxyDisconnect with a non-existent SessionId — NOT_FOUND.
        # current_session_id is still live (step 9 disconnects it).
        # ----------------------------------------------------------------
        self.step(8)
        non_existent_session_id = 0xFFFE  # guaranteed non-existent
        logger.info("Sending ProxyDisconnectRequest with non-existent sessionId=%d",
                    non_existent_session_id)
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyDisconnectRequest(sessionId=non_existent_session_id),
            )
            asserts.fail("Expected NOT_FOUND for non-existent sessionId but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND for non-existent sessionId, got {e.status}")
            logger.info("ProxyDisconnectRequest with invalid sessionId correctly returned NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 9: ProxyDisconnect with valid current_session_id — SUCCESS.
        # ----------------------------------------------------------------
        self.step(9)
        logger.info("Sending ProxyDisconnectRequest with valid sessionId=%d", current_session_id)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyDisconnectRequest(sessionId=current_session_id),
        )
        logger.info("ProxyDisconnectRequest succeeded for sessionId=%d", current_session_id)

        # ----------------------------------------------------------------
        # Step 10: ProxyConnectRequest with Timeout=1 while ED is not
        # commissionable (it was commissioned in step 6 and is no longer
        # advertising via WiFiPAF).  The DUT MUST return TIMEOUT after
        # ~1 second.
        # ----------------------------------------------------------------
        self.step(10)
        logger.info("Sending ProxyConnectRequest(Timeout=1) — expecting TIMEOUT")
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=single_transport,
                    discriminator=ed_discriminator,
                    vendorId=0,
                    productId=0,
                    timeout=1,
                    wiFiBand=single_band,
                ),
                interactionTimeoutMs=6000,  # 1 s DUT timeout + 5 s IM margin
            )
            asserts.fail("Expected TIMEOUT but ProxyConnectRequest succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Timeout,
                                 f"Expected TIMEOUT for short-timeout connect, got {e.status}")
            logger.info("ProxyConnectRequest(Timeout=1) correctly returned TIMEOUT")

        # ----------------------------------------------------------------
        # Step 11: ProxyConnectRequest with unsupported Transport bit.
        # Find the first defined transport bit absent from valid_transports.
        # ----------------------------------------------------------------
        defined_transports = [
            int(cp.Bitmaps.CapabilitiesBitmap.kBle),
            int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF),
        ]
        unsupported_transport = next(
            (b for b in defined_transports if not (valid_transports & b)), None)
        if unsupported_transport is None:
            self.skip_step(11)
            logger.info("Step 11 skipped: all defined transport bits are in valid_transports")
        else:
            self.step(11)
            logger.info("Using unsupported transport bit 0x%02x for step 11", unsupported_transport)
            try:
                await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyConnectRequest(
                        address=NullValue,
                        transport=unsupported_transport,
                        discriminator=ed_discriminator,
                        vendorId=0,
                        productId=0,
                        timeout=30,
                    ),
                    interactionTimeoutMs=10000,
                )
                asserts.fail("Expected INVALID_TRANSPORT_TYPE but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidTransportType,
                                     f"Expected INVALID_TRANSPORT_TYPE, got {e.status}")
                logger.info("Got expected INVALID_TRANSPORT_TYPE for unsupported transport")

        # ----------------------------------------------------------------
        # Step 12: ProxyConnectRequest with WiFiPAF transport and a WiFiBand
        # not in valid_bands (WI feature only).
        # ----------------------------------------------------------------
        if not has_wi:
            self.skip_step(12)
        else:
            defined_bands = [
                int(cp.Bitmaps.WiFiBandBitmap.k5g),
                int(cp.Bitmaps.WiFiBandBitmap.k2g4),
            ]
            invalid_band = next((b for b in defined_bands if not (valid_bands & b)), None)
            if invalid_band is None:
                self.skip_step(12)
                logger.info("Step 12 skipped: all defined WiFiBand bits are in valid_bands")
            else:
                self.step(12)
                logger.info("Using invalid WiFiBand bit 0x%04x for step 12", invalid_band)
                try:
                    await self.default_controller.SendCommand(
                        nodeId=self.dut_node_id,
                        endpoint=self.cp_endpoint,
                        payload=cp.Commands.ProxyConnectRequest(
                            address=NullValue,
                            transport=single_transport,
                            discriminator=ed_discriminator,
                            vendorId=0,
                            productId=0,
                            timeout=30,
                            wiFiBand=invalid_band,
                        ),
                        interactionTimeoutMs=10000,
                    )
                    asserts.fail("Expected INVALID_TRANSPORT_TYPE but command succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.InvalidTransportType,
                                         f"Expected INVALID_TRANSPORT_TYPE, got {e.status}")
                    logger.info("Got expected INVALID_TRANSPORT_TYPE for invalid WiFiBand")

        # ----------------------------------------------------------------
        # Step 13: ProxyMessageRequest referencing a non-existent SessionId.
        # ----------------------------------------------------------------
        self.step(13)
        logger.info("Sending ProxyMessageRequest with non-existent sessionId=0xFFFE")
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
            asserts.fail("Expected NOT_FOUND for non-existent sessionId but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND for non-existent sessionId, got {e.status}")
            logger.info("ProxyMessageRequest with non-existent sessionId correctly returned NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 14: ProxyDisconnectRequest(SessionId=0xFFFF) to cancel an
        # in-flight ProxyConnectRequest.
        #
        # Two coroutines run concurrently via asyncio.gather (same pattern as
        # the concurrent ProxyScanRequest test in TC_COMPRO_2_2):
        #   1. _pending_connect  — sends ProxyConnectRequest(Timeout=30).
        #      ED is not commissionable so this hangs for up to 30 s.
        #   2. _cancel_pending   — waits 2 s then sends
        #      ProxyDisconnectRequest(SessionId=0xFFFF) to cancel it.
        #
        # Expected:
        #   _cancel_pending  → SUCCESS (or INVALID_IN_STATE on race)
        #   _pending_connect → InteractionModelError (connect was cancelled)
        # ----------------------------------------------------------------
        self.step(14)

        async def _pending_connect():
            try:
                return await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
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
                    interactionTimeoutMs=35000,
                )
            except InteractionModelError as e:
                return e

        async def _cancel_pending():
            await asyncio.sleep(2)  # allow the connect request to reach the DUT first
            try:
                await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyDisconnectRequest(sessionId=0xFFFF),
                    interactionTimeoutMs=5000,
                )
                return "SUCCESS"
            except InteractionModelError as e:
                return e

        connect_result, cancel_result = await asyncio.gather(
            _pending_connect(), _cancel_pending())

        # Validate cancel outcome: SUCCESS or INVALID_IN_STATE (race)
        if isinstance(cancel_result, str):
            logger.info("Step 14: ProxyDisconnect(0xFFFF) returned SUCCESS (pending connect cancelled)")
        elif isinstance(cancel_result, InteractionModelError):
            asserts.assert_equal(
                cancel_result.status, Status.InvalidInState,
                f"ProxyDisconnect(0xFFFF) returned unexpected status {cancel_result.status}; "
                "expected SUCCESS or INVALID_IN_STATE")
            logger.info("Step 14: ProxyDisconnect(0xFFFF) returned INVALID_IN_STATE (race: connect already responded)")
        else:
            asserts.fail(f"Step 14: unexpected cancel_result type {type(cancel_result)}")

        # Validate connect outcome: must not succeed (it was cancelled or timed out)
        asserts.assert_true(
            isinstance(connect_result, InteractionModelError),
            f"ProxyConnectRequest must not succeed when cancelled via 0xFFFF; got {connect_result}")
        logger.info("Step 14: ProxyConnectRequest resolved with %s (expected)", connect_result.status)

        # Cleanup
        await self.ensure_ed_not_commissionable(
            ed,
            manual_prompt=(
                "Test complete. Power off or factory-reset the End Device so it stops "
                "advertising and is ready for the next test run. Press Enter when done."
            ),
        )


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
