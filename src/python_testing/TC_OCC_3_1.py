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
import time

from chip import ChipDeviceCtrl
from chip.interaction_model import Status
import chip.clusters as Clusters
from chip.clusters.Types import NullValue
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
            TestStep(4, "Trigger DUT occupancy sensing to change the occupancy state and start a timer.")
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
        
        hold_time = 10 # 10 seconds

        self.step(1) # commissioning and getting cluster attribute list
        attributes = Clusters.OccupancySensing.Attributes
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        
        self.step(2) 
        if attributes.HoldTime.attribute_id in attribute_list:
            # write 10 as a HoldTime attibute
            write_res = await ChipDeviceCtrl.WriteAttribute(node_id, [(endpoint, attributes.HoldTime(hold_time))])
            asserts.assert_equal(write_res[0].status, Status.Success, "Write HoldTime failed")

        else:
            logging.info("No HoldTime attribute supports. Just test occupancy attribute triggering")
            
        self.step(3)
        # check if Occupancy attribute is 0
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
        
        if occupancy_dut is 1:
            # Don't trigger occupancy sensor to render occupancy attribute to 0
            if attributes.HoldTime.attribute_id in attribute_list:
                time.sleep(hold_time) # wait 10 seconds
                
            else: # a user wait until a sensor specific time to change occupancy attribute to 0
                print('Type any letter and press ENTER after the sensor occupancy is detection ready state (occupancy attribute = 0)')
                user_wait = input()
        
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)        
        asserts.assert_equal(occupancy_dut, 0, "Occupancy attribute is 1.")

        self.step(4)
        # Trigger occupancy sensor to change Occupancy attribute value to 1 => TEST ACTION on DUT
        print('Type any letter and press ENTER after a user triggers sensor occupancy.')
        user_wait = input()
                
        # And then check if Occupancy attribute has changed.
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
        asserts.assert_equal(occupancy_dut, 1, "Occupancy state is not 1")
            
        # step 5
        # check if Occupancy attribute is back to 0 after HoldTime attribute period
        if attributes.HoldTime.attribute_id in attribute_list:
            self.step(5)

            # Trigger occupancy sensor to change Occupancy attribute value to 1 => TEST ACTION on DUT
            print('Type any letter and press ENTER after a user triggers sensor occupancy.')
            user_wait = input()
        
            # Start a timer based on HoldTime
            time.sleep(hold_time) # wait 10 seconds
        
            occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
            asserts.assert_equal(occupancy_dut, 0, "Occupancy state is not 0 after HoldTime period")
            
        else:
            
            logging.info("No HoldTime attribute supports. Skip this test procedure.")
            self.skip_step(5)