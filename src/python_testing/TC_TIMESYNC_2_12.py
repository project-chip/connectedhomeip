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

import queue
import time
import typing
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError
from chip.testing.matter_testing import MatterBaseTest, SimpleEventCallback, async_test_body, default_matter_test_main, type_matches
from chip.testing.timeoperations import get_wait_seconds_from_set_time, utc_time_in_matter_epoch
from chip.tlv import uint
from mobly import asserts


class TC_TIMESYNC_2_12(MatterBaseTest):
    async def send_set_time_zone_cmd(self, tz: typing.List[Clusters.Objects.TimeSynchronization.Structs.TimeZoneStruct]) -> Clusters.Objects.TimeSynchronization.Commands.SetTimeZoneResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetTimeZone(timeZone=tz), endpoint=self.endpoint)
        return ret

    async def send_set_dst_cmd(self, dst: typing.List[Clusters.Objects.TimeSynchronization.Structs.DSTOffsetStruct]) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDSTOffset(DSTOffset=dst))

    async def send_set_utc_cmd(self, utc: uint) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetUTCTime(UTCTime=utc, granularity=Clusters.Objects.TimeSynchronization.Enums.GranularityEnum.kMillisecondsGranularity))

    def wait_for_tz_status(self, th_utc, wait_s, expected_offset, expected_name):
        timeout = get_wait_seconds_from_set_time(th_utc, wait_s)
        try:
            ret = self.q.get(block=True, timeout=timeout)
            asserts.assert_true(type_matches(received_value=ret.Data,
                                desired_type=Clusters.TimeSynchronization.Events.TimeZoneStatus), "Incorrect type received for event")
            asserts.assert_equal(ret.Data.offset, expected_offset, "Unexpected offset returned")
            asserts.assert_equal(ret.Data.name, expected_name, "Unexpected name returned")
        except queue.Empty:
            asserts.fail("Did not receive TZStatus event")

    def pics_TC_TIMESYNC_2_12(self) -> list[str]:
        return ["TIMESYNC.S.F00"]

    @async_test_body
    async def test_TC_TIMESYNC_2_12(self):

        self.endpoint = 0

        self.print_step(0, "Commissioning, already done")
        time_cluster = Clusters.Objects.TimeSynchronization
        tz_struct = time_cluster.Structs.TimeZoneStruct
        dst_struct = time_cluster.Structs.DSTOffsetStruct

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

        self.print_step(3, "Subscribe to TimeZoneStatus event")
        event = time_cluster.Events.TimeZoneStatus
        self.q = queue.Queue()
        cb = SimpleEventCallback("TimeZoneStatus", event.cluster_id, event.event_id, self.q)
        urgent = 1
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(self.endpoint, event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        self.print_step(4, "TH reads the TimeZoneListMaxSize")
        tz_list_size = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TimeZoneListMaxSize)
        asserts.assert_greater_equal(tz_list_size, 1, "Invalid tz list size")

        self.print_step(5, "TH sets two TZ items if dst_list_size > 1")
        th_utc = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc))
        tz_first = tz_struct(offset=3600, validAt=0, name="Not/Real")
        if tz_list_size > 1:
            valid_second = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc) + timedelta(seconds=10))
            tz_second = tz_struct(offset=7200, validAt=valid_second, name="Un/Real")
            tz = [tz_first, tz_second]
            await self.send_set_time_zone_cmd(tz)

        self.print_step(6, "TH sets 1 TZ item if tz_list_size == 1")
        if tz_list_size == 1:
            tz = [tz_first]
            await self.send_set_time_zone_cmd(tz)

        self.print_step(7, "TH sets DST offsets")
        dst = [dst_struct(offset=0, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)

        self.print_step(8, "TH reads LocalTime")
        await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.LocalTime)

        self.print_step(9, "TH waits for TimeZoneStatus event until th_utc + 5s")
        self.wait_for_tz_status(th_utc, 5, 3600, "Not/Real")

        self.print_step(10, "If tz_list_size > 1, TH waits until th_utc + 15s")
        if tz_list_size > 1:
            time.sleep(get_wait_seconds_from_set_time(th_utc, 15))

        self.print_step(11, "If tz_list_size > 1, TH reads LocalTime")
        if tz_list_size > 1:
            await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.LocalTime)

        self.print_step(12, "if tz_list_size > 1, TH waits for a TimeZoneStatus event until th_utc + 20s")
        if tz_list_size > 1:
            self.wait_for_tz_status(th_utc, 20, 7200, "Un/Real")

        self.print_step(13, "Set time zone back to 0")
        tz = [tz_struct(offset=0, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(14, "Set DST back to 0")
        dst = [dst_struct(offset=0, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)


if __name__ == "__main__":
    default_matter_test_main()
