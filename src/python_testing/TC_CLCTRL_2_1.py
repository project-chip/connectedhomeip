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
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import typing

import chip.clusters as Clusters
from chip.clusters.Types import Nullable, NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.tlv import uint
from mobly import asserts


class TC_CLCTRL_2_1(MatterBaseTest):
    async def read_closurecontrol_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_2_1(self) -> str:
        return "[TC-CLCTRL-2.1] Attributes with Server as DUT"

    def steps_TC_CLCTRL_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the AttributeList attribute to determine supported attributes",
                     "AttributeList of the ClosureControl cluster is returned by the DUT"),
            TestStep(3, "Read the FeatureMap attribute to determine supported features",
                     "FeatureMap of the ClosureControl cluster is returned by the DUT"),
            TestStep(4, "Read the CountdownTime attribute", "CountdownTime is returned by the DUT if it is supported, otherwise skip"),
            TestStep(5, "Read the MainState attribute", "MainState of the ClosureControl cluster is returned by the DUT"),
            TestStep(6, "Read the CurrentErrorList attribute", "CurrentErrorList of the ClosureControl cluster is returned by the DUT"),
            TestStep(7, "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT and the fields of the struct are validated"),
            TestStep(8, "Read the OverallTargetState attribute",
                     "OverallTargetState of the ClosureControl cluster is returned by the DUT and the fields of the struct are validated"),
            TestStep(9, "Read the LatchControlModes attribute",
                     "LatchControlModes of the ClosureControl cluster is returned by the DUT if the LT feature is supported"),
        ]
        return steps

    def pics_TC_CLCTRL_2_1(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_2_1(self):

        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # STEP 2: Read AttributeList attribute to determine supported attributes
        self.step(2)
        attribute_list: typing.List[uint] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        logging.info(f"AttributeList: {attribute_list}")

        # STEP 3: Read FeatureMap attribute to determine supported features
        self.step(3)
        feature_map: uint = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_speed_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        is_instantaneous: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kInstantaneous

        logging.info(f"FeatureMap: {feature_map}")
        logging.info(f"-> Positioning supported: \t\t {bool(is_positioning_supported)}")
        logging.info(f"-> MotionLatching supported:\t {bool(is_latching_supported)}")
        logging.info(f"-> Speed supported: \t\t {bool(is_speed_supported)}")

        # STEP 4: Read CountdownTime attribute if supported
        self.step(4)
        if attributes.CountdownTime.attribute_id in attribute_list:
            asserts.assert_true(
                is_positioning_supported and not is_instantaneous, "CountdownTime attribute should not be present if Positioning is not supported or Instantaneous is supported")

            countdown_time: typing.Union[None, Nullable, uint] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            logging.info(f"CountdownTime: {countdown_time}")

            if countdown_time is not NullValue:
                asserts.assert_less_equal(countdown_time, 259200, "CountdownTime attribute is out of range")
                asserts.assert_greater_equal(countdown_time, 0, "CountdownTime attribute is out of range")
        else:
            logging.info("CountdownTime attribute not supported")

        # STEP 5: Read MainState attribute
        self.step(5)
        main_state: Clusters.ClosureControl.Enums.MainStateEnum = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        logging.info(f"MainState: {main_state, Clusters.ClosureControl.Enums.MainStateEnum(main_state).name}")
        asserts.assert_less(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kUnknownEnumValue,
                            "MainState attribute is out of range")
        asserts.assert_greater_equal(main_state, 0, "MainState attribute is out of range")

        # STEP 6: Read CurrentErrorList attribute
        self.step(6)
        current_error_list: typing.List[Clusters.ClosureControl.Enums.ClosureErrorEnum] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentErrorList)
        logging.info(f"CurrentErrorList: {current_error_list}")

        asserts.assert_less_equal(len(current_error_list), 10, "CurrentErrorList length is out of range")
        asserts.assert_true(len(current_error_list) == len(set(current_error_list)), "CurrentErrorList contains duplicate values")

        for error in current_error_list:
            # Check if error is a valid value, if the list is not empty
            asserts.assert_less_equal(error, 0xBF, "CurrentErrorList value is out of range")
            asserts.assert_greater_equal(error, 0x00, "CurrentErrorList value is out of range")

        # STEP 7: Read OverallCurrentState attribute
        self.step(7)
        overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)

        if overall_current_state is NullValue:
            logging.info("OverallCurrentState is NULL, skipping field validations")
        else:
            logging.info(f"OverallCurrentState: {overall_current_state}")

            # Check Positioning feature in OverallCurrentState - PS feature (bit 0)
            if is_positioning_supported and overall_current_state.position is not NullValue:
                logging.info(
                    f"OverallCurrentState.position: {overall_current_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum(overall_current_state.position).name}")
                asserts.assert_true(isinstance(overall_current_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum),
                                    "OverallCurrentState.position is not a CurrentPositionEnum value")
                asserts.assert_less(
                    overall_current_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum.kUnknownEnumValue, "OverallCurrentState.position is out of range")
                asserts.assert_greater_equal(
                    overall_current_state.position, 0, "OverallCurrentState.position is out of range")

            # Check MotionLatching feature in OverallCurrentState - LT feature (bit 1)
            if is_latching_supported and overall_current_state.latch is not NullValue:
                logging.info(f"OverallCurrentState.latch: {overall_current_state.latch}")
                asserts.assert_true(isinstance(overall_current_state.latch, bool),
                                    "OverallCurrentState.latch is not a boolean value")

            # Check Speed feature in OverallCurrentState - SP feature (bit 3)
            if is_speed_supported:
                logging.info(
                    f"OverallCurrentState.speed: {overall_current_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum(overall_current_state.speed).name}")
                asserts.assert_true(isinstance(overall_current_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum),
                                    "OverallCurrentState.speed is not a ThreeLevelAutoEnum value")
                asserts.assert_less(overall_current_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue,
                                    "OverallCurrentState.speed is out of range")
                asserts.assert_greater_equal(overall_current_state.speed, 0, "OverallCurrentState.speed is out of range")

            # Check SecureState attribute in OverallCurrentState
            logging.info(f"OverallCurrentState.secureState: {overall_current_state.secureState}")
            if overall_current_state.secureState is not NullValue:
                asserts.assert_true(isinstance(overall_current_state.secureState, bool),
                                    "OverallCurrentState.secureState is not a boolean value")

        # STEP 8: Read OverallTargetState attribute
        self.step(8)
        overall_target: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallTargetStateStruct] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallTargetState)

        if overall_target is NullValue:
            logging.info("OverallTargetState is NULL, skipping field validations")
        else:
            logging.info(f"OverallTargetState: {overall_target}")

            # Check Positioning feature in OverallTargetState
            if is_positioning_supported and overall_target.position is not NullValue:  # PS feature (bit 0)
                logging.info(
                    f"OverallTargetState.position: {overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum(overall_target.position).name}")
                asserts.assert_true(isinstance(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum),
                                    "OverallTargetState.position is not a TargetPositionEnum value")
                asserts.assert_less(
                    overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kUnknownEnumValue, "OverallTargetState.position is out of range")
                asserts.assert_greater_equal(
                    overall_target.position, 0, "OverallTargetState.position is out of range")

            # Check MotionLatching feature in OverallTargetState
            if is_latching_supported and overall_target.latch is not NullValue:  # LT feature (bit 1)
                logging.info(f"OverallTargetState.latch: {overall_target.latch}")
                asserts.assert_true(isinstance(overall_target.latch, bool), "OverallTargetState.latch is not a boolean value")

            # Check Speed feature in OverallTargetState
            if is_speed_supported:  # SP feature (bit 3)
                logging.info(
                    f"OverallTargetState.speed: {overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum(overall_target.speed).name}")
                asserts.assert_true(isinstance(overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum),
                                    "OverallTargetState.speed is not a ThreeLevelAutoEnum value")
                asserts.assert_less(overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue,
                                    "OverallTargetState.speed is out of range")
                asserts.assert_greater_equal(overall_target.speed, 0,
                                             "OverallTargetState.speed is out of range")

        # STEP 9: Read LatchControlModes attribute if LT is supported
        self.step(9)
        if is_latching_supported:
            latch_control_modes: uint = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
            logging.info(f"LatchControlModes: {latch_control_modes}")

            asserts.assert_less_equal(latch_control_modes, 3, "LatchControlModes attribute is out of range")
            asserts.assert_greater_equal(latch_control_modes, 0, "LatchControlModes attribute is out of range")

        else:
            asserts.assert_true(
                attributes.LatchControlModes.attribute_id not in attribute_list,
                "LatchControlModes attribute should not be present if MotionLatching is not supported")
            logging.info("LT not supported and LatchControlModes not present")


if __name__ == "__main__":
    default_matter_test_main()
