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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ENERGY_GATEWAY_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_EGCTestBase import ElectricalGridConditionsTestBaseHelper

cluster = Clusters.ElectricalGridConditions


class TC_EGC_2_1(ElectricalGridConditionsTestBaseHelper, MatterBaseTest):
    """Implementation of test case TC_EGC_2_1."""

    def desc_TC_EGC_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-EGC-2.1] Attributes with DUT as Server"

    def pics_TC_EGC_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EGC.S"]

    def steps_TC_EGC_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the LocalGenerationAvailable attribute.",
                     "Verify that the DUT response contains either null or a bool value."),
            TestStep("3", "TH reads from the DUT the CurrentConditions attribute.",
                     "Verify that the DUT response contains either null or an ElectricalGridConditionsStruct value."),
            TestStep("4", "TH reads from the DUT the ForecastConditions attribute.",
                     "Verify that the DUT response contains a list of ElectricalGridConditionsStruct entries"),
        ]
        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalGridConditions))
    async def test_TC_EGC_2_1(self):
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.LocalGenerationAvailable)
        if val is not NullValue:
            matter_asserts.assert_valid_bool(val, 'LocalGenerationAvailable')

        self.step("3")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentConditions)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.ElectricalGridConditionsStruct), "val must be of type ElectricalGridConditionsStruct")
            self.check_ElectricalGridConditionsStruct(cluster=cluster, struct=val)

        self.step("4")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ForecastConditions):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ForecastConditions)
            self.check_ForecastConditions(cluster=cluster, forecastConditions=val)


if __name__ == "__main__":
    default_matter_test_main()
