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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --camera-remote-analysis --trace-to json:${TRACE_APP}.json --app-pipe /tmp/zonemgmt_2_5_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/zonemgmt_2_5_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import sys
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ZONEMGMT_2_5(MatterTestCommissionedDevice):
    def desc_TC_ZONEMGMT_2_5(self) -> str:
        return "[TC-ZONEMGMT-2.5] Verification for the RemoteZones feature [DUT_Server]"

    def pics_TC_ZONEMGMT_2_5(self) -> list[str]:
        return ["ZONEMGMT.S", "ZONEMGMT.S.F04"]

    def steps_TC_ZONEMGMT_2_5(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep(
                "2",
                "TH sends the CreateTwoDCartesianZone command with the Zone field set to PIXIT.ZONEMGMT.TwoDCartesianZoneStruct_Zone1, NodeId set to PIXIT.ZONEMGMT.Remote_NodeId and EndpointId set to PIXIT.ZONEMGMT.Remote_EndpointId.",
                "Verify that the DUT responds with a CreateTwoDCartesianZoneResponse command, the ZoneID is within 0 to 65534, is a new and unique value in the Zones attribute, and its ZoneInformationStruct has NodeId set to PIXIT.ZONEMGMT.Remote_NodeId and EndpointId set to PIXIT.ZONEMGMT.Remote_EndpointId.",
            ),
            TestStep(
                "3",
                "TH sends the CreateTwoDCartesianZone command with the Zone field set to PIXIT.ZONEMGMT.TwoDCartesianZoneStruct_Zone2 and EndpointId set to PIXIT.ZONEMGMT.Remote_EndpointId (NodeId is absent).",
                "Verify that the DUT responds with CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                "4",
                "TH sends the CreateTwoDCartesianZone command with the Zone field set to PIXIT.ZONEMGMT.TwoDCartesianZoneStruct_Zone2 and NodeId set to an invalid NodeId (e.g. 0xFFFF_FFFF_FFFF_FFFF).",
                "Verify that the DUT responds with CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                "5",
                "Trigger the DUT to generate ZoneTriggered Event for ZoneID_Remote.",
                "Verify that the TH receives the ZoneTriggered event, the Zone value is equal to ZoneID_Remote, and the NodeId value is equal to PIXIT.ZONEMGMT.Remote_NodeId and EndpointId value is equal to PIXIT.ZONEMGMT.Remote_EndpointId.",
            ),
            TestStep(
                "6",
                "TH sends the UpdateTwoDCartesianZone command with the ZoneID field set to ZoneID_Remote and NodeId/EndpointId set to different valid values.",
                "Verify that the DUT responds with SUCCESS status code and the ZoneInformationStruct in the Zones Attribute for the ZoneID_Remote is updated with the new NodeId and EndpointId.",
            ),
            TestStep(
                "7",
                "TH sends the RemoveZone command with the ZoneID field set to ZoneID_Remote.",
                "Verify that the DUT responds with SUCCESS status code.",
            ),
        ]

    async def _trigger_motion_event(self, zone_id: int, prompt_msg: str | None = None):
        # CI: Use app pipe to trigger zone event.
        # Manual: User should trigger a motion event from the defined zone.
        if self.is_pics_sdk_ci_only or self.matter_test_config.pipe_name is not None:
            self.write_to_app_pipe({"Name": "ZoneTriggered", "ZoneId": zone_id})
        else:
            if prompt_msg is None:
                prompt_msg = f"Press enter and immediately start motion activity in zone {zone_id}."
            self.wait_for_user_input(prompt_msg=prompt_msg)

    def _get_int_param(self, keys: list[str], default: int) -> int:
        for key in keys:
            if key in self.user_params:
                val = self.user_params[key]
                return int(val, 0) if isinstance(val, str) else int(val)
            if key in self.matter_test_config.global_test_params:
                val = self.matter_test_config.global_test_params[key]
                return int(val, 0) if isinstance(val, str) else int(val)
        return default

    def _select_distinct_motion_zone_vertices(
        self,
        cluster,
        zones: list,
        two_d_max,
        count: int = 2,
    ) -> list[list]:
        existing_motion_polygons = set()
        for existing_zone in zones or []:
            two_d_cart = getattr(existing_zone, "twoDCartesianZone", None)
            if (
                two_d_cart is not None
                and getattr(two_d_cart, "use", None) == cluster.Enums.ZoneUseEnum.kMotion
                and getattr(two_d_cart, "vertices", None)
            ):
                existing_motion_polygons.add(
                    tuple((int(v.x), int(v.y)) for v in two_d_cart.vertices)
                )

        max_x = max(int(two_d_max.x), 1)
        max_y = max(int(two_d_max.y), 1)
        preferred_boxes = [
            ((10, 10), (20, 10), (20, 20), (10, 20)),
            ((30, 30), (40, 30), (40, 40), (30, 40)),
            ((50, 50), (60, 50), (60, 60), (50, 60)),
        ]
        selected: list[list] = []
        for coords in preferred_boxes:
            if all(x <= max_x and y <= max_y for x, y in coords):
                if coords not in existing_motion_polygons:
                    existing_motion_polygons.add(coords)
                    selected.append(
                        [cluster.Structs.TwoDCartesianVertexStruct(x, y) for x, y in coords]
                    )
                    if len(selected) == count:
                        return selected

        box_w = min(10, max_x)
        box_h = min(10, max_y)
        max_offset_x = max(max_x - box_w, 0)
        max_offset_y = max(max_y - box_h, 0)
        for step_idx in range((max_offset_x + 1) * (max_offset_y + 1) + 1):
            offset_x = (step_idx * (box_w + 5)) % (max_offset_x + 1) if max_offset_x > 0 else 0
            offset_y = (step_idx * (box_h + 5)) % (max_offset_y + 1) if max_offset_y > 0 else 0
            coords = (
                (offset_x, offset_y),
                (offset_x + box_w, offset_y),
                (offset_x + box_w, offset_y + box_h),
                (offset_x, offset_y + box_h),
            )
            if coords not in existing_motion_polygons:
                existing_motion_polygons.add(coords)
                selected.append(
                    [cluster.Structs.TwoDCartesianVertexStruct(x, y) for x, y in coords]
                )
                if len(selected) == count:
                    return selected

        fallback_polygons = [
            ((0, 0), (max_x, 0), (0, max_y)),
            ((0, 0), (max_x, 0), (max_x, max_y), (0, max_y)),
        ]
        for coords in fallback_polygons:
            if len(selected) < count:
                selected.append(
                    [cluster.Structs.TwoDCartesianVertexStruct(x, y) for x, y in coords]
                )
        return selected

    @run_if_endpoint_matches(
        has_feature(Clusters.ZoneManagement, Clusters.ZoneManagement.Bitmaps.Feature.kRemoteZones)
    )
    async def test_TC_ZONEMGMT_2_5(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ZoneManagement
        attr = Clusters.ZoneManagement.Attributes
        commands = Clusters.ZoneManagement.Commands
        enums = Clusters.ZoneManagement.Enums

        self.step("1")
        # Commission DUT - already done
        aFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap
        )
        log.info("Rx'd FeatureMap: %s", aFeatureMap)
        self.twoDCartSupported = bool(aFeatureMap & cluster.Bitmaps.Feature.kTwoDimensionalCartesianZone)
        self.userDefinedSupported = bool(aFeatureMap & cluster.Bitmaps.Feature.kUserDefined)
        self.remoteZonesSupported = bool(aFeatureMap & cluster.Bitmaps.Feature.kRemoteZones)
        self.perZoneSenseSupported = bool(aFeatureMap & cluster.Bitmaps.Feature.kPerZoneSensitivity)

        asserts.assert_true(
            self.remoteZonesSupported and self.twoDCartSupported and self.userDefinedSupported,
            "RemoteZones feature requires TwoDimensionalCartesianZone and UserDefined features",
        )

        max_operational_node_id = 0xFFFFFFEFFFFFFFFF
        max_valid_endpoint_id = 0xFFFE

        default_node_id = self.dut_node_id if self.dut_node_id is not None else 1
        remote_node_id = self._get_int_param(
            ["PIXIT.ZONEMGMT.Remote_NodeId", "remote_node_id"], default=default_node_id
        )
        remote_endpoint_id = self._get_int_param(
            ["PIXIT.ZONEMGMT.Remote_EndpointId", "remote_endpoint_id"], default=endpoint
        )

        if 1 <= remote_node_id < max_operational_node_id:
            default_updated_node_id = remote_node_id + 1
        else:
            default_updated_node_id = max(1, min(max_operational_node_id, remote_node_id - 1))
        updated_remote_node_id = self._get_int_param(
            ["updated_remote_node_id"], default=default_updated_node_id
        )
        if updated_remote_node_id == remote_node_id or not (1 <= updated_remote_node_id <= max_operational_node_id):
            updated_remote_node_id = default_updated_node_id

        if 0 <= remote_endpoint_id < max_valid_endpoint_id:
            default_updated_endpoint_id = remote_endpoint_id + 1
        else:
            default_updated_endpoint_id = max(0, min(max_valid_endpoint_id, remote_endpoint_id - 1))
        updated_remote_endpoint_id = self._get_int_param(
            ["updated_remote_endpoint_id"], default=default_updated_endpoint_id
        )
        if updated_remote_endpoint_id == remote_endpoint_id or not (0 <= updated_remote_endpoint_id <= max_valid_endpoint_id):
            updated_remote_endpoint_id = default_updated_endpoint_id

        twoDCartesianMax = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.TwoDCartesianMax
        )
        log.info("Rx'd TwoDCartesianMax: %s", twoDCartesianMax)

        zoneID_Remote = None
        created_trigger = False
        event_listener = None

        try:
            self.step("2")
            zonesBeforeCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info("Rx'd Zones before Create: %s", zonesBeforeCreate)

            zone1Vertices, zone2Vertices = self._select_distinct_motion_zone_vertices(
                cluster, zonesBeforeCreate, twoDCartesianMax, count=2
            )
            zone1 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1",
                use=enums.ZoneUseEnum.kMotion,
                vertices=zone1Vertices,
                color="#00FFFF",
            )
            zone2 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone2",
                use=enums.ZoneUseEnum.kMotion,
                vertices=zone2Vertices,
                color="#00FFFF",
            )

            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zone1,
                nodeID=remote_node_id,
                endpointID=remote_endpoint_id,
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            log.info("Rx'd CreateTwoDCartesianZoneResponse: %s", cmdResponse)
            asserts.assert_equal(
                type(cmdResponse),
                commands.CreateTwoDCartesianZoneResponse,
                "Incorrect response type",
            )
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianZoneResponse does not contain ZoneID"
            )
            zoneID_Remote = cmdResponse.zoneID
            asserts.assert_true(
                0 <= zoneID_Remote <= 65534,
                f"ZoneID {zoneID_Remote} is out of valid range [0, 65534]",
            )

            matchingZoneBefore = next(
                (z for z in zonesBeforeCreate if z.zoneID == zoneID_Remote), None
            )
            asserts.assert_is_none(
                matchingZoneBefore, f"Zone with ID {zoneID_Remote} already existed in Zones"
            )

            zonesAfterCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info("Rx'd Zones after Create: %s", zonesAfterCreate)

            matchingZonesAfter = [z for z in zonesAfterCreate if z.zoneID == zoneID_Remote]
            asserts.assert_equal(
                len(matchingZonesAfter),
                1,
                f"Expected exactly 1 Zone with ID {zoneID_Remote} in Zones, found {len(matchingZonesAfter)}",
            )
            matchingZone = matchingZonesAfter[0]
            asserts.assert_equal(
                matchingZone.zoneType,
                enums.ZoneTypeEnum.kTwoDCARTZone,
                "ZoneType of created Zone is not TwoDCARTZone",
            )
            asserts.assert_equal(
                matchingZone.zoneSource,
                enums.ZoneSourceEnum.kUser,
                "ZoneSource of created Zone is not User",
            )
            asserts.assert_equal(
                matchingZone.twoDCartesianZone.vertices,
                zone1Vertices,
                "ZoneVertices of created Zone are mismatched",
            )
            asserts.assert_equal(
                matchingZone.nodeID,
                remote_node_id,
                f"NodeID of created Zone ({matchingZone.nodeID}) does not match {remote_node_id}",
            )
            asserts.assert_equal(
                matchingZone.endpointID,
                remote_endpoint_id,
                f"EndpointID of created Zone ({matchingZone.endpointID}) does not match {remote_endpoint_id}",
            )

            self.step("3")
            for invalid_node_id in (None, NullValue):
                try:
                    createMissingNodeIdCmd = commands.CreateTwoDCartesianZone(
                        zone=zone2,
                        nodeID=invalid_node_id,
                        endpointID=remote_endpoint_id,
                    )
                    await self.send_single_cmd(endpoint=endpoint, cmd=createMissingNodeIdCmd)
                    asserts.fail(
                        "Unexpected success when expecting CONSTRAINT_ERROR due to EndpointID present with NodeID absent/null"
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status,
                        Status.ConstraintError,
                        "Unexpected error returned when trying to create zone with EndpointID and no NodeID",
                    )

            self.step("4")
            try:
                createInvalidNodeIdCmd = commands.CreateTwoDCartesianZone(
                    zone=zone2,
                    nodeID=0xFFFFFFFFFFFFFFFF,
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=createInvalidNodeIdCmd)
                asserts.fail(
                    "Unexpected success when expecting CONSTRAINT_ERROR due to invalid NodeID"
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.ConstraintError,
                    "Unexpected error returned when trying to create zone with invalid NodeID",
                )

            self.step("5")
            triggers = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Triggers
            )
            existingTrigger = next(
                (t for t in triggers if t.zoneID == zoneID_Remote), None
            )
            if existingTrigger is None:
                if self.perZoneSenseSupported:
                    sensitivityMax = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=cluster, attribute=attr.SensitivityMax
                    )
                    sensitivity = min(4, max(1, int(sensitivityMax)))
                else:
                    sensitivity = None
                zoneTrigger = cluster.Structs.ZoneTriggerControlStruct(
                    zoneID=zoneID_Remote,
                    initialDuration=4,
                    augmentationDuration=2,
                    maxDuration=10,
                    blindDuration=4,
                    sensitivity=sensitivity,
                )
                await self.send_single_cmd(
                    endpoint=endpoint,
                    cmd=commands.CreateOrUpdateTrigger(trigger=zoneTrigger),
                )
                created_trigger = True

            event_listener = EventSubscriptionHandler(expected_cluster=cluster)
            await event_listener.start(
                self.default_controller,
                self.dut_node_id,
                endpoint=endpoint,
                min_interval_sec=0,
                max_interval_sec=30,
            )

            await self._trigger_motion_event(
                zoneID_Remote,
                prompt_msg=f"Press enter and immediately start motion activity in remote zone {zoneID_Remote}.",
            )

            deadline = time.monotonic() + 30.0
            while True:
                remaining = deadline - time.monotonic()
                event = event_listener.wait_for_event_type_report(
                    cluster.Events.ZoneTriggered, timeout_sec=remaining
                )
                if event.zone == zoneID_Remote:
                    break
            log.info("Rx'd ZoneTriggered event: %s", event)
            asserts.assert_equal(
                type(event),
                cluster.Events.ZoneTriggered,
                "Incorrect event type",
            )
            asserts.assert_equal(
                event.zone,
                zoneID_Remote,
                f"Expected ZoneTriggered event zone {zoneID_Remote}, got {event.zone}",
            )
            asserts.assert_equal(
                event.nodeID,
                remote_node_id,
                f"Expected ZoneTriggered event nodeID {remote_node_id}, got {event.nodeID}",
            )
            asserts.assert_equal(
                event.endpointID,
                remote_endpoint_id,
                f"Expected ZoneTriggered event endpointID {remote_endpoint_id}, got {event.endpointID}",
            )

            event_listener.cancel()
            event_listener = None

            if created_trigger:
                await self.send_single_cmd(
                    endpoint=endpoint,
                    cmd=commands.RemoveTrigger(zoneID=zoneID_Remote),
                )
                created_trigger = False

            self.step("6")
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID_Remote,
                zone=zone1,
                nodeID=updated_remote_node_id,
                endpointID=updated_remote_endpoint_id,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)

            zonesAfterUpdate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info("Rx'd Zones after Update: %s", zonesAfterUpdate)

            updatedZone = next(
                (z for z in zonesAfterUpdate if z.zoneID == zoneID_Remote), None
            )
            asserts.assert_is_not_none(
                updatedZone, f"Zone with ID {zoneID_Remote} not found after update"
            )
            asserts.assert_equal(
                updatedZone.zoneType,
                enums.ZoneTypeEnum.kTwoDCARTZone,
                "ZoneType of updated Zone is not TwoDCARTZone",
            )
            asserts.assert_equal(
                updatedZone.zoneSource,
                enums.ZoneSourceEnum.kUser,
                "ZoneSource of updated Zone is not User",
            )
            asserts.assert_equal(
                updatedZone.twoDCartesianZone.vertices,
                zone1Vertices,
                "ZoneVertices of updated Zone are mismatched",
            )
            asserts.assert_equal(
                updatedZone.nodeID,
                updated_remote_node_id,
                f"NodeID of updated Zone ({updatedZone.nodeID}) does not match {updated_remote_node_id}",
            )
            asserts.assert_equal(
                updatedZone.endpointID,
                updated_remote_endpoint_id,
                f"EndpointID of updated Zone ({updatedZone.endpointID}) does not match {updated_remote_endpoint_id}",
            )

            self.step("7")
            removedZoneId = zoneID_Remote
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.RemoveZone(zoneID=zoneID_Remote)
            )
            zoneID_Remote = None

            zonesAfterRemove = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            asserts.assert_is_none(
                next((z for z in zonesAfterRemove if z.zoneID == removedZoneId), None),
                f"Zone with ID {removedZoneId} still present in Zones after RemoveZone",
            )
        finally:
            if event_listener is not None:
                event_listener.cancel()
            cleanup_error = None
            if created_trigger and zoneID_Remote is not None:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=commands.RemoveTrigger(zoneID=zoneID_Remote),
                    )
                except InteractionModelError as e:
                    log.info("Cleanup RemoveTrigger: %s", e)
                    if cleanup_error is None:
                        cleanup_error = e
            if zoneID_Remote is not None:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=commands.RemoveZone(zoneID=zoneID_Remote),
                    )
                except InteractionModelError as e:
                    log.info("Cleanup RemoveZone: %s", e)
                    if cleanup_error is None:
                        cleanup_error = e
            if cleanup_error is not None and sys.exc_info()[0] is None:
                raise cleanup_error


if __name__ == "__main__":
    default_matter_test_main()
