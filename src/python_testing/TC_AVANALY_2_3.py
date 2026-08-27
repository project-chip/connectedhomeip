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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_3(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_3(self) -> str:
        return "[TC-AVANALY-2.3] EnableContextTriggers and DisableContextTriggers functionality with local context."

    def steps_TC_AVANALY_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Test Step 2"),
            TestStep(3, "Test Step 3"),
            TestStep(4, "Test Step 4"),
            TestStep(5, "Test Step 5"),
            TestStep(6, "Test Step 6"),
            TestStep(7, "Test Step 7"),
            TestStep(8, "Test Step 8"),
            TestStep(9, "Test Step 9"),
            TestStep(10, "Test Step 10"),
            TestStep(11, "Test Step 11"),
            TestStep(12, "Test Step 12"),
            TestStep(13, "Test Step 13"),
            TestStep(14, "Test Step 14"),
        ]

    def pics_TC_AVANALY_2_3(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_feature(Clusters.AvAnalysis,
                                         Clusters.AvAnalysis.Bitmaps.Feature.kLocalContextDetection)) 
    async def test_TC_AVANALY_2_3(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        structs = cluster.Structs
        endpoint = self.get_endpoint()
        
        feature_map = await self.read_avanaly_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_perzonedetect = (feature_map & cluster.Bitmaps.Feature.kPerZoneContextDetection) != 0

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        supported_ambient_contexts_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)

        zones = []
        zoneIDs = []
        if self.has_feature_perzonedetect:
            self.step(3)
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
        else:
            self.skip_step(3)        
        
        self.step(4)
        # Send a context trigger with an invalid context (namespace), ensure CONSTRAINT_ERROR
        semantic_tag = Globals.Structs.SemanticTagStruct(
            mfgCode = NullValue,
            namespaceID = 0x47,
            tag = 0x01,
            label = None)
        
        # set the ZoneIDs to None or Null depending on feature setting
        invalid_context_zoneID = NullValue if self.has_feature_perzonedetect else None
        invalid_context_triggers = [structs.ContextTriggerStruct(context = semantic_tag, zoneIDs = invalid_context_zoneID)]
        await self.send_enable_context_triggers_command(endpoint, invalid_context_triggers, expected_status=Status.ConstraintError)

        self.step(5)
        # If we have ZoneIDs, try one not in the list
        if len(zoneIDs) >= 1:
            invalid_zone_context_triggers = []
            invalid_zone_context_trigger = structs.ContextTriggerStruct(context = supported_ambient_contexts_dut[0], zoneIDs = [zoneIDs[-1] + 1])
            invalid_zone_context_triggers.append(invalid_zone_context_trigger)
            
            await self.send_enable_context_triggers_command(endpoint, invalid_zone_context_triggers)       
        
        self.step(6)
        # Send an enable with the first from the set of available triggers
        valid_context_triggers = []
        
        # Set ZoneIDs to None if no feature, Null if feature and no zone IDs, or the first zoneID if we have those
        valid_context_zoneID = None
        if self.has_feature_perzonedetect:
            valid_context_zoneID = zoneIDs[0] if len(zoneIDs) >= 1 else NullValue

        context_trigger = structs.ContextTriggerStruct(context = supported_ambient_contexts_dut[0], zoneIDs = [valid_context_zoneID])
        valid_context_triggers.append(context_trigger)
            
        await self.send_enable_context_triggers_command(endpoint, valid_context_triggers)
            
        self.step(7)
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(valid_context_triggers, active_ambient_context_triggers_dut, "Active triggers should equate to the enabled triggers.")

        self.step(8)
        # Skip zones steps for now

        self.step(9)
        full_set_of_context_triggers = []
        for ambient_context in supported_ambient_contexts_dut:
            zoneID = NullValue if self.has_feature_perzonedetect else None
            context_trigger = structs.ContextTriggerStruct(context = ambient_context, zoneIDs = zoneID)
            full_set_of_context_triggers.append(context_trigger)

        await self.send_enable_context_triggers_command(endpoint, NullValue)
        
        self.step(10)
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(full_set_of_context_triggers, active_ambient_context_triggers_dut, "Active triggers should equate to the full set of supported contexts.")

        self.step(11)
        # Disable the first item in the full set, as we've set all contexts all zones (Null) as active, we have to have zones as null.
        # First provide a Zone, this should have a DynamicConstraint error.  Only do this if we have zoneIDs
        if len(zoneIDs) >= 1:
            await self.send_disable_context_triggers_command(endpoint, valid_context_triggers, expected_status = Status.DynamicConstraintError)
        
        valid_context_triggers[0].zoneIDs = NullValue
        await self.send_disable_context_triggers_command(endpoint, valid_context_triggers)     
                
        self.step(12)
        del full_set_of_context_triggers[0]

        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(full_set_of_context_triggers, active_ambient_context_triggers_dut, "Active triggers should equate to the reduced set of supported contexts.")
        
        self.step(13)
        await self.send_disable_context_triggers_command(endpoint, NullValue)
        
        self.step(14)
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(len(active_ambient_context_triggers_dut), 0, "Active triggers should be empty.")
        

if __name__ == "__main__":
    default_matter_test_main()
