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


import logging
import typing

from mobly import asserts
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, pics
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# Smallest SceneTableSize the spec permits.
_MIN_SCENE_TABLE_SIZE = 16


class TC_S_2_5(MatterTestCommissionedDevice):
    # Established in Step 4b and kept alive for the remainder of the test.
    _scene_info_subscription: typing.Any
    _scene_info_cb: AttributeSubscriptionHandler
    _scene_endpoint: int
    _fabric_index: int

    def teardown_test(self) -> None:
        sub = getattr(self, "_scene_info_subscription", None)
        if sub is not None:
            sub.Shutdown()
            del self._scene_info_subscription

    def _assert_remaining_capacity(self, fabric_scene_info, expected: int, context: str) -> None:
        """Assert RemainingCapacity for the TH fabric entry in a FabricSceneInfo list."""
        for entry in fabric_scene_info or []:
            if entry.fabricIndex == self._fabric_index:
                asserts.assert_equal(
                    entry.remainingCapacity,
                    expected,
                    f"{context}: RemainingCapacity for fabric {self._fabric_index}",
                )
                return
        asserts.fail(f"{context}: no FabricSceneInfo entry for fabric {self._fabric_index}")

    def _await_remaining_capacity(self, expected: int, context: str) -> None:
        """Wait for the next FabricSceneInfo report and assert RemainingCapacity."""
        report = self._scene_info_cb.wait_for_attribute_report(timeout_sec=10)
        self._assert_remaining_capacity(report.value, expected, context)

    @pics("S.S")
    @async_test_body
    async def test_TC_S_2_5(self) -> None:
        """[TC-S-2.5] RemainingCapacity functionality with DUT as Server"""
        self._scene_endpoint = self.get_endpoint()
        dev_ctrl = self.default_controller

        group_g1 = 0x0001
        keyset1 = 0x01A1
        keyset1_key0 = bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf")
        keyset1_key1 = bytes.fromhex("b0b1b2b3b4b5b6b7b8b9babbbcbdbebf")

        self.step(0, "Commissioning, already done")
        groupcast_enabled = await is_groupcast_on_root_node(self)
        # FabricSceneInfo is a fabric-scoped list; reports are matched on the TH's own entry.
        self._fabric_index = typing.cast(
            int,
            await self.read_single_attribute_check_success(
                cluster=Clusters.OperationalCredentials,
                attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
                endpoint=0,
            ),
        )

        # Step 0a: KeySetWrite for keyset 0x01a1.
        self.step("0a",
                  description="TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT "
                  "with GroupKeySetID 0x01a1.",
                  expectation="DUT sends a SUCCESS response.")

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
        keyset1_installed = True

        try:
            if groupcast_enabled:
                self.skip_step("0b")
            else:
                self.step("0b",
                          description="If the Groupcast cluster is enabled on the RootNode endpoint, skip this step."
                          "Otherwise, TH writes the GroupKeyMap attribute on the GroupKeyManagement "
                          "cluster binding G1 to GroupKeySetID 0x01a1.",
                          expectation="DUT sends a SUCCESS response.")
                result = await dev_ctrl.WriteAttribute(
                    self.dut_node_id,
                    [
                        (
                            0,
                            Clusters.GroupKeyManagement.Attributes.GroupKeyMap(
                                [Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                                    groupId=group_g1, groupKeySetID=keyset1)]
                            ),
                        )
                    ],
                )
                asserts.assert_equal(result[0].Status, Status.Success, "Step 0b: GroupKeyMap write failed")

            membership = []
            if groupcast_enabled:
                self.step("1a",
                          description="If the Groupcast cluster is enabled on the RootNode endpoint, TH reads the "
                          "Groupcast Membership attribute on the DUT. Otherwise, skip this step.",
                          expectation="DUT sends the Membership attribute; its contents decide whether Step 1b "
                          "needs to leave any group.")
                membership = await self.read_single_attribute_check_success(
                    cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership, endpoint=0
                )
            else:
                self.skip_step("1a")

            self.step("1b",
                      description="If the Groupcast cluster is enabled on the RootNode endpoint and Membership "
                      "was not empty in Step 1a, TH sends the Groupcast LeaveGroup command with "
                      "GroupID 0. Otherwise, TH sends a Groups RemoveAllGroups command.",
                      expectation="DUT sends a LeaveGroupResponse (groupcast) or a SUCCESS response "
                      "(legacy Groups).")
            if groupcast_enabled:
                if membership:
                    # GroupID 0 leaves every group of this fabric.
                    await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
                else:
                    log.info("Step 1b: Groupcast Membership is already empty, nothing to leave.")
            else:
                await self.send_single_cmd(Clusters.Groups.Commands.RemoveAllGroups(), endpoint=self._scene_endpoint)

            self.step(2,
                      description="If the Groupcast cluster is enabled on the RootNode endpoint, TH sends a "
                      "Groupcast JoinGroup command with GroupID G1, the scene endpoint and KeySetID 0x01a1. "
                      "Otherwise, TH sends a Groups AddGroup command with GroupID G1.",
                      expectation="DUT sends SUCCESS (JoinGroup) or an AddGroupResponse with Status 0x00 "
                      "(SUCCESS) and GroupID G1.")
            if groupcast_enabled:
                await self.send_single_cmd(
                    endpoint=0,
                    cmd=Clusters.Groupcast.Commands.JoinGroup(
                        groupID=group_g1, endpoints=[self._scene_endpoint], keySetID=keyset1),
                )
            else:
                resp = await self.send_single_cmd(
                    Clusters.Groups.Commands.AddGroup(groupID=group_g1, groupName=""), endpoint=self._scene_endpoint
                )
                asserts.assert_equal(resp.status, Status.Success, "Step 2: AddGroup G1 status")
                asserts.assert_equal(resp.groupID, group_g1, "Step 2: AddGroup G1 groupID")

            self.step(3,
                      description="TH sends a RemoveAllScenes command to DUT with GroupID G1.",
                      expectation="DUT sends a RemoveAllScenesResponse with Status 0x00 (SUCCESS) and GroupID G1.")
            resp = await self.send_single_cmd(
                Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=group_g1), endpoint=self._scene_endpoint
            )
            asserts.assert_equal(resp.status, Status.Success, "Step 3: RemoveAllScenes status")
            asserts.assert_equal(resp.groupID, group_g1, "Step 3: RemoveAllScenes groupID")

            self.step("4a",
                      description="TH reads the SceneTableSize attribute from the DUT.",
                      expectation="DUT sends SceneTableSize, which is at least 16. MaxRemainingCapacity is "
                      "(SceneTableSize - 1) / 2.")
            scene_table_size = typing.cast(
                int,
                await self.read_single_attribute_check_success(
                    endpoint=self._scene_endpoint,
                    cluster=Clusters.ScenesManagement,
                    attribute=Clusters.ScenesManagement.Attributes.SceneTableSize,
                ),
            )
            asserts.assert_greater_equal(
                scene_table_size, _MIN_SCENE_TABLE_SIZE, f"Step 4a: SceneTableSize must be at least {_MIN_SCENE_TABLE_SIZE}"
            )
            max_remaining_capacity = (scene_table_size - 1) // 2
            log.info("SceneTableSize is %s, MaxRemainingCapacity is %s", scene_table_size, max_remaining_capacity)

            self.step("4b",
                      description="TH sends a subscription request action for FabricSceneInfo to the DUT with "
                      "MinIntervalFloor 1 and MaxIntervalCeiling 5.",
                      expectation="The subscription is activated and the DUT reports FabricSceneInfo with "
                      "RemainingCapacity equal to MaxRemainingCapacity for this fabric's entry.")
            self._scene_info_subscription = await dev_ctrl.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[(self._scene_endpoint, Clusters.ScenesManagement.Attributes.FabricSceneInfo)],
                reportInterval=(1, 5),
                fabricFiltered=False,
                keepSubscriptions=True,
                autoResubscribe=False,
            )
            self._scene_info_cb = AttributeSubscriptionHandler(
                expected_cluster=Clusters.ScenesManagement,
                expected_attribute=Clusters.ScenesManagement.Attributes.FabricSceneInfo,
            )
            self._scene_info_subscription.SetAttributeUpdateCallback(self._scene_info_cb)

            # Priming report is already in the subscription cache; the callback only fires on later updates.
            priming = self._scene_info_subscription.GetAttributes()
            fabric_scene_info = priming[self._scene_endpoint][Clusters.ScenesManagement][
                Clusters.ScenesManagement.Attributes.FabricSceneInfo
            ]
            self._assert_remaining_capacity(fabric_scene_info, max_remaining_capacity, "Step 4b")

            self.step("5a",
                      description="TH sends an AddScene command to DUT with GroupID G1, SceneID 0x01, "
                      "TransitionTime 20000 and no extension field sets.",
                      expectation="DUT sends an AddSceneResponse with Status 0x00 (SUCCESS), GroupID G1 and "
                      "SceneID 0x01.")
            self._scene_info_cb.reset()
            resp = await self.send_single_cmd(
                Clusters.ScenesManagement.Commands.AddScene(
                    groupID=group_g1, sceneID=0x01, transitionTime=20000, sceneName="", extensionFieldSetStructs=[]
                ),
                endpoint=self._scene_endpoint,
            )
            asserts.assert_equal(resp.status, Status.Success, "Step 5a: AddScene status")
            asserts.assert_equal(resp.groupID, group_g1, "Step 5a: AddScene groupID")
            asserts.assert_equal(resp.sceneID, 0x01, "Step 5a: AddScene sceneID")
            scene_count = 1

            self.step("5b",
                      description="TH waits for a FabricSceneInfo report and records RemainingCapacity.",
                      expectation="RemainingCapacity equals (MaxRemainingCapacity - 1).")
            self._await_remaining_capacity(max_remaining_capacity - scene_count, "Step 5b")

            if max_remaining_capacity - scene_count > 0:
                self.step("6a",
                          description="If RemainingCapacity is greater than 0, TH sends a StoreScene command to DUT "
                          "with GroupID G1 and SceneID 0x02. Otherwise, skip to Step 8a.",
                          expectation="DUT sends a StoreSceneResponse with Status 0x00 (SUCCESS), GroupID G1 and "
                          "SceneID 0x02.")
                # clear the subscription cache
                self._scene_info_cb.reset()
                resp = await self.send_single_cmd(
                    Clusters.ScenesManagement.Commands.StoreScene(groupID=group_g1, sceneID=0x02),
                    endpoint=self._scene_endpoint,
                )
                asserts.assert_equal(resp.status, Status.Success, "Step 6a: StoreScene status")
                asserts.assert_equal(resp.groupID, group_g1, "Step 6a: StoreScene groupID")
                asserts.assert_equal(resp.sceneID, 0x02, "Step 6a: StoreScene sceneID")
                scene_count += 1

                self.step("6b",
                          description="TH waits for a FabricSceneInfo report and records RemainingCapacity.",
                          expectation="RemainingCapacity equals (MaxRemainingCapacity - 2).")
                self._await_remaining_capacity(max_remaining_capacity - scene_count, "Step 6b")
            else:
                self.skip_step("6a")
                self.skip_step("6b")

            if max_remaining_capacity - scene_count > 0:
                self.step("7a",
                          description="If RemainingCapacity is greater than 0, TH sends an AddScene command to DUT "
                          "with GroupID G1, SceneID 0x03, TransitionTime 20000 and no extension field "
                          "sets. Otherwise, skip to Step 8a.",
                          expectation="DUT sends an AddSceneResponse with Status 0x00 (SUCCESS), GroupID G1 and "
                          "SceneID 0x03.")
                # clear the subscription cache
                self._scene_info_cb.reset()
                resp = await self.send_single_cmd(
                    Clusters.ScenesManagement.Commands.AddScene(
                        groupID=group_g1, sceneID=0x03, transitionTime=20000, sceneName="", extensionFieldSetStructs=[]
                    ),
                    endpoint=self._scene_endpoint,
                )
                asserts.assert_equal(resp.status, Status.Success, "Step 7a: AddScene status")
                asserts.assert_equal(resp.groupID, group_g1, "Step 7a: AddScene groupID")
                asserts.assert_equal(resp.sceneID, 0x03, "Step 7a: AddScene sceneID")
                scene_count += 1

                self.step("7b",
                          description="TH waits for a FabricSceneInfo report and records RemainingCapacity.",
                          expectation="RemainingCapacity equals (MaxRemainingCapacity - 3).")
                self._await_remaining_capacity(max_remaining_capacity - scene_count, "Step 7b")
            else:
                self.skip_step("7a")
                self.skip_step("7b")

            self.step("8a",
                      description="TH sends a RemoveScene command to DUT with GroupID G1 and SceneID 0x01.",
                      expectation="DUT sends a RemoveSceneResponse with Status 0x00 (SUCCESS), GroupID G1 and "
                      "SceneID 0x01.")
            # clear the subscription cache
            self._scene_info_cb.reset()
            resp = await self.send_single_cmd(
                Clusters.ScenesManagement.Commands.RemoveScene(groupID=group_g1, sceneID=0x01),
                endpoint=self._scene_endpoint,
            )
            asserts.assert_equal(resp.status, Status.Success, "Step 8a: RemoveScene status")
            asserts.assert_equal(resp.groupID, group_g1, "Step 8a: RemoveScene groupID")
            asserts.assert_equal(resp.sceneID, 0x01, "Step 8a: RemoveScene sceneID")
            scene_count -= 1

            self.step("8b",
                      description="TH waits for a FabricSceneInfo report and records RemainingCapacity.",
                      expectation="RemainingCapacity equals (MaxRemainingCapacity - 2).")
            self._await_remaining_capacity(max_remaining_capacity - scene_count, "Step 8b")

            self.step("9a",
                      description="TH sends a RemoveAllScenes command to DUT with GroupID G1.",
                      expectation="DUT sends a RemoveAllScenesResponse with Status 0x00 (SUCCESS) and GroupID G1.")
            # clear the subscription cache
            self._scene_info_cb.reset()
            resp = await self.send_single_cmd(
                Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=group_g1), endpoint=self._scene_endpoint
            )
            asserts.assert_equal(resp.status, Status.Success, "Step 9a: RemoveAllScenes status")
            asserts.assert_equal(resp.groupID, group_g1, "Step 9a: RemoveAllScenes groupID")

            self.step("9b",
                      description="TH waits for a FabricSceneInfo report and records RemainingCapacity.",
                      expectation="RemainingCapacity equals MaxRemainingCapacity.")
            self._await_remaining_capacity(max_remaining_capacity, "Step 9b")

            self.step(10,
                      description="TH sends a KeySetRemove command to the GroupKeyManagement cluster with "
                      "GroupKeySetID 0x01a1.",
                      expectation="DUT sends a SUCCESS response.")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keyset1))
            keyset1_installed = False
        finally:
            if keyset1_installed:
                try:
                    await self.send_single_cmd(
                        endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keyset1)
                    )
                except Exception:
                    # Cleanup must not mask the failure that got us here.
                    log.exception("Cleanup: failed to remove GroupKeySetID 0x%04x", keyset1)


if __name__ == "__main__":
    default_matter_test_main()
