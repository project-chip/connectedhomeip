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
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS examples/rvc-app/rvc-common/pics/rvc-app-pics-values --endpoint 1 --json-arg PIXIT.SEAR.VALID_AREAS:'[7, 1234567]' --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_SEAR_1_3(MatterBaseTest):
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

        self.supported_areas = supported_areas

        return [a.areaID for a in supported_areas]

    async def read_selected_areas(self, step):
        self.print_step(step, "Read SelectedAreas attribute")
        selected_areas = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.SelectedAreas)
        logging.info(f"SelectedAreas {selected_areas}")

        return selected_areas

    async def send_cmd_select_areas_expect_response(self, step, new_areas, expected_response):
        self.print_step(step, f"Send SelectAreas command with NewAreas({new_areas})")
        ret = await self.send_single_cmd(cmd=Clusters.Objects.ServiceArea.Commands.SelectAreas(newAreas=new_areas),
                                         endpoint=self.endpoint)

        asserts.assert_equal(ret.status,
                             expected_response,
                             f"Command response ({ret.status}) doesn't match the expected one")

    def TC_SEAR_1_3(self) -> list[str]:
        return ["SEAR.S"]

    @async_test_body
    async def test_TC_SEAR_1_3(self):
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
        asserts.assert_true(len(self.supported_areas) > 0, "SupportedAreas is empty")
        valid_area_id = supported_area_ids[0]
        invalid_area_id = 1 + max(supported_area_ids)

        duplicated_areas = [valid_area_id, valid_area_id]

        await self.send_cmd_select_areas_expect_response(step=3, new_areas=duplicated_areas, expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kSuccess)

        selected_areas = await self.read_selected_areas(step=4)
        asserts.assert_true(selected_areas == [valid_area_id], "SelectedAreas should be empty")

        await self.send_cmd_select_areas_expect_response(step=5, new_areas=[], expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kSuccess)

        selected_areas = await self.read_selected_areas(step=6)
        asserts.assert_true(len(selected_areas) == 0, "SelectedAreas should be empty")

        await self.send_cmd_select_areas_expect_response(step=7, new_areas=[invalid_area_id], expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kUnsupportedArea)

        if "PIXIT.SEAR.VALID_AREAS" not in self.matter_test_config.global_test_params:
            # All the remaining tests require the PIXIT.SEAR.VALID_AREAS
            return

        valid_areas = self.matter_test_config.global_test_params['PIXIT.SEAR.VALID_AREAS']

        if self.check_pics("SEAR.S.M.INVALID_STATE_FOR_SELECT_AREAS") and self.check_pics("SEAR.S.M.HAS_MANUAL_SELAREA_STATE_CONTROL"):
            test_step = "Manually intervene to put the device in a state that prevents it from executing the SelectAreas command"
            self.print_step("8", test_step)
            if self.is_ci:
                await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=1), endpoint=self.endpoint)
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.send_cmd_select_areas_expect_response(step=9, new_areas=valid_areas, expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kInvalidInMode)

        if self.check_pics("SEAR.S.M.VALID_STATE_FOR_SELECT_AREAS") and self.check_pics("SEAR.S.M.HAS_MANUAL_SELAREA_STATE_CONTROL"):
            test_step = f"Manually intervene to put the device in a state that allows it to execute the SelectAreas({valid_areas}) command"
            self.print_step("10", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Reset"})
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.send_cmd_select_areas_expect_response(step=11, new_areas=valid_areas, expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kSuccess)

            selected_areas = await self.read_selected_areas(step=12)
            asserts.assert_true(selected_areas == valid_areas,
                                f"SelectedAreas({selected_areas}) should match SupportedAreas({valid_areas})")

            await self.send_cmd_select_areas_expect_response(step=13, new_areas=valid_areas, expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kSuccess)

        if self.check_pics("SEAR.S.M.VALID_STATE_FOR_SELECT_AREAS") and self.check_pics("SEAR.S.M.HAS_MANUAL_SELAREA_STATE_CONTROL") and self.check_pics("SEAR.S.M.SELECT_AREAS_WHILE_NON_IDLE"):
            test_step = f"Manually intervene to put the device in a state that allows it to execute the SelectAreas({valid_areas}) command, and put the device in a non-idle state"
            self.print_step("14", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Reset"})
                await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=1), endpoint=self.endpoint)
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            if self.check_pics("SEAR.S.F00"):
                await self.send_cmd_select_areas_expect_response(step=15, new_areas=valid_areas, expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kSuccess)
            else:
                await self.send_cmd_select_areas_expect_response(step=15, new_areas=valid_areas, expected_response=Clusters.ServiceArea.Enums.SelectAreasStatus.kInvalidInMode)


if __name__ == "__main__":
    default_matter_test_main()
