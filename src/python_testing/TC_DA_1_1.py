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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     app-ready-pattern: "APP STATUS: Starting event loop"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import random

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.commissioning import CommissioningInfo, SetupParameters, commission_device
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main
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
        return [
            TestStep("precondition", "DUT is commissioned to TH1's fabric", is_commissioning=True),
            TestStep(1, "TH1 does a non-fabric filtered read of the NOCs attribute from the Node Operational Credentials cluster and saves the returned list as nocs_th1"),
            TestStep(2, "TH1 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster", """
                     - Verify that TH1's fabric is present in the list
                     - Locate TH1's NOC entry by matching fabric index"""),
            TestStep(3, "Factory reset DUT", "Perform the necessary actions to put the DUT into a commissionable state"),
            TestStep(4, "TH2 opens a PASE session with the DUT"),
            TestStep(5, "TH2 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster", """
                     - Verify that TH1's fabric (public key + fabric ID) is not present in TH2's Fabrics list"""),
            TestStep(6, "DUT is commissioned to TH2's fabric"),
            TestStep(7, "TH2 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster", """
                     - Verify that TH2's FabricID is present in the Fabrics list
                     - Verify that TH2 and TH1's Fabrics attribute's FabricIDs are different"""),
            TestStep(8, "TH2 does a non-fabric-filtered read of the NOCs attribute from the Node Operational Credentials cluster", """
                     - Locate TH2's NOC entry by matching fabric index
                     - Verify that TH2's NOCs entry's public key is different than TH1's NOCs entry's public key"""),
            TestStep(9, "TH2 opens an Enhanced Commissioning Window on the DUT"),
            TestStep(10, "TH1 commissions DUT to TH1's fabric via the window; TH1 removes TH2's fabric", """
                     - Verify that TH1's fabric is present in the Fabrics list
                     - Verify that TH2's fabric is absent from the Fabrics list""")
        ]

    def get_new_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        return new_fabric_admin.NewController(paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

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

    @async_test_body
    async def test_TC_DA_1_1(self):
        # Get setup payload info from the original commissioning parameters
        setupPayloadInfo = self.get_setup_payload_info()
        if not setupPayloadInfo:
            asserts.fail(f"Setup payload info is required for commissioning, found '{setupPayloadInfo}'")

        # Build setup parameters for commissioning
        setup_params = SetupParameters(
            discriminator=setupPayloadInfo[0].filter_value,
            passcode=setupPayloadInfo[0].passcode
        )

        # Setup
        root_node_id = 0
        self.discriminator = random.randint(0, 4095)
        th1 = self.default_controller
        th2 = self.get_new_controller()
        opcreds_cluster = Clusters.OperationalCredentials
        fabrics_attr = opcreds_cluster.Attributes.Fabrics
        nocs_attr = opcreds_cluster.Attributes.NOCs
        commissioning_info = CommissioningInfo(
            commissionee_ip_address_just_for_testing=self.matter_test_config.commissionee_ip_address_just_for_testing,
            commissioning_method=self.matter_test_config.commissioning_method,
            thread_operational_dataset=self.matter_test_config.thread_operational_dataset,
            wifi_passphrase=self.matter_test_config.wifi_passphrase,
            wifi_ssid=self.matter_test_config.wifi_ssid,
            tc_version_to_simulate=self.matter_test_config.tc_version_to_simulate,
            tc_user_response_to_simulate=self.matter_test_config.tc_user_response_to_simulate,
            thread_ba_host=self.matter_test_config.thread_ba_host,
            thread_ba_port=self.matter_test_config.thread_ba_port,
        )

        # *** PRECONDITION ***
        # DUT is commissioned to TH1's fabric
        self.step("precondition")

        # *** STEP 1 ***
        # TH1 does a non-fabric filtered read of the NOCs attribute from the Node
        # Operational Credentials cluster and saves the returned list as nocs_th1
        self.step(1)
        nocs_th1 = await self.read_nocs(th1)

        # *** STEP 2 ***
        # TH1 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster
        self.step(2)
        fabrics_th1 = await self.read_fabrics(th1)

        # Verify that TH1's fabric is present in the list (device may have pre-existing fabrics)
        th1_fabric = next((f for f in fabrics_th1 if f.fabricID == th1.fabricId), None)
        asserts.assert_is_not_none(
            th1_fabric, f"TH1 FabricID ({th1.fabricId}) not found in Fabrics list: {[f.fabricID for f in fabrics_th1]}")

        # Locate TH1's NOC entry by matching fabric index
        th1_noc = next((n for n in nocs_th1 if n.fabricIndex == th1_fabric.fabricIndex), None)
        asserts.assert_is_not_none(th1_noc, f"No NOC entry found for TH1's fabric index ({th1_fabric.fabricIndex})")

        # *** STEP 3 ***
        # Factory reset DUT
        self.step(3)
        await self.request_device_factory_reset()

        # *** STEP 4 ***
        # TH2 opens a PASE session with the DUT
        self.step(4)
        pase_node_id = self.dut_node_id + 1
        await th2.FindOrEstablishPASESession(setupCode=setup_params.qr_code, nodeId=pase_node_id)

        # *** STEP 5 ***
        # TH2 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster
        self.step(5)
        fabrics_th2_pase = await th2.ReadAttribute(
            nodeId=pase_node_id,
            attributes=[(root_node_id, fabrics_attr)],
            fabricFiltered=False
        )

        # Verify that TH1's fabric (public key + fabric ID) is not present in TH2's Fabrics list
        fabrics_th2_ids_pase = [(f.rootPublicKey, f.fabricID) for f in fabrics_th2_pase[0][opcreds_cluster][fabrics_attr]]
        th1_fabric_identity = (th1_fabric.rootPublicKey, th1_fabric.fabricID)
        asserts.assert_not_in(th1_fabric_identity, fabrics_th2_ids_pase,
                              f"TH1's fabric (rootPublicKey={th1_fabric.rootPublicKey.hex()}, fabricID={th1_fabric.fabricID}) should not be present in TH2's Fabrics list, found: {fabrics_th2_ids_pase}")

        # *** STEP 6 ***
        # DUT is commissioned to TH2's fabric
        self.step(6)
        th2_node_id = pase_node_id
        status = await commission_device(th2, th2_node_id, setupPayloadInfo[0], commissioning_info)
        asserts.assert_true(status, f"TH2 commissioning failed: {status}")

        # *** STEP 7 ***
        # TH2 does a non-fabric-filtered read of the Fabrics attribute from the Node Operational Credentials cluster
        self.step(7)
        fabrics_th2 = await self.read_single_attribute_check_success(
            dev_ctrl=th2,
            node_id=th2_node_id,
            endpoint=0,
            cluster=opcreds_cluster,
            attribute=fabrics_attr,
            fabric_filtered=False)

        # Verify that TH2's FabricID is present in the Fabrics list
        th2_fabric = next((f for f in fabrics_th2 if f.fabricID == th2.fabricId), None)
        asserts.assert_is_not_none(th2_fabric,
                                   f"TH2 FabricID ({th2.fabricId}) not found in Fabrics list: {[f.fabricID for f in fabrics_th2]}")

        # Verify that TH2 and TH1's Fabrics attribute's FabricIDs are different
        asserts.assert_not_equal(th2_fabric.fabricID, th1_fabric.fabricID,
                                 f"TH2's FabricID ({th2_fabric.fabricID}) must be different from TH1's FabricID ({th1_fabric.fabricID})")

        # *** STEP 8 ***
        # TH2 does a non-fabric-filtered read of the NOCs attribute from the Node Operational Credentials cluster
        self.step(8)
        nocs_th2 = await self.read_single_attribute_check_success(
            dev_ctrl=th2,
            node_id=th2_node_id,
            endpoint=0,
            cluster=opcreds_cluster,
            attribute=nocs_attr,
            fabric_filtered=False)

        # Locate TH2's NOC entry by matching fabric index
        th2_noc = next((n for n in nocs_th2 if n.fabricIndex == th2_fabric.fabricIndex), None)
        asserts.assert_is_not_none(th2_noc, f"No NOC entry found for TH2's fabric index ({th2_fabric.fabricIndex})")

        # Verify that TH2's NOCs entry's public key is different than TH1's NOCs entry's public key
        nocs_th1_decoded_pk = TLVReader(th1_noc.noc).get()["Any"][9]
        nocs_th2_decoded_pk = TLVReader(th2_noc.noc).get()["Any"][9]
        asserts.assert_not_equal(nocs_th1_decoded_pk, nocs_th2_decoded_pk,
                                 f"The public key of the TH2 NOCs entry ({nocs_th2_decoded_pk.hex()}) must be different from TH1's NOCs entry public key ({nocs_th1_decoded_pk.hex()})")

        # *** STEP 9 ***
        # TH2 opens an Enhanced Commissioning Window on the DUT
        self.step(9)
        params = await th2.OpenCommissioningWindow(
            nodeId=th2_node_id,
            timeout=180,
            iteration=1000,
            discriminator=self.discriminator,
            option=1
        )

        # *** STEP 10 ***
        # TH1 commissions via the window; TH1 removes TH2's fabric
        self.step(10)
        await th1.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminator
        )

        # TH1 removes TH2's fabric
        remove_fabric_cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_fabric.fabricIndex)
        await th1.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=remove_fabric_cmd)

        fabrics_final = await self.read_fabrics(th1)
        fabric_ids_final = [(f.rootPublicKey, f.fabricID) for f in fabrics_final]
        th1_fabric_identity = (th1_fabric.rootPublicKey, th1_fabric.fabricID)
        th2_fabric_identity = (th2_fabric.rootPublicKey, th2_fabric.fabricID)

        # Verify that TH1's fabric is present in the Fabrics list
        asserts.assert_in(th1_fabric_identity, fabric_ids_final,
                          "TH1's fabric should be present in Fabrics list after re-commissioning")

        # Verify that TH2's fabric is absent from the Fabrics list
        asserts.assert_not_in(th2_fabric_identity, fabric_ids_final,
                              "TH2's fabric should be absent from Fabrics list after removal")


if __name__ == "__main__":
    default_matter_test_main()
