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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts


class TC_TIMESYNC_2_5(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def send_set_dst_cmd(self, dst: typing.List[Clusters.Objects.TimeSynchronization.Structs.DSTOffsetStruct]) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDSTOffset(DSTOffset=dst))

    async def send_set_dst_cmd_expect_error(self, dst: typing.List[Clusters.Objects.TimeSynchronization.Structs.DSTOffsetStruct], error: Status) -> None:
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDSTOffset(DSTOffset=dst))
            asserts.assert_true(False, "Unexpected SetTimeZone command success")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    def pics_TC_TIMESYNC_2_5(self) -> list[str]:
        return ["TIMESYNC.S.F00"]

    @async_test_body
    async def test_TC_TIMESYNC_2_5(self):

        # Time sync is required to be on endpoint 0 if it is present
        self.endpoint = 0

        time_cluster = Clusters.Objects.TimeSynchronization
        dst_struct = time_cluster.Structs.DSTOffsetStruct

        self.print_step(0, "Commissioning, already done")
        attributes = Clusters.TimeSynchronization.Attributes
        dst_offset_attr = attributes.DSTOffset

        self.print_step(1, "Read DSTOffsetListMaxSize attribute")
        dst_max_size_dut = await self.read_ts_attribute_expect_success(attribute=attributes.DSTOffsetListMaxSize)
        asserts.assert_greater_equal(dst_max_size_dut, 1, "DSTOffsetListMaxSize must be at least 1")

        self.print_step(2, "Send SetDSTOffset command")
        dst = []
        await self.send_set_dst_cmd(dst=dst)

        self.print_step(3, "Read DSTOffset attribute")
        attr = await self.read_ts_attribute_expect_success(dst_offset_attr)
        asserts.assert_false(attr, "DSTOffset not set correctly to empty list")

        self.print_step(4, "Test setting unsorted list - expect error")
        if dst_max_size_dut > 1:
            th_utc = utc_time_in_matter_epoch()
            dst = [dst_struct(offset=3600, validStarting=th_utc, validUntil=th_utc+1.577e+13),
                   dst_struct(offset=3600, validStarting=0, validUntil=th_utc)]
            await self.send_set_dst_cmd_expect_error(dst=dst, error=Status.ConstraintError)

        self.print_step(5, "Read DSTOffset attribute - expect empty")
        if dst_max_size_dut > 1:
            attr = await self.read_ts_attribute_expect_success(dst_offset_attr)
            asserts.assert_false(attr, "DSTOffset not set correctly to empty list")

        self.print_step(6, "Test setting list with invalid second entry - expect error")
        if dst_max_size_dut > 1:
            th_utc = utc_time_in_matter_epoch()
            dst = [dst_struct(offset=3600, validStarting=0, validUntil=th_utc+3e+8),
                   dst_struct(offset=3600, validStarting=th_utc, validUntil=th_utc+1.577e+13)]
            await self.send_set_dst_cmd_expect_error(dst=dst, error=Status.ConstraintError)

        self.print_step(7, "Read DSTOffset attribute - expect empty")
        if dst_max_size_dut > 1:
            attr = await self.read_ts_attribute_expect_success(dst_offset_attr)
            asserts.assert_false(attr, "DSTOffset not set correctly to empty list")

        self.print_step(8, "Test setting list with two null values - expect error")
        if dst_max_size_dut > 1:
            th_utc = utc_time_in_matter_epoch()
            dst = [dst_struct(offset=3600, validStarting=0, validUntil=NullValue),
                   dst_struct(offset=3600, validStarting=th_utc+3e+8, validUntil=NullValue)]
            await self.send_set_dst_cmd_expect_error(dst=dst, error=Status.ConstraintError)

        self.print_step(9, "Read DSTOffset attribute - expect empty")
        if dst_max_size_dut > 1:
            attr = await self.read_ts_attribute_expect_success(dst_offset_attr)
            asserts.assert_false(attr, "DSTOffset not set correctly to empty list")

        self.print_step(10, "Test setting list with null value not at end - expect error")
        if dst_max_size_dut > 1:
            th_utc = utc_time_in_matter_epoch()
            dst = [dst_struct(offset=3600, validStarting=0, validUntil=NullValue),
                   dst_struct(offset=3600, validStarting=th_utc+3e+8, validUntil=th_utc+1.577e+13)]
            await self.send_set_dst_cmd_expect_error(dst=dst, error=Status.ConstraintError)

        self.print_step(11, "Read DSTOffset attribute - expect empty")
        if dst_max_size_dut > 1:
            attr = await self.read_ts_attribute_expect_success(dst_offset_attr)
            asserts.assert_false(attr, "DSTOffset not set correctly to empty list")

        self.print_step(12, "Test setting too many entries")
        dst = []
        th_utc = utc_time_in_matter_epoch()
        for i in range(dst_max_size_dut+1):
            year = 3.156e+13
            six_months = 1.577e+13
            vstart = year*i + six_months
            vuntil = year * (i+1)
            dst.append(dst_struct(offset=3600, validStarting=vstart, validUntil=vuntil))
        await self.send_set_dst_cmd_expect_error(dst=dst, error=Status.ResourceExhausted)

        self.print_step(13, "Read DSTOffset attribute - expect empty")
        attr = await self.read_ts_attribute_expect_success(dst_offset_attr)
        asserts.assert_false(attr, "DSTOffset not set correctly to empty list")

        self.print_step(14, "Set valid list with null ValidUntil")
        th_utc = utc_time_in_matter_epoch()
        dst = [dst_struct(offset=3600, validStarting=th_utc+3e+8, validUntil=NullValue)]
        await self.send_set_dst_cmd(dst=dst)

        self.print_step(15, "Read back list")
        dut_dst = await self.read_ts_attribute_expect_success(dst_offset_attr)
        asserts.assert_equal(dut_dst, dst)

        self.print_step(16, "Set valid list with non-null ValidUntil")
        th_utc = utc_time_in_matter_epoch()
        dst = [dst_struct(offset=3600, validStarting=th_utc+3e+8, validUntil=th_utc+1.577e+13)]
        await self.send_set_dst_cmd(dst=dst)

        self.print_step(17, "Read back list")
        dut_dst = await self.read_ts_attribute_expect_success(dst_offset_attr)
        asserts.assert_equal(dut_dst, dst)

        self.print_step(18, "Test setting max entries")
        dst = []
        th_utc = utc_time_in_matter_epoch()
        for i in range(dst_max_size_dut):
            year = 3.156e+13
            six_months = 1.577e+13
            vstart = th_utc + year*i + six_months
            vuntil = th_utc + year * (i+1)
            dst.append(dst_struct(offset=3600, validStarting=vstart, validUntil=vuntil))
        await self.send_set_dst_cmd(dst=dst)

        self.print_step(19, "Read back list")
        dut_dst = await self.read_ts_attribute_expect_success(dst_offset_attr)
        asserts.assert_equal(dut_dst, dst)

        self.print_step(20, "Set empty list")
        dst = []
        await self.send_set_dst_cmd(dst=dst)


if __name__ == "__main__":
    default_matter_test_main()
