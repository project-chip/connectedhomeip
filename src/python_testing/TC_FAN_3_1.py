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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
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

import asyncio
import logging
import time
import random
import queue

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

from matter_testing_infrastructure.chip.testing.matter_testing import AttributeChangeCallbackFan, AttributeValue, ClusterAttributeChangeAccumulator
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters.Attribute import AsyncReadTransaction, AttributePath, SubscriptionTransaction, TypedAttributePath


# import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AsyncReadTransaction, AttributePath, SubscriptionTransaction, TypedAttributePath
from chip.clusters.enum import MatterIntEnum
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.matter_testing import (AttributeChangeCallback, EventChangeCallback, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)

# import pdb

logger = logging.getLogger(__name__)

class TC_FAN_3_1(MatterBaseTest):
    # def steps_TC_FAN_3_1(self):
    #     return [TestStep("1", "Commissioning already done."),
    #             TestStep("2", "Action", "Verification"),
    #             ]
    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BasicInformation
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_fan_mode(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.FanMode)

    async def read_percent_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.PercentCurrent)

    async def write_fan_mode(self, endpoint, fan_mode) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.FanMode(fan_mode))])
        return result[0].Status

    async def write_percent_setting(self, endpoint, percent_setting) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.PercentSetting(percent_setting))])
        return result[0].Status

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]


    @async_test_body
    async def test_TC_FAN_3_1(self):
        # pdb.set_trace()

        # endpoint = self.get_endpoint(default=1)
        # cluster = Clusters.FanControl
        # fan_mode_off = cluster.Enums.FanModeEnum.kOff
        # fan_mode_low = cluster.Enums.FanModeEnum.kLow
        # fan_mode_medium = cluster.Enums.FanModeEnum.kMedium
        # fan_mode_high = cluster.Enums.FanModeEnum.kHigh
        # fan_mode_attribute = cluster.Attributes.FanMode
        # attribute_subscription = ClusterAttributeChangeAccumulator(cluster)















        # Sub info
        TH: ChipDeviceController = self.default_controller
        min_interval_floor_sec: int = 0
        max_interval_ceiling_sec: int = 15

        # Define attributes
        # endpoint = 1
        # attr_to_sub = Clusters.FanControl.Attributes.FanMode
        # attr_to_sub_path = [(endpoint, attr_to_sub)]
        # attr_to_write = Clusters.FanControl.Attributes.PercentSetting

        # Switcharoo to Node Label
        endpoint = 0
        attr_to_sub = Clusters.BasicInformation.Attributes.NodeLabel
        attr_to_sub_path = [(endpoint, attr_to_sub)]
        attr_to_write = attr_to_sub

        # Subscribe to attribute
        sub = await TH.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=attr_to_sub_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=True
        )

        # Set attribute update callback
        update_cb = AttributeChangeCallbackFan(attr_to_sub)
        sub.SetAttributeUpdateCallback(update_cb)
        
        # Updates loop
        for value_to_write in range(1, 11):

            # Write to attribute
            result = await TH.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attr_to_write(value=value_to_write * 10))]
            )
            print(f"\t\t [FANS] write_status: {result[0].Status.name}\n\n\n\n")

            # Wait for update callback and attribute value
            update_callback_start_time = time.time()
            attr_value = update_cb.wait_for_report()
            update_callback_end_time = time.time()
            update_callback_total_time = update_callback_end_time - update_callback_start_time
            print(f"\t\t [FANS] ** update_callback_total_time: {update_callback_total_time}, value written: {value_to_write}, attribute value: {attr_value} **")

            

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        # # Initializa to FanMode Off
        # write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_off)
        # write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        # asserts.assert_true(write_status_ok, "PercentSetting write did not return a value of Success or InvalidInState")

        # # Set initial conditions
        # last_fan_mode = fan_mode_off
        # last_percent_current = 0
        
        # for percent_to_write in range(1, 101):
        #     # Write PercentSetting to DUT
        #     write_status = await self.write_percent_setting(endpoint=endpoint, percent_setting=percent_to_write)
        #     write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        #     asserts.assert_true(write_status_ok, "PercentSetting write did not return a value of Success or InvalidInState")

        #     # Wait
        #     # await attribute_subscription.start(self.default_controller, self.dut_node_id, endpoint)
        #     # expected_fan_mode = AttributeValue(endpoint_id=endpoint, attribute=fan_mode_attribute, value=fan_mode_low)
        #     # attribute_subscription.await_all_final_values_reported(expected_final_values=[expected_fan_mode], timeout_sec=1)

        #     current_fan_mode = await self.read_fan_mode(endpoint=endpoint)
        #     current_percent_current = await self.read_percent_current(endpoint=endpoint)
            
        #     if current_fan_mode != last_fan_mode:
        #         print(f"\n\n\n\n\t\t\t\t\t [FANS] fan mode has changed to: {current_fan_mode}")
        #         print(f"\t\t\t\t\t [FANS] read_percent_current: {current_percent_current} \n\n\n\n\t\t\t\t\t")
        #         asserts.assert_greater(current_percent_current, last_percent_current)
        #         last_fan_mode = current_fan_mode
        #         last_percent_current = current_percent_current
        #     else:
        #         print(f"\n\n\n\n\t\t\t\t\t [FANS] fan mode is the same: {current_fan_mode}")



if __name__ == "__main__":
    default_matter_test_main()
