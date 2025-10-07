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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/soil_2_2_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --app-pipe /tmp/soil_2_2_fifo
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_SOIL_2_2(MatterBaseTest):
    async def read_soil_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.SoilMeasurement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_SOIL_2_2(self) -> str:
        return "[TC-SOIL-2.2] Primary functionality with DUT as Server"

    def steps_TC_SOIL_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Set up a subscription wildcard subscription, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false"),

            TestStep(3, "Read SoilMoistureMeasurementLimits attribute, save the MinMeasuredValue field as min_bound and save the MaxMeasuredValue field as max_bound"),
            TestStep(4, "Read SoilMoistureMeasuredValue attribute and save the value as measurement"),
            TestStep(5, "Perform action to change the moisture of the measured medium"),
            TestStep(6, "After a few seconds, read SoilMoistureMeasuredValue attribute"),
            TestStep(7, "Verify that the DUT sends at least one attribute report for SoilMoistureMeasuredValue"),
        ]
        return steps

    def pics_TC_SOIL_2_2(self) -> list[str]:
        pics = [
            "SOIL.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.SoilMeasurement))
    async def test_TC_SOIL_2_2(self):

        endpoint = self.get_endpoint(default=1)

        self.step(1)
        cluster = Clusters.SoilMeasurement
        attributes = cluster.Attributes

        self.step(2)
        sub_handler = AttributeSubscriptionHandler(expected_cluster=cluster)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        soil_moisture_limits = await self.read_soil_attribute_expect_success(endpoint=endpoint, attribute=attributes.SoilMoistureMeasurementLimits)
        min_bound = soil_moisture_limits.minMeasuredValue
        max_bound = soil_moisture_limits.maxMeasuredValue

        if self.is_pics_sdk_ci_only:
            # Set the initial soil moisture to the min_bound value, since it inits as null.
            logging.info(f"Simulated soil moisture value: {min_bound}")
            self.write_to_app_pipe({"Name": "SetSimulatedSoilMoisture", "SoilMoistureValue": min_bound, "EndpointId": endpoint})

        self.step(4)
        measurement = await self.read_soil_attribute_expect_success(endpoint=endpoint, attribute=attributes.SoilMoistureMeasuredValue)
        asserts.assert_true(measurement != NullValue, "SoilMoistureMeasuredValue is NullValue")
        asserts.assert_greater_equal(measurement, min_bound, "SoilMoistureMeasuredValue is out of range")
        asserts.assert_less_equal(measurement, max_bound, "SoilMoistureMeasuredValue is out of range")

        self.step(5)
        if self.is_pics_sdk_ci_only:
            # Simulate a change in soil moisture, changing to max_bound.
            logging.info(f"Simulated soil moisture value: {max_bound}")
            self.write_to_app_pipe({"Name": "SetSimulatedSoilMoisture", "SoilMoistureValue": max_bound, "EndpointId": endpoint})

        else:
            self.wait_for_user_input(
                prompt_msg="Perform action to change the moisture of the measured medium and wait for measurement, then continue")

        self.step(6)
        measurement_after_action = await self.read_soil_attribute_expect_success(endpoint=endpoint, attribute=attributes.SoilMoistureMeasuredValue)
        asserts.assert_true(measurement_after_action != NullValue, "SoilMoistureMeasuredValue is NullValue")
        asserts.assert_true(measurement_after_action != measurement, "SoilMoistureMeasuredValue is equal to the previous value")
        asserts.assert_greater_equal(measurement_after_action, min_bound, "SoilMoistureMeasuredValue is out of range")
        asserts.assert_less_equal(measurement_after_action, max_bound, "SoilMoistureMeasuredValue is out of range")

        self.step(7)
        count = sub_handler.attribute_report_counts.get(attributes.SoilMoistureMeasuredValue, 0)
        asserts.assert_greater_equal(count, 1, "Unexpected number of SoilMoistureMeasuredValue reports")


if __name__ == "__main__":
    default_matter_test_main()
