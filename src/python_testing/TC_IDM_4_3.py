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

import copy
import logging
import queue
import time

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AttributePath, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from matter_testing_support import (MatterBaseTest, async_test_body, default_matter_test_main)
# from matter_testing_support import (AttributeChangeCallback, MatterBaseTest, async_test_body, default_matter_test_main,
#                                     wait_for_attribute_report)
from mobly import asserts

'''
Category:
Functional

Description:
This test case will verify the report data messages sent from the DUT after activating
subscription are according to specification.

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#443-tc-idm-43-report-data-messages-post-subscription-activation-from-dut-test-cases-dut_server
'''


class TC_IDM_4_3(MatterBaseTest):

    @async_test_body
    async def test_TC_IDM_4_3(self):

        # Test setup
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        node_label_attr_path = [(0, node_label_attr)]
        TH: ChipDeviceController = self.default_controller

        # *** Step 1a ***
        self.print_step("1a", "DUT and TH activate the subscription.")

        # Subscribe to attribute
        sub_th_step1a = await TH.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(3, 10),
            keepSubscriptions=False
        )
        
        # Verify that the subscription is activated between TH and DUT
        # Verify on the TH, a report data message is received.
        asserts.assert_true(sub_th_step1a.subscriptionId, "Subscription not activated")

        # Verify subscriptionId field is present
        asserts.assert_is_not_none(sub_th_step1a.subscriptionId, "SubscriptionId field not present")

        # Verify MaxInterval field is present
        sub_th_step1a_intervals = sub_th_step1a.GetReportingIntervalsSeconds()
        sub_th_step1a_min_interval_sec, sub_th_step1a_max_interval_sec = sub_th_step1a_intervals
        asserts.assert_is_not_none(sub_th_step1a_max_interval_sec, "MaxInterval field not present")

        sub_th_step1a.Shutdown()

        # *** Step 1b ***
        self.print_step("1b", "Change the value of the attribute which has been subscribed on the DUT by manually changing some settings on the device.")

        # Modify attribute value
        new_node_label_write = "NewNodeLabel_11001100"
        await TH.WriteAttribute(
            self.dut_node_id,
            [(0, node_label_attr(value=new_node_label_write))]
        )
        
        
        


if __name__ == "__main__":
    default_matter_test_main()
