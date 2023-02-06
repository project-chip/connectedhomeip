#
#    Copyright (c) 2022 Project CHIP Authors
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
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


def acl_subject(cat: int) -> int:
    return 0xFFFFFFFD00000000 | cat


class TC_ACE_1_3(MatterBaseTest):

    async def write_acl(self, acl):
        # This returns an attribute status
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        print(result)

    async def read_descriptor_expect_success(self, th):
        cluster = Clusters.Objects.Descriptor
        attribute = Clusters.Descriptor.Attributes.DeviceTypeList
        await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)

    async def read_descriptor_expect_unsupported_access(self, th):
        cluster = Clusters.Objects.Descriptor
        attribute = Clusters.Descriptor.Attributes.DeviceTypeList
        await self.read_single_attribute_expect_error(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute, error=Status.UnsupportedAccess)

    @async_test_body
    async def test_TC_ACE_1_3(self):
        cat1_id = 0x11110000
        cat2_id = 0x22220000

        cat1v1 = cat1_id | 0x0001
        cat1v2 = cat1_id | 0x0002
        cat1v3 = cat1_id | 0x0003
        cat2v1 = cat2_id | 0x0001
        cat2v2 = cat2_id | 0x0002
        cat2v3 = cat2_id | 0x0003
        logging.info('cat1v1 0x%x', cat1v1)

        self.print_step(1, "Commissioning, already done")
        TH0 = self.default_controller
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]

        TH0_nodeid = self.matter_test_config.controller_node_id
        TH1_nodeid = self.matter_test_config.controller_node_id + 1
        TH2_nodeid = self.matter_test_config.controller_node_id + 2
        TH3_nodeid = self.matter_test_config.controller_node_id + 3

        TH1 = fabric_admin.NewController(nodeId=TH1_nodeid,
                                         paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
                                         catTags=[cat1v3])
        TH2 = fabric_admin.NewController(nodeId=TH2_nodeid,
                                         paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
                                         catTags=[cat1v2, cat2v1])
        TH3 = fabric_admin.NewController(nodeId=TH3_nodeid,
                                         paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
                                         catTags=[cat1v1, cat2v2])

        self.print_step(2, "TH0 writes ACL all view on PIXIT.ACE.TESTENDPOINT")
        TH0_admin_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                                                                                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                                subjects=[TH0_nodeid],
                                                                                targets=[Clusters.AccessControl.Structs.Target(endpoint=0, cluster=0x001f)])
        all_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                           authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                           subjects=[],
                                                                           targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])
        acl = [TH0_admin_acl, all_view]
        await self.write_acl(acl)

        self.print_step(3, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(4, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(5, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(6, "TH0 writes ACL TH1 view on EP0")
        th1_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                           authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                           subjects=[TH1_nodeid],
                                                                           targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])
        acl = [TH0_admin_acl, th1_view]
        await self.write_acl(acl)
        self.print_step(7, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(8, "TH2 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH2)

        self.print_step(9, "TH3 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH3)

        self.print_step(10, "TH0 writes ACL TH2 view on EP0")
        th2_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                           authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                           subjects=[TH2_nodeid],
                                                                           targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, th2_view]
        await self.write_acl(acl)
        self.print_step(11, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH1)

        self.print_step(12, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(13, "TH3 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH3)

        self.print_step(14, "TH0 writes ACL TH3 view on EP0")
        th3_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                           authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                           subjects=[TH3_nodeid],
                                                                           targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, th3_view]
        await self.write_acl(acl)
        self.print_step(15, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH1)

        self.print_step(16, "TH2 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH2)

        self.print_step(17, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(18, "TH0 writes ACL TH1 TH2 view on EP0")
        th12_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                            subjects=[TH1_nodeid, TH2_nodeid],
                                                                            targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, th12_view]
        await self.write_acl(acl)
        self.print_step(19, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(20, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(21, "TH3 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH3)

        self.print_step(22, "TH0 writes ACL TH1 TH3 view on EP0")
        th13_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                            subjects=[TH1_nodeid, TH3_nodeid],
                                                                            targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, th13_view]
        await self.write_acl(acl)
        self.print_step(23, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(24, "TH2 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH2)

        self.print_step(25, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(26, "TH0 writes ACL TH2 TH3 view on EP0")
        th23_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                            subjects=[TH2_nodeid, TH3_nodeid],
                                                                            targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, th23_view]
        await self.write_acl(acl)
        self.print_step(27, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH1)

        self.print_step(28, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(29, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(30, "TH0 writes ACL TH1 TH2 TH3 view on EP0")
        th123_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                             authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                             subjects=[TH1_nodeid, TH2_nodeid, TH3_nodeid],
                                                                             targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, th123_view]
        await self.write_acl(acl)
        self.print_step(31, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(32, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(33, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(34, "TH0 writes ACL cat1v1 view on EP0")
        cat1v1_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                              authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                              subjects=[acl_subject(cat1v1)],
                                                                              targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])
        acl = [TH0_admin_acl, cat1v1_view]
        await self.write_acl(acl)

        self.print_step(35, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(36, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(37, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(38, "TH0 writes ACL cat1v2 view on EP0")
        cat1v2_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                              authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                              subjects=[acl_subject(cat1v2)],
                                                                              targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, cat1v2_view]
        await self.write_acl(acl)

        self.print_step(39, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(40, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(41, "TH3 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH3)

        self.print_step(42, "TH0 writes ACL cat1v3 view on EP0")
        cat1v3_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                              authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                              subjects=[acl_subject(cat1v3)],
                                                                              targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, cat1v3_view]
        await self.write_acl(acl)

        self.print_step(43, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH1)

        self.print_step(44, "TH2 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH2)

        self.print_step(45, "TH3 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH3)

        self.print_step(46, "TH0 writes ACL cat2v1 view on EP0")
        cat2v1_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                              authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                              subjects=[acl_subject(cat2v1)],
                                                                              targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, cat2v1_view]
        await self.write_acl(acl)

        self.print_step(47, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH1)

        self.print_step(48, "TH2 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH2)

        self.print_step(49, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(50, "TH0 writes ACL cat2v2 view on EP0")
        cat2v2_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                              authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                              subjects=[acl_subject(cat2v2)],
                                                                              targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, cat2v2_view]
        await self.write_acl(acl)

        self.print_step(51, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH1)

        self.print_step(52, "TH2 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH2)

        self.print_step(53, "TH3 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(TH3)

        self.print_step(54, "TH0 writes ACL cat2v3 view on EP0")
        cat2v3_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                                              authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                              subjects=[acl_subject(cat2v3)],
                                                                              targets=[Clusters.AccessControl.Structs.Target(endpoint=0)])

        acl = [TH0_admin_acl, cat2v3_view]
        await self.write_acl(acl)

        self.print_step(55, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH1)

        self.print_step(56, "TH2 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH2)

        self.print_step(57, "TH3 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(TH3)

        self.print_step(58, "TH0 writes ACL back to default")

        acl = [TH0_admin_acl]
        await self.write_acl(acl)


if __name__ == "__main__":
    default_matter_test_main()
