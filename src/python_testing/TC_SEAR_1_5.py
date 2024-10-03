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

# TODO - this was copied/pasted from abother test, it needs to be reviewed and updated
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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_SEAR_1_5(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = None
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_sear_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ServiceArea
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_supported_areas(self, step):
        self.print_step(step, "Read SupportedAreas attribute")
        supported_areas = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.SupportedAreas)
        logging.info("SupportedAreas: %s" % supported_areas)

        return [a.areaID for a in supported_areas]

    async def read_selected_areas(self, step):
        self.print_step(step, "Read SelectedAreas attribute")
        selected_areas = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.SelectedAreas)
        logging.info(f"SelectedAreas {selected_areas}")

        return selected_areas

    async def read_progress(self, step):
        self.print_step(step, "Read Progress attribute")
        progress = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.Progress)
        logging.info(f"Progress {progress}")

        return progress

    async def read_current_area(self, step):
        self.print_step(step, "Read CurrentArea attribute")
        current_area = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.CurrentArea)
        logging.info(f"CurrentArea {current_area}")

        return current_area

    async def send_cmd_skip_area_expect_response(self, step, skipped_area, expected_response):
        self.print_step(step, f"Send SkipArea command with SkippedArea({skipped_area})")
        ret = await self.send_single_cmd(cmd=Clusters.Objects.ServiceArea.Commands.SkipArea(skippedArea=skipped_area),
                                         endpoint=self.endpoint)

        asserts.assert_equal(ret.status,
                             expected_response,
                             f"Command response ({ret.status}) doesn't match the expected one")

    def TC_SEAR_1_5(self) -> list[str]:
        return ["SEAR.S", "SEAR.S.C02.Rsp"]

    @async_test_body
    async def test_TC_SEAR_1_5(self):
        self.endpoint = self.matter_test_config.endpoint
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set")
            self.app_pipe = self.app_pipe + str(app_pid)

        self.print_step(1, "Commissioning, already done")

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe({"Name": "Reset"})

        supported_area_ids = await self.read_supported_areas(step=2)
        asserts.assert_true(len(supported_area_ids) > 0, "SupportedAreas is empty")
        valid_area_id = supported_area_ids[0]
        invalid_area_id = 1 + max(supported_area_ids)

        if self.check_pics("SEAR.S.M.INVALID_STATE_FOR_SKIP") and self.check_pics("SEAR.S.M.HAS_MANUAL_SKIP_STATE_CONTROL"):
            test_step = "Manually intervene to put the device in a state that prevents it from executing the SkipArea command \
                  (e.g. set CurrentArea to null or make it not operate, i.e. be in the idle state). Ensure that SelectedArea is not empty."
            self.print_step("3", test_step)
            if self.is_ci:
                await self.send_single_cmd(cmd=Clusters.Objects.ServiceArea.Commands.SelectAreas(newAreas=[7]),
                                           endpoint=self.endpoint)
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.send_cmd_skip_area_expect_response(step=4, skipped_area=valid_area_id,
                                                          expected_response=Clusters.ServiceArea.Enums.SkipAreaStatus.kInvalidInMode)

        if self.check_pics("SEAR.S.M.NO_SELAREA_FOR_SKIP") and self.check_pics("SEAR.S.M.HAS_MANUAL_SKIP_STATE_CONTROL"):
            test_step = "Manually intervene to put the device in a state where the state would allow it to execute the SkipArea command, \
                if SelectedAreas wasn't empty, and SelectedAreas is empty"
            self.print_step("5", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Reset"})
                await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=1),
                                           endpoint=self.endpoint)
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.send_cmd_skip_area_expect_response(step=6, skipped_area=valid_area_id,
                                                          expected_response=Clusters.ServiceArea.Enums.SkipAreaStatus.kInvalidAreaList)

        if self.check_pics("SEAR.S.M.VALID_STATE_FOR_SKIP") and self.check_pics("SEAR.S.M.HAS_MANUAL_SKIP_STATE_CONTROL"):
            test_step = "Manually intervene to put the device in a state that allows it to execute the SkipArea command"
            self.print_step("7", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Reset"})
                await self.send_single_cmd(cmd=Clusters.Objects.ServiceArea.Commands.SelectAreas(newAreas=[7, 1234567]),
                                           endpoint=self.endpoint)
                await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=1),
                                           endpoint=self.endpoint)
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.send_cmd_skip_area_expect_response(step=8, skipped_area=invalid_area_id,
                                                          expected_response=Clusters.ServiceArea.Enums.SkipAreaStatus.kInvalidSkippedArea)

        if not self.check_pics("SEAR.S.M.VALID_STATE_FOR_SKIP"):
            return

        if self.check_pics("SEAR.S.A0005"):
            old_progress_list = await self.read_progress(step=9)
            asserts.assert_true(len(old_progress_list) > 0, f"len of Progress({len(old_progress_list)}) should not be zero)")

        selected_areas = await self.read_selected_areas(step=10)
        asserts.assert_true(len(selected_areas) > 0, "SelectedAreas is empty")

        old_current_area = NullValue
        if self.check_pics("SEAR.S.A0003"):
            old_current_area = await self.read_current_area(step=11)

            self.print_step("12", "")
            if old_current_area is not NullValue:
                await self.send_cmd_skip_area_expect_response(step=13, skipped_area=old_current_area,
                                                              expected_response=Clusters.ServiceArea.Enums.SkipAreaStatus.kSuccess)
                if self.check_pics("SEAR.S.M.HAS_MANUAL_SKIP_STATE_CONTROL"):
                    test_step = "(Manual operation) wait for the device to skip the current area, and start operating at\
                            the next one it should process, or stop operating"
                    self.print_step("14", test_step)
                    if not self.is_ci:
                        self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

                if self.check_pics("SEAR.S.A0005"):
                    new_progress_list = await self.read_progress(step=15)
                    asserts.assert_true(len(new_progress_list) > 0,
                                        f"len of Progress({len(new_progress_list)}) should not be zero)")

                    prog_areas = [p.areaID for p in new_progress_list]

                    asserts.assert_true(old_current_area in prog_areas, f"Progress should include area {old_current_area}")

                    new_current_area = await self.read_current_area(step=16)
                    for p in new_progress_list:
                        if p.areaID == old_current_area:
                            asserts.assert_true(p.status == Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                                "Progress for areaID({old_current_area}) should be Skipped")
                            break
                    test_step = "Indicate whether the device has stopped operating (y/n)"
                    ret = self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

                    # Verify that if the device hasn't stopped operating, the `new_progress_list`'s entry matching `new_current_area` shows the Operating status
                    if ret != "y":
                        for p in new_progress_list:
                            if p.areaID == new_current_area:
                                asserts.assert_true(p.status == Clusters.ServiceArea.Enums.OperationalStatusEnum.kOperating,
                                                    "Progress for areaID({new_current_area}) should be Operating")
                                break

                    # next, we need to check for something else (so the condition of the 'if' above becomes part of the 'then' statement below):
                    # if before skipping all areas, except the current one, were Skipped or Completed, the device MUST have stopped operating
                    was_only_skipped_or_completed = True
                    for p in old_progress_list:
                        if p.areaID != old_current_area:
                            if p.status not in (Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                                Clusters.ServiceArea.Enums.OperationalStatusEnum.kCompleted):
                                was_only_skipped_or_completed = False
                                break
                    if was_only_skipped_or_completed:
                        asserts.assert_true(ret == "y", "The device should not be operating")

                    self.print_step("17", "")
                    return

        if not self.check_pics("SEAR.S.A0005"):
            return

        if self.check_pics("SEAR.S.M.HAS_MANUAL_SKIP_STATE_CONTROL"):
            test_step = "Manually intervene to put the device in a state that allows it to execute the SkipArea command"
            self.print_step("18", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Reset"})
                await self.send_single_cmd(cmd=Clusters.Objects.ServiceArea.Commands.SelectAreas(newAreas=[7, 1234567]),
                                           endpoint=self.endpoint)
                await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=1),
                                           endpoint=self.endpoint)
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

        self.print_step("19", "")
        if len(old_progress_list) == 0:
            return

        area_to_skip = NullValue
        self.print_step("20", "")
        for p in old_progress_list:
            if p.status in (Clusters.ServiceArea.Enums.OperationalStatusEnum.kPending,
                            Clusters.ServiceArea.Enums.OperationalStatusEnum.kOperating):
                area_to_skip = p.areaID
                break

        if area_to_skip is NullValue:
            return

        await self.send_cmd_skip_area_expect_response(step=21, skipped_area=area_to_skip,
                                                      expected_response=Clusters.ServiceArea.Enums.SkipAreaStatus.kSuccess)

        if self.check_pics("SEAR.S.M.HAS_MANUAL_SKIP_STATE_CONTROL"):
            test_step = "(Manual operation) wait for the device to update Progress or to stop operating"
            self.print_step("22", test_step)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

        new_progress_list = await self.read_progress(step=23)
        asserts.assert_true(len(new_progress_list) > 0, f"len of Progress({len(new_progress_list)}) should not be zero)")

        for p in new_progress_list:
            if p.areaID == area_to_skip:
                asserts.assert_true(p.status == Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                    "Progress for areaID({new_current_area}) should be Skipped")
                break

        test_step = "Indicate whether the device has stopped operating (y/n)"
        ret = self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

        was_only_skipped_or_completed = True
        for p in old_progress_list:
            if p.areaID != area_to_skip:
                if p.status not in (Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                    Clusters.ServiceArea.Enums.OperationalStatusEnum.kCompleted):
                    was_only_skipped_or_completed = False
                    break
        if was_only_skipped_or_completed:
            asserts.assert_true(ret == "y", "The device should not be operating")
            for p in new_progress_list:
                if p.areaID == old_current_area:
                    asserts.assert_true(p.status == Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                        "Progress for areaID({old_current_area}) should be Skipped")
                break


if __name__ == "__main__":
    default_matter_test_main()
