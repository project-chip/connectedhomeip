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

import chip.clusters as Clusters
from chip.tlv import TLVReader
from chip.utils import CommissioningBuildingBlocks
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from test_plan_support import commission_if_required, commission_from_existing, verify_commissioning_successful, read_attribute, remove_fabric, verify_success
from mobly import asserts


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
        # TODO: implement
        opcreds = Clusters.OperationalCredentials

        self.step(0)

        self.step(1)
        # Create a new controller on a new fabric called CR2. Commission the new controller from CR1 as follows:
        dev_ctrl = self.default_controller

        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_vid = 0xFFF1
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
        tlvReaderRCAC = TLVReader(rcacResp).get()["Any"]
        rcac_CR2 = tlvReaderRCAC[9]  # public key is field 9

        print(f"Success: {success}, nocResp: {nocResp}, rcacResp: {rcacResp}")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=cr2_new_admin_ctrl, node_id=cr2_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=False)

        print(f"nocs: {nocs}")

        # rcac_CR2 = TLVReader(rcacResp).get()["Any"]
        # print(rcac_CR2[9])

        self.step(2)
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr3_vid = 0xFFF1
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
        # rcac_CR3 = TLVReader(rcacResp).get()["Any"]

        # print(f"Success: {success}, nocResp: {nocResp}, rcacResp: {rcacResp}")

        self.step(3)
        cr2_read_fabricIndex = await self.read_single_attribute_check_success(dev_ctrl=cr2_new_admin_ctrl, node_id=cr2_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex)
        print(f"fabric_index_CR2: {cr2_read_fabricIndex}")
        if fabric_index_CR2 == cr2_read_fabricIndex:
            print("Success fabric_index_CR2 is equal to read fabricIndex from CR2")
        else:
            print("Fail fabric_index_CR2 is not equal to read fabricIndex from CR2")

        self.step(4)
        cr3_read_fabricIndex = await self.read_single_attribute_check_success(dev_ctrl=cr3_new_admin_ctrl, node_id=cr3_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex)
        print(f"fabric_index_CR3: {cr3_read_fabricIndex}")
        if fabric_index_CR3 == cr3_read_fabricIndex:
            print("Success fabric_index_CR3 is equal to read fabricIndex from CR3")
        else:
            print("Fail fabric_index_CR3 is not equal to read fabricIndex from CR3")

        self.step(5)
        cr2_fabric = await self.read_single_attribute_check_success(dev_ctrl=cr2_new_admin_ctrl, node_id=cr2_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.Fabrics, fabric_filtered=True)
        print(f"cr2_fabric: {cr2_fabric}")

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

        print(f"cr2_fabric_fabricIndex: {cr2_fabric_fabricIndex}, cr2_fabric_rootPublicKey: {cr2_fabric_rootPublicKey}, cr2_fabric_vendorId: {cr2_fabric_vendorId}, cr2_fabric_fabricId: {cr2_fabric_fabricId}")

        self.step(6)
        self.step(7)
        self.step(8)


if __name__ == "__main__":
    default_matter_test_main()
