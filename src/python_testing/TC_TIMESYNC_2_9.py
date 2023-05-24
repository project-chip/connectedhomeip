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

import typing
from datetime import timedelta

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError
from chip.tlv import uint
from matter_testing_support import (MatterBaseTest, async_test_body, compare_time, default_matter_test_main, type_matches,
                                    utc_time_in_matter_epoch)
from mobly import asserts


class TC_TIMESYNC_2_9(MatterBaseTest):

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

    @async_test_body
    async def test_TC_TIMESYNC_2_9(self):

        self.endpoint = self.user_params.get("endpoint", 0)

        self.print_step(0, "Commissioning, already done")
        time_cluster = Clusters.Objects.TimeSynchronization
        attributes = time_cluster.Attributes
        tz_struct = time_cluster.Structs.TimeZoneStruct
        dst_struct = time_cluster.Structs.DSTOffsetStruct
        utc_attr = attributes.UTCTime
        local_attr = attributes.LocalTime

        self.print_step(1, "Send SetUTCCommand")
        # It doesn't actually matter if this succeeds. The DUT is free to reject this command and use its own time.
        # If the DUT fails to get the time completely, all other tests will fail.
        try:
            await self.send_set_utc_cmd(utc_time_in_matter_epoch())
        except InteractionModelError:
            pass

        self.print_step(2, "Send SetTimeZone command")
        tz = [tz_struct(offset=7200, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(3, "Send SetDSTOffset command")
        dst = [dst_struct(offset=3600, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(4, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(5, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=7200+3600), tolerance=timedelta(seconds=5))

        self.print_step(6, "Send SetDSTOffset command")
        dst = [dst_struct(offset=-3600, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(7, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(8, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=7200-3600), tolerance=timedelta(seconds=5))

        self.print_step(9, "Send SetTimeZone command")
        tz = [tz_struct(offset=-7200, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(10, "Send SetDSTOffset command")
        dst = [dst_struct(offset=3600, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(11, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(12, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=-7200+3600), tolerance=timedelta(seconds=5))

        self.print_step(13, "Send SetDSTOffset command")
        dst = [dst_struct(offset=-3600, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(14, "Read UTCTime")
        utc = await self.read_ts_attribute_expect_success(utc_attr)
        compare_time(received=utc, offset=timedelta(), tolerance=timedelta(seconds=5))

        self.print_step(15, "Read LocalTime")
        local = await self.read_ts_attribute_expect_success(local_attr)
        compare_time(received=local, offset=timedelta(seconds=-7200-3600), tolerance=timedelta(seconds=5))

        self.print_step(16, "Send SetTimeZone command")
        tz = [tz_struct(offset=0, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(17, "Send SetDSTOffset command")
        dst = [dst_struct(offset=0, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)


if __name__ == "__main__":
    default_matter_test_main()
