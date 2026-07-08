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

The Python TH drives the full commissioning tunnel for each transport advertised
by the DUT.  Steps 4–9 iterate once per transport bit in valid_transports
(WiFiPAF first, then BLE).

  Steps 1–3:  Commission DUT; read Transport and WiFiBand attributes.
  Steps 4–9:  For each transport bit in valid_transports:
                Make ED commissionable; send ProxyConnectRequest; tunnel PASE +
                commissioning via ProxyMessageRequest/Response until ED is on the
                fabric; send ProxyMessageRequest(ResponseTimeout=0); send
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
  Provide ``ed_app_path`` (and optionally ``ed_ssh_host`` for a remote RPi).  The test
  starts and stops the ED app automatically for each transport iteration.

Manual (physical hardware):
  Omit ``ed_app_path``.  The test will pause at each iteration and prompt the operator
  to make the ED commissionable via the transport being tested.

User-defined arguments (all via --string-arg / --int-arg NAME:VALUE):
  wifi_ssid              — Wi-Fi SSID to provision on the ED (required for step 5)
  wifi_password          — Wi-Fi password (default: empty string)
  ed_discriminator       — ED commissionable discriminator (default: 3841)
  ed_passcode            — ED PASE passcode (default: 20202021)
  ed_app_path            — path to the ED binary; if omitted the test prompts the operator
  ed_ssh_host            — IP/hostname of a remote host on which to run the ED binary over SSH
  ed_ssh_user            — SSH username for ed_ssh_host (default: ubuntu)
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

Example — both transports, remote ED RPi:
    ```bash
    python3 TC_COMPRO_2_4.py \\
        --commissioning-method on-network \\
        --discriminator 3947 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 1 \\
        --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_ssh_host:172.16.62.110 \\
        --string-arg 'wifipaf_ed_extra_args:--wifi --wifipaf freq_list=2437' \\
        --string-arg 'ble_ed_extra_args:--wifi' \\
        --int-arg ed_discriminator:3840 ed_passcode:20202021
    ```

Example — WiFiPAF only (single transport), remote ED RPi:
    ```bash
    python3 TC_COMPRO_2_4.py \\
        --commissioning-method on-network \\
        --discriminator 3947 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 1 \\
        --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
        --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
        --string-arg ed_ssh_host:172.16.62.110 \\
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

# Minimal Matter MRP message header bytes (8 zero bytes) used for the
# ResponseTimeout=0 and non-existent-session negative tests.
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
            TestStep(6, "For each transport bit in valid_transports: TH acts as Commissioner "
                     "and performs full commissioning flow by tunneling PASE and commissioning "
                     "traffic through the DUT",
                     "Commissioning procedure completes; ED is commissioned onto the fabric"),
            TestStep(7, "For each transport bit in valid_transports: TH sends "
                     "ProxyMessageRequest(SessionID=current_session_id, ResponseTimeout=0, "
                     "Message=<valid frame>)",
                     "DUT returns ProxyMessageResponse immediately with "
                     "SessionID=current_session_id; Message may be null"),
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
        return EDFixture(
            app_path=app_path,
            discriminator=int(params.get('ed_discriminator', 3841)),
            passcode=int(params.get('ed_passcode', 20202021)),
            ssh_host=params.get('ed_ssh_host'),
            ssh_user=params.get('ed_ssh_user', 'ubuntu'),
            extra_args=extra_args,
            ed_transport=ed_transport,
        )

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
        single_transport = transports_to_test[0]
        single_band: int | None = (self.pick_single_transport_bit(valid_bands)
                                   if has_wi and (transports_to_test[0] == kWiFiPAF_bit)
                                   else None)

        # ----------------------------------------------------------------
        # Steps 4–9: iterate once per transport bit in valid_transports.
        # Each step is registered with the framework exactly once (before
        # the loop); the loop body runs sequentially for each transport.
        # When valid_transports has a single bit the loop runs exactly once,
        # identical to the previous single-transport behaviour.
        # ----------------------------------------------------------------
        self.step(4)
        self.step(5)
        self.step(6)
        self.step(7)
        self.step(8)
        self.step(9)

        for iteration_index, transport_bit in enumerate(transports_to_test):
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

            # -- Step 4 work: ensure ED commissionable for this transport --
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
            logger.info("[%s] Sending ProxyConnectRequest "
                        "(transport=0x%02x discriminator=%d wiFiBand=%s)",
                        transport_label, single_transport, ed_discriminator,
                        hex(single_band) if single_band is not None else "None")
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
            current_session_id = connect_response.sessionID
            asserts.assert_true(
                current_session_id <= 0xFFFE,
                f"[{transport_label}] SessionID {current_session_id:#06x} must be "
                "≤ 0xFFFE (ProxyConnectResponse SessionID constraint is max 65534)")
            logger.info("[%s] ProxyConnectResponse: sessionID=%d", transport_label, current_session_id)

            # -- Step 6 work: CommissionViaProxy --
            if wifi_ssid:
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

            # -- Step 7 work: ProxyMessageRequest(ResponseTimeout=0) --
            msg_response = await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyMessageRequest(
                    sessionID=current_session_id,
                    responseTimeout=0,
                    message=_MINIMAL_MATTER_MSG,
                ),
                interactionTimeoutMs=5000,
            )
            asserts.assert_equal(
                msg_response.sessionID, current_session_id,
                f"[{transport_label}] ProxyMessageResponse sessionID must match request sessionID")
            logger.info("[%s] ProxyMessageResponse received immediately (ResponseTimeout=0): "
                        "sessionID=%d message=%s",
                        transport_label, msg_response.sessionID,
                        "null" if msg_response.message is NullValue
                        else f"{len(msg_response.message)} bytes")

            # -- Step 8 work: ProxyDisconnect invalid SessionID → NOT_FOUND --
            non_existent_session_id = 0xFFFE
            logger.info("[%s] Sending ProxyDisconnectRequest with non-existent sessionID=%d",
                        transport_label, non_existent_session_id)
            try:
                await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyDisconnectRequest(
                        sessionID=non_existent_session_id),
                )
                asserts.fail(f"[{transport_label}] Expected NOT_FOUND for non-existent "
                             "sessionID but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.NotFound,
                    f"[{transport_label}] Expected NOT_FOUND for non-existent sessionID, "
                    f"got {e.status}")
                logger.info("[%s] ProxyDisconnectRequest with invalid sessionID correctly "
                            "returned NOT_FOUND", transport_label)

            # -- Step 9 work: ProxyDisconnect valid current_session_id → SUCCESS --
            logger.info("[%s] Sending ProxyDisconnectRequest with valid sessionID=%d",
                        transport_label, current_session_id)
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyDisconnectRequest(sessionID=current_session_id),
            )
            logger.info("[%s] ProxyDisconnectRequest succeeded for sessionID=%d",
                        transport_label, current_session_id)

            # Remove the commissioned ED from the controller fabric while it is
            # still running and reachable (UnpairDevice sends RemoveFabric to
            # the device; it would time out if called after the ED is stopped).
            logger.info("[%s] Unpairing ED (nodeId=0x%04x) from controller fabric",
                        transport_label, ed_node_id)
            await self.default_controller.UnpairDevice(ed_node_id)
            # Now stop the ED and clear the eth0 block for the next iteration.
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
                try:
                    await self.default_controller.SendCommand(
                        nodeId=self.dut_node_id,
                        endpoint=self.cp_endpoint,
                        payload=cp.Commands.ProxyConnectRequest(
                            address=NullValue,
                            transport=kWiFiPAF_bit,
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
        # Step 13: ProxyMessageRequest referencing a non-existent SessionID.
        # ----------------------------------------------------------------
        self.step(13)
        logger.info("Sending ProxyMessageRequest with non-existent sessionID=0xFFFE")
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyMessageRequest(
                    sessionID=0xFFFE,
                    responseTimeout=10,
                    message=_MINIMAL_MATTER_MSG,
                ),
                interactionTimeoutMs=15000,
            )
            asserts.fail("Expected NOT_FOUND for non-existent sessionID but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND for non-existent sessionID, got {e.status}")
            logger.info("ProxyMessageRequest with non-existent sessionID correctly returned NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 14: ProxyDisconnectRequest(SessionID=null) to cancel an
        # in-flight ProxyConnectRequest.
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
                    payload=cp.Commands.ProxyDisconnectRequest(sessionID=NullValue),
                    interactionTimeoutMs=5000,
                )
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
