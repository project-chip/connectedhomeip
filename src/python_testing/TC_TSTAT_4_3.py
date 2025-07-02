#
#    Copyright (c) 2024 Project CHIP Authors
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

import copy
import logging
import random
from collections import namedtuple

import chip.clusters as Clusters
from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.Thermostat


class TC_TSTAT_4_3(MatterBaseTest):

    def check_atomic_response(self, response: object, expected_status: Status = Status.Success,
                              expected_overall_status: Status = Status.Success,
                              expected_preset_status: Status = Status.Success,
                              expected_schedules_status: Status = None,
                              expected_timeout: int = None):
        asserts.assert_equal(expected_status, Status.Success, "We expected we had a valid response")
        asserts.assert_equal(response.statusCode, expected_overall_status, "Response should have the right overall status")
        found_preset_status = False
        found_schedules_status = False
        for attrStatus in response.attributeStatus:
            if attrStatus.attributeID == cluster.Attributes.Presets.attribute_id:
                asserts.assert_equal(attrStatus.statusCode, expected_preset_status,
                                     "Preset attribute should have the right status")
                found_preset_status = True
            if attrStatus.attributeID == cluster.Attributes.Schedules.attribute_id:
                asserts.assert_equal(attrStatus.statusCode, expected_schedules_status,
                                     "Schedules attribute should have the right status")
                found_schedules_status = True
        if expected_timeout is not None:
            asserts.assert_equal(response.timeout, expected_timeout,
                                 "Timeout should have the right value")
        asserts.assert_true(found_preset_status, "Preset attribute should have a status")
        if expected_schedules_status is not None:
            asserts.assert_true(found_schedules_status, "Schedules attribute should have a status")
            asserts.assert_equal(attrStatus.statusCode, expected_schedules_status,
                                 "Schedules attribute should have the right status")
        asserts.assert_true(found_preset_status, "Preset attribute should have a status")

    def check_returned_presets(self, sent_presets: list, returned_presets: list):
        asserts.assert_true(len(sent_presets) == len(returned_presets), "Returned presets are a different length than sent presets")
        for i, sent_preset in enumerate(sent_presets):
            returned_preset = returned_presets[i]
            if sent_preset.presetHandle is NullValue:
                sent_preset = copy.copy(sent_preset)
                sent_preset.presetHandle = returned_preset.presetHandle
            if sent_preset.builtIn is NullValue:
                sent_preset.builtIn = returned_preset.builtIn
            asserts.assert_equal(sent_preset, returned_preset,
                                 "Returned preset is not the same as sent preset")

    def count_preset_scenarios(self, presets: list):
        presetScenarios = {}
        for preset in presets:
            if preset.presetScenario not in presetScenarios:
                presetScenarios[preset.presetScenario] = 1
            else:
                presetScenarios[preset.presetScenario] += 1
        return presetScenarios

    def get_available_scenario(self, presetTypes: list, presetScenarioCounts: map):
        availableScenarios = list(presetType.presetScenario for presetType in presetTypes if presetScenarioCounts.get(
            presetType.presetScenario, 0) < presetType.numberOfPresets)
        if len(availableScenarios) > 0:
            return availableScenarios[0]
        return None

    def make_preset(self, presetScenario,  coolSetpoint, heatSetpoint, presetHandle=NullValue, name=None, builtIn=False):
        preset = cluster.Structs.PresetStruct(presetHandle=presetHandle, presetScenario=presetScenario, builtIn=builtIn)
        if self.check_pics("TSTAT.S.F00"):
            preset.heatingSetpoint = heatSetpoint
        if self.check_pics("TSTAT.S.F01"):
            preset.coolingSetpoint = coolSetpoint
        if name is not None:
            preset.name = name
        return preset

    async def write_presets(self,
                            endpoint,
                            presets,
                            dev_ctrl: ChipDeviceCtrl = None,
                            expected_status: Status = Status.Success) -> Status:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        result = await dev_ctrl.WriteAttribute(self.dut_node_id, [(endpoint, cluster.Attributes.Presets(presets))])
        status = result[0].Status
        asserts.assert_equal(status, expected_status, f"Presets write returned {status.name}; expected {expected_status.name}")
        return status

    async def send_atomic_request_begin_command(self,
                                                dev_ctrl: ChipDeviceCtrl = None,
                                                endpoint: int = None,
                                                timeout: int = 1800,
                                                expected_status: Status = Status.Success,
                                                expected_overall_status: Status = Status.Success,
                                                expected_preset_status: Status = Status.Success,
                                                expected_schedules_status: Status = None,
                                                expected_timeout: int = None):
        try:
            response = await self.send_single_cmd(cmd=cluster.Commands.AtomicRequest(requestType=Globals.Enums.AtomicRequestTypeEnum.kBeginWrite,
                                                                                     attributeRequests=[
                                                                                         cluster.Attributes.Presets.attribute_id],
                                                                                     timeout=timeout),
                                                  dev_ctrl=dev_ctrl,
                                                  endpoint=endpoint)
            self.check_atomic_response(response, expected_status, expected_overall_status,
                                       expected_preset_status, expected_schedules_status, expected_timeout)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_atomic_request_commit_command(self,
                                                 dev_ctrl: ChipDeviceCtrl = None,
                                                 endpoint: int = None,
                                                 expected_status: Status = Status.Success,
                                                 expected_overall_status: Status = Status.Success,
                                                 expected_preset_status: Status = Status.Success,
                                                 expected_schedules_status: Status = None):
        try:
            response = await self.send_single_cmd(cmd=cluster.Commands.AtomicRequest(requestType=Globals.Enums.AtomicRequestTypeEnum.kCommitWrite,
                                                                                     attributeRequests=[cluster.Attributes.Presets.attribute_id]),
                                                  dev_ctrl=dev_ctrl,
                                                  endpoint=endpoint)
            self.check_atomic_response(response, expected_status, expected_overall_status,
                                       expected_preset_status, expected_schedules_status)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_atomic_request_rollback_command(self,
                                                   dev_ctrl: ChipDeviceCtrl = None,
                                                   endpoint: int = None,
                                                   expected_status: Status = Status.Success,
                                                   expected_overall_status: Status = Status.Success,
                                                   expected_preset_status: Status = Status.Success,
                                                   expected_schedules_status: Status = None):
        try:
            response = await self.send_single_cmd(cmd=cluster.Commands.AtomicRequest(requestType=Globals.Enums.AtomicRequestTypeEnum.kRollbackWrite,
                                                                                     attributeRequests=[cluster.Attributes.Presets.attribute_id]),
                                                  dev_ctrl=dev_ctrl,
                                                  endpoint=endpoint)
            self.check_atomic_response(response, expected_status, expected_overall_status,
                                       expected_preset_status, expected_schedules_status)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_set_active_preset_handle_request_command(self,
                                                            endpoint: int = None,
                                                            value: bytes = None,
                                                            expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=cluster.Commands.SetActivePresetRequest(value),
                                       endpoint=endpoint)
            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    def desc_TC_TSTAT_4_3(self) -> str:
        """Returns a description of this test"""
        return "3.1.5 [TC-TSTAT-4-3] This test case verifies that the DUT can respond to Preset suggestion commands."

    def pics_TC_TSTAT_4_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["TSTAT.S"]

    def steps_TC_TSTAT_4_3(self) -> list[TestStep]:
        steps = [
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
                     "Verify that the AddThermostatSuggestion command returns a AddThermostatSuggestionResponse with a distinct value in the UniqueID field. Verify that the ThermostatSuggestions has one entry with the UniqueID field matching the UniqueID sent in the AddThermostatSuggestionResponse. Verify that the CurrentThermostatSuggestion attribute is set to the uniqueID, preset handle, the EffectiveTime, and the EffectiveTime plus ExpirationInMinutes (converted to seconds) passed in the AddThermostatSuggestion command. If the ThermostatSuggestionNotFollowingReason is set to null, verify that the ActivePresetHandle attribute is set to the PresetHandle field of the CurrentThermostatSuggestion attribute."),
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

        return steps

    @ async_test_body
    async def test_TC_TSTAT_4_3(self):
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        logger.info("Commissioning under second controller")
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=1234, option=1)

        secondary_authority = self.certificate_authority_manager.NewCertificateAuthority()
        secondary_fabric_admin = secondary_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        secondary_controller = secondary_fabric_admin.NewController(nodeId=112233)

        await secondary_controller.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=1234)

        secondary_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=secondary_controller, endpoint=0, cluster=Clusters.Objects.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)

        current_presets = []
        presetTypes = []
        presetScenarioCounts = {}
        numberOfPresetsSupported = 0
        minHeatSetpointLimit = 700
        maxHeatSetpointLimit = 3000
        minCoolSetpointLimit = 1600
        maxCoolSetpointLimit = 3200

        supportsHeat = self.check_pics("TSTAT.S.F00")
        supportsCool = self.check_pics("TSTAT.S.F01")

        if supportsHeat:
            # If the server supports MinHeatSetpointLimit & MaxHeatSetpointLimit, use those
            if self.check_pics("TSTAT.S.A0015") and self.check_pics("TSTAT.S.A0016"):
                minHeatSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
                maxHeatSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
            elif self.check_pics("TSTAT.S.A0003") and self.check_pics("TSTAT.S.A0004"):
                # Otherwise, if the server supports AbsMinHeatSetpointLimit & AbsMaxHeatSetpointLimit, use those
                minHeatSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinHeatSetpointLimit)
                maxHeatSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxHeatSetpointLimit)

            asserts.assert_true(minHeatSetpointLimit < maxHeatSetpointLimit, "Heat setpoint range invalid")

        if supportsCool:
            # If the server supports MinCoolSetpointLimit & MaxCoolSetpointLimit, use those
            if self.check_pics("TSTAT.S.A0017") and self.check_pics("TSTAT.S.A0018"):
                minCoolSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
                maxCoolSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
            elif self.check_pics("TSTAT.S.A0005") and self.check_pics("TSTAT.S.A0006"):
                # Otherwise, if the server supports AbsMinCoolSetpointLimit & AbsMaxCoolSetpointLimit, use those
                minCoolSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinCoolSetpointLimit)
                maxCoolSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxCoolSetpointLimit)

            asserts.assert_true(minCoolSetpointLimit < maxCoolSetpointLimit, "Cool setpoint range invalid")

        # Servers that do not support occupancy are always "occupied"
        occupied = True

        supportsOccupancy = self.check_pics("TSTAT.S.F02")
        if supportsOccupancy:
            occupied = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Occupancy) & 1

        # Target setpoints
        heatSetpoint = minHeatSetpointLimit + ((maxHeatSetpointLimit - minHeatSetpointLimit) / 2)
        coolSetpoint = minCoolSetpointLimit + ((maxCoolSetpointLimit - minCoolSetpointLimit) / 2)

        # Set the heating and cooling setpoints to something other than the target setpoints
        if occupied:
            if supportsHeat:
                await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(heatSetpoint-1), endpoint_id=endpoint)
            if supportsCool:
                await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(coolSetpoint-1), endpoint_id=endpoint)
        else:
            if supportsHeat:
                await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(heatSetpoint-1), endpoint_id=endpoint)
            if supportsCool:
                await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(coolSetpoint-1), endpoint_id=endpoint)

        self.step("2")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("3")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("4")


        self.step("5")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("6a")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("6b")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("7a")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("7b")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("7c")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("8a")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("8b")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("8c")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("9a")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("9b")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("9c")
        if self.pics_guard(self.check_pics("TSTAT.F09")):


        self.step("10")
        if self.pics_guard(self.check_pics("TSTAT.F09")):



        self.step("11")
        if self.pics_guard(self.check_pics("TSTAT.F09")):



        
if __name__ == "__main__":
    default_matter_test_main()
