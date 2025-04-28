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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue

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
            TestStep(2, "Read AttributeList attribute to determine supported attributes"),
            TestStep(3, "Read FeatureMap attribute to determine supported features"),
            TestStep(4, "Read CountdownTime attribute if supported"),
            TestStep(5, "Read MainState attribute"),
            TestStep(6, "Read CurrentErrorList attribute"),
            TestStep(7, "Read OverallState attribute"),
            TestStep(8, "Read OverallTarget attribute"),
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
        attribute_list = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        logging.info(f"Attribute list: {attribute_list}")
        
        # STEP 3: Read FeatureMap attribute to determine supported features
        self.step(3)
        feature_map = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)     
        
        is_latching_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_positioning_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_speed_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        
        # STEP 4: Read CountdownTime attribute if supported
        self.step(4)
        if attributes.CountdownTime.attribute_id in attribute_list:
            countdown_time = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            logging.info(f"CountdownTime: {countdown_time}")
            if countdown_time is not NullValue:
                asserts.assert_less_equal(countdown_time, 259200, "CountdownTime attribute is out of range")
                asserts.assert_greater_equal(countdown_time, 0, "CountdownTime attribute is out of range")
        else:
            logging.info("CountdownTime attribute not supported, skipping step")

        # STEP 5: Read MainState attribute
        self.step(5)
        main_state = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        asserts.assert_less_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kSetupRequired, "MainState attribute is out of range")
        asserts.assert_greater_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kStopped, "MainState attribute is out of range")
        logging.info(f"MainState: {main_state}")

        # STEP 6: Read CurrentErrorList attribute
        self.step(6)
        current_error_list = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentErrorList)
        asserts.assert_less_equal(len(current_error_list), 10, "CurrentErrorList length is out of range")
        for error in current_error_list:
            # Check if error is a valid value, if the list is not empty
            asserts.assert_less_equal(error, 0xBF, "CurrentErrorList value is out of range")
            asserts.assert_greater_equal(error, 0x00, "CurrentErrorList value is out of range")
        logging.info(f"CurrentErrorList: {current_error_list}")

        # STEP 7: Read OverallState attribute
        self.step(7)
        overall_state = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        logging.info(f"OverallState: {overall_state}")

        if overall_state is not NullValue:
            # Check Positioning feature in OverallState - PS feature (bit 0)
            if is_positioning_supported and overall_state.positioning is not NullValue:
                asserts.assert_less_equal(overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kOpenedAtSignature, "OverallState.positioning is out of range")
                asserts.assert_greater_equal(overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kFullyClosed, "OverallState.positioning is out of range")
                logging.info(f"OverallState.positioning: {overall_state.positioning}")
        
            # Check MotionLatching feature in OverallState - LT feature (bit 1)
            if is_latching_supported and overall_state.latch is not NullValue:
                asserts.assert_true(isinstance(overall_state.latch, bool), "OverallState.latch is not a boolean value")
                logging.info(f"OverallState.latch: {overall_state.latch}")
        
            # Check Speed feature in OverallState - SP feature (bit 3)
            if is_speed_supported and overall_state.speed is not NullValue:
                asserts.assert_less_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kHigh, "OverallState.speed is out of range")
                asserts.assert_greater_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kAuto, "OverallState.speed is out of range")
                logging.info(f"OverallState.speed: {overall_state.speed}")

            # Check SecureState attribute in OverallState
            if is_positioning_supported or is_latching_supported:
                asserts.assert_true(isinstance(overall_state.secureState, bool), "OverallState.SecureState is not a boolean value")
        else:
            logging.info("OverallState is NULL, skipping steps")
            
        # STEP 8: Read OverallTarget attribute
        self.step(8)
        overall_target = await self.read_closurecontrol_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallTarget)
        logging.info(f"OverallTarget: {overall_target}")
        
        if overall_target is not NullValue:
            # Check Positioning feature in OverallTarget
            if is_positioning_supported:  # PS feature (bit 0)
                asserts.assert_less_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kSignature, "OverallTarget.position is out of range")
                asserts.assert_greater_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kCloseInFull, "OverallTarget.position is out of range")
                logging.info(f"OverallTarget.position: {overall_target.position}")
        
            # Check MotionLatching feature in OverallTarget
            if is_latching_supported:  # LT feature (bit 1)
                asserts.assert_true(isinstance(overall_target.latch, bool), "OverallTarget.latch is not a boolean value")
                logging.info(f"OverallTarget.latch: {overall_target.latch}")
        
            # Check Speed feature in OverallTarget
            if is_speed_supported:  # SP feature (bit 3)
                asserts.assert_less_equal(overall_target.speed, Clusters.Globals.ThreeLevelAutoEnum.kHigh, "OverallTarget.speed is out of range")
                asserts.assert_greater_equal(overall_target.speed, Clusters.Globals.ThreeLevelAutoEnum.kAuto, "OverallTarget.speed is out of range")
                logging.info(f"OverallTarget.speed: {overall_target.speed}")
        else:
            logging.info("OverallTarget is NULL, skipping steps")
            
if __name__ == "__main__":
    default_matter_test_main()