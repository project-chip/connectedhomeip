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

from datetime import timedelta

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from chip.testing.timeoperations import compare_time, utc_time_in_matter_epoch
from mobly import asserts


class TC_TIMESYNC_2_2(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def pics_TC_TIMESYNC_2_2(self) -> list[str]:
        return ["TIMESYNC.S"]

    @async_test_body
    async def test_TC_TIMESYNC_2_2(self):

        # Time sync is required to be on endpoint 0 if it is present
        endpoint = 0

        time_cluster = Clusters.TimeSynchronization
        timesync_attr_list = time_cluster.Attributes.AttributeList
        attribute_list = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=time_cluster, attribute=timesync_attr_list)
        timesource_attr_id = time_cluster.Attributes.TimeSource.attribute_id

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.TimeSynchronization.Attributes

        self.print_step(2, "Read UTCTime attribute")
        utc_dut_initial = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.UTCTime)
        th_utc = utc_time_in_matter_epoch()

        code = 0
        try:
            await self.send_single_cmd(cmd=time_cluster.Commands.SetUTCTime(UTCTime=th_utc, granularity=time_cluster.Enums.GranularityEnum.kMillisecondsGranularity), endpoint=endpoint)
        except InteractionModelError as e:
            # The python layer discards the cluster specific portion of the status IB, so for now we just expect a generic FAILURE error
            # see #26521
            code = e.status
            pass

        if utc_dut_initial is NullValue:
            asserts.assert_equal(code, 0, "Unexpected error returned for null UTCTime")
        else:
            asserts.assert_true(code in [0, 1], "Unexpected error returned for non-null UTCTime")

        self.print_step(3, "Read Granulatiry attribute")
        granularity_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.Granularity)
        asserts.assert_less(granularity_dut, time_cluster.Enums.GranularityEnum.kUnknownEnumValue,
                            "Granularity out of expected range")
        asserts.assert_not_equal(granularity_dut, time_cluster.Enums.GranularityEnum.kNoTimeGranularity)

        self.print_step(4, "Read UTC time")
        th_utc = utc_time_in_matter_epoch()
        utc_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.UTCTime)
        asserts.assert_is_not(utc_dut, NullValue, "Received null value for UTCTime after set")
        if granularity_dut == time_cluster.Enums.GranularityEnum.kMinutesGranularity:
            tolerance = timedelta(minutes=10)
        else:
            tolerance = timedelta(minutes=1)
        compare_time(received=utc_dut, utc=th_utc, tolerance=tolerance)

        self.print_step(5, "Read time source")
        if timesource_attr_id in attribute_list:
            source = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TimeSource)
            if utc_dut_initial is NullValue:
                asserts.assert_equal(source, Clusters.Objects.TimeSynchronization.Enums.TimeSourceEnum.kAdmin)


if __name__ == "__main__":
    default_matter_test_main()
