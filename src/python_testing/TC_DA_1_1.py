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
import pathlib
import asyncio
import matter.clusters as Clusters
from mobly import asserts
from matter import ChipDeviceCtrl
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from matter.tlv import TLVReader

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

    def steps_TC_DA_1_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("precondition", "DUT Commissioned to TH1's fabric", is_commissioning=True),
            TestStep(1, "TH1 does a non-fabric filtered read of the NOCs attribute from the Node Operational Credentials cluster and saves the returned list as nocs_th1", """
                     - Verify that there is a single entry in the NOCs list"""),
            TestStep(2, "TH1 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster", """
                     - Verify that there is a single entry in the Fabrics list
                     - Verify that the FabricID for that entry matches the FabricID for TH1"""),
            TestStep(3, "Factory reset DUT and perform the necessary actions to put the DUT into a commissionable state"),
            TestStep(4, "Commission DUT to TH2's Fabric"),
            TestStep(5, "TH2 does a non-fabric-filtered read of Fabrics attribute list from DUT", """
                     - Verify that there is a single entry in the Fabrics list
                     - Verify that the FabricID is the same as TH2's FabricID
                     - Verify that TH2's Fabrics attribute's FabricID is different than from TH1's Fabrics attribute's FabricID"""),
            TestStep(6, "TH2 does a non-fabric-filtered read of NOCs attribute list from DUT", """
                     - Verify that there is a single entry in the NOCs list
                     - Verify that TH2's NOCs entry's public key is different than TH1's NOCs entry's public key"""),
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
            endpoint=0,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.NOCs,
            fabric_filtered=False)

    async def read_fabrics(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController):
        return await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            endpoint=0,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.Fabrics,
            fabric_filtered=False)

    async def factory_reset_dut(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController):
        # Delete KVS files
        kvs_files = list(pathlib.Path(".").glob("kvs1*"))
        for kvs_file in kvs_files:
            logging.info(f"Deleting KVS file: {kvs_file}")
            kvs_file.unlink(missing_ok=True)

        # Delete admin storage file
        admin_storage = pathlib.Path("admin_storage.json")
        if admin_storage.exists():
            admin_storage.unlink()

        # Trigger app restart for factory reset
        restart_flag_file = self.get_restart_flag_file()
        if restart_flag_file:
            with open(restart_flag_file, "w") as f:
                f.write("restart")
            logging.info("Created restart flag file to signal app restart for factory reset")

            # Wait for app to restart and be ready
            await asyncio.sleep(5)

            # Expire sessions and re-establish connections
            dev_ctrl.ExpireSessions(self.dut_node_id)
            
            logging.info("App restart completed successfully")

    @async_test_body
    async def test_TC_DA_1_1(self):
        # Get setup payload info from the original commissioning parameters
        setupPayloadInfo = self.get_setup_payload_info()
        if not setupPayloadInfo:
            asserts.fail("Setup payload info is required for commissioning.")

        # Setup
        self.discriminator = random.randint(0, 4095)
        th1 = self.default_controller
        th2 = self.get_new_controller()

        # *** PRECONDITION ***
        # DUT Commissioned to TH1's fabric
        self.step("precondition")

        # *** STEP 1 ***
        # TH1 does a non-fabric filtered read of the NOCs attribute from the Node
        # Operational Credentials cluster and saves the returned list as nocs_th1
        self.step(1)
        nocs_th1 = await self.read_nocs(th1)

        # Verify that there is a single entry in the NOCs list
        asserts.assert_true(len(nocs_th1) == 1, f"NOCs attribute must contain a single entry in the list, got {len(nocs_th1)}")

        # *** STEP 2 ***
        # TH1 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster
        self.step(2)
        fabrics_th1 = await self.read_fabrics(th1)

        # Verify that there is a single entry in the Fabrics list
        asserts.assert_true(len(fabrics_th1) == 1, f"Fabrics attribute must contain a single entry in the list, got {len(fabrics_th1)}")

        # Verify that the FabricID for that entry matches the FabricID for TH1
        asserts.assert_equal(fabrics_th1[0].fabricID, th1.fabricId, f"TH1 FabricID ({fabrics_th1[0].fabricID}) and Fabrics attribute FabricID ({th1.fabricId}) must match")

        # *** STEP 3 ***
        # Factory reset DUT and perform the necessary actions to put the DUT into a commissionable state
        self.step(3)
        await self.factory_reset_dut(th1)

        # *** STEP 4 ***
        # Commission DUT to TH2's Fabric
        self.step(4)
        logging.info(f"Commissioning TH2 with {setupPayloadInfo[0].filter_type.value} discriminator: {setupPayloadInfo[0].filter_value}, passcode: {setupPayloadInfo[0].passcode}")
        await th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setupPayloadInfo[0].passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=setupPayloadInfo[0].filter_value
        )

        # *** STEP 5 ***
        # TH2 does a non-fabric-filtered read of Fabrics attribute list from DUT
        self.step(5)
        fabrics_th2 = await self.read_fabrics(th2)

        # Verify that there is only one entry in the Fabrics List
        asserts.assert_true(len(fabrics_th2) == 1, f"Fabrics attribute must contain a single entry in the list, got {len(fabrics_th2)}")

        # Verify that the FabricID is the same as TH2's FabricID
        asserts.assert_equal(fabrics_th2[0].fabricID, th2.fabricId, f"TH2 FabricID ({fabrics_th2[0].fabricID})and Fabrics attribute FabricID ({th2.fabricId}) must match")

        # Verify that TH2's Fabrics attribute's FabricID is different than from TH1's Fabrics attribute's FabricID
        asserts.assert_not_equal(fabrics_th2[0].fabricID, fabrics_th1[0].fabricID,
                                 f"TH2's Fabrics attribute's FabricID ({fabrics_th2[0].fabricID}) must be different than from TH1's Fabrics attribute's FabricID ({fabrics_th1[0].fabricID})")

        # *** STEP 6 ***
        # TH2 does a non-fabric-filtered read of NOCs attribute list from DUT
        self.step(6)
        nocs_th2 = await self.read_nocs(th2)

        # Verify that there is a single entry in the NOCs list
        asserts.assert_true(len(nocs_th2) == 1, f"NOCs attribute must contain a single entry in the list, got {len(nocs_th2)}")

        # Verify that TH2's NOCs entry's public key is different than TH1's NOCs entry's public key
        nocs_th1_decoded_pk = TLVReader(nocs_th1[0].noc).get()["Any"][9]
        nocs_th2_decoded_pk = TLVReader(nocs_th2[0].noc).get()["Any"][9]
        asserts.assert_not_equal(nocs_th1_decoded_pk, nocs_th2_decoded_pk,
                                 f"The public key of the TH2 NOCs entry ({nocs_th1_decoded_pk.hex()}) must be different than from TH1 ({nocs_th2_decoded_pk.hex()})")


if __name__ == "__main__":
    default_matter_test_main()
