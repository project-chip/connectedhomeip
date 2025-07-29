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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import typing
from datetime import timedelta

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts


class TC_TIMESYNC_2_4(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def send_set_time_zone_cmd(self, tz: typing.List[Clusters.Objects.TimeSynchronization.Structs.TimeZoneStruct]) -> Clusters.Objects.TimeSynchronization.Commands.SetTimeZoneResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetTimeZone(timeZone=tz), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.TimeSynchronization.Commands.SetTimeZoneResponse),
                            "Unexpected return type for SetTimeZone")
        return ret

    async def send_set_time_zone_cmd_expect_error(self, tz: typing.List[Clusters.Objects.TimeSynchronization.Structs.TimeZoneStruct], error: Status) -> None:
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetTimeZone(timeZone=tz), endpoint=self.endpoint)
            asserts.assert_true(False, "Unexpected SetTimeZone command success")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    def pics_TC_TIMESYNC_2_4(self) -> list[str]:
        return ["TIMESYNC.S.F00"]

    @async_test_body
    async def test_TC_TIMESYNC_2_4(self):

        # Time sync is required to be on endpoint 0 if it is present
        self.endpoint = 0

        time_cluster = Clusters.Objects.TimeSynchronization
        tz_struct = time_cluster.Structs.TimeZoneStruct

        self.print_step(0, "Commissioning, already done")
        attributes = Clusters.TimeSynchronization.Attributes

        self.print_step(1, "Read TimeZoneDatabase attribute")
        tz_database_dut = await self.read_ts_attribute_expect_success(attribute=attributes.TimeZoneDatabase)

        self.print_step(2, "Read TimeZoneListMaxSize attribute")
        tz_max_size_dut = await self.read_ts_attribute_expect_success(attribute=attributes.TimeZoneListMaxSize)

        self.print_step(3, "Send SetTimeZone command")
        tz = [tz_struct(offset=0, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(4, "Send SetTimeZone command")
        tz = [tz_struct(offset=0, validAt=0, name="")]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(5, "Send SetTimeZone command")
        tz = [tz_struct(offset=-43200, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(6, "Send SetTimeZone command")
        tz = [tz_struct(offset=50400, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(7, "Send SetTimeZone command")
        tz = [tz_struct(offset=3600, validAt=0, name="FakeCountry/FakeCity")]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(8, "Send SetTimeZone command")
        tz = [tz_struct(offset=3600, validAt=0, name="Europe/Dublin")]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        if tz_database_dut is time_cluster.Enums.TimeZoneDatabaseEnum.kNone:
            asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(9, "Send SetTimeZone command")
        if tz_max_size_dut == 2:
            tz = [tz_struct(offset=3600, validAt=0, name="Europe/Dublin"),
                  tz_struct(offset=7200, validAt=utc_time_in_matter_epoch() + timedelta(minutes=2).microseconds, name="Europe/Athens")]
            ret = await self.send_set_time_zone_cmd(tz=tz)

        self.print_step(10, "Send SetTimeZone command - bad validAt time")
        tz = [tz_struct(offset=3600, validAt=utc_time_in_matter_epoch(), name="Europe/Dublin")]
        await self.send_set_time_zone_cmd_expect_error(tz=tz, error=Status.ConstraintError)

        self.print_step(11, "Send SetTimeZone command - bad second entry")
        if tz_max_size_dut == 2:
            tz = [tz_struct(offset=3600, validAt=0, name="Europe/Dublin"), tz_struct(offset=0, validAt=0, name="Europe/Athens")]
            await self.send_set_time_zone_cmd_expect_error(tz=tz, error=Status.ConstraintError)

        self.print_step(12, "Send SetTimeZone command - bad offset (low)")
        tz = [tz_struct(offset=-43201, validAt=0)]
        await self.send_set_time_zone_cmd_expect_error(tz=tz, error=Status.ConstraintError)

        self.print_step(13, "Send SetTimeZone command - bad offset (high)")
        tz = [tz_struct(offset=50401, validAt=0)]
        await self.send_set_time_zone_cmd_expect_error(tz=tz, error=Status.ConstraintError)

        self.print_step(14, "Send SetTimeZone command with max sized name")
        tz = [tz_struct(offset=0, validAt=0, name="AVeryLongStringWithSixtyFourChars/ThisIsSomeExtraPaddingForTheSt")]
        ret = await self.send_set_time_zone_cmd(tz=tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(15, "Send SetTimeZone command - too long name")
        tz = [tz_struct(offset=0, validAt=0, name="AVeryLongStringWithSixtyFiveChars/ThisIsSomeExtraPaddingForTheStr")]
        await self.send_set_time_zone_cmd_expect_error(tz=tz, error=Status.ConstraintError)

        self.print_step(16, "Send SetTimeZone command - too many entries")
        if tz_max_size_dut == 2:
            tz = [tz_struct(offset=3600, validAt=0, name="Europe/Dublin"),
                  tz_struct(offset=7200, validAt=utc_time_in_matter_epoch() +
                            timedelta(minutes=2).microseconds, name="Europe/Athens"),
                  tz_struct(offset=10800, validAt=utc_time_in_matter_epoch() +
                            timedelta(minutes=4).microseconds, name="Europe/Istanbul")
                  ]
            await self.send_set_time_zone_cmd_expect_error(tz=tz, error=Status.ResourceExhausted)

        self.print_step(17, "Send SetTimeZone command - too many entries")
        if tz_max_size_dut == 1:
            tz = [tz_struct(offset=3600, validAt=0, name="Europe/Dublin"),
                  tz_struct(offset=7200, validAt=utc_time_in_matter_epoch() +
                            timedelta(minutes=2).microseconds, name="Europe/Athens")
                  ]
            await self.send_set_time_zone_cmd_expect_error(tz=tz,  error=Status.ResourceExhausted)

        self.print_step(18, "Reset time zone")
        tz = [tz_struct(offset=0, validAt=0)]
        await self.send_set_time_zone_cmd(tz=tz)


if __name__ == "__main__":
    default_matter_test_main()
