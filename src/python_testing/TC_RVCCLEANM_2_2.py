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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_RVC_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS examples/rvc-app/rvc-common/pics/rvc-app-pics-values --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import enum

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class RvcStatusEnum(enum.IntEnum):
    # TODO remove this class once InvalidInMode response code is implemented in python SDK
    Success = 0x0
    UnsupportedMode = 0x1
    GenericFailure = 0x2
    InvalidInMode = 0x3


class TC_RVCCLEANM_2_2(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = 0
        self.supported_run_modes = {}  # these are the ModeOptionStructs
        self.supported_run_modes_dut = []
        self.supported_clean_modes_dut = []
        self.run_mode_dut = 0
        self.old_clean_mode_dut = 0
        self.new_clean_mode_th = 0
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_mod_attribute_expect_success(self, cluster, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def read_run_supported_modes(self) -> Clusters.Objects.RvcRunMode.Attributes.SupportedModes:
        ret = await self.read_mod_attribute_expect_success(
            Clusters.RvcRunMode,
            Clusters.RvcRunMode.Attributes.SupportedModes)
        return ret

    async def read_clean_supported_modes(self) -> Clusters.Objects.RvcCleanMode.Attributes.SupportedModes:
        ret = await self.read_mod_attribute_expect_success(
            Clusters.RvcCleanMode,
            Clusters.RvcCleanMode.Attributes.SupportedModes)
        return ret

    async def read_feature_map_attribute(self):
        ret = await self.read_mod_attribute_expect_success(Clusters.RvcCleanMode,
                                                           Clusters.RvcCleanMode.Attributes.FeatureMap)
        return ret

    async def send_clean_change_to_mode_cmd(self, newMode) -> Clusters.Objects.RvcCleanMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcCleanMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        return ret

    async def send_run_change_to_mode_cmd(self, newMode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        return ret

    # Prints the instruction and waits for a user input to continue
    def print_instruction(self, step_number, instruction):
        self.print_step(step_number, instruction)
        self.wait_for_user_input(prompt_msg=f"{instruction}, and press Enter when ready.")

    def pics_TC_RVCCLEANM_2_2(self) -> list[str]:
        return ["RVCCLEANM.S"]

    @async_test_body
    async def test_TC_RVCCLEANM_2_2(self):
        self.directmodech_bit_mask = Clusters.RvcCleanMode.Bitmaps.Feature.kDirectModeChange
        self.endpoint = self.matter_test_config.endpoint
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set.c")
            self.app_pipe = self.app_pipe + str(app_pid)

        asserts.assert_true(self.check_pics("RVCCLEANM.S"), "RVCCLEANM.S must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.A0000"), "RVCRUNM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.A0001"), "RVCRUNM.S.A0001 must be supported")

        self.print_step(1, "Commissioning, already done")

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe({"Name": "Reset"})

        self.print_step(
            2, "Manually put the device in a state in which the RVC Run Mode cluster’s CurrentMode attribute is set to a mode without the Idle mode tag.")
        if self.is_ci:
            await self.send_run_change_to_mode_cmd(1)
        else:
            self.wait_for_user_input(
                prompt_msg="Manually put the device in a state in which the RVC Run Mode cluster’s CurrentMode attribute is set to a mode without the Idle mode tag, and press Enter when done.")

        self.print_step(3, "Read the RvcRunMode SupportedModes attribute")
        supported_run_modes = await self.read_run_supported_modes()
        for mode in supported_run_modes:
            self.supported_run_modes[mode.mode] = mode
            # Save the Mode field values as supported_run_modes_dut
            self.supported_run_modes_dut.append(mode.mode)

        self.print_step(4, "Read the RvcRunMode CurrentMode attribute")
        current_run_mode = await self.read_mod_attribute_expect_success(
            Clusters.RvcRunMode,
            Clusters.RvcRunMode.Attributes.CurrentMode)

        # Save the value as run_mode_dut
        self.run_mode_dut = current_run_mode

        # Verify that the supported_run_modes_dut entry matching run_mode_dut does not have the Idle (0x4000) mode tag.
        for t in self.supported_run_modes[current_run_mode].modeTags:
            asserts.assert_true(t.value != Clusters.RvcRunMode.Enums.ModeTag.kIdle,
                                "The device must be in a mode without the Idle (0x4000) mode tag.")

        self.print_step(5, "Read the RvcCleanMode SupportedModes attribute")
        supported_clean_modes = await self.read_clean_supported_modes()
        for mode in supported_clean_modes:
            # Save the Mode field values as supported_run_modes_dut
            self.supported_clean_modes_dut.append(mode.mode)

        self.print_step(6, "Read the RvcCleanMode CurrentMode attribute")
        current_clean_mode = await self.read_mod_attribute_expect_success(
            Clusters.RvcCleanMode,
            Clusters.RvcCleanMode.Attributes.CurrentMode)

        # Save the value as old_clean_mode_dut
        self.old_clean_mode_dut = current_clean_mode

        # Select from supported_clean_modes_dut a value which is different from old_clean_mode_dut,
        # and save it as new_clean_mode_th
        for mode in self.supported_clean_modes_dut:
            if mode != self.old_clean_mode_dut:
                self.new_clean_mode_th = mode
                break

        self.print_step("7a", "Read FeatureMap Attribute")
        feature_map = await self.read_feature_map_attribute()
        directmode_enabled = feature_map & self.directmodech_bit_mask

        self.print_step("7b", "Send ChangeToMode command")
        response = await self.send_clean_change_to_mode_cmd(self.new_clean_mode_th)
        asserts.assert_true(type_matches(response, Clusters.RvcCleanMode.Commands.ChangeToModeResponse),
                            "The response should ChangeToModeResponse command")
        if directmode_enabled:
            asserts.assert_equal(response.status, RvcStatusEnum.Success,
                                 "The response should contain a ChangeToModeResponse command "
                                 "with the Status set to Success(0x0).")
        else:
            asserts.assert_equal(response.status, RvcStatusEnum.InvalidInMode,
                                 "The response should contain a ChangeToModeResponse command "
                                 "with the Status set to InvalidInMode(0x03).")


if __name__ == "__main__":
    default_matter_test_main()
