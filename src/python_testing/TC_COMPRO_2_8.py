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

"""TC-COMPRO-2.8 — Commissioning Proxy cluster: Fabric Isolation with DUT as Server.

Verifies that the Commissioning Proxy server enforces fabric isolation: sessions
and background scans belonging to one fabric cannot be accessed or manipulated
by a different fabric.

  Step 5:   TH1 (Fabric A) establishes a proxy session to the ED.
  Steps 6-7: TH2 (Fabric B) attempts to use that session and SHALL be rejected
             with NOT_FOUND.
  Step 8:   TH1 cleanly disconnects.
  Steps 9-11 (BGS feature only): TH1 starts a background scan; TH2 cannot stop
             it (NOT_FOUND); TH1 can stop it (SUCCESS).

Test plan reference: TC-COMPRO-2.8

Example — automated with remote ED RPi:
    ```bash
    python3 TC_COMPRO_2_8.py \\
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

import logging

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main
from matter.utils import CommissioningBuildingBlocks
from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

logger = logging.getLogger(__name__)

CONNECT_TIMEOUT_S = 120


class TC_COMPRO_2_8(COMPROBaseTest):

    @property
    def default_timeout(self) -> int:
        # Step 5 ProxyConnect: up to CONNECT_TIMEOUT_S; remaining steps: ~30 s
        return 300

    def desc_TC_COMPRO_2_8(self) -> str:
        return "[TC-COMPRO-2.8] Fabric Isolation with DUT as Server"

    def pics_TC_COMPRO_2_8(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_8(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to Fabric A (TH1) and Fabric B (TH2)", is_commissioning=True),
            TestStep(2, "Ensure ED is commissionable"),
            TestStep(3, "TH1 (Fabric A) reads Transport attribute",
                     "Store as valid_transports"),
            TestStep(4, "TH1 (Fabric A) reads WiFiBand attribute",
                     "Store as valid_bands"),
            TestStep(5, "TH1 (Fabric A) sends ProxyConnectRequest",
                     "DUT returns ProxyConnectResponse with SUCCESS; save sessionId as fabric_a_session"),
            TestStep(6, "TH2 (Fabric B) sends ProxyMessageRequest(fabric_a_session)",
                     "DUT returns NOT_FOUND (session belongs to Fabric A)"),
            TestStep(7, "TH2 (Fabric B) sends ProxyDisconnectRequest(fabric_a_session)",
                     "DUT returns NOT_FOUND (session belongs to Fabric A)"),
            TestStep(8, "TH1 (Fabric A) sends ProxyDisconnectRequest(fabric_a_session)",
                     "DUT returns SUCCESS"),
            TestStep(9, "TH1 (Fabric A) sends ProxyBackGroundScanStartRequest",
                     "DUT returns SUCCESS"),
            TestStep(10, "TH2 (Fabric B) sends ProxyBackGroundScanStopRequest",
                     "DUT returns NOT_FOUND (background scan belongs to Fabric A)"),
            TestStep(11, "TH1 (Fabric A) sends ProxyBackGroundScanStopRequest",
                     "DUT returns SUCCESS"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_8(self):
        cp = self.cp

        # ----------------------------------------------------------------
        # Step 1 — Commission DUT to Fabric A (TH1) and Fabric B (TH2)
        # TH1 is self.default_controller, commissioned by the framework.
        # TH2 is a new controller on a separate CA/fabric, added here.
        # Manual re-runs without factory-reset will fail at AddNOCForNewFabricFromExisting
        # because Fabric B already exists on the DUT.  Always factory-reset between runs.
        # ----------------------------------------------------------------
        self.step(1)
        th2_ca = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_ca.NewFabricAdmin(
            vendorId=0xFFF1,
            fabricId=self.matter_test_config.fabric_id + 1,
        )
        th2 = th2_fabric_admin.NewController(
            nodeId=self.matter_test_config.controller_node_id + 1,
        )
        success, _noc_resp, _cert_chain = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=self.default_controller,
            newFabricDevCtrl=th2,
            existingNodeId=self.dut_node_id,
            newNodeId=self.dut_node_id,
        )
        asserts.assert_true(success, "Failed to commission DUT to Fabric B (TH2)")
        logger.info("Step 1: DUT commissioned to Fabric A (TH1) and Fabric B (TH2)")

        # ----------------------------------------------------------------
        # Step 2 — Ensure ED is commissionable
        # ----------------------------------------------------------------
        self.step(2)
        ed = self._ed_fixture_from_params()
        await self.ensure_ed_commissionable(ed)

        # ----------------------------------------------------------------
        # Step 3 — TH1 reads Transport
        # ----------------------------------------------------------------
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("Step 3: valid_transports = 0x%02x", valid_transports)

        # ----------------------------------------------------------------
        # Step 4 — TH1 reads WiFiBand (conditional on WI feature)
        # ----------------------------------------------------------------
        feature_map = await self.read_feature_map()
        has_wi = self.has_feature_wi(feature_map)
        has_bgs = self.has_feature_bgs(feature_map)
        valid_bands: int = 0
        if has_wi:
            self.step(4)
            valid_bands = await self.read_wifi_band()
            logger.info("Step 4: valid_bands = 0x%04x", valid_bands)
        else:
            self.skip_step(4)

        single_transport = self.pick_single_transport_bit(valid_transports)
        single_band = self.pick_single_transport_bit(valid_bands) if has_wi else None

        # ----------------------------------------------------------------
        # Step 5 — TH1 ProxyConnectRequest → fabric_a_session
        # ----------------------------------------------------------------
        self.step(5)
        params = getattr(self, 'user_params', {}) or {}
        ed_discriminator = int(params.get('ed_discriminator', 3841))
        proxy_connect_timeout = int(params.get('proxy_connect_timeout', CONNECT_TIMEOUT_S))

        logger.info("Step 5: ProxyConnectRequest (transport=0x%02x, discriminator=%d)",
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
        fabric_a_session = connect_response.sessionId
        asserts.assert_true(
            0x0001 <= fabric_a_session <= 0xFFFE,
            f"fabric_a_session {fabric_a_session:#06x} must be in range 0x0001–0xFFFE")
        logger.info("Step 5: fabric_a_session = %d (0x%04x)", fabric_a_session, fabric_a_session)

        # ----------------------------------------------------------------
        # Step 6 — TH2 ProxyMessageRequest(fabric_a_session) → NOT_FOUND
        # ----------------------------------------------------------------
        self.step(6)
        logger.info("Step 6: TH2 ProxyMessageRequest(sessionId=%d) (expect NOT_FOUND)", fabric_a_session)
        try:
            await th2.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyMessageRequest(
                    sessionId=fabric_a_session,
                    responseTimeout=10,
                    message=NullValue,
                ),
            )
            asserts.fail("Expected NOT_FOUND but TH2 ProxyMessageRequest succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.NotFound,
                f"Expected NOT_FOUND for Fabric B ProxyMessageRequest, got {e.status}")
            logger.info("Step 6: TH2 ProxyMessageRequest correctly got NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 7 — TH2 ProxyDisconnectRequest(fabric_a_session) → NOT_FOUND
        # ----------------------------------------------------------------
        self.step(7)
        logger.info("Step 7: TH2 ProxyDisconnectRequest(sessionId=%d) (expect NOT_FOUND)", fabric_a_session)
        try:
            await th2.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyDisconnectRequest(sessionId=fabric_a_session),
            )
            asserts.fail("Expected NOT_FOUND but TH2 ProxyDisconnectRequest succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.NotFound,
                f"Expected NOT_FOUND for Fabric B ProxyDisconnectRequest, got {e.status}")
            logger.info("Step 7: TH2 ProxyDisconnectRequest correctly got NOT_FOUND")

        # ----------------------------------------------------------------
        # Step 8 — TH1 ProxyDisconnectRequest(fabric_a_session) → SUCCESS
        # ----------------------------------------------------------------
        self.step(8)
        logger.info("Step 8: TH1 ProxyDisconnectRequest(sessionId=%d)", fabric_a_session)
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=self.cp_endpoint,
            payload=cp.Commands.ProxyDisconnectRequest(sessionId=fabric_a_session),
        )
        logger.info("Step 8: ProxyDisconnectRequest succeeded")

        # ----------------------------------------------------------------
        # Steps 9-11 — BGS fabric isolation (conditional on BGS feature)
        # ----------------------------------------------------------------
        if not has_bgs:
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
        else:
            # Step 9 — TH1 ProxyBackGroundScanStartRequest → SUCCESS
            self.step(9)
            logger.info("Step 9: TH1 ProxyBackGroundScanStartRequest (transport=0x%02x)", valid_transports)
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyBackGroundScanStartRequest(
                    transport=valid_transports,
                    timeout=0,
                    wiFiBands=valid_bands if has_wi else None,
                ),
            )
            logger.info("Step 9: ProxyBackGroundScanStartRequest succeeded")

            # Step 10 — TH2 ProxyBackGroundScanStopRequest → NOT_FOUND
            self.step(10)
            logger.info("Step 10: TH2 ProxyBackGroundScanStopRequest (expect NOT_FOUND)")
            try:
                await th2.SendCommand(
                    nodeId=self.dut_node_id,
                    endpoint=self.cp_endpoint,
                    payload=cp.Commands.ProxyBackGroundScanStopRequest(
                        transport=valid_transports,
                        wiFiBands=valid_bands if has_wi else None,
                    ),
                )
                asserts.fail("Expected NOT_FOUND but TH2 ProxyBackGroundScanStopRequest succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.NotFound,
                    f"Expected NOT_FOUND for Fabric B ProxyBackGroundScanStopRequest, got {e.status}")
                logger.info("Step 10: TH2 ProxyBackGroundScanStopRequest correctly got NOT_FOUND")

            # Step 11 — TH1 ProxyBackGroundScanStopRequest → SUCCESS
            self.step(11)
            logger.info("Step 11: TH1 ProxyBackGroundScanStopRequest (transport=0x%02x)", valid_transports)
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id,
                endpoint=self.cp_endpoint,
                payload=cp.Commands.ProxyBackGroundScanStopRequest(
                    transport=valid_transports,
                    wiFiBands=valid_bands if has_wi else None,
                ),
            )
            logger.info("Step 11: ProxyBackGroundScanStopRequest succeeded")

        # Remove TH2 (Fabric B) from the DUT so re-runs without factory-reset succeed.
        th2_fabric_index = await self.read_single_attribute(
            dev_ctrl=th2,
            node_id=self.dut_node_id,
            endpoint=0,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
        )
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=0,
            payload=Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=th2_fabric_index),
        )
        logger.info("TH2 fabric (index %d) removed from DUT", th2_fabric_index)

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
