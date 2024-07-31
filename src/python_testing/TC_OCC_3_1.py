#
#    Copyright (c) 2024 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${TYPE_OF_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===
#  There are CI issues to be followed up for the test cases below that implements manually controlling sensor device for 
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 4
#  [TC-OCC-3.2] test precedure step 3c

import logging
import time

from chip import ChipDeviceCtrl
from chip.interaction_model import Status
import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OCC_3_1(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OccupancySensing
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_3_1(self) -> str:
        return "[TC-OCC-3.1] Primary functionality with server as DUT"

    def steps_TC_OCC_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH and obtain DUT attribute list.", is_commissioning=True),
            TestStep(2, "Change DUT HoldTime attribute value to 10 seconds."),
            TestStep(3, "Do not trigger DUT occupancy sensing for the period of HoldTime. TH reads Occupancy attribute from DUT."),
            TestStep(4, "Trigger DUT occupancy sensing to change the occupancy state and start a timer."),
            TestStep(5, "After 10 seconds, TH reads Occupancy attribute from DUT.")
        ]
        return steps

    def pics_TC_OCC_3_1(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_3_1(self):

        endpoint = self.user_params.get("endpoint", 1)
        node_id = self.matter_test_config.dut_node_ids[0]
        hold_time = 10 # 10 seconds for occupancy state hold time

        self.step(1) # commissioning and getting cluster attribute list
        attributes = Clusters.OccupancySensing.Attributes
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        
        self.step(2)
        if attributes.HoldTime.attribute_id in attribute_list:
            # write 10 as a HoldTime attibute
            write_res = await ChipDeviceCtrl.WriteAttribute(node_id, [(endpoint, attributes.HoldTime(hold_time))])
            asserts.assert_equal(write_res[0].status, Status.Success, "Write HoldTime failed")

        else:
            logging.info("No HoldTime attribute supports. Will test only occupancy attribute triggering functionality")
            
        self.step(3)
        # check if Occupancy attribute is 0
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)

        # if occupancy is on, then wait until the sensor occupancy state is 0.
        if occupancy_dut == 1:
            # Don't trigger occupancy sensor to render occupancy attribute to 0
            if attributes.HoldTime.attribute_id in attribute_list:
                time.sleep(hold_time + 2) # add some extra 2 seconds to ensure hold time has passed.
            else: # a user wait until a sensor specific time to change occupancy attribute to 0.  This is the case where the sensor doesn't support HoldTime.
                self.wait_for_user_input(
                    prompt_msg="Type any letter and press ENTER after the sensor occupancy is detection ready state (occupancy attribute = 0)")

        # check sensor occupancy state is 0 for the next test step
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
        asserts.assert_equal(occupancy_dut, 0, "Occupancy attribute is still 1.")

        self.step(4)
        # Trigger occupancy sensor to change Occupancy attribute value to 1 => TEST ACTION on DUT
        self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after a sensor occupancy is triggered.")

        # And then check if Occupancy attribute has changed.
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
        asserts.assert_equal(occupancy_dut, 1, "Occupancy state is not changed to 1")
        
        self.step(5)
        # check if Occupancy attribute is back to 0 after HoldTime attribute period
        if attributes.HoldTime.attribute_id in attribute_list:
            # message to the tester
            self.wait_for_user_input(prompt_msg="Do not trigger sensor during HoldTime.")
            
            # Start a timer based on HoldTime
            time.sleep(hold_time+2) # add some extra 2 seconds to ensure hold time has passed.
        
            occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
            asserts.assert_equal(occupancy_dut, 0, "Occupancy state is not 0 after HoldTime period")
            
        else:
            logging.info("HoldTime attribute not supported. Skip this test procedure.")
            self.skip_step(5)


if __name__ == "__main__":
    default_matter_test_main()
