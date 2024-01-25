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

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments.
# Run the test with
# --int-arg PIXIT.RVCRUNM.MODE_A:<mode id> PIXIT.RVCRUNM.MODE_B:<mode id>


def error_enum_to_text(error_enum):
    try:
        return f'{Clusters.RvcRunMode.Enums.ModeTag(error_enum).name} 0x{error_enum:02x}'
    except AttributeError:
        if error_enum == 0:
            return "Success(0x00)"
        elif error_enum == 0x1:
            return "UnsupportedMode(0x01)"
        elif error_enum == 0x2:
            return "GenericFailure(0x02)"
        elif error_enum == 0x3:
            return "InvalidInMode(0x03)"

    raise AttributeError("Unknown Enum value")


class TC_RVCRUNM_2_2(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = 0
        self.mode_a = 0
        self.mode_b = 0
        self.supported_run_modes = {}  # these are the ModeOptionStructs
        self.supported_run_modes_dut = []
        self.idle_mode_dut = 0

    async def read_mod_attribute_expect_success(self, cluster, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def read_run_supported_modes(self) -> Clusters.Objects.RvcRunMode.Attributes.SupportedModes:
        ret = await self.read_mod_attribute_expect_success(
            Clusters.RvcRunMode,
            Clusters.RvcRunMode.Attributes.SupportedModes)
        return ret

    async def read_current_mode_with_check(self, expected_mode):
        run_mode = await self.read_mod_attribute_expect_success(
            Clusters.RvcRunMode,
            Clusters.RvcRunMode.Attributes.CurrentMode)
        asserts.assert_true(run_mode == expected_mode,
                            "Expected the current mode to be %i, got %i" % (expected_mode, run_mode))

    async def send_change_to_mode_cmd(self, new_mode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=new_mode),
                                         endpoint=self.endpoint)
        return ret

    async def send_change_to_mode_with_check(self, new_mode, expected_error):
        response = await self.send_change_to_mode_cmd(new_mode)
        asserts.assert_true(response.status == expected_error,
                            "Expected a ChangeToMode response status of %s, got %s" %
                            (error_enum_to_text(expected_error), error_enum_to_text(response.status)))

    # Prints the instruction and waits for a user input to continue
    def print_instruction(self, step_number, instruction):
        self.print_step(step_number, instruction)
        input("Press Enter when done.\n")

    def pics_TC_RVCRUNM_2_2(self) -> list[str]:
        return ["RVCRUNM.S"]

    @async_test_body
    async def test_TC_RVCRUNM_2_2(self):

        if 'PIXIT.RVCRUNM.MODE_A' not in self.matter_test_config.global_test_params or \
                'PIXIT.RVCRUNM.MODE_B' not in self.matter_test_config.global_test_params:
            asserts.fail("There are missing arguments to the `--int-arg` flag! "
                         "Make sure that all of these arguments are given to this flag: \n"
                         "PIXIT.RVCRUNM.MODE_A:<mode id> \n"
                         "PIXIT.RVCRUNM.MODE_B:<mode id>")

        self.endpoint = self.matter_test_config.endpoint
        self.mode_a = self.matter_test_config.global_test_params['PIXIT.RVCRUNM.MODE_A']
        self.mode_b = self.matter_test_config.global_test_params['PIXIT.RVCRUNM.MODE_B']

        asserts.assert_true(self.check_pics("RVCRUNM.S"), "RVCRUNM.S must be supported")
        # I think that the following PICS should be listed in the preconditions section in the test plan as if either
        # of these PICS is not supported, this test would not be useful.
        asserts.assert_true(self.check_pics("RVCRUNM.S.A0000"), "RVCRUNM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.A0001"), "RVCRUNM.S.A0001 must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.C00.Rsp"), "RVCRUNM.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.M.CAN_MANUALLY_CONTROLLED"),
                            "RVCRUNM.S.M.CAN_MANUALLY_CONTROLLED must be supported")

        # Starting the test steps
        self.print_step(1, "Commissioning, already done")

        self.print_instruction(2, "Manually put the device in a RVC Run Mode cluster mode with "
                                  "the Idle(0x4000) mode tag and in a device state that allows changing to either "
                                  "of these modes: %i, %i" % (self.mode_a, self.mode_b))

        self.print_step(3, "Read the RvcRunMode SupportedModes attribute")
        supported_run_modes = await self.read_run_supported_modes()
        for mode in supported_run_modes:
            self.supported_run_modes[mode.mode] = mode
            # Save the Mode field values as supported_run_modes_dut
            self.supported_run_modes_dut.append(mode.mode)

        asserts.assert_false(self.mode_a == self.mode_b,
                             "PIXIT.RVCRUNM.MODE_A must be different from PIXIT.RVCRUNM.MODE_B")

        if self.mode_a not in self.supported_run_modes_dut or \
                self.mode_b not in self.supported_run_modes_dut:
            asserts.fail("PIXIT.RVCRUNM.MODE_A and PIXIT.RVCRUNM.MODE_B must be valid supported modes.")

        for tag in self.supported_run_modes[self.mode_a].modeTags:
            if tag.value == Clusters.RvcRunMode.Enums.ModeTag.kIdle:
                asserts.fail("The mode chosen for PIXIT.RVCRUNM.MODE_A must not have the Idle(0x4000) mode tag.")

        for tag in self.supported_run_modes[self.mode_b].modeTags:
            if tag.value == Clusters.RvcRunMode.Enums.ModeTag.kIdle:
                asserts.fail("The mode chosen for PIXIT.RVCRUNM.MODE_B must not have the Idle(0x4000) mode tag.")

        self.print_step(4, "Read the RvcRunMode CurrentMode attribute")
        current_run_mode = await self.read_mod_attribute_expect_success(
            Clusters.RvcRunMode,
            Clusters.RvcRunMode.Attributes.CurrentMode)

        # Save the value as idle_mode_dut
        self.idle_mode_dut = current_run_mode

        # Verify that the supported_run_modes_dut entry matching idle_mode_dut has the Idle (0x4000) mode tag.
        idle_tag_present = False
        for t in self.supported_run_modes[current_run_mode].modeTags:
            if t.value == Clusters.RvcRunMode.Enums.ModeTag.kIdle:
                idle_tag_present = True
        asserts.assert_true(idle_tag_present, "The device must be in a mode with the Idle (0x4000) mode tag.")

        self.print_step(5, "Send ChangeToMode MODE_A command")
        await self.send_change_to_mode_with_check(self.mode_a, 0)
        # This step is not described in the test plan, but it ought to be
        await self.read_current_mode_with_check(self.mode_a)

        self.print_step(6, "Send ChangeToMode MODE_B command")
        await self.send_change_to_mode_with_check(self.mode_b, 3)

        self.print_step(7, "Send ChangeToMode idle command")
        await self.send_change_to_mode_with_check(self.idle_mode_dut, 0)
        # This step is not described in the test plan, but it ought to be
        await self.read_current_mode_with_check(self.idle_mode_dut)

        self.print_step(8, "Send ChangeToMode MODE_B command")
        await self.send_change_to_mode_with_check(self.mode_b, 0)
        # This step is not described in the test plan, but it ought to be
        await self.read_current_mode_with_check(self.mode_b)

        self.print_step(9, "Send ChangeToMode idle command")
        await self.send_change_to_mode_with_check(self.idle_mode_dut, 0)
        # This step is not described in the test plan, but it ought to be
        await self.read_current_mode_with_check(self.idle_mode_dut)


if __name__ == "__main__":
    default_matter_test_main()
