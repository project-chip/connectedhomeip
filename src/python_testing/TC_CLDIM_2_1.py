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
#     app: ${CLOSURE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 2
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_asserts import assert_valid_map8, assert_valid_uint16
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.tlv import uint
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
            TestStep(3, "Read CurrentState attribute, if supported"),
            TestStep(4, "Read TargetState attribute, if supported"),
            TestStep(5, "Read Resolution attribute, if supported"),
            TestStep(6, "Read StepValue attribute, if supported"),
            TestStep(7, "Read Unit attribute, if supported"),
            TestStep(8, "Read UnitRange attribute, if supported"),
            TestStep(9, "Read LimitRange attribute, if supported"),
            TestStep(10, "Read TranslationDirection attribute, if supported"),
            TestStep(11, "Read RotationAxis attribute, if supported"),
            TestStep(12, "Read Overflow attribute, if supported"),
            TestStep(13, "Read ModulationType attribute, if supported"),
            TestStep(14, "Read LatchControlModes attribute, if supported"),
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

        # Default values
        resolution = 1

        # STEP 2a: Read feature map and determine supported features
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_unit_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kUnit
        is_limitation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kLimitation
        is_speed_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed
        is_translation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kTranslation
        is_rotation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kRotation
        is_modulation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kModulation

        # STEP 3: Read CurrentState attribute
        self.step(3)
        current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        if current_state is not NullValue:
            asserts.assert_is_instance(current_state, Clusters.ClosureDimension.Structs.DimensionStateStruct,
                                       "CurrentState is not of expected type")
            if is_positioning_supported:
                asserts.assert_true(current_state.position is NullValue or (0 <= current_state.position <=
                                    10000), "Position is not NullValue or not in the expected range [0:100.00]")
            if is_latching_supported:
                asserts.assert_true(isinstance(current_state.latch, bool) or current_state.latch is NullValue,
                                    "Latch is not a boolean or NullValue")
            if is_speed_supported:
                asserts.assert_less(current_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue,
                                    f"Speed is not in the expected range [0:{Clusters.Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue.value - 1}]")

        # STEP 4: Read TargetState attribute
        self.step(4)
        target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        if target_state is not NullValue:
            asserts.assert_is_instance(target_state, Clusters.ClosureDimension.Structs.DimensionStateStruct,
                                       "TargetState is not of expected type")
            if is_positioning_supported:
                asserts.assert_true(target_state.position is NullValue or (0 <= target_state.position <= 10000),
                                    "Position is not NullValue or not in the expected range [0:100.00]")
            if is_latching_supported:
                asserts.assert_true(isinstance(target_state.latch, bool) or target_state.latch is NullValue,
                                    "Latch is not a boolean or NullValue")
            if is_speed_supported:
                asserts.assert_less(target_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue,
                                    f"Speed is not in the expected range [0:{Clusters.Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue.value - 1}]")

        # STEP 5: Read Resolution attribute
        self.step(5)
        if is_positioning_supported:
            resolution = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Resolution)
            assert_valid_uint16(resolution, "Resolution is not of expected type")
            asserts.assert_true(resolution >= 1, "Resolution is less than 1")
            asserts.assert_true(resolution <= 10000, "Resolution is more than 10000")
        else:
            logging.info("Positioning feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 6: Read StepValue attribute
        self.step(6)
        if is_positioning_supported:
            step_value = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.StepValue)
            asserts.assert_is_instance(step_value, uint, "StepValue is not of expected type")
            asserts.assert_true(step_value >= 1, "StepValue is less than 1")
            asserts.assert_true(step_value <= 10000, "StepValue is more than 10000")
            asserts.assert_true(step_value % resolution == 0, "StepValue is not a multiple of Resolution")
        else:
            logging.info("Positioning feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 7: Read Unit attribute
        self.step(7)
        unit = None
        if is_unit_supported:
            unit = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Unit)
            asserts.assert_is_instance(unit, Clusters.ClosureDimension.Enums.ClosureUnitEnum, "Unit is not of expected type")
            asserts.assert_less(unit, Clusters.ClosureDimension.Enums.ClosureUnitEnum.kUnknownEnumValue,
                                f"Unit is not in the expected range [0:{Clusters.ClosureDimension.Enums.ClosureUnitEnum.kUnknownEnumValue.value - 1}]")
        else:
            logging.info("Unit feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 8: Read UnitRange attribute
        self.step(8)
        if is_unit_supported:
            unit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.UnitRange)
            if unit_range is not NullValue:
                asserts.assert_is_instance(unit_range, Clusters.ClosureDimension.Structs.UnitRangeStruct,
                                           "UnitRange is not of expected type")
                asserts.assert_less(unit, Clusters.ClosureDimension.Enums.ClosureUnitEnum.kUnknownEnumValue,
                                    "Unit is unknown - cannot check UnitRange")

                if unit == 0:
                    asserts.assert_true(0 <= unit_range.min, "UnitRange.min is not larger than or equal to zero")
                    asserts.assert_true(unit_range.min <= unit_range.max <= 32767,
                                        "UnitRange.max is not in the expected range [UnitRange.Min:32767]")

                if unit == 1:
                    asserts.assert_true(-360 <= unit_range.min <= 359, "UnitRange.min is not in the expected range [-360:359]")
                    asserts.assert_true(unit_range.min <= unit_range.max <= 360,
                                        "UnitRange.max is not in the expected range [UnitRange.Min:360]")
                    asserts.assert_true(1 <= unit_range.max - unit_range.min <= 360,
                                        "UnitRange.max - UnitRange.min is not in the expected range [1:360]")
        else:
            logging.info("Unit feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 9: Read LimitRange attribute
        self.step(9)
        if is_limitation_supported:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            asserts.assert_is_instance(limit_range, Clusters.ClosureDimension.Structs.RangePercent100thsStruct,
                                       "LimitRange is not of expected type")
            asserts.assert_true(0 <= limit_range.min <= 10000, "LimitRange.min is not in the expected range [0:10000]")
            asserts.assert_true(limit_range.min % resolution == 0, "LimitRange.min is not a multiple of Resolution")
            asserts.assert_true(limit_range.min <= limit_range.max <= 10000,
                                "LimitRange.max is not in the expected range [LimitRange.Min:10000]")
            asserts.assert_true(limit_range.max % resolution == 0, "LimitRange.max is not a multiple of Resolution")
        else:
            logging.info("Limitation feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 10: Read TranslationDirection attribute
        self.step(10)
        if is_translation_supported:
            translation_direction = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TranslationDirection)
            asserts.assert_is_instance(translation_direction, Clusters.ClosureDimension.Enums.TranslationDirectionEnum,
                                       "TranslationDirection is not of expected type")
            asserts.assert_less(translation_direction, Clusters.ClosureDimension.Enums.TranslationDirectionEnum.kUnknownEnumValue,
                                f"TranslationDirection is not in the expected range [0:{Clusters.ClosureDimension.Enums.TranslationDirectionEnum.kUnknownEnumValue.value - 1}]")
        else:
            logging.info("Translation feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 11: Read RotationAxis attribute
        self.step(11)
        if is_rotation_supported:
            rotation_axis = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.RotationAxis)
            asserts.assert_is_instance(rotation_axis, Clusters.ClosureDimension.Enums.RotationAxisEnum,
                                       "RotationAxis is not of expected type")
            asserts.assert_less(rotation_axis, Clusters.ClosureDimension.Enums.RotationAxisEnum.kUnknownEnumValue,
                                f"RotationAxis is not in the expected range [0:{Clusters.ClosureDimension.Enums.RotationAxisEnum.kUnknownEnumValue.value - 1}]")
        else:
            logging.info("Rotation feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 12: Read Overflow attribute
        self.step(12)
        if is_rotation_supported:
            overflow = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Overflow)
            asserts.assert_is_instance(overflow, Clusters.ClosureDimension.Enums.OverflowEnum, "Overflow is not of expected type")
            asserts.assert_less(overflow, Clusters.ClosureDimension.Enums.OverflowEnum.kUnknownEnumValue,
                                f"Overflow is not in the expected range [0:{Clusters.ClosureDimension.Enums.OverflowEnum.kUnknownEnumValue.value - 1}]")
        else:
            logging.info("Rotation feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 13: Read ModulationType attribute
        self.step(13)
        if is_modulation_supported:
            modulation_type = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.ModulationType)
            asserts.assert_is_instance(modulation_type, Clusters.ClosureDimension.Enums.ModulationTypeEnum,
                                       "ModulationType is not of expected type")
            asserts.assert_less(modulation_type, Clusters.ClosureDimension.Enums.ModulationTypeEnum.kUnknownEnumValue,
                                f"ModulationType is not in the expected range [0:{Clusters.ClosureDimension.Enums.ModulationTypeEnum.kUnknownEnumValue.value - 1}]")
        else:
            logging.info("Modulation feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()

        # STEP 14: Read LatchControlModes attribute
        self.step(14)
        if is_latching_supported:
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
            assert_valid_map8(latch_control_modes, "LatchControlModes")
            asserts.assert_true(0 <= latch_control_modes <= 3, "LatchControlModes is not in the expected range [0:3]")
        else:
            logging.info("Latching feature is not supported by the DUT, skipping this step.")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
