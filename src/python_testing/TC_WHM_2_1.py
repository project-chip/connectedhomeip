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
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from enum import Enum

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_WHM_2_1(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = 0

    def steps_TC_WHM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the SupportedModes attribute"),
            TestStep(3, "Read the CurrentMode attribute"),
            TestStep(4, "Send ChangeToMode command with NewMode"),
            TestStep(5, "Manually put the device in a state from which it will FAIL to transition"),
            TestStep(6, "Read CurrentMode attribute"),
            TestStep(7, "Send ChangeToMode command with NewMode"),
            TestStep(8, "Read CurrentMode attribute"),
            TestStep(9, "Manually put the device in a state from which it will SUCCESSFULLY transition"),
            TestStep(10, "Read CurrentMode attribute"),
            TestStep(11, "Send ChangeToMode command with NewMode"),
            TestStep(12, "Read CurrentMode attribute"),
            TestStep(13, "Send ChangeToMode command with NewMode set to an invalid mode"),
            TestStep(14, "Read CurrentMode attribute"),
        ]
        return steps

    async def read_mode_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.WaterHeaterMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_change_to_mode_cmd(self, newMode) -> Clusters.Objects.WaterHeaterMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.WaterHeaterMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.WaterHeaterMode.Commands.ChangeToModeResponse),
                            "Unexpected return type for Water Heater Mode ChangeToMode")
        return ret

    def pics_TC_WHM_2_1(self) -> list[str]:
        return ["WHM.S"]

    @async_test_body
    async def test_TC_WHM_2_1(self):

        asserts.assert_true('PIXIT.WHM.MODE_CHANGE_OK' in self.matter_test_config.global_test_params,
                            "PIXIT.WHM.MODE_CHANGE_OK must be included on the command line in "
                            "the --int-arg flag as PIXIT.WHM.MODE_CHANGE_OK:<mode id>")
        asserts.assert_true('PIXIT.WHM.MODE_CHANGE_FAIL' in self.matter_test_config.global_test_params,
                            "PIXIT.WHM.MODE_CHANGE_FAIL must be included on the command line in "
                            "the --int-arg flag as PIXIT.WHM.MODE_CHANGE_FAIL:<mode id>")

        self.endpoint = self.matter_test_config.endpoint
        self.mode_ok = self.matter_test_config.global_test_params['PIXIT.WHM.MODE_CHANGE_OK']
        self.mode_fail = self.matter_test_config.global_test_params['PIXIT.WHM.MODE_CHANGE_FAIL']
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set.c")
            self.app_pipe = self.app_pipe + str(app_pid)

        # Valid modes. Only ModeManual referred to in this test
        # ModeOff    = 0
        ModeManual = 1
        # ModeTimed  = 2

        attributes = Clusters.WaterHeaterMode.Attributes

        self.step(1)
        # Commission DUT - already done

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe({"Name": "Reset"})

        self.step(2)

        supported_modes = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

        logging.info(f"SupportedModes: {supported_modes}")

        asserts.assert_greater_equal(len(supported_modes), 2,
                                     "SupportedModes must have at least two entries!")

        self.step(3)

        old_current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info(f"CurrentMode: {old_current_mode}")

        # pick a value that's not on the list of supported modes
        modes = [m.mode for m in supported_modes]
        invalid_mode = max(modes) + 1

        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02
            INVALID_IN_MODE = 0x03

        self.step(4)

        ret = await self.send_change_to_mode_cmd(newMode=old_current_mode)
        logging.info(f"ret.status {ret.status}")
        asserts.assert_equal(ret.status, Status.Success,
                             "Changing the mode to the current mode should be a no-op")

        if 1 or self.check_pics("WHM.S.M.CAN_TEST_MODE_FAILURE"):
            asserts.assert_true(self.mode_fail in modes,
                                "The MODE_CHANGE_FAIL PIXIT value (%d) is not a supported mode" % (self.mode_fail))
            self.step(5)
            if self.is_ci:
                print("Change to WHM mode ManualMode")
                await self.send_change_to_mode_cmd(newMode=1)
            else:
                self.wait_for_user_input(
                    prompt_msg="Manually put the device in a state from which it will FAIL to transition to mode %d, and press Enter when ready." % (self.mode_fail))

            self.step(6)
            old_current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

            logging.info("CurrentMode: %s" % (old_current_mode))

            self.step(7)

            ret = await self.send_change_to_mode_cmd(newMode=self.mode_fail)
            st = ret.status
            is_mfg_code = st in range(0x80, 0xC0)
            is_err_code = (st == CommonCodes.GENERIC_FAILURE.value) or (st == CommonCodes.INVALID_IN_MODE.value) or is_mfg_code
            asserts.assert_true(is_err_code, "Changing to mode %d must fail due to the current state of the device" % (self.mode_fail))
            st_text_len = len(ret.statusText)
            asserts.assert_true(st_text_len in range(1, 65), "StatusText length (%d) must be between 1 and 64" % (st_text_len))

            self.step(8)
            current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

            logging.info("CurrentMode: %s" % (current_mode))

            asserts.assert_true(current_mode == old_current_mode, "CurrentMode changed after failed ChangeToMode command!")

        else:
            self.step(5)
            self.step(6)
            self.step(7)
            self.step(8)

        self.step(9)
        if self.is_ci:
            print("Continuing...")
        else:
            self.wait_for_user_input(
                prompt_msg="Manually put the device in a state from which it will SUCCESSFULLY transition to mode %d, and press Enter when ready." % (self.mode_ok))

        self.step(10)

        old_current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info(f"CurrentMode: {old_current_mode}")

        self.step(11)

        ret = await self.send_change_to_mode_cmd(newMode=ModeManual)
        asserts.assert_true(ret.status == Status.Success,
                            f"Changing to mode {ModeManual}must succeed due to the current state of the device")

        self.step(12)

        current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info(f"CurrentMode: {current_mode}")

        asserts.assert_true(current_mode == ModeManual,
                            "CurrentMode doesn't match the argument of the successful ChangeToMode command!")

        self.step(13)

        ret = await self.send_change_to_mode_cmd(newMode=invalid_mode)
        logging.info(f"ret {ret}")
        asserts.assert_true(ret.status == Status.Failure,
                            f"Attempt to change to invalid mode {invalid_mode} didn't fail as expected")

        self.step(14)

        current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info(f"CurrentMode: {current_mode}")

        asserts.assert_true(current_mode == ModeManual,
                            "CurrentMode changed after failed ChangeToMode command!")


if __name__ == "__main__":
    default_matter_test_main()
