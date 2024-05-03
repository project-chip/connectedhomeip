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

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DISHM_2_1(MatterBaseTest):

    async def read_mode_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.DishwasherMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_change_to_mode_cmd(self, newMode) -> Clusters.Objects.DishwasherMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.DishwasherMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.DishwasherMode.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToMode")
        return ret

    def desc_TC_DISHM_2_1(self) -> str:
        return "[TC-DISHM-2.1] Change to Mode functionality with DUT as Server"

    def steps_TC_DISHM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute."),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(4, "TH sends a ChangeToMode command to the DUT with NewMode set to old_current_mode_dut"),
            TestStep(5, "Manually put the device in a state from which it will FAIL to transition to PIXIT.DISHM.MODE_CHANGE_FAIL"),
            TestStep(6, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(7, "TH sends a ChangeToMode command to the DUT with NewMode set to PIXIT.DISHM.MODE_CHANGE_FAIL"),
            TestStep(8, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(9, "Manually put the device in a state from which it will SUCCESSFULLY transition to PIXIT.DISHM.MODE_CHANGE_OK"),
            TestStep(10, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(11, "TH sends a ChangeToMode command to the DUT with NewMode set to PIXIT.DISHM.MODE_CHANGE_OK"),
            TestStep(12, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(13, "TH sends a ChangeToMode command to the DUT with NewMode set to invalid_mode_th"),
            TestStep(14, "TH reads from the DUT the CurrentMode attribute."),
        ]
        return steps

    def pics_TC_DISHM_2_1(self) -> list[str]:
        pics = [
            "DISHM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_DISHM_2_1(self):

        asserts.assert_true('PIXIT.DISHM.MODE_CHANGE_OK' in self.matter_test_config.global_test_params,
                            "PIXIT.DISHM.MODE_CHANGE_OK must be included on the command line in "
                            "the --int-arg flag as PIXIT.DISHM.MODE_CHANGE_OK:<mode id>")
        asserts.assert_true('PIXIT.DISHM.MODE_CHANGE_FAIL' in self.matter_test_config.global_test_params,
                            "PIXIT.DISHM.MODE_CHANGE_FAIL must be included on the command line in "
                            "the --int-arg flag as PIXIT.DISHM.MODE_CHANGE_FAIL:<mode id>")

        self.endpoint = self.user_params.get("endpoint", 1)
        logging.info("This test expects to find this cluster on endpoint 1")

        self.modeOk = self.matter_test_config.global_test_params['PIXIT.DISHM.MODE_CHANGE_OK']
        self.modeFail = self.matter_test_config.global_test_params['PIXIT.DISHM.MODE_CHANGE_FAIL']

        asserts.assert_true(self.check_pics("DISHM.S.A0000"), "DISHM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.A0001"), "DISHM.S.A0001 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.C00.Rsp"), "DISHM.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.C01.Tx"), "DISHM.S.C01.Tx must be supported")

        attributes = Clusters.DishwasherMode.Attributes

        self.step(1)

        self.step(2)
        # read the supported modes
        supported_modes = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)
        logging.info("SupportedModes: %s" % (supported_modes))
        # need at least 2 modes
        asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least two entries!")
        # save them
        supported_modes_dut = [m.mode for m in supported_modes]
        # check that mode change fail is present
        asserts.assert_true(self.modeFail in supported_modes_dut, "SupportedModes must include MODE_CHANGE_FAIL")

        self.step(3)
        # Read the current mode
        old_current_mode_dut = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)
        logging.info("CurrentMode: %s" % (old_current_mode_dut))
        # pick a value that's not in the list of supported modes
        invalid_mode_th = max(supported_modes_dut) + 1

        from enum import Enum

        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02
            INVALID_IN_MODE = 0x03

        self.step(4)
        logging.info("Send ChangeToMode command with NewMode set to %d" % (old_current_mode_dut))
        ret = await self.send_change_to_mode_cmd(newMode=old_current_mode_dut)
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the mode to the current mode should be a no-op")

        if self.check_pics("DISHM.S.M.CAN_TEST_MODE_FAILURE"):
            self.step(5)
            logging.info("Manually put the device in a state from which it will FAIL to transition to mode %d" % (self.modeFail))
            input("Press Enter when done.\n")

            self.step(6)
            old_current_mode_dut = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)
            logging.info("CurrentMode: %s" % (old_current_mode_dut))

            self.step(7)
            logging.info("Send ChangeToMode command with NewMode set to %d" % (self.modeFail))

            ret = await self.send_change_to_mode_cmd(newMode=self.modeFail)
            st = ret.status
            logging.info("ChangeToMode Status: %s" % (ret.status))
            is_mfg_code = st in range(0x80, 0xBF)
            is_err_code = (st == CommonCodes.GENERIC_FAILURE.value) or (st == CommonCodes.INVALID_IN_MODE.value) or is_mfg_code
            asserts.assert_true(
                is_err_code, "Changing to mode %d must fail due to the current state of the device" % (self.modeFail))
            logging.info("Status Text: %s" % (ret.statusText))
            st_text_len = len(ret.statusText)
            asserts.assert_true(st_text_len in range(1, 65), "StatusText length (%d) must be between 1 and 64" % (st_text_len))

            self.step(8)
            current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)
            logging.info("CurrentMode: %s" % (current_mode))
            asserts.assert_true(current_mode == old_current_mode_dut, "CurrentMode changed after failed ChangeToMode command!")
        else:
            for x in range(5, 9):
                self.skip(x)

        self.step(9)
        logging.info("Manually put the device in a state from which it will SUCCESSFULLY transition to mode %d" % (self.modeOk))
        input("Press Enter when done.\n")

        self.step(10)
        old_current_mode_dut = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)
        logging.info("CurrentMode: %s" % (old_current_mode_dut))

        self.step(11)
        logging.info("Send ChangeToMode command with NewMode set to %d" % (self.modeOk))
        ret = await self.send_change_to_mode_cmd(newMode=self.modeOk)
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value,
                            "Changing to mode %d must succeed due to the current state of the device" % (self.modeOk))

        self.step(12)
        current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)
        logging.info("CurrentMode: %s" % (current_mode))
        asserts.assert_true(current_mode == self.modeOk,
                            "CurrentMode doesn't match the argument of the successful ChangeToMode command!")

        self.step(13)
        logging.info("Send ChangeToMode command with NewMode set to %d" % (invalid_mode_th))
        ret = await self.send_change_to_mode_cmd(newMode=invalid_mode_th)
        asserts.assert_true(ret.status == CommonCodes.UNSUPPORTED_MODE.value,
                            "Attempt to change to invalid mode %d didn't fail as expected" % (invalid_mode_th))

        self.step(14)
        current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)
        logging.info("CurrentMode: %s" % (current_mode))
        asserts.assert_true(current_mode == self.modeOk, "CurrentMode changed after failed ChangeToMode command!")


if __name__ == "__main__":
    default_matter_test_main()
