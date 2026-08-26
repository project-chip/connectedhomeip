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
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 5
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --string-arg ed_app_path:${ED_APP} wifi_ssid:MyNetwork wifi_password:MyPassword
#       --int-arg ed_discriminator:3841 ed_passcode:20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""TC-COMPRO-2.4 — Commissioning Proxy cluster: Proxy Connect, Message and Disconnect.

For the test rig topology, the Python wheel requirement and how to run this
suite, see ``support_modules/compro_support.py``.

The Python TH drives the full commissioning tunnel for each transport advertised
by the DUT.  Steps 4–9 iterate once per transport bit in valid_transports
(WiFiPAF first, then BLE).

  Steps 1–3:  Commission DUT; read Transport and WiFiBand attributes.
  Steps 4–9:  For each transport bit in valid_transports:
                Make ED commissionable; send ProxyConnectRequest; send
                ProxyMessageRequest(ResponseTimeout=0, Message=null) and verify an
                immediate ProxyMessageResponse; tunnel PASE + commissioning via
                ProxyMessageRequest/Response until ED is on the fabric; send
                ProxyDisconnectRequest with invalid and valid SessionIds.
  Step 10:    ProxyConnectRequest with Timeout=1 (ED not commissionable) — DUT
              MUST return TIMEOUT after the timeout fires.
  Steps 11–12: ProxyConnectRequest negative transport/band validation.
  Step 13:    ProxyMessageRequest with a non-existent SessionID — DUT MUST return
              NOT_FOUND.
  Step 14:    ProxyDisconnectRequest(SessionID=null) to cancel an in-flight
              ProxyConnectRequest — DUT MUST return SUCCESS (or INVALID_IN_STATE
              on a race).

ED control modes
-----------------
Automated (recommended for CI / RPi lab):
  Provide ``ed_app_path`` (and optionally ``ed_serial_port`` to drive an ED whose
  Ethernet is disconnected over its UART login console).  The test starts and stops
  the ED app automatically for each transport iteration.

Manual (physical hardware):
  Omit ``ed_app_path``.  The test will pause at each iteration and prompt the operator
  to make the ED commissionable via the transport being tested.

User-defined arguments (all via --string-arg / --int-arg NAME:VALUE):
  wifi_ssid              — Wi-Fi SSID to provision on the ED (required for step 5)
  wifi_password          — Wi-Fi password (default: empty string)
  ed_discriminator       — ED commissionable discriminator (default: 3841)
  ed_passcode            — ED PASE passcode (default: 20202021)
  ed_app_path            — path to the ED binary; if omitted the test prompts the operator
  ed_serial_port         — serial port of the ED's UART login console; when set the ED
                           binary is started/stopped over that console instead of locally
  ed_extra_args          — fallback extra CLI args for the ED app when transport-specific
                           args are not provided
  wifipaf_ed_extra_args  — extra CLI args for the ED when testing the WiFiPAF transport
                           (e.g. "--wifi --wifipaf freq_list=2437")
                           Falls back to ed_extra_args when absent.
  ble_ed_extra_args      — extra CLI args for the ED when testing the BLE transport
                           (e.g. "--wifi")
                           Falls back to ed_extra_args when absent.
  ed_transport           — single-transport fallback: 'wifipaf' (default) or 'ble'.
                           Ignored when valid_transports has multiple bits set.
  proxy_connect_timeout  — ProxyConnectRequest timeout in seconds (default: 120)

Test plan reference: TC-COMPRO-2.4

Example — both transports, serial-driven ED RPi:
    ```bash
    python3 TC_COMPRO_2_4.py \\
        --commissioning-method on-network \\
        --discriminator 3947 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 5 \\
        --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_serial_port:/dev/ttyUSB0 \\
        --string-arg 'wifipaf_ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --string-arg 'ble_ed_extra_args:--wifi' \\
        --int-arg ed_discriminator:3840 ed_passcode:20202021
    ```

Example — WiFiPAF only (single transport), serial-driven ED RPi:
    ```bash
    python3 TC_COMPRO_2_4.py \\
        --commissioning-method on-network \\
        --discriminator 3947 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 5 \\
        --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_serial_port:/dev/ttyUSB0 \\
        --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --int-arg ed_discriminator:3840 ed_passcode:20202021
    ```
"""

import asyncio
import logging

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, EDFixture, commission_if_needed

from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout for ProxyConnectRequest (seconds)
CONNECT_TIMEOUT_S = 120

# Base node ID for EDs commissioned via proxy; each transport iteration uses
# ED_NODE_ID + iteration_index to avoid fabric storage collisions.
ED_NODE_ID = 0x1001

# Minimal placeholder message (8 zero bytes) for the non-existent-session
# negative test (step 13).  This is NOT a well-formed Matter frame; it is
# sufficient only because the DUT rejects the request (NOT_FOUND) before parsing
# it.  (Step 6's ResponseTimeout=0 test uses a null Message, not this.)
_MINIMAL_MATTER_MSG = bytes(8)


class TC_COMPRO_2_4(COMPROBaseTest):

    @property
    def default_timeout(self) -> int:
        # Steps 4–9 run once per transport (up to 2 iterations):
        #   Step 5 ProxyConnect: proxy_connect_timeout (default 120 s) + margin
        #   Step 6 CommissionViaProxy: PASE + commissioning ~30–60 s
        # Step 10 ProxyConnect Timeout=1: ~2 s
        # Step 14 asyncio.gather: ~5 s
        # 2 iterations × ~200 s + steps 10–14 overhead ~30 s
        return 600

    def desc_TC_COMPRO_2_4(self) -> str:
        return "[TC-COMPRO-2.4] Proxy Connect, Message and Disconnect feature functionality"

    def pics_TC_COMPRO_2_4(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT (proxy) to TH", is_commissioning=True),
            TestStep(2, "TH reads Transport attribute", "Store as valid_transports"),
            TestStep(3, "TH reads WiFiBand attribute (if WI supported)", "Store as valid_bands"),
            TestStep(4, "For each transport bit in valid_transports: Ensure ED is commissionable "
                     "via that transport",
                     "ED is advertising via the transport under test"),
            TestStep(5, "For each transport bit in valid_transports: TH sends "
                     "ProxyConnectRequest with that transport bit set",
                     "DUT returns ProxyConnectResponse with SUCCESS and a SessionID "
                     "≤ 0xFFFE (max 65534); save as current_session_id"),
            TestStep(6, "For each transport bit in valid_transports: TH sends "
                     "ProxyMessageRequest(SessionID=current_session_id, ResponseTimeout=0, "
                     "Message=null)",
                     "DUT returns ProxyMessageResponse immediately (without waiting for a "
                     "Commissionee reply) with SessionID=current_session_id; Message may be "
                     "null or a queued message"),
            TestStep(7, "For each transport bit in valid_transports: TH acts as Commissioner "
                     "and performs full commissioning flow by tunneling PASE and commissioning "
                     "traffic through the DUT",
                     "Commissioning procedure completes; ED is commissioned onto the fabric"),
            TestStep(8, "For each transport bit in valid_transports: TH sends "
                     "ProxyDisconnectRequest(SessionID=<non-existent>)",
                     "DUT returns NOT_FOUND"),
            TestStep(9, "For each transport bit in valid_transports: TH sends "
                     "ProxyDisconnectRequest(SessionID=current_session_id)",
                     "DUT returns SUCCESS"),
            TestStep(10, "TH sends ProxyConnectRequest with Timeout=1 (ED not commissionable)",
                     "DUT returns TIMEOUT after approximately 1 second"),
            TestStep(11, "TH sends ProxyConnectRequest with an unsupported Transport bit",
                     "DUT returns INVALID_TRANSPORT_TYPE"),
            TestStep(12, "TH sends ProxyConnectRequest with WiFiPAF transport and a "
                     "WiFiBand not in valid_bands (if WI supported)",
                     "DUT returns INVALID_TRANSPORT_TYPE"),
            TestStep(13, "TH sends ProxyMessageRequest with a non-existent SessionID",
                     "DUT returns NOT_FOUND"),
            TestStep(14, "TH sends ProxyConnectRequest(Timeout=30), then immediately sends "
                     "ProxyDisconnectRequest(SessionID=null) to cancel the pending connect",
                     "ProxyDisconnectRequest returns SUCCESS (or INVALID_IN_STATE on race); "
                     "ProxyConnectRequest resolves with an error"),
        ]

    def _ed_fixture_for_transport(
        self,
        transport_bit: int,
        params: dict,
        ed_extra_args_fallback: str,
    ) -> 'EDFixture | None':
        """Build an EDFixture configured for the given transport bit."""
        app_path = params.get('ed_app_path')
        if not app_path:
            return None
        kWiFiPAF = int(self.cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF)
        is_wifipaf = (transport_bit == kWiFiPAF)
        if is_wifipaf:
            extra_args = params.get('wifipaf_ed_extra_args', ed_extra_args_fallback)
            ed_transport = 'wifipaf'
        else:
            extra_args = params.get('ble_ed_extra_args', ed_extra_args_fallback)
            ed_transport = 'ble'
        return self.track_ed(EDFixture(
            app_path=app_path,
            discriminator=int(params.get('ed_discriminator', 3841)),
            passcode=int(params.get('ed_passcode', 20202021)),
            extra_args=extra_args,
            ed_transport=ed_transport,
            serial_port=params.get('ed_serial_port'),
        ))

    @async_test_body
    async def test_TC_COMPRO_2_4(self):
        cp = self.cp
        params = getattr(self, 'user_params', {}) or {}

        ed_discriminator = int(params.get('ed_discriminator', 3841))
        ed_passcode = int(params.get('ed_passcode', 20202021))
        proxy_connect_timeout = int(params.get('proxy_connect_timeout', CONNECT_TIMEOUT_S))
        wifi_ssid = params.get('wifi_ssid')
        wifi_password = params.get('wifi_password', '')
        ed_extra_args_fallback = params.get('ed_extra_args', '')

        # Step 7 commissions the ED through the proxy, which runs the network-setup
        # stages and needs Wi-Fi credentials.  Without an SSID the commissioner skips
        # those stages, the ED never joins the operational network, and the run dies
        # much later in an mDNS resolution timeout.  Fail here instead.
        asserts.assert_true(
            bool(wifi_ssid),
            "wifi_ssid is required: pass --string-arg wifi_ssid:<ssid> (and "
            "wifi_password:<password>) so the ED can be provisioned onto the "
            "operational network through the proxy")

        # Step 1 — commissioning of the DUT done by the framework
        self.step(1)

        # Step 2 — read Transport attribute
        self.step(2)
        valid_transports = await self.read_transport()
        logger.info("valid_transports = 0x%02x", valid_transports)

        # Step 3 — read WiFiBand (conditional on WI feature)
        feature_map = await self.read_feature_map()
        has_wi = self.has_feature_wi(feature_map)
        valid_bands: int = 0
        if has_wi:
            self.step(3)
            valid_bands = await self.read_wifi_band()
            logger.info("valid_bands = 0x%04x", valid_bands)
        else:
            self.skip_step(3)

        # Determine which transport bits to exercise, in ascending bit-value order
        # (lowest CapabilitiesBitmap bit first: kBle=0x02, kWiFiPAF=0x08, ...).
        # Sorting by bit value is deterministic and forward-compatible with any
        # transports added to CapabilitiesBitmap in the future.
        # When valid_transports has only one bit set the loop runs once —
        # identical to the previous single-transport behaviour.
        kWiFiPAF_bit = int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF)
        kBle_bit = int(cp.Bitmaps.CapabilitiesBitmap.kBle)
        all_defined_transports = sorted([kBle_bit, kWiFiPAF_bit])
        transports_to_test = [b for b in all_defined_transports if valid_transports & b]

        # Variables set by each iteration and used by steps 10–14.
        asserts.assert_true(len(transports_to_test) > 0, "No supported transports found to test")
        single_transport = transports_to_test[0]
        single_band: int | None = (self.pick_single_transport_bit(valid_bands)
                                   if has_wi and (transports_to_test[0] == kWiFiPAF_bit)
                                   else None)

        # ----------------------------------------------------------------
        # Steps 4–9: iterate once per transport bit in valid_transports.
        # step() accepts each step exactly once and in order, so the markers cannot
        # simply move into the loop.  They are issued on the first pass only, each one
        # immediately before its own work: a failure then reports against the step it
        # happened in.  Marking all six up front instead made every failure in the loop
        # report as step 9, which is indistinguishable from a genuine step-9
        # ProxyDisconnect failure.  A second transport re-runs the same work with the
        # markers already consumed.
        # ----------------------------------------------------------------
        for iteration_index, transport_bit in enumerate(transports_to_test):
            first_pass = (iteration_index == 0)
            is_wifipaf = (transport_bit == kWiFiPAF_bit)
            transport_label = 'WiFiPAF' if is_wifipaf else 'BLE'
            single_transport = transport_bit
            single_band = (self.pick_single_transport_bit(valid_bands)
                           if has_wi and is_wifipaf else None)
            ed_node_id = ED_NODE_ID + iteration_index

            logger.info("=== Transport iteration %d/%d: %s (0x%02x) — ED nodeId=0x%04x ===",
                        iteration_index + 1, len(transports_to_test),
                        transport_label, transport_bit, ed_node_id)

            ed = self._ed_fixture_for_transport(transport_bit, params, ed_extra_args_fallback)

            # Clear any Wi-Fi credential the ED stored while commissioning on a
            # previous transport, and restart its wpa_supplicant, so this
            # iteration starts with an unassociated radio.
            #
            # Done at the top of every iteration rather than at the end, so a
            # failure inside the loop body cannot leave the next iteration
            # running against a dirty radio.
            await self.renew_ed_environment(
                ed,
                manual_prompt=(
                    f"[Transport {iteration_index + 1}/{len(transports_to_test)}: {transport_label}] "
                    "Run ~/script/renew-comee_env.sh on the End Device to clear its stored "
                    "Wi-Fi credentials and restart wpa_supplicant, so it starts this "
                    "iteration unassociated. Press Enter when done."
                ),
            )

            # -- Step 4 work: ensure ED commissionable for this transport --
            if first_pass:
                self.step(4)
            await self.ensure_ed_commissionable(
                ed,
                manual_prompt=(
                    f"[Transport {iteration_index + 1}/{len(transports_to_test)}: {transport_label}] "
                    f"Make the End Device commissionable via {transport_label} "
                    f"(discriminator={ed_discriminator}, passcode={ed_passcode}). "
                    "The device does NOT need an Ethernet/IP connection — it will be "
                    f"discovered by the DUT over {transport_label}. Press Enter when ready."
                ),
            )

            # -- Step 5 work: ProxyConnectRequest --
            if first_pass:
                self.step(5)
            logger.info("[%s] Sending ProxyConnectRequest "
                        "(transport=0x%02x discriminator=%d wiFiBand=%s)",
                        transport_label, single_transport, ed_discriminator,
                        hex(single_band) if single_band is not None else "None")
            connect_response = await self.send_cp_command(
                cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=single_transport,
                    discriminator=ed_discriminator,
                    vendorID=0,
                    productID=0,
                    timeout=proxy_connect_timeout,
                    wiFiBand=single_band,
                ),
                # Async ProxyConnect: the invoke wait is governed by
                # interactionTimeoutMs, so it must cover proxy_connect_timeout;
                # None caps it at the ~13s MRP default and guillotines
                # slower-but-valid transport connects.
                timeout_ms=proxy_connect_timeout * 1000 + 10000)
            current_session_id = connect_response.sessionID
            asserts.assert_true(
                current_session_id <= 0xFFFE,
                f"[{transport_label}] SessionID {current_session_id:#06x} must be "
                "≤ 0xFFFE (ProxyConnectResponse SessionID constraint is max 65534)")
            logger.info("[%s] ProxyConnectResponse: sessionID=%d", transport_label, current_session_id)

            # -- Step 6 work: ProxyMessageRequest(ResponseTimeout=0, Message=null) --
            if first_pass:
                self.step(6)
            # Sent before commissioning so it does not perturb the tunneled flow.
            # Per spec: ResponseTimeout=0 means no response is expected and the
            # proxy responds immediately with success; Message=null means the proxy
            # SHALL NOT forward anything to the Commissionee (it returns a queued
            # message or null).  The two combined verify the immediate-success
            # behaviour without sending any frame to the ED.
            msg_response = await self.send_cp_command(
                cp.Commands.ProxyMessageRequest(
                    sessionID=current_session_id,
                    responseTimeout=0,
                    message=NullValue,
                ),
                timeout_ms=5000)
            asserts.assert_equal(
                msg_response.sessionID, current_session_id,
                f"[{transport_label}] ProxyMessageResponse sessionID must match request sessionID")
            logger.info("[%s] ProxyMessageResponse received immediately (ResponseTimeout=0, "
                        "Message=null): sessionID=%d message=%s",
                        transport_label, msg_response.sessionID,
                        "null" if msg_response.message is NullValue
                        else f"{len(msg_response.message)} bytes")

            # -- Step 7 work: CommissionViaProxy --
            if first_pass:
                self.step(7)
            self.default_controller.SetWiFiCredentials(wifi_ssid, wifi_password)

            logger.info("[%s] Commissioning ED via proxy (sessionID=%d nodeId=0x%04x "
                        "discriminator=%d)",
                        transport_label, current_session_id, ed_node_id, ed_discriminator)
            await self.default_controller.CommissionViaProxy(
                proxyNodeId=self.dut_node_id,
                proxySessionId=current_session_id,
                remoteNodeId=ed_node_id,
                discriminator=ed_discriminator,
                setupPinCode=ed_passcode,
                proxyEndpoint=self.cp_endpoint,
            )
            logger.info("[%s] ED commissioned successfully via proxy (nodeId=0x%04x)",
                        transport_label, ed_node_id)

            # -- Step 8 work: ProxyDisconnect invalid SessionID → NOT_FOUND --
            if first_pass:
                self.step(8)
            non_existent_session_id = 0xFFFE
            logger.info("[%s] Sending ProxyDisconnectRequest with non-existent sessionID=%d",
                        transport_label, non_existent_session_id)
            await self.expect_command_rejected(
                cp.Commands.ProxyDisconnectRequest(sessionID=non_existent_session_id),
                Status.NotFound,
                f"[{transport_label}] Step 8 ProxyDisconnectRequest with a non-existent sessionID")

            # -- Step 9 work: ProxyDisconnect valid current_session_id → SUCCESS --
            if first_pass:
                self.step(9)
            logger.info("[%s] Sending ProxyDisconnectRequest with valid sessionID=%d",
                        transport_label, current_session_id)
            await self.send_cp_command(
                cp.Commands.ProxyDisconnectRequest(sessionID=current_session_id))
            logger.info("[%s] ProxyDisconnectRequest succeeded for sessionID=%d",
                        transport_label, current_session_id)

            # Remove the commissioned ED from the controller fabric while it is
            # still running and reachable (UnpairDevice sends RemoveFabric to
            # the device; it would time out if called after the ED is stopped).
            logger.info("[%s] Unpairing ED (nodeId=0x%04x) from controller fabric",
                        transport_label, ed_node_id)
            await self.default_controller.UnpairDevice(ed_node_id)
            # Now stop the ED so the next iteration starts from a clean state.
            await self.ensure_ed_not_commissionable(
                ed,
                manual_prompt=(
                    f"[Transport {iteration_index + 1}/{len(transports_to_test)}: {transport_label}] "
                    "Iteration complete. Stop the End Device from advertising. "
                    "Press Enter when done."
                ),
            )

        # After the loop, single_transport / single_band hold values from the
        # last iteration and are reused for steps 10–14 negative tests.

        # ----------------------------------------------------------------
        # Step 10: ProxyConnectRequest with Timeout=1 while ED is not
        # commissionable (stopped at end of last loop iteration).
        # ----------------------------------------------------------------
        self.step(10)
        logger.info("Sending ProxyConnectRequest(Timeout=1) — expecting TIMEOUT")
        await self.expect_command_rejected(
            cp.Commands.ProxyConnectRequest(
                address=NullValue,
                transport=single_transport,
                discriminator=ed_discriminator,
                vendorID=0,
                productID=0,
                timeout=1,
                wiFiBand=single_band,
            ),
            Status.Timeout,
            "Step 10 ProxyConnectRequest(Timeout=1) with the ED not commissionable",
            timeout_ms=6000)  # 1 s DUT timeout + 5 s IM margin

        # ----------------------------------------------------------------
        # Step 11: ProxyConnectRequest with unsupported Transport bit.
        # ----------------------------------------------------------------
        defined_transports = [kBle_bit, kWiFiPAF_bit]
        unsupported_transport = next(
            (b for b in defined_transports if not (valid_transports & b)), None)
        if unsupported_transport is None:
            self.skip_step(11)
            logger.info("Step 11 skipped: all defined transport bits are in valid_transports")
        else:
            self.step(11)
            logger.info("Using unsupported transport bit 0x%02x for step 11", unsupported_transport)
            await self.expect_command_rejected(
                cp.Commands.ProxyConnectRequest(
                    address=NullValue,
                    transport=unsupported_transport,
                    discriminator=ed_discriminator,
                    vendorID=0,
                    productID=0,
                    timeout=30,
                ),
                Status.InvalidTransportType,
                "Step 11 ProxyConnectRequest with an unsupported transport bit",
                timeout_ms=10000)

        # ----------------------------------------------------------------
        # Step 12: ProxyConnectRequest with WiFiPAF transport and a WiFiBand
        # not in valid_bands (WI feature only; skipped for BLE-only DUTs).
        # ----------------------------------------------------------------
        if not has_wi or not (valid_transports & kWiFiPAF_bit):
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
                await self.expect_command_rejected(
                    cp.Commands.ProxyConnectRequest(
                        address=NullValue,
                        transport=kWiFiPAF_bit,
                        discriminator=ed_discriminator,
                        vendorID=0,
                        productID=0,
                        timeout=30,
                        wiFiBand=invalid_band,
                    ),
                    Status.InvalidTransportType,
                    "Step 12 ProxyConnectRequest with a WiFiBand outside valid_bands",
                    timeout_ms=10000)

        # ----------------------------------------------------------------
        # Step 13: ProxyMessageRequest referencing a non-existent SessionID.
        # ----------------------------------------------------------------
        self.step(13)
        logger.info("Sending ProxyMessageRequest with non-existent sessionID=0xFFFE")
        await self.expect_command_rejected(
            cp.Commands.ProxyMessageRequest(
                sessionID=0xFFFE,
                responseTimeout=10,
                message=_MINIMAL_MATTER_MSG,
            ),
            Status.NotFound,
            "Step 13 ProxyMessageRequest with a non-existent sessionID",
            timeout_ms=15000)

        # ----------------------------------------------------------------
        # Step 14: ProxyDisconnectRequest(SessionID=null) to cancel an
        # in-flight ProxyConnectRequest.
        # ----------------------------------------------------------------
        self.step(14)

        async def _pending_connect():
            try:
                return await self.send_cp_command(
                    cp.Commands.ProxyConnectRequest(
                        address=NullValue,
                        transport=single_transport,
                        discriminator=ed_discriminator,
                        vendorID=0,
                        productID=0,
                        timeout=30,
                        wiFiBand=single_band,
                    ),
                    timeout_ms=35000)
            except InteractionModelError as e:
                return e

        async def _cancel_pending():
            await asyncio.sleep(2)  # allow the connect request to reach the DUT first
            try:
                await self.send_cp_command(
                    cp.Commands.ProxyDisconnectRequest(sessionID=NullValue),
                    timeout_ms=5000)
                return "SUCCESS"
            except InteractionModelError as e:
                return e

        connect_result, cancel_result = await asyncio.gather(
            _pending_connect(), _cancel_pending())

        if isinstance(cancel_result, str):
            logger.info("Step 14: ProxyDisconnect(null) returned SUCCESS "
                        "(pending connect cancelled)")
        elif isinstance(cancel_result, InteractionModelError):
            asserts.assert_equal(
                cancel_result.status, Status.InvalidInState,
                f"ProxyDisconnect(null) returned unexpected status {cancel_result.status}; "
                "expected SUCCESS or INVALID_IN_STATE")
            logger.info("Step 14: ProxyDisconnect(null) returned INVALID_IN_STATE "
                        "(race: connect already responded)")
        else:
            asserts.fail(f"Step 14: unexpected cancel_result type {type(cancel_result)}")

        asserts.assert_true(
            isinstance(connect_result, InteractionModelError),
            f"ProxyConnectRequest must not succeed when cancelled via null SessionID; "
            f"got {connect_result}")
        logger.info("Step 14: ProxyConnectRequest resolved with %s (expected)",
                    connect_result.status)


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
