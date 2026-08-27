#
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

from mobly import asserts

import matter.clusters as Clusters


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

    async def read_avanaly_features(self, endpoint):
        """Read FeatureMap attribute and populate feature flags."""
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        feature_map = await self.read_avanaly_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_lclcondetect = (feature_map & cluster.Bitmaps.Feature.kLocalContextDetection) != 0
        self.has_feature_remcondetect = (feature_map & cluster.Bitmaps.Feature.kRemoteContextDetection) != 0
        self.has_feature_perzonedetect = (feature_map & cluster.Bitmaps.Feature.kPerZoneContextDetection) != 0
        return feature_map

    async def send_enable_context_triggers_cmd(self, endpoint, context_triggers=None):
        """Send EnableContextTriggers command to the AvAnalysis cluster."""
        cmd = Clusters.Objects.AvAnalysis.Commands.EnableContextTriggers(contextTriggers=context_triggers)
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

    async def send_disable_context_triggers_cmd(self, endpoint, context_triggers=None):
        """Send DisableContextTriggers command to the AvAnalysis cluster."""
        cmd = Clusters.Objects.AvAnalysis.Commands.DisableContextTriggers(contextTriggers=context_triggers)
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

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
