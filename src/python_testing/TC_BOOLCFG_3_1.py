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

import logging
from random import choice

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_feature,
                                           run_if_endpoint_matches, async_test_body)


class TC_BOOLCFG_3_1(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_3_1(self) -> str:
        return "[TC-BOOLCFG-3.1] SensitivityLevel with DUT as Server"

    def steps_TC_BOOLCFG_3_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read AttributeList attribute", "DUT replies with the AttributeList attribute"),
            TestStep(3, "Read SupportedSensitivityLevels attribute", "DUT response is success"),
            TestStep(4, "Read DefaultSensitivityLevel attribute, if supported", "DUT response is success"),
            TestStep(5, "Read CurrentSensitivityLevel attribute", "DUT response is success"),
            TestStep(6, "TH loops through the number of supported sensitivity levels",
                     "DUT response is success for all write interactions"),
            TestStep(7, "Write CurrentSensitivityLevel attribute to non-default value", "DUT response is success"),
            TestStep(8, "Write CurrentSensitivityLevel attribute to default value", "DUT response is success"),
            TestStep(9, "Write CurrentSensitivityLevel attribute to max number of level (one above SupportedSensitivityLevels)",
                     "DUT response is contraint error"),
            TestStep(10, "Write CurrentSensitivityLevel attribute to 255", "DUT response is constraint error"),
            TestStep(11, "Write CurrentSensitivityLevel attribute to the initial current value", "DUT response is success"),
        ]

    def pics_TC_BOOLCFG_3_1(self) -> list[str]:
        return [
            "BOOLCFG.S.F03",
        ]

    @run_if_endpoint_matches(has_feature(Clusters.BooleanStateConfiguration, Clusters.BooleanStateConfiguration.Bitmaps.Feature.kSensitivityLevel))
    async def test_TC_BOOLCFG_3_1(self):

        endpoint = self.get_endpoint()
        attributes = Clusters.BooleanStateConfiguration.Attributes

        # Commissioning
        self.step(1)

        self.step(2)
        attribute_list = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        numberOfSupportedLevels = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedSensitivityLevels)
        asserts.assert_greater_equal(numberOfSupportedLevels, 2,
                                     "SupportedSensitivityLevels must be greater or equal than 2 when SENSLVL feature is supported")

        self.step(4)
        if attributes.DefaultSensitivityLevel.attribute_id in attribute_list:
            default_level = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultSensitivityLevel)
        else:
            logging.info("DefaultSensitivityLevel not present in AttributeList, step 4 skipped")
            self.mark_current_step_skipped()

        self.step(5)
        current_level = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentSensitivityLevel)

        self.step(6)
        for sens_level in range(numberOfSupportedLevels):
            logging.info(f"Write sensitivity level ({sens_level}) to CurrentSensitivityLevel)")
            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.CurrentSensitivityLevel(sens_level))])
            asserts.assert_equal(result[0].Status, Status.Success, "CurrentSensitivityLevel write failed")

        self.step(7)
        if attributes.DefaultSensitivityLevel.attribute_id in attribute_list:
            selected_non_default_level = choice([i for i in range(numberOfSupportedLevels) if i not in [default_level]])
            logging.info(f"Write non-default sensitivity level ({selected_non_default_level}) to CurrentSensitivityLevel)")
            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.CurrentSensitivityLevel(selected_non_default_level))])
            asserts.assert_equal(result[0].Status, Status.Success, "CurrentSensitivityLevel write failed")

        self.step(8)
        if attributes.DefaultSensitivityLevel.attribute_id in attribute_list:
            logging.info(f"Write default sensitivity level ({default_level}) to CurrentSensitivityLevel)")
            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.CurrentSensitivityLevel(default_level))])
            asserts.assert_equal(result[0].Status, Status.Success, "CurrentSensitivityLevel write failed")

        self.step(9)
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.CurrentSensitivityLevel(numberOfSupportedLevels))])
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "CurrentSensitivityLevel did not return CONSTRAINT_ERROR")

        self.step(10)
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.CurrentSensitivityLevel(255))])
        asserts.assert_equal(result[0].Status, Status.ConstraintError,
                             "CurrentSensitivityLevel did not return CONSTRAINT_ERROR")

        self.step(11)
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.CurrentSensitivityLevel(current_level))])
        asserts.assert_equal(result[0].Status, Status.Success, "CurrentSensitivityLevel write failed")


if __name__ == "__main__":
    default_matter_test_main()
