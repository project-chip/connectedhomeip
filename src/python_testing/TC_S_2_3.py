#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the \"License\");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an \"AS IS\" BASIS,
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
#   run2:
#     app: ${ALL_CLUSTERS_NO_GROUPCAST_APP}
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
#   run3:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234 --groupcast
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
from TC_GC_common import is_groupcast_on_root_node
from TC_S_common import build_extension_fields, is_writable, read_scenable_value, select_scenable_attribute, value_other_than

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Indeterminate scene-table capacity values that the spec allows a DUT to report
# instead of an exact remaining count (0xfe == "at least one", null == unknown).
_INDETERMINATE_CAPACITY = (0xFE, NullValue)

# Controller-side GroupInfo flags (matches the C++ GroupInfo flag bitmask).
_USE_IANA_ADDR = 0
_USE_PER_GROUP_ADDR = 2


class TC_S_2_3(MatterBaseTest):
    def desc_TC_S_2_3(self) -> str:
        return "[TC-S-2.3] Secondary functionality with DUT as Server"

    def pics_TC_S_2_3(self):
        return ["S"]

    def steps_TC_S_2_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(
                "0a",
                "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT with GroupKeySetID 0x01a1.",
                "DUT sends a SUCCESS response.",
            ),
            TestStep(
                "0b",
                "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT with GroupKeySetID 0x01a2.",
                "DUT sends a SUCCESS response.",
            ),
            TestStep(
                "0c",
                "If the Groupcast cluster is enabled on the RootNode endpoint, skip this step. Otherwise, TH writes the GroupKeyMap attribute on the GroupKeyManagement cluster binding G1 to GroupKeySetID 0x01a1 and G2 to GroupKeySetID 0x01a2.",
                "DUT sends a SUCCESS response.",
            ),
            TestStep(
                "1a",
                "If the Groupcast cluster is enabled on the RootNode endpoint, TH sends Groupcast JoinGroup command for G1 with KeySetID 0x01a1. Otherwise, TH sends a Groups AddGroup command with GroupID G1.",
                "DUT sends SUCCESS (JoinGroup) or AddGroupResponse with Status 0x00 (SUCCESS) and GroupID G1.",
            ),
            TestStep(
                "1b",
                "If the Groupcast cluster is enabled on the RootNode endpoint, TH sends Groupcast JoinGroup command for G2 with KeySetID 0x01a2. Otherwise, TH sends a Groups AddGroup command with GroupID G2.",
                "DUT sends SUCCESS (JoinGroup) or AddGroupResponse with Status 0x00 (SUCCESS) and GroupID G2.",
            ),
            TestStep(
                "1c",
                "TH installs access control on the device to allow group commands to take actions.",
                "DUT sends a WriteResponseMessage with Status 0x00 (SUCCESS).",
            ),
            TestStep(
                "1d",
                "TH sends a RemoveAllScenes command to DUT with GroupID G1.",
                "DUT sends a RemoveAllScenesResponse with Status 0x00 (SUCCESS) and GroupID G1.",
            ),
            TestStep(
                "1e",
                "TH sends a RemoveAllScenes command to DUT with GroupID G2.",
                "DUT sends a RemoveAllScenesResponse with Status 0x00 (SUCCESS) and GroupID G2.",
            ),
            TestStep(
                "1f",
                "TH sends a GetSceneMembership command to DUT with GroupID G1.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity recorded into SC0, GroupID G1 and SceneList containing 0 entry.",
            ),
            TestStep(
                2,
                "TH sends an AddScene command to DUT with GroupID G1, SceneID 0x01, TransitionTime 1000 and a set of extension fields producing AC1.",
                "DUT sends an AddSceneResponse with Status 0x00 (SUCCESS), GroupID G1 and SceneID 0x01.",
            ),
            TestStep(
                3,
                "TH sends a ViewScene command to DUT with GroupID G1 and SceneID 0x01.",
                "DUT sends a ViewSceneResponse with Status 0x00 (SUCCESS), GroupID G1, SceneID 0x01, TransitionTime 1000 and extension fields appropriate to AC1.",
            ),
            TestStep(
                4,
                "TH sends a GetSceneMembership command to DUT with GroupID G1.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity (SC0 - 1)/0xfe/null, GroupID G1 and SceneList containing only SceneID 0x01.",
            ),
            TestStep(
                "5a",
                "TH configures AC2 on DUT for all implemented application clusters supporting scenes.",
                "DUT is configured with AC2 which is different from AC1.",
            ),
            TestStep(
                "5b",
                "TH sends a RecallScene command to group G1 with GroupID G1 and SceneID 0x01.",
                "There is no status response since this was a groupcast. DUT returns to AC1.",
            ),
            TestStep(
                "6a",
                "TH sends an AddScene command to group G1 with GroupID G1, SceneID 0x03, TransitionTime 1000 and extension fields appropriate to AC1.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "6b",
                "TH sends a ViewScene command to DUT with GroupID G1 and SceneID 0x03.",
                "DUT sends a ViewSceneResponse with Status 0x00 (SUCCESS), GroupID G1, SceneID 0x03, TransitionTime 1 and extension fields appropriate to AC1.",
            ),
            TestStep(
                "6c",
                "TH sends a ViewScene command to group G1 with GroupID G1 and SceneID 0x03.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "6d",
                "TH sends a RemoveScene command to group G1 with GroupID G1 and SceneID 0x03.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "6e",
                "TH sends a ViewScene command to DUT with GroupID G1 and SceneID 0x03.",
                "DUT sends a ViewSceneResponse with Status 0x8b (NOT_FOUND), GroupID G1, SceneID 0x03, and TransitionTime/extension fields omitted.",
            ),
            TestStep(
                "6f",
                "TH sends a StoreScene command to group G1 with GroupID G1 and SceneID 0x03.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "6g",
                "TH sends a ViewScene command to DUT with GroupID G1 and SceneID 0x03.",
                "DUT sends a ViewSceneResponse with Status 0x00 (SUCCESS), GroupID G1 and SceneID 0x03.",
            ),
            TestStep(
                "6h",
                "TH sends a RemoveScene command to group G1 with GroupID G1 and SceneID 0x03.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "6i",
                "TH sends a ViewScene command to DUT with GroupID G1 and SceneID 0x03.",
                "DUT sends a ViewSceneResponse with Status 0x8b (NOT_FOUND), GroupID G1, SceneID 0x03, and TransitionTime/extension fields omitted.",
            ),
            TestStep(
                "7a",
                "TH sends a CopyScene command to DUT with mode 0x00, group from G1, scene from 0x01, group to G1 and scene to 0x02.",
                "DUT sends a CopySceneResponse with Status 0x00 (SUCCESS), group from G1 and scene from 0x01.",
            ),
            TestStep(
                "7b",
                "TH sends a CopyScene command to DUT with mode 0x00, group from GI (absent from group table), scene from 0x01, group to GI and scene to 0x02.",
                "DUT sends a CopySceneResponse with Status 0x85 (INVALID_COMMAND), group from GI and scene from 0x01.",
            ),
            TestStep(
                "7c",
                "TH sends a CopyScene command to DUT with mode 0x00, group from G1, scene from 0xfe, group to G1 and scene to 0x02.",
                "DUT sends a CopySceneResponse with Status 0x8b (NOT_FOUND), group from G1 and scene from 0xfe.",
            ),
            TestStep(
                "7d",
                "TH sends a GetSceneMembership command to DUT with GroupID G1.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity (SC0 - 2)/0xfe/null, GroupID G1 and SceneList containing SceneIDs 0x01 and 0x02.",
            ),
            TestStep(
                "8a",
                "TH sends a CopyScene command to DUT with mode 0x01, group from G1, scene from 0x01, group to G2 and scene to 0x02.",
                "DUT sends a CopySceneResponse with Status 0x00 (SUCCESS), group from G1 and scene from 0x01.",
            ),
            TestStep(
                "8b",
                "TH sends a GetSceneMembership command to DUT with GroupID G2.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity (SC0 - 4)/0xfe/null, GroupID G2 and SceneList containing SceneIDs 0x01 and 0x02.",
            ),
            TestStep(
                "8c",
                "TH sends a CopyScene command to group G2 with mode 0x00, group from G2, scene from 0x02, group to G2 and scene to 0x03.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "8d",
                "TH sends a GetSceneMembership command to DUT with GroupID G2.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity (SC0 - 5)/0xfe/null, GroupID G2 and SceneList containing SceneIDs 0x01, 0x02 and 0x03.",
            ),
            TestStep(
                "9a",
                "TH sends a RemoveAllScenes command to group G1 with GroupID G1.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "9b",
                "TH sends a GetSceneMembership command to DUT with GroupID G1.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity (SC0 - 3)/0xfe/null, GroupID G1 and SceneList containing 0 entry.",
            ),
            TestStep(
                "10a",
                "TH sends a RemoveAllScenes command to group G2 with GroupID G2.",
                "There is no status response since this was a groupcast.",
            ),
            TestStep(
                "10b",
                "TH sends a GetSceneMembership command to DUT with GroupID G2.",
                "DUT sends a GetSceneMembershipResponse with Status 0x00 (SUCCESS), Capacity SC0/0xfe/null, GroupID G2 and SceneList containing 0 entry.",
            ),
            TestStep(
                11,
                "TH sends a KeySetRemove command to the GroupKeyManagement cluster with GroupKeySetID 0x01a1.",
                "DUT sends a SUCCESS response.",
            ),
            TestStep(
                12,
                "TH sends a KeySetRemove command to the GroupKeyManagement cluster with GroupKeySetID 0x01a2.",
                "DUT sends a SUCCESS response.",
            ),
        ]

    def _assert_extension_fields_match(self, extension_field_sets, expected_value: int, context: str) -> None:
        """Asserts the returned extension fields carry the selected attribute at expected_value."""
        asserts.assert_true(
            extension_field_sets is not None and extension_field_sets is not NullValue,
            f"{context}: extension fields should be present for a stored scene",
        )
        cluster_sets = [efs for efs in extension_field_sets if efs.clusterID == self._scenable.cluster.id]
        asserts.assert_equal(len(cluster_sets), 1, f"{context}: expected exactly one extension field set for the scenable cluster")
        pairs = [p for p in cluster_sets[0].attributeValueList if p.attributeID == self._scenable.attribute.attribute_id]
        asserts.assert_equal(len(pairs), 1, f"{context}: expected the scenable attribute in the extension field set")
        asserts.assert_equal(
            getattr(pairs[0], self._scenable.value_field), expected_value, f"{context}: scenable attribute value should match AC1"
        )

    def _assert_capacity(self, capacity, sc0, delta: int, context: str) -> None:
        """Validates a GetSceneMembership capacity against the recorded baseline SC0.

        The spec allows the DUT to report an indeterminate value (0xfe or null). When a
        concrete value is reported and SC0 was also concrete, it must equal (SC0 - delta).
        """
        if capacity in _INDETERMINATE_CAPACITY or sc0 in _INDETERMINATE_CAPACITY:
            return
        asserts.assert_equal(capacity, sc0 - delta, f"{context}: capacity should be SC0 - {delta}")

    async def _get_scene_membership(self, group_id: int, context: str):
        """Sends a unicast GetSceneMembership command and asserts a successful response."""
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.GetSceneMembership(groupID=group_id), endpoint=self._scene_endpoint
        )
        asserts.assert_equal(resp.status, Status.Success, f"{context}: GetSceneMembership status")
        asserts.assert_equal(resp.groupID, group_id, f"{context}: GetSceneMembership groupID")
        return resp

    async def _view_scene(self, group_id: int, scene_id: int):
        """Sends a unicast ViewScene command and returns the response."""
        return await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.ViewScene(groupID=group_id, sceneID=scene_id), endpoint=self._scene_endpoint
        )

    @async_test_body
    async def test_TC_S_2_3(self):
        self._scene_endpoint = self.get_endpoint()
        dev_ctrl = self.default_controller
        th_node_id = self.matter_test_config.controller_node_id

        group_g1 = 0x0001
        group_g2 = 0x0002
        # GI is a group identifier that is intentionally never added to the group table.
        group_gi = 0x000A
        keyset1 = 0x01A1
        keyset2 = 0x01A2

        keyset1_key0 = bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf")
        keyset1_key1 = bytes.fromhex("b0b1b2b3b4b5b6b7b8b9babbbcbdbebf")
        keyset2_key0 = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf")
        keyset2_key1 = bytes.fromhex("e0e1e2e3e4e5e6e7e8e9eaebecedeeef")

        transition_time = 1000

        self.step(0)
        groupcast_enabled = await is_groupcast_on_root_node(self)

        # Discover a scenable attribute on the scene endpoint and derive two distinct
        # application configurations from it: AC1 (stored in scenes and restored via
        # RecallScene) and AC2 (a different device state used to prove RecallScene works).
        self._scenable = await select_scenable_attribute(self, self._scene_endpoint)
        current_value = await read_scenable_value(self, self._scenable, self._scene_endpoint)
        ac1_value = value_other_than(self._scenable, current_value)
        writable = is_writable(self._scenable)
        log.info(
            "Using scenable attribute %s::%s (%s, writable=%s): AC1=%s, current(AC2)=%s",
            self._scenable.cluster.__name__,
            self._scenable.attribute.__name__,
            self._scenable.xml_attribute.datatype,
            writable,
            ac1_value,
            current_value,
        )

        # Step 0a: KeySetWrite for keyset 0x01a1 and mirror it onto the controller's group store.
        self.step("0a")
        await self.send_single_cmd(
            endpoint=0,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
                groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                    groupKeySetID=keyset1,
                    groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                    epochKey0=keyset1_key0,
                    epochStartTime0=1110000,
                    epochKey1=keyset1_key1,
                    epochStartTime1=1110001,
                    epochKey2=NullValue,
                    epochStartTime2=NullValue,
                )
            ),
        )
        dev_ctrl.SetGroupKeySet(
            keyset_id=keyset1,
            policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            num_keys=2,
            epoch_key0=keyset1_key0,
            epoch_start_time0=1110000,
            epoch_key1=keyset1_key1,
            epoch_start_time1=1110001,
        )

        # Step 0b: KeySetWrite for keyset 0x01a2 and mirror it onto the controller's group store.
        self.step("0b")
        await self.send_single_cmd(
            endpoint=0,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
                groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                    groupKeySetID=keyset2,
                    groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                    epochKey0=keyset2_key0,
                    epochStartTime0=2220000,
                    epochKey1=keyset2_key1,
                    epochStartTime1=2220001,
                    epochKey2=NullValue,
                    epochStartTime2=NullValue,
                )
            ),
        )
        dev_ctrl.SetGroupKeySet(
            keyset_id=keyset2,
            policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            num_keys=2,
            epoch_key0=keyset2_key0,
            epoch_start_time0=2220000,
            epoch_key1=keyset2_key1,
            epoch_start_time1=2220001,
        )

        # Configure the controller's group store so it can later send group(cast) commands.
        # Legacy groups only support per-group multicast addressing; groupcast uses IANA here.
        group_info_policy = _USE_IANA_ADDR if groupcast_enabled else _USE_PER_GROUP_ADDR
        dev_ctrl.SetGroupKey(group_g1, keyset1)
        dev_ctrl.SetGroupKey(group_g2, keyset2)
        dev_ctrl.SetGroupInfo(group_g1, "G1", group_info_policy)
        dev_ctrl.SetGroupInfo(group_g2, "G2", group_info_policy)

        # Step 0c: bind GroupIds to KeySetIDs via GroupKeyMap (legacy path only).
        if groupcast_enabled:
            self.skip_step("0c")
        else:
            self.step("0c")
            result = await dev_ctrl.WriteAttribute(
                self.dut_node_id,
                [
                    (
                        0,
                        Clusters.GroupKeyManagement.Attributes.GroupKeyMap(
                            [
                                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=group_g1, groupKeySetID=keyset1),
                                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=group_g2, groupKeySetID=keyset2),
                            ]
                        ),
                    )
                ],
            )
            asserts.assert_equal(result[0].Status, Status.Success, "Step 0c: GroupKeyMap write failed")

        # Step 1a: add G1 (groupcast JoinGroup or legacy AddGroup).
        self.step("1a")
        if groupcast_enabled:
            await self.send_single_cmd(
                endpoint=0,
                cmd=Clusters.Groupcast.Commands.JoinGroup(groupID=group_g1, endpoints=[self._scene_endpoint], keySetID=keyset1),
            )
        else:
            resp = await self.send_single_cmd(
                Clusters.Groups.Commands.AddGroup(groupID=group_g1, groupName=""), endpoint=self._scene_endpoint
            )
            asserts.assert_equal(resp.status, Status.Success, "Step 1a: AddGroup G1 status")
            asserts.assert_equal(resp.groupID, group_g1, "Step 1a: AddGroup G1 groupID")

        # Step 1b: add G2 (groupcast JoinGroup or legacy AddGroup).
        self.step("1b")
        if groupcast_enabled:
            await self.send_single_cmd(
                endpoint=0,
                cmd=Clusters.Groupcast.Commands.JoinGroup(groupID=group_g2, endpoints=[self._scene_endpoint], keySetID=keyset2),
            )
        else:
            resp = await self.send_single_cmd(
                Clusters.Groups.Commands.AddGroup(groupID=group_g2, groupName=""), endpoint=self._scene_endpoint
            )
            asserts.assert_equal(resp.status, Status.Success, "Step 1b: AddGroup G2 status")
            asserts.assert_equal(resp.groupID, group_g2, "Step 1b: AddGroup G2 groupID")

        # Step 1c: install ACL granting group members Manage on the Scenes cluster, keeping CASE admin.
        self.step("1c")
        acl_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th_node_id],
            targets=NullValue,
        )
        acl_group = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
            subjects=[group_g1, group_g2],
            targets=[
                Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=self._scene_endpoint, cluster=Clusters.ScenesManagement.id
                )
            ],
        )
        result = await dev_ctrl.WriteAttribute(
            self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin, acl_group]))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "Step 1c: ACL write failed")

        # Step 1d: RemoveAllScenes for G1.
        self.step("1d")
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=group_g1), endpoint=self._scene_endpoint
        )
        asserts.assert_equal(resp.status, Status.Success, "Step 1d: RemoveAllScenes status")
        asserts.assert_equal(resp.groupID, group_g1, "Step 1d: RemoveAllScenes groupID")

        # Step 1e: RemoveAllScenes for G2.
        self.step("1e")
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=group_g2), endpoint=self._scene_endpoint
        )
        asserts.assert_equal(resp.status, Status.Success, "Step 1e: RemoveAllScenes status")
        asserts.assert_equal(resp.groupID, group_g2, "Step 1e: RemoveAllScenes groupID")

        # Step 1f: GetSceneMembership for G1, record baseline capacity SC0.
        self.step("1f")
        resp = await self._get_scene_membership(group_g1, "Step 1f")
        sc0 = resp.capacity
        asserts.assert_equal(list(resp.sceneList or []), [], "Step 1f: SceneList should be empty")

        # Step 2: AddScene G1/0x01 producing AC1.
        self.step(2)
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.AddScene(
                groupID=group_g1,
                sceneID=0x01,
                transitionTime=transition_time,
                sceneName="",
                extensionFieldSetStructs=build_extension_fields(self._scenable, ac1_value),
            ),
            endpoint=self._scene_endpoint,
        )
        asserts.assert_equal(resp.status, Status.Success, "Step 2: AddScene status")
        asserts.assert_equal(resp.groupID, group_g1, "Step 2: AddScene groupID")
        asserts.assert_equal(resp.sceneID, 0x01, "Step 2: AddScene sceneID")

        # Step 3: ViewScene G1/0x01 and confirm AC1 was stored.
        self.step(3)
        resp = await self._view_scene(group_g1, 0x01)
        asserts.assert_equal(resp.status, Status.Success, "Step 3: ViewScene status")
        asserts.assert_equal(resp.groupID, group_g1, "Step 3: ViewScene groupID")
        asserts.assert_equal(resp.sceneID, 0x01, "Step 3: ViewScene sceneID")
        asserts.assert_equal(resp.transitionTime, transition_time, "Step 3: ViewScene transitionTime")
        self._assert_extension_fields_match(resp.extensionFieldSetStructs, ac1_value, "Step 3")

        # Step 4: GetSceneMembership G1 contains only scene 0x01.
        self.step(4)
        resp = await self._get_scene_membership(group_g1, "Step 4")
        self._assert_capacity(resp.capacity, sc0, 1, "Step 4")
        asserts.assert_equal(sorted(resp.sceneList or []), [0x01], "Step 4: SceneList should contain only 0x01")

        # Step 5a: configure AC2, a device state different from AC1. When the scenable
        # attribute is writable, AC2 is actively written; otherwise the DUT's current
        # state is used as AC2 (AC1 was chosen to differ from it). Either way the device
        # must not already be in AC1 before the recall.
        self.step("5a")
        if writable:
            ac2_value = value_other_than(self._scenable, ac1_value)
            result = await dev_ctrl.WriteAttribute(self.dut_node_id, [(self._scene_endpoint, self._scenable.attribute(ac2_value))])
            asserts.assert_equal(result[0].Status, Status.Success, "Step 5a: writing AC2 failed")
            await asyncio.sleep(1)
        else:
            log.info("Step 5a: scenable attribute is read-only; using the DUT's current state as AC2.")
        ac2_reading = await read_scenable_value(self, self._scenable, self._scene_endpoint)
        asserts.assert_not_equal(ac2_reading, ac1_value, "Step 5a: DUT should be in AC2 (a state different from AC1) before recall")

        # Step 5b: groupcast RecallScene G1/0x01 returns the device to AC1.
        self.step("5b")
        dev_ctrl.SendGroupCommand(group_g1, Clusters.ScenesManagement.Commands.RecallScene(groupID=group_g1, sceneID=0x01))
        await asyncio.sleep(3)
        recalled = await read_scenable_value(self, self._scenable, self._scene_endpoint)
        asserts.assert_equal(recalled, ac1_value, "Step 5b: DUT should return to AC1 after RecallScene")

        # Step 6a: groupcast AddScene G1/0x03 with AC1.
        self.step("6a")
        dev_ctrl.SendGroupCommand(
            group_g1,
            Clusters.ScenesManagement.Commands.AddScene(
                groupID=group_g1,
                sceneID=0x03,
                transitionTime=transition_time,
                sceneName="",
                extensionFieldSetStructs=build_extension_fields(self._scenable, ac1_value),
            ),
        )
        await asyncio.sleep(3)

        # Step 6b: ViewScene G1/0x03 confirms the groupcast AddScene took effect.
        self.step("6b")
        resp = await self._view_scene(group_g1, 0x03)
        asserts.assert_equal(resp.status, Status.Success, "Step 6b: ViewScene status")
        asserts.assert_equal(resp.groupID, group_g1, "Step 6b: ViewScene groupID")
        asserts.assert_equal(resp.sceneID, 0x03, "Step 6b: ViewScene sceneID")
        asserts.assert_equal(resp.transitionTime, transition_time, "Step 6b: ViewScene transitionTime")
        self._assert_extension_fields_match(resp.extensionFieldSetStructs, ac1_value, "Step 6b")

        # Step 6c: groupcast ViewScene (no response expected).
        self.step("6c")
        dev_ctrl.SendGroupCommand(group_g1, Clusters.ScenesManagement.Commands.ViewScene(groupID=group_g1, sceneID=0x03))
        await asyncio.sleep(3)

        # Step 6d: groupcast RemoveScene G1/0x03 (no response expected).
        self.step("6d")
        dev_ctrl.SendGroupCommand(group_g1, Clusters.ScenesManagement.Commands.RemoveScene(groupID=group_g1, sceneID=0x03))
        await asyncio.sleep(3)

        # Step 6e: ViewScene G1/0x03 now NOT_FOUND after the groupcast remove.
        self.step("6e")
        resp = await self._view_scene(group_g1, 0x03)
        asserts.assert_equal(resp.status, Status.NotFound, "Step 6e: ViewScene should be NOT_FOUND")
        asserts.assert_equal(resp.groupID, group_g1, "Step 6e: ViewScene groupID")
        asserts.assert_equal(resp.sceneID, 0x03, "Step 6e: ViewScene sceneID")
        asserts.assert_true(resp.transitionTime is None, "Step 6e: transitionTime should be omitted")
        asserts.assert_true(resp.extensionFieldSetStructs is None, "Step 6e: extension fields should be omitted")

        # Step 6f: groupcast StoreScene G1/0x03 (no response expected).
        self.step("6f")
        dev_ctrl.SendGroupCommand(group_g1, Clusters.ScenesManagement.Commands.StoreScene(groupID=group_g1, sceneID=0x03))
        await asyncio.sleep(3)

        # Step 6g: ViewScene G1/0x03 now SUCCESS after the groupcast store.
        self.step("6g")
        resp = await self._view_scene(group_g1, 0x03)
        asserts.assert_equal(resp.status, Status.Success, "Step 6g: ViewScene should be SUCCESS")
        asserts.assert_equal(resp.groupID, group_g1, "Step 6g: ViewScene groupID")
        asserts.assert_equal(resp.sceneID, 0x03, "Step 6g: ViewScene sceneID")

        # Step 6h: groupcast RemoveScene G1/0x03 (no response expected).
        self.step("6h")
        dev_ctrl.SendGroupCommand(group_g1, Clusters.ScenesManagement.Commands.RemoveScene(groupID=group_g1, sceneID=0x03))
        await asyncio.sleep(3)

        # Step 6i: ViewScene G1/0x03 NOT_FOUND again.
        self.step("6i")
        resp = await self._view_scene(group_g1, 0x03)
        asserts.assert_equal(resp.status, Status.NotFound, "Step 6i: ViewScene should be NOT_FOUND")
        asserts.assert_equal(resp.groupID, group_g1, "Step 6i: ViewScene groupID")
        asserts.assert_equal(resp.sceneID, 0x03, "Step 6i: ViewScene sceneID")
        asserts.assert_true(resp.transitionTime is None, "Step 6i: transitionTime should be omitted")
        asserts.assert_true(resp.extensionFieldSetStructs is None, "Step 6i: extension fields should be omitted")

        # Step 7a: CopyScene G1/0x01 -> G1/0x02 (mode 0x00).
        self.step("7a")
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.CopyScene(
                mode=0x00,
                groupIdentifierFrom=group_g1,
                sceneIdentifierFrom=0x01,
                groupIdentifierTo=group_g1,
                sceneIdentifierTo=0x02,
            ),
            endpoint=self._scene_endpoint,
        )
        asserts.assert_equal(resp.status, Status.Success, "Step 7a: CopyScene status")
        asserts.assert_equal(resp.groupIdentifierFrom, group_g1, "Step 7a: CopyScene groupIdentifierFrom")
        asserts.assert_equal(resp.sceneIdentifierFrom, 0x01, "Step 7a: CopyScene sceneIdentifierFrom")

        # Step 7b: CopyScene from an absent group GI -> INVALID_COMMAND.
        self.step("7b")
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.CopyScene(
                mode=0x00,
                groupIdentifierFrom=group_gi,
                sceneIdentifierFrom=0x01,
                groupIdentifierTo=group_gi,
                sceneIdentifierTo=0x02,
            ),
            endpoint=self._scene_endpoint,
        )
        asserts.assert_equal(resp.status, Status.InvalidCommand, "Step 7b: CopyScene should be INVALID_COMMAND")
        asserts.assert_equal(resp.groupIdentifierFrom, group_gi, "Step 7b: CopyScene groupIdentifierFrom")
        asserts.assert_equal(resp.sceneIdentifierFrom, 0x01, "Step 7b: CopyScene sceneIdentifierFrom")

        # Step 7c: CopyScene from a non-existent scene 0xfe -> NOT_FOUND.
        self.step("7c")
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.CopyScene(
                mode=0x00,
                groupIdentifierFrom=group_g1,
                sceneIdentifierFrom=0xFE,
                groupIdentifierTo=group_g1,
                sceneIdentifierTo=0x02,
            ),
            endpoint=self._scene_endpoint,
        )
        asserts.assert_equal(resp.status, Status.NotFound, "Step 7c: CopyScene should be NOT_FOUND")
        asserts.assert_equal(resp.groupIdentifierFrom, group_g1, "Step 7c: CopyScene groupIdentifierFrom")
        asserts.assert_equal(resp.sceneIdentifierFrom, 0xFE, "Step 7c: CopyScene sceneIdentifierFrom")

        # Step 7d: GetSceneMembership G1 contains scenes 0x01 and 0x02.
        self.step("7d")
        resp = await self._get_scene_membership(group_g1, "Step 7d")
        self._assert_capacity(resp.capacity, sc0, 2, "Step 7d")
        asserts.assert_equal(sorted(resp.sceneList or []), [0x01, 0x02], "Step 7d: SceneList should contain 0x01 and 0x02")

        # Step 8a: CopyScene G1/0x01 -> G2/0x02 (mode 0x01, copy all scenes).
        self.step("8a")
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.CopyScene(
                mode=0x01,
                groupIdentifierFrom=group_g1,
                sceneIdentifierFrom=0x01,
                groupIdentifierTo=group_g2,
                sceneIdentifierTo=0x02,
            ),
            endpoint=self._scene_endpoint,
        )
        asserts.assert_equal(resp.status, Status.Success, "Step 8a: CopyScene status")
        asserts.assert_equal(resp.groupIdentifierFrom, group_g1, "Step 8a: CopyScene groupIdentifierFrom")
        asserts.assert_equal(resp.sceneIdentifierFrom, 0x01, "Step 8a: CopyScene sceneIdentifierFrom")

        # Step 8b: GetSceneMembership G2 contains scenes 0x01 and 0x02.
        self.step("8b")
        resp = await self._get_scene_membership(group_g2, "Step 8b")
        self._assert_capacity(resp.capacity, sc0, 4, "Step 8b")
        asserts.assert_equal(sorted(resp.sceneList or []), [0x01, 0x02], "Step 8b: SceneList should contain 0x01 and 0x02")

        # Step 8c: groupcast CopyScene G2/0x02 -> G2/0x03 (no response expected).
        self.step("8c")
        dev_ctrl.SendGroupCommand(
            group_g2,
            Clusters.ScenesManagement.Commands.CopyScene(
                mode=0x00,
                groupIdentifierFrom=group_g2,
                sceneIdentifierFrom=0x02,
                groupIdentifierTo=group_g2,
                sceneIdentifierTo=0x03,
            ),
        )
        await asyncio.sleep(3)

        # Step 8d: GetSceneMembership G2 contains scenes 0x01, 0x02 and 0x03.
        self.step("8d")
        resp = await self._get_scene_membership(group_g2, "Step 8d")
        self._assert_capacity(resp.capacity, sc0, 5, "Step 8d")
        asserts.assert_equal(
            sorted(resp.sceneList or []), [0x01, 0x02, 0x03], "Step 8d: SceneList should contain 0x01, 0x02 and 0x03"
        )

        # Step 9a: groupcast RemoveAllScenes G1 (no response expected).
        self.step("9a")
        dev_ctrl.SendGroupCommand(group_g1, Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=group_g1))
        await asyncio.sleep(3)

        # Step 9b: GetSceneMembership G1 is empty.
        self.step("9b")
        resp = await self._get_scene_membership(group_g1, "Step 9b")
        self._assert_capacity(resp.capacity, sc0, 3, "Step 9b")
        asserts.assert_equal(list(resp.sceneList or []), [], "Step 9b: SceneList should be empty")

        # Step 10a: groupcast RemoveAllScenes G2 (no response expected).
        self.step("10a")
        dev_ctrl.SendGroupCommand(group_g2, Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=group_g2))
        await asyncio.sleep(3)

        # Step 10b: GetSceneMembership G2 is empty.
        self.step("10b")
        resp = await self._get_scene_membership(group_g2, "Step 10b")
        self._assert_capacity(resp.capacity, sc0, 0, "Step 10b")
        asserts.assert_equal(list(resp.sceneList or []), [], "Step 10b: SceneList should be empty")

        # Step 11: remove keyset 0x01a1.
        self.step(11)
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keyset1))

        # Step 12: remove keyset 0x01a2.
        self.step(12)
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keyset2))


if __name__ == "__main__":
    default_matter_test_main()
