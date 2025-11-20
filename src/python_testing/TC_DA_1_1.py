#
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     app-ready-pattern: "Server initialization complete"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
# === END CI TEST ARGUMENTS ===

import logging
import random
from time import sleep

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

'''
Purpose
This test case validates the following condition:
- NOCs attribute gets deleted on the DUT after factory reset.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/deviceattestation.adoc#211-tc-da-11-the-noc-shall-be-wiped-on-factory-reset-dut---commissionee
'''


class TC_DA_1_1(MatterBaseTest):

    def desc_TC_DA_1_1(self) -> str:
        return "The NOC SHALL be wiped on Factory Reset [DUT - Commissionee]"

    def pics_TC_DA_1_1(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "MCORE.ROLE.COMMISSIONEE"
        ]
        return pics

    def steps_TC_DA_1_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("precondition", "DUT Commissioned to TH1's fabric", is_commissioning=True),
        ]

        return steps

    def get_new_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        new_controller = new_fabric_admin.NewController(paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        return new_controller

    async def read_nocs(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController):
        return await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.NOCs,
            fabric_filtered=False)

    async def read_fabrics(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController):
        return await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.Fabrics,
            fabric_filtered=False)

    def factory_reset_dut(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController):
        restart_flag_file = self.get_restart_flag_file()

        if not restart_flag_file:
            # No restart flag file: ask user to manually reboot
            self.wait_for_user_input(prompt_msg="Reboot the DUT. Press Enter when ready.\n")

            # After manual reboot, expire previous sessions so that we can re-establish connections
            logging.info("Expiring sessions after manual device reboot")
            dev_ctrl.ExpireSessions(self.dut_node_id)
            logging.info("Manual device reboot completed")

        else:
            try:
                # Create the restart flag file to signal the test runner
                with open(restart_flag_file, "w") as f:
                    f.write("restart")
                logging.info("Created restart flag file to signal app restart")

                # The test runner will automatically wait for the app-ready-pattern before continuing
                # Waiting 1 second after the app-ready-pattern is detected as we need to wait a tad longer for the app to be ready and stable, otherwise TH2 connection fails later on in test step 14.
                sleep(1)

                # Expire sessions and re-establish connections
                dev_ctrl.ExpireSessions(self.dut_node_id)

                logging.info("App restart completed successfully")

            except Exception as e:
                logging.error(f"Failed to restart app: {e}")
                asserts.fail(f"App restart failed: {e}")

    async def commission_dut(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController):
        params = await dev_ctrl.OpenCommissioningWindow(
            nodeId=self.dut_node_id,
            timeout=900,
            iteration=10000,
            discriminator=self.discriminator,
            option=1)

        await dev_ctrl.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminator)

    @async_test_body
    async def test_TC_DA_1_1(self):

        self.step("precondition")

        self.discriminator = random.randint(0, 4095)
        th1 = self.default_controller
        th2 = self.get_new_controller()

        nocs_th1 = await self.read_nocs(th1)
        asserts.assert_true(len(nocs_th1) == 1, "NOCs attribute must contain single entry in the list")

        fabrics_th1 = await self.read_fabrics(th1)
        asserts.assert_true(len(fabrics_th1) == 1, "Fabrics attribute must contain single entry in the list")
        asserts.assert_equal(fabrics_th1[0].fabricID, self.th1.fabricId, "TH1 FabricID and Fabrics attribute FabricID must match")

        self.factory_reset_dut(th1)

        await self.commission_dut(th2)

        nocs_th1 = await self.read_nocs(th2)


if __name__ == "__main__":
    default_matter_test_main()
