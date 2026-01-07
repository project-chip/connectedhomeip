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
#     app: ${CHIP_RVC_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/rvcopstate_2_5_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/rvcopstate_2_5_fifo
#       --PICS examples/rvc-app/rvc-common/pics/rvc-app-pics-values
#       --int-arg runmode_cleanmode:1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import enum
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest, TestStep, matchers
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class RvcStatusEnum(enum.IntEnum):
    # TODO remove this class once InvalidInMode response code is implemented in python SDK
    Success = 0x0
    UnsupportedMode = 0x1
    GenericFailure = 0x2
    InvalidInMode = 0x3


def error_enum_to_text(error_enum):
    if error_enum == RvcStatusEnum.Success:
        return "Success(0x00)"
    if error_enum == RvcStatusEnum.UnsupportedMode:
        return "UnsupportedMode(0x01)"
    if error_enum == RvcStatusEnum.GenericFailure:
        return "GenericFailure(0x02)"
    if error_enum == RvcStatusEnum.InvalidInMode:
        return "InvalidInMode(0x03)"
    raise AttributeError("Unknown Enum value")


def verify_mode_tag_in_supported_modes(supported_modes, mode_value, expected_tag):
    for entry in supported_modes:
        if entry.mode == mode_value:
            tag_values = [tag.value for tag in entry.modeTags]
            log.info(f"Matched SupportedMode entry: {entry}, ModeTags: {tag_values}")
            asserts.assert_in(expected_tag.value, tag_values,
                              f"Expected ModeTag '{expected_tag.name}' (0x{expected_tag.value:04x}) not found in ModeTags: {tag_values}")
            return
    asserts.fail(f"No SupportedModes entry matched CurrentMode value {mode_value}")


class TC_RVCOPSTATE_2_5(MatterBaseTest):

    min_report_interval_sec = 0
    max_report_interval_sec = 30

    def desc_TC_RVCOPSTATE_2_5(self) -> str:
        return "[TC-RVCOPSTATE-2.5] Attributes with DUT as Server"

    def steps_TC_RVCOPSTATE_2_5(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep(
                "2", "Manually put the device in a RVC Run Mode cluster mode with the Idle mode tag and in a device state that allows changing to {PIXIT_RUNMODE_CLEANMODE}"),
            TestStep("3", "TH reads the SupportedModes attribute of the RVC Run Mode cluster"),
            TestStep("4", "TH establishes a subscription to the CurrentMode attribute of the RVC Run Mode cluster of the DUT"),
            TestStep("5", "TH sends a RVC Run Mode cluster ChangeToMode command to the DUT with NewMode set to PIXIT.CLEANMODE"),
            TestStep("6", "Wait for DUT to leave dock and begin cleaning activities"),
            TestStep("7", "TH reads CurrentMode attribute of the RVC Run Mode cluster"),
            TestStep("8", "TH sends GoHome command to the DUT"),
            TestStep("9", "Manually confirm DUT has returned to the dock and completed docking-related activities"),
            TestStep("10", "TH reads CurrentMode attribute of the RVC Run Mode cluster"),
        ]

    def pics_TC_RVCOPSTATE_2_5(self) -> list[str]:
        return [
            "RVCOPSTATE.S",
        ]

    async def read_rvcrunm_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcRunMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_rvcopstate_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_supported_mode(self, endpoint):
        return await self.read_rvcrunm_attribute_expect_success(endpoint=endpoint,
                                                                attribute=Clusters.RvcRunMode.Attributes.SupportedModes)

    async def read_current_mode_with_check(self, expected_mode, endpoint):
        run_mode = await self.read_rvcrunm_attribute_expect_success(endpoint=endpoint,
                                                                    attribute=Clusters.RvcRunMode.Attributes.CurrentMode)
        asserts.assert_true(run_mode == expected_mode,
                            f"Expected the current mode to be {expected_mode}, got {run_mode}")
        return run_mode

    async def read_operational_state(self, endpoint):
        return await self.read_rvcopstate_attribute_expect_success(endpoint=endpoint,
                                                                   attribute=Clusters.RvcOperationalState.Attributes.OperationalState)

    async def read_operational_state_with_check(self, expected_operational_state):
        operational_state = await self.read_operational_state(endpoint=self.endpoint)
        asserts.assert_true(operational_state == expected_operational_state,
                            # TODO:  nice conversion of states to strings
                            f"Expected OperationalState of {expected_operational_state}, got {operational_state}"
                            )
        return operational_state

    async def send_change_to_mode_cmd(self, new_mode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        return await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=new_mode),
                                          endpoint=self.endpoint)

    async def send_change_to_mode_with_check(self, new_mode, expected_error):
        response = await self.send_change_to_mode_cmd(new_mode)
        asserts.assert_true(response.status == expected_error,
                            f"Expected a ChangeToMode response status of {error_enum_to_text(expected_error)}, got {error_enum_to_text(response.status)}")

    async def send_go_home_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.GoHome(),
                                         endpoint=self.endpoint)
        asserts.assert_true(matchers.is_type(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for GoHome")
        return ret

    async def send_go_home_cmd_with_check(self, expected_error):
        ret = await self.send_go_home_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, expected_error,
                             f"errorStateID({ret.commandResponseState.errorStateID}) should be {error_enum_to_text(expected_error)}")

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_RVCOPSTATE_2_5(self):

        self.endpoint = self.get_endpoint()
        # Allow some user input steps to be skipped if running under CI
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        # Commission DUT to TH
        self.step("1")
        self.print_step(1, "Commissioning, already done")

        required_pics = [
            "RVCOPSTATE.S.A0004",
            "RVCOPSTATE.S.C80.Rsp",
            "RVCRUNM.S.A0000",
            "RVCRUNM.S.A0001",
            "RVCRUNM.S.M.CAN_MANUALLY_CONTROLLED",
            "RVCOPSTATE.S.M.CAN_MANUALLY_CONTROLLED",
        ]
        if self.pics_guard(all(self.check_pics(p) for p in required_pics)):

            # Manually put the device in a RVC Run Mode cluster mode with the Idle mode tag and in a device state that allows changing to {PIXIT_RUNMODE_CLEANMODE}
            self.step("2")
            if not self.is_ci:
                step_name_idle_mode = "Manually put the device in a RVC Run Mode cluster mode with the Idle mode tag and in a device state that allows changing to {PIXIT_RUNMODE_CLEANMODE}"
                self.wait_for_user_input(prompt_msg=f"{step_name_idle_mode}, and press Enter when ready.")
            else:
                self.write_to_app_pipe({"Name": "Reset"})
            # TH reads the SupportedModes attribute of the RVC Run Mode cluster
            self.step("3")
            supported_run_modes_dut = await self.read_supported_mode(endpoint=self.endpoint)
            # Logging the SupportedModes Attribute output responses from the DUT:
            log.info("SupportedModes: %s" % (supported_run_modes_dut))
            # As per SPEC, the SupportedModes must have at least two entries
            asserts.assert_greater_equal(len(supported_run_modes_dut), 2, "SupportedModes must have at least two entries!")

            # Dynamically extract Idle and Cleaning mode from SupportedModes
            idle_mode = next((m.mode for m in supported_run_modes_dut
                              if Clusters.RvcRunMode.Enums.ModeTag.kIdle.value in [t.value for t in m.modeTags]), None)
            cleaning_mode = self.user_params.get("runmode_cleanmode")
            asserts.assert_is_not_none(idle_mode, "Idle mode not found in SupportedModes!")
            asserts.assert_is_not_none(cleaning_mode, "Cleaning mode not provided via --int-arg runmode_cleanmode")

            # TH establishes a subscription to the CurrentMode attribute of the RVC Run Mode cluster of the DUT
            self.step("4")
            current_mode_accumulator = AttributeSubscriptionHandler(
                Clusters.RvcRunMode,
                Clusters.RvcRunMode.Attributes.CurrentMode)
            await current_mode_accumulator.start(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id, endpoint=self.endpoint,
                min_interval_sec=self.min_report_interval_sec,
                max_interval_sec=self.max_report_interval_sec)

            # TH sends a RVC Run Mode cluster ChangeToMode command to the DUT with NewMode set to PIXIT.CLEANMODE
            self.step("5")
            await self.send_change_to_mode_with_check(cleaning_mode, RvcStatusEnum.Success)
            # This step is not described in the test plan, but it ought to be
            await self.read_current_mode_with_check(cleaning_mode, endpoint=self.endpoint)

            # Wait for DUT to leave dock and begin cleaning activities
            self.step("6")
            current_mode_match = AttributeMatcher.from_callable(
                "CurrentMode is CLEANING",
                lambda report: report.value == cleaning_mode)
            # Wait for attribute reports to match
            current_mode_accumulator.await_all_expected_report_matches([current_mode_match], timeout_sec=10)

            await asyncio.sleep(1)

            # TH reads CurrentMode attribute of the RVC Run Mode cluster
            self.step("7")
            cleaning_run_mode_dut = await self.read_current_mode_with_check(cleaning_mode, endpoint=self.endpoint)
            # Logging the CurrentMode Attribute output responses from the DUT:
            log.info(f"CurrentMode: {cleaning_run_mode_dut}")
            verify_mode_tag_in_supported_modes(supported_run_modes_dut, cleaning_run_mode_dut,
                                               Clusters.RvcRunMode.Enums.ModeTag.kCleaning)

            # TH sends GoHome command to the DUT
            self.step("8")

            await self.send_go_home_cmd_with_check(Clusters.OperationalState.Enums.ErrorStateEnum.kNoError)
            await self.read_operational_state_with_check(Clusters.RvcOperationalState.Enums.OperationalStateEnum.kSeekingCharger)

            # Manually confirm DUT has returned to the dock and completed docking-related activities
            self.step("9")
            if not self.is_ci:
                confirm_docking_complete = "Manually confirm DUT has returned to the dock and completed docking-related activities"
                self.wait_for_user_input(prompt_msg=f"{confirm_docking_complete}, and press Enter when ready.")
            else:
                self.write_to_app_pipe({"Name": "Docked"})

            current_mode_match = AttributeMatcher.from_callable(
                "CurrentMode is IDLE",
                lambda report: report.value == idle_mode)
            # Wait for attribute reports to match
            current_mode_accumulator.await_all_expected_report_matches([current_mode_match], timeout_sec=10)

            # TH reads CurrentMode attribute of the RVC Run Mode cluster
            self.step("10")
            post_docking_run_mode_dut = await self.read_current_mode_with_check(expected_mode=idle_mode, endpoint=self.endpoint)
            # Logging the CurrentMode Attribute output responses from the DUT:
            log.info(f"CurrentMode: {post_docking_run_mode_dut}")
            verify_mode_tag_in_supported_modes(supported_run_modes_dut, post_docking_run_mode_dut,
                                               Clusters.RvcRunMode.Enums.ModeTag.kIdle)

        else:
            self.mark_all_remaining_steps_skipped("2")
            log.info(
                "Device does not support mandatory Attributes/Commands which is required to run this test case, skipped all the test steps")


if __name__ == "__main__":
    default_matter_test_main()
