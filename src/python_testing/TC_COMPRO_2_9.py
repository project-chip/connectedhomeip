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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""TC-COMPRO-2.9 — Commissioning Proxy cluster: Device Type Requirements with DUT as Server.

For the test rig topology, the Python wheel requirement and how to run this
suite, see ``support_modules/compro_support.py``.

Verifies that the Commissioning By Proxy device type (0x0092) requirements are met:

    Step 1:  DUT is commissioned to the TH and a CASE session can be established,
             confirming it possesses valid Operational Credentials.
    Step 2:  DeviceTypeList on the cluster endpoint SHALL contain device type
             0x0092 (Commissioning By Proxy).
    Step 3:  Transport attribute SHALL have at least one defined transport bit
             (BLE bit 1, WiFiPAF bit 3 or NTL bit 4) set and no reserved bits.

The device type's "Other Requirements" state that the device "SHALL be
commissioned onto the Fabric before a Commissioner can use its services, ensuring
that it possesses valid Operational Credentials and can establish trusted CASE
sessions".  Step 1 covers the positive half of that requirement.

The negative half — that the proxy services cannot be used before commissioning —
is verified by TC-COMPRO-2.6 phase 1, which sends ProxyConnectRequest,
ProxyMessageRequest and ProxyDisconnectRequest over a PASE session to an
uncommissioned DUT and confirms each is rejected with UNSUPPORTED_ACCESS.  It is
deliberately NOT repeated here: both tests are gated on the same PICS item
(``COMPRO.S``), so 2.6 always runs whenever 2.9 does, and the device type
introduces no failure mode beyond the per-command CASE-session requirement already
stated in the cluster specification.

Because this test no longer needs a PASE session, it commissions the DUT the
ordinary way (``--commissioning-method``) rather than PASE-first
(``--in-test-commissioning-method``), and it does not read or write any cluster
attribute over PASE.  It uses no End Device: none of the ``ed_*`` arguments apply,
and ``--paa-trust-store-path`` is unnecessary — there is no tunnelled
commissioning to attest.

Test plan reference: TC-COMPRO-2.9 (PROVISIONAL)

Example:
    ```bash
    python3 TC_COMPRO_2_9.py \\
        --commissioning-method on-network \\
        --discriminator 1234 \\
        --passcode 20202021 \\
        --storage-path /tmp/compro_admin_storage.json \\
        --endpoint 5
    ```
"""

import logging

from mobly import asserts
from support_modules.compro_support import COMPROBaseTest, commission_if_needed

import matter.clusters as Clusters
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
            TestStep(1, "Commission DUT to TH; verify the DUT has valid Operational "
                     "Credentials and a CASE session can be established",
                     "CASE session establishment succeeds",
                     is_commissioning=True),
            TestStep(2, "TH reads DeviceTypeList attribute on the endpoint under test",
                     "List SHALL contain an entry with DeviceType 0x0092 (Commissioning By Proxy)"),
            TestStep(3, "TH reads Transport attribute",
                     "Value is a CapabilitiesBitmap with at least one of the BLE (bit 1), "
                     "WiFiPAF (bit 3) or NTL (bit 4) bits set and no reserved bits set"),
        ]

    @async_test_body
    async def test_TC_COMPRO_2_9(self):
        # Step 1 — commissioning is done by the framework.  Reaching this point
        # with a usable operational session is itself the assertion: the DUT holds
        # valid Operational Credentials and CASE establishment succeeded.
        self.step(1)
        logger.info("Step 1: DUT commissioned; CASE available (dut_node_id=0x%016x)",
                    self.dut_node_id)

        # Step 2 — read DeviceTypeList; verify 0x0092 is present.
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

        # Step 3 — read Transport; verify at least one defined transport bit
        # (BLE/WiFiPAF/NTL) is set and no reserved bits are set.
        self.step(3)
        valid_transports = await self.read_transport()
        logger.info("Step 3: Transport = 0x%02x", valid_transports)
        self.assert_transport_value_valid(valid_transports)


if __name__ == "__main__":
    commission_if_needed()
    default_matter_test_main()
