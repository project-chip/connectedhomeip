#
#    Copyright (c) 2023 Project CHIP Authors
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

import logging
from time import sleep

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT.RVCCLEANM.MODE_CHANGE_OK:<mode id> PIXIT.RVCCLEANM.MODE_CHANGE_FAIL:<mode id>


class TC_RVCCLEANM_2_1(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = 0
        self.mode_ok = 0
        self.mode_fail = 0
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcCleanMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_clean_change_to_mode_cmd(self, newMode) -> Clusters.Objects.RvcCleanMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcCleanMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcCleanMode.Commands.ChangeToModeResponse),
                            "Unexpected return type for RVC Clean Mode ChangeToMode")
        return ret

    async def send_run_change_to_mode_cmd(self, newMode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse),
                            "Unexpected return type for RVC Run Mode ChangeToMode")
        return ret

    # Sends and out-of-band command to the rvc-app
    def write_to_app_pipe(self, command):
        with open(self.app_pipe, "w") as app_pipe:
            app_pipe.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests are flaky)
        # TODO(#31239): centralize pipe write logic and remove the need of sleep
        sleep(0.001)

    def pics_TC_RVCCLEANM_2_1(self) -> list[str]:
        return ["RVCCLEANM.S"]

    @async_test_body
    async def test_TC_RVCCLEANM_2_1(self):

        asserts.assert_true('PIXIT.RVCCLEANM.MODE_CHANGE_OK' in self.matter_test_config.global_test_params,
                            "PIXIT.RVCCLEANM.MODE_CHANGE_OK must be included on the command line in "
                            "the --int-arg flag as PIXIT.RVCCLEANM.MODE_CHANGE_OK:<mode id>")
        asserts.assert_true('PIXIT.RVCCLEANM.MODE_CHANGE_FAIL' in self.matter_test_config.global_test_params,
                            "PIXIT.RVCCLEANM.MODE_CHANGE_FAIL must be included on the command line in "
                            "the --int-arg flag as PIXIT.RVCCLEANM.MODE_CHANGE_FAIL:<mode id>")

        self.endpoint = self.matter_test_config.endpoint
        self.mode_ok = self.matter_test_config.global_test_params['PIXIT.RVCCLEANM.MODE_CHANGE_OK']
        self.mode_fail = self.matter_test_config.global_test_params['PIXIT.RVCCLEANM.MODE_CHANGE_FAIL']
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set")
            self.app_pipe = self.app_pipe + str(app_pid)

        asserts.assert_true(self.check_pics("RVCCLEANM.S.A0000"), "RVCCLEANM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("RVCCLEANM.S.A0001"), "RVCCLEANM.S.A0001 must be supported")
        asserts.assert_true(self.check_pics("RVCCLEANM.S.C00.Rsp"), "RVCCLEANM.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("RVCCLEANM.S.C01.Tx"), "RVCCLEANM.S.C01.Tx must be supported")

        attributes = Clusters.RvcCleanMode.Attributes

        self.print_step(1, "Commissioning, already done")

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe('{"Name": "Reset"}')

        self.print_step(2, "Read SupportedModes attribute")
        supported_modes = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

        logging.info("SupportedModes: %s" % (supported_modes))

        asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least two entries!")

        modes = [m.mode for m in supported_modes]

        self.print_step(3, "Read CurrentMode attribute")

        old_current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (old_current_mode))

        # pick a value that's not on the list of supported modes
        invalid_mode = max(modes) + 1

        from enum import Enum

        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02
            INVALID_IN_MODE = 0x03

        rvcCleanCodes = [code.value for code in Clusters.RvcCleanMode.Enums.StatusCode]

        self.print_step(4, "Send ChangeToMode command with NewMode set to %d" % (old_current_mode))

        ret = await self.send_clean_change_to_mode_cmd(newMode=old_current_mode)
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the mode to the current mode should be a no-op")

        if self.check_pics("RVCCLEANM.S.M.CAN_TEST_MODE_FAILURE"):
            asserts.assert_true(self.mode_fail in modes,
                                "The MODE_CHANGE_FAIL PIXIT value (%d) is not a supported mode" % (self.mode_fail))
            self.print_step(5, "Manually put the device in a state from which it will FAIL to transition to mode %d" % (self.mode_fail))
            if self.is_ci:
                print("Changing mode to Cleaning")
                await self.send_run_change_to_mode_cmd(1)
            else:
                input("Press Enter when done.\n")

            self.print_step(6, "Read CurrentMode attribute")
            old_current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

            logging.info("CurrentMode: %s" % (old_current_mode))

            self.print_step(7, "Send ChangeToMode command with NewMode set to %d" % (self.mode_fail))

            ret = await self.send_clean_change_to_mode_cmd(newMode=self.mode_fail)
            st = ret.status
            is_mfg_code = st in range(0x80, 0xC0)
            is_err_code = (st == CommonCodes.GENERIC_FAILURE.value) or (
                st == CommonCodes.INVALID_IN_MODE.value) or (st in rvcCleanCodes) or is_mfg_code
            asserts.assert_true(
                is_err_code, "Changing to mode %d must fail due to the current state of the device" % (self.mode_fail))
            st_text_len = len(ret.statusText)
            asserts.assert_true(st_text_len in range(1, 65), "StatusText length (%d) must be between 1 and 64" % (st_text_len))

            self.print_step(8, "Read CurrentMode attribute")
            current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

            logging.info("CurrentMode: %s" % (current_mode))

            asserts.assert_true(current_mode == old_current_mode, "CurrentMode changed after failed ChangeToMode command!")

        self.print_step(9, "Manually put the device in a state from which it will SUCCESSFULLY transition to mode %d" % (self.mode_ok))
        if self.is_ci:
            print("Changing mode to Idle")
            await self.send_run_change_to_mode_cmd(0)
        else:
            input("Press Enter when done.\n")

        self.print_step(10, "Read CurrentMode attribute")
        old_current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (old_current_mode))

        self.print_step(11, "Send ChangeToMode command with NewMode set to %d" % (self.mode_ok))

        ret = await self.send_clean_change_to_mode_cmd(newMode=self.mode_ok)
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value,
                            "Changing to mode %d must succeed due to the current state of the device" % (self.mode_ok))

        self.print_step(12, "Read CurrentMode attribute")
        current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (current_mode))

        asserts.assert_true(current_mode == self.mode_ok,
                            "CurrentMode doesn't match the argument of the successful ChangeToMode command!")

        self.print_step(13, "Send ChangeToMode command with NewMode set to %d" % (invalid_mode))

        ret = await self.send_clean_change_to_mode_cmd(newMode=invalid_mode)
        asserts.assert_true(ret.status == CommonCodes.UNSUPPORTED_MODE.value,
                            "Attempt to change to invalid mode %d didn't fail as expected" % (invalid_mode))

        self.print_step(14, "Read CurrentMode attribute")
        current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (current_mode))

        asserts.assert_true(current_mode == self.mode_ok, "CurrentMode changed after failed ChangeToMode command!")


if __name__ == "__main__":
    default_matter_test_main()
