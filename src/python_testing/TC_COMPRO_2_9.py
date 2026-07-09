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

"""TC-COMPRO-2.9 — Commissioning Proxy cluster: Device Type Requirements with DUT as Server.

Verifies that the Commissioning By Proxy device type (0x0092) requirements
are met:

  Step 2:  DeviceTypeList on the cluster endpoint SHALL contain device type
           0x0092 (Commissioning By Proxy).
  Step 3:  Transport attribute SHALL have at least one defined transport bit
           (BLE bit 1, WiFiPAF bit 3 or NTL bit 4) set and no reserved bits.
  Step 4:  The three commands for which the spec explicitly requires a CASE
           session SHALL be rejected with UNSUPPORTED_ACCESS when sent over
           a PASE session: ProxyConnectRequest, ProxyDisconnectRequest, and
           ProxyMessageRequest.

Test plan reference: TC-COMPRO-2.9 (PROVISIONAL)

Example:
    ```bash
    python3 TC_COMPRO_2_9.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --paa-trust-store-path ~/matter_tests/paa-trust-store \\
        --endpoint 1
    ```
"""

import logging

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

COMMISSIONING_BY_PROXY_DEVICE_TYPE = 0x0092


class TC_COMPRO_2_9(COMPROBaseTest):

    def desc_TC_COMPRO_2_9(self) -> str:
        return "[TC-COMPRO-2.9] Device Type Requirements with DUT as Server"

    def pics_TC_COMPRO_2_9(self) -> list[str]:
        return ["COMPRO.S"]

    def steps_TC_COMPRO_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT (proxy) to TH", is_commissioning=True),
            TestStep(2, "TH reads DeviceTypeList attribute on the endpoint under test",
                     "List SHALL contain an entry with DeviceType 0x0092 (Commissioning By Proxy)"),
            TestStep(3, "TH reads Transport attribute",
                     "Value is a CapabilitiesBitmap with at least one of the BLE (bit 1), "
                     "WiFiPAF (bit 3) or NTL (bit 4) bits set and no reserved bits set"),
            TestStep(4, "TH opens a commissioning window, establishes a PASE session, "
                     "and sends ProxyConnectRequest, ProxyDisconnectRequest, and "
                     "ProxyMessageRequest over that PASE session",
                     "DUT returns UNSUPPORTED_ACCESS for each command"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_9(self):
        cp = self.cp

        # Step 1 — commissioning done by the framework
        self.step(1)

        # ----------------------------------------------------------------
        # Step 2 — read DeviceTypeList; verify 0x0092 is present
        # ----------------------------------------------------------------
        self.step(2)
        result = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(self.cp_endpoint, Clusters.Descriptor.Attributes.DeviceTypeList)],
        )
        device_type_list = result[self.cp_endpoint][Clusters.Descriptor][
            Clusters.Descriptor.Attributes.DeviceTypeList
        ]
        device_type_ids = [entry.deviceType for entry in device_type_list]
        logger.info("Step 2: DeviceTypeList = %s", [hex(x) for x in device_type_ids])
        asserts.assert_in(
            COMMISSIONING_BY_PROXY_DEVICE_TYPE,
            device_type_ids,
            f"DeviceTypeList on endpoint {self.cp_endpoint} does not contain "
            f"Commissioning By Proxy device type (0x{COMMISSIONING_BY_PROXY_DEVICE_TYPE:04x})",
        )

        # ----------------------------------------------------------------
        # Step 3 — read Transport; verify at least one defined transport bit
        # (BLE/WiFiPAF/NTL) is set and no reserved bits are set.
        # ----------------------------------------------------------------
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("Step 3: Transport = 0x%02x", valid_transports)
        self.assert_transport_value_valid(valid_transports)
        single_transport = self.pick_single_transport_bit(valid_transports)

        # ----------------------------------------------------------------
        # Step 4 — PASE session → spec-mandated CASE commands → UNSUPPORTED_ACCESS
        # ----------------------------------------------------------------
        self.step(4)
        # The commissioning window is not explicitly revoked after this step; the
        # proxy's fail-safe timer closes it automatically.  CI always runs with a
        # factory-reset proxy so this has no effect there.
        window_params = await self.open_commissioning_window()
        pase_node_id = self.dut_node_id + 1
        await self.default_controller.FindOrEstablishPASESession(
            setupCode=window_params.commissioningParameters.setupQRCode,
            nodeId=pase_node_id,
        )
        logger.info("Step 4: PASE session established (node_id=0x%016x)", pase_node_id)

        # Only the three commands the spec marks as fabric-scoped (O F in the command
        # table).  Fabric-scoped commands require a session that carries a fabric
        # identity (CASE); a PASE session has no fabric, so the access control layer
        # rejects them with UNSUPPORTED_ACCESS.  ProxyScanRequest and the BGS commands
        # are O-only (no F flag), so they are accessible over PASE and are not tested
        # here.
        # Note: sessionID values below are irrelevant — UNSUPPORTED_ACCESS fires before
        # any session lookup.
        pase_commands = [
            ("ProxyConnectRequest", cp.Commands.ProxyConnectRequest(
                address=NullValue,
                transport=single_transport,
                discriminator=0,
                vendorId=0,
                productId=0,
                timeout=30,
            )),
            ("ProxyDisconnectRequest", cp.Commands.ProxyDisconnectRequest(
                sessionID=0xFFFE,
            )),
            ("ProxyMessageRequest", cp.Commands.ProxyMessageRequest(
                sessionID=0xFFFE,
                responseTimeout=10,
                message=NullValue,
            )),
        ]

        for cmd_name, payload in pase_commands:
            try:
                await self.default_controller.SendCommand(
                    nodeId=pase_node_id,
                    endpoint=self.cp_endpoint,
                    payload=payload,
                )
                asserts.fail(
                    f"Expected UNSUPPORTED_ACCESS but {cmd_name} over PASE succeeded"
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.UnsupportedAccess,
                    f"Expected UNSUPPORTED_ACCESS for PASE {cmd_name}, got {e.status}",
                )
                logger.info("Step 4: %s correctly got UNSUPPORTED_ACCESS", cmd_name)


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
