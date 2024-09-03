#
#    Copyright (c) 2024 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.tlv import TLVReader
from chip.utils import CommissioningBuildingBlocks
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from test_plan_support import (commission_from_existing, commission_if_required, read_attribute, remove_fabric,
                               verify_commissioning_successful, verify_success)


def verify_fabric(controller: str) -> str:
    return (f"- Verify there is one entry returned. Verify FabricIndex matches `fabric_index_{controller}`.\n"
            f"- Verify the RootPublicKey matches the public key for rcac_{controller}.\n"
            f"- Verify the VendorID matches the vendor ID for {controller}.\n"
            f"- Verify the FabricID matches the fabricID for {controller}")


class TC_OPCREDS_3_2(MatterBaseTest):
    def desc_TC_OPCREDS_3_2(self):
        return " Attribute-CurrentFabricIndex validation [DUTServer]"

    def steps_TC_OPCREDS_3_2(self):
        return [TestStep(0, commission_if_required('CR1'), is_commissioning=True),
                TestStep(1, f"{commission_from_existing('CR1', 'CR2')}\n. Save the FabricIndex from the NOCResponse as `fabric_index_CR2`.",
                         verify_commissioning_successful()),
                TestStep(2, f"{commission_from_existing('CR1', 'CR3')}\n. Save the FabricIndex from the NOCResponse as `fabric_index_CR3`.",
                         verify_commissioning_successful()),
                TestStep(3, f"CR2 {read_attribute('CurrentFabricIndex')}",
                         "Verify the returned value is `fabric_index_CR2`"),
                TestStep(4, f"CR3 {read_attribute('CurrentFabricIndex')}",
                         "Verify the returned value is `fabric_index_CR3`"),
                TestStep(
                    5, f"CR2 {read_attribute('Fabrics')} using a fabric-filtered read", verify_fabric('CR2')),
                TestStep(
                    6, f"CR3 {read_attribute('Fabrics')} using a fabric-filtered read", verify_fabric('CR3')),
                TestStep(7, remove_fabric(
                    'fabric_index_CR2', 'CR1'), verify_success()),
                TestStep(8, remove_fabric(
                    'fabric_index_CR3', 'CR1'), verify_success()),
                ]

    @async_test_body
    async def test_TC_OPCREDS_3_2(self):
        opcreds = Clusters.OperationalCredentials

        self.step(0)

        self.step(1)
        dev_ctrl = self.default_controller

        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_vid = 0xFFF2
        cr2_fabricId = 2222
        cr2_new_fabric_admin = new_certificate_authority.NewFabricAdmin(
            vendorId=cr2_vid, fabricId=cr2_fabricId)
        cr2_nodeid = self.default_controller.nodeId+1
        cr2_dut_node_id = self.dut_node_id+1

        cr2_new_admin_ctrl = cr2_new_fabric_admin.NewController(
            nodeId=cr2_nodeid)
        success, nocResp, rcacResp = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=cr2_new_admin_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )

        fabric_index_CR2 = nocResp.fabricIndex
        tlvReaderRCAC_CR2 = TLVReader(rcacResp).get()["Any"]
        rcac_CR2 = tlvReaderRCAC_CR2[9]  # public key is field 9

        self.step(2)
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr3_vid = 0xFFF3
        cr3_fabricId = 3333
        cr3_new_fabric_admin = new_certificate_authority.NewFabricAdmin(
            vendorId=cr3_vid, fabricId=cr3_fabricId)
        cr3_nodeid = self.default_controller.nodeId+2
        cr3_dut_node_id = self.dut_node_id+2

        cr3_new_admin_ctrl = cr3_new_fabric_admin.NewController(
            nodeId=cr3_nodeid)
        success, nocResp, rcacResp = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=cr3_new_admin_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=cr3_dut_node_id
        )

        fabric_index_CR3 = nocResp.fabricIndex
        tlvReaderRCAC_CR3 = TLVReader(rcacResp).get()["Any"]
        rcac_CR3 = tlvReaderRCAC_CR3[9]

        self.step(3)
        cr2_read_fabricIndex = await self.read_single_attribute_check_success(
            dev_ctrl=cr2_new_admin_ctrl,
            node_id=cr2_dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.CurrentFabricIndex
        )

        asserts.assert_equal(fabric_index_CR2, cr2_read_fabricIndex,
                             "Fail fabric_index_CR2 is not equal to read fabricIndex from CR2")

        self.step(4)
        cr3_read_fabricIndex = await self.read_single_attribute_check_success(
            dev_ctrl=cr3_new_admin_ctrl,
            node_id=cr3_dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.CurrentFabricIndex
        )

        asserts.assert_equal(fabric_index_CR3, cr3_read_fabricIndex,
                             "Fail fabric_index_CR3 is not equal to read fabricIndex from CR3")

        self.step(5)
        cr2_fabric = await self.read_single_attribute_check_success(
            dev_ctrl=cr2_new_admin_ctrl,
            node_id=cr2_dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.Fabrics,
            fabric_filtered=True
        )

        for fabric in cr2_fabric:
            cr2_fabric_fabricIndex = fabric.fabricIndex
            cr2_fabric_rootPublicKey = fabric.rootPublicKey
            cr2_fabric_vendorId = fabric.vendorID
            cr2_fabric_fabricId = fabric.fabricID

        asserts.assert_equal(cr2_fabric_fabricIndex,
                             fabric_index_CR2, "Unexpected CR2 fabric index")
        asserts.assert_equal(cr2_fabric_rootPublicKey, rcac_CR2,
                             "Unexpected RootPublicKey does not match with rcac_CR2")
        asserts.assert_equal(cr2_fabric_vendorId, cr2_vid,
                             "Unexpected vendorId does not match with CR2 VendorID")
        asserts.assert_equal(cr2_fabric_fabricId, cr2_fabricId,
                             "Unexpected fabricId does not match with CR2 fabricID")

        self.step(6)
        cr3_fabric = await self.read_single_attribute_check_success(
            dev_ctrl=cr3_new_admin_ctrl,
            node_id=cr3_dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.Fabrics,
            fabric_filtered=True
        )

        for fabric in cr3_fabric:
            cr3_fabric_fabricIndex = fabric.fabricIndex
            cr3_fabric_rootPublicKey = fabric.rootPublicKey
            cr3_fabric_vendorId = fabric.vendorID
            cr3_fabric_fabricId = fabric.fabricID

        asserts.assert_equal(cr3_fabric_fabricIndex,
                             fabric_index_CR3, "Unexpected CR3 fabric index")
        asserts.assert_equal(cr3_fabric_rootPublicKey, rcac_CR3,
                             "Unexpected RootPublicKey does not match with rcac_CR3")
        asserts.assert_equal(cr3_fabric_vendorId, cr3_vid,
                             "Unexpected vendorId does not match with CR3 VendorID")
        asserts.assert_equal(cr3_fabric_fabricId, cr3_fabricId,
                             "Unexpected fabricId does not match with CR3 fabricID")

        self.step(7)
        cmd = opcreds.Commands.RemoveFabric(fabric_index_CR2)
        resp = await self.send_single_cmd(cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk)

        self.step(8)
        cmd = opcreds.Commands.RemoveFabric(fabric_index_CR3)
        resp = await self.send_single_cmd(cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk)


if __name__ == "__main__":
    default_matter_test_main()
