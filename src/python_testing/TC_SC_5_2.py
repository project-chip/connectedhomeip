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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts
from TC_GC_common import get_feature_map, is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SC_5_2(MatterBaseTest):

    def desc_TC_SC_5_2(self) -> str:
        return "26.1.2. [TC-SC-5.2] Receiving a group message - TH to DUT"

    def pics_TC_SC_5_2(self):
        return ["MCORE.ROLE.COMMISSIONEE"]

    def steps_TC_SC_5_2(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("1", "TH writes the ACL attribute in the Access Control cluster to add Manage privileges for group 0x0103."),
            TestStep("2", "TH sends KeySetWrite command with pre-installed key."),
            TestStep("3", "If Groupcast enabled on RootNode, skip to step 12. Otherwise, TH binds GroupId 0x0103 and 0x0101 with GroupKeySetID 0x01a3 in GroupKeyMap."),
            TestStep("4", "TH sends RemoveAllGroups command to DUT on PIXIT.G.ENDPOINT."),
            TestStep("5", "TH sends AddGroup Command with GroupID 0x0103 to DUT on PIXIT.G.ENDPOINT."),
            TestStep("6", "TH sends AddGroup command for GroupID 0x0101 as a group command using GroupID 0x0103."),
            TestStep("7", "TH sends ViewGroup with GroupID 0x0101 (GroupNames supported)."),
            TestStep("8", "TH sends ViewGroup with GroupID 0x0101 (GroupNames not supported)."),
            TestStep("9", "TH sends RemoveGroup with GroupID 0x0101."),
            TestStep("10", "TH sends ViewGroup with GroupID 0x0101 to confirm removal."),
            TestStep("11", "TH sends RemoveAllGroups to clean up legacy groups."),
            TestStep("12", "If Groupcast NOT enabled or Listener disabled, skip to step 17. TH sends LeaveGroup(groupID=0)."),
            TestStep("13", "TH sends JoinGroup command with GroupID 0x0103."),
            TestStep("14", "TH reads Membership attribute from Groupcast cluster."),
            TestStep("15", "TH sends a group command using GroupID 0x0103 to modify an attribute on DUT."),
            TestStep("16", "TH validates group message was received by reading the modified attribute."),
            TestStep("17", "TH sends KeySetRemove with GroupKeySetID 0x01a3."),
            TestStep("18", "TH writes ACL to restore default access."),
        ]

    @async_test_body
    async def test_TC_SC_5_2(self):
        dev_ctrl = self.default_controller
        dev_ctrl.InitGroupTestingData()
        groups_endpoint = self.matter_test_config.endpoint
        node_id = self.dut_node_id
        groupcast_enabled = await is_groupcast_on_root_node(self)

        self.step("0")

        # Step 1: Write ACL
        self.step("1")
        acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[dev_ctrl.nodeId],
                targets=NullValue),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[0x0103],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

        # Step 2: KeySetWrite
        self.step("2")
        key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=0x01a3,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=2220000,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=2220001,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=2220002)
        await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(key_set))

        if groupcast_enabled:
            self.mark_step_range_skipped("3", "11")
        else:
            # Step 3: GroupKeyMap binding
            self.step("3")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=0x0103, groupKeySetID=0x01a3, fabricIndex=1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=0x0101, groupKeySetID=0x01a3, fabricIndex=1),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 4: RemoveAllGroups
            self.step("4")
            await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.RemoveAllGroups())

            # Step 5: AddGroup 0x0103
            self.step("5")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.AddGroup(0x0103, "Test Group 0103"))
            asserts.assert_equal(result.status, Status.Success, "AddGroup 0x0103 failed")

            # Step 6: AddGroup 0x0101 as group command via GroupID 0x0103
            self.step("6")
            await dev_ctrl.SendGroupCommand(0x0103, Clusters.Groups.Commands.AddGroup(0x0101, "Test Group 0101"))
            await asyncio.sleep(3)

            # Check if GroupNames are supported
            group_feature_map = await dev_ctrl.read_single_attribute_check_success(
                cluster=Clusters.Groups,
                attribute=Clusters.Groups.Attributes.FeatureMap,
                endpoint=0)
            group_names_supported = bool(group_feature_map & Clusters.Groups.Bitmaps.Feature.kGroupNames)

            # Step 7: ViewGroup 0x0101 with GroupNames
            if group_names_supported:
                self.step("7")
                result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0101))
                asserts.assert_equal(result.status, Status.Success, "ViewGroup failed")
                asserts.assert_equal(result.groupID, 0x0101, "ViewGroup groupID mismatch")
                asserts.assert_equal(result.groupName, "Test Group 0101", "ViewGroup groupName mismatch")
                self.skip_step("8")
            # Step 8: ViewGroup 0x0101 without GroupNames
            else:
                self.skip_step("7")
                self.step("8")
                result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0101))
                asserts.assert_equal(result.status, Status.Success, "ViewGroup failed")
                asserts.assert_equal(result.groupID, 0x0101, "ViewGroup groupID mismatch")
                asserts.assert_equal(result.groupName, "", "ViewGroup groupName mismatch")

            # Step 9: RemoveGroup 0x0101
            self.step("9")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.RemoveGroup(0x0101))
            asserts.assert_equal(result.status, Status.Success, "RemoveGroup failed")

            # Step 10: ViewGroup 0x0101 confirm removed
            self.step("10")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0101))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND after removal")

            # Step 11: RemoveAllGroups cleanup
            self.step("11")
            await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        is_groupcast_listener = False
        if groupcast_enabled:
            ln_enabled, _, _ = await get_feature_map(self)
            is_groupcast_listener = ln_enabled

        if not is_groupcast_listener:
            self.mark_step_range_skipped("12", "16")
        else:
            # update the controller GroupInfo for groupID 0x0103 to use IANA address policy
            dev_ctrl.SetGroupInfo(0x0103, "Group #3", 0)
            # Step 12: LeaveGroup
            self.step("12")
            # Check if there are any groups on the DUT.
            membership = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
            if membership:
                # LeaveGroup with groupID 0 will leave all groups on the fabric.
                await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

            # Step 13: JoinGroup
            self.step("13")
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=0x0103, endpoints=[groups_endpoint], keySetID=0x01a3))

            # Step 14: Read Membership
            self.step("14")
            membership = await self.read_single_attribute_check_success(
                cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership, endpoint=0)
            group_ids = [entry.groupID for entry in membership]
            asserts.assert_in(0x0103, group_ids, "GroupID 0x0103 not found in Membership")

            # Step 15: Send group command via GroupID 0x0103
            self.step("15")
            dev_ctrl.SendGroupCommand(0x0103, Clusters.OnOff.Commands.On())
            await asyncio.sleep(3)
            # Step 16: Validate group command received
            self.step("16")
            on_off = await self.read_single_attribute_check_success(endpoint=groups_endpoint,
                                                                    cluster=Clusters.OnOff, attribute=Clusters.OnOff.Attributes.OnOff)

            asserts.assert_true(on_off, "OnOff should be TRUE after group On command")
            await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.OnOff.Commands.Off())

            # restore the GroupInfo for groupID 0x0103 to use Per-Group address policy
            dev_ctrl.SetGroupInfo(0x0103, "Group #3")

        # Step 17: KeySetRemove
        self.step("17")
        await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(0x01a3))

        # Step 18: Restore ACL
        self.step("18")
        acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[dev_ctrl.nodeId],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])


if __name__ == "__main__":
    default_matter_test_main()
