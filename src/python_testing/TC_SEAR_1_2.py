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

# TODO - this was copied/pasted from another test, it needs to be reviewed and updated
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


class TC_SEAR_1_2(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = None
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"
        self.mapid_list = []

        # this must be kept in sync with the definitions from the Common Landmark Semantic Tag Namespace
        self.MAX_LANDMARK_ID = 0x33

        # this must be kept in sync with the definitions from the Common Relative Position Semantic Tag Namespace
        self.MAX_RELPOS_ID = 0x07

    async def read_sear_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ServiceArea
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_and_validate_supported_maps(self, step):
        self.print_step(step, "Read SupportedMaps attribute")
        supported_maps = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.SupportedMaps)
        logging.info("SupportedMaps: %s" % supported_maps)
        asserts.assert_less_equal(len(supported_maps), 255,
                                  "SupportedMaps should have max 255 entries")

        mapid_list = [m.mapID for m in supported_maps]
        asserts.assert_true(len(set(mapid_list)) == len(mapid_list), "SupportedMaps must have unique MapID values!")

        name_list = [m.name for m in supported_maps]
        asserts.assert_true(len(set(name_list)) == len(name_list), "SupportedMaps must have unique Name values!")

        # save so other methods can use this if needed
        self.mapid_list = mapid_list

    async def read_and_validate_supported_areas(self, step):
        self.print_step(step, "Read SupportedAreas attribute")
        supported_areas = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.SupportedAreas)
        logging.info("SupportedAreas: %s" % supported_areas)
        asserts.assert_less_equal(len(supported_areas), 255,
                                  "SupportedAreas should have max 255 entries")
        areaid_list = []
        areainfo_s = set()
        for a in supported_areas:
            asserts.assert_true(a.areaID not in areaid_list, "SupportedAreas must have unique AreaID values!")

            areaid_list.append(a.areaID)

            if len(self.mapid_list) > 0:
                asserts.assert_is_not(a.mapID, NullValue,
                                      f"SupportedAreas entry with AreaID({a.areaID}) should not have null MapID")
                asserts.assert_true(a.mapID in self.mapid_list,
                                    f"SupportedAreas entry with AreaID({a.areaID}) has unknown MapID({a.mapID})")
                k = f"mapID:{a.mapID} areaInfo:{a.areaInfo}"
                asserts.assert_true(k not in areainfo_s,
                                    f"SupportedAreas must have unique MapID({a.mapID}) + AreaInfo({a.areaInfo}) values!")
                areainfo_s.add(k)
            else:
                # empty SupportedMaps
                asserts.assert_is(a.mapID, NullValue,
                                  f"SupportedAreas entry with AreaID({a.areaID}) should have null MapID")
                k = f"areaInfo:{a.areaInfo}"
                asserts.assert_true(k not in areainfo_s, f"SupportedAreas must have unique AreaInfo({a.areaInfo}) values!")
                areainfo_s.add(k)

            if a.areaInfo.locationInfo is NullValue and a.areaInfo.landmarkInfo is NullValue:
                asserts.assert_true(
                    f"SupportedAreas entry with AreaID({a.areaID}) should not have null LocationInfo and null LandmarkInfo")
            if a.areaInfo.landmarkInfo is not NullValue:
                asserts.assert_true(a.areaInfo.landmarkInfo.landmarkTag <= self.MAX_LANDMARK_ID,
                                    f"SupportedAreas entry with AreaID({a.areaID}) has invalid LandmarkTag({a.areaInfo.landmarkInfo.landmarkTag})")
                asserts.assert_true(a.areaInfo.landmarkInfo.relativePositionTag is NullValue or a
                                    .areaInfo.landmarkInfo.relativePositionTag in range(0, self.MAX_RELPOS_ID),
                                    f"SupportedAreas entry with AreaID({a.areaID}) has invalid RelativePositionTag({a.areaInfo.landmarkInfo.relativePositionTag})")
        # save so other methods can use this if needed
        self.areaid_list = areaid_list

    async def read_and_validate_selected_areas(self, step):
        self.print_step(step, "Read SelectedAreas attribute")
        selected_areas = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.SelectedAreas)
        logging.info(f"SelectedAreas {selected_areas}")

        # TODO how to check if all entries are uint32?

        asserts.assert_true(len(selected_areas) <= len(self.areaid_list),
                            f"SelectedAreas(len {len(selected_areas)}) should have at most {len(self.areaid_list)} entries")

        asserts.assert_true(len(set(selected_areas)) == len(selected_areas), "SelectedAreas must have unique AreaID values!")

        for a in selected_areas:
            asserts.assert_true(a in self.areaid_list,
                                f"SelectedAreas entry {a} has invalid value")
        # save so other methods can use this if needed
        self.selareaid_list = selected_areas

    async def read_and_validate_current_area(self, step):
        self.print_step(step, "Read CurrentArea attribute")
        current_area = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.CurrentArea)
        logging.info(f"CurrentArea {current_area}")

        if current_area is not NullValue:
            asserts.assert_true(current_area in self.areaid_list,
                                f"CurrentArea {current_area} is not in SupportedAreas: {self.areaid_list}.")

        # save so other methods can use this if needed
        self.current_area = current_area

    async def read_and_validate_estimated_end_time(self, step):
        import time
        read_time = int(time.time())
        self.print_step(step, "Read EstimatedEndTime attribute")
        estimated_end_time = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.EstimatedEndTime)
        logging.info(f"EstimatedEndTime {estimated_end_time}")

        if self.current_area is NullValue:
            asserts.assert_true(estimated_end_time is NullValue,
                                "EstimatedEndTime should be null if CurrentArea is null.")

        if estimated_end_time is not NullValue:
            # allow for some clock skew
            asserts.assert_true(estimated_end_time >= read_time - 3*60,
                                f"EstimatedEndTime({estimated_end_time}) should be greater than the time when it was read({read_time})")

    async def read_and_validate_progress(self, step):
        self.print_step(step, "Read Progress attribute")
        progress = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.Progress)
        logging.info(f"Progress {progress}")

        asserts.assert_true(len(progress) <= len(self.areaid_list),
                            f"Progress(len {len(progress)}) should have at most {len(self.areaid_list)} entries")

        progareaid_list = []
        for p in progress:
            if p.areaID in progareaid_list:
                asserts.fail("Progress must have unique AreaID values!")
            else:
                progareaid_list.append(p.areaID)
                asserts.assert_true(p.areaID in self.areaid_list,
                                    f"Progress entry has invalid AreaID value ({p.areaID})")
                asserts.assert_true(p.status in (Clusters.ServiceArea.Enums.OperationalStatusEnum.kPending,
                                                 Clusters.ServiceArea.Enums.OperationalStatusEnum.kOperating,
                                                 Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped,
                                                 Clusters.ServiceArea.Enums.OperationalStatusEnum.kCompleted),
                                    f"Progress entry has invalid Status value ({p.status})")
                if p.status not in (Clusters.ServiceArea.Enums.OperationalStatusEnum.kSkipped, Clusters.ServiceArea.Enums.OperationalStatusEnum.kCompleted):
                    asserts.assert_true(p.totalOperationalTime is NullValue,
                                        f"Progress entry should have a null TotalOperationalTime value (Status is {p.status})")
                # TODO how to check that InitialTimeEstimate is either null or uint32?

    def TC_SEAR_1_2(self) -> list[str]:
        return ["SEAR.S"]

    @async_test_body
    async def test_TC_SEAR_1_2(self):
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

        if self.check_pics("SEAR.S.F02"):
            await self.read_and_validate_supported_maps(step=2)

        await self.read_and_validate_supported_areas(step=3)

        await self.read_and_validate_selected_areas(step=4)

        if self.check_pics("SEAR.S.A0003"):
            await self.read_and_validate_current_area(step=5)

        if self.check_pics("SEAR.S.A0004"):
            await self.read_and_validate_estimated_end_time(step=6)

        if self.check_pics("SEAR.S.A0005"):
            await self.read_and_validate_progress(step=7)

        if self.check_pics("SEAR.S.F02") and self.check_pics("SEAR.S.M.REMOVE_MAP"):
            test_step = "Manually ensure the SupportedMaps attribute is not empty and that the device is not operating"
            self.print_step("8", test_step)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_maps(step=9)
            old_supported_maps = self.mapid_list

            test_step = "Manually intervene to remove one or more entries in the SupportedMaps list"
            self.print_step("10", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "RemoveMap", "MapId": 3})
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_maps(step=11)
            new_supported_maps = self.mapid_list
            asserts.assert_true(len(old_supported_maps) > len(new_supported_maps), "Failed to remove map(s)")

            # NOTE the following operations are all part of step 11 - read all these attributes and check the data consistency
            #     after removing map(s)
            await self.read_and_validate_supported_areas(step=11)

            await self.read_and_validate_selected_areas(step=11)

            if self.check_pics("SEAR.S.A0003"):
                await self.read_and_validate_current_area(step=11)

            if self.check_pics("SEAR.S.A0004"):
                await self.read_and_validate_estimated_end_time(step=11)

            if self.check_pics("SEAR.S.A0005"):
                await self.read_and_validate_progress(step=11)

        if self.check_pics("SEAR.S.F02") and self.check_pics("SEAR.S.M.ADD_MAP"):
            test_step = "Manually ensure the SupportedMaps attribute has less than 255 entries and that the device is not operating"
            self.print_step("12", test_step)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_maps(step=13)
            old_supported_maps = self.mapid_list

            test_step = "Manually intervene to add one or more entries to the SupportedMaps list"
            self.print_step("14", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "AddMap", "MapId": 1, "MapName": "NewTestMap1"})
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_maps(step=15)
            new_supported_maps = self.mapid_list
            asserts.assert_true(len(old_supported_maps) < len(new_supported_maps), "Failed to add map(s)")

            # NOTE the following operations are all part of step 15 - read all these attributes and check the data consistency
            #     after adding map(s)
            await self.read_and_validate_supported_areas(step=15)

            await self.read_and_validate_selected_areas(step=15)

            if self.check_pics("SEAR.S.A0003"):
                await self.read_and_validate_current_area(step=15)

            if self.check_pics("SEAR.S.A0004"):
                await self.read_and_validate_estimated_end_time(step=15)

            if self.check_pics("SEAR.S.A0005"):
                await self.read_and_validate_progress(step=15)

        if self.check_pics("SEAR.S.M.REMOVE_AREA"):
            test_step = "Manually ensure the SupportedAreas attribute is not empty and that the device is not operating"
            self.print_step("16", test_step)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_areas(step=17)
            old_supported_areas = self.areaid_list

            test_step = "Manually intervene to remove one or more entries from the SupportedAreas list"
            self.print_step("18", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "RemoveArea", "AreaId": 10050})
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_areas(step=19)
            new_supported_areas = self.areaid_list
            asserts.assert_true(len(old_supported_areas) > len(new_supported_areas), "Failed to remove area(s)")

            # NOTE the following operations are all part of step 19 - read all these attributes and check the data consistency
            #     after removing areas(s)

            await self.read_and_validate_selected_areas(step=19)

            if self.check_pics("SEAR.S.A0003"):
                await self.read_and_validate_current_area(step=19)

            if self.check_pics("SEAR.S.A0004"):
                await self.read_and_validate_estimated_end_time(step=19)

            if self.check_pics("SEAR.S.A0005"):
                await self.read_and_validate_progress(step=19)

        if self.check_pics("SEAR.S.M.ADD_AREA"):
            test_step = "Manually ensure the SupportedAreas attribute has less than 255 entries and that the device is not operating"
            self.print_step("20", test_step)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_areas(step=21)
            old_supported_areas = self.areaid_list

            test_step = "Manually intervene to add one or more entries to the SupportedAreas list"
            self.print_step("22", test_step)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "AddArea", "AreaId": 42, "MapId": 1, "LocationName": "NewTestArea1"})
            else:
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")

            await self.read_and_validate_supported_areas(step=23)
            new_supported_areas = self.areaid_list
            asserts.assert_true(len(old_supported_areas) < len(new_supported_areas), "Failed to add area(s)")

            # NOTE the following operations are all part of step 23 - read all these attributes and check the data consistency
            #     after removing areas(s)

            await self.read_and_validate_selected_areas(step=23)

            if self.check_pics("SEAR.S.A0003"):
                await self.read_and_validate_current_area(step=23)

            if self.check_pics("SEAR.S.A0004"):
                await self.read_and_validate_estimated_end_time(step=23)

            if self.check_pics("SEAR.S.A0005"):
                await self.read_and_validate_progress(step=23)


if __name__ == "__main__":
    default_matter_test_main()
