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

import asyncio
import contextlib
import queue
import typing
from datetime import datetime, timedelta, timezone

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, matchers
from matter.testing.runner import default_matter_test_main
from matter.testing.timeoperations import get_wait_seconds_from_set_time, utc_time_in_matter_epoch
from matter.tlv import uint


class TC_TIMESYNC_2_11(MatterBaseTest):
    async def send_set_time_zone_cmd(self, tz: typing.List[Clusters.Objects.TimeSynchronization.Structs.TimeZoneStruct]) -> Clusters.Objects.TimeSynchronization.Commands.SetTimeZoneResponse:
        return await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetTimeZone(timeZone=tz), endpoint=self.endpoint)

    async def send_set_dst_cmd(self, dst: typing.List[Clusters.Objects.TimeSynchronization.Structs.DSTOffsetStruct]) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDSTOffset(DSTOffset=dst))

    async def send_set_utc_cmd(self, utc: uint) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetUTCTime(UTCTime=utc, granularity=Clusters.Objects.TimeSynchronization.Enums.GranularityEnum.kMillisecondsGranularity))

    def wait_for_dst_status(self, th_utc, wait_s, expect_active, cb):
        """
        Waits for the DSTStatus event to be received and validates its contents.

        This function waits for the DSTStatus event to be received via the given callback.
        It verifies that the event type is correct and that the field DSTOffsetAcive matches the expected value.

        Parameters:
            th_utc (int): The UTC time in seconds used for timeout.
            wait_s (int): Additional wait time in secods.
            expect_active (bool): Expected value of the DSTOffsetActive field in the event.
            cb: The callback object from which the event will be pulled.

        Raises:
            AssertionError: If no event is received in time, the type is incorrect or the value does not match.
        """

        timeout = get_wait_seconds_from_set_time(th_utc, wait_s)
        try:
            ret = cb.get_event_from_queue(block=True, timeout=timeout)
            asserts.assert_true(matchers.is_type(received_value=ret.Data,
                                desired_type=Clusters.TimeSynchronization.Events.DSTStatus), "Unexpected event type returned")
            asserts.assert_equal(ret.Data.DSTOffsetActive, expect_active, "Unexpected value for DSTOffsetActive")
        except queue.Empty:
            asserts.fail("Did not receive DSTStatus event")
            pass

    def pics_TC_TIMESYNC_2_11(self) -> list[str]:
        return ["TIMESYNC.S.F00"]

    @async_test_body
    async def test_TC_TIMESYNC_2_11(self):

        self.endpoint = 0

        self.print_step(0, "Commissioning, already done")
        time_cluster = Clusters.Objects.TimeSynchronization
        tz_struct = time_cluster.Structs.TimeZoneStruct
        dst_struct = time_cluster.Structs.DSTOffsetStruct

        self.print_step(1, "Send SetUTCCommand")
        # It doesn't actually matter if this succeeds. The DUT is free to reject this command and use its own time.
        # If the DUT fails to get the time completely, all other tests will fail.
        with contextlib.suppress(InteractionModelError):
            await self.send_set_utc_cmd(utc_time_in_matter_epoch())

        self.print_step(2, "Send SetTimeZone command")
        tz = [tz_struct(offset=7200, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(3, "Subscribe to DSTStatus event")
        event = time_cluster.Events.DSTStatus
        cb = EventSubscriptionHandler(expected_cluster_id=event.cluster_id, expected_event_id=event.event_id)
        urgent = 1
        subscription = await self.default_controller.ReadEvent(nodeId=self.dut_node_id, events=[(self.endpoint, event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        self.print_step(4, "TH reads the DSTOffsetListMaxSize")
        dst_list_size = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.DSTOffsetListMaxSize)
        asserts.assert_greater_equal(dst_list_size, 1, "Invalid dst list size")

        self.print_step(5, "TH sets two DST items if dst_list_size > 1")
        th_utc = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc))
        expiry_first = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc) + timedelta(seconds=10))
        dst_first = dst_struct(offset=3600, validStarting=0, validUntil=expiry_first)
        if dst_list_size > 1:
            start_second = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc) + timedelta(seconds=25))
            expiry_second = utc_time_in_matter_epoch(datetime.now(tz=timezone.utc) + timedelta(seconds=40))
            dst_second = dst_struct(offset=3600, validStarting=start_second, validUntil=expiry_second)
            dst = [dst_first, dst_second]
            await self.send_set_dst_cmd(dst)

        self.print_step(6, "TH sets 1 DST item if dst_list_size == 1")
        if dst_list_size == 1:
            dst = [dst_first]
            await self.send_set_dst_cmd(dst)

        self.print_step(7, "TH reads LocalTime")
        await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.LocalTime)

        self.print_step(8, "TH waits for DSTStatus event until th_utc + 5s")
        self.wait_for_dst_status(th_utc, 5, True, cb)

        self.print_step(9, "If dst_list_size > 1, TH waits until th_utc + 15s")
        if dst_list_size > 1:
            await asyncio.sleep(get_wait_seconds_from_set_time(th_utc, 15))

        self.print_step(10, "If dst_list_size > 1, TH reads LocalTime")
        if dst_list_size > 1:
            await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.LocalTime)

        self.print_step(11, "If dst_list_size > 1, TH waits for DSTStatus event until th_utc + 20s")
        if dst_list_size > 1:
            self.wait_for_dst_status(th_utc, 20, False, cb)

        self.print_step(12, "If dst_list_size > 1, TH waits until th_utc + 30s")
        if dst_list_size > 1:
            await asyncio.sleep(get_wait_seconds_from_set_time(th_utc, 30))

        self.print_step(13, "If dst_list_size > 1, TH reads LocalTime")
        if dst_list_size > 1:
            await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.LocalTime)

        self.print_step(14, "If dst_list_size > 1, TH waits for a DSTStatus event until th_utc + 35s")
        if dst_list_size > 1:
            self.wait_for_dst_status(th_utc, 35, True, cb)

        self.print_step(15, "Set time zone back to 0")
        tz = [tz_struct(offset=0, validAt=0)]
        ret = await self.send_set_time_zone_cmd(tz)
        asserts.assert_true(ret.DSTOffsetRequired, "DSTOffsetRequired not set to true")

        self.print_step(16, "Set DST back to 0")
        dst = [dst_struct(offset=0, validStarting=0, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst)


if __name__ == "__main__":
    default_matter_test_main()
