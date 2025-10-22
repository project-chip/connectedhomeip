
#    Copyright (c) 2025 Project CHIP Authors
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
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/dggen_2_1_fifo
#     app-ready-pattern: "Server initialization complete"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --app-pipe /tmp/dggen_2_1_fifo
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
# === END CI TEST ARGUMENTS ===


import asyncio
import logging
import time
from typing import List

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.testing.matter_testing import asserts


class TC_DGGEN_2_1_Py(MatterBaseTest):
    """
    [TC-DGGEN-2.1] Attributes (DUT as Server) — Python version with adjustments:
      - No factory reset at start
      - UpTime check and reboot before RebootCount++ check
      - NetworkInterfaces validated in code
      - TotalOperationalHours: verify >2 (manual precondition) and post-factory reset <=1
    """

    def desc_TC_DGGEN_2_1_Py(self) -> str:
        return "[TC-DGGEN-2.1] Attributes (Server) — Pythonized + tweaks"

    def pics_TC_DGGEN_2_1_Py(self):
        return ["DGGEN.S"]

    def steps_TC_DGGEN_2_1_Py(self) -> List[TestStep]:
        return [
            TestStep(0,  "Manual pre-check: Device in use >2h without factory reset (tester confirms)"),
            TestStep(1,  "Read TotalOperationalHours and verify >2"),
            TestStep(2,  "Step 2a mod: Read and save RebootCount (boot_count1)"),
            TestStep(3,  "Step 4a/4b: Read UpTime (uptime1), wait 10s, read UpTime (uptime2) and verify uptime2 >= uptime1"),
            TestStep(4,  "Step 4c: Reboot DUT (manual/automated according to your environment), wait for reconnection"),
            TestStep(5,  "Step 2b mod: Read RebootCount and verify boot_count == boot_count1 + 1"),
            TestStep(6,  "UpTime post-reboot (uptime3) < uptime2"),
            TestStep(7,  "Step 3 mod: Programmatically validate NetworkInterfaces"),
            TestStep(8,  "Factory reset (manual), re-commission, verify TotalOperationalHours <= 1", is_commissioning=True),
        ]

    # ------- helpers -------

    async def _read_attr(self, dev_ctrl, cluster_attr, endpoint=None, node_id=None):
        return await self.read_single_attribute(
            dev_ctrl=dev_ctrl,
            node_id=node_id if node_id is not None else self.req_node_id,
            endpoint=endpoint if endpoint is not None else self.endpoint,
            attribute=cluster_attr
        )

    async def _read_reboot_count(self, dev_ctrl) -> int:
        return int(await self._read_attr(dev_ctrl, Clusters.GeneralDiagnostics.Attributes.RebootCount))

    async def _read_uptime(self, dev_ctrl) -> int:
        return int(await self._read_attr(dev_ctrl, Clusters.GeneralDiagnostics.Attributes.UpTime))

    async def _read_total_hrs(self, dev_ctrl) -> int:
        return int(await self._read_attr(dev_ctrl, Clusters.GeneralDiagnostics.Attributes.TotalOperationalHours))

    async def _read_net_ifaces(self, dev_ctrl):
        return await self._read_attr(dev_ctrl, Clusters.GeneralDiagnostics.Attributes.NetworkInterfaces)

    async def _wait_for_commissionee(self, node_id=None, timeout_s=120):
        # If your harness has a specific helper, replace this.
        # Here we do a small poll to State (read + sleep).
        node_id = node_id if node_id is not None else self.req_node_id
        end = self.event_loop.time() + timeout_s
        while self.event_loop.time() < end:
            try:
                # Try to read something cheap to confirm it's back
                # (UpTime, for example)
                _ = await self._read_uptime(self.default_controller)
                return
            except Exception:
                await asyncio.sleep(1.0)
        raise TimeoutError(f"DUT {node_id} did not become accessible again in {timeout_s}s")

    def _validate_network_interfaces(self, interfaces) -> None:
        """
        Basic verifications:
          - Non-empty list.
          - Each entry has: Name (str), IsOperational (bool),
            OffPremiseServicesReachableIPv4/IPv6 (bool | None),
            HardwareAddress (bytes | str hex),
            IPv4Addresses list[bytes],
            IPv6Addresses list[bytes],
            Type enum/integer.
        Without assuming exact names/lengths (platform dependent).
        """
        assert isinstance(interfaces, list), "NetworkInterfaces must be a list"
        assert len(interfaces) >= 1, "At least one network interface expected"

        for i, nif in enumerate(interfaces, start=1):
            # Fields can come as typed object attributes
            name = getattr(nif, "name", None) or getattr(nif, "Name", None)
            is_op = getattr(nif, "isOperational", None) or getattr(nif, "IsOperational", None)
            r4 = getattr(nif, "offPremiseServicesReachableIPv4", None) or getattr(nif, "OffPremiseServicesReachableIPv4", None)
            r6 = getattr(nif, "offPremiseServicesReachableIPv6", None) or getattr(nif, "OffPremiseServicesReachableIPv6", None)
            mac = getattr(nif, "hardwareAddress", None) or getattr(nif, "HardwareAddress", None)
            ip4s = getattr(nif, "iPv4Addresses", None) or getattr(nif, "IPv4Addresses", None)
            ip6s = getattr(nif, "iPv6Addresses", None) or getattr(nif, "IPv6Addresses", None)
            ityp = getattr(nif, "type", None) or getattr(nif, "Type", None)

            assert isinstance(name, str), f"[iface #{i}] Name must be string"
            assert isinstance(is_op, bool), f"[iface #{i}] IsOperational must be bool"
            assert (r4 is None) or isinstance(r4, bool), f"[iface #{i}] OffPremiseServicesReachableIPv4 must be bool or None"
            assert (r6 is None) or isinstance(r6, bool), f"[iface #{i}] OffPremiseServicesReachableIPv6 must be bool or None"
            assert mac is not None, f"[iface #{i}] HardwareAddress missing"
            assert isinstance(ip4s, list), f"[iface #{i}] IPv4Addresses must be list"
            assert isinstance(ip6s, list), f"[iface #{i}] IPv6Addresses must be list"
            assert ityp is not None, f"[iface #{i}] Type missing"

    # ------- test body -------

    @async_test_body
    async def test_TC_DGGEN_2_1_Py(self):
        self.endpoint = self.get_endpoint(0)
        self.req_node_id = self.user_params.get('req_node_id', 0x12344321)

        ctrl = self.default_controller

        # CI-specific setup: Set uptime > 2 hours for CI testing
        if self.is_pics_sdk_ci_only:
            logging.info("CI environment detected - setting uptime > 2 hours via named pipe")
            # Set TotalOperationalHours to 3 hours (10800 seconds) for CI testing
            self.write_to_app_pipe({"Name": "SetTotalOperationalHours", "Hours": 3})
            logging.info("Uptime manipulation completed for CI")

        # Step 0: Manual precondition
        self.step(0)
        logging.info(
            "[Manual] Confirm: the DUT was in use >2h WITHOUT recent factory reset. "
            "If not, retry later."
        )

        # Step 1: TotalOperationalHours > 2
        self.step(1)
        total_hrs = await self._read_total_hrs(ctrl)
        logging.info(f"TotalOperationalHours (pre): {total_hrs}")

        assert total_hrs > 2, f"Precondition not met: TotalOperationalHours={total_hrs} <= 2. Device must have been in use for more than 2 hours without factory reset."
        logging.info("Precondition met: TotalOperationalHours > 2")

        # Step 2: 2a mod — save RebootCount (without requiring value)
        self.step(2)
        boot_count1 = await self._read_reboot_count(ctrl)
        logging.info(f"RebootCount (boot_count1): {boot_count1}")

        # Step 3: 4a/4b — UpTime before/after 10s
        self.step(3)
        uptime1 = await self._read_uptime(ctrl)
        await asyncio.sleep(10.0)
        uptime2 = await self._read_uptime(ctrl)
        logging.info(f"UpTime: before={uptime1}, after10s={uptime2}")
        assert uptime2 >= uptime1, f"UpTime did not increase as expected ({uptime1} -> {uptime2})"

        # Step 4: 4c — Reboot (manual/automated) + wait for reconnection
        self.step(4)
        # Check if restart flag file is available (indicates test runner supports app restart)
        restart_flag_file = self.get_restart_flag_file()

        if not restart_flag_file:
            # No restart flag file: ask user to manually reboot
            self.wait_for_user_input(prompt_msg="Reboot the DUT. Press Enter when ready.\n")

            # After manual reboot, expire previous sessions so that we can re-establish connections
            logging.info("Expiring sessions after manual device reboot")
            self.th1.ExpireSessions(self.dut_node_id)
            self.th2.ExpireSessions(self.dut_node_id)
            logging.info("Manual device reboot completed")

        else:
            try:
                # Create the restart flag file to signal the test runner
                with open(restart_flag_file, "w") as f:
                    f.write("restart")
                logging.info("Created restart flag file to signal app restart")

                # The test runner will automatically wait for the app-ready-pattern before continuing
                # Waiting 1 second after the app-ready-pattern is detected as we need to wait a tad longer for the app to be ready and stable, otherwise TH2 connection fails later on in test step 14.
                time.sleep(1)

                # Expire sessions and re-establish connections
                self.th1.ExpireSessions(self.dut_node_id)
                self.th2.ExpireSessions(self.dut_node_id)

                logging.info("App restart completed successfully")

            except Exception as e:
                logging.error(f"Failed to restart app: {e}")
                asserts.fail(f"App restart failed: {e}")

        # Step 5: 2b mod — RebootCount must be boot_count1 + 1
        self.step(5)
        boot_count2 = await self._read_reboot_count(ctrl)
        logging.info(f"RebootCount (post-reboot): {boot_count2}")
        assert boot_count2 == boot_count1 + 1, f"Expected RebootCount={boot_count1 + 1}, actual={boot_count2}"

        # Step 6: UpTime post-reboot < uptime2
        self.step(6)
        uptime3 = await self._read_uptime(ctrl)
        logging.info(f"UpTime post-reboot={uptime3}, previous UpTime={uptime2}")
        assert uptime3 < uptime2, "Expected lower UpTime after reboot"

        # Step 7: NetworkInterfaces programmatic validation
        self.step(7)
        ifaces = await self._read_net_ifaces(ctrl)
        logging.info(f"NetworkInterfaces: {len(ifaces)} entries")
        self._validate_network_interfaces(ifaces)

        # Step 8: Factory reset (manual), re-commission, TotalOperationalHours <= 1
        self.step(8)
        logging.info(
            "[Required action] Perform Factory Reset of the DUT and re-commission it in this fabric. "
            "Then continue; the test will wait for the DUT to be accessible."
        )
        await self._wait_for_commissionee(timeout_s=240)

        total_hrs_after_fr = await self._read_total_hrs(ctrl)
        logging.info(f"TotalOperationalHours post-factory-reset: {total_hrs_after_fr}")
        assert total_hrs_after_fr <= 1, f"Expected TotalOperationalHours ≤1 post-FR (actual={total_hrs_after_fr})"


if __name__ == "__main__":
    default_matter_test_main()
