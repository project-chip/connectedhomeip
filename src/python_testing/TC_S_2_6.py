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


import logging
import time
import typing

from mobly import asserts, signals

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, pics
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

_MIN_INTERVAL_FLOOR_SEC = 1
_MAX_INTERVAL_CEILING_SEC = 5

# Reports are throttled by MinIntervalFloor, so allow several floor periods of margin.
_REPORT_TIMEOUT_SEC = 2 * _MAX_INTERVAL_CEILING_SEC

# How long to wait when asserting that another fabric did not receive an updated report.
_NO_UPDATE_WAIT_SEC = _MIN_INTERVAL_FLOOR_SEC + 2

# Smallest SceneTableSize the spec permits.
_MIN_SCENE_TABLE_SIZE = 16


_GROUP_ID = 0x0000

_TRANSITION_TIME_MS = 20000


class TC_S_2_6(MatterTestCommissionedDevice):
    def teardown_test(self):
        self._shutdown_subscriptions()

    def _shutdown_subscriptions(self) -> None:
        for attr in ("_th1_sub", "_th2_sub", "_th3_sub"):
            sub = getattr(self, attr, None)
            if sub is None:
                continue
            delattr(self, attr)
            try:
                sub.Shutdown()
            except Exception:
                # Cleanup runs in a finally block: never mask the original failure.
                log.exception("Cleanup: failed to shut down subscription %s", attr)

    def _remaining_capacity(self, fabric_scene_info, fabric_index: int) -> int | None:
        for entry in fabric_scene_info or []:
            if entry.fabricIndex == fabric_index:
                return entry.remainingCapacity
        return None

    def _assert_remaining_capacity(self, fabric_scene_info, fabric_index: int, expected: int, context: str) -> None:
        actual = self._remaining_capacity(fabric_scene_info, fabric_index)
        asserts.assert_is_not_none(actual, f"{context}: no FabricSceneInfo entry for fabric {fabric_index}")
        asserts.assert_equal(actual, expected, f"{context}: RemainingCapacity for fabric {fabric_index}")

    def _await_remaining_capacity(
        self, cb: AttributeSubscriptionHandler, fabric_index: int, expected: int, context: str
    ) -> None:
        report = cb.wait_for_attribute_report()
        self._assert_remaining_capacity(report.value, fabric_index, expected, context)

    def _await_remaining_capacity_equals(
        self, cb: AttributeSubscriptionHandler, fabric_index: int, expected: int, context: str
    ) -> None:
        """Drain reports until RemainingCapacity equals expected (or time out)."""
        deadline = time.time() + _REPORT_TIMEOUT_SEC
        last: int | None = None
        while time.time() < deadline:
            remaining_timeout = max(0.1, deadline - time.time())
            try:
                report = cb.wait_for_attribute_report(timeout_sec=remaining_timeout)
            except signals.TestFailure:
                # The handler fails the test on timeout; report the last value seen instead.
                break
            last = self._remaining_capacity(report.value, fabric_index)
            if last == expected:
                return
        asserts.fail(f"{context}: RemainingCapacity never reached {expected} (last seen: {last})")

    def _assert_no_remaining_capacity_change(
        self,
        cb: AttributeSubscriptionHandler,
        fabric_index: int,
        expected_unchanged: int,
        context: str,
        wait_sec: float = _NO_UPDATE_WAIT_SEC,
    ) -> None:
        """Wait briefly; any reports that arrive must still carry the unchanged RemainingCapacity."""
        time.sleep(wait_sec)
        while cb.attribute_queue.qsize() > 0:
            report = cb.attribute_queue.get()
            actual = self._remaining_capacity(report.value, fabric_index)
            asserts.assert_equal(
                actual,
                expected_unchanged,
                f"{context}: unexpected RemainingCapacity change for fabric {fabric_index}",
            )

    async def _commission_on_new_fabric(self, fabric_id_offset: int, controller_node_id_offset: int):
        ca = self.certificate_authority_manager.NewCertificateAuthority()
        fabric_admin = ca.NewFabricAdmin(
            vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + fabric_id_offset
        )
        controller = fabric_admin.NewController(
            nodeId=self.matter_test_config.controller_node_id + controller_node_id_offset,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )
        params = await self.open_commissioning_window(self.th1, self.dut_node_id)
        await controller.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=params.randomDiscriminator,
        )
        fabric_index = typing.cast(
            int,
            await self.read_single_attribute_check_success(
                dev_ctrl=controller,
                endpoint=0,
                cluster=Clusters.OperationalCredentials,
                attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
            ),
        )
        return controller, fabric_index

    async def _remove_fabric(self, fabric_index: int, context: str) -> None:
        resp = await self.send_single_cmd(
            Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_index),
            dev_ctrl=self.th1,
            endpoint=0,
        )
        asserts.assert_equal(resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk, context)
        asserts.assert_equal(resp.fabricIndex, fabric_index, f"{context} fabricIndex")
        self._fabrics_to_remove.remove(fabric_index)

    async def _remove_remaining_fabrics(self) -> None:
        """Remove fabrics the test commissioned but did not get to remove."""
        for fabric_index in list(self._fabrics_to_remove):
            try:
                await self._remove_fabric(fabric_index, "Cleanup: RemoveFabric")
            except Exception:
                # Cleanup runs in a finally block: never mask the original failure.
                log.exception("Cleanup: failed to remove fabric %s", fabric_index)

    async def _subscribe_fabric_scene_info(self, controller):
        subscription = await controller.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(self._endpoint, Clusters.ScenesManagement.Attributes.FabricSceneInfo)],
            reportInterval=(_MIN_INTERVAL_FLOOR_SEC, _MAX_INTERVAL_CEILING_SEC),
            fabricFiltered=True,
            keepSubscriptions=True,
            autoResubscribe=False,
        )
        cb = AttributeSubscriptionHandler(
            expected_cluster=Clusters.ScenesManagement,
            expected_attribute=Clusters.ScenesManagement.Attributes.FabricSceneInfo,
        )
        subscription.SetAttributeUpdateCallback(cb)
        return subscription, cb

    def _priming_fabric_scene_info(self, subscription):
        priming = subscription.GetAttributes()
        return priming[self._endpoint][Clusters.ScenesManagement][Clusters.ScenesManagement.Attributes.FabricSceneInfo]

    async def _remove_all_scenes(self, controller, context: str):
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=_GROUP_ID),
            dev_ctrl=controller,
            endpoint=self._endpoint,
        )
        asserts.assert_equal(resp.status, Status.Success, f"{context}: RemoveAllScenes status")
        asserts.assert_equal(resp.groupID, _GROUP_ID, f"{context}: RemoveAllScenes groupID")

    async def _add_scene(self, controller, scene_id: int, context: str, expected_status: Status = Status.Success):
        resp = await self.send_single_cmd(
            Clusters.ScenesManagement.Commands.AddScene(
                groupID=_GROUP_ID,
                sceneID=scene_id,
                transitionTime=_TRANSITION_TIME_MS,
                sceneName="",
                extensionFieldSetStructs=[],
            ),
            dev_ctrl=controller,
            endpoint=self._endpoint,
        )
        asserts.assert_equal(resp.status, expected_status, f"{context}: AddScene status")
        asserts.assert_equal(resp.groupID, _GROUP_ID, f"{context}: AddScene groupID")
        asserts.assert_equal(resp.sceneID, scene_id, f"{context}: AddScene sceneID")
        return resp

    async def _fill_until_remaining_zero(
        self,
        controller,
        cb: AttributeSubscriptionHandler,
        fabric_index: int,
        start_scene_id: int,
        context: str,
    ) -> int:
        """Add scenes from start_scene_id until RemainingCapacity reports 0. Returns next unused SceneID."""
        scene_id = start_scene_id
        while True:
            cb.reset()
            await self._add_scene(controller, scene_id, f"{context} scene 0x{scene_id:02x}")
            report = cb.wait_for_attribute_report()
            remaining = self._remaining_capacity(report.value, fabric_index)
            asserts.assert_is_not_none(remaining, f"{context}: missing FabricSceneInfo entry after AddScene 0x{scene_id:02x}")
            log.info("%s: after AddScene 0x%02x RemainingCapacity=%s", context, scene_id, remaining)
            scene_id += 1
            if remaining == 0:
                return scene_id

    @pics("S.S")
    @async_test_body
    async def test_TC_S_2_6(self):
        """[TC-S-2.6] RemainingCapacity functionality with DUT as Server - Multi-Fabric"""
        self._endpoint = self.get_endpoint()
        self.th1 = self.default_controller
        self._fabrics_to_remove: list[int] = []

        self.step(0, "Commission DUT to TH1, TH2 and TH3 on distinct fabrics.", is_commissioning=True)
        self._th1_fabric = typing.cast(
            int,
            await self.read_single_attribute_check_success(
                cluster=Clusters.OperationalCredentials,
                attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
                endpoint=0,
            ),
        )
        try:
            self.th2, self._th2_fabric = await self._commission_on_new_fabric(fabric_id_offset=1, controller_node_id_offset=2)
            self._fabrics_to_remove.append(self._th2_fabric)
            self.th3, self._th3_fabric = await self._commission_on_new_fabric(fabric_id_offset=2, controller_node_id_offset=3)
            self._fabrics_to_remove.append(self._th3_fabric)
            log.info("Fabric indexes: TH1=%s TH2=%s TH3=%s", self._th1_fabric, self._th2_fabric, self._th3_fabric)

            self.step("1a", "TH1 sends a RemoveAllScenes command to DUT with GroupID 0x0000. "
                      "DUT sends RemoveAllScenesResponse with Status SUCCESS and GroupID 0x0000.")
            await self._remove_all_scenes(self.th1, "Step 1a")

            self.step("1b", "Repeat Step 1a with TH2.")
            await self._remove_all_scenes(self.th2, "Step 1b")

            self.step("1c", "Repeat Step 1a with TH3.")
            await self._remove_all_scenes(self.th3, "Step 1c")

            self.step("2a", "TH1 reads the SceneTableSize attribute from the DUT. "
                      "DUT reports SceneTableSize (at least 16). MaxRemainingCapacity is (SceneTableSize - 1) / 2.")
            scene_table_size = typing.cast(
                int,
                await self.read_single_attribute_check_success(
                    endpoint=self._endpoint,
                    cluster=Clusters.ScenesManagement,
                    attribute=Clusters.ScenesManagement.Attributes.SceneTableSize,
                ),
            )
            asserts.assert_greater_equal(
                scene_table_size, _MIN_SCENE_TABLE_SIZE, f"Step 2a: SceneTableSize must be at least {_MIN_SCENE_TABLE_SIZE}"
            )
            max_remaining_capacity = (scene_table_size - 1) // 2
            log.info("SceneTableSize=%s MaxRemainingCapacity=%s", scene_table_size, max_remaining_capacity)

            self.step("2b", "TH1 subscribes to FabricSceneInfo. "
                      "Subscription activates; RemainingCapacity equals MaxRemainingCapacity for TH1's fabric entry.")
            self._th1_sub, self._th1_cb = await self._subscribe_fabric_scene_info(self.th1)
            self._assert_remaining_capacity(
                self._priming_fabric_scene_info(self._th1_sub), self._th1_fabric, max_remaining_capacity, "Step 2b"
            )

            self.step("2c", "Repeat Step 2b with TH2.")
            self._th2_sub, self._th2_cb = await self._subscribe_fabric_scene_info(self.th2)
            self._assert_remaining_capacity(
                self._priming_fabric_scene_info(self._th2_sub), self._th2_fabric, max_remaining_capacity, "Step 2c"
            )

            self.step("2d", "Repeat Step 2b with TH3.")
            self._th3_sub, self._th3_cb = await self._subscribe_fabric_scene_info(self.th3)
            self._assert_remaining_capacity(
                self._priming_fabric_scene_info(self._th3_sub), self._th3_fabric, max_remaining_capacity, "Step 2d"
            )

            self.step("3a", "TH1 sends AddScene with GroupID 0x0000, SceneID 0x01, TransitionTime 20000 and no "
                      "extension field sets. "
                      "DUT sends AddSceneResponse with Status SUCCESS, GroupID 0x0000 and SceneID 0x01.")
            self._th1_cb.reset()
            await self._add_scene(self.th1, 0x01, "Step 3a")

            self.step("3b", "TH1 waits for a FabricSceneInfo report. "
                      "RemainingCapacity equals MaxRemainingCapacity - 1.")
            self._await_remaining_capacity(self._th1_cb, self._th1_fabric, max_remaining_capacity - 1, "Step 3b")

            self.step("4a", "TH1 sends AddScene with SceneID starting at 2 and incrementing until "
                      "RemainingCapacity becomes 0. "
                      "Each AddScene succeeds; RemainingCapacity decreases to 0.")
            th1_next_scene = await self._fill_until_remaining_zero(
                self.th1, self._th1_cb, self._th1_fabric, start_scene_id=0x02, context="Step 4a"
            )

            self.step("4b", "TH1 sends AddScene with SceneID one more than the last value used in Step 4a. "
                      "DUT sends AddSceneResponse with Status RESOURCE_EXHAUSTED.")
            await self._add_scene(self.th1, th1_next_scene, "Step 4b", expected_status=Status.ResourceExhausted)

            self.step("5a", "Repeat Step 4a with TH2. "
                      "TH2 RemainingCapacity decreases to 0. TH3 RemainingCapacity decreases to "
                      "SceneTableSize - (2 * MaxRemainingCapacity).")
            self._th3_cb.reset()
            th2_next_scene = await self._fill_until_remaining_zero(
                self.th2, self._th2_cb, self._th2_fabric, start_scene_id=0x01, context="Step 5a"
            )
            th3_expected_after_th2_full = scene_table_size - (2 * max_remaining_capacity)
            self._await_remaining_capacity_equals(
                self._th3_cb, self._th3_fabric, th3_expected_after_th2_full, "Step 5a TH3"
            )

            self.step("5b", "Repeat Step 4b with TH2. "
                      "DUT sends AddSceneResponse with Status RESOURCE_EXHAUSTED.")
            await self._add_scene(self.th2, th2_next_scene, "Step 5b", expected_status=Status.ResourceExhausted)

            self.step("6a", "Repeat Step 4a with TH3. "
                      "TH3 RemainingCapacity decreases to 0.")
            # TH3 already has RemainingCapacity == SceneTableSize - 2*MaxRemainingCapacity; fill to 0.
            # Use SceneIDs starting at 0x02 so SceneID 0x01 remains free for Step 6b.
            th3_next_scene = await self._fill_until_remaining_zero(
                self.th3, self._th3_cb, self._th3_fabric, start_scene_id=0x02, context="Step 6a"
            )
            asserts.assert_greater(th3_next_scene, 0x01, "Step 6a: expected SceneID 0x01 to remain unused")

            self.step("6b", "TH3 sends AddScene with GroupID 0x0000 and SceneID 0x01. "
                      "DUT sends AddSceneResponse with Status RESOURCE_EXHAUSTED.")
            await self._add_scene(self.th3, 0x01, "Step 6b", expected_status=Status.ResourceExhausted)

            self.step(7, "TH3 sends StoreScene with GroupID 0x0000 and SceneID 0xfe. "
                      "DUT sends StoreSceneResponse with Status RESOURCE_EXHAUSTED.")
            resp = await self.send_single_cmd(
                Clusters.ScenesManagement.Commands.StoreScene(groupID=_GROUP_ID, sceneID=0xFE),
                dev_ctrl=self.th3,
                endpoint=self._endpoint,
            )
            asserts.assert_equal(resp.status, Status.ResourceExhausted, "Step 7: StoreScene status")
            asserts.assert_equal(resp.groupID, _GROUP_ID, "Step 7: StoreScene groupID")
            asserts.assert_equal(resp.sceneID, 0xFE, "Step 7: StoreScene sceneID")

            self.step(8, "TH1 sends CopyScene mode 0x00 from GroupID/SceneID 0x0000/0x01 to a unused "
                      "destination SceneID. DUT sends CopySceneResponse with Status RESOURCE_EXHAUSTED, groupIdentifierFrom 0x0000 "
                      "and sceneIdentifierFrom 0x01.")
            copy_to_scene = th1_next_scene
            resp = await self.send_single_cmd(
                Clusters.ScenesManagement.Commands.CopyScene(
                    mode=0x00,
                    groupIdentifierFrom=_GROUP_ID,
                    sceneIdentifierFrom=0x01,
                    groupIdentifierTo=_GROUP_ID,
                    sceneIdentifierTo=copy_to_scene,
                ),
                dev_ctrl=self.th1,
                endpoint=self._endpoint,
            )
            asserts.assert_equal(resp.status, Status.ResourceExhausted, "Step 8: CopyScene status")
            asserts.assert_equal(resp.groupIdentifierFrom, _GROUP_ID, "Step 8: CopyScene groupIdentifierFrom")
            asserts.assert_equal(resp.sceneIdentifierFrom, 0x01, "Step 8: CopyScene sceneIdentifierFrom")

            self.step("9a", "TH1 sends RemoveAllScenes with GroupID 0x0000. "
                      "DUT sends RemoveAllScenesResponse with Status SUCCESS and GroupID 0x0000.")
            self._th1_cb.reset()
            self._th2_cb.reset()
            await self._remove_all_scenes(self.th1, "Step 9a")

            self.step("9b", "TH1 waits for a FabricSceneInfo report; TH2 must not see an updated RemainingCapacity. "
                      "TH1 RemainingCapacity equals MaxRemainingCapacity. TH2 RemainingCapacity is unchanged.")
            self._await_remaining_capacity(self._th1_cb, self._th1_fabric, max_remaining_capacity, "Step 9b")
            self._assert_no_remaining_capacity_change(self._th2_cb, self._th2_fabric, 0, "Step 9b TH2")

            self.step("10a", "TH2 sends RemoveAllScenes with GroupID 0x0000. "
                      "DUT sends RemoveAllScenesResponse with Status SUCCESS and GroupID 0x0000.")
            self._th1_cb.reset()
            self._th2_cb.reset()
            await self._remove_all_scenes(self.th2, "Step 10a")

            self.step("10b", "TH2 waits for a FabricSceneInfo report; TH1 must not see an updated RemainingCapacity. "
                      "TH2 RemainingCapacity equals MaxRemainingCapacity. TH1 RemainingCapacity is unchanged.")
            self._await_remaining_capacity(self._th2_cb, self._th2_fabric, max_remaining_capacity, "Step 10b")
            self._assert_no_remaining_capacity_change(
                self._th1_cb, self._th1_fabric, max_remaining_capacity, "Step 10b TH1"
            )

            self.step("11a", "TH1 sends RemoveFabric for TH2's fabric index. "
                      "DUT responds with Status SUCCESS and TH2's fabric index.")
            await self._remove_fabric(self._th2_fabric, "Step 11a")

            self.step("11b", "TH1 sends RemoveFabric for TH3's fabric index. "
                      "DUT responds with Status SUCCESS and TH3's fabric index.")
            await self._remove_fabric(self._th3_fabric, "Step 11b")
        finally:
            self._shutdown_subscriptions()
            await self._remove_remaining_fabrics()


if __name__ == "__main__":
    default_matter_test_main()
