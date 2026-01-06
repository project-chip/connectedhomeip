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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
# === END CI TEST ARGUMENTS ===

import logging
import random

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_ACL_2_4(MatterBaseTest):
    def desc_TC_ACL_2_4(self) -> str:
        return "[TC-ACL-2.4] ACL Attribute"

    def pics_TC_ACL_2_4(self) -> list[str]:
        return ['ACL.S']

    def steps_TC_ACL_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "TH1 commissions DUT using admin node ID N1", is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute and saves as acl_original",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 1 element with admin entry N1"),
            TestStep(4, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with 3 elements:\n" +
                     "1. Admin entry (N1, privilege: Administer, authMode: CASE)\n" +
                     "2. View entry (subjects: [111, 222, 333, 444], privilege: View, authMode: Group, targets: [{Cluster: 11}, {Endpoint: 22}])\n" +
                     "3. Operate entry (subjects: [555, 666, 777, 888], privilege: Operate, authMode: Group, targets: [{Cluster: 55}, {Endpoint: 66}])",
                     "Result is SUCCESS"),
            TestStep(5, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 3 elements"),
            TestStep(6, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with modified entries (N1 admin, N2 manage, N3 admin)", "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 3 elements with updated privileges"),
            TestStep(8, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with updated entries", "Result is SUCCESS"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 3 elements with updated entries"),
            TestStep(10, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with null subjects in the last 2 elements ", "Result is SUCCESS"),
            TestStep(11, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 3 elements with null subjects in the last 2 elements"),
            TestStep(12, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with null targets", "Result is SUCCESS"),
            TestStep(13, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 3 elements with null targets"),
            TestStep(14, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with 2 elements", "Result is SUCCESS"),
            TestStep(15, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 2 elements"),
            TestStep(16, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with ProxyView privilege", "Result is SUCCESS"),
            TestStep(17, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 2 elements with ProxyView privilege for the last element"),
            TestStep(18, "TH1 reads DUT Endpoint 0 AccessControl cluster SubjectsPerAccessControlEntry attribute",
                     "Result is SUCCESS, value is stored as MAXSUBJECTS"),
            TestStep(19, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with max subjects", "Result is SUCCESS"),
            TestStep(20, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value contains MAXSUBJECTS random node IDs"),
            TestStep(21, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with 4 valid CATs as subjects in the last entry", "Result is SUCCESS"),
            TestStep(22, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 2 elements with last element having 4 valid CATs as subjects"),
            TestStep(23, "TH1 reads DUT Endpoint 0 AccessControl cluster TargetsPerAccessControlEntry attribute",
                     "Result is SUCCESS, value is stored as MAXTARGETS"),
            TestStep(24, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with max targets", "Result is SUCCESS"),
            TestStep(25, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value contains MAXTARGETS targets"),
            TestStep(26, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntriesPerFabric attribute",
                     "Result is SUCCESS, value is stored as MAXENTRIES"),
            TestStep(27, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute with a list of MAXENTRIES elements", "Result is SUCCESS"),
            TestStep(28, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing MAXENTRIES elements"),
            TestStep(29, "TH1 resets DUT Endpoint 0 AccessControl cluster ACL attribute, writing it with acl_original",
                     "Result is SUCCESS"),
            TestStep(30, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing PASE auth mode",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected "),
            TestStep(31, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value contains only the admin entry"),
            TestStep(32, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid combination of Group auth mode with admin privilege",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(33, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid privilege",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(34, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid auth mode",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(35, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid subject ID 0",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(36, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing max node ID",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(37, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing an invalid CAT as subject",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(38, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid Group Node ID",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(39, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing empty target",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(40, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid cluster ID",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(41, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid endpoint ID",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(42, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing invalid device type",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(43, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing an invalid target entry containing both endpoint and device type fields in the same entry",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(44, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute testing an invalid target entry containing endpoint, device type and cluster fields in the same entry",
                     "Result is CONSTRAINT_ERROR.  Second invalid element is rejected"),
            TestStep(45, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute value is acl_original",
                     "Result is SUCCESS"),
        ]

    @async_test_body
    async def test_TC_ACL_2_4(self):
        # Step 1: Commission the device
        self.step(1)
        self.th1 = self.default_controller

        # Step 2: Read CurrentFabricIndex
        self.step(2)
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=oc_cluster,
            attribute=cfi_attribute
        )
        log.info(f"CurrentFabricIndex: {f1}")

        # Step 3: Read initial ACL attribute
        self.step(3)
        acl_cluster = Clusters.AccessControl
        acl_attribute = acl_cluster.Attributes.Acl
        acl_original = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )

        # Verify initial ACL contains single admin entry
        asserts.assert_equal(len(acl_original), 1, "Initial ACL should contain exactly 1 entry")
        asserts.assert_equal(acl_original[0].privilege, Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister)
        asserts.assert_equal(acl_original[0].authMode, Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase)
        asserts.assert_equal(acl_original[0].subjects, [self.th1.nodeId])
        asserts.assert_true(isinstance(acl_original[0].targets, Nullable), "Targest should currently be null")
        asserts.assert_equal(acl_original[0].fabricIndex, f1)

        # Step 4: Write ACL with 3 entries
        self.step(4)
        new_acl = [
            # Admin entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # View entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[111, 222, 333, 444],
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=11,
                        endpoint=NullValue,
                        deviceType=NullValue
                    ),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=22,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            ),
            # Operate entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[555, 666, 777, 888],
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=55,
                        endpoint=NullValue,
                        deviceType=NullValue
                    ),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=66,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=new_acl))]
        )
        # Add explicit check for unexpected errors
        if result[0].Status != Status.Success:
            asserts.fail(f"Expected Success but got {result[0].Status}")

        # Step 5: Read back and verify ACL
        self.step(5)
        read_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read ACL: {read_acl}")

        asserts.assert_equal(len(read_acl), 3, "ACL should contain exactly 3 entries")
        asserts.assert_in(new_acl[0], read_acl, "Missing Admin entry")
        asserts.assert_in(new_acl[1], read_acl, "Missing View entry")
        asserts.assert_in(new_acl[2], read_acl, "Missing Operate entry")

        # Step 6: Write modified ACL entries
        self.step(6)
        modified_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Manage entry (modified from View)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[444, 333, 222, 111],
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=44,
                        endpoint=NullValue,
                        deviceType=NullValue
                    ),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=33,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            ),
            # Admin entry (modified from Operate)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[888, 777, 666, 555],
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=88,
                        endpoint=NullValue,
                        deviceType=NullValue
                    ),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=77,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=modified_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success)

        # Step 7: Read and verify modified ACL
        self.step(7)
        read_modified_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Modified ACL: {read_modified_acl}")

        # Verify modified ACL contents
        asserts.assert_equal(len(read_modified_acl), 3, "ACL should contain exactly 3 entries")
        asserts.assert_in(modified_acl[0], read_modified_acl, "Missing Admin entry")
        asserts.assert_in(modified_acl[1], read_modified_acl, "Missing Manage entry")
        asserts.assert_in(modified_acl[2], read_modified_acl, "Missing Admin entry")

        # Step 8: Write ACL with updated targets
        self.step(8)
        updated_targets_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # View entry with complex targets
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[111, 222, 333, 444],
                targets=[
                    # First target with cluster and endpoint
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=11,
                        endpoint=22,
                        deviceType=NullValue
                    ),
                    # Second target with cluster and deviceType
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=33,
                        endpoint=NullValue,
                        deviceType=44
                    )
                ],
                fabricIndex=f1
            ),
            # Operate entry with complex targets
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[555, 666, 777, 888],
                targets=[
                    # First target with cluster and endpoint
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=55,
                        endpoint=66,
                        deviceType=NullValue
                    ),
                    # Second target with cluster and deviceType
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=77,
                        endpoint=NullValue,
                        deviceType=88
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=updated_targets_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with updated targets should succeed")

        # Step 9: Read and verify updated targets ACL
        self.step(9)
        read_updated_targets_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Updated Targets ACL: {read_updated_targets_acl}")

        # Verify updated targets ACL contents
        asserts.assert_equal(len(read_updated_targets_acl), 3, "ACL should contain exactly 3 entries")
        asserts.assert_in(updated_targets_acl[0], read_updated_targets_acl, "Missing Admin entry")
        asserts.assert_in(updated_targets_acl[1], read_updated_targets_acl, "Missing View entry")
        asserts.assert_in(updated_targets_acl[2], read_updated_targets_acl, "Missing Operate entry")

        # Step 10: Write ACL with null subjects
        self.step(10)
        null_subjects_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # View entry with null subjects
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=11,
                        endpoint=22,
                        deviceType=NullValue
                    ),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=33,
                        endpoint=NullValue,
                        deviceType=44
                    )
                ],
                fabricIndex=f1
            ),
            # Operate entry with null subjects
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=55,
                        endpoint=66,
                        deviceType=NullValue
                    ),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=77,
                        endpoint=NullValue,
                        deviceType=88
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=null_subjects_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with null subjects should succeed")

        # Step 11: Read and verify null subjects ACL
        self.step(11)
        read_null_subjects_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Null Subjects ACL: {read_null_subjects_acl}")

        # Verify null subjects ACL contents
        asserts.assert_equal(len(read_null_subjects_acl), 3, "ACL should contain exactly 3 entries")
        asserts.assert_in(null_subjects_acl[0], read_null_subjects_acl, "Missing Admin entry")
        asserts.assert_in(null_subjects_acl[1], read_null_subjects_acl, "Missing View entry")
        asserts.assert_in(null_subjects_acl[2], read_null_subjects_acl, "Missing Operate entry")

        # Step 12: Write ACL with null targets
        self.step(12)
        null_targets_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # View entry with null targets
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[111, 222, 333, 444],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Operate entry with null targets
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[555, 666, 777, 888],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=null_targets_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with null targets should succeed")

        # Step 13: Read and verify null targets ACL
        self.step(13)
        read_null_targets_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Null Targets ACL: {read_null_targets_acl}")

        # Verify null targets ACL contents
        asserts.assert_equal(len(read_null_targets_acl), 3, "ACL should contain exactly 3 entries")
        asserts.assert_in(null_targets_acl[0], read_null_targets_acl, "Missing Admin entry")
        asserts.assert_in(null_targets_acl[1], read_null_targets_acl, "Missing View entry")
        asserts.assert_in(null_targets_acl[2], read_null_targets_acl, "Missing Operate entry")

        # Step 14: Write ACL with 2 elements
        self.step(14)
        two_element_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Operate entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=two_element_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with 2 elements should succeed")

        # Step 15: Read and verify two element ACL
        self.step(15)
        read_two_element_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Two Element ACL: {read_two_element_acl}")

        # Verify two element ACL contents
        asserts.assert_equal(len(read_two_element_acl), 2, "ACL should contain exactly 2 entries")
        asserts.assert_in(two_element_acl[0], read_two_element_acl, "Missing Admin entry")
        asserts.assert_in(two_element_acl[1], read_two_element_acl, "Missing Operate entry")

        # Step 16: Write ACL with ProxyView privilege
        self.step(16)
        proxy_view_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # ProxyView entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kProxyView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=proxy_view_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with ProxyView privilege should succeed")

        # Step 17: Read and verify ProxyView ACL
        self.step(17)
        read_proxy_view_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read ProxyView ACL: {read_proxy_view_acl}")

        # Verify ProxyView ACL contents
        asserts.assert_equal(len(read_proxy_view_acl), 2, "ACL should contain exactly 2 entries")
        asserts.assert_in(proxy_view_acl[0], read_proxy_view_acl, "Missing Admin entry")
        asserts.assert_in(proxy_view_acl[1], read_proxy_view_acl, "Missing ProxyView entry")

        # Step 18: Read SubjectsPerAccessControlEntry attribute
        self.step(18)
        subjects_per_entry_attr = acl_cluster.Attributes.SubjectsPerAccessControlEntry
        max_subjects = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=subjects_per_entry_attr
        )
        log.info(f"SubjectsPerAccessControlEntry value: {max_subjects}")

        # Step 19: Write ACL with max subjects
        self.step(19)
        random_subjects = [random.randint(1, 0xFFFFFFFF) for _ in range(max_subjects)]
        max_subjects_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with max subjects
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=random_subjects,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=max_subjects_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with max subjects should succeed")

        # Step 20: Read and verify max subjects ACL
        self.step(20)
        read_max_subjects_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Max Subjects ACL: {read_max_subjects_acl}")

        # Verify max subjects ACL contents
        asserts.assert_equal(len(read_max_subjects_acl), 2, "ACL should contain exactly 2 entries")
        asserts.assert_in(max_subjects_acl[0], read_max_subjects_acl, "Missing Admin entry")
        asserts.assert_in(max_subjects_acl[1], read_max_subjects_acl, "Missing Operate entry")

        # Step 21: Write ACL with 4 valid CATs as subjects
        self.step(21)
        # Define various Valid CATs taken from Test Spec
        CAT1 = 0xFFFFFFFDABCD0001  # 'CAT1' in hex
        CAT2 = 0xFFFFFFFDFFFF0001  # 'CAT2' in hex
        CAT3 = 0xFFFFFFFD071C1074  # 'CAT3' in hex
        CAT4 = 0xFFFFFFFDAB120003  # 'CAT4' in hex

        specific_subjects_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with specific subjects
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[CAT1, CAT2, CAT3, CAT4],  # Using defined constants
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=specific_subjects_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with specific subjects should succeed")

        # Step 22: Read and verify specific subjects ACL
        self.step(22)
        read_specific_subjects_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Specific Subjects ACL: {read_specific_subjects_acl}")

        # Verify specific subjects ACL contents
        asserts.assert_equal(len(read_specific_subjects_acl), 2, "ACL should contain exactly 2 entries")
        asserts.assert_in(specific_subjects_acl[0], read_specific_subjects_acl, "Missing Admin entry")
        asserts.assert_in(specific_subjects_acl[1], read_specific_subjects_acl, "Missing Operate entry")

        # Step 23: Read TargetsPerAccessControlEntry attribute
        self.step(23)
        targets_per_entry_attr = acl_cluster.Attributes.TargetsPerAccessControlEntry
        max_targets = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=targets_per_entry_attr
        )
        log.info(f"TargetsPerAccessControlEntry value: {max_targets}")

        # Step 24: Write ACL with max targets
        self.step(24)
        random_targets = []
        for i in range(max_targets):
            # Alternate between different valid target configurations
            if i % 3 == 0:
                # Target with cluster only
                target = Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    cluster=i + 1,
                    endpoint=NullValue,
                    deviceType=NullValue
                )
            elif i % 3 == 1:
                # Target with endpoint only
                target = Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    cluster=NullValue,
                    endpoint=i + 1,
                    deviceType=NullValue
                )
            else:
                # Target with deviceType only
                target = Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    cluster=NullValue,
                    endpoint=NullValue,
                    deviceType=i + 1
                )
            random_targets.append(target)

        max_targets_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with max targets
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=random_targets,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=max_targets_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with max targets should succeed")

        # Step 25: Read and verify max targets ACL
        self.step(25)
        read_max_targets_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Max Targets ACL: {read_max_targets_acl}")

        # Verify max targets ACL contents
        asserts.assert_equal(len(read_max_targets_acl), 2, "ACL should contain exactly 2 entries")
        asserts.assert_in(max_targets_acl[0], read_max_targets_acl, "Missing Admin entry")
        asserts.assert_in(max_targets_acl[1], read_max_targets_acl, "Missing Operate entry")

        # Step 26: Read AccessControlEntriesPerFabric attribute
        self.step(26)
        entries_per_fabric_attr = acl_cluster.Attributes.AccessControlEntriesPerFabric
        max_entries = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=entries_per_fabric_attr
        )
        log.info(f"AccessControlEntriesPerFabric value: {max_entries}")

        # Step 27: Write ACL with MAXENTRIES elements
        self.step(27)
        max_entries_acl = [
            # First entry is always admin
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        # Add remaining entries up to MAXENTRIES
        for i in range(1, max_entries):
            entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
            max_entries_acl.append(entry)

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=max_entries_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write ACL with max entries should succeed")

        # Step 28: Read and verify max entries ACL
        self.step(28)
        read_max_entries_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read Max Entries ACL: {read_max_entries_acl}")

        # Verify max entries ACL contents
        asserts.assert_equal(len(read_max_entries_acl), max_entries,
                             f"ACL should contain exactly {max_entries} entries")
        asserts.assert_in(max_entries_acl[0], read_max_entries_acl, "Missing Admin entry")
        asserts.assert_in(max_entries_acl[1], read_max_entries_acl, "Missing Operate entry")

        # First reset ACL to only admin entry
        self.step(29)
        admin_only_acl = [
            # Admin entry only
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        # Reset to admin-only ACL first
        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=admin_only_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write admin-only ACL should succeed")

        # Verify reset was successful
        read_admin_only = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(read_admin_only), 1,
                             "ACL should contain only admin entry after reset")

        # Now test PASE auth mode (should fail)
        self.step(30)
        pase_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with PASE auth mode (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kPase,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=pase_acl))]
        )
        # Should fail with CONSTRAINT_ERROR
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with PASE auth mode should fail with CONSTRAINT_ERROR")

        # Step 31: Read and verify ACL after failed PASE write
        self.step(31)
        read_after_pase_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        log.info(f"Read ACL after failed PASE write: {read_after_pase_acl}")

        # Verify only admin entry remains
        asserts.assert_equal(len(read_after_pase_acl), 1,
                             "ACL should contain only admin entry")
        admin_entry = read_after_pase_acl[0]
        asserts.assert_equal(admin_entry.privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Entry should have admin privilege")
        asserts.assert_equal(admin_entry.subjects, [self.th1.nodeId],
                             "Entry should have admin node ID")

        # Step 32: Test Group auth mode for admin (should fail)
        self.step(32)
        group_admin_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with Group auth mode and admin privilege (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=group_admin_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with Group auth mode for admin should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 33: Test invalid privilege value (should fail)
        self.step(33)
        invalid_privilege_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid privilege (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=255,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=invalid_privilege_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid privilege should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 34: Test invalid auth mode value (should fail)
        self.step(34)
        invalid_auth_mode_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid auth mode (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=255,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=invalid_auth_mode_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid auth mode should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 35: Test invalid subject ID 0 (should fail)
        self.step(35)
        invalid_subject_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid subject ID (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[0],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=invalid_subject_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid subject ID should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 36: Test max node ID (should fail)
        self.step(36)
        max_node_id_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with max node ID (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[0xFFFFFFFFFFFFFFFF],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=max_node_id_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with max node ID should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 37: Test invalid CAT (Case-Authenticated Tag) as subject (should fail)
        self.step(37)
        group_id_range_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid group ID range (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[0xFFFFFFFD00000000],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=group_id_range_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with an invalid CAT as subject should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 38: Test invalid Group Node ID (should fail)
        self.step(38)
        fabric_scoped_id_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid Group Node ID (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[0xFFFFFFFFFFFF0000],
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=fabric_scoped_id_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid Group Node ID should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 39: Test empty target (should fail)
        self.step(39)
        empty_target_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with empty target (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=NullValue,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=empty_target_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with empty target should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 40: Test invalid cluster ID (should fail)
        self.step(40)
        invalid_cluster_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid cluster ID (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=0xFFFFFFFF,
                        endpoint=NullValue,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=invalid_cluster_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid cluster ID should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 41: Test invalid endpoint ID (should fail)
        self.step(41)
        invalid_endpoint_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid endpoint ID (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=65535,
                        deviceType=NullValue
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=invalid_endpoint_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid endpoint ID should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 42: Test invalid device type (should fail)
        self.step(42)
        invalid_device_type_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with invalid device type (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=NullValue,
                        deviceType=0xFFFFFFFF
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=invalid_device_type_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with invalid device type should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 43: Test endpoint with device type (should fail)
        self.step(43)
        endpoint_with_device_type_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with both endpoint and device type (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=NullValue,
                        endpoint=22,
                        deviceType=33
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=endpoint_with_device_type_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with both endpoint and device type should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 44: Test all target fields (should fail)
        self.step(44)
        all_target_fields_acl = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Entry with all target fields set (should fail)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=NullValue,
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        cluster=11,
                        endpoint=22,
                        deviceType=33
                    )
                ],
                fabricIndex=f1
            )
        ]

        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=all_target_fields_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "Write ACL with all target fields should fail with CONSTRAINT_ERROR")

        # Verify ACL still contains only admin entry after constraint error
        verify_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(verify_acl), 1, "ACL should contain only admin entry after failed operation")
        asserts.assert_equal(verify_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry must be admin")
        asserts.assert_equal(verify_acl[0].subjects, [self.th1.nodeId],
                             "Admin entry must have correct node ID")

        # Step 45: Write minimum required ACL (admin only)
        self.step(45)
        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=acl_original))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write admin-only ACL should succeed")
        log.info("Successfully reset ACL to admin-only entry")

        # Final verification that ACL contains only admin entry
        final_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(final_acl), 1,
                             "ACL should contain exactly one admin entry")
        asserts.assert_equal(final_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry should have admin privilege")
        asserts.assert_equal(final_acl[0].subjects, [self.th1.nodeId],
                             "Entry should have admin node ID")


if __name__ == "__main__":
    default_matter_test_main()
