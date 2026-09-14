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

from typing import Any

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status


class AVANALYTestBase:
    SPEC_MAX_COUNT_SUPPORTEDAMBIENTCONTEXTS = 50
    SPEC_MAX_COUNT_ANALYSIS_STREAMS = 255

    has_feature_lclcondetect = False
    has_feature_remcondetect = False
    has_feature_perzonedetect = False

    async def read_avanaly_attribute_expect_success(self, endpoint, attribute):
        """Read the provided attribute from the cluster instance on the provided endpoint.

        Args:
            attribute: The attribute to be read, must be a valid AvAnalysis cluster attribute
        """
        cluster = Clusters.Objects.AvAnalysis
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def check_avanaly_attribute(self, attribute, expected_value, endpoint):
        """Validate that the current instance of an attribute has the value provided

        Args:
            attribute:      The attribute to be read and validated, must be a valid AvAnalysis cluster attribute.
            expected_value: The value to be matched, assert failure if this is not equal to the current attribute value
        """
        value = await self.read_avanaly_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def send_enable_context_triggers_cmd(
        self, endpoint: int, context_triggers: Any = NullValue, expected_status: Status = Status.Success
    ) -> Any:
        """Send EnableContextTriggers command to the AvAnalysis cluster and assert expected status.

        Args:
            endpoint: The endpoint to send the command to.
            context_triggers: The value of the ContextTriggers field that is sent. Defaults to NullValue.
            expected_status: The status response to match. Defaults to Status.Success.
        """
        if context_triggers is None:
            context_triggers = NullValue
        cmd = Clusters.Objects.AvAnalysis.Commands.EnableContextTriggers(contextTriggers=context_triggers)
        try:
            resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.assert_equal(
                expected_status,
                Status.Success,
                f"Expected status {expected_status} on enabling context triggers, but command succeeded",
            )
            return resp
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                expected_status,
                f"Unexpected error returned on enabling context triggers: expected {expected_status}, got {e.status}",
            )
            return None

    # Alias for backwards compatibility
    send_enable_context_triggers_command = send_enable_context_triggers_cmd

    async def send_disable_context_triggers_cmd(
        self, endpoint: int, context_triggers: Any = NullValue, expected_status: Status = Status.Success
    ) -> Any:
        """Send DisableContextTriggers command to the AvAnalysis cluster and assert expected status.

        Args:
            endpoint: The endpoint to send the command to.
            context_triggers: The value of the ContextTriggers field that is sent. Defaults to NullValue.
            expected_status: The status response to match. Defaults to Status.Success.
        """
        if context_triggers is None:
            context_triggers = NullValue
        cmd = Clusters.Objects.AvAnalysis.Commands.DisableContextTriggers(contextTriggers=context_triggers)
        try:
            resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.assert_equal(
                expected_status,
                Status.Success,
                f"Expected status {expected_status} on disabling context triggers, but command succeeded",
            )
            return resp
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                expected_status,
                f"Unexpected error returned on disabling context triggers: expected {expected_status}, got {e.status}",
            )
            return None

    # Alias for backwards compatibility
    send_disable_context_triggers_command = send_disable_context_triggers_cmd

    async def get_zoneids_from_zone_management(self, endpoint, min_count: int = 1) -> list[int]:
        """Returns a list of ZoneIDs that are available from the ZoneManagement cluster on the target endpoint. If fewer
        than min_count are present it will attempt to create additional zones.

        Args:
            endpoint:  Target endpoint.
            min_count: Minimum number of zones required (defaults to 1).

        Returns:
            list[int]: the list of ZoneIDs that are found/created.
        """
        zoneIDs = []

        # pull zone ids from Zone Management, if not enough, try to create
        clusterZM = Clusters.Objects.ZoneManagement
        attributesZM = clusterZM.Attributes
        aFeatureMapZM = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=clusterZM, attribute=attributesZM.FeatureMap)
        twoDCartSupported = (aFeatureMapZM & clusterZM.Bitmaps.Feature.kTwoDimensionalCartesianZone) != 0
        userDefinedSupported = (aFeatureMapZM & clusterZM.Bitmaps.Feature.kUserDefined) != 0

        zones = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=clusterZM, attribute=attributesZM.Zones)
        if zones:
            for zone in zones:
                zoneIDs.append(zone.zoneID)

        # If not enough zones on the DUT, create additional zones if supported
        while len(zoneIDs) < min_count and twoDCartSupported and userDefinedSupported:
            idx = len(zoneIDs) + 1
            offset = idx * 15
            zoneVertices = [
                clusterZM.Structs.TwoDCartesianVertexStruct(offset, offset),
                clusterZM.Structs.TwoDCartesianVertexStruct(offset + 10, offset),
                clusterZM.Structs.TwoDCartesianVertexStruct(offset + 10, offset + 10),
                clusterZM.Structs.TwoDCartesianVertexStruct(offset, offset + 10)
            ]
            zoneToCreate = clusterZM.Structs.TwoDCartesianZoneStruct(
                name=f"Zone{idx}", use=clusterZM.Enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            createTwoDCartesianCmd = clusterZM.Commands.CreateTwoDCartesianZone(
                zone=zoneToCreate
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            asserts.assert_equal(type(cmdResponse), clusterZM.Commands.CreateTwoDCartesianZoneResponse,
                                 "Incorrect response type")
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianCmdResponse does not contain ZoneID")
            zoneIDs.append(cmdResponse.zoneID)

        return zoneIDs

    async def read_avanaly_features(self, endpoint):
        """Read FeatureMap attribute and populate feature flags."""
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        feature_map = await self.read_avanaly_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_lclcondetect = (feature_map & cluster.Bitmaps.Feature.kLocalContextDetection) != 0
        self.has_feature_remcondetect = (feature_map & cluster.Bitmaps.Feature.kRemoteContextDetection) != 0
        self.has_feature_perzonedetect = (feature_map & cluster.Bitmaps.Feature.kPerZoneContextDetection) != 0
        return feature_map

    async def send_establish_analysis_stream_cmd(self, endpoint, node_id):
        """Send EstablishAnalysisStream command to the AvAnalysis cluster."""
        cmd = Clusters.Objects.AvAnalysis.Commands.EstablishAnalysisStream(nodeID=node_id)
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

    async def send_activate_analysis_stream_cmd(self, endpoint, analysis_stream_id, webrtc_endpoint_id=None, pushav_endpoint_id=None):
        """Send ActivateAnalysisStream command to the AvAnalysis cluster."""
        cmd = Clusters.Objects.AvAnalysis.Commands.ActivateAnalysisStream(
            analysisStreamID=analysis_stream_id,
            webRTCEndpointID=webrtc_endpoint_id,
            pushAVEndpointID=pushav_endpoint_id
        )
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

    async def send_deactivate_analysis_stream_cmd(self, endpoint, analysis_stream_id):
        """Send DeactivateAnalysisStream command to the AvAnalysis cluster."""
        cmd = Clusters.Objects.AvAnalysis.Commands.DeactivateAnalysisStream(analysisStreamID=analysis_stream_id)
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

    async def send_remove_analysis_stream_cmd(self, endpoint, analysis_stream_id):
        """Send RemoveAnalysisStream command to the AvAnalysis cluster."""
        cmd = Clusters.Objects.AvAnalysis.Commands.RemoveAnalysisStream(analysisStreamID=analysis_stream_id)
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
