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

The Python TH drives the full commissioning tunnel natively:

  Step 5: TH sends ProxyConnectRequest → DUT returns ProxyConnectResponse with SessionId
  Step 6: TH commissions the ED through the proxy tunnel via ProxyMessageRequest/Response
  Step 7: TH sends ProxyDisconnectRequest with an invalid SessionId → NOT_FOUND
  Step 8: TH sends ProxyDisconnectRequest with the valid SessionId → SUCCESS

The ED (End Device) is expected to be reachable by the DUT via WiFiPAF only — it does
NOT need to be on the Ethernet/IP network used by the TH.  The DUT acts as the
commissioning tunnel: all Matter packets flow via ProxyMessageRequest/Response.

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
    The test will pause at step 2 and ask the operator to make the ED commissionable
    via WiFiPAF (press the button / power-cycle), then again at the end to confirm
    advertising has stopped.
"""

import logging

from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout for ProxyConnectRequest (seconds)
CONNECT_TIMEOUT_S = 120

# Node ID assigned to the ED during proxy commissioning
ED_NODE_ID = 0x1001


class TC_COMPRO_2_4(COMPROBaseTest):

    def desc_TC_COMPRO_2_4(self) -> str:
        return "TC-COMPRO-2.4: Proxy Connect, Message and Disconnect functionality"

    def pics_TC_COMPRO_2_4(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT (proxy) to TH", is_commissioning=True),
            TestStep(2, "Ensure ED IS commissionable (advertising via WiFiPAF; does not need to be on Ethernet)"),
            TestStep(3, "TH reads Transport attribute", "Store as valid_transports"),
            TestStep(4, "TH reads WiFiBand attribute (if WI supported)", "Store as valid_bands"),
            TestStep(5, "TH sends ProxyConnectRequest to DUT",
                     "DUT sends ProxyConnectResponse with SUCCESS status and a valid SessionId; "
                     "save SessionId as current_session_id"),
            TestStep(6, "TH uses ProxyMessageRequest/Response to commission the ED through the proxy",
                     "Commissioning procedure completes successfully"),
            TestStep(7, "TH sends ProxyDisconnectRequest(SessionId=<non-existent>)",
                     "DUT responds with NOT_FOUND cluster error"),
            TestStep(8, "TH sends ProxyDisconnectRequest(SessionId=current_session_id)",
                     "DUT responds with SUCCESS"),
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
        # reachable over Ethernet from the TH.  If ed_app_path was not provided
        # the operator is prompted to power-cycle or press the commissioning
        # button on the physical device.
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

        # ----------------------------------------------------------------
        # Step 5: Python TH sends ProxyConnectRequest → saves current_session_id
        # ----------------------------------------------------------------
        self.step(5)
        single_transport = self.pick_single_transport_bit(valid_transports)
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
                wiFiBand=valid_bands if has_wi else None,
            ),
            interactionTimeoutMs=proxy_connect_timeout * 1000 + 5000,
        )
        current_session_id = connect_response.sessionId
        asserts.assert_not_equal(current_session_id, 0,
                                 "SessionId in ProxyConnectResponse must not be 0")
        logger.info("ProxyConnectResponse: sessionId=%d", current_session_id)

        # ----------------------------------------------------------------
        # Step 6: TH commissions the ED through the proxy tunnel opened in
        # step 5. The SDK routes PASE + commissioning packets via
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
        # Step 7: invalid disconnect — NOT_FOUND
        # current_session_id from step 5 is still live; a non-existent ID is used.
        # ----------------------------------------------------------------
        self.step(7)
        non_existent_session_id = 0xFFFE  # guaranteed non-existent
        logger.info("Sending ProxyDisconnectRequest with non-existent sessionId=%d",
                    non_existent_session_id)
        with asserts.assert_raises(InteractionModelError) as ctx:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyDisconnectRequest(sessionId=non_existent_session_id),
            )
        asserts.assert_equal(ctx.exception.status, Status.NotFound,
                             f"Expected NOT_FOUND for non-existent sessionId, got {ctx.exception.status}")
        logger.info("ProxyDisconnectRequest with invalid sessionId correctly returned NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 8: cleanly disconnect the session opened above (still live
        # because step 7 used a different, invalid session ID).
        # ----------------------------------------------------------------
        self.step(8)
        logger.info("Sending ProxyDisconnectRequest with valid sessionId=%d", current_session_id)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyDisconnectRequest(sessionId=current_session_id),
        )
        logger.info("ProxyDisconnectRequest succeeded for sessionId=%d", current_session_id)

        # Cleanup — for physical hardware, prompt the operator to power off or
        # factory-reset the ED so it is ready for the next test run.
        await self.ensure_ed_not_commissionable(
            ed,
            manual_prompt=(
                "Commissioning test complete. Power off or factory-reset the End Device "
                "so it stops advertising and is ready for the next test run. "
                "Press Enter when done."
            ),
        )


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
