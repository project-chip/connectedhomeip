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

import logging
from random import choice

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_BOOLCFG_3_1(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_3_1(self) -> str:
        return "[TC-BOOLCFG-3.1] SensitivityLevel with DUT as Server"

    def steps_TC_BOOLCFG_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "{comDutTH}.", "", is_commissioning=True),
            TestStep("2a", "{THread} _Featuremap_ attribute.", "{DUTreply} the _Featuremap_ attribute."),
            TestStep("2b", "If the _{F_SENSLVL}_ {featIsNotSupported}, skip remaining steps and end test case.", ""),
            TestStep("2c", "{THread} _AttributeList_ attribute.", "{DUTreply} the _AttributeList_ attribute."),
            TestStep(3, "{THread} _{A_SUPPORTEDSENSITIVITYLEVELS}_ attribute. {storeValueAs} numberOfSupportedLevels.", "{resDutSuccess}."),
            TestStep(4, "{ifAttrIsSupported}, {THread} _{A_DEFAULTSENSITIVITYLEVEL}_ attribute. {storeValueAs} defaultLevel.", "{resDutSuccess}."),
            TestStep(5, "{THread} _{A_CURRENTSENSITIVITYLEVEL}_ attribute. {storeValueAs} currentLevel.", "{resDutSuccess}."),
            TestStep(6, "TH loops through the number of supported sensitivity levels (0 to numberOfSupportedLevels - 1) and\n                {THwrite} each value to the _{A_CURRENTSENSITIVITYLEVEL}_ attribute.", "{resDutSuccess} for all write interactions."),
            TestStep(7, "If the _{A_DEFAULTSENSITIVITYLEVEL}_ {attrIsSupported}, {THwrite} _{A_CURRENTSENSITIVITYLEVEL}_ attribute to a supported value that is NOT the value of defaultLevel.", "{resDutSuccess}."),
            TestStep(8, "If the _{A_DEFAULTSENSITIVITYLEVEL}_ {attrIsSupported}, {THwrite} _{A_CURRENTSENSITIVITYLEVEL}_ attribute to the value of defaultLevel.", "{resDutSuccess}."),
            TestStep(9, "{THwrite} _{A_CURRENTSENSITIVITYLEVEL}_ attribute to numberOfSupportedLevels.", "{resDutConstraintError}."),
            TestStep(10, "{THwrite} _{A_CURRENTSENSITIVITYLEVEL}_ attribute to 255.", "{resDutConstraintError}."),
            TestStep(11, "{THwrite} _{A_CURRENTSENSITIVITYLEVEL}_ attribute to the value of currentLevel.", "{resDutSuccess}."),
        ]
        return steps

    def pics_TC_BOOLCFG_3_1(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BOOLCFG_3_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.BooleanStateConfiguration.Attributes

        self.step("2a")
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_sens_level_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kSensitivityLevel

        self.step("2b")
        if not is_sens_level_feature_supported:
            logging.info("SENS feature not supported, skipping test case")

            # Skipping all remainig steps
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")

            return
        else:
            logging.info("Test step skipped")

        self.step("2c")
        attribute_list = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        numberOfSupportedLevels = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedSensitivityLevels)

        self.step(4)
        if attributes.DefaultSensitivityLevel.attribute_id in attribute_list:
            default_level = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultSensitivityLevel)
        else:
            logging.info("Test step skipped")

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
