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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import copy
import logging

import chip.clusters as Clusters
from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.Thermostat


initial_presets = []
initial_presets.append(cluster.Structs.PresetStruct(presetHandle=b'\x01', presetScenario=cluster.Enums.PresetScenarioEnum.kOccupied,
                       name=None, coolingSetpoint=2500, heatingSetpoint=2100, builtIn=True))
initial_presets.append(cluster.Structs.PresetStruct(presetHandle=b'\x02', presetScenario=cluster.Enums.PresetScenarioEnum.kUnoccupied,
                       name=None, coolingSetpoint=2600, heatingSetpoint=2000, builtIn=True))

new_presets = initial_presets.copy()
new_presets.append(cluster.Structs.PresetStruct(presetHandle=NullValue, presetScenario=cluster.Enums.PresetScenarioEnum.kSleep,
                   name="Sleep", coolingSetpoint=2700, heatingSetpoint=1900, builtIn=False))

new_presets_with_handle = initial_presets.copy()
new_presets_with_handle.append(cluster.Structs.PresetStruct(
    presetHandle=b'\x03', presetScenario=cluster.Enums.PresetScenarioEnum.kSleep, name="Sleep", coolingSetpoint=2700, heatingSetpoint=1900, builtIn=False))


class TC_TSTAT_4_2(MatterBaseTest):

    def check_atomic_response(self, response: object, expected_status: Status = Status.Success,
                              expected_overall_status: Status = Status.Success,
                              expected_preset_status: Status = Status.Success):
        asserts.assert_equal(expected_status, Status.Success, "We expected we had a valid response")
        asserts.assert_equal(response.statusCode, expected_overall_status, "Response should have the right overall status")
        found_preset_status = False
        for attrStatus in response.attributeStatus:
            if attrStatus.attributeID == cluster.Attributes.Presets.attribute_id:
                asserts.assert_equal(attrStatus.statusCode, expected_preset_status,
                                     "Preset attribute should have the right status")
                found_preset_status = True
        asserts.assert_true(found_preset_status, "Preset attribute should have a status")

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
                                                expected_status: Status = Status.Success,
                                                expected_overall_status: Status = Status.Success,
                                                expected_preset_status: Status = Status.Success):
        try:
            response = await self.send_single_cmd(cmd=cluster.Commands.AtomicRequest(requestType=Globals.Enums.AtomicRequestTypeEnum.kBeginWrite,
                                                                                     attributeRequests=[
                                                                                         cluster.Attributes.Presets.attribute_id],
                                                                                     timeout=1800),
                                                  dev_ctrl=dev_ctrl,
                                                  endpoint=endpoint)
            self.check_atomic_response(response, expected_status, expected_overall_status, expected_preset_status)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_atomic_request_commit_command(self,
                                                 dev_ctrl: ChipDeviceCtrl = None,
                                                 endpoint: int = None,
                                                 expected_status: Status = Status.Success,
                                                 expected_overall_status: Status = Status.Success,
                                                 expected_preset_status: Status = Status.Success):
        try:
            response = await self.send_single_cmd(cmd=cluster.Commands.AtomicRequest(requestType=Globals.Enums.AtomicRequestTypeEnum.kCommitWrite,
                                                                                     attributeRequests=[cluster.Attributes.Presets.attribute_id, cluster.Attributes.Schedules.attribute_id]),
                                                  dev_ctrl=dev_ctrl,
                                                  endpoint=endpoint)
            self.check_atomic_response(response, expected_status, expected_overall_status, expected_preset_status)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_atomic_request_rollback_command(self,
                                                   dev_ctrl: ChipDeviceCtrl = None,
                                                   endpoint: int = None,
                                                   expected_status: Status = Status.Success,
                                                   expected_overall_status: Status = Status.Success,
                                                   expected_preset_status: Status = Status.Success):
        try:
            response = await self.send_single_cmd(cmd=cluster.Commands.AtomicRequest(requestType=Globals.Enums.AtomicRequestTypeEnum.kRollbackWrite,
                                                                                     attributeRequests=[cluster.Attributes.Presets.attribute_id, cluster.Attributes.Schedules.attribute_id]),
                                                  dev_ctrl=dev_ctrl,
                                                  endpoint=endpoint)
            self.check_atomic_response(response, expected_status, expected_overall_status, expected_preset_status)
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
        return "3.2.4 [TC-TSTAT-4-2] Preset write and command attributes test case with server as DUT"

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
        ]

        return steps

    @ async_test_body
    async def test_TC_TSTAT_4_2(self):
        endpoint = self.user_params.get("endpoint", 1)

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

        self.step("2")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050")):
            presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            logger.info(f"Rx'd Presets: {presets}")
            asserts.assert_equal(presets, initial_presets, "Presets do not match initial value")

            # Write to the presets attribute without calling AtomicRequest command
            await self.write_presets(endpoint=endpoint, presets=new_presets, expected_status=Status.InvalidInState)

        self.step("3")

        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after calling AtomicRequest command
            status = await self.write_presets(endpoint=endpoint, presets=new_presets)
            status_ok = (status == Status.Success)
            asserts.assert_true(status_ok, "Presets write did not return Success as expected")

            # Read the presets attribute and verify it was updated by the write
            presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            logger.info(f"Rx'd Presets: {presets}")
            asserts.assert_equal(presets, new_presets_with_handle, "Presets were updated, as expected")

            await self.send_atomic_request_rollback_command()

            # Read the presets attribute and verify it has been properly rolled back
            presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            asserts.assert_equal(presets, initial_presets, "Presets were updated which is not expected")

        self.step("4")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after calling AtomicRequest command
            await self.write_presets(endpoint=endpoint, presets=new_presets)

            # Send the AtomicRequest commit command
            await self.send_atomic_request_commit_command()

            # Read the presets attribute and verify it was updated since AtomicRequest commit was called after writing presets
            presets = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
            logger.info(f"Rx'd Presets: {presets}")
            asserts.assert_equal(presets, new_presets_with_handle, "Presets were not updated which is not expected")

        self.step("5")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after removing a built in preset from the list. Remove the first entry.
            test_presets = new_presets_with_handle.copy()
            test_presets.pop(0)
            await self.write_presets(endpoint=endpoint, presets=test_presets)

            # Send the AtomicRequest commit command and expect ConstraintError for presets.
            await self.send_atomic_request_commit_command(expected_overall_status=Status.Failure, expected_preset_status=Status.ConstraintError)

        self.step("6")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.C06.Rsp") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the SetActivePresetRequest command
            await self.send_set_active_preset_handle_request_command(value=b'\x03')

            # Read the active preset handle attribute and verify it was updated to preset handle
            activePresetHandle = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle)
            logger.info(f"Rx'd ActivePresetHandle: {activePresetHandle}")
            asserts.assert_equal(activePresetHandle, b'\x03', "Active preset handle was not updated as expected")

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after removing the preset that was set as the active preset handle. Remove the last entry with preset handle (b'\x03')
            test_presets = new_presets_with_handle.copy()
            del test_presets[-1]
            await self.write_presets(endpoint=endpoint, presets=test_presets)

            # Send the AtomicRequest commit command and expect InvalidInState for presets.
            await self.send_atomic_request_commit_command(expected_overall_status=Status.Failure, expected_preset_status=Status.InvalidInState)

        self.step("7")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after setting the builtIn flag to False for preset with handle (b'\x01')
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets[0].builtIn = False

            await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("8")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after adding a preset with builtIn set to True
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets.append(cluster.Structs.PresetStruct(presetHandle=NullValue, presetScenario=cluster.Enums.PresetScenarioEnum.kWake,
                                name="Wake", coolingSetpoint=2800, heatingSetpoint=1800, builtIn=True))

            status = await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("9")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after adding a preset with a preset handle that doesn't exist in Presets attribute
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets.append(cluster.Structs.PresetStruct(presetHandle=b'\x08', presetScenario=cluster.Enums.PresetScenarioEnum.kWake,
                                name="Wake", coolingSetpoint=2800, heatingSetpoint=1800, builtIn=True))

            status = await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.NotFound)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("10")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after adding a duplicate preset with handle (b'\x03')
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets.append(cluster.Structs.PresetStruct(
                presetHandle=b'\x03', presetScenario=cluster.Enums.PresetScenarioEnum.kSleep, name="Sleep", coolingSetpoint=2700, heatingSetpoint=1900, builtIn=False))

            await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("11")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after setting the builtIn flag to True for preset with handle (b'\x03')
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets[2].builtIn = True

            await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("12")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute after setting a name for preset with handle (b'\x01') that doesn't support names
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets[0].name = "Occupied"

            await self.write_presets(endpoint=endpoint, presets=test_presets, expected_status=Status.ConstraintError)

            # Clear state for next test.
            await self.send_atomic_request_rollback_command()

        self.step("13")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Write to the presets attribute with a new valid preset added
            test_presets = copy.deepcopy(new_presets_with_handle)
            test_presets.append(cluster.Structs.PresetStruct(presetHandle=NullValue, presetScenario=cluster.Enums.PresetScenarioEnum.kWake,
                                name="Wake", coolingSetpoint=2800, heatingSetpoint=1800, builtIn=False))

            await self.write_presets(endpoint=endpoint, presets=test_presets)

            # Roll back
            await self.send_atomic_request_rollback_command()

            # Send the AtomicRequest commit command and expect InvalidInState as the previous edit request was cancelled
            await self.send_atomic_request_commit_command(expected_status=Status.InvalidInState)

        self.step("14")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command
            await self.send_atomic_request_begin_command()

            # Send the AtomicRequest begin command from separate controller, which should receive busy
            status = await self.send_atomic_request_begin_command(dev_ctrl=secondary_controller, expected_overall_status=Status.Failure, expected_preset_status=Status.Busy)

            # Roll back
            await self.send_atomic_request_rollback_command()

        self.step("15")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):
            # Send the AtomicRequest begin command from the secondary controller
            await self.send_atomic_request_begin_command()

            await self.write_presets(endpoint=endpoint, presets=test_presets, dev_ctrl=secondary_controller, expected_status=Status.Busy)

            # Roll back
            await self.send_atomic_request_rollback_command()

        self.step("16")
        if self.pics_guard(self.check_pics("TSTAT.S.F08") and self.check_pics("TSTAT.S.A0050") and self.check_pics("TSTAT.S.Cfe.Rsp")):

            # Send the AtomicRequest begin command from the secondary controller
            await self.send_atomic_request_begin_command(dev_ctrl=secondary_controller)

            # Primary controller removes the second fabric
            await self.send_single_cmd(Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=2),  endpoint=0)

            # Send the AtomicRequest begin command from primary controller, which should succeed, as the secondary controller's atomic write state has been cleared
            status = await self.send_atomic_request_begin_command()

            # Roll back
            await self.send_atomic_request_rollback_command()

        # TODO: Add tests for the total number of Presets exceeds the NumberOfPresets supported. Also Add tests for adding presets with preset scenario not present in PresetTypes.


if __name__ == "__main__":
    default_matter_test_main()
