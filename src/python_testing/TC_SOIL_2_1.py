#
#    Copyright (c) 2025 Project CHIP Authors
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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts


class TC_SOIL_2_1(MatterBaseTest):
    async def read_soil_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.SoilMeasurement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_SOIL_2_1(self) -> str:
        return "[TC-SOIL-2.1] Attributes with DUT as Server"

    def steps_TC_SOIL_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read SoilMoistureMeasurementLimits attribute and saves the value as soil_moisture_limits"),
            TestStep(3, "Verify the MeasurementType field in soil_moisture_limits"),
            TestStep(4, "Verify the MinMeasuredValue field in soil_moisture_limits and save the MinMeasuredValue field as min_bound"),
            TestStep(5, "Verify the MaxMeasuredValue field in soil_moisture_limits and save the MaxMeasuredValue field as max_bound"),
            TestStep(6, "Verify the number of entries in the AccuracyRanges in soil_moisture_limits"),
            TestStep(7, "Verify the RangeMin field of the AccuracyRanges entry in soil_moisture_limits"),
            TestStep(8, "Verify the RangeMax field of the AccuracyRanges entry in soil_moisture_limits"),
            TestStep(9, "Verify the PercentMax field of the AccuracyRanges entry in soil_moisture_limits"),
            TestStep(10, "Verify no other fields is present in the AccuracyRanges entry"),
            TestStep(11, "Read SoilMoistureMeasuredValue attribute."),
        ]
        return steps

    def pics_TC_SOIL_2_1(self) -> list[str]:
        pics = [
            "SOIL.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.SoilMeasurement))
    async def test_TC_SOIL_2_1(self):

        endpoint = self.get_endpoint(default=1)

        self.step(1)
        attributes = Clusters.SoilMeasurement.Attributes

        self.step(2)
        soil_moisture_limits = await self.read_soil_attribute_expect_success(endpoint=endpoint, attribute=attributes.SoilMoistureMeasurementLimits)

        self.step(3)
        asserts.assert_equal(soil_moisture_limits.measurementType, 17, "MeasurementType field is not the correct value")

        self.step(4)
        min_bound = soil_moisture_limits.minMeasuredValue
        asserts.assert_greater_equal(min_bound, 0, "MinMeasuredValue field is out of range")
        asserts.assert_less_equal(min_bound, 99, "MinMeasuredValue field is out of range")

        self.step(5)
        max_bound = soil_moisture_limits.maxMeasuredValue
        asserts.assert_greater_equal(max_bound,
                                     (min_bound + 1), "MaxMeasuredValue field is out of range")
        asserts.assert_less_equal(max_bound, 100, "MaxMeasuredValue field is out of range")

        self.step(6)
        number_of_entries = len(soil_moisture_limits.accuracyRanges)
        asserts.assert_equal(number_of_entries, 1, "Number of entries in AccuracyRanges is not equal to 1")

        self.step(7)
        range_min = soil_moisture_limits.accuracyRanges[0].rangeMin
        asserts.assert_equal(range_min, min_bound, "RangeMin field is out of range")

        self.step(8)
        range_max = soil_moisture_limits.accuracyRanges[0].rangeMax
        asserts.assert_equal(range_max, max_bound, "RangeMax field is out of range")

        self.step(9)
        percent_max = soil_moisture_limits.accuracyRanges[0].percentMax
        asserts.assert_less_equal(percent_max, 10, "PercentMax field is out of range")

        self.step(10)
        for field in soil_moisture_limits.accuracyRanges[0].__dict__:
            if field not in ["rangeMin", "rangeMax", "percentMax"]:
                asserts.assert_equal(soil_moisture_limits.accuracyRanges[0].__dict__[
                                     field], None, f"Field {field} is present and contains a value")

        self.step(11)
        soil_moisture_measurement = await self.read_soil_attribute_expect_success(endpoint=endpoint, attribute=attributes.SoilMoistureMeasuredValue)

        # If the response is not NullValue, check if it is in the range of min_bound and max_bound
        if soil_moisture_measurement is not NullValue:
            asserts.assert_greater_equal(soil_moisture_measurement, min_bound, "SoilMoistureMeasuredValue is out of range")
            asserts.assert_less_equal(soil_moisture_measurement, max_bound, "SoilMoistureMeasuredValue is out of range")


if __name__ == "__main__":
    default_matter_test_main()
