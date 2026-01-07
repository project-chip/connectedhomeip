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
#     app: ${ALL_CLUSTERS_APP}
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
import time
from datetime import datetime, timedelta, timezone

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue

from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main
from matter.testing.timeoperations import utc_time_in_matter_epoch

log = logging.getLogger(__name__)

cluster = Clusters.Thermostat
time_cluster = Clusters.TimeSynchronization


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
            TestStep("2a", "TH reads the FeatureMap attribute.",
                     "Verify that the TSUGGEST bit is set in the FeatureMap value."),
            TestStep("2b", "TH reads the Presets attribute and saves it in a SupportedPresets variable.",
                     "Verify that the read returned a list of presets with count >=2."),
            TestStep("3", "TH checks if the Thermostat does not has a valid UTC time.",
                     "Verify if the Thermostat does not have a valid UTC time and set the timeNotSet variable to true"),
            TestStep("4a", "If skipSettingTime is False, TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes.",
                     "Verify that the AddThermostatSuggestion command returns INVALID_IN_STATE."),
            TestStep("4b", "If skipSettingTime is False, TH sends Time Synchronization command to DUT using a time source.",
                     "Verify that the Time Synchronization command returns SUCCESS otherwise fail the test."),
            TestStep("5", "TH picks a random preset handle that does not match any entry in the Presets attribute and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp ExpirationInMinutes is set to 30 minutes.",
                     "Verify that the AddThermostatSuggestion command returns NOT_FOUND."),
            TestStep("6a", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("6b", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute to clean up the test.",
                     "Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute."),
            TestStep("7a", "TH sets TemperatureSetpointHold to SetpointHoldOn and TemperatureSetpointHoldDuration to null. TH reads the ActivePresetHandle attribute. TH picks any preset handle from the \"SupportedPresets\" variable that does not match the ActivePresetHandle and and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the TemperatureSetpointHold is set to SetpointHoldOn and TemperatureSetpointHoldDuration is set to null. Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command, the ThermostatSuggestionNotFollowingReason is set to OngoingHold and the ActivePresetHandle attribute is not updated to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("7b", "TH sets TemperatureSetpointHold to SetpointHoldOff after 10 seconds. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the TemperatureSetpointHold is set to SetpointHoldOff. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is updated to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("7c", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute to clean up the test",
                     "Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute."),
            TestStep("8a", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.",
                     "Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("8b", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to a value not matching the UniqueID field of the CurrentThermostatSuggestion attribute.",
                     "Verify that RemoveThermostatSuggestion command returns NOT_FOUND."),
            TestStep("8c", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute.",
                     "Verify that that RemoveThermostatSuggestion command returns SUCCESS, the entry with the relevant UniqueID is removed from the ThermostatSuggestions attribute."),
            TestStep("9a", "TH reads the ActivePresetHandle attribute and saves it. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 2 minutes. TH calls the AddThermostatSuggestion command again with the saved ActivePresetHandle attribute value, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes.",
                     "Verify that both the AddThermostatSuggestion command return a AddThermostatSuggestionResponse with distinct values in the UniqueID field. TH saves both the UniqueID values. Verify that the ThermostatSuggestions has two entries with the UniqueID field matching one of the UniqueID fields sent in the two AddThermostatSuggestionResponse(s). Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) of one of the entries in ThermostatSuggestions. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
            TestStep("9b", "TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID fields of the two entries in the ThermostatSuggestions attribute.",
                     "Verify that the both the entries are removed from the ThermostatSuggestions attribute."),
            TestStep("10", "TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp plus 25 hours and the ExpirationInMinutes is set to 30 minutes.",
                     "Verify that the AddThermostatSuggestion command returns INVALID_COMMAND."),
            TestStep("11", "TH reads the MaxThermostatSuggestions attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes for the number of times specified in the value of MaxThermostatSuggestions + 1.",
                     "Verify that the AddThermostatSuggestion command returns SUCCESS and the ThermostatSuggestions attribute has one entry added to it for the first {MaxThermostatSuggestions} times. Verify that when the AddThermostatSuggestion command is called for the {MaxThermostatSuggestions + 1} time, the AddThermostatSuggestion command returns RESOURCE_EXHAUSTED.")
        ]

    @async_test_body
    async def test_TC_TSTAT_4_3(self):
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        self.step("2a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the FeatureMap attribute.
            feature_map = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.FeatureMap)
            log.info(f"FeatureMap: {feature_map}")

            # Verify that the TSUGGEST bit is set in the FeatureMap value.
            asserts.assert_true(feature_map & 0b10000000000, "TSUGGEST bit is not set in the FeatureMap")

        self.step("2b")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the Presets attribute and saves it in a SupportedPresets variable.
            supported_presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            log.info(f"Supported Presets: {supported_presets}")

            # Verify that the read returned a list of presets with count >=2.
            asserts.assert_greater_equal(len(supported_presets), 2)

        skipSettingTime = True
        self.step("3")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH checks if the Thermostat does not has a valid UTC time.
            currentUTC = await self.read_single_attribute_check_success(endpoint=0, cluster=time_cluster, attribute=time_cluster.Attributes.UTCTime)

            if currentUTC is NullValue:
                skipSettingTime = False

        if not skipSettingTime:
            self.step("4a")
            if self.pics_guard(self.check_pics("TSTAT.S.F0a")):

                    # If skipSettingTime is False, TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle,
                    # the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes.
                    activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
                    log.info(f"Active Preset Handlers: {activePresetHandle}")

                    possiblePresetHandles = [
                        preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
                    if len(possiblePresetHandles) > 0:
                        preset_handle = possiblePresetHandles[0]
                        # Verify that the AddThermostatSuggestion command returns INVALID_IN_STATE.
   
                        await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                          preset_handle=preset_handle,
                                                                          effective_time=currentUTC,
                                                                          expiration_in_minutes=30,
                                                                          expected_status=Status.InvalidInState)
                    else:
                        log.info("Couldn't run test step 3 since all preset handles are also the ActivePresetHandle on this Thermostat")

            self.step("4b")
            if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
                # If skipSettingTime is False, TH sends Time Synchronization command to DUT using a time source.
                try:
                    th_utc = utc_time_in_matter_epoch()
                    await self.send_single_cmd(cmd=time_cluster.Commands.SetUTCTime(UTCTime=th_utc, granularity=time_cluster.Enums.GranularityEnum.kMillisecondsGranularity), endpoint=0)
                except InteractionModelError as e:
                    # The python layer discards the cluster specific portion of the status IB, so for now we just expect a generic FAILURE error
                    # see #26521
                    code = e.status

                # Verify that the Time Synchronization command returns SUCCESS otherwise fail the test.
                asserts.assert_equal(code, 0, "Test failed because Thermostat has no UTCTime set and not allowing UTCTime to be set. Thermostat suggestions fetaure needs UTCTime.")
        else:
            log.info("Thermostat has UTC time set.")
            self.skip_step("4a")
            self.skip_step("4b")

        self.step("5")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH picks a random preset handle that does not match any entry in the Presets attribute and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp ExpirationInMinutes is set to 30 minutes.
            counter = 0
            for preset in supported_presets:
                new_preset_handle = f"{counter}".encode('ascii')

                if preset.presetHandle == new_preset_handle:
                    counter+= 1
                else:
                    continue
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            # Verify that the AddThermostatSuggestion command returns NOT_FOUND.
            await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                              preset_handle=new_preset_handle,
                                                              effective_time=currentUTC,
                                                              expiration_in_minutes=30,
                                                              expected_status=Status.NotFound)

        self.step("6a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp
            # and ExpirationInMinutes is set to 30 minutes. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            log.info(f"Active Preset Handlers: {activePresetHandle}")
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 6a since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            if currentUTC is not NullValue:
                expirationInMinutes = 30
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
                log.info(f"UniqueID from AddThermostatSuggestionResponse: {addThermostatSuggestionResponse_uniqueID}")

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
            # TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute to clean up the test.
            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=addThermostatSuggestionResponse_uniqueID,
                                                                 expected_status=Status.Success)

            # Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")

        self.step("7a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a") & self.check_pics("TSTAT.S.A0023") & self.check_pics("TSTAT.S.A0024")):
            # TH sets TemperatureSetpointHold to SetpointHoldOn and TemperatureSetpointHoldDuration to null.
            await self.write_single_attribute(attribute_value=cluster.Attributes.TemperatureSetpointHold(cluster.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOn), endpoint_id=endpoint, expect_success=True)
            await self.write_single_attribute(attribute_value=cluster.Attributes.TemperatureSetpointHoldDuration(0), endpoint_id=endpoint, expect_success=True)

            # TH reads the ActivePresetHandle attribute.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            log.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks any preset handle from the \"SupportedPresets\" variable that does not match the ActivePresetHandle and and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 7a since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = 30
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
            asserts.assert_equal(temperatureSetpointHold, cluster.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOn,
                                 "TemperatureSetpointHold is not equal to SetpointHoldOn")

            temperatureSetpointHoldDuration = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TemperatureSetpointHoldDuration)
            asserts.assert_equal(temperatureSetpointHoldDuration, 0, "TemperatureSetpointHoldDuration is not equal to Null")

            # Verify that the AddThermostatSuggestion command returns an AddThermostatSuggestionResponse with a distinct value in the UniqueID field.
            if addThermostatSuggestionResponse:
                addThermostatSuggestionResponse_uniqueID = addThermostatSuggestionResponse.uniqueID
                log.info(f"UniqueID from AddThermostatSuggestionResponse: {addThermostatSuggestionResponse_uniqueID}")

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
            # TODO: For now the ReEvaluateCurrentSuggestion API is in the delegate and we can't check whether a hold is set on the Thermostat. However as part of #39949, this will be addressed and the test
            # can be uncommented.
            #asserts.assert_equal(thermostatSuggestionNotFollowingReason,
                                 #cluster.Bitmaps.ThermostatSuggestionNotFollowingReasonBitmap.kOngoingHold,
                                 #"ThermostatSuggestionNotFollowingReason attribute should be equal to OngoingHold.")
            #asserts.assert_not_equal(activePresetHandle, presetHandle,
                                     #"ActivePresetHandle attribute should not be equal to the PresetHandle in the CurrentThermostatSuggestion attribute.")

        self.step("7b")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH sets TemperatureSetpointHold to SetpointHoldOff after 2 seconds.
            await asyncio.sleep(2)
            await self.write_single_attribute(attribute_value=cluster.Attributes.TemperatureSetpointHold(cluster.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOff), endpoint_id=endpoint, expect_success=True)

            # TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            thermostatSuggestionNotFollowingReason = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestionNotFollowingReason)
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)

            # Verify that the TemperatureSetpointHold is set to SetpointHoldOff.
            temperatureSetpointHold = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TemperatureSetpointHold)
            asserts.assert_equal(temperatureSetpointHold, cluster.Enums.TemperatureSetpointHoldEnum.kSetpointHoldOff,
                                 "TemperatureSetpointHold is not equal to SetpointHoldOff")

            # If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is updated to the PresetHandle field of the CurrentThermostatSuggestion attribute.
            if thermostatSuggestionNotFollowingReason is None:
                asserts.assert_equal(activePresetHandle, presetHandle,
                                     "ActivePresetHandle attribute should be equal to the PresetHandle in the CurrentThermostatSuggestion attribute when ThermostatSuggestionNotFollowingReason is set to null.")

        self.step("7c")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID field of then CurrentThermostatSuggestion attribute to clean up the test.
            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=addThermostatSuggestionResponse_uniqueID,
                                                                 expected_status=Status.Success)

            # Verify that the entry with the UniqueID matching the UniqueID field in the CurrentThermostatSuggestion attribute is removed from the ThermostatSuggestions attribute.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")

        self.step("8a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute. TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp
            # and ExpirationInMinutes is set to 30 minutes. TH reads the CurrentThermostatSuggestion, the ThermostatSuggestionNotFollowingReason and the ActivePresetHandle attributes.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            log.info(f"Active Preset Handlers: {activePresetHandle}")

            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 8a since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = 30
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
                log.info(f"UniqueID from AddThermostatSuggestionResponse: {addThermostatSuggestionResponse_uniqueID}")

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
            currentThermostatSuggestion = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentThermostatSuggestion)
            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=currentThermostatSuggestion.uniqueID,
                                                                 expected_status=Status.Success)
            # Verify that that RemoveThermostatSuggestion command returns SUCCESS, the entry with the relevant UniqueID is removed from the ThermostatSuggestions attribute and the CurrentThermostatSuggestion attribute is set to null.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should not have any entries after the relevant entry was removed by UniqueID.")

        self.step("9a")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute and saves it.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            log.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and the ExpirationInMinutes is set to 35 minutes.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 9a since all preset handles are also the ActivePresetHandle on this Thermostat")
            firstPresetHandle = possiblePresetHandles[0]
            firstCurrentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            firstExpirationInMinutes = 35
            firstAddThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                     preset_handle=firstPresetHandle,
                                                                                                     effective_time=firstCurrentUTC,
                                                                                                     expiration_in_minutes=firstExpirationInMinutes,
                                                                                                     expected_status=Status.Success)

            # TH calls the AddThermostatSuggestion command again with the saved ActivePresetHandle attribute value, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes.
            secondPresetHandle = activePresetHandle
            secondCurrentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            secondExpirationInMinutes = 30
            secondAddThermostatSuggestionResponse = await self.send_add_thermostat_suggestion_command(endpoint=endpoint,
                                                                                                      preset_handle=secondPresetHandle,
                                                                                                      effective_time=secondCurrentUTC,
                                                                                                      expiration_in_minutes=secondExpirationInMinutes,
                                                                                                      expected_status=Status.Success)

            # Verify that both the AddThermostatSuggestion command return a AddThermostatSuggestionResponse with distinct values in the UniqueID field.
            # TH saves both the UniqueID values.
            firstAddThermostatSuggestionResponse_uniqueID = firstAddThermostatSuggestionResponse.uniqueID
            log.info(f"UniqueID from First AddThermostatSuggestionResponse: {firstAddThermostatSuggestionResponse_uniqueID}")

            secondAddThermostatSuggestionResponse_uniqueID = secondAddThermostatSuggestionResponse.uniqueID
            log.info(f"UniqueID from Second AddThermostatSuggestionResponse: {secondAddThermostatSuggestionResponse_uniqueID}")

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
            # TH calls the RemoveThermostatSuggestion command with the UniqueID field set to the UniqueID fields of the added Thermostat Suggestions to clean up the test.
            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=firstAddThermostatSuggestionResponse_uniqueID,
                                                                 expected_status=Status.Success)

            # Verify that the ThermostatSuggestions attribute has one entry in it.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 1,
                                 "ThermostatSuggestions should have 1 entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")

            await self.send_remove_thermostat_suggestion_command(endpoint=endpoint,
                                                                 uniqueID=secondAddThermostatSuggestionResponse_uniqueID,
                                                                 expected_status=Status.Success)

            # Verify that the ThermostatSuggestions attribute has no entry in it.
            thermostatSuggestions = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ThermostatSuggestions)
            asserts.assert_equal(len(thermostatSuggestions), 0,
                                 "ThermostatSuggestions should have 0 entries after the ExpirationTime field in CurrentThermostatSuggestion expired.")

        self.step("10")
        if self.pics_guard(self.check_pics("TSTAT.S.F0a")):
            # TH reads the ActivePresetHandle attribute.
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            log.info(f"Active Preset Handlers: {activePresetHandle}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp plus 25 hours and the ExpirationInMinutes is set to 30 minutes.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 10 since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds()) + int(timedelta(hours=25).total_seconds())
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
            log.info(f"MaxThermostatSuggestions: {maxThermostatSuggestions}")
            # TH picks a preset handle from an entry in the SupportedPresets that does not match the ActivePresetHandle and calls the AddThermostatSuggestion command with the preset handle, the EffectiveTime set to the current UTC timestamp and ExpirationInMinutes is set to 30 minutes for the number of times specified in the value of MaxThermostatSuggestions + 1.
            possiblePresetHandles = [
                preset.presetHandle for preset in supported_presets if preset.presetHandle != activePresetHandle]
            asserts.assert_greater_equal(
                len(possiblePresetHandles), 1, "Couldn't run test step 11 since all preset handles are also the ActivePresetHandle on this Thermostat")
            presetHandle = possiblePresetHandles[0]
            currentUTC = int((datetime.now(timezone.utc) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
            expirationInMinutes = 30

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
