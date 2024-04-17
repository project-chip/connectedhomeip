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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TMP_2_1(MatterBaseTest):
    def desc_TC_TMP_2_1(self) -> str:
        return "[TC-TMP-2.1] Attributes with Server as DUT"

    def pics_TC_TMP_2_1(self):
        return ["TMP.S"]

    def steps_TC_TMP_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(
                2, "Set default bounds `min_bound` = -27315, `max_bound` = 32767"),
            TestStep(3, "TH reads the MinMeasuredValue attribute from the DUT and saves as `min_measured_value`. If `min_measured_value` is not null, set `min_bound` to `min_measured_value`"),
            TestStep(4, "TH reads the MaxMeasuredValue attribute from the DUT and saves as `max_measured_value`. If `max_measured_value` is not null, set `max_bound` to `max_measured_value",
                     "Verify that `max_measured_value` is either null or an int16 where min_bound < `max_measured_value` ≤ 32767."),
            TestStep(5, "If `min_measured_value` is not null, verify min measured value range",
                     "Verify that -27315 ≤ `min_measured_value` < `max_bound`"),
            TestStep(6, "TH reads the MeasuredValue attribute from the DUT",
                     "Verify that the DUT response contains either null or a int16 where `min_bound` ≤ MeasuredValue ≤ `max_bound`."),
            TestStep(7, "TH reads the Tolerance attribute from the DUT",
                     "Verify that Tolerance is in the range of 0 to 2048"),
        ]

    @async_test_body
    async def test_TC_TMP_2_1(self):
        cluster = Clusters.TemperatureMeasurement
        attr = Clusters.TemperatureMeasurement.Attributes

        # Commission DUT - already done
        self.step(1)

        self.step(2)
        min_bound = -27315
        max_bound = 32767

        self.step(3)
        min_measured_value = await self.read_single_attribute_check_success(cluster=cluster, attribute=attr.MinMeasuredValue)
        if min_measured_value != NullValue:
            min_bound = min_measured_value

        self.step(4)
        max_measured_value = await self.read_single_attribute_check_success(cluster=cluster, attribute=attr.MaxMeasuredValue)
        if max_measured_value != NullValue:
            max_bound = max_measured_value
            asserts.assert_greater(max_measured_value, min_bound,
                                   "MaxMeasuredValue is not greater than the minimum bound")
            asserts.assert_less_equal(
                max_measured_value, 32767, "MaxMeasuredValue is not less than or equal to than 32767")

        self.step(5)
        if min_measured_value != NullValue:
            asserts.assert_greater_equal(min_measured_value, -27315, "MinMeasuredValue is out of range")
            asserts.assert_less(min_measured_value, max_bound, "MinMeasuredValue is out of range")
        else:
            self.mark_current_step_skipped()

        self.step(6)
        measured_value = await self.read_single_attribute_check_success(cluster=cluster, attribute=attr.MeasuredValue)
        if measured_value != NullValue:
            print(measured_value)
            print(min_bound)
            asserts.assert_greater_equal(
                measured_value, min_bound, "Measured value is less than min bound")
            asserts.assert_less_equal(
                measured_value, max_bound, "Measured value is greater than max bound")

        self.step(7)
        tolerance = await self.read_single_attribute_check_success(cluster=cluster, attribute=attr.Tolerance)
        asserts.assert_greater_equal(tolerance, 0, "Tolerance is less than 0")
        asserts.assert_less_equal(
            tolerance, 2048, "Tolerance is greater than 2048")


if __name__ == "__main__":
    default_matter_test_main()
