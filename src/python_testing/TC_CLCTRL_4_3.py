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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_CLCTRL_4_3(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_3(self) -> str:
        return "[TC-CLCTRL-4.3] MoveTo Command Field Sanity Check with Server as DUT"

    def steps_TC_CLCTRL_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Send MoveTo command with no fields"),
            TestStep("3a", "Send MoveTo command with fields where corresponding feature is not supported - !MotionLatching"),
            TestStep("3b", "Send MoveTo command with fields where corresponding feature is not supported - !Speed"),
            TestStep("3c", "Send MoveTo command with fields where corresponding feature is not supported - !Positioning"),
            TestStep("4a", "Send MoveTo command with invalid data in fields - invalid Position"),
            TestStep("4b", "Send MoveTo command with invalid data in fields - invalid Speed"),
            TestStep("4c", "Send MoveTo command with invalid data in fields - invalid Position and valid Speed"),
            TestStep("4d", "Send MoveTo command with invalid data in fields - valid Position and invalid Speed"),
        ]
        return steps

    def pics_TC_CLCTRL_4_3(self) -> list[str]:
        pics = [
            "CLCTRL.S",
            "CLCTRL.C.C01.Rsp(MoveTo)",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_3(self):
        
        endpoint = self.get_endpoint(default=1)
        
        # STEP 1: Commission DUT to TH
        self.step(1)
        cluster = Clusters.Objects.ClosureControl
        
        # Read the feature map
        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")

        is_ps_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_lt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_sp_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        
        # STEP 2: Send MoveTo command with no fields
        self.step(2)
        try:
            response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({}))
            logging.error("MoveTo command with no fields should have failed but succeeded")
            asserts.assert_true(False, "MoveTo command with no fields should have failed but succeeded")
        except Exception as e:
            logging.info(f"Expected exception caught: {e}")
            # Verify that we got an Invalid Command error
            asserts.assert_true("INVALID_COMMAND" in str(e), 
                                f"Expected INVALID_COMMAND error but got: {e}")
            
        # STEP 3
        self.step("3a")        
        # Test Latch field if Latching feature is not supported
        if not is_lt_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"latch": True}))
                logging.info("MoveTo command with Latch field when feature not supported was accepted")
            except Exception as e:
                logging.error(f"MoveTo with Latch when feature not supported failed: {e}")
                asserts.assert_false(True, "MoveTo with Latch when feature not supported should be accepted")
        
        self.step("3b")
        # Test Speed field if Speed feature is not supported
        if not is_sp_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"speed": 1}))  # Low speed
                logging.info("MoveTo command with Speed field when feature not supported was accepted")
            except Exception as e:
                logging.error(f"MoveTo with Speed when feature not supported failed: {e}")
                asserts.assert_false(True, "MoveTo with Speed when feature not supported should be accepted")
        
        self.step("3c")
        # Test Position field if Positioning feature is not supported
        if not is_ps_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 0}))  # CloseInFull
                logging.info("MoveTo command with Position field when feature not supported was accepted")
            except Exception as e:
                logging.error(f"MoveTo with Position when feature not supported failed: {e}")
                asserts.assert_false(True, "MoveTo with Position when feature not supported should be accepted")

        # STEP 4
        self.step("4a")
        # Test invalid Position value
        if is_ps_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 6}))
                logging.error("MoveTo command with invalid Position should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with invalid Position should have failed but succeeded")
            except Exception as e:
                logging.info(f"Expected exception caught for invalid Position: {e}")
                # Verify that we got a Constraint Error
                asserts.assert_true("CONSTRAINT_ERROR" in str(e), 
                                    f"Expected CONSTRAINT_ERROR for invalid Position but got: {e}")
        
        self.step("4b")
        # Test invalid Speed value
        if is_ps_feature_supported and is_sp_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"speed": 4}))
                logging.error("MoveTo command with invalid Speed should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with invalid Speed should have failed but succeeded")
            except Exception as e:
                logging.info(f"Expected exception caught for invalid Speed: {e}")
                # Verify that we got a Constraint Error
                asserts.assert_true("CONSTRAINT_ERROR" in str(e), 
                                    f"Expected CONSTRAINT_ERROR for invalid Speed but got: {e}")
        
        self.step("4c")
        # Test invalid Position and valid Speed
        if is_ps_feature_supported and is_sp_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 6, "speed": 3}))
                logging.error("MoveTo command with invalid Position and valid Speed should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with invalid Position and valid Speed should have failed but succeeded")
            except Exception as e:
                logging.info(f"Expected exception caught for invalid Position and valid Speed: {e}")
                # Verify that we got a Constraint Error
                asserts.assert_true("CONSTRAINT_ERROR" in str(e), 
                                    f"Expected CONSTRAINT_ERROR for invalid Position but got: {e}")
        
        self.step("4d")
        # Test valid Position and invalid Speed
        if is_ps_feature_supported and is_sp_feature_supported:
            try:
                response = await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 0, "speed": 4}))
                logging.error("MoveTo command with valid Position and invalid Speed should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with valid Position and invalid Speed should have failed but succeeded")
            except Exception as e:
                logging.info(f"Expected exception caught for valid Position and invalid Speed: {e}")
                # Verify that we got a Constraint Error
                asserts.assert_true("CONSTRAINT_ERROR" in str(e), 
                                    f"Expected CONSTRAINT_ERROR for invalid Speed but got: {e}")

if __name__ == "__main__":
    default_matter_test_main()