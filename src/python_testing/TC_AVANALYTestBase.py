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
                             
    async def send_enable_context_triggers_command(self, endpoint, context_triggers, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.AvAnalysis.Commands.EnableContextTriggers(
                contextTriggers=context_triggers),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned on enabling context triggers")
            
    async def send_disable_context_triggers_command(self, endpoint, context_triggers, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.AvAnalysis.Commands.DisableContextTriggers(
                contextTriggers=context_triggers),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned on disabling context triggers")
            
    async def get_zoneids_from_zone_management(self, endpoint) -> [int]:
        zones = []
        zoneIDs = []
        
        # pull zone ids from Zone Management, if none, try to create 
        clusterZM = Clusters.Objects.ZoneManagement
        attributesZM = clusterZM.Attributes         
        aFeatureMapZM = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=clusterZM, attribute=attributesZM.FeatureMap)
        twoDCartSupported = aFeatureMapZM & clusterZM.Bitmaps.Feature.kTwoDimensionalCartesianZone
        userDefinedSupported = aFeatureMapZM & clusterZM.Bitmaps.Feature.kUserDefined
            
        zones = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=clusterZM, attribute=attributesZM.Zones)
            
        if len(zones) >= 1:
            for zone in zones:
                zoneIDs.append(zone.zoneID)
        else:
            # None on the DUT, create one, but only if possible
            if twoDCartSupported and userDefinedSupported:
                # Form the Create request and send
                zoneVertices = [
                    clusterZM.Structs.TwoDCartesianVertexStruct(10, 10),
                    clusterZM.Structs.TwoDCartesianVertexStruct(20, 10),
                    clusterZM.Structs.TwoDCartesianVertexStruct(20, 20),
                    clusterZM.Structs.TwoDCartesianVertexStruct(10, 20)
                ]
                zoneToCreate = clusterZM.Structs.TwoDCartesianZoneStruct(
                    name="Zone1", use=clusterZM.Enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
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
