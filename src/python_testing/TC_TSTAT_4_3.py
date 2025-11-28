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
#     app: ${all-clusters}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import random
from datetime import datetime, timedelta, timezone

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = Clusters.Thermostat


class TC_TSTAT_4_3(MatterBaseTest):

    # Command to send AddThermostatSuggestion command
    async def send_add_thermostat_suggestion_command(self,
                                                     endpoint: int = None,
                                                     preset_handle: bytes = None,
                                                     effective_time: int = None,
                                                     expiration_in_minutes: int = None,
                                                     expected_status: Status = Status.Success):
        try:
            addThermostatSuggestionResponse = await self.send_single_cmd(cmd=cluster.Commands.AddThermostatSuggestion(preset_handle, effective_time, expiration_in_minutes),
                                                                         endpoint=endpoint)
            asserts.assert_equal(expected_status, Status.Success)
            return addThermostatSuggestionResponse

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    # Command to send RemoveThermostatSuggestion command
    async def send_remove_thermostat_suggestion_command(self,
                                                        endpoint: int = None,
                                                        uniqueID: int = None,
                                                        expected_status: Status = Status.Success):
        try:
            removeThermostatSuggestionResponse = await self.send_single_cmd(cmd=cluster.Commands.RemoveThermostatSuggestion(uniqueID),
                                                                            endpoint=endpoint)
            asserts.assert_equal(expected_status, Status.Success)
            return removeThermostatSuggestionResponse

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    def desc_TC_TSTAT_4_3(self) -> str:
        """Returns a description of this test"""
        return "3.1.5 [TC-TSTAT-4-3] This test case verifies that the DUT can respond to Preset suggestion commands."

    def pics_TC_TSTAT_4_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["TSTAT.S", "TSTAT.S.F0a"]

    def steps_TC_TSTAT_4_3(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads the Presets attribute and saves it in a SupportedPresets variable.",
                     "Verify that the read returned a list of presets with count >=2."),
            TestStep("3", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.",
                     "Verify that the AddThermostatSuggestion command returns INVALID_IN_STATE."),
            TestStep("4", "TH sends Time Synchronization command to DUT using a time source.",
                     "Verify that TH and DUT are now time synchronized."),
            TestStep("5", "TH picks a random preset handle that does not match any entry in the Presets attribute and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp the ExpirationInMinutes is set to 1 minute.",
                     "Verify that the AddThermostatSuggestion command returns NOT_FOUND."),
            TestStep("6a", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("6b", "TH waits until the UTC timestamp specified in the ExpirationTime field in the CurrentThermostatSuggestion for the suggestion to expire.",
                     "Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null."),
            TestStep("7a", "TH sets TemperatureSetpointHold to SetpointHoldOn and TemperatureSetpointHoldDuration to null. TH reads the ActivePresetHandle attribute. TH picks any preset handle from the \"SupportedPresets\" variable that does not match the ActivePresetHandle and and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the TemperatureSetpointHold is set to SetpointHoldOn and TemperatureSetpointHoldDuration is set to null. Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command, the ThermostatSuggestionNotFollowingReason is set to OngoingHold and the ActivePresetHandle attribute is not updated to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("7b", "TH sets TemperatureSetpointHold to SetpointHoldOff after 10 seconds. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the TemperatureSetpointHold is set to SetpointHoldOff. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is updated to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("7c", "TH waits until the UTC timestamp specified in the ExpirationTime field in the CurrentThermostatSuggestion for the suggestion to expire.",
                     "Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null."),
            TestStep("8a", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("8b", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to a value not matching the UniqueID field of the CurrentThermostatSuggestion attribute.",
                     "Verify that RemoveThermostatSuggestion command returns NOT_FOUND."),
            TestStep("8c", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute.",
                     "Verify that that RemoveThermostatSuggestion command returns SUCCESS, the entry with the relevant UniqueID is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null."),
            TestStep("9a", "TH reads the ActivePresetHandle attribute and saves it. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 2 minutes. TH calls the AddThermostatSuggestion command again with the saved ActivePresetHandle attribute value, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.",
                     "Verify that both the AddThermostatSuggestion command return a AddThermostatSuggestionResponse with distinct values in the UniqueID field. TH saves both the UniqueID values. Verify that the ThermostatSuggestions has two entries with the UniqueID field matching one of the UniqueID fields sent in the two AddThermostatSuggestionResponse(s). Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) of one of the entries in ThermostatSuggestions. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("9b", "TH waits until the timestamp value specified in the earliest ExpirationTime field in the two entries in the ThermostatSuggestions attribute.",
                     "Verify that the entry with the UniqueID that matches the earliest ExpirationTime in the two entries in the ThermostatSuggestions attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to the remaining entry in the ThermostatSuggestions attribute. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("9c", "TH waits until the UTC timestamp specified in the ExpirationTime field in the CurrentThermostatSuggestion for the suggestion to expire.",
                     "Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null."),
            TestStep("10", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp plus 25 hours and the ExpirationInMinutes is set to 30 minutes.",
                     "Verify that the AddThermostatSuggestion command returns INVALID_COMMAND."),
            TestStep("11", "TH reads the MaxThermostatSuggestions attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute for the number of times specified in the value of MaxThermostatSuggestions + 1.",
                     "Verify that the AddThermostatSuggestion command returns SUCCESS and the ThermostatSuggestions attribute has one entry added to it for the first {MaxThermostatSuggestions} times. Verify that when the AddThermostatSuggestion command is called for the {MaxThermostatSuggestions + 1} time, the AddThermostatSuggestion command returns RESOURCE_EXHAUSTED.")
        ]

    @async_test_body
    async def test_TC_TSTAT_4_3(self):
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the Presets attribute and saves it in a SupportedPresets variable.
            supported_presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            logger.info(f"Supported Presets: {supported_presets}")

            # Verify that the read returned a list of presets with count >=2.
            asserts.assert_greater_equal(len(supported_presets), 2)

        # TODO Remove skipTimeSync once TimeSync details are ironed out in the test plan
        skipTimeSync = True
        if not skipTimeSync:
            self.step("3")
            if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
                # TH reads the ActivePresetHandle attribute.
                activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
                logger.info(f"Active Preset Handlers: {activePresetHandle}")

                # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.
                possiblePresetHandles = [
                    preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
                if len(possiblePresetHandles) > 0:
                    preset_handle = possiblePresetHandles[0]
                    # Verify that the AddThermostatSuggestion command returns INVALID_IN_STATE.
                    currentUTC = int(int((datetime.now(timezone.utc) - datetime(2000,
                                     1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds()))
                    await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                      preset_handle=preset_handle,
                                                                      effective_time=currentUTC,
                                                                      expiration_in_minutes=30,
                                                                      expected_status=Status.InvalidInState)
                else:
                    logger.info("Couldn't run test step 3 since all preset handles are also the ActivePresetHandle on this Thermostat")

            self.step("4")
            if not skipTimeSync:
                # TH sends Time Synchronization command to DUT using a time source.
                tts = Clusters.TimeSynchronization.Structs.FabricScopedTrustedTimeSourceStruct(nodeID=self.dut_node_id, endpoint=0)
                await self.send_single_cmd(cmd=Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(trustedTimeSource=tts), endpoint=endpoint)
                # Verify that TH and DUT are now time synchronized.
                # TODO Unsure how to validate this one. Read DUT through TimeSynchronization cluster and compare to datetime current time?
        else:
            logger.info("TimeSync steps need to be ironed out, skipping for now.")
            self.skip_step("3")
            self.skip_step("4")

        self.step("5")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH picks a random preset handle that does not match any entry in the Presets attribute and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp the ExpirationInMinutes is set to 1 minute.
            random_preset_handle = random.randbytes(16)
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            # Verify that the AddThermostatSuggestion command returns NOT_FOUND.
            await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                              preset_handle=random_preset_handle,
                                                              effective_time=currentUTC,
                                                              expiration_in_minutes=30,
                                                              expected_status=Status.NotFound)

        self.step("6a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 6a since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = 1
            addThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                preset_handle=presetHandle,
                                                                                                effective_time=currentUTC,
                                                                                                expiration_in_minutes=expirationInMinutes,
                                                                                                expected_status=Status.Success)

            # TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            thermostatSuggestionNotFollowingReason = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestionNotFollowingReason)
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)

            # Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field.
            if addThermostatSuggestionResponse:
                addThermostatSuggestionResponse_uniqueID = addThermostatSuggestionResponse.uniqueID
                logger.info(f"UniqueID from AddThermostatSuggestionResponse: {addThermostatSuggestionResponse_uniqueID}")

            # Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            if len(thermostatSuggestions) > 0:
                asserts.assert_equal(thermostatSuggestions[0].uniqueID, addThermostatSuggestionResponse_uniqueID,
                                     "UniqueID in the entry for ThermostatSuggestions does not match the UniqueID entry from AddThermostatSuggestionResponse.")

            # Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command.
            asserts.assert_equal(currentThermostatSuggestion.uniqueID, addThermostatSuggestionResponse_uniqueID,
                                 "UniqueID in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            asserts.assert_equal(currentThermostatSuggestion.presetHandle, presetHandle,
                                 "PresetHandle in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            asserts.assert_equal(currentThermostatSuggestion.effectiveTime, currentUTC,
                                 "EffectiveTime in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            expirationTime = currentUTC + int(timedelta(minutes=expirationInMinutes).total_seconds())
            asserts.assert_equal(currentThermostatSuggestion.expirationTime, expirationTime,
                                 "ExpirationTime in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")

            # If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            if thermostatSuggestionNotFollowingReason is None:
                asserts.assert_equal(activePresetHandle, presetHandle,
                                     "ActivePresetHandle attribute should be equal to the PresetHandle in the CurrentThermostatSuggestion attribute when ThermostatSuggestionNotFollowingReason is set to null.")

        self.step("6b")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH waits until the UTC timestamp specified in the ExpirationTime field in the CurrentThermostatSuggestion for the suggestion to expire.
            logger.info(
                f"Waiting until ExpirationTime field in CurrentThermostatSuggestion expires: {expirationInMinutes} minute/s")
            await asyncio.sleep(expirationInMinutes * 60)

            # Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            asserts.assert_equal(currentThermostatSuggestion, None,
                                 "CurrentThermostatSuggestion should be Null after the ExpirationTime field in CurrentThermostatSuggestion expired.")

        self.step("7a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH sets TemperatureSetpointHold to SetpointHoldOn and TemperatureSetpointHoldDuration to null.
            await self.write_single_attribute(attribute_value=cluster.Attributes.TemperatureSetpointHold(cluster.Thermostat.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOn), endpoint_id=endpoint, expect_success=True)
            await self.write_single_attribute(attribute_value=cluster.Attributes.TemperatureSetpointHoldDuration(None), endpoint_id=endpoint, expect_success=True)

            # TH reads the ActivePresetHandle attribute.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks any preset handle from the \"SupportedPresets\" variable that does not match the ActivePresetHandle and and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 7a since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = 1
            addThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                preset_handle=presetHandle,
                                                                                                effective_time=currentUTC,
                                                                                                expiration_in_minutes=expirationInMinutes,
                                                                                                expected_status=Status.Success)

            # TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            thermostatSuggestionNotFollowingReason = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestionNotFollowingReason)
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)

            # Verify that the TemperatureSetpointHold is set to SetpointHoldOn and TemperatureSetpointHoldDuration is set to null.
            temperatureSetpointHold = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TemperatureSetpointHold)
            asserts.assert_equal(temperatureSetpointHold, cluster.Thermostat.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOn,
                                 "TemperatureSetpointHold is not equal to SetpointHoldOn")

            temperatureSetpointHoldDuration = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TemperatureSetpointHoldDuration)
            asserts.assert_equal(temperatureSetpointHoldDuration, None, "TemperatureSetpointHoldDuration is not equal to Null")

            # Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field.
            if addThermostatSuggestionResponse:
                addThermostatSuggestionResponse_uniqueID = addThermostatSuggestionResponse.uniqueID
                logger.info(f"UniqueID from AddThermostatSuggestionResponse: {addThermostatSuggestionResponse_uniqueID}")

            # Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            if len(thermostatSuggestions) > 0:
                asserts.assert_equal(thermostatSuggestions[0].uniqueID, addThermostatSuggestionResponse_uniqueID,
                                     "UniqueID in the entry for ThermostatSuggestions does not match the UniqueID entry from AddThermostatSuggestionResponse.")

            # Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus
            #   ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command,
            asserts.assert_equal(currentThermostatSuggestion.uniqueID, addThermostatSuggestionResponse_uniqueID,
                                 "UniqueID in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            asserts.assert_equal(currentThermostatSuggestion.presetHandle, presetHandle,
                                 "PresetHandle in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            asserts.assert_equal(currentThermostatSuggestion.effectiveTime, currentUTC,
                                 "EffectiveTime in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            expirationTime = currentUTC + int(timedelta(minutes=expirationInMinutes).total_seconds())
            asserts.assert_equal(currentThermostatSuggestion.expirationTime, expirationTime,
                                 "ExpirationTime in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")

            # the ThermostatSuggestionNotFollowingReason is set to OngoingHold and the ActivePresetHandle attribute is not updated to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            asserts.assert_equal(thermostatSuggestionNotFollowingReason,
                                 cluster.Thermostat.Bitmaps.ThermostatSuggestionNotFollowingReasonBitmap.kOngoingHold,
                                 "ThermostatSuggestionNotFollowingReason attribute should be equal to OngoingHold.")
            asserts.assert_not_equal(activePresetHandle, presetHandle,
                                     "ActivePresetHandle attribute should not be equal to the PresetHandle in the CurrentThermostatSuggestion attribute.")

        self.step("7b")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH sets TemperatureSetpointHold to SetpointHoldOff after 10 seconds.
            await asyncio.sleep(10)
            await self.write_single_attribute(attribute_value=cluster.Attributes.TemperatureSetpointHold(cluster.Thermostat.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOff), endpoint_id=endpoint, expect_success=True)

            # TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            thermostatSuggestionNotFollowingReason = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestionNotFollowingReason)
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)

            # Verify that the TemperatureSetpointHold is set to SetpointHoldOff.
            temperatureSetpointHold = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TemperatureSetpointHold)
            asserts.assert_equal(temperatureSetpointHold, cluster.Thermostat.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOff,
                                 "TemperatureSetpointHold is not equal to SetpointHoldOff")

            # If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is updated to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            if thermostatSuggestionNotFollowingReason is None:
                asserts.assert_equal(activePresetHandle, presetHandle,
                                     "ActivePresetHandle attribute should be equal to the PresetHandle in the CurrentThermostatSuggestion attribute when ThermostatSuggestionNotFollowingReason is set to null.")

        self.step("7c")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH waits until the UTC timestamp specified in the ExpirationTime field in the CurrentThermostatSuggestion for the suggestion to expire.
            logger.info(
                f"Waiting until ExpirationTime field in CurrentThermostatSuggestion expires: {expirationInMinutes} minute/s")
            await asyncio.sleep(expirationInMinutes * 60)

            # Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            asserts.assert_equal(currentThermostatSuggestion, None,
                                 "CurrentThermostatSuggestion should be Null after the ExpirationTime field in CurrentThermostatSuggestion expired.")

        self.step("8a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 8a since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = 1
            addThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                preset_handle=presetHandle,
                                                                                                effective_time=currentUTC,
                                                                                                expiration_in_minutes=expirationInMinutes,
                                                                                                expected_status=Status.Success)

            # TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            thermostatSuggestionNotFollowingReason = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestionNotFollowingReason)
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)

            # Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a value in the UniqueID field.
            if addThermostatSuggestionResponse:
                addThermostatSuggestionResponse_uniqueID = addThermostatSuggestionResponse.uniqueID
                logger.info(f"UniqueID from AddThermostatSuggestionResponse: {addThermostatSuggestionResponse_uniqueID}")

            # Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            if len(thermostatSuggestions) > 0:
                asserts.assert_equal(thermostatSuggestions[0].uniqueID, addThermostatSuggestionResponse_uniqueID,
                                     "UniqueID in the entry for ThermostatSuggestions does not match the UniqueID entry from AddThermostatSuggestionResponse.")

            # Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command.
            asserts.assert_equal(currentThermostatSuggestion.uniqueID, addThermostatSuggestionResponse_uniqueID,
                                 "UniqueID in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            asserts.assert_equal(currentThermostatSuggestion.presetHandle, presetHandle,
                                 "PresetHandle in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            asserts.assert_equal(currentThermostatSuggestion.effectiveTime, currentUTC,
                                 "EffectiveTime in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")
            expirationTime = currentUTC + int(timedelta(minutes=expirationInMinutes).total_seconds())
            asserts.assert_equal(currentThermostatSuggestion.expirationTime, expirationTime,
                                 "ExpirationTime in the CurrentThermostatSuggestion does not match the entry from AddThermostatSuggestion command.")

            # If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            if thermostatSuggestionNotFollowingReason is None:
                asserts.assert_equal(activePresetHandle, presetHandle,
                                     "ActivePresetHandle attribute should be equal to the PresetHandle in the CurrentThermostatSuggestion attribute when ThermostatSuggestionNotFollowingReason is set to null.")

        self.step("8b")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            print()
            # TH calls the RemoveThermostatSuggestion command with the UniqueID field set to a value not matching the UniqueID field of the CurrentThermostatSuggestion attribute.
            random_uniqueID = addThermostatSuggestionResponse_uniqueID
            while random_uniqueID == addThermostatSuggestionResponse_uniqueID:
                random_uniqueID = random.randint(0, 255)
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            # Verify that the RemoveThermostatSuggestion command returns NOT_FOUND.
            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=random_uniqueID,
                                                                 expected_status=Status.NotFound)

        self.step("8c")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            print()
            # TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute.
            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=addThermostatSuggestionResponse_uniqueID,
                                                                 expected_status=Status.Success)
            # Verify that that RemoveThermostatSuggestion command returns SUCCESS, the entry with the relevant UniqueID is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the relevant entry was removed by UniqueID.")
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            asserts.assert_equal(currentThermostatSuggestion, None,
                                 "CurrentThermostatSuggestion should be Null after the relevant entry was removed by UniqueID.")

        self.step("9a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute and saves it.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 2 minutes.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 9a since all preset handles are also the ActivePresetHandle on this Thermostat")
            firstPresetHandle = possiblePresetHandles[0]
            firstCurrentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            firstExpirationInMinutes = 2
            firstAddThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                     preset_handle=firstPresetHandle,
                                                                                                     effective_time=firstCurrentUTC,
                                                                                                     expiration_in_minutes=firstExpirationInMinutes,
                                                                                                     expected_status=Status.Success)

            # TH calls the AddThermostatSuggestion command again with the saved ActivePresetHandle attribute value, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute.
            secondPresetHandle = activePresetHandle
            secondCurrentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            secondExpirationInMinutes = 1
            secondAddThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                      preset_handle=secondPresetHandle,
                                                                                                      effective_time=secondCurrentUTC,
                                                                                                      expiration_in_minutes=secondExpirationInMinutes,
                                                                                                      expected_status=Status.Success)

            # Verify that both the AddThermostatSuggestion command return a AddThermostatSuggestionResponse with distinct values in the UniqueID field.
            # TH saves both the UniqueID values.
            firstAddThermostatSuggestionResponse_uniqueID = firstAddThermostatSuggestionResponse.uniqueID
            logger.info(f"UniqueID from First AddThermostatSuggestionResponse: {firstAddThermostatSuggestionResponse_uniqueID}")

            secondAddThermostatSuggestionResponse_uniqueID = secondAddThermostatSuggestionResponse.uniqueID
            logger.info(f"UniqueID from Second AddThermostatSuggestionResponse: {secondAddThermostatSuggestionResponse_uniqueID}")

            # Verify that the ThermostatSuggestions has two entries with the UniqueID field matching one of the UniqueID fields sent in the two AddThermostatSuggestionResponse(s).
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 2, "Expected two thermostat suggestions")
            thermostatSuggestionUniqueIDs = {s.uniqueID for s in thermostatSuggestions}
            asserts.assert_in(firstAddThermostatSuggestionResponse_uniqueID,
                              thermostatSuggestionUniqueIDs, "First suggestion's UniqueID not found")
            asserts.assert_in(secondAddThermostatSuggestionResponse_uniqueID,
                              thermostatSuggestionUniqueIDs, "Second suggestion's UniqueID not found")

            # Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) of one of the entries in ThermostatSuggestions.
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            thermostatSuggestionNotFollowingReason = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestionNotFollowingReason)
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)

            try:
                asserts.assert_equal(currentThermostatSuggestion.uniqueID,
                                     thermostatSuggestions[0].uniqueID, "UniqueID in the entry for CurrentThermostatSuggestion does not match the UniqueID entry from First AddThermostatSuggestionResponse.")
                thermSuggestionIndex = 0
            except AssertionError:
                asserts.assert_equal(currentThermostatSuggestion.uniqueID,
                                     thermostatSuggestions[1].uniqueID, "UniqueID in the entry for CurrentThermostatSuggestion does not match the UniqueID entry from First or Second AddThermostatSuggestionResponse.")
                thermSuggestionIndex = 1

            asserts.assert_equal(currentThermostatSuggestion.presetHandle, thermostatSuggestions[thermSuggestionIndex].presetHandle,
                                 "PresetHandle in the entry for CurrentThermostatSuggestion does not match the PresetHandle entry from either AddThermostatSuggestionResponse.")
            asserts.assert_equal(currentThermostatSuggestion.effectiveTime, thermostatSuggestions[thermSuggestionIndex].effectiveTime,
                                 "EffectiveTime in the entry for CurrentThermostatSuggestion does not match the EffectiveTime entry from either AddThermostatSuggestionResponse.")
            asserts.assert_equal(currentThermostatSuggestion.expirationTime, thermostatSuggestions[thermSuggestionIndex].expirationTime,
                                 "ExpirationTime in the entry for CurrentThermostatSuggestion does not match the ExpirationTime entry from either AddThermostatSuggestionResponse.")

            # If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            if thermostatSuggestionNotFollowingReason is None:
                asserts.assert_equal(activePresetHandle, currentThermostatSuggestion.presetHandle,
                                     "ActivePresetHandle attribute should be equal to the PresetHandle in the CurrentThermostatSuggestion attribute when ThermostatSuggestionNotFollowingReason is set to null.")

        self.step("9b")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH waits until the timestamp value specified in the earliest ExpirationTime field in the two entries in the ThermostatSuggestions attribute.
            expirationInMinutes = firstExpirationInMinutes if firstExpirationInMinutes < secondExpirationInMinutes else secondExpirationInMinutes
            logger.info(
                f"Waiting until both ExpirationTime fields in ThermostatSuggestions expires: {expirationInMinutes} minute/s")
            await asyncio.sleep(expirationInMinutes * 60)
            # Verify that the entry with the UniqueID that matches the earliest ExpirationTime in the two entries in the ThermostatSuggestions attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to the remaining entry in the ThermostatSuggestions attribute. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            uniqueIDRemaining = secondAddThermostatSuggestionResponse_uniqueID if expirationInMinutes == firstExpirationInMinutes else firstAddThermostatSuggestionResponse_uniqueID
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 1,
                                 "ThermostatSuggestions should not have more than 1 entry after the ExpirationTime field in the earlier ThermostatSuggestion expired.")
            asserts.assert_equal(thermostatSuggestions[0].uniqueID, uniqueIDRemaining,
                                 "ThermostatSuggestions should have 1 entry with the UniqueID matching the ThermostatSuggestion with the longer ExpirationTime.")

            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            asserts.assert_equal(currentThermostatSuggestion,
                                 thermostatSuggestions[0], "CurrentThermostatSuggestion should be set to the ThermostatSuggestion with the longer ExpirationTime.")

        self.step("9c")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            longerExpirationInMinutes = firstExpirationInMinutes if firstExpirationInMinutes > secondExpirationInMinutes else secondExpirationInMinutes
            shorterExpirationInMinutes = firstExpirationInMinutes if firstExpirationInMinutes < secondExpirationInMinutes else secondExpirationInMinutes
            expirationInMinutes = longerExpirationInMinutes - shorterExpirationInMinutes
            # TH waits until the UTC timestamp specified in the ExpirationTime field in the CurrentThermostatSuggestion for the suggestion to expire.
            logger.info(
                f"Waiting until ExpirationTime field in CurrentThermostatSuggestion expires: {expirationInMinutes} minute/s")
            await asyncio.sleep(expirationInMinutes * 60)

            # Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            asserts.assert_equal(currentThermostatSuggestion, None,
                                 "CurrentThermostatSuggestion should be Null after the ExpirationTime field in CurrentThermostatSuggestion expired.")

        self.step("10")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp plus 25 hours and the ExpirationInMinutes is set to 30 minutes.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 10 since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)
                              ).total_seconds()) + int(timedelta(hours=25).total_seconds())
            expirationInMinutes = 30
            # Verify that the AddThermostatSuggestion command returns INVALID_COMMAND.
            addThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                preset_handle=presetHandle,
                                                                                                effective_time=currentUTC,
                                                                                                expiration_in_minutes=expirationInMinutes,
                                                                                                expected_status=Status.InvalidCommand)

        self.step("11")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            print()
            # TH reads the MaxThermostatSuggestions attribute.
            maxThermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxThermostatSuggestions)
            logger.info(f"MaxThermostatSuggestions: {maxThermostatSuggestions}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 1 minute for the number of times specified in the value of MaxThermostatSuggestions + 1.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 11 since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = maxThermostatSuggestions + 1

            # Verify that the AddThermostatSuggestion command returns SUCCESS and the ThermostatSuggestions attribute has one entry added to it for the first {MaxThermostatSuggestions} times.
            for i in range(0, maxThermostatSuggestions + 1):
                if i < maxThermostatSuggestions:
                    addThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                        preset_handle=presetHandle,
                                                                                                        effective_time=currentUTC,
                                                                                                        expiration_in_minutes=expirationInMinutes,
                                                                                                        expected_status=Status.Success)
                    thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
                    asserts.assert_equal(len(thermostatSuggestions), i+1,
                                         f"ThermostatSuggestions should have {i+1} number of entries.")
                else:
                    # Verify that when the AddThermostatSuggestion command is called for the {MaxThermostatSuggestions + 1} time, the AddThermostatSuggestion command returns RESOURCE_EXHAUSTED.
                    addThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                        preset_handle=presetHandle,
                                                                                                        effective_time=currentUTC,
                                                                                                        expiration_in_minutes=expirationInMinutes,
                                                                                                        expected_status=Status.ResourceExhausted)


if __name__ == "__main__":
    default_matter_test_main()
