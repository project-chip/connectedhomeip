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


class TC_TSTAT_4_2(MatterBaseTest):

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

    def desc_TC_TSTAT_4_2(self) -> str:
        """Returns a description of this test"""
        return "3.2.4 [TC-TSTAT-4-2] Test cases to read/write attributes and invoke commands for Preset feature with server as DUT"

    def pics_TC_TSTAT_4_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["TSTAT.S"]

    def steps_TC_TSTAT_4_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH writes to the Presets attribute without calling the AtomicRequest command",
                     " Verify that the write request returns INVALID_IN_STATE error since the client didn't send a request to edit the presets by calling AtomicRequest command."),
            TestStep("3", "TH writes to the Presets attribute after calling the AtomicRequest begin command but doesn't call AtomicRequest commit",
                     "Verify that the Presets attribute was not updated since AtomicRequest commit command was not called."),
            TestStep("4", "TH writes to the Presets attribute after calling the AtomicRequest begin command and calls AtomicRequest commit",
                     "Verify that the Presets attribute was updated with new presets."),
            TestStep("5", "TH writes to the Presets attribute with a built-in preset removed",
                     "Verify that the AtomicRequest commit returned CONSTRAINT_ERROR (0x87)."),
            TestStep("6", "TH writes to the Presets attribute with a preset removed whose handle matches the value in the ActivePresetHandle attribute",
                     "Verify that the AtomicRequest commit returned INVALID_IN_STATE (0xcb)."),
            TestStep("7", "TH writes to the Presets attribute with a built-in preset modified to be not built-in",
                     "Verify that the AtomicRequest commit returned CONSTRAINT_ERROR (0x87)."),
            TestStep("8", "TH writes to the Presets attribute with a new preset having builtIn set to true",
                     "Verify that the AtomicRequest commit returned CONSTRAINT_ERROR (0x87)."),
            TestStep("9", "TH writes to the Presets attribute with a new preset having a preset handle that doesn't exist in the Presets attribute",
                     "Verify that the AtomicRequest commit returned NOT_FOUND (0x8b)."),
            TestStep("10", "TH writes to the Presets attribute with duplicate presets",
                     "Verify that the AtomicRequest commit returned CONSTRAINT_ERROR (0x87)."),
            TestStep("11", "TH writes to the Presets attribute with a non built-in preset modified to be built-in",
                     "Verify that the AtomicRequest commit returned CONSTRAINT_ERROR (0x87)."),
            TestStep("12", "TH writes to the Presets attribute with a preset that doesn't support names in the PresetTypeFeatures bitmap but has a name",
                     "Verify that the AtomicRequest commit returned CONSTRAINT_ERROR (0x87)."),
            TestStep("13", "TH writes to the Presets attribute but calls the AtomicRequest rollback command to cancel the edit request",
                     "Verify that the edit request was rolled back"),
            TestStep("14", "TH starts an atomic write, and TH2 attempts to open an atomic write before TH is complete",
                     "Verify that the atomic request is rejected"),
            TestStep("15", "TH starts an atomic write, and TH2 attempts to write to presets",
                     "Verify that the write request is rejected"),
            TestStep("16", "TH starts an atomic write, and before it's complete, TH2 removes TH's fabric; TH2 then opens an atomic write",
                     "Verify that the atomic request is successful"),
            TestStep("17", "TH writes to the Presets attribute with a preset that has a presetScenario not present in PresetTypes attribute",
                     "Verify that the write request returned CONSTRAINT_ERROR (0x87)."),
            TestStep("18", "TH writes to the Presets attribute such that the total number of presets is greater than the number of presets supported",
                     "Verify that the write request returned RESOURCE_EXHAUSTED (0x89)."),
        ]

        return steps

    @ async_test_body
    async def test_TC_TSTAT_4_2(self):
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
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050")):

            # Read the numberOfPresets supported.
            numberOfPresetsSupported = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfPresets)

            # Read the PresetTypes to get the preset scenarios supported by the Thermostat.
            presetTypes = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PresetTypes)
            logger.info(f"Rx'd Preset Types: {presetTypes}")

            current_presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            logger.info(f"Rx'd Presets: {current_presets}")

            presetScenarioCounts = self.count_preset_scenarios(current_presets)

            # Write to the presets attribute without calling AtomicRequest command
            await self.write_presets(endpoint=endpoint, presets=current_presets, expected_status=Status.InvalidInState)

        self.step("3")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            availableScenario = self.get_available_scenario(presetTypes=presetTypes, presetScenarioCounts=presetScenarioCounts)

            if availableScenario is not None and len(current_presets) < numberOfPresetsSupported:

                # Set the preset builtIn fields to a null built-in value
                test_presets = copy.deepcopy(current_presets)
                for preset in test_presets:
                    preset.builtIn = NullValue

                test_presets.append(self.make_preset(availableScenario, coolSetpoint, heatSetpoint))

                await self.send_atomic_request_begin_command()

                # Write to the presets attribute after calling AtomicRequest command
                status = await self.write_presets(endpoint=endpoint, presets=test_presets)
                status_ok = (status == Status.Success)
                asserts.assert_true(status_ok, "Presets write did not return Success as expected")

                # Read the presets attribute and verify it was updated by the write
                saved_presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
                logger.info(f"Rx'd Presets: {saved_presets}")
                self.check_returned_presets(test_presets, saved_presets)

                await self.send_atomic_request_rollback_command()

                # Read the presets attribute and verify it has been properly rolled back
                presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
                asserts.assert_equal(presets, current_presets, "Presets were updated which is not expected")
            else:
                logger.info(
                    "Couldn't run test step 3 since there was no available preset scenario to append")

        self.step("4")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            availableScenario = self.get_available_scenario(presetTypes=presetTypes, presetScenarioCounts=presetScenarioCounts)

            # Set the existing preset to a null built-in value; will be replaced with true on reading
            test_presets = copy.deepcopy(current_presets)

            if availableScenario is not None:
                builtInPresets = list(preset for preset in test_presets if preset.builtIn)

                if len(builtInPresets) > 0:
                    builtInPresets[0].builtIn = NullValue

                test_presets.append(self.make_preset(availableScenario, coolSetpoint, heatSetpoint))

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                # Write to the presets attribute after calling AtomicRequest command
                await self.write_presets(endpoint=endpoint, presets=test_presets)

                # Send the AtomicRequest commit command
                await self.send_atomic_request_commit_command()

                # Read the presets attribute and verify it was updated since AtomicRequest commit was called after writing presets
                current_presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
                logger.info(f"Rx'd Presets: {current_presets}")
                self.check_returned_presets(test_presets, current_presets)

                presetScenarioCounts = self.count_preset_scenarios(current_presets)
            else:
                logger.info(
                    "Couldn't run test step 4 since there were no built-in presets")

        self.step("5")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Write to the presets attribute after removing a built in preset from the list. Remove the first entry.
            test_presets = current_presets.copy()

            builtInPresets = list(preset for preset in test_presets if preset.builtIn)
            if len(builtInPresets) > 0:
                builtInPreset = builtInPresets[0]
                test_presets.remove(builtInPreset)

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command(timeout=5000, expected_timeout=3000)

                # Write to the presets attribute after calling AtomicRequest command
                await self.write_presets(endpoint=endpoint, presets=test_presets)

                # Send the AtomicRequest commit command and expect ConstraintError for presets.
                await self.send_atomic_request_commit_command(expected_overall_status=Status.Failure, expected_preset_status=Status.ConstraintError)
            else:
                logger.info(
                    "Couldn't run test step 5 since there were no built-in presets")

        self.step("6")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.C06.Rsp") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            notBuiltInPresets = list(preset for preset in current_presets if preset.builtIn is False)
            if len(notBuiltInPresets) > 0:
                activePreset = notBuiltInPresets[0]

                # Send the SetActivePresetRequest command
                await self.send_set_active_preset_handle_request_command(value=activePreset.presetHandle)

                # Read the active preset handle attribute and verify it was updated to preset handle
                activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
                logger.info(f"Rx'd ActivePresetHandle: {activePresetHandle}")
                asserts.assert_equal(activePresetHandle, activePreset.presetHandle,
                                     "Active preset handle was not updated as expected")

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                # Write to the presets attribute after removing the preset that was set as the active preset handle.
                test_presets = list(preset for preset in current_presets if preset.presetHandle is not activePresetHandle)
                logger.info(f"Sending Presets: {test_presets}")
                await self.write_presets(endpoint=endpoint, presets=test_presets)

                # Send the AtomicRequest commit command and expect InvalidInState for presets.
                await self.send_atomic_request_commit_command(expected_overall_status=Status.Failure, expected_preset_status=Status.InvalidInState)
            else:
                logger.info(
                    "Couldn't run test step 6 since there were no non-built-in presets to activate and delete")

            # Write the occupied cooling setpoint to a different value
            if occupied:
                if supportsHeat:
                    await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(heatSetpoint+1), endpoint_id=endpoint)
                elif supportsCool:
                    await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(coolSetpoint+1), endpoint_id=endpoint)
            else:
                if supportsHeat:
                    await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(heatSetpoint+1), endpoint_id=endpoint)
                elif supportsCool:
                    await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(coolSetpoint+1), endpoint_id=endpoint)

            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Rx'd ActivePresetHandle: {activePresetHandle}")
            asserts.assert_equal(activePresetHandle, NullValue, "Active preset handle was not cleared as expected")

        self.step("7")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Write to the presets attribute after setting the builtIn flag to False for a built-in preset.
            test_presets = copy.deepcopy(current_presets)

            builtInPresets = list(preset for preset in test_presets if preset.builtIn is True)
            if len(builtInPresets) > 0:

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                builtInPresets[0].builtIn = False

                await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 7 since there was no built-in presets")

        self.step("8")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            availableScenario = self.get_available_scenario(presetTypes=presetTypes, presetScenarioCounts=presetScenarioCounts)

            if len(current_presets) < numberOfPresetsSupported and availableScenario is not None:

                test_presets = copy.deepcopy(current_presets)

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                # Write to the presets attribute after adding a preset with builtIn set to True
                test_presets.append(self.make_preset(availableScenario, coolSetpoint, heatSetpoint, builtIn=True))

                status = await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 8 since there was no available preset scenario to append")

        self.step("9")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after adding a preset with a preset handle that doesn't exist in Presets attribute
            test_presets = copy.deepcopy(current_presets)
            test_presets.append(self.make_preset(cluster.Enums.PresetScenarioEnum.kWake, coolSetpoint,
                                heatSetpoint,  presetHandle=random.randbytes(16), name="Wake",  builtIn=True))

            status = await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.NotFound)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("10")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            availableScenario = self.get_available_scenario(presetTypes=presetTypes, presetScenarioCounts=presetScenarioCounts)

            if len(current_presets) < numberOfPresetsSupported and availableScenario is not None:

                test_presets = copy.deepcopy(current_presets)
                duplicatePreset = test_presets[0]

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                # Write to the presets attribute after adding a duplicate preset
                test_presets.append(self.make_preset(availableScenario, coolSetpoint,
                                    heatSetpoint, presetHandle=duplicatePreset.presetHandle))

                await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 10 since there was no available preset scenario to duplicate")

        self.step("11")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            test_presets = copy.deepcopy(current_presets)

            notBuiltInPresets = list(preset for preset in test_presets if preset.builtIn is False)

            if len(notBuiltInPresets) > 0:
                # Write to the presets attribute after setting the builtIn flag to True for a non-built-in preset.
                notBuiltInPresets[0].builtIn = True

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 11 since there were no presets that were not built-in")

        self.step("12")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            availableScenarios = list(presetType.presetScenario for presetType in presetTypes if (presetType.presetTypeFeatures & cluster.Bitmaps.PresetTypeFeaturesBitmap.kSupportsNames) == 0 and presetScenarioCounts.get(
                presetType.presetScenario, 0) <= presetType.numberOfPresets)

            test_presets = copy.deepcopy(current_presets)
            presets_without_name_support = list(preset for preset in test_presets if preset.presetScenario in availableScenarios)

            if len(presets_without_name_support) == 0 and len(availableScenarios) > 0:
                new_preset = self.make_preset(availableScenarios[0], coolSetpoint, heatSetpoint, builtIn=True)
                test_presets.append(new_preset)
                presets_without_name_support = [new_preset]

            if len(availableScenarios) > 0:

                # Write to the presets attribute after setting a name for preset with handle (b'\x01') that doesn't support names
                presets_without_name_support[0].name = "Name"

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 12 since there was no available preset scenario without name support")

        self.step("13")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            availableScenario = self.get_available_scenario(presetTypes=presetTypes, presetScenarioCounts=presetScenarioCounts)

            if len(current_presets) < numberOfPresetsSupported and availableScenario is not None:

                # Write to the presets attribute with a new valid preset added
                test_presets = copy.deepcopy(current_presets)
                test_presets.append(self.make_preset(availableScenario, coolSetpoint, heatSetpoint))

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                await self.write_presets(endpoint=endpoint, presets=test_presets)

                # Roll back
                await self.send_atomic_request_rollback_command()

                # Send the AtomicRequest commit command and expect InvalidInState as the previous edit request was cancelled
                await self.send_atomic_request_commit_command(expected_status=Status.InvalidInState)
            else:
                logger.info(
                    "Couldn't run test step 13 since there was no available preset scenario to add")

        self.step("14")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):
            await self.send_atomic_request_begin_command()
            # Send the AtomicRequest begin command from separate controller, which should receive busy
            status = await self.send_atomic_request_begin_command(dev_ctrl=secondary_controller, expected_overall_status=Status.Failure, expected_preset_status=Status.Busy)

            # Roll back
            await self.send_atomic_request_rollback_command()

        self.step("15")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):
            # Send the AtomicRequest begin command from the secondary controller
            await self.send_atomic_request_begin_command()

            await self.write_presets(endpoint=endpoint, presets=current_presets, dev_ctrl=secondary_controller, expected_status=Status.Busy)

            # Roll back
            await self.send_atomic_request_rollback_command()

        self.step("16")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command from the secondary controller
            await self.send_atomic_request_begin_command(dev_ctrl=secondary_controller)

            # Primary controller removes the second fabric
            await self.send_single_cmd(Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=secondary_fabric_index),  endpoint=0)

            # Send the AtomicRequest begin command from primary controller, which should succeed, as the secondary controller's atomic write state has been cleared
            status = await self.send_atomic_request_begin_command()

            # Roll back
            await self.send_atomic_request_rollback_command()

        self.step("17")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Find a preset scenario not present in PresetTypes to run this test.
            supportedScenarios = set(presetType.presetScenario for presetType in presetTypes)
            unavailableScenarios = list(
                presetScenario for presetScenario in cluster.Enums.PresetScenarioEnum if presetScenario not in supportedScenarios)
            if len(unavailableScenarios) > 0:
                test_presets = current_presets.copy()
                test_presets.append(self.make_preset(unavailableScenarios[0], coolSetpoint, heatSetpoint, name="Preset"))

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 17 since all preset scenarios in PresetScenarioEnum are supported by this Thermostat")

        self.step("18")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            ScenarioHeadroom = namedtuple("ScenarioHeadroom", "presetScenario remaining")
            # Generate list of tuples of scenarios and number of remaining presets per scenario allowed
            presetScenarioHeadrooms = list(ScenarioHeadroom(presetType.presetScenario,
                                           presetType.numberOfPresets - presetScenarioCounts.get(presetType.presetScenario, 0)) for presetType in presetTypes)

            if presetScenarioHeadrooms:
                # Find the preset scenario with the smallest number of remaining allowed presets
                presetScenarioHeadrooms = sorted(presetScenarioHeadrooms, key=lambda psh: psh.remaining)
                presetScenarioHeadroom = presetScenarioHeadrooms[0]

                # Add one more preset than is allowed by the preset type
                test_presets = copy.deepcopy(current_presets)
                test_presets.extend([cluster.Structs.PresetStruct(presetHandle=NullValue, presetScenario=presetScenarioHeadroom.presetScenario,
                                                                  coolingSetpoint=coolSetpoint, heatingSetpoint=heatSetpoint, builtIn=False)] * (presetScenarioHeadroom.remaining + 1))

                await self.send_atomic_request_begin_command()

                await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ResourceExhausted)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()

            # Calculate the length of the Presets list that could be created using the preset scenarios in PresetTypes and numberOfPresets supported for each scenario.
            totalExpectedPresetsLength = sum(presetType.numberOfPresets for presetType in presetTypes)

            if totalExpectedPresetsLength > numberOfPresetsSupported:
                testPresets = []
                for presetType in presetTypes:
                    scenario = presetType.presetScenario

                    # For each supported scenario, copy all the existing presets that match it, then add more presets
                    # until we hit the cap on the number of presets for that scenario.
                    presetsAddedForScenario = 0
                    for preset in current_presets:
                        if scenario == preset.presetScenario:
                            testPresets.append(preset)
                            presetsAddedForScenario = presetsAddedForScenario + 1

                    while presetsAddedForScenario < presetType.numberOfPresets:
                        testPresets.append(self.make_preset(scenario, coolSetpoint, heatSetpoint))
                        presetsAddedForScenario = presetsAddedForScenario + 1

                # Send the AtomicRequest begin command
                await self.send_atomic_request_begin_command()

                await self.write_presets(endpoint=endpoint, presets=testPresets, expected_status=Status.ResourceExhausted)

                # Clear state for next test.
                await self.send_atomic_request_rollback_command()
            else:
                logger.info(
                    "Couldn't run test step 18 since there are not enough preset types to build a Presets list that exceeds the number of presets supported")


if __name__ == "__main__":
    default_matter_test_main()
