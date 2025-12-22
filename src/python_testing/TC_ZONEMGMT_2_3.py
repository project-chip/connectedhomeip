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

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ZONEMGMT_2_3(MatterBaseTest):
    def desc_TC_ZONEMGMT_2_3(self) -> str:
        return "[TC-ZONEMGMT-2.3] Verify UpdateTwoDCartesianZone command with Server as DUT"

    def pics_TC_ZONEMGMT_2_3(self):
        return ["ZONEMGMT.S"]

    def steps_TC_ZONEMGMT_2_3(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command with",
                     " valid parameters",
                     "Verify that the DUT response contains a new zoneId and the corresponding zone information matches."),
            TestStep("2a", "TH sends UpdateTwoDCartesianZone command with ZoneID set to the created ZoneID in step 2 and zone field set to Zone1_Updated ",
                     "Verify that the DUT responds with a Success status code and the updated zone information matches."),
            TestStep("3", "TH sends UpdateTwoDCartesianZone command with ZoneID that does not exist in Zones attribute",
                     "Verify that the DUT responds with a NotFound status code"),
            TestStep("4", "TH retrieves a zone whose ZoneSource is Mfg, if it exists and attempts to update that zone",
                     "Verify that the DUT responds with a ConstraintError status code"),
            TestStep("5", "TH sends UpdateTwoDCartesianZone command with any X or Y value of zone vertices set to TwoDCartesianMax + 1",
                     "Verify that the DUT responds with a DynamicConstraintError status code."),
            TestStep("6", "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command with",
                     " new valid parameters",
                     "Verify that the DUT response contains a new zoneId and the corresponding zone information matches."),
            TestStep("6a", "TH sends the UpdateTwoDCartesianZone command with the same zoneID  as step 2a(ZoneID1) and zone vertices as  in step 6",
                     "Verify that the DUT responds with an AlreadyExists status code."),
            TestStep("7", "TH sends the UpdateTwoDCartesianZone command with the zone vertices forming a self-intersecting polygonal shape",
                     "Verify that the DUT responds with a DynamicConstraintError status code."),
            TestStep("8", "TH reads Zones attribute and sends RemoveZone command for all reviously created zones",
                     "Verify that the DUT responds with a Success status code for each command."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ZoneManagement) and
                             has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_ZONEMGMT_2_3(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ZoneManagement
        attr = Clusters.ZoneManagement.Attributes
        commands = Clusters.ZoneManagement.Commands
        enums = Clusters.ZoneManagement.Enums

        self.step("1")
        # Commission DUT - already done
        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.twoDCartSupported = aFeatureMap & cluster.Bitmaps.Feature.kTwoDimensionalCartesianZone
        self.userDefinedSupported = aFeatureMap & cluster.Bitmaps.Feature.kUserDefined
        self.focusZonesSupported = aFeatureMap & cluster.Bitmaps.Feature.kFocusZones
        self.perZoneSenseSupported = aFeatureMap & cluster.Bitmaps.Feature.kPerZoneSensitivity

        self.step("2")
        if self.twoDCartSupported and self.userDefinedSupported:
            # Fetch the zones attribute before calling Create
            zonesBeforeCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )

            # Form the Create request and send
            zoneVertices = [
                cluster.Structs.TwoDCartesianVertexStruct(10, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 20),
                cluster.Structs.TwoDCartesianVertexStruct(10, 20)
            ]
            zoneToCreate = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            log.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
            asserts.assert_equal(type(cmdResponse), commands.CreateTwoDCartesianZoneResponse,
                                 "Incorrect response type")
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianCmdResponse does not contain ZoneID")
            zoneID1 = cmdResponse.zoneID

            # Check that zoneID1 did not exist before
            matchingZone = next(
                (z for z in zonesBeforeCreate if z.zoneID == zoneID1), None)
            asserts.assert_is_none(matchingZone, "fZone with {zoneID1} already existed in Zones")

            # Fetch the zones attribute after Create
            zonesAfterCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info(f"Rx'd Zones: {zonesAfterCreate}")

            matchingZone = next(
                (z for z in zonesAfterCreate if z.zoneID == zoneID1), None)
            asserts.assert_is_not_none(matchingZone, "fZone with {zoneID1} not found")
            asserts.assert_equal(matchingZone.zoneType, enums.ZoneTypeEnum.kTwoDCARTZone,
                                 "ZoneType of created Zone is not of type TwoDCartZone")
            asserts.assert_equal(matchingZone.zoneSource, enums.ZoneSourceEnum.kUser,
                                 "ZoneSource of created Zone is not of type User")
            asserts.assert_equal(matchingZone.twoDCartesianZone.vertices, zoneVertices,
                                 "ZoneVertices of created Zone are mismatched")

            self.step("2a")
            updatedZoneVertices = [
                cluster.Structs.TwoDCartesianVertexStruct(20, 20),
                cluster.Structs.TwoDCartesianVertexStruct(30, 20),
                cluster.Structs.TwoDCartesianVertexStruct(30, 30),
                cluster.Structs.TwoDCartesianVertexStruct(20, 30)
            ]
            zoneToUpdate = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1_Updated", use=enums.ZoneUseEnum.kMotion, vertices=updatedZoneVertices,
                color="#00FFFF")
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to update zone")
                pass
            # Fetch the zones attribute after Update
            zonesAfterUpdate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info(f"Rx'd Zones: {zonesAfterUpdate}")

            matchingZone = next(
                (z for z in zonesAfterUpdate if z.zoneID == zoneID1), None)
            asserts.assert_is_not_none(matchingZone, "fZone with {zoneID1} not found")
            asserts.assert_equal(matchingZone.zoneType, enums.ZoneTypeEnum.kTwoDCARTZone,
                                 "ZoneType of updated Zone is not of type TwoDCartZone")
            asserts.assert_equal(matchingZone.zoneSource, enums.ZoneSourceEnum.kUser,
                                 "ZoneSource of updated Zone is not of type User")
            asserts.assert_equal(matchingZone.twoDCartesianZone.name, "Zone1_Updated",
                                 "Name of updated Zone does not match")
            asserts.assert_equal(matchingZone.twoDCartesianZone.vertices, updatedZoneVertices,
                                 "ZoneVertices of created Zone are mismatched")

            self.step("3")
            # Fetch the zones attribute
            zones = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info(f"Rx'd Zones: {zones}")
            newZoneId = 0
            # Get the max zoneId
            for zone in zones:
                if newZoneId < zone.zoneID:
                    newZoneId = zone.zoneID

            # Assign a new one greater than the max
            newZoneId = newZoneId + 1
            try:
                zoneVertices = [
                    cluster.Structs.TwoDCartesianVertexStruct(10, 10),
                    cluster.Structs.TwoDCartesianVertexStruct(50, 10),
                    cluster.Structs.TwoDCartesianVertexStruct(50, 50),
                    cluster.Structs.TwoDCartesianVertexStruct(10, 50)
                ]
                zoneToUpdate = cluster.Structs.TwoDCartesianZoneStruct(
                    name="Zone_3", use=enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                    color="#00FFFF")
                updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                    zoneID=newZoneId, zone=zoneToUpdate
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting NOT_FOUND due to new zone id being used")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.NotFound,
                    "Unexpected error returned expecting NOT_FOUND due to new zone id being used",
                )
                pass

            # Fetch the zones attribute
            zones = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info(f"Rx'd Zones: {zones}")
            matchingZone = next(
                (z for z in zones if z.zoneSource == enums.ZoneSourceEnum.kMfg), None)
            if matchingZone is not None:
                self.step("4")
                log.info(f"Found Mfg Zone with Id : {matchingZone.zoneID}")

                try:
                    # use previously crafted zone in step 3
                    updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                        zoneID=matchingZone.zoneID, zone=zoneToUpdate
                    )
                    await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                    asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to Mfg zone being updated by user")
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status,
                        Status.ConstraintError,
                        "Unexpected error returned expecting CONSTRAINT_ERROR due to Mfg zone being updated by user",
                    )
                    pass
            else:
                log.info("No Mfg Zone found")
                self.skip_step("4")

            self.step("5")
            # Get the TwoDCartesianMax attribute
            twoDCartesianMax = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.TwoDCartesianMax
            )
            log.info(f"Rx'd TwoDCartesianMax: {twoDCartesianMax}")

            # Form the Create request with one vertex exceeding TwoDCartesianMax and send
            zoneVertices = [
                cluster.Structs.TwoDCartesianVertexStruct(10, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 10),
                cluster.Structs.TwoDCartesianVertexStruct(twoDCartesianMax.x + 1, twoDCartesianMax.y + 1),
                cluster.Structs.TwoDCartesianVertexStruct(10, 20)
            ]
            zoneToUpdate = cluster.Structs.TwoDCartesianZoneStruct(
                name="ZoneExceedsTwoDCartMax", use=enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            try:
                updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                    zoneID=zoneID1, zone=zoneToUpdate
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to TwoDCartesianMax exceeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

            self.step("6")
            # Fetch the zones attribute before calling Create
            zonesBeforeCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )

            # Form the Create request and send
            zone2Vertices = [
                cluster.Structs.TwoDCartesianVertexStruct(100, 100),
                cluster.Structs.TwoDCartesianVertexStruct(200, 100),
                cluster.Structs.TwoDCartesianVertexStruct(200, 200),
                cluster.Structs.TwoDCartesianVertexStruct(100, 200)
            ]
            zoneToCreate = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone2", use=enums.ZoneUseEnum.kMotion, vertices=zone2Vertices,
                color="#00FFFF")
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            log.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
            asserts.assert_equal(type(cmdResponse), commands.CreateTwoDCartesianZoneResponse,
                                 "Incorrect response type")
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianCmdResponse does not contain ZoneID")
            zoneID2 = cmdResponse.zoneID

            # Check that zoneID2 did not exist before
            matchingZone = next(
                (z for z in zonesBeforeCreate if z.zoneID == zoneID2), None)
            asserts.assert_is_none(matchingZone, "fZone with {zoneID2} already existed in Zones")

            # Fetch the zones attribute after Create
            zonesAfterCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            log.info(f"Rx'd Zones: {zonesAfterCreate}")

            matchingZone = next(
                (z for z in zonesAfterCreate if z.zoneID == zoneID2), None)
            asserts.assert_is_not_none(matchingZone, "fZone with {zoneID2} not found")
            asserts.assert_equal(matchingZone.zoneType, enums.ZoneTypeEnum.kTwoDCARTZone,
                                 "ZoneType of created Zone is not of type TwoDCartZone")
            asserts.assert_equal(matchingZone.zoneSource, enums.ZoneSourceEnum.kUser,
                                 "ZoneSource of created Zone is not of type User")
            asserts.assert_equal(matchingZone.twoDCartesianZone.vertices, zone2Vertices,
                                 "ZoneVertices of created Zone are mismatched")

            self.step("6a")
            # Attempt updating ZoneID1 with Zone2Vertices from ZoneID2
            zoneToUpdate = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1_Updated", use=enums.ZoneUseEnum.kMotion, vertices=zone2Vertices,
                color="#00FFFF")
            try:
                updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                    zoneID=zoneID1, zone=zoneToUpdate
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                # TODO : Double check if this validation is for any zone other than
                # the current one
                asserts.fail("Unexpected success when expecting ALREADY_EXISTS due to duplicate zone existing")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.AlreadyExists, "Unexpected error returned when trying to update zone")
                pass

            # Test to check zone update with self intersecting shapes. Command
            # shall return DynamicConstraintError status code.
            # This step uses different self-intersecting shapes to test the
            # DUT
            self.step("7")
            # Form the self intersecting test zone polygons
            selfIntersectingVertices1 = [
                cluster.Structs.TwoDCartesianVertexStruct(100, 100),
                cluster.Structs.TwoDCartesianVertexStruct(200, 200),
                cluster.Structs.TwoDCartesianVertexStruct(200, 100),
                cluster.Structs.TwoDCartesianVertexStruct(100, 200)
            ]
            selfIntersectingVertices2 = [
                cluster.Structs.TwoDCartesianVertexStruct(100, 100),
                cluster.Structs.TwoDCartesianVertexStruct(200, 100),
                cluster.Structs.TwoDCartesianVertexStruct(200, 200),
                cluster.Structs.TwoDCartesianVertexStruct(150, 50),
                cluster.Structs.TwoDCartesianVertexStruct(100, 200)
            ]
            selfIntersectingVertices3 = [
                cluster.Structs.TwoDCartesianVertexStruct(20, 20),
                cluster.Structs.TwoDCartesianVertexStruct(100, 20),
                cluster.Structs.TwoDCartesianVertexStruct(70, 20),
                cluster.Structs.TwoDCartesianVertexStruct(100, 10),
                cluster.Structs.TwoDCartesianVertexStruct(10, 10)
            ]
            # Degenerate zones with duplicate vertices and subsequent
            # self-intersection
            selfIntersectingVertices4 = [
                cluster.Structs.TwoDCartesianVertexStruct(0, 0),
                cluster.Structs.TwoDCartesianVertexStruct(1, 0),
                cluster.Structs.TwoDCartesianVertexStruct(2, 0),
                cluster.Structs.TwoDCartesianVertexStruct(2, 1),
                cluster.Structs.TwoDCartesianVertexStruct(1, 0),
                cluster.Structs.TwoDCartesianVertexStruct(0, 1),
            ]
            selfIntersectingVertices5 = [
                cluster.Structs.TwoDCartesianVertexStruct(0, 0),
                cluster.Structs.TwoDCartesianVertexStruct(2, 0),
                cluster.Structs.TwoDCartesianVertexStruct(2, 1),
                cluster.Structs.TwoDCartesianVertexStruct(2, 0),
                cluster.Structs.TwoDCartesianVertexStruct(1, 0),
            ]
            selfIntersectingVertices6 = [
                cluster.Structs.TwoDCartesianVertexStruct(0, 0),
                cluster.Structs.TwoDCartesianVertexStruct(1, 0),
                cluster.Structs.TwoDCartesianVertexStruct(0, 0),
                cluster.Structs.TwoDCartesianVertexStruct(0, 1)
            ]
            # Degenerate zone with 3 vertices
            selfIntersectingVertices7 = [
                cluster.Structs.TwoDCartesianVertexStruct(0, 0),
                cluster.Structs.TwoDCartesianVertexStruct(2, 0),
                cluster.Structs.TwoDCartesianVertexStruct(1, 0)
            ]
            zoneToUpdate1 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices1,
                color="#00FFFF")
            zoneToUpdate2 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices2,
                color="#00FFFF")
            zoneToUpdate3 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices3,
                color="#00FFFF")
            zoneToUpdate4 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices4,
                color="#00FFFF")
            zoneToUpdate5 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices5,
                color="#00FFFF")
            zoneToUpdate6 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices6,
                color="#00FFFF")
            zoneToUpdate7 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices7,
                color="#00FFFF")

            # Update and send the command with selfIntersectingVertices1
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate1
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Update and send the command with selfIntersectingVertices2
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate2
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

            # Update and send the command with selfIntersectingVertices3
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate3
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

            # Update and send the command with selfIntersectingVertices4
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate4
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

            # Update and send the command with selfIntersectingVertices5
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate5
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

            # Update and send the command with selfIntersectingVertices6
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate6
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

            # Update and send the command with selfIntersectingVertices7
            updateTwoDCartesianCmd = commands.UpdateTwoDCartesianZone(
                zoneID=zoneID1, zone=zoneToUpdate7
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=updateTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to update zone")
                pass

        else:
            log.info("TwoDCartZone or UserDefinedZones Feature not supported. Test steps skipped")
            self.skip_step("2")
            self.skip_step("2a")
            self.skip_step("3")
            self.skip_step("4")
            self.skip_step("5")
            self.skip_step("6")
            self.skip_step("7")

        # Test to remove all created zones
        self.step("8")
        zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Zones
        )
        log.info(f"Rx'd Zones: {zones}")
        for zone in zones:
            try:
                log.info(f"Removing zone with Id : {zone.zoneID}")
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=zone.zoneID))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to remove zone")
                pass


if __name__ == "__main__":
    default_matter_test_main()
