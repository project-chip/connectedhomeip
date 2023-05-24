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

from datetime import timedelta

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, compare_time, default_matter_test_main, utc_time_in_matter_epoch
from mobly import asserts


class TC_TIMESYNC_2_2(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @async_test_body
    async def test_TC_TIMESYNC_2_2(self):

        endpoint = self.user_params.get("endpoint", 0)

        time_cluster = Clusters.Objects.TimeSynchronization

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.TimeSynchronization.Attributes

        self.print_step(2, "Read UTCTime attribute")
        utc_dut_initial = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.UTCTime)
        th_utc = utc_time_in_matter_epoch()
        try:
            await self.send_single_cmd(cmd=time_cluster.Commands.SetUTCTime(UTCTime=th_utc, granularity=time_cluster.Enums.GranularityEnum.kMillisecondsGranularity), endpoint=endpoint)
            code = 0
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

        th_utc = utc_time_in_matter_epoch()
        utc_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.UTCTime)
        asserts.assert_is_not(utc_dut, NullValue, "Received null value for UTCTime after set")
        if granularity_dut == time_cluster.Enums.GranularityEnum.kMinutesGranularity:
            tolerance = timedelta(minutes=10)
        else:
            tolerance = timedelta(minutes=1)
        compare_time(received=utc_dut, utc=th_utc, tolerance=tolerance)


if __name__ == "__main__":
    default_matter_test_main()
