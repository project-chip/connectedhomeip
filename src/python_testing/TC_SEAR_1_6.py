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


class TC_SEAR_1_6(MatterBaseTest):
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

    def TC_SEAR_1_6(self) -> list[str]:
        return ["SEAR.S", "SEAR.S.A0005", "SEAR.S.A0000", "SEAR.S.A0002", "SEAR.S.M.HAS_MANUAL_OPERATING_STATE_CONTROL"]

    @async_test_body
    async def test_TC_SEAR_1_6(self):
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

        test_step = "Manually intervene to put the device in the idle state and ensure SupportedAreas and SelectedAreas are not empty"
        self.print_step("2", test_step)
        if self.is_ci:
            await self.send_single_cmd(cmd=Clusters.Objects.ServiceArea.Commands.SelectAreas(newAreas=[7, 1234567]),
                                       endpoint=self.endpoint)
        else:
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

        supported_area_ids = await self.read_supported_areas(step=3)
        asserts.assert_true(len(supported_area_ids) > 0, "SupportedAreas is empty")

        selected_areas = await self.read_selected_areas(step=4)
        asserts.assert_true(len(selected_areas) > 0, "SelectedAreas is empty")

        test_step = "Manually intervene to put the device in the operating state"
        self.print_step("5", test_step)
        if self.is_ci:
            await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=1),
                                       endpoint=self.endpoint)
        else:
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

        progress_list_operating = await self.read_progress(step=6)
        asserts.assert_true(len(selected_areas) == len(progress_list_operating),
                            f"len of SelectedAreas({len(selected_areas)}) should be equal to len of Progress({len(progress_list_operating)})")

        for p in progress_list_operating:
            asserts.assert_true(p.areaID in selected_areas, f"Progress entry with unknown AreaID({p.areaID})")
            asserts.assert_true(p.status in (Clusters.ServiceArea.Enums.OperationalStatusEnum.kPending,
                                             Clusters.ServiceArea.Enums.OperationalStatusEnum.kOperating),
                                f"Progress entry with unexpected Status({p.status})")
            asserts.assert_true(type(p.totalOperationalTime) in [type(None), type(
                NullValue)], "Progress.TotalOperationalTime has a value")

        test_step = "While all entries in Progress show the Pending or Operating status (i.e. \
                    before any area is skipped or completed), manually intervene to put the device \
                    in the idle state, by ending the operation unexpectedly (e.g. force an error)"
        self.print_step("7", test_step)
        if self.is_ci:
            await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=0),
                                       endpoint=self.endpoint)
        else:
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

        progress_list_idle = await self.read_progress(step=8)
        asserts.assert_true(len(selected_areas) == len(progress_list_idle),
                            f"len of SelectedAreas({len(selected_areas)}) should be equal to len of Progress({len(progress_list_idle)})")

        for p in progress_list_idle:
            asserts.assert_true(p.areaID in selected_areas, f"Progress entry with unknown AreaID({p.areaID})")
            asserts.assert_true(p.status == Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                f"Progress entry with unexpected Status({p.status})")


if __name__ == "__main__":
    default_matter_test_main()
