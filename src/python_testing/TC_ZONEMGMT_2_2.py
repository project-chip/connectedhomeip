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
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches

log = logging.getLogger(__name__)


class TC_ZONEMGMT_2_2(MatterBaseTest):
    def desc_TC_ZONEMGMT_2_2(self) -> str:
        return "[TC-ZONEMGMT-2.2] Verify CreateTwoDCartesianZone and RemoveZone commands with Server as DUT"

    def pics_TC_ZONEMGMT_2_2(self):
        return ["ZONEMGMT.S"]

    def steps_TC_ZONEMGMT_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command with",
                     " valid parameters",
                     "Verify that the DUT response contains a new zoneId and the corresponding zone information matches."),
            TestStep("2a", "TH sends RemoveZone command with ZoneID set to the created ZoneID in step 2 ",
                     "Verify that the DUT responds with a Success status code."),
            TestStep("3", "TH sends CreateTwoDCartesianZone command with valid non-duplicated Zone information _MaxUserDefinedZones_ + 1 times",
                     "Verify that the DUT responds with a ResourceExhausted status code at the _MaxUserDefinedZones_ + 1 th command."),
            TestStep("3a", "TH sends RemoveZone command for each ZoneID created in step 3 ",
                     "Verify that the DUT responds with a Success status code."),
            TestStep("4", "TH sends CreateTwoDCartesianZone command with valid Zone information and ZoneUse set to Focus",
                     "If DUT supports FocusZones verify that the DUT responds with a CreateTwoDCartesianZoneResponse command",
                     "Othersie DUT responds with ConstraintError status code"),
            TestStep("5", "TH sends CreateTwoDCartesianZone command with any X or Y value of zone vertices set to TwoDCartesianMax + 1",
                     "Verify that the DUT responds with a DynamicConstraintError status code."),
            TestStep("6", "TH sends the same CreateTwoDCartesianZone command with valid parameters 2 times",
                     "Verify that the first DUT response contains a new zoneId and the second command returns AlreadyExists status code."),
            TestStep("7", "TH sends the CreateTwoDCartesianZone command with the zone vertices forming a self-intersecting polygonal shape",
                     "Verify that the DUT responds with a DynamicConstraintError status code."),
            TestStep("8", "TH reads Zones attribute and sends RemoveZone command for all reviously created zones",
                     "Verify that the DUT responds with a Success status code for each command."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ZoneManagement) and
                             has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_ZONEMGMT_2_2(self):
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
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=zoneID1))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to remove zone")
                pass

            self.step("3")
            maxUserDefinedZones = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxUserDefinedZones
            )
            log.info(f"Rx'd MaxUserDefinedZones: {maxUserDefinedZones}")

            numOfZones = 0
            zoneIDs = []
            # Create max number of user-defined zones
            while numOfZones < maxUserDefinedZones:
                zoneVertices = [
                    cluster.Structs.TwoDCartesianVertexStruct(10+numOfZones, 10),
                    cluster.Structs.TwoDCartesianVertexStruct(50, 10),
                    cluster.Structs.TwoDCartesianVertexStruct(50, 50),
                    cluster.Structs.TwoDCartesianVertexStruct(10+numOfZones, 50)
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
                zoneIDs.append(cmdResponse.zoneID)
                numOfZones = numOfZones + 1

            # Try creating an extra zone; Expected to receive ResourceExhausted
            # error
            try:
                zoneVertices = [
                    cluster.Structs.TwoDCartesianVertexStruct(10+numOfZones, 10),
                    cluster.Structs.TwoDCartesianVertexStruct(50, 10),
                    cluster.Structs.TwoDCartesianVertexStruct(50, 50),
                    cluster.Structs.TwoDCartesianVertexStruct(10+numOfZones, 50)
                ]
                zoneToCreate = cluster.Structs.TwoDCartesianZoneStruct(
                    name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                    color="#00FFFF")
                createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                    zone=zoneToCreate
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting RESOURCE_EXHAUSTED due to Max UserDefined zones already created")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.ResourceExhausted,
                    "Unexpected error returned expecting RESOURCE_EXHAUSTED due to Max UserDefined zones already created",
                )
                pass

            # Remove all created zones
            self.step("3a")
            for createdZoneId in zoneIDs:
                try:
                    log.info(f"Removing Zone with ID = {createdZoneId}")
                    await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=createdZoneId))
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to remove zone")
                    pass

            # Test to check the creation of Focus zones, if supported
            self.step("4")
            # Form the Create request and send
            zoneVertices = [
                cluster.Structs.TwoDCartesianVertexStruct(10, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 20),
                cluster.Structs.TwoDCartesianVertexStruct(10, 20)
            ]
            zoneToCreate = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone3", use=enums.ZoneUseEnum.kFocus, vertices=zoneVertices,
                color="#00FFFF")

            if self.focusZonesSupported:
                createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                    zone=zoneToCreate
                )
                cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                log.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
                asserts.assert_equal(type(cmdResponse), commands.CreateTwoDCartesianZoneResponse,
                                     "Incorrect response type")
            else:
                # DUT does not support FocusZones
                try:
                    createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                        zone=zoneToCreate
                    )
                    await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                    asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to Focus zones not supported")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned when trying to create zone")
                    pass
            # Test to check vertices with coordinates beyond TwoDCartesianMax
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
            zoneToCreate = cluster.Structs.TwoDCartesianZoneStruct(
                name="ZoneExceedsTwoDCartMax", use=enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            try:
                createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                    zone=zoneToCreate
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to TwoDCartesianMax exceeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Test to check duplicate zone creation returning AlreadyExists
            # status code
            self.step("6")
            # Form the Create request and send
            zoneVertices = [
                cluster.Structs.TwoDCartesianVertexStruct(50, 50),
                cluster.Structs.TwoDCartesianVertexStruct(100, 50),
                cluster.Structs.TwoDCartesianVertexStruct(100, 100),
                cluster.Structs.TwoDCartesianVertexStruct(50, 100)
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

            # Send the same command again
            try:
                createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                    zone=zoneToCreate
                )
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting ALREADY_EXISTS due to duplicate zone creation")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.AlreadyExists, "Unexpected error returned when trying to create zone")
                pass

            # Test to check zone creation with self intersecting shapes. Command
            # shall return DynamicConstraintError status code.
            # This step uses 2 different self-intersecting shapes to test the
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
            zoneToCreate1 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices1,
                color="#00FFFF")
            zoneToCreate2 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices2,
                color="#00FFFF")
            zoneToCreate3 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices3,
                color="#00FFFF")
            zoneToCreate4 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices4,
                color="#00FFFF")
            zoneToCreate5 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices5,
                color="#00FFFF")
            zoneToCreate6 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices6,
                color="#00FFFF")
            zoneToCreate7 = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=enums.ZoneUseEnum.kMotion, vertices=selfIntersectingVertices7,
                color="#00FFFF")

            # Create and send the command with selfIntersectingVertices1
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate1
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Create and send the command with selfIntersectingVertices2
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate2
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Create and send the command with selfIntersectingVertices3
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate3
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Create and send the command with selfIntersectingVertices4
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate4
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Create and send the command with selfIntersectingVertices5
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate5
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Create and send the command with selfIntersectingVertices6
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate6
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

            # Create and send the command with selfIntersectingVertices7
            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate7
            )
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
                asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to self intersecting vertices")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError,
                                     "Unexpected error returned when trying to create zone")
                pass

        else:
            log.info("TwoDCartZone or UserDefinedZones Feature not supported. Test steps skipped")
            self.skip_step("2")
            self.skip_step("2a")
            self.skip_step("3")
            self.skip_step("3a")
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
