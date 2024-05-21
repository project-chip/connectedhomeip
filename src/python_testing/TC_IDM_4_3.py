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
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
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

    def steps_TC_IDM_4_3(self):
        return [TestStep("1a", "DUT and TH activate the subscription.",
                         "Verify on the TH, a report data message is received. Verify on the TH the Subscribe Response has the following fields: SubscriptionId and MaxInterval In the following Steps 2, 3, 5-10, 13, and 15, the MaxInterval time reference in each step is the MaxInterval presented in the Subscribe Response of the subscription."),
                TestStep("1b", "Change the value of the attribute which has been subscribed on the DUT by manually changing some settings on the device. Example: Temperature sensor may update the value of the room temperature. Turning on/off on a light bulb.",
                         "Verify that there is a report data message sent from the DUT for the changed value of the attribute. Verify that the Report Data is sent when the minimum interval time is reached and before the MaxInterval time."),
                # TestStep(2, "DUT and TH activate the subscription. Change the value of the attribute which has been subscribed on the DUT by sending an IMWrite or Invoke message to the DUT from the TH.",
                #          "Verify that there is a report data message sent from the DUT for the changed value of the attribute. Verify that the Report Data is sent when the minimum interval time is reached and before the MaxInterval time."),
                # TestStep(3, "DUT and TH activate the subscription for an attribute. Do not change the value of the attribute which has been subscribed.",
                #          "Verify that there is an empty report data message sent from the DUT to the TH after MaxInterval time."),
                # TestStep(4, "DUT and TH activate the subscription. Change the value of the attribute which has been subscribed on the DUT. TH force sends a status response with an \"invalid subscription\". Change the value of the attribute which has been subscribed on the DUT.",
                #          "Verify that DUT does not send report data for the second time after the subscription has been terminated."),
                # TestStep(5, "Activate the subscription between the DUT and the TH for an attribute of data type bool. Modify that attribute on the DUT. DUT should send the report data with the modified attribute value. Modify the attribute multiple times (3 times) before the MaxInterval time specified during the subscription activation.",
                #          "Verify on the TH that the DUT sends the correct value of the attribute."),
                # TestStep(6, "Activate the subscription between the DUT and the TH for an attribute of data type string. Modify that attribute on the DUT. DUT should send the report data with the modified attribute value Modify the attribute multiple times (3 times) before the MaxInterval time specified during the subscription activation.",
                #          "Verify on the TH that the DUT sends the correct value of the attribute."),
                # TestStep(7, "Activate the subscription between the DUT and the TH for an attribute of data type \"unsigned integer\". Modify that attribute on the DUT. DUT should send the report data with the modified attribute value. Modify the attribute multiple times (3 times) before the MaxInterval time specified during the subscription activation.",
                #          "Verify on the TH that the DUT sends the correct value of the attribute."),
                # TestStep(8, "Activate the subscription between the DUT and the TH for an attribute of data type \"signed integer\". Modify that attribute on the DUT. DUT should send the report data with the modified attribute value. Modify the attribute multiple times (3 times)before the MaxInterval time specified during the subscription activation.",
                #          "Verify on the TH that the DUT sends the correct value of the attribute."),
                # TestStep(9, "Activate the subscription between the DUT and the TH for an attribute of data type \"floating point\". Modify that attribute on the DUT. DUT should send the report data with the modified attribute value. Modify the attribute multiple times (3 times) before the MaxInterval time specified during the subscription activation.",
                #          "Verify on the TH that the DUT sends the correct value of the attribute."),
                # TestStep(10, "Activate the subscription between the DUT and the TH for an attribute of data type list. Modify that attribute on the DUT. DUT should send the report data with the modified attribute value. Modify the attribute multiple times (3 times) before the MaxInterval time specified during the subscription activation.",
                #          "Verify on the TH that the DUT sends the correct value of the attribute."),
                # TestStep(11, "Activate the subscription between the DUT and the TH for any attribute. KeepSubscriptions flag should be set to False After the Maximum interval time is elapsed, TH should send another subscription request message with different parameters than before. KeepSubscriptions flag should be set to False Change the value of the attribute requested on the DUT.",
                #          "Verify that the DUT sends the changed value of the attribute with the newest subscription id sent with the second request."),
                # TestStep(12, "Activate the subscription between the DUT and the TH for any attribute After the Maximum interval time is elapsed, change the value of the attribute requested on the DUT.",
                #          "Verify that the DUT sends the changed value of the attribute to the TH after the next MinIntervalFloor time has passed."),
                # TestStep(13, "Activate the subscription between the DUT and the TH for an attribute There are no attribute value changes before MaxInterval elapses.",
                #          "Verify that the DUT sends a Report Data action with no data to keep the subscription alive."),
                # TestStep(14, "TH sends a subscription request action for an attribute to the DUT with the KeepSubscriptions flag set to True. Activate the subscription between DUT and the TH. Initiate another subscription request action to the DUT for another attribute with the KeepSubscriptions flag set to True. Change both the attribute values on the DUT.",
                #          "Verify that both the subscriptions are active and the TH receives reports for both these attributes on both subscriptions."),
                # TestStep(15, "TH sends a subscription request action for an attribute to the DUT with the KeepSubscriptions flag set to True. Activate the subscription between DUT and the TH. Initiate another subscription request action to the DUT for another attribute with the KeepSubscriptions flag set to False. Change both the attribute values on the DUT.",
                #          "Verify that both the subscriptions are active and the TH receives notifications for both these attributes. Verify that the first subscription is terminated after the MaxInterval of the first subscription is reached."),
                # TestStep(16, "TH sends a subscription request action for an attribute and all events. Set the MinIntervalFloor to some value say \"N\"(seconds). Change the value of the attribute and trigger an action on the DUT to trigger any event.",
                #          "Verify on TH that DUT sends a report action data for both the attribute and the event after N seconds."),
                # TestStep(17, "TH sends a subscription request action for attribute wildcard - AttributePath = [[Endpoint = EndpointID, Cluster = ClusterID]]. Set the MinIntervalFloor to some value say \"N\"(seconds). Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed after N seconds."),
                # TestStep(18, "TH sends a subscription request to subscribe to an attribute on a specific cluster from all endpoints AttributePath = [[Attribute = Attribute, Cluster = ClusterID ]]. Set the MinIntervalFloor to some value say \"N\"(seconds). Change the attribute on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed after N seconds."),
                # TestStep(19, "TH sends a subscription request to subscribe to all attributes from all clusters from all endpoints. AttributePath = [[]]. Set the MinIntervalFloor to some value say \"N\"(seconds). Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed after N seconds."),
                # TestStep(20, "TH sends a subscription request to subscribe to all attributes from all clusters on an endpoint. AttributePath = [[Endpoint = EndpointID]]. Set the MinIntervalFloor to some value say \"N\"(seconds). Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed after N seconds."),
                # TestStep(21, "TH sends a subscription request to subscribe to all attributes from a specific cluster on all endpoints. AttributePath = [[Cluster = ClusterID]]. Set the MinIntervalFloor to some value say \"N\"(seconds). Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed after N seconds.")
                ]

    @async_test_body
    async def test_TC_IDM_4_3(self):

        # Test setup
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        node_label_attr_path = [(0, node_label_attr)]
        TH: ChipDeviceController = self.default_controller

        # *** Step 1a ***
        # DUT and TH activate the subscription.
        self.step("1a")

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
        sub_th_step1a_min_interval_sec, sub_th_step1a_max_interval_sec = sub_th_step1a.GetReportingIntervalsSeconds()
        asserts.assert_is_not_none(sub_th_step1a_max_interval_sec, "MaxInterval field not present")

        sub_th_step1a.Shutdown()

        # *** Step 1b ***
        # Change the value of the attribute which has been subscribed on the DUT by manually changing some
        # settings on the device. Example: Temperature sensor may update the value of the room temperature.
        # Turning on/off on a light bulb.
        self.step("1b")

        # Modify attribute value
        new_node_label_write = "NewNodeLabel_11001100"
        await TH.WriteAttribute(
            self.dut_node_id,
            [(0, node_label_attr(value=new_node_label_write))]
        )


if __name__ == "__main__":
    default_matter_test_main()
