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
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc --camera-test-audiosrc
#     script-args: >
#       --storage-path admin_storage.json
#       --string-arg th_server_app_path:${PUSH_AV_SERVER}
#       --string-arg host_ip:localhost
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --timeout 300
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess, SupportedIngestInterface
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVST_2_15(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_15(self) -> str:
        """Test case description."""
        return "[TC-PAVST-2.15] Validate UpdateMotionZoneOptions command with Server as DUT - PROVISIONAL"

    def pics_TC_PAVST_2_15(self) -> list[str]:
        """Required PICS for this test case."""
        return ["PAVST.S", "PAVST.S.F00", "ZONEMGMT.S", "AVSM.S"]

    @async_test_body
    async def setup_class(self) -> None:
        """Set up test class resources including mock push AV server."""
        self.tlsEndpointId: int | None = None
        th_server_app = self.user_params.get("th_server_app_path", None)
        self.server = PushAvServerProcess(server_path=th_server_app)
        self.server.start(
            expected_output="Running on https://0.0.0.0:1234",
            timeout=30,
        )
        super().setup_class()

    def teardown_class(self) -> None:
        """Tear down test class resources."""
        if self.server is not None:
            self.server.terminate()
        super().teardown_class()

    @async_test_body
    async def teardown_test(self) -> None:
        """Clean up per-test resources."""
        tls_endpoint_id = getattr(self, "tlsEndpointId", None)
        if tls_endpoint_id is not None:
            await self.postcondition_remove_tls_endpoint(tls_endpoint_id)
        super().teardown_test()

    def steps_TC_PAVST_2_15(self) -> list[TestStep]:
        """Test steps definition."""
        return [
            TestStep("precondition", "Commissioning and Zone Setup", is_commissioning=True),
            TestStep(1, "TH1 allocates a PushAV transport with TriggerType = Motion.",
                     "Verify successful allocation. Store ConnectionID as aConnectionID."),
            TestStep(2, "TH1 Reads MaxZones attribute from Zone Management cluster", "Store value as aMaxZones."),
            TestStep(3, "TH1 sends the UpdateMotionZoneOptions command with ConnectionID != aConnectionID.", "DUT responds with NOT_FOUND."),
            TestStep(4, "TH2 sends the UpdateMotionZoneOptions command with ConnectionID = aConnectionID.",
                     "DUT responds with NOT_FOUND (cross-fabric)."),
            TestStep(5, "TH1 sends the UpdateMotionZoneOptions command with ConnectionID = aConnectionID and duplicate zones.",
                     "DUT responds with ALREADY_EXISTS."),
            TestStep(6, "TH1 sends the UpdateMotionZoneOptions command with aMaxZones + 1 valid Zone IDs.",
                     "DUT responds with DYNAMIC_CONSTRAINT_ERROR."),
            TestStep(7, "TH1 sends the UpdateMotionZoneOptions command with invalid ZoneID.", "DUT responds with InvalidZone."),
            TestStep(8, "TH1 sends the UpdateMotionZoneOptions command with empty list [].", "DUT responds with SUCCESS."),
            TestStep(9, "TH1 sends FindTransport command for aConnectionID.", "Verify MotionZones is empty."),
            TestStep(
                10, "TH1 sends the UpdateMotionZoneOptions command with MotionZones = [aZoneID1, aZoneID2].", "DUT responds with SUCCESS."),
            TestStep(11, "TH1 sends FindTransport command for aConnectionID.", "Verify MotionZones matches updated zones."),
            TestStep(12, "If PERZONESENS is False, TH1 sends command with invalid MotionSensitivity (11).",
                     "DUT responds with CONSTRAINT_ERROR."),
            TestStep(13, "If PERZONESENS is False, TH1 sends command with valid MotionSensitivity (5).", "DUT responds with SUCCESS."),
            TestStep(14, "If PERZONESENS is False, TH1 sends FindTransport.", "Verify MotionSensitivity is 5."),
            TestStep(15, "If PERZONESENS is True, TH1 sends command with MotionSensitivity.", "DUT responds with INVALID_COMMAND."),
        ]

    async def send_update_motion_zone_options(self, endpoint: int, connectionID: int, motionZones=None, motionSensitivity=None, expected_status: Status = Status.Success, expected_cluster_status=None, devCtrl=None) -> Status | int:
        """Helper to send UpdateMotionZoneOptions and validate response."""
        pvcluster = Clusters.PushAvStreamTransport
        dev_ctrl = devCtrl if devCtrl is not None else self.default_controller

        args = {"connectionID": connectionID}
        if motionZones is not None:
            args["motionZones"] = motionZones
        if motionSensitivity is not None:
            args["motionSensitivity"] = motionSensitivity

        try:
            # Note: UpdateMotionZoneOptions might not be generated in the SDK yet.
            # This call will fail at runtime if the command is missing in the SDK.
            cmd = pvcluster.Commands.UpdateMotionZoneOptions(args)
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, dev_ctrl=dev_ctrl)
            asserts.assert_is_none(
                expected_cluster_status,
                f"Expected cluster status {expected_cluster_status} but command succeeded",
            )
            asserts.assert_equal(expected_status, Status.Success, "Expected failure but succeeded")
            return Status.Success
        except InteractionModelError as e:
            if expected_cluster_status is not None:
                asserts.assert_equal(e.clusterStatus, expected_cluster_status, "Cluster status mismatch")
                return e.clusterStatus
            asserts.assert_equal(e.status, expected_status, "Status mismatch")
            return e.status
        except AttributeError:
            # Fallback/Log error if the command is indeed missing in the generated SDK
            log.error("UpdateMotionZoneOptions command not found in SDK")
            raise

    @run_if_endpoint_matches(lambda wildcard, endpoint: has_cluster(Clusters.PushAvStreamTransport)(wildcard, endpoint) and has_cluster(Clusters.ZoneManagement)(wildcard, endpoint) and has_cluster(Clusters.CameraAvStreamManagement)(wildcard, endpoint))
    async def test_TC_PAVST_2_15(self) -> None:
        """Run TC-PAVST-2.15 test."""
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        zmcluster = Clusters.ZoneManagement
        zmattr = Clusters.ZoneManagement.Attributes

        # Precondition: Commissioning and Zone Setup
        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        self.tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(
            server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf)

        # Read FeatureMap of PushAvStreamTransport
        pvFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.FeatureMap)
        self.perZoneSenseSupported = (pvFeatureMap & pvcluster.Bitmaps.Feature.kPerZoneSensitivity) != 0

        # Read FeatureMap of ZoneManagement
        zmFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmattr.FeatureMap)
        twoDCartSupported = zmFeatureMap & zmcluster.Bitmaps.Feature.kTwoDimensionalCartesianZone
        userDefinedSupported = zmFeatureMap & zmcluster.Bitmaps.Feature.kUserDefined

        # Read existing zones
        aZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmattr.Zones)

        aZoneID1 = None
        aZoneID2 = None

        if twoDCartSupported and userDefinedSupported:
            # Create Zone 1
            zoneVertices = [
                zmcluster.Structs.TwoDCartesianVertexStruct(x=10, y=10),
                zmcluster.Structs.TwoDCartesianVertexStruct(x=20, y=10),
                zmcluster.Structs.TwoDCartesianVertexStruct(x=20, y=20),
                zmcluster.Structs.TwoDCartesianVertexStruct(x=10, y=20)
            ]
            zoneToCreate = zmcluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=zmcluster.Enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=zmcluster.Commands.CreateTwoDCartesianZone(zone=zoneToCreate))
            aZoneID1 = cmdResponse.zoneID

            # Create Zone 2
            zoneToCreate = zmcluster.Structs.TwoDCartesianZoneStruct(
                name="Zone2", use=zmcluster.Enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#FF00FF")
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=zmcluster.Commands.CreateTwoDCartesianZone(zone=zoneToCreate))
            aZoneID2 = cmdResponse.zoneID
        else:
            motion_zones = [z for z in aZones if z.use == zmcluster.Enums.ZoneUseEnum.kMotion]
            asserts.assert_greater_equal(
                len(motion_zones), 2, "Test requires at least 2 pre-existing motion zones if UserDefined is not supported")
            aZoneID1 = motion_zones[0].zoneID
            aZoneID2 = motion_zones[1].zoneID

        # Clean up existing transports
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(status, Status.Success, "Cleanup of transports failed")

        # Step 1: Allocate transport with Motion trigger
        self.step(1)
        # Allocate streams first
        await self.allocate_one_video_stream()
        await self.allocate_one_audio_stream()

        # Allocate transport with Motion trigger
        # We need valid MotionZones to allocate with Motion trigger
        initZoneList = [{"zone": aZoneID1, "sensitivity": 4}]
        triggerOptions = {
            "triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
            "motionZones": initZoneList,
            "motionTimeControl": {"initialDuration": 5, "augmentationDuration": 2, "maxDuration": 10, "blindDuration": 1}
        }
        status = await self.allocate_one_pushav_transport(
            endpoint,
            trigger_Options=triggerOptions,
            tlsEndPoint=self.tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success, "Push AV Transport allocation failed")

        # Get connection ID
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        asserts.assert_equal(len(transport_configs), 1, "TransportConfigurations must be 1")
        aConnectionID = transport_configs[0].connectionID

        # Step 2: Read MaxZones
        self.step(2)
        aMaxZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmattr.MaxZones)
        log.info("aMaxZones: %s", aMaxZones)

        # Step 3: Update with wrong ConnectionID
        self.step(3)
        wrong_connection_id = aConnectionID + 100
        await self.send_update_motion_zone_options(
            endpoint, wrong_connection_id, expected_status=Status.NotFound)

        # Step 4: TH2 sends command (cross-fabric)
        self.step(4)
        th2 = await self.psvt_create_test_harness_controller()
        await self.send_update_motion_zone_options(
            endpoint, aConnectionID, expected_status=Status.NotFound, devCtrl=th2)
        # Clean up TH2 fabric
        resp = await self.psvt_remove_current_fabric(th2)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk,
            "Expected removal of TH2's fabric to succeed")

        # Step 5: Duplicate zones
        self.step(5)
        duplicate_zones = [
            {"zone": aZoneID1, "sensitivity": 4},
            {"zone": aZoneID1, "sensitivity": 4}
        ]
        await self.send_update_motion_zone_options(
            endpoint, aConnectionID, motionZones=duplicate_zones, expected_status=Status.AlreadyExists)

        # Step 6: Too many zones (aMaxZones + 1)
        self.step(6)
        unique_zone_ids = [aZoneID1, aZoneID2]
        temp_zone_ids = []
        if twoDCartSupported and userDefinedSupported:
            for i in range(len(unique_zone_ids), aMaxZones):
                zoneVertices = [
                    zmcluster.Structs.TwoDCartesianVertexStruct(x=10, y=10),
                    zmcluster.Structs.TwoDCartesianVertexStruct(x=20, y=10),
                    zmcluster.Structs.TwoDCartesianVertexStruct(x=20, y=20),
                    zmcluster.Structs.TwoDCartesianVertexStruct(x=10, y=20),
                ]
                zoneToCreate = zmcluster.Structs.TwoDCartesianZoneStruct(
                    name=f"TempZone{i}",
                    use=zmcluster.Enums.ZoneUseEnum.kMotion,
                    vertices=zoneVertices,
                    color="#00FFFF",
                )
                try:
                    cmdResponse = await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=zmcluster.Commands.CreateTwoDCartesianZone(zone=zoneToCreate),
                    )
                    temp_zone_ids.append(cmdResponse.zoneID)
                    unique_zone_ids.append(cmdResponse.zoneID)
                except InteractionModelError as e:
                    log.warning("Could not create additional temp zone: %s", e)
                    break

        # Generate unique zone IDs up to aMaxZones + 1 (with no duplicates to avoid ALREADY_EXISTS)
        next_id = max(unique_zone_ids) + 1 if unique_zone_ids else 1
        while len(unique_zone_ids) < aMaxZones + 1:
            unique_zone_ids.append(next_id)
            next_id += 1

        try:
            too_many_zones = [
                {"zone": zid, "sensitivity": 4}
                for zid in unique_zone_ids[:aMaxZones + 1]
            ]
            await self.send_update_motion_zone_options(
                endpoint,
                aConnectionID,
                motionZones=too_many_zones,
                expected_status=Status.DynamicConstraintError,
            )
        finally:
            for zid in temp_zone_ids:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=zmcluster.Commands.DeleteZone(zoneID=zid),
                    )
                except Exception as e:
                    log.warning("Failed to delete temp zone %s: %s", zid, e)

        # Step 7: Invalid ZoneID
        self.step(7)
        invalid_zone_id = 9999  # Hopefully invalid
        # Check if it is indeed not in aZones
        while any(z.zoneID == invalid_zone_id for z in aZones):
            invalid_zone_id += 1

        invalid_zones = [{"zone": invalid_zone_id, "sensitivity": 4}]
        await self.send_update_motion_zone_options(
            endpoint, aConnectionID, motionZones=invalid_zones,
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidZone)

        # Step 8: Empty list
        self.step(8)
        await self.send_update_motion_zone_options(
            endpoint, aConnectionID, motionZones=[], expected_status=Status.Success)

        # Step 9: Verify empty via FindTransport
        self.step(9)
        cmd = pvcluster.Commands.FindTransport(connectionID=aConnectionID)
        response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        asserts.assert_equal(len(response.transportConfigurations), 1, "Expected 1 transport config")
        config = response.transportConfigurations[0]
        # Verify MotionZones is empty
        asserts.assert_equal(len(config.transportOptions.triggerOptions.motionZones), 0, "MotionZones should be empty")

        # Step 10: Update with valid zones
        self.step(10)
        valid_zones = [
            {"zone": aZoneID1, "sensitivity": 4},
            {"zone": aZoneID2, "sensitivity": 4}
        ]
        await self.send_update_motion_zone_options(
            endpoint, aConnectionID, motionZones=valid_zones, expected_status=Status.Success)

        # Step 11: Verify updated zones
        self.step(11)
        response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        config = response.transportConfigurations[0]
        # Verify MotionZones matches
        updated_zones = config.transportOptions.triggerOptions.motionZones
        asserts.assert_equal(len(updated_zones), 2, "Expected 2 motion zones")
        # Check that they match aZoneID1 and aZoneID2
        zone_ids = [z.zone for z in updated_zones]
        asserts.assert_true(aZoneID1 in zone_ids, f"Zone {aZoneID1} not in updated zones")
        asserts.assert_true(aZoneID2 in zone_ids, f"Zone {aZoneID2} not in updated zones")

        # Step 12: Sensitivity tests (PERZONESENS == False)
        if not self.perZoneSenseSupported:
            self.step(12)
            await self.send_update_motion_zone_options(
                endpoint, aConnectionID, motionSensitivity=11, expected_status=Status.ConstraintError)
        else:
            self.skip_step(12)

        # Step 13: Valid Sensitivity (PERZONESENS == False)
        if not self.perZoneSenseSupported:
            self.step(13)
            await self.send_update_motion_zone_options(
                endpoint, aConnectionID, motionSensitivity=5, expected_status=Status.Success)
        else:
            self.skip_step(13)

        # Step 14: Verify Sensitivity (PERZONESENS == False)
        if not self.perZoneSenseSupported:
            self.step(14)
            response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            config = response.transportConfigurations[0]
            asserts.assert_equal(config.transportOptions.triggerOptions.motionSensitivity, 5, "Expected motionSensitivity to be 5")
        else:
            self.skip_step(14)

        # Step 15: Sensitivity test (PERZONESENS == True)
        if self.perZoneSenseSupported:
            self.step(15)
            await self.send_update_motion_zone_options(
                endpoint, aConnectionID, motionSensitivity=5, expected_status=Status.InvalidCommand)
        else:
            self.skip_step(15)


if __name__ == "__main__":
    default_matter_test_main()
