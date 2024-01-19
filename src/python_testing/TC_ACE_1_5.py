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

import logging

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACE_1_5(MatterBaseTest):

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    async def write_acl(self, acl: Clusters.AccessControl, th: ChipDeviceCtrl):
        result = await th.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

    @async_test_body
    async def test_TC_ACE_1_5(self):
        self.print_step(1, "Comissioning, already done")
        self.th1 = self.default_controller

        # TODO: move into base class and adjust tests (#31521)
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)

        TH1_nodeid = self.matter_test_config.controller_node_id
        TH2_nodeid = self.matter_test_config.controller_node_id + 2

        self.th2 = new_fabric_admin.NewController(nodeId=TH2_nodeid,
                                                  paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

        params = self.openCommissioningWindow(self.th1, self.dut_node_id)
        self.print_step(2, "TH1 opens the commissioning window on the DUT")

        errcode = self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator)
        logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
        self.print_step(3, "TH2 commissions DUT using admin node ID N2")

        self.print_step(4, "TH2 reads its fabric index from the Operational Credentials cluster CurrentFabricIndex attribute")
        th2FabricIndex = await self.read_currentfabricindex(self.th2)

        self.print_step(
            5, "TH1 writes DUT Endpoint 0 ACL cluster ACL attribute, value is list of ACLEntryStruct containing 2 elements")
        admin_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])
        descriptor_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.Descriptor.id)])
        acl = [admin_acl, descriptor_view]
        await self.write_acl(acl, self.th1)

        self.print_step(
            6, "TH2 writes DUT Endpoint 0 ACL cluster ACL attribute, value is list of ACLEntryStruct containing 2 elements")
        admin_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=th2FabricIndex,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH2_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])
        descriptor_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=th2FabricIndex,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.BasicInformation.id)])
        acl = [admin_acl, descriptor_view]
        await self.write_acl(acl, self.th2)

        self.print_step(7, "TH1 reads DUT Endpoint 0 Descriptor cluster DeviceTypeList attribute")
        await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Descriptor.Attributes.DeviceTypeList)

        self.print_step(8, "TH1 reads DUT Endpoint 0 Basic Information cluster VendorID attribute")
        await self.read_single_attribute_expect_error(
            dev_ctrl=self.th1, endpoint=0,
            cluster=Clusters.Objects.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID,
            error=Status.UnsupportedAccess)

        self.print_step(9, "TH2 reads DUT Endpoint 0 Descriptor cluster DeviceTypeList attribute")
        await self.read_single_attribute_expect_error(
            dev_ctrl=self.th2, endpoint=0,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Descriptor.Attributes.DeviceTypeList,
            error=Status.UnsupportedAccess)

        self.print_step(10, "TH2 reads DUT Endpoint 0 Basic Information cluster VendorID attribute")
        await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0,
            cluster=Clusters.Objects.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID)

        self.print_step(11, "TH1 resets the ACLs to default value by writing DUT EP0")
        full_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid],
            targets=[])

        acl = [full_acl]
        await self.write_acl(acl, self.th1)

        self.print_step(
            12, "TH1 removes the TH2 fabric by sending the RemoveFabric command to the DUT with the FabricIndex set to th2FabricIndex")
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2FabricIndex)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)


if __name__ == "__main__":
    default_matter_test_main()
