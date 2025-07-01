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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_ACL_2_9(MatterBaseTest):
    async def read_and_check_min_value(self, attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor, min_value: int):
        result = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, attribute)])
        value = result[0][Clusters.Objects.AccessControl][attribute]
        asserts.assert_greater_equal(
            value,
            min_value,
            f"{attribute.__name__} attribute should be {min_value} or greater, but got {value}"
        )

    async def read_event_expect_unsupported_access(self, event: Clusters.ClusterObjects.ClusterEvent):
        result = await self.default_controller.ReadEvent(self.dut_node_id, [(0, event)])
        asserts.assert_equal(result[0].Status, Status.UnsupportedAccess, f"Expected UnsupportedAccess but got {result[0].Status}")

    def desc_TC_ACL_2_9(self) -> str:
        return "[TC-ACL-2.9] Cluster access"

    def steps_TC_ACL_2_9(self) -> list[TestStep]:
        steps = [
            TestStep(
                1,
                "TH1 commissions DUT using admin node ID N1",
                "DUT is commissioned on TH1 fabric",
                is_commissioning=True),
            TestStep(
                2,
                "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 1 element",
                "Result is SUCCESS"),
            TestStep(
                3,
                "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                4,
                "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 1 element",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                5,
                "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                6,
                "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                7,
                "TH1 reads DUT Endpoint 0 AccessControl cluster SubjectsPerAccessControlEntry attribute",
                "Result is SUCCESS,value is an integer with value 4 or greater."),
            TestStep(
                8,
                "TH1 reads DUT Endpoint 0 AccessControl cluster TargetsPerAccessControlEntry attribute",
                "Result is SUCCESS,value is an integer with value 3 or greater."),
            TestStep(
                9,
                "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntriesPerFabric attribute",
                "Result is SUCCESS, value is an integer with value 4 or greater."),
            TestStep(
                10,
                "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                11,
                "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_9(self):
        self.step(1)
        self.th1 = self.default_controller

        self.step(2)
        # TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is
        # list of AccessControlEntryStruct containing 1 element
        acl_attribute = Clusters.AccessControl.Attributes.Acl
        new_acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
            ),
        ]
        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=new_acl))]
        )
        # Add explicit check for Success
        if result[0].Status != Status.Success:
            asserts.fail(f"Expected Success but got {result[0].Status}")

        self.step(3)
        # TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute
        acl_attribute = Clusters.AccessControl.Attributes.Acl
        await self.read_single_attribute_expect_error(
            cluster=Clusters.Objects.AccessControl,
            attribute=acl_attribute,
            error=Status.UnsupportedAccess,
            endpoint=0
        )

        self.step(4)
        # TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is
        # list of AccessControlEntryStruct containing 1 element
        new_acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
            ),
        ]
        result2 = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=new_acl))]
        )
        # Add explicit check for UnsupportedAccess error
        if result2[0].Status != Status.UnsupportedAccess:
            asserts.fail(f"Expected UnsupportedAccess but got {result2[0].Status}")

        self.step(5)
        # TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute
        extension_attribute = Clusters.AccessControl.Attributes.Extension
        await self.read_single_attribute_expect_error(
            cluster=Clusters.Objects.AccessControl,
            attribute=extension_attribute,
            error=Status.UnsupportedAccess,
            endpoint=0
        )

        self.step(6)
        # TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list
        new_extension = []
        result3 = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attribute(value=new_extension))]
        )
        # Add explicit check for UnsupportedAccess error
        if result3[0].Status != Status.UnsupportedAccess:
            asserts.fail(f"Expected UnsupportedAccess but got {result3[0].Status}")

        self.step(7)
        # TH1 reads DUT Endpoint 0 AccessControl cluster SubjectsPerAccessControlEntry attribute
        await self.read_and_check_min_value(Clusters.AccessControl.Attributes.SubjectsPerAccessControlEntry, 4)

        self.step(8)
        # TH1 reads DUT Endpoint 0 AccessControl cluster TargetsPerAccessControlEntry attribute
        await self.read_and_check_min_value(Clusters.AccessControl.Attributes.TargetsPerAccessControlEntry, 3)

        self.step(9)
        # TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntriesPerFabric attribute
        await self.read_and_check_min_value(Clusters.AccessControl.Attributes.AccessControlEntriesPerFabric, 4)

        self.step(10)
        # TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event
        await self.read_event_expect_unsupported_access(Clusters.AccessControl.Events.AccessControlEntryChanged)

        self.step(11)
        # TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event
        await self.read_event_expect_unsupported_access(Clusters.AccessControl.Events.AccessControlExtensionChanged)

        # TODO: Add factory reset to reset ACL back to admin privileges after test runs


if __name__ == "__main__":
    default_matter_test_main()
