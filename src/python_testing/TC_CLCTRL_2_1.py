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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import typing
import chip.clusters as Clusters

from chip.tlv import uint
from chip.clusters.Types import Nullable, NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_CLCTRL_2_1(MatterBaseTest):
    async def read_closurecontrol_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_2_1(self) -> str:
        return "[TC-CLCTRL-2.1] Attributes with Server as DUT"

    def steps_TC_CLCTRL_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the AttributeList attribute to determine supported attributes", "AttributeList of the ClosureControl cluster is returned by the DUT"),
            TestStep(3, "Read the FeatureMap attribute to determine supported features", "FeatureMap of the ClosureControl cluster is returned by the DUT"),
            TestStep(4, "Read the CountdownTime attribute", "CountdownTime is returned by the DUT if it is supported, otherwise skip"),
            TestStep(5, "Read the MainState attribute", "MainState of the ClosureControl cluster is returned by the DUT"),
            TestStep(6, "Read the CurrentErrorList attribute", "CurrentErrorList of the ClosureControl cluster is returned by the DUT"),
            TestStep(7, "Read the OverallState attribute", "OverallState of the ClosureControl cluster is returned by the DUT and the fields of the struct are validated"),
            TestStep(8, "Read the OverallTarget attribute", "OverallTarget of the ClosureControl cluster is returned by the DUT and the fields of the struct are validated"),
            TestStep(9, "Read the LatchControlModes attribute", "LatchControlModes of the ClosureControl cluster is returned by the DUT if the LT feature is supported, otherwise skip"),
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
        
        logging.info(f"FeatureMap: {feature_map}")
        logging.info(f"-> Positioning supported: \t\t {bool(is_positioning_supported)}")
        logging.info(f"-> MotionLatching supported:\t {bool(is_latching_supported)}")
        logging.info(f"-> Speed supported: \t\t {bool(is_speed_supported)}")

        # STEP 4: Read CountdownTime attribute if supported
        self.step(4)
        if attributes.CountdownTime.attribute_id in attribute_list:
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
        asserts.assert_less_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kSetupRequired,
                                  "MainState attribute is out of range")
        asserts.assert_greater_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kStopped,
                                     "MainState attribute is out of range")

        # STEP 6: Read CurrentErrorList attribute
        self.step(6)
        current_error_list: typing.List[Clusters.ClosureControl.Enums.ClosureErrorEnum] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentErrorList)
        logging.info(f"CurrentErrorList: {current_error_list}")
        asserts.assert_less_equal(len(current_error_list), 10, "CurrentErrorList length is out of range")
        for error in current_error_list:
            # Check if error is a valid value, if the list is not empty
            asserts.assert_less_equal(error, 0xBF, "CurrentErrorList value is out of range")
            asserts.assert_greater_equal(error, 0x00, "CurrentErrorList value is out of range")

        # STEP 7: Read OverallState attribute
        self.step(7)
        overall_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallStateStruct] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        
        if overall_state is NullValue:
            logging.info("OverallState is NULL, skipping field validations")
        else:
            logging.info(f"OverallState: {overall_state}")
            
            # Check Positioning feature in OverallState - PS feature (bit 0)
            if is_positioning_supported and overall_state.positioning is not NullValue:
                logging.info(f"OverallState.positioning: {overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum(overall_state.positioning).name}")
                asserts.assert_less_equal(
                    overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kOpenedAtSignature, "OverallState.positioning is out of range")
                asserts.assert_greater_equal(
                    overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kFullyClosed, "OverallState.positioning is out of range")

            # Check MotionLatching feature in OverallState - LT feature (bit 1)
            if is_latching_supported and overall_state.latch is not NullValue:
                logging.info(f"OverallState.latch: {overall_state.latch}")
                asserts.assert_true(isinstance(overall_state.latch, bool), "OverallState.latch is not a boolean value")
            
            # Check Speed feature in OverallState - SP feature (bit 3)
            if is_speed_supported:
                logging.info(f"OverallState.speed: {overall_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum(overall_state.speed).name}")
                asserts.assert_less_equal(overall_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kHigh,
                                          "OverallState.speed is out of range")
                asserts.assert_greater_equal(overall_state.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kAuto,
                                             "OverallState.speed is out of range")

            # Check SecureState attribute in OverallState
            if is_positioning_supported or is_latching_supported:
                logging.info(f"OverallState.secureState: {overall_state.secureState}")
                asserts.assert_true(isinstance(overall_state.secureState, bool), "OverallState.secureState is not a boolean value")
            
        # STEP 8: Read OverallTarget attribute
        self.step(8)
        overall_target: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallTargetStruct] = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallTarget)

        if overall_target is NullValue:
            logging.info("OverallTarget is NULL, skipping field validations")
        else:
            logging.info(f"OverallTarget: {overall_target}")
            
            # Check Positioning feature in OverallTarget
            if is_positioning_supported and overall_target.position is not NullValue:  # PS feature (bit 0)
                logging.info(f"OverallTarget.position: {overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum(overall_target.position).name}")
                asserts.assert_less_equal(
                    overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kSignature, "OverallTarget.position is out of range")
                asserts.assert_greater_equal(
                    overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kCloseInFull, "OverallTarget.position is out of range")
                
            # Check MotionLatching feature in OverallTarget
            if is_latching_supported and overall_target.latch is not NullValue:  # LT feature (bit 1)
                logging.info(f"OverallTarget.latch: {overall_target.latch}")
                asserts.assert_true(isinstance(overall_target.latch, bool), "OverallTarget.latch is not a boolean value")

            # Check Speed feature in OverallTarget
            if is_speed_supported:  # SP feature (bit 3)
                logging.info(f"OverallTarget.speed: {overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum(overall_target.speed).name}")
                asserts.assert_less_equal(overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kHigh,
                                          "OverallTarget.speed is out of range")
                asserts.assert_greater_equal(overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kAuto,
                                             "OverallTarget.speed is out of range")            
            
        # STEP 9: Read LatchControlModes attribute if LT is supported
        self.step(9)
        if not is_latching_supported:
            logging.info("LatchControlModes attribute not supported")
        else:
            latch_control_modes: uint = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
            logging.info(f"LatchControlModes: {latch_control_modes}")
            
            asserts.assert_less_equal(latch_control_modes, 3, "LatchControlModes attribute is out of range")
            asserts.assert_greater_equal(latch_control_modes, 0, "LatchControlModes attribute is out of range")    

if __name__ == "__main__":
    default_matter_test_main()
