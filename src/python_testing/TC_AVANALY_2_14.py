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
#     app-args: --discriminator 1234 --KVS kvs1 --camera-remote-analysis --trace-to json:${TRACE_APP}.json --app-pipe /tmp/avanaly_2_14_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/avanaly_2_14_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import sys

from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_14(MatterTestCommissionedDevice, AVANALYTestBase):

    def desc_TC_AVANALY_2_14(self) -> str:
        return "[TC-AVANALY-2.14] Validate RemoteZones integration and SourceNodeId consistency with Server as DUT"

    def steps_TC_AVANALY_2_14(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(
                2,
                "TH creates a remote zone in Zone Management cluster with a valid NodeId. Save as remote_zone_id.",
            ),
            TestStep(
                3,
                "TH establishes an analysis stream to the same remote camera NodeId. Verify success response.",
            ),
            TestStep(4, "Simulate detection of an ambient context in remote_zone_id."),
            TestStep(
                5,
                "TH receives AnalysisSessionStart event. Verify SourceNodeId matches remote camera NodeId.",
            ),
            TestStep(
                6,
                "Verify TriggeredZones contains remote_zone_id in AnalysisSessionStart event.",
            ),
            TestStep(
                7,
                "TH receives PerceivedContext event. Verify SourceNodeId matches remote camera NodeId.",
            ),
            TestStep(
                8,
                "TH receives AnalysisSessionEnd event. Verify SourceNodeId matches remote camera NodeId.",
            ),
        ]

    def pics_TC_AVANALY_2_14(self) -> list[str]:
        return [
            "AVANALY.S",
            "ZONEMGMT.S.F00",
            "ZONEMGMT.S.F02",
            "ZONEMGMT.S.F04",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_14(self):
        cluster = Clusters.Objects.AvAnalysis
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        zone_cluster = Clusters.Objects.ZoneManagement
        has_two_d_cart = self.check_pics("ZONEMGMT.S.F00")
        has_user_defined = self.check_pics("ZONEMGMT.S.F02")
        has_remote_zones = self.check_pics("ZONEMGMT.S.F04")
        if hasattr(zone_cluster.Bitmaps.Feature, "kRemoteZones"):
            try:
                zone_feature_map = await self.read_single_attribute_check_success(
                    endpoint=endpoint,
                    cluster=zone_cluster,
                    attribute=zone_cluster.Attributes.FeatureMap,
                )
                has_two_d_cart = (
                    zone_feature_map
                    & zone_cluster.Bitmaps.Feature.kTwoDimensionalCartesianZone
                ) != 0
                has_user_defined = (
                    zone_feature_map & zone_cluster.Bitmaps.Feature.kUserDefined
                ) != 0
                has_remote_zones = (
                    zone_feature_map & zone_cluster.Bitmaps.Feature.kRemoteZones
                ) != 0
            except InteractionModelError as e:
                log.info("ZoneManagement cluster query exception: %s", e)

        if not (has_remote_zones and has_two_d_cart and has_user_defined):
            log.info(
                "Required ZoneManagement features (TwoDimensionalCartesianZone, UserDefined, RemoteZones) not supported, skipping TC-AVANALY-2.14"
            )
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            return

        if hasattr(self, "get_camera_node_id"):
            remote_node_id = self.get_camera_node_id()
        else:
            remote_node_id = self.user_params.get(
                "camera_node_id", self.user_params.get("remote_node_id", self.dut_node_id)
            )
            remote_node_id = (
                int(remote_node_id, 0)
                if isinstance(remote_node_id, str)
                else int(remote_node_id)
            )

        self.step(2)
        zones = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=zone_cluster,
            attribute=zone_cluster.Attributes.Zones,
        )
        two_d_max = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=zone_cluster,
            attribute=zone_cluster.Attributes.TwoDCartesianMax,
        )
        existing_motion_polygons = set()
        for existing_zone in zones or []:
            two_d_cart = getattr(existing_zone, "twoDCartesianZone", None)
            if (
                two_d_cart is not None
                and getattr(two_d_cart, "use", None)
                == zone_cluster.Enums.ZoneUseEnum.kMotion
                and getattr(two_d_cart, "vertices", None)
            ):
                existing_motion_polygons.add(
                    tuple((int(v.x), int(v.y)) for v in two_d_cart.vertices)
                )

        idx = (len(zones) if zones else 0) + 1
        max_x = max(int(two_d_max.x), 1)
        max_y = max(int(two_d_max.y), 1)
        box_w = min(10, max_x)
        box_h = min(10, max_y)
        max_offset_x = max(max_x - box_w, 0)
        max_offset_y = max(max_y - box_h, 0)
        zone_vertices = None
        for step_idx in range((max_offset_x + 1) * (max_offset_y + 1) + 1):
            candidate_idx = idx + step_idx
            offset_x = (
                ((candidate_idx - 1) * (box_w + 5) + step_idx) % (max_offset_x + 1)
                if max_offset_x > 0
                else 0
            )
            offset_y = (
                ((candidate_idx - 1) * (box_h + 5) + step_idx) % (max_offset_y + 1)
                if max_offset_y > 0
                else 0
            )
            candidate_coords = (
                (offset_x, offset_y),
                (offset_x + box_w, offset_y),
                (offset_x + box_w, offset_y + box_h),
                (offset_x, offset_y + box_h),
            )
            if candidate_coords not in existing_motion_polygons:
                zone_vertices = [
                    zone_cluster.Structs.TwoDCartesianVertexStruct(x, y)
                    for x, y in candidate_coords
                ]
                break
        if zone_vertices is None:
            zone_vertices = [
                zone_cluster.Structs.TwoDCartesianVertexStruct(0, 0),
                zone_cluster.Structs.TwoDCartesianVertexStruct(max_x, 0),
                zone_cluster.Structs.TwoDCartesianVertexStruct(0, max_y),
            ]
        zone_to_create = zone_cluster.Structs.TwoDCartesianZoneStruct(
            name=f"RemoteZone{idx}",
            use=zone_cluster.Enums.ZoneUseEnum.kMotion,
            vertices=zone_vertices,
            color="#00FFFF",
        )
        create_remote_zone_cmd = zone_cluster.Commands.CreateTwoDCartesianZone(
            zone=zone_to_create,
            nodeID=remote_node_id,
        )
        remote_zone_id = None
        event_callback = None
        stream_id = None

        try:
            create_resp = await self.send_single_cmd(
                endpoint=endpoint, cmd=create_remote_zone_cmd
            )
            remote_zone_id = getattr(create_resp, "zoneID", None)
            asserts.assert_equal(
                type(create_resp),
                zone_cluster.Commands.CreateTwoDCartesianZoneResponse,
                "Expected CreateTwoDCartesianZoneResponse",
            )
            asserts.assert_is_not_none(
                remote_zone_id, "CreateTwoDCartesianZoneResponse must contain zoneID"
            )

            zones_after = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=zone_cluster,
                attribute=zone_cluster.Attributes.Zones,
            )
            matching_zones = [z for z in zones_after if z.zoneID == remote_zone_id]
            asserts.assert_equal(
                len(matching_zones),
                1,
                f"Created remote zone {remote_zone_id} not found in Zones attribute",
            )
            asserts.assert_equal(
                matching_zones[0].nodeID,
                remote_node_id,
                f"Expected remote zone nodeID {remote_node_id}, got {matching_zones[0].nodeID}",
            )
            log.info(
                "Remote NodeId: %d, Remote Zone ID: %d", remote_node_id, remote_zone_id
            )

            self.step(3)
            resp = await self.send_establish_analysis_stream_cmd(
                endpoint, node_id=remote_node_id
            )
            log.info("EstablishAnalysisStreamResponse: %s", resp)
            asserts.assert_is_not_none(resp, "Expected EstablishAnalysisStreamResponse")
            stream_id = getattr(resp, "analysisStreamID", None)
            asserts.assert_is_not_none(
                stream_id, "EstablishAnalysisStreamResponse must contain analysisStreamID"
            )

            supported_contexts = await self.read_avanaly_attribute_expect_success(
                endpoint, cluster.Attributes.SupportedAmbientContexts
            )
            if supported_contexts:
                await self.send_enable_context_triggers_cmd(
                    endpoint, context_triggers=NullValue
                )

            # Set up event subscription handler
            event_callback = EventSubscriptionHandler(expected_cluster=cluster)
            await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

            self.step(4)
            if self.matter_test_config.pipe_name:
                self.write_to_app_pipe(
                    {
                        "Name": "AvAnalysisSessionStart",
                        "ZoneIds": [remote_zone_id],
                        "SourceNodeId": remote_node_id,
                    }
                )
            elif not self.is_ci:
                self.wait_for_user_input(
                    prompt_msg=f"Simulate detection of an ambient context in remote zone {remote_zone_id} for node {remote_node_id}. Press Enter once initiated."
                )

            self.step(5)
            if self.matter_test_config.pipe_name or not self.is_ci:
                start_event = event_callback.wait_for_event_report(
                    cluster.Events.AnalysisSessionStart, timeout_sec=30
                )
                log.info("AnalysisSessionStart event: %s", start_event)
                asserts.assert_is_not_none(
                    start_event, "Expected AnalysisSessionStart event"
                )
                asserts.assert_is_not_none(
                    start_event.sourceNodeId,
                    "SourceNodeId must not be None in AnalysisSessionStart",
                )
                asserts.assert_equal(
                    start_event.sourceNodeId,
                    remote_node_id,
                    "SourceNodeId mismatch in AnalysisSessionStart",
                )

                self.step(6)
                asserts.assert_is_not_none(
                    start_event.triggeredZones,
                    "TriggeredZones must not be None in AnalysisSessionStart",
                )
                asserts.assert_in(
                    remote_zone_id,
                    start_event.triggeredZones,
                    f"Remote zone {remote_zone_id} not found in triggeredZones {start_event.triggeredZones}",
                )
            else:
                log.info("CI mode: skipping blocking event wait in Steps 5 & 6")
                self.step(6)

            self.step(7)
            if self.matter_test_config.pipe_name:
                context_to_send = supported_contexts[0] if supported_contexts else None
                if context_to_send is not None:
                    self.write_to_app_pipe(
                        {
                            "Name": "AvAnalysisPerceivedContext",
                            "NewContexts": [
                                {
                                    "NamespaceId": context_to_send.namespaceID,
                                    "Tag": context_to_send.tag,
                                    "IdentifiedContextId": 1,
                                }
                            ],
                            "SourceNodeId": remote_node_id,
                        }
                    )
            if self.matter_test_config.pipe_name or not self.is_ci:
                perceived_event = event_callback.wait_for_event_report(
                    cluster.Events.PerceivedContext, timeout_sec=30
                )
                log.info("PerceivedContext event: %s", perceived_event)
                asserts.assert_is_not_none(
                    perceived_event, "Expected PerceivedContext event"
                )
                asserts.assert_is_not_none(
                    perceived_event.sourceNodeId,
                    "SourceNodeId must not be None in PerceivedContext",
                )
                asserts.assert_equal(
                    perceived_event.sourceNodeId,
                    remote_node_id,
                    "SourceNodeId mismatch in PerceivedContext",
                )
            else:
                log.info("CI mode: skipping blocking event wait in Step 7")

            self.step(8)
            if self.matter_test_config.pipe_name:
                self.write_to_app_pipe(
                    {
                        "Name": "AvAnalysisSessionEnd",
                        "SourceNodeId": remote_node_id,
                    }
                )
            if self.matter_test_config.pipe_name or not self.is_ci:
                end_event = event_callback.wait_for_event_report(
                    cluster.Events.AnalysisSessionEnd, timeout_sec=30
                )
                log.info("AnalysisSessionEnd event: %s", end_event)
                asserts.assert_is_not_none(end_event, "Expected AnalysisSessionEnd event")
                asserts.assert_is_not_none(
                    end_event.sourceNodeId,
                    "SourceNodeId must not be None in AnalysisSessionEnd",
                )
                asserts.assert_equal(
                    end_event.sourceNodeId,
                    remote_node_id,
                    "SourceNodeId mismatch in AnalysisSessionEnd",
                )
            else:
                log.info("CI mode: skipping blocking event wait in Step 8")
        finally:
            if event_callback:
                event_callback.cancel()
            cleanup_error = None
            if stream_id is not None:
                try:
                    await self.send_remove_analysis_stream_cmd(
                        endpoint, analysis_stream_id=stream_id
                    )
                except Exception as e:
                    log.info("Cleanup RemoveAnalysisStream: %s", e)
                    if cleanup_error is None:
                        cleanup_error = e
            try:
                await self.send_disable_context_triggers_cmd(
                    endpoint, context_triggers=NullValue
                )
            except Exception as e:
                log.info("Cleanup DisableContextTriggers: %s", e)
                if cleanup_error is None:
                    cleanup_error = e
            if remote_zone_id is not None:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=zone_cluster.Commands.RemoveZone(zoneID=remote_zone_id),
                    )
                except Exception as e:
                    log.info("Cleanup RemoveZone: %s", e)
                    if cleanup_error is None:
                        cleanup_error = e
            if cleanup_error is not None and sys.exc_info()[0] is None:
                raise cleanup_error


if __name__ == "__main__":
    default_matter_test_main()
