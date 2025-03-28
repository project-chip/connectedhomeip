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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from random import choice

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

class TC_CLDIM_2_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_2_1(self) -> str:
        return "[TC_CLDIM_2_1] Attributes with DUT as Server"

    def steps_TC_CLDIM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "Read feature map determine supported features"),
            TestStep("2b", "Read attribute list to determine supported attributes"),
            TestStep(3, "Read CurrentState attribute, if supported"),
            TestStep(4, "Read Target attribute, if supported"),
            TestStep(5, "Read Resolution attribute, if supported"),
            TestStep(6, "Read StepValue attribute, if supported"),
            TestStep(7, "Read Unit attribute, if supported"),
            TestStep(8, "Read UnitRange attribute, if supported"),
            TestStep(9, "Read LimitRange attribute, if supported"),
            TestStep(10, "Read TranslationDirection attribute, if supported"),
            TestStep(11, "Read RotationAxis attribute, if supported"),
            TestStep(12, "Read Overflow attribute, if supported"),
            TestStep(13, "Read ModulationType attribute, if supported"),
        ]
        return steps

    def pics_TC_CLDIM_2_1(self) -> list[str]:
        pics = [
            "CLDIM.S"
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_2_1(self):
        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes

        # STEP 2a: Read feature map and determine supported features
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_speed_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed
        
        # STEP 2b: Read attribute list to determine supported attributes
        self.step("2b")
        attribute_list = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
    
        # STEP 3: Read CurrentState attribute
        self.step(3)
        if attributes.CurrentState.attribute_id in attribute_list:
            kCurrentState = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            if kCurrentState is not NullValue:
                if is_positioning_supported:
                    asserts.assert_true(0 <= kCurrentState.position <= 10000, "Position is not in the expected range")
                if is_latching_supported:
                    asserts.assert_is_instance(kCurrentState.Latch, bool, "Latch is not a boolean")
                if is_speed_supported:
                    asserts.assert_true(0 <= kCurrentState.speed <= 3, "Speed is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 4: Read Target attribute
        self.step(4)
        if attributes.Target.attribute_id in attribute_list:
            kTarget = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            if kTarget is not NullValue:
                if is_positioning_supported:
                    asserts.assert_true(0 <= kTarget.position <= 10000, "Position is not in the expected range")
                if is_latching_supported:
                    asserts.assert_is_instance(kCurrentState.Latch, bool, "Latch is not a boolean")
                if is_speed_supported:
                    asserts.assert_true(0 <= kTarget.speed <= 3, "Speed is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 5: Read Resolution attribute
        self.step(5)
        if attributes.Resolution.attribute_id in attribute_list:
            kResolution = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Resolution)
            asserts.assert_true(1 <= kResolution <= 10000, "Resolution is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 6: Read StepValue attribute
        self.step(6)
        if attributes.StepValue.attribute_id in attribute_list:
            kStepValue = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.StepValue)
            asserts.assert_true(0 <= kStepValue <= 10000, "StepValue is not in the expected range")
            asserts.assert_true(kStepValue % kResolution == 0, "StepValue is not a multiple of Resolution")
        else:
            logging.info("Test step skipped")

        # STEP 7: Read Unit attribute
        self.step(7)
        if attributes.Unit.attribute_id in attribute_list:
            kUnit = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Unit)
            asserts.assert_true(0 <= kUnit <= 1, "Unit is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 8: Read UnitRange attribute
        self.step(8)
        if attributes.UnitRange.attribute_id in attribute_list:
            kUnitRange = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.UnitRange)
            if kUnitRange is not NullValue:
                asserts.assert_true(-32768 <= kUnitRange.Min <= kUnitRange.Max -1, "UnitRange.Min is not in the expected range")
                asserts.assert_true(kUnitRange.Min +1  <= kUnitRange.Max <= 32767, "UnitRange.Max is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 9: Read LimitRange attribute
        self.step(9)
        if attributes.LimitRange.attribute_id in attribute_list:
            kLimitRange = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            asserts.assert_true(0 <= kLimitRange.Min <= 10000, "LimitRange.Min is not in the expected range")
            asserts.assert_true(kLimitRange.Min % kResolution == 0, "LimitRange.Min is not a multiple of Resolution")
            asserts.assert_true(kLimitRange.Min <= kLimitRange.Max <= 10000, "LimitRange.Max is not in the expected range")
            asserts.assert_true(kLimitRange.Max % kResolution == 0, "LimitRange.Max is not a multiple of Resolution")
        else:
            logging.info("Test step skipped")

        # STEP 10: Read TranslationDirection attribute
        self.step(10)
        if attributes.TranslationDirection.attribute_id in attribute_list:
            kTranslationDirection = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TranslationDirection)
            asserts.assert_true(0 <= kTranslationDirection <= 15, "TranslationDirection is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 11: Read RotationAxis attribute
        self.step(11)
        if attributes.RotationAxis.attribute_id in attribute_list:
            kRotationAxis = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.RotationAxis)
            asserts.assert_true(0 <= kRotationAxis <= 10, "RotationAxis is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 12: Read Overflow attribute
        self.step(12)
        if attributes.Overflow.attribute_id in attribute_list:
            kOverflow = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Overflow)
            asserts.assert_true(0 <= kOverflow <= 10, "Overflow is not in the expected range")
        else:
            logging.info("Test step skipped")

        # STEP 13: Read ModulationType attribute
        self.step(13)
        if attributes.ModulationType.attribute_id in attribute_list:
            kModulationType = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.ModulationType)
            asserts.assert_true(0 <= kModulationType <= 4, "ModulationType is not in the expected range")
        else:
            logging.info("Test step skipped")

if __name__ == "__main__":
    default_matter_test_main()
