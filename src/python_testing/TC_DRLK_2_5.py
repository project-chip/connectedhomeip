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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_LOCK_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

logger = logging.getLogger(__name__)

drlkcluster = Clusters.DoorLock


class TC_DRLK_2_5(MatterBaseTest):

    def steps_TC_DRLK_2_5(self) -> list[TestStep]:
        steps = [


            TestStep("1", "TH reads NumberOfWeekDaySchedulesSupportedPerUser attribute.",
                     "Verify that TH is able to read the attribute successfully."),
            TestStep("2a", "TH sends SetUser Command to DUT.", "Verify that the DUT sends SUCCESS response."),
            TestStep("2b", "TH sends SetWeekDaySchedule Command to DUT.", "Verify that the DUT sends SUCCESS response."),
            TestStep("2c", "TH sends SetWeekDaySchedule to DUT with different DaysMaskMap.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("3", "TH sends GetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT sends GetWeekDayScheduleResponse command with expected values."),
            TestStep("4", "TH sends SetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT responds with INVALID_COMMAND."),
            TestStep("5", "TH sends SetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT responds with INVALID_COMMAND."),
            TestStep("6", "TH sends SetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT responds with INVALID_COMMAND."),
            TestStep("7", "TH sends SetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT responds with INVALID_COMMAND."),
            TestStep("8", "TH sends GetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT responds with with INVALID_COMMAND."),
            TestStep("9", "TH sends GetWeekDaySchedule Command to DUT.",
                     "Verify that the DUT responds with GetWeekDayScheduleResponse command with Status NOT_FOUND."),
            TestStep("10a", "TH sends ClearWeekDaySchedule Command to DUT.", "Verify that the DUT sends SUCCESS response."),
            TestStep("10b", "TH sends ClearWeekDaySchedule Command to DUT.", "Verify that the DUT sends INVALID_COMMAND response."),
            TestStep("10c", "TH sends ClearWeekDaySchedule Command to DUT.", "Verify that the DUT sends INVALID_COMMAND response."),
            TestStep("11", "TH sends GetWeekDaySchedule Command to DUT.", "Verify that the DUT sends NOT_FOUND response."),
            TestStep("12", "TH sends ClearUser Command to DUT.", "Verify that the DUT sends SUCCESS response."),
        ]

        return steps

    async def read_attributes_from_dut(self, endpoint, cluster, attribute, expected_status: Status = Status.Success):
        try:
            attribute_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                             attribute=attribute)
            asserts.assert_equal(expected_status, Status.Success)
            return attribute_value
        except Exception as e:
            logging.error(e)
            logging.info("Error reading attributes,%s" % (attribute))

    def pics_TC_DRLK_2_5(self) -> list[str]:
        return ["DRLK.S", "DRLK.S.F04"]

    async def clear_user_cmd(self, user_index, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearUser(userIndex=user_index),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
            asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def clear_week_day_schedule_cmd(self, week_day_index, user_index, expected_status):
        try:
            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearWeekDaySchedule(weekDayIndex=week_day_index, userIndex=user_index),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def get_weekday_schedule_cmd(self,  week_day_index, user_index, days_Mask, start_Hour, start_Minute, end_Hour, end_Minute, expected_status):

        try:

            response = await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.GetWeekDaySchedule(
                weekDayIndex=week_day_index, userIndex=user_index),
                endpoint=self.app_cluster_endpoint,
                timedRequestTimeoutMs=1000)
            asserts.assert_true(type_matches(response, Clusters.DoorLock.Commands.GetWeekDayScheduleResponse),
                                "Unexpected return type for GetWeekDayScheduleResponse")

            if (expected_status == Status.Success):

                asserts.assert_true(response.weekDayIndex == week_day_index,
                                    "Error when executing GetWeekDayScheduleResponse command, weekDayIndex={}".format(
                                        str(response.weekDayIndex)))
                asserts.assert_true(response.userIndex == user_index,
                                    "Error when executing GetWeekDayScheduleResponse command, userIndex={}".format(
                                        str(response.userIndex)))

                asserts.assert_true(response.daysMask == days_Mask,
                                    "Error when executing GetWeekDayScheduleResponse command, days_Mask={}".format(
                                        str(response.daysMask)))
                asserts.assert_true(response.startHour == start_Hour,
                                    "Error when executing GetWeekDayScheduleResponse command, startHour={}".format(
                                        str(response.startHour)))
                asserts.assert_true(response.startMinute == start_Minute,
                                    "Error when executing GetWeekDayScheduleResponse command, startMinute={}".format(
                                        str(response.startMinute)))
                asserts.assert_true(response.endHour == end_Hour,
                                    "Error when executing GetWeekDayScheduleResponse command, endHour={}".format(
                                        str(response.endHour)))
                asserts.assert_true(response.endMinute == end_Minute,
                                    "Error when executing GetWeekDayScheduleResponse command, endMinute={}".format(
                                        str(response.endMinute)))

            return response
        except InteractionModelError as e:
            logging.error(e)
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def set_week_days_schedule_cmd(self, week_day_index, user_index, day_mask_map_index, start_Hour, start_Minute, end_Hour, end_Minute, expected_status):
        try:

            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.SetWeekDaySchedule(
                weekDayIndex=week_day_index,
                userIndex=user_index,
                daysMask=day_mask_map_index,
                startHour=start_Hour,
                startMinute=start_Minute,
                endHour=end_Hour,
                endMinute=end_Minute),
                endpoint=self.app_cluster_endpoint,
                timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            logging.error(e)
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def clear_credentials_cmd(self, credential, step=None, expected_status: Status = Status.Success):
        try:

            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearCredential(credential=credential),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            logging.exception(e)
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    @async_test_body
    async def test_TC_DRLK_2_5(self):

        self.common_cluster_endpoint = 0
        self.app_cluster_endpoint = 1
        user_name = "xxx"
        user_index = 1
        user_unique_id = 6452
        week_day_index = 1
        day_mask_map_index = 1
        start_Hour = 15
        start_Minute = 45
        end_Hour = 16
        end_Minute = 55

        self.step("1")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.A0014")):
            number_week_day_schedules_supported_per_user = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                                               cluster=drlkcluster,
                                                                                               attribute=Clusters.DoorLock.Attributes.NumberOfWeekDaySchedulesSupportedPerUser)
            logging.info("NumberOfWeekDaySchedulesSupportedPerUser %s" % (number_week_day_schedules_supported_per_user))
            asserts.assert_in(number_week_day_schedules_supported_per_user, range(
                0, 255), "NumberOfWeekDaySchedulesSupportedPerUser value is out of range")
        self.step("2a")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=drlkcluster.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=user_index,
                    userName=user_name,
                    userUniqueID=user_unique_id,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("2b")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0b.Rsp")):
            try:
                await self.set_week_days_schedule_cmd(
                    week_day_index,
                    user_index,
                    day_mask_map_index,
                    start_Hour,
                    start_Minute,
                    end_Hour,
                    end_Minute,
                    Status.Success)

            except InteractionModelError as e:
                logging.exception(e)
        self.step("2c")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0b.Rsp")):
            try:
                days_masks_map = [2, 4, 8, 16, 32, 64, 24, 42, 51, 107, 123, 127]
                for day_mask_map_index in days_masks_map:
                    await self.set_week_days_schedule_cmd(
                        week_day_index,
                        user_index,
                        day_mask_map_index,
                        start_Hour,
                        start_Minute,
                        end_Hour,
                        end_Minute,
                        Status.Success)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("3")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0c.Rsp") and self.check_pics("DRLK.S.C0c.Tx")):
            day_mask_map_index = 127
            await self.get_weekday_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.Success)
        self.step("4")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0b.Rsp")):
            week_day_index = 0
            day_mask_map_index = 2
            await self.set_week_days_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.InvalidCommand)
        self.step("5")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0b.Rsp")):
            week_day_index = 1  # Valid value
            day_mask_map_index = 0  # invalid value
            await self.set_week_days_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.InvalidCommand)
        self.step("6")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0b.Rsp")):
            week_day_index = 1  # valid value
            day_mask_map_index = 128  # invalid value
            await self.set_week_days_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.InvalidCommand)

        self.step("7")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0b.Rsp")):
            week_day_index = 1  # valid value
            day_mask_map_index = 1  # valid value
            start_Hour = 18  # inavlid value > end hour
            await self.set_week_days_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.InvalidCommand)

        self.step("8")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0c.Rsp") and self.check_pics("DRLK.S.C0c.Tx")):
            week_day_index = 0
            user_index = 1
            await self.get_weekday_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.InvalidCommand)
        self.step("9")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0c.Rsp") and self.check_pics("DRLK.S.C0c.Tx")):
            week_day_index = 1
            user_index = 2  # invalid value as it does not exist
            await self.get_weekday_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.NotFound)
        self.step("10a")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0d.Rsp")):
            week_day_index = 1
            user_index = 1
            await self.clear_week_day_schedule_cmd(week_day_index,
                                                   user_index,
                                                   Status.Success)

        self.step("10b")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0d.Rsp")):
            week_day_index = 0
            user_index = 1
            try:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearWeekDaySchedule(week_day_index, user_index),
                                           endpoint=self.app_cluster_endpoint,
                                           timedRequestTimeoutMs=1000)
                asserts.fail("Unexpected success in sending ClearWeekDaySchedule Command  with invalid week_day_index")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidCommand,
                                     "Unexpected error sending ClearWeekDaySchedule Command  with invalid week_day_index")
        self.step("10c")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0d.Rsp")):
            week_day_index = 1
            user_index = 0

            try:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearWeekDaySchedule(week_day_index, user_index),
                                           endpoint=self.app_cluster_endpoint,
                                           timedRequestTimeoutMs=1000)
                asserts.fail("Unexpected success in sending ClearWeekDaySchedule Command  with invalid week_day_index")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidCommand,
                                     "Unexpected error sending ClearWeekDaySchedule Command  with invalid week_day_index")

        self.step("11")
        if self.pics_guard(self.check_pics("DRLK.S.F04") and self.check_pics("DRLK.S.C0c.Rsp") and self.check_pics("DRLK.S.C0c.Tx")):
            user_index = 1  # invalid value as it is cleared
            await self.get_weekday_schedule_cmd(
                week_day_index,
                user_index,
                day_mask_map_index,
                start_Hour,
                start_Minute,
                end_Hour,
                end_Minute,
                Status.NotFound)
        self.step("12")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1d.Rsp")):
            await self.clear_user_cmd(user_index)


if __name__ == '__main__':
    default_matter_test_main()
