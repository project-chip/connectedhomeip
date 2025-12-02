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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/zonemgmt_2_4_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/zonemgmt_2_4_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches

logger = logging.getLogger(__name__)


class TC_ZONEMGMT_2_4(MatterBaseTest):
    def desc_TC_ZONEMGMT_2_4(self) -> str:
        return "[TC-ZONEMGMT-2.4] Verify CreateOrUpdateTrigger, RemoveTrigger and RemoveZone commands with Server as DUT"

    def pics_TC_ZONEMGMT_2_4(self):
        return ["ZONEMGMT.S"]

    def steps_TC_ZONEMGMT_2_4(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command with",
                     " valid parameters",
                     "Verify that the DUT response contains a new zoneId and the corresponding zone information matches."),
            TestStep("2a", "TH sends CreateOrUpdateTrigger command with a valid ZoneTriggerControlStruct and ZoneID set to the created ZoneID in step 2 ",
                     "Verify that the DUT responds with a Success status code.",
                     "Verify that the new ZoneTrigger is added to the Triggers list"),
            TestStep("2b", "TH reads the Triggers attribute from DUT",
                     "Verify that the response contains a list of ZoneTriggerControlStruct entries",
                     "Verify that the Trigger list contains the Trigger created in step 2a"),
            TestStep("3", "Trigger the DUT to generate ZoneTriggered event",
                     "Verify that the TH receives the ZoneTriggered event and the ZoneID matches one created in step 2",
                     "Verify that the reason is 0(Motion) and the event has priority set as INFO"),
            TestStep("3a", "Prevent DUT from detecting motion for period > InitialDuration"),
            TestStep("3b", "DUT sends the ZoneStopped event",
                     "Verify that the TH receives the ZoneStopped event and the ZoneID matches one created in step 2",
                     "Verify that the reason is 0(ActionStopped)"),
            TestStep("4", "Repeat step 3"),
            TestStep("4a", "Trigger the DUT to generate another ZoneTriggered event to increment the TriggerDetectedDuration by the AugmentationDuration"),
            TestStep("4b", "DUT sends the ZoneStopped event",
                     "Verify that the TH receives the ZoneStopped event after (InitialDuration + AugmentationDuration) and the ZoneID matches one in step 2",
                     "Verify that the reason is 0(ActionStopped)"),
            TestStep("5", "Repeat step 3"),
            TestStep("5a", "Make the DUT keep detecting motion for a period > MaxDuration"),
            TestStep("5b", "DUT sends the ZoneStopped event",
                     "Verify that the TH receives the ZoneStopped event and the ZoneID matches one created in step 2",
                     "Verify that the reason is 1(Timeout)"),
            TestStep("5c", "Trigger the DUT to generate ZoneTriggered event during BlindDuration after ZoneStopped event was received",
                     "Verify that the TH does not receive any ZoneTriggered event from DUT"),
            TestStep("6", "TH sends CreateOrUpdateTrigger command with a ZoneID that does not exist in the Zones attribute",
                     "Verify that the DUT responds with a NotFound status code."),
            TestStep("7", "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command with",
                     "ZoneUse = Privacy and other valid parameters",
                     "Verify that the DUT response contains a new zoneId and the corresponding zone information matches."),
            TestStep("7a", "TH sends CreateOrUpdateTrigger command with a ZoneID of the Privacy Zone created in step 6",
                     "Verify that the DUT responds with a ConstraintError status code."),
            TestStep("8", "TH sends RemoveZone command with ZoneID set to the created ZoneID in step 2",
                     "Verify that the DUT responds with a InvalidInState status code."),
            TestStep("8a", "TH sends RemoveTrigger command with ZoneID that does not exist in Zones attribute",
                     "Verify that the DUT responds with a NotFound status code"),
            TestStep("8b", "TH sends RemoveTrigger command with ZoneID set to the created ZoneID in step 2",
                     "Verify that the DUT responds with a Success status code."),
            TestStep("8c", "TH sends RemoveZone command with ZoneID set to the created ZoneID in step 2",
                     "Verify that the DUT responds with a Success status code."),
            TestStep("9", "TH retrieves a zone whose ZoneSource is Mfg, if it exists and attempts to remove that zone",
                     "Verify that the DUT responds with a ConstraintError status code"),
            TestStep("10", "TH sends RemoveZone command with ZoneID that does not exist in Zones attribute",
                     "Verify that the DUT responds with a NotFound status code"),
        ]

    async def _trigger_motion_event(self, zone_id, prompt_msg=None):
        # CI: Use app pipe to trigger zone event.
        # Manual: User should trigger a motion event from the defined zone.
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "ZoneTriggered", "ZoneId": zone_id})
        else:
            if prompt_msg is None:
                prompt_msg = f"Press enter and immediately start motion activity in zone {zone_id}."
            self.wait_for_user_input(prompt_msg=prompt_msg)

    @run_if_endpoint_matches(has_cluster(Clusters.ZoneManagement) and
                             has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_ZONEMGMT_2_4(self):
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
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
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
            logger.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
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
            logger.info(f"Rx'd Zones: {zonesAfterCreate}")

            matchingZone = next(
                (z for z in zonesAfterCreate if z.zoneID == zoneID1), None)
            asserts.assert_is_not_none(matchingZone, "fZone with {zoneID1} not found")
            asserts.assert_equal(matchingZone.zoneType, enums.ZoneTypeEnum.kTwoDCARTZone,
                                 "ZoneType of created Zone is not of type TwoDCartZone")
            asserts.assert_equal(matchingZone.zoneSource, enums.ZoneSourceEnum.kUser,
                                 "ZoneSource of created Zone is not of type User")
            asserts.assert_equal(matchingZone.twoDCartesianZone.vertices, zoneVertices,
                                 "ZoneVertices of created Zone are mismatched")
        else:
            logging.info("TwoDCartZone or UserDefinedZones Feature not supported. Test steps skipped")
            self.skip_step("2")

        self.step("2a")
        # Fetch the Triggers before creation
        triggersBeforeCreate = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Triggers
        )
        logger.info(f"Rx'd Triggers: {triggersBeforeCreate}")

        # Check that zoneID1 trigger did not exist before
        matchingTrigger = next(
            (t for t in triggersBeforeCreate if t.zoneID == zoneID1), None)
        asserts.assert_is_none(matchingTrigger, "fTrigger with {zoneID1} already existed in Triggers")

        # Define the parameters of the trigger
        initDuration = 4
        augDuration = 3
        maxDuration = 10
        blindDuration = 4
        if self.perZoneSenseSupported:
            sensitivity = 4
        else:
            sensitivity = None

        # Create a ZoneTrigger command with a valid ZoneTriggerControlStruct
        zoneTrigger = cluster.Structs.ZoneTriggerControlStruct(
            zoneID=zoneID1, initialDuration=initDuration, augmentationDuration=augDuration, maxDuration=maxDuration, blindDuration=blindDuration, sensitivity=sensitivity)
        try:
            logger.info(f"Create/Update Trigger with ID = {zoneID1}")
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.CreateOrUpdateTrigger(trigger=zoneTrigger))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to create zone trigger")
            pass

        triggersAfterCreate = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Triggers
        )
        logger.info(f"Rx'd Triggers: {triggersAfterCreate}")

        matchingTrigger = next(
            (t for t in triggersAfterCreate if t.zoneID == zoneID1), None)
        asserts.assert_is_not_none(matchingTrigger, "fTrigger with {zoneID1} not found")

        self.step("2b")

        triggers = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Triggers
        )
        logger.info(f"Rx'd Triggers: {triggers}")

        asserts.assert_is(type(triggers), list)
        for t in triggers:
            asserts.assert_is(type(t), cluster.Structs.ZoneTriggerControlStruct,
                              "Invalid entry in Triggers")

        matchingTrigger = next(
            (t for t in triggers if t.zoneID == zoneID1), None)
        asserts.assert_is_not_none(matchingTrigger, "fTrigger with {zoneID1} not found")
        asserts.assert_equal(matchingTrigger.initialDuration, initDuration,
                             "InitialDuration of created Trigger does not match")
        asserts.assert_equal(matchingTrigger.augmentationDuration, augDuration,
                             "AugmentationDuration of created Trigger does not match")
        asserts.assert_equal(matchingTrigger.maxDuration, maxDuration,
                             "MaxDuration of created Trigger does not match")
        asserts.assert_equal(matchingTrigger.blindDuration, blindDuration,
                             "BlindDuration of created Trigger does not match")
        if self.perZoneSenseSupported:
            asserts.assert_equal(matchingTrigger.sensitivity, sensitivity,
                                 "Sensitivity of created Trigger does not match")

        self.step("3")

        node_id = self.dut_node_id
        dev_ctrl = self.default_controller
        event_listener = EventSubscriptionHandler(expected_cluster=cluster)
        await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)
        event_delay_seconds = 5.0
        await self._trigger_motion_event(zoneID1, prompt_msg=f"Press enter and immediately start motion activity in zone {zoneID1} and stop any motion after {initDuration} seconds of pressing enter.")

        event = event_listener.wait_for_event_report(
            cluster.Events.ZoneTriggered, timeout_sec=event_delay_seconds)
        asserts.assert_equal(type(event), cluster.Events.ZoneTriggered,
                             "Incorrect event type")
        asserts.assert_equal(event.zone, zoneID1, "Unexpected zoneID on ZoneTriggered")
        asserts.assert_equal(event.reason, enums.ZoneEventTriggeredReasonEnum.kMotion, "Unexpected reason on ZoneTriggered")

        self.step("3a")
        # Wait for a duration exceeding the initial duration without triggering
        # any more events
        event_delay_seconds = initDuration + 1
        self.step("3b")
        event = event_listener.wait_for_event_report(
            cluster.Events.ZoneStopped, timeout_sec=event_delay_seconds)
        asserts.assert_equal(type(event), cluster.Events.ZoneStopped,
                             "Incorrect event type")
        asserts.assert_equal(event.zone, zoneID1, "Unexpected zoneID on ZoneTriggered")
        asserts.assert_equal(event.reason, enums.ZoneEventStoppedReasonEnum.kActionStopped, "Unexpected reason on ZoneStopped")
        await asyncio.sleep(blindDuration)

        self.step("4")
        # Generate 2 triggers in quick succession to see if ZoneStopped comes after Augmentation duration

        event_listener = EventSubscriptionHandler(expected_cluster=cluster)
        await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)
        event_delay_seconds = 5.0
        await self._trigger_motion_event(zoneID1, prompt_msg=f"Press enter and immediately start motion activity in zone {zoneID1}. Stop after 2-3 seconds. Be ready for a second prompt for another motion activity to trigger an event.")

        event = event_listener.wait_for_event_report(
            cluster.Events.ZoneTriggered, timeout_sec=event_delay_seconds)
        asserts.assert_equal(type(event), cluster.Events.ZoneTriggered,
                             "Incorrect event type")
        asserts.assert_equal(event.zone, zoneID1, "Unexpected zoneID on ZoneTriggered")
        asserts.assert_equal(event.reason, enums.ZoneEventTriggeredReasonEnum.kMotion, "Unexpected reason on ZoneTriggered")

        self.step("4a")
        await self._trigger_motion_event(zoneID1, prompt_msg=f"Press enter and immediately start motion activity in zone {zoneID1} for a second time in quick succession.")

        event_delay_seconds = initDuration + augDuration + 1

        self.step("4b")
        event = event_listener.wait_for_event_report(
            cluster.Events.ZoneStopped, timeout_sec=event_delay_seconds)
        asserts.assert_equal(type(event), cluster.Events.ZoneStopped,
                             "Incorrect event type")
        asserts.assert_equal(event.zone, zoneID1, "Unexpected zoneID on ZoneTriggered")
        asserts.assert_equal(event.reason, enums.ZoneEventStoppedReasonEnum.kActionStopped, "Unexpected reason on ZoneStopped")
        await asyncio.sleep(blindDuration)

        self.step("5")
        # Repeat Step 3
        await self._trigger_motion_event(zoneID1, prompt_msg=f"Press enter and immediately start motion activity in zone {zoneID1}. Stop after 2-3 seconds.")

        event = event_listener.wait_for_event_report(
            cluster.Events.ZoneTriggered, timeout_sec=event_delay_seconds)
        asserts.assert_equal(type(event), cluster.Events.ZoneTriggered,
                             "Incorrect event type")
        asserts.assert_equal(event.zone, zoneID1, "Unexpected zoneID on ZoneTriggered")
        asserts.assert_equal(event.reason, enums.ZoneEventTriggeredReasonEnum.kMotion, "Unexpected reason on ZoneTriggered")

        self.step("5a")
        # Make the DUT keep detecting motion for period exceeding MaxDuration
        # time
        # Generate some activity triggers to facilitate advancing of triggerdetectedDuration
        # beyond maxduration
        if self.is_pics_sdk_ci_only:
            for i in range(maxDuration):
                self.write_to_app_pipe({"Name": "ZoneTriggered", "ZoneId": zoneID1})
                await asyncio.sleep(1)
        else:
            self.wait_for_user_input(
                prompt_msg=f"""Press enter and immediately start, and keep generating motion activity from zone {zoneID1} for a period exceeding {maxDuration} seconds.
After {maxDuration}, keep generating some motion activity during the {blindDuration} seconds blind duration phase. DUT should not send any ZoneTriggered event during this phase.""")

        event_delay_seconds = maxDuration
        event = event_listener.wait_for_event_report(
            cluster.Events.ZoneStopped, timeout_sec=event_delay_seconds)

        self.step("5b")
        asserts.assert_equal(type(event), cluster.Events.ZoneStopped,
                             "Incorrect event type")
        asserts.assert_equal(event.zone, zoneID1, "Unexpected zoneID on ZoneTriggered")
        asserts.assert_equal(event.reason, enums.ZoneEventStoppedReasonEnum.kTimeout, "Unexpected reason on ZoneStopped")

        self.step("5c")
        event_delay_seconds = blindDuration + 1
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "ZoneTriggered", "ZoneId": zoneID1})
        event = event_listener.wait_for_event_expect_no_report(timeout_sec=event_delay_seconds)
        logger.info(f"Successfully timed out without receiving any ZoneTriggered event during blind duration for zone: {zoneID1}")

        self.step("6")

        # Fetch the zones attribute
        zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Zones
        )
        logger.info(f"Rx'd Zones: {zones}")
        maxZoneId = 0
        # Get the max zoneId
        for zone in zones:
            if maxZoneId < zone.zoneID:
                maxZoneId = zone.zoneID

        # Assign a new one greater than the max
        nonExistentZoneId = maxZoneId + 1

        # Create a ZoneTrigger command with the non-existing ZoneId
        zoneTrigger = cluster.Structs.ZoneTriggerControlStruct(
            zoneID=nonExistentZoneId, initialDuration=10, augmentationDuration=2, maxDuration=50, blindDuration=5, sensitivity=sensitivity)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.CreateOrUpdateTrigger(trigger=zoneTrigger))
            asserts.fail("Unexpected success when expecting NOT_FOUND due to new zone id being used")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.NotFound,
                "Unexpected error returned expecting NOT_FOUND due to new zone id being used",
            )
            pass

        if self.twoDCartSupported and self.userDefinedSupported:
            self.step("7")

            # Form the Create request and send
            zoneVertices = [
                cluster.Structs.TwoDCartesianVertexStruct(10, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 10),
                cluster.Structs.TwoDCartesianVertexStruct(20, 20),
                cluster.Structs.TwoDCartesianVertexStruct(10, 20)
            ]
            zoneToCreate = cluster.Structs.TwoDCartesianZoneStruct(
                name="Zone4", use=enums.ZoneUseEnum.kPrivacy, vertices=zoneVertices,
                color="#00FFFF")

            createTwoDCartesianCmd = commands.CreateTwoDCartesianZone(
                zone=zoneToCreate
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            logger.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
            asserts.assert_equal(type(cmdResponse), commands.CreateTwoDCartesianZoneResponse,
                                 "Incorrect response type")
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianCmdResponse does not contain ZoneID")
            zoneID4 = cmdResponse.zoneID

            # Fetch the zones attribute after Create
            zonesAfterCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            logger.info(f"Rx'd Zones: {zonesAfterCreate}")

            matchingZone = next(
                (z for z in zonesAfterCreate if z.zoneID == zoneID4), None)
            asserts.assert_is_not_none(matchingZone, "fZone with {zoneID4} not found")
            asserts.assert_equal(matchingZone.zoneType, enums.ZoneTypeEnum.kTwoDCARTZone,
                                 "ZoneType of created Zone is not of type TwoDCartZone")
            asserts.assert_equal(matchingZone.zoneSource, enums.ZoneSourceEnum.kUser,
                                 "ZoneSource of created Zone is not of type User")
            asserts.assert_equal(matchingZone.twoDCartesianZone.vertices, zoneVertices,
                                 "ZoneVertices of created Zone are mismatched")

            self.step("7a")

            # Create a ZoneTrigger command for ZoneID4(Privacy Zone)
            zoneTrigger = cluster.Structs.ZoneTriggerControlStruct(
                zoneID=zoneID4, initialDuration=10, augmentationDuration=2, maxDuration=50, blindDuration=5, sensitivity=sensitivity)
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.CreateOrUpdateTrigger(trigger=zoneTrigger))
                asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to zone use not being User")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.ConstraintError,
                    "Unexpected error returned expecting CONSTRAINT_ERROR due to zone use not being User",
                )
                pass

        else:
            logging.info("TwoDCartZone or UserDefinedZones Feature not supported. Test steps skipped")
            self.skip_step("7")
            self.skip_step("7a")

        if zoneID1 is not None:
            self.step("8")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=zoneID1))
                asserts.fail("Unexpected success when expecting INVALID_IN_STATE due to a trigger being active on the zoneID")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Unexpected error returned expecting INVALID_IN_STATE due to a trigger being active on the zoneID",
                )
                pass

            self.step("8a")
            # Fetch the zones attribute
            zones = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Zones
            )
            logger.info(f"Rx'd Zones: {zones}")
            maxZoneId = 0
            # Get the max zoneId
            for zone in zones:
                if maxZoneId < zone.zoneID:
                    maxZoneId = zone.zoneID

            # Assign a new one greater than the max
            nonExistentZoneId = maxZoneId + 1

            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveTrigger(zoneID=nonExistentZoneId))
                asserts.fail("Unexpected success when expecting NOT_FOUND due to the zoneID not existing")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.NotFound,
                    "Unexpected error returned expecting NOT_FOUND due to the zoneID not existing",
                )
                pass

            self.step("8b")

            try:
                logger.info(f"Removing trigger with Id : {zoneID1}")
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveTrigger(zoneID=zoneID1))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to remove zone")
                pass

            self.step("8c")
            try:
                logger.info(f"Removing zone with Id : {zoneID1}")
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=zoneID1))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to remove zone")
                pass
        else:
            logging.info("ZoneID1 not created. Test steps skipped")
            self.skip_step("8")
            self.skip_step("8a")
            self.skip_step("8b")
            self.skip_step("8c")

        # Fetch the zones attribute
        zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Zones
        )
        logger.info(f"Rx'd Zones: {zones}")
        matchingZone = next(
            (z for z in zones if z.zoneSource == enums.ZoneSourceEnum.kMfg), None)
        if matchingZone is not None:
            self.step("9")
            logger.info(f"Found Mfg Zone with Id : {matchingZone.zoneID}")

            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=matchingZone.zoneID))
                asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to Mfg zone being updated by user")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.ConstraintError,
                    "Unexpected error returned expecting CONSTRAINT_ERROR due to Mfg zone being updated by user",
                )
                pass
        else:
            logger.info("No Mfg Zone found")
            self.skip_step("9")

        self.step("10")
        # Fetch the zones attribute
        zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Zones
        )
        logger.info(f"Rx'd Zones: {zones}")
        maxZoneId = 0
        # Get the max zoneId
        for zone in zones:
            if maxZoneId < zone.zoneID:
                maxZoneId = zone.zoneID

        # Assign a new one greater than the max
        nonExistentZoneId = maxZoneId + 1
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=nonExistentZoneId))
            asserts.fail("Unexpected success when expecting NOT_FOUND due to new zone id being used")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.NotFound,
                "Unexpected error returned expecting NOT_FOUND due to new zone id being used",
            )
            pass

        # Clean up all zones
        for zone in zones:
            try:
                logger.info(f"Removing zone with Id : {zone.zoneID}")
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.RemoveZone(zoneID=zone.zoneID))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned when trying to remove zone")
                pass


if __name__ == "__main__":
    default_matter_test_main()
