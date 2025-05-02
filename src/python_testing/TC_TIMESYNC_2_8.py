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

import time
import typing
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from chip.testing.timeoperations import compare_time, utc_time_in_matter_epoch
from chip.tlv import uint
from mobly import asserts


class TC_TIMESYNC_2_8(MatterBaseTest):
    @property
    def default_timeout(self) -> int:
        # This test has potentially 6 15 s waits, so set timeout to 100
        return 100

    async def read_ts_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def send_set_time_zone_cmd(self, tz: typing.List[Clusters.Objects.TimeSynchronization.Structs.TimeZoneStruct]) -> Clusters.Objects.TimeSynchronization.Commands.SetTimeZoneResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetTimeZone(timeZone=tz), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.TimeSynchronization.Commands.SetTimeZoneResponse),
                            "Unexpected return type for SetTimeZone")
        return ret

    async def send_set_dst_cmd(self, dst: typing.List[Clusters.Objects.TimeSynchronization.Structs.DSTOffsetStruct]) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDSTOffset(DSTOffset=dst))

    async def send_set_utc_cmd(self, utc: uint) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetUTCTime(UTCTime=utc, granularity=Clusters.Objects.TimeSynchronization.Enums.GranularityEnum.kMillisecondsGranularity))

    def pics_TC_TIMESYNC_2_8(self) -> list[str]:
        return ["TIMESYNC.S.F00"]

    @async_test_body
    async def test_TC_TIMESYNC_2_8(self):

        # Time sync is required to be on endpoint 0 if it is present
        self.endpoint = 0

        self.print_step(0, "Commissioning, already done")
        time_cluster = Clusters.Objects.TimeSynchronization
        attributes = time_cluster.Attributes
        tz_struct = time_cluster.Structs.TimeZoneStruct
        dst_struct = time_cluster.Structs.DSTOffsetStruct
        utc_attr = attributes.UTCTime
        local_attr = attributes.LocalTime

        self.print_step(1, "Send SetTimeZone command with 0 offset")
        tz = [tz_struct(offset=0, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(2, "Send SetDSTOffset command")
        dst = [dst_struct(offset=0, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(3, "Send SetUTCCommand")
        # It doesn't actually matter if this succeeds. The DUT is free to reject this command and use its own time.
        # If the DUT fails to get the time completely, all other tests will fail.
        try:
            await self.send_set_utc_cmd(utc_time_in_matter_epoch())
        except InteractionModelError:
            pass

        self.print_step(4, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(5, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(6, "Send SetDSTOffset command")
        th_utc = utc_time_in_matter_epoch()
        dst = [dst_struct(offset=3600, validStarting=0, validUntil=th_utc+1e+7)]
        await self.send_set_dst_cmd(dst)

        self.print_step(7, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(8, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=3600), tolerance=timedelta(seconds=5))

        self.print_step(9, "Wait 15s")
        time.sleep(15)

        self.print_step(10, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(11, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        asserts.assert_equal(local, NullValue, "LocalTime cannot be calculated since DST is empty")

        self.print_step(12, "Send SetDSTOffset command")
        dst = [dst_struct(offset=3600, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(13, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=3600), tolerance=timedelta(seconds=5))

        self.print_step(14, "Wait 15s")
        time.sleep(15)

        self.print_step(15, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=3600), tolerance=timedelta(seconds=5))

        self.print_step(16, "Read DSTOffsetListMaxSize")
        dst_list_size = await self.read_ts_attribute_expect_success(attributes.DSTOffsetListMaxSize)

        self.print_step(17, "Send multiple DST offsets")
        if dst_list_size > 1:
            th_utc = utc_time_in_matter_epoch()
            dst = [dst_struct(offset=3600, validStarting=0, validUntil=th_utc+1e+7),
                   dst_struct(offset=7200, validStarting=th_utc+2.5e+7, validUntil=th_utc+4e+7)]
            await self.send_set_dst_cmd(dst)

        self.print_step(18, "Read LocalTime")
        if dst_list_size > 1:
            local = await self.read_ts_attribute_expect_success(local_attr)
            compare_time(received=local, offset=timedelta(seconds=3600), tolerance=timedelta(seconds=5))

        self.print_step(19, "Wait 15s")
        if dst_list_size > 1:
            time.sleep(15)

        self.print_step(20, "Read LocalTime")
        if dst_list_size > 1:
            local = await self.read_ts_attribute_expect_success(local_attr)
            compare_time(received=local, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(21, "Wait 15s")
        if dst_list_size > 1:
            time.sleep(15)

        self.print_step(22, "Read LocalTime")
        if dst_list_size > 1:
            local = await self.read_ts_attribute_expect_success(local_attr)
            compare_time(received=local, offset=timedelta(seconds=7200), tolerance=timedelta(seconds=5))

        self.print_step(23, "Wait 15s")
        if dst_list_size > 1:
            time.sleep(15)

        self.print_step(24, "Read LocalTime")
        if dst_list_size > 1:
            local = await self.read_ts_attribute_expect_success(local_attr)
            asserts.assert_equal(local, NullValue, "LocalTime cannot be calculated since DST is empty")

        self.print_step(25, "Send SetDSTOffset command")
        dst = [dst_struct(offset=-3600, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(26, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=-3600), tolerance=timedelta(seconds=5))

        self.print_step(27, "Send SetDSTOffset command with DST starting in the future")
        valid = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc) + timedelta(seconds=10))
        dst = [dst_struct(offset=3600, validStarting=valid, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(28, "Read Localtime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=0), tolerance=timedelta(seconds=5))

        self.print_step(29, "Wait 15s")
        time.sleep(15)

        self.print_step(30, "Read Localtime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=3600), tolerance=timedelta(seconds=5))

        self.print_step(31, "Send SetDSTOffset command")
        dst = [dst_struct(offset=0, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)


if __name__ == "__main__":
    default_matter_test_main()
