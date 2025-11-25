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
#

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --app-pipe /tmp/dishm_2_1_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --int-arg PIXIT.DISHM.MODE_CHANGE_OK:0
#       --int-arg PIXIT.DISHM.MODE_CHANGE_FAIL:2
#       --app-pipe /tmp/dishm_2_1_fifo
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import logging
from enum import Enum

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_asserts import is_valid_int_value
from matter.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_cluster, matchers,
                                           run_if_endpoint_matches)


class TC_DISHM_2_1(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.mode_ok = 0
        self.mode_fail = 0
        self.is_ci = False

    def desc_TC_DISHM_2_1(self) -> str:
        return "[TC-DISHM-2.1] Change to Mode functionality with DUT as Server"

    def steps_TC_DISHM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute", "Verify that the DUT response contains a list of ModeOptionsStruct entries:"
                     "Verify that the list has two or more entries"
                     "Verify that PIXIT.DISHM.MODE_CHANGE_FAIL is one of supported_modes_dut"),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute", "CurrentMode attribute value is an integer value"
                     "Select a value which is NOT in supported_modes_dut and save it as invalid_mode_th"),
            TestStep(4, "TH sends a ChangeToMode command to the DUT with NewMode set to old_current_mode_dut",
                     "DUT responds contains a ChangeToModeResponse command with a SUCCESS (value 0x00) status response"),
            TestStep(5, "Manually put the device in a state from which it will FAIL to transition to PIXIT.DISHM.MODE_CHANGE_FAIL"),
            TestStep(6, "TH reads from the DUT the CurrentMode attribute", "CurrentMode attribute value is an integer value"),
            TestStep(7, "TH sends a ChangeToMode command to the DUT with NewMode set to PIXIT.DISHM.MODE_CHANGE_FAIL",
                     "DUT responds contains a ChangeToModeResponse command with Status field is set to GenericFailure(0x02), InvalidInMode(0x03), or in the MfgCodes (0x80 to 0xBF) range and StatusText field has a length between 1 and 64"),
            TestStep(8, "TH reads from the DUT the CurrentMode attribute",
                     "CurrentMode attribute value is an integer value and equal to old_current_mode_dut"),
            TestStep(9, "Manually put the device in a state from which it will SUCCESSFULLY transition to PIXIT.DISHM.MODE_CHANGE_OK"),
            TestStep(10, "TH reads from the DUT the CurrentMode attribute", "CurrentMode attribute value is an integer value"),
            TestStep(11, "TH sends a ChangeToMode command to the DUT with NewMode set to PIXIT.DISHM.MODE_CHANGE_OK",
                     "DUT responds contains a ChangeToModeResponse command with a SUCCESS (value 0x00) status response"),
            TestStep(12, "TH reads from the DUT the CurrentMode attribute",
                     "CurrentMode attribute value is an integer value and it is the same as step 11"),
            TestStep(13, "TH sends a ChangeToMode command to the DUT with NewMode set to invalid_mode_th",
                     "DUT responds contains a ChangeToModeResponse command with a UnsupportedMode(0x01) status response"),
            TestStep(14, "TH reads from the DUT the CurrentMode attribute",
                     "CurrentMode attribute value is an integer value and same as step 12")
        ]
        return steps

    def pics_TC_DISHM_2_1(self) -> list[str]:
        pics = [
            "DISHM.S"
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.DishwasherMode))
    async def test_TC_DISHM_2_1(self):

        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02
            INVALID_IN_MODE = 0x03

        cluster = Clusters.Objects.DishwasherMode
        supported_modes_attribute = cluster.Attributes.SupportedModes
        current_mode_attribute = cluster.Attributes.CurrentMode
        endpoint = self.get_endpoint()

        asserts.assert_true('PIXIT.DISHM.MODE_CHANGE_OK' in self.matter_test_config.global_test_params,
                            "PIXIT.DISHM.MODE_CHANGE_OK must be included on the command line in "
                            "the --int-arg flag as PIXIT.DISHM.MODE_CHANGE_OK:<mode id>")
        asserts.assert_true('PIXIT.DISHM.MODE_CHANGE_FAIL' in self.matter_test_config.global_test_params,
                            "PIXIT.DISHM.MODE_CHANGE_FAIL must be included on the command line in "
                            "the --int-arg flag as PIXIT.DISHM.MODE_CHANGE_FAIL:<mode id>")

        self.mode_fail = self.matter_test_config.global_test_params['PIXIT.DISHM.MODE_CHANGE_FAIL']
        self.mode_ok = self.matter_test_config.global_test_params['PIXIT.DISHM.MODE_CHANGE_OK']
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        # Commissioning, already done
        self.step(1)

        # TH reads from the DUT the SupportedModes attribute
        self.step(2)
        supported_modes_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=supported_modes_attribute)
        modes = [m.mode for m in supported_modes_dut]

        # Check if the list of supported modes is larger than 2
        asserts.assert_greater_equal(len(supported_modes_dut), 2, "SupportedModes must have at least 2 entries!")

        # Verify that PIXIT.DISHM.MODE_CHANGE_FAIL is one of supported_modes_dut
        asserts.assert_true(self.mode_fail in modes,
                            f"{self.mode_fail} is not in supported_modes_dut: {modes}")

        # TH reads from the DUT the CurrentMode attribute
        self.step(3)
        old_current_mode_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute)

        # CurrentMode attribute value is an integer value
        is_valid_int_value(old_current_mode_dut)
        asserts.assert_in(old_current_mode_dut, modes, "Current mode is not in the supported modes")

        invalid_mode_th = max(modes) + 1
        asserts.assert_true(invalid_mode_th not in modes,
                            f"{invalid_mode_th} is in {modes} and it shouldn't")

        # TH sends a ChangeToMode command to the DUT with NewMode set to old_current_mode_dut
        self.step(4)
        cmd = cluster.Commands.ChangeToMode(newMode=old_current_mode_dut)
        change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        # DUT responds contains a ChangeToModeResponse command with a SUCCESS (value 0x00) status response
        asserts.assert_true(matchers.is_type(change_to_mode_response, cluster.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToMode")
        asserts.assert_equal(change_to_mode_response.status, CommonCodes.SUCCESS.value,
                             f"Status is {change_to_mode_response.status} and it should be SUCCESS 0x00")

        # Manually put the device in a state from which it will FAIL to transition to PIXIT.DISHM.MODE_CHANGE_FAIL
        self.step(5)

        # This step prepares the DUT for a negative transition scenario.
        # The goal is to place the DUT in a state where a subsequent ChangeToMode(newMode=MODE_CHANGE_FAIL)
        # would fail (e.g. device is busy/incompatible state). On real devices this can be done
        # manually, in CI the example app does not expose a way to synthesize the internal condition.
        # Therefore, in CI we do not switch the DUT to mode_fail.

        can_test_mode_failure = self.check_pics("DISHM.S.M.CAN_TEST_MODE_FAILURE")
        can_manually_control = self.check_pics("DISHM.S.M.CAN_MANUALLY_CONTROLLED")
        if can_test_mode_failure and can_manually_control:
            asserts.assert_true(self.mode_fail in modes,
                                f"The MODE_CHANGE_FAIL PIXIT value {self.mode_fail} is not a supported mode")
            if self.is_ci:
                self.mark_current_step_skipped()
            else:
                self.wait_for_user_input(
                    prompt_msg=f"Manually put the device in a state from which it will FAIL to transition to mode {self.mode_fail}, and press Enter when ready.")

        # TH reads from the DUT the CurrentMode attribute
        self.step(6)
        old_current_mode_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute)

        # CurrentMode attribute value is an integer value
        is_valid_int_value(old_current_mode_dut)

        # TH sends a ChangeToMode command to the DUT with NewMode set to PIXIT.DISHM.MODE_CHANGE_FAIL
        # NOTE: Skip this step as SDK is not enabled with this failure response

        supports_negative_response = False  # set True if DUT implements it

        if not (can_test_mode_failure and supports_negative_response):
            self.skip_step(7)
        else:
            self.step(7)
            cmd = cluster.Commands.ChangeToMode(newMode=self.mode_fail)
            change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

            # DUT responds contains a ChangeToModeResponse command with Status field is set to GenericFailure(0x02), InvalidInMode(0x03), or in the MfgCodes (0x80 to 0xBF) range and StatusText field has a length between 1 and 64
            asserts.assert_true(matchers.is_type(change_to_mode_response, cluster.Commands.ChangeToModeResponse),
                                "Unexpected return type for ChangeToMode")

            logging.info(f"response: {change_to_mode_response}")

            st = change_to_mode_response.status

            is_mfg_code = st in range(0x80, 0xC0)
            is_err_code = (st == CommonCodes.GENERIC_FAILURE.value) or (st == CommonCodes.INVALID_IN_MODE.value) or is_mfg_code

            asserts.assert_true(is_err_code, f"Changing to mode {self.mode_fail} must fail due to the current state of the device")

            st_text_len = len(change_to_mode_response.statusText)
            asserts.assert_true(st_text_len in range(1, 65), f"StatusText length {st_text_len} must be between 1 and 64")

        # TH reads from the DUT the CurrentMode attribute
        self.step(8)
        old_current_mode_dut_2 = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute)

        # CurrentMode attribute value is an integer value
        is_valid_int_value(old_current_mode_dut_2)

        # old_current_mode_dut_2 is equal to old_current_mode_dut
        asserts.assert_equal(old_current_mode_dut_2, old_current_mode_dut,
                             f"{old_current_mode_dut_2} is not equal to old_current_mode_dut: {old_current_mode_dut}")

        # Manually put the device in a state from which it will SUCCESSFULLY transition to PIXIT.DISHM.MODE_CHANGE_OK
        self.step(9)
        if self.is_ci:
            logging.info("Continuing...")
        else:
            self.wait_for_user_input(
                prompt_msg=f"Manually put the device in a state from which it will SUCCESSFULLY transition to mode {self.mode_ok}, and press Enter when ready.")

        # TH reads from the DUT the CurrentMode attribute
        self.step(10)
        old_current_mode_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute)

        # CurrentMode attribute value is an integer value
        is_valid_int_value(old_current_mode_dut)

        # TH sends a ChangeToMode command to the DUT with NewMode set to PIXIT.DISHM.MODE_CHANGE_OK
        self.step(11)
        cmd = cluster.Commands.ChangeToMode(newMode=self.mode_ok)
        change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        # DUT responds contains a ChangeToModeResponse command with a SUCCESS (value 0x00) status response
        asserts.assert_true(matchers.is_type(change_to_mode_response, cluster.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToMode")
        asserts.assert_equal(change_to_mode_response.status, CommonCodes.SUCCESS.value,
                             f"Status is {change_to_mode_response.status} and it should be SUCCESS 0x00")

        # TH reads from the DUT the CurrentMode attribute
        self.step(12)
        current_mode = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute)

        # CurrentMode attribute value is an integer value and it is the same as step 11
        is_valid_int_value(current_mode)
        asserts.assert_equal(current_mode, self.mode_ok,
                             f"CurrentMode: {current_mode} doesn't match the argument of the successful ChangeToMode command: {self.mode_ok}")

        # TH sends a ChangeToMode command to the DUT with NewMode set to invalid_mode_th
        self.step(13)
        cmd = cluster.Commands.ChangeToMode(newMode=invalid_mode_th)
        change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        # DUT responds contains a ChangeToModeResponse command with a UnsupportedMode(0x01) status response
        asserts.assert_equal(change_to_mode_response.status, CommonCodes.UNSUPPORTED_MODE.value,
                             f"Attempt to change to invalid mode {invalid_mode_th} didn't fail as expected")

        # TH reads from the DUT the CurrentMode attribute
        self.step(14)
        current_mode = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute)

        # CurrentMode attribute value is an integer value and same as step 12
        asserts.assert_equal(current_mode, self.mode_ok,
                             f"CurrentMode: {current_mode} doesn't match the argument of the successful ChangeToMode command: {self.mode_ok}")


if __name__ == "__main__":
    default_matter_test_main()
