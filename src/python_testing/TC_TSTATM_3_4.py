#
#    Copyright (c) 2026 Project CHIP Authors
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
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --int-arg PIXIT.TSTATM.MODE_CHANGE_OK:1
#       --int-arg PIXIT.TSTATM.MODE_CHANGE_FAIL:2
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from enum import Enum

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_asserts import is_valid_int_value
from matter.testing.matter_testing import MatterBaseTest, matchers
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_TSTATM_3_4(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.mode_fail = 2

    def desc_TC_TSTATM_3_4(self) -> str:
        return "[TC-TSTATM-3.4] Change to Mode By Core Tag functionality with DUT as Server"

    def steps_TC_TSTATM_3_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute",
                     "Verify that the DUT response contains a list of ModeOptionsStruct entries: "
                     "Verify that the list has two or more entries "
                     "Verify that PIXIT.TSTATM.MODE_CHANGE_FAIL is one of supported_modes_dut"),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute",
                     "CurrentMode attribute value is an integer value. "
                     "Select a value which is NOT in supported_modes_dut and save it as invalid_mode_th"),
            TestStep(4, "TH reads from the DUT the CoreModeTags attribute",
                     "Save the list as core_mode_tags_dut"),
            TestStep(5, "TH sends a ChangeToModeByCoreTag command to the DUT with NewModeTag set to a value not in core_mode_tags_dut",
                     "DUT responds with a ChangeToModeResponse command with Status set to UnsupportedMode(0x01)"),
            TestStep(6, "TH sends a ChangeToModeByCoreTag command to the DUT with NewModeTag set to a value from core_mode_tags_dut",
                     "DUT responds with a ChangeToModeResponse command with Status set to Success(0x00)"),
            TestStep(7, "Manually put the device in a state from which it will FAIL to transition to PIXIT.TSTATM.MODE_CHANGE_FAIL"),
            TestStep(8, "TH reads from the DUT the CurrentMode attribute",
                     "Save value as old_current_mode_dut. Find the tag value in supported_modes_dut that corresponds to "
                     "PIXIT.TSTATM.MODE_CHANGE_FAIL and matches a value in core_mode_tags_dut and save as fail_mode_tag_dut"),
            TestStep(9, "TH sends a ChangeToModeByCoreTag command to the DUT with NewModeTag set to fail_mode_tag_dut",
                     "DUT responds with a ChangeToModeResponse command with Status field set to GenericFailure(0x02), "
                     "InvalidInMode(0x03), or in the MfgCodes range and StatusText field has length between 1 and 64"),
            TestStep(10, "TH reads from the DUT the CurrentMode attribute",
                     "CurrentMode attribute value is an integer value equal to old_current_mode_dut"),
        ]

    def pics_TC_TSTATM_3_4(self) -> list[str]:
        return [
            "TSTATM.S.F01"
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_TSTATM_3_4(self):
        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02
            INVALID_IN_MODE = 0x03

        cluster = Clusters.Objects.ThermostatMode
        supported_modes_attribute = cluster.Attributes.SupportedModes
        current_mode_attribute = cluster.Attributes.CurrentMode
        core_mode_tags_attribute = cluster.Attributes.CoreModeTags
        endpoint = self.get_endpoint()

        can_test_mode_failure = self.check_pics("TSTATM.S.M.CAN_TEST_MODE_FAILURE")
        can_manually_control = self.check_pics("TSTATM.S.M.CAN_MANUALLY_CONTROLLED")
        test_failure = can_test_mode_failure and can_manually_control and ('PIXIT.TSTATM.MODE_CHANGE_FAIL' in self.matter_test_config.global_test_params)

        if test_failure:
            self.mode_fail = self.matter_test_config.global_test_params['PIXIT.TSTATM.MODE_CHANGE_FAIL']
            logger.info("Mode Fail: %s", self.mode_fail)
        else:
            logger.info("Skipping mode change failure steps (CAN_TEST_MODE_FAILURE not enabled or PIXIT.TSTATM.MODE_CHANGE_FAIL omitted)")

        # Step 1: Commissioning, already done
        self.step(1)

        # Step 2: TH reads SupportedModes attribute
        self.step(2)
        supported_modes_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=supported_modes_attribute
        )
        modes = [m.mode for m in supported_modes_dut]
        logger.info("SupportedModes: %s", supported_modes_dut)
        logger.info("Modes: %s", modes)

        asserts.assert_greater_equal(len(supported_modes_dut), 2, "SupportedModes must have at least 2 entries!")

        if test_failure:
            asserts.assert_true(self.mode_fail in modes,
                                f"{self.mode_fail} is not in supported_modes_dut: {modes}")

        # Step 3: TH reads CurrentMode attribute
        self.step(3)
        old_current_mode_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute
        )
        is_valid_int_value(old_current_mode_dut)
        asserts.assert_in(old_current_mode_dut, modes, "Current mode is not in the supported modes")

        invalid_mode_th = max(modes) + 1
        asserts.assert_true(invalid_mode_th not in modes,
                            f"{invalid_mode_th} is in {modes} and it shouldn't")

        # Step 4: TH reads CoreModeTags attribute
        self.step(4)
        core_mode_tags_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=core_mode_tags_attribute
        )
        logger.info("CoreModeTags: %s", core_mode_tags_dut)
        asserts.assert_greater_equal(len(core_mode_tags_dut), 1, "CoreModeTags must have at least 1 entry")

        # Step 5: TH sends ChangeToModeByCoreTag command with NewModeTag not in core_mode_tags_dut
        self.step(5)
        invalid_core_tag = 0xFFFF
        if invalid_core_tag in core_mode_tags_dut:
            invalid_core_tag = 0xFFFE
        cmd = cluster.Commands.ChangeToModeByCoreTag(newModeTag=invalid_core_tag)
        change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        asserts.assert_true(matchers.is_type(change_to_mode_response, cluster.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToModeByCoreTag")
        asserts.assert_equal(change_to_mode_response.status, CommonCodes.UNSUPPORTED_MODE.value,
                             f"Status is {change_to_mode_response.status} and it should be UnsupportedMode 0x01")

        # Step 6: TH sends ChangeToModeByCoreTag command with NewModeTag from core_mode_tags_dut
        self.step(6)
        valid_core_tag = core_mode_tags_dut[0]
        cmd = cluster.Commands.ChangeToModeByCoreTag(newModeTag=valid_core_tag)
        change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        asserts.assert_true(matchers.is_type(change_to_mode_response, cluster.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToModeByCoreTag")
        asserts.assert_equal(change_to_mode_response.status, CommonCodes.SUCCESS.value,
                             f"Status is {change_to_mode_response.status} and it should be Success 0x00")

        # Step 7: Manually put the device in a state from which it will FAIL to transition to PIXIT.TSTATM.MODE_CHANGE_FAIL
        self.step(7)
        if self.pics_guard(test_failure):
            self.wait_for_user_input(
                prompt_msg=f"Manually put the device in a state from which it will FAIL to transition to mode {self.mode_fail}, and press Enter when ready"
            )

        # Step 8: TH reads CurrentMode attribute and finds fail_mode_tag_dut
        self.step(8)
        fail_mode_tag_dut = None
        if self.pics_guard(test_failure):
            old_current_mode_dut = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute
            )
            is_valid_int_value(old_current_mode_dut)

            # Find the tag value in supported_modes_dut that corresponds to PIXIT.TSTATM.MODE_CHANGE_FAIL
            # and matches a value in core_mode_tags_dut list
            for mode_option in supported_modes_dut:
                if mode_option.mode == self.mode_fail:
                    for tag in mode_option.modeTags:
                        if tag.value in core_mode_tags_dut:
                            fail_mode_tag_dut = tag.value
                            break
                    if fail_mode_tag_dut is not None:
                        break
            asserts.assert_is_not_none(
                fail_mode_tag_dut,
                f"Could not find a core mode tag matching fail mode {self.mode_fail} in {supported_modes_dut}"
            )

        # Step 9: TH sends ChangeToModeByCoreTag command with NewModeTag set to fail_mode_tag_dut
        self.step(9)
        if self.pics_guard(test_failure):
            cmd = cluster.Commands.ChangeToModeByCoreTag(newModeTag=fail_mode_tag_dut)
            change_to_mode_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

            asserts.assert_true(matchers.is_type(change_to_mode_response, cluster.Commands.ChangeToModeResponse),
                                "Unexpected return type for ChangeToModeByCoreTag")
            logger.info("response: %s", change_to_mode_response)

            st = change_to_mode_response.status
            is_mfg_code = st in range(0x80, 0xC0)
            is_err_code = (st == CommonCodes.GENERIC_FAILURE.value) or (st == CommonCodes.INVALID_IN_MODE.value) or is_mfg_code
            asserts.assert_true(is_err_code, f"Changing to core tag {fail_mode_tag_dut} must fail due to current device state")

            st_text_len = len(change_to_mode_response.statusText) if change_to_mode_response.statusText else 0
            asserts.assert_true(st_text_len in range(1, 65), f"StatusText length {st_text_len} must be between 1 and 64")

        # Step 10: TH reads CurrentMode attribute
        self.step(10)
        if self.pics_guard(test_failure):
            old_current_mode_dut_2 = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=current_mode_attribute
            )
            is_valid_int_value(old_current_mode_dut_2)
            asserts.assert_equal(old_current_mode_dut_2, old_current_mode_dut,
                                 f"{old_current_mode_dut_2} is not equal to old_current_mode_dut: {old_current_mode_dut}")


if __name__ == "__main__":
    default_matter_test_main()
