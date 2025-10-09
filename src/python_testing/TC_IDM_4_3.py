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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import inspect
import logging
import queue
import time
from enum import IntFlag

from mobly import asserts, signals

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters import ClusterObjects as ClusterObjects
from matter.clusters.Attribute import SubscriptionTransaction
from matter.clusters.enum import MatterIntEnum
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

'''
Category:
Functional

Description:
This test case will verify the report data messages sent from the DUT after activating
subscription are according to specification.

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#443-tc-idm-43-report-data-messages-post-subscription-activation-from-dut-test-cases-dut_server
'''


class TC_IDM_4_3(MatterBaseTest, BasicCompositionTests):

    # ANSI escape codes for background colors
    BACKGROUND_COLORS = {
        'black': '\033[40m',
        'red': '\033[41m',
        'green': '\033[42m',
        'yellow': '\033[43m',
        'blue': '\033[44m',
        'magenta': '\033[45m',
        'cyan': '\033[46m',
        'white': '\033[47m',
        'reset': '\033[0m'
    }

    # Function to print text with a specific background color
    def fprint(self, text: str, background_color: str, padding: int = 0):
        double_space = "  " * padding
        padding_space = "\n" * (padding - 1)
        print(f"{padding_space}{double_space}{self.BACKGROUND_COLORS.get(background_color,
              self.BACKGROUND_COLORS['reset'])}{text}{self.BACKGROUND_COLORS['reset']}{padding_space}")

    def steps_TC_IDM_4_3(self):
        return [TestStep(1, "DUT and TH activate the subscription for an attribute. Do not change the value of the attribute which has been subscribed.",
                         "Verify that there is an empty report data message sent from the DUT to the TH after the MinInterval time and no later than the MaxInterval time plus an additional duration equal to the total retransmission time according to negotiated MRP parameters."),
                TestStep(2, "Activate the subscription between the DUT and the TH for an attribute of data type bool. If no such attribute exists, skip this step.",
                         "Verify the subscription was successfully activated and a priming data report was sent"),
                # TestStep(3, "Activate the subscription between the DUT and the TH for an attribute of data type string. If no such attribute exists, skip this step.",
                #          "Verify the subscription was successfully activated and a priming data report was sent"),
                # TestStep(4, "Activate the subscription between the DUT and the TH for an attribute of data type unsigned integer. If no such attribute exists, skip this step.",
                #          "Verify the subscription was successfully activated and a priming data report was sent"),
                # TestStep(5, "Activate the subscription between the DUT and the TH for an attribute of data type signed integer. If no such attribute exists, skip this step.",
                #          "Verify the subscription was successfully activated and a priming data report was sent"),
                # TestStep(6, "Activate the subscription between the DUT and the TH for an attribute of data type floating point. If no such attribute exists, skip this step.",
                #          "Verify the subscription was successfully activated and a priming data report was sent"),
                # TestStep(7, "Activate the subscription between the DUT and the TH for an attribute of data type list. If no such attribute exists, skip this step.",
                #          "Verify the subscription was successfully activated and a priming data report was sent"),
                # TestStep(8, "Activate the subscription between the DUT and the TH for any attribute. KeepSubscriptions flag should be set to False Save the returned MaxInterval value as original_max_interval TH then sends another subscription request message for the same attribute with different parameters than before. KeepSubscriptions flag should be set to False Wait for original_max_interval. Change the value of the attribute requested on the DUT.",
                #          "Verify that the DUT sends the changed value of the attribute with the newest subscription id sent with the second request."),
                # TestStep(9, "Activate the subscription between the DUT and the TH for any attribute with MinIntervalFloor set to 5 seconds and MaxIntervalCeiling set to 10. Save the returned MaxInterval as max_interval. Wait to receive the empty report on the subscription and save the time the report was received as time_empty. TH then changes the attribute and waits for a data report. Save the time the report was received as time_data. TH then waits for a second empty report on the subscription and saves the time the report was received as time_empty_2",
                #          "Verify that time_data - time_empty is larger than the MinIntervalFloor and smaller than max_interval plus an additional duration equal to the total retransmission time according to negotiated MRP parameters. Verify that time_empty_2 - time_data is larger than the MinIntervalFloor and smaller than max_interval plus an additional duration equal to the total retransmission time according to negotiated MRP parameters."),
                # TestStep(10, "TH sends a subscription request action for an attribute to the DUT with the KeepSubscriptions flag set to False. Activate the subscription between DUT and the TH. Initiate another subscription request action to the DUT for another attribute with the KeepSubscriptions flag set to True. Change both the attribute values on the DUT.",
                #          "Verify that the TH receives reports for both these attributes on their respective subscriptions."),
                # TestStep(11, "TH sends a subscription request action for an attribute to the DUT with the KeepSubscriptions flag set to False. Activate the subscription between DUT and the TH. Initiate another subscription request action to the DUT for another attribute with the KeepSubscriptions flag set to False. Change both the attribute values on the DUT.",
                #          "Verify that the TH receives a report for the second attribute on the second subscription. Verify that that the TH does not receive a report on the first subscription."),
                # TestStep(12, "TH sends a subscription request action for an attribute and all events. Change the value of the attribute and trigger an action on the DUT to trigger any event.",
                #          "Verify on TH that DUT sends a report action data for both the attribute and the event."),
                # TestStep(13, "TH sends a subscription request action for attribute wildcard - AttributePath = [[Endpoint = EndpointID, Cluster = ClusterID]] for a cluster where more than 1 attribute can be changed by the TH. Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed."),
                # TestStep(14, "TH sends a subscription request to subscribe to an attribute on a specific cluster from all endpoints AttributePath = [[Attribute = Attribute, Cluster = ClusterID ]]. Change the attribute on the DUT",
                #          "Verify that the DUT sends a priming reports for all the attributes."),
                # TestStep(15, "TH sends a subscription request to subscribe to all attributes from all clusters from all endpoints. AttributePath = [[]]. Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed."),
                # TestStep(16, "TH sends a sub scription request to subscribe to all attributes from all clusters on an endpoint. AttributePath = [[Endpoint = EndpointID]]. Change all or few of the attributes on the DUT",
                #          "Verify that the DUT sends reports for all the attributes that have changed."),
                # TestStep(17, "TH sends a subscription request to subscribe to all attributes from a specific cluster on all endpoints. AttributePath = [[Cluster = ClusterID]].",
                #          "Verify that the DUT sends a priming reports for all the attributes."),
                ]

    def on_notify_subscription_still_active(self):

        self.previous_report_data_time = self.current_report_data_time
        self.current_report_data_time = time.time()

        if self.previous_report_data_time > 0:
            diff = self.current_report_data_time - self.previous_report_data_time
        else:
            diff = 0

        self.fprint(f"NotifyLogic {time.time()} >>> t: {diff}", "red")

    def on_notify_subscription_still_active_empty_report(self):
        self.report_data_received = True
        self.empty_report_time = time.time()

    def wait_for_attribute_update_report(self, expected_attribute, output):
        try:
            path, transaction = output.get(block=True, timeout=10)
        except queue.Empty:
            asserts.fail(
                f"[AttributeChangeCallback | Local] Failed to receive a report for the {expected_attribute} attribute change")

        asserts.assert_equal(path.AttributeType, expected_attribute,
                             f"[AttributeChangeCallback | Local] Received incorrect report. Expected: {expected_attribute}, received: {path.AttributeType}")
        try:
            attribute_value = transaction.GetAttribute(path)

            self.attr_update_report_data_time = time.time()

            logging.debug(
                f"[AttributeChangeCallback | Local] Got attribute subscription report. Attribute {path.AttributeType}. Updated value: {attribute_value}. SubscriptionId: {transaction.subscriptionId}")
        except KeyError:
            asserts.fail("[AttributeChangeCallback | Local] Attribute {expected_attribute} not found in returned report")

    def all_device_clusters(self) -> set:
        device_clusters = set()
        for endpoint in self.endpoints:
            device_clusters |= set(self.endpoints[endpoint].keys())
        return device_clusters

    async def all_type_attributes_for_cluster(self, cluster: ClusterObjects.Cluster, desired_type: type) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        all_attributes = [attribute for attribute in cluster.Attributes.__dict__.values() if inspect.isclass(
            attribute) and issubclass(attribute, ClusterObjects.ClusterAttributeDescriptor)]

        # Hackish way to get enums to return properly -- the default behavior (under else block) returns a BLANK LIST without this workaround
        # If type(attribute.attribute_type.Type) or type(ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type).Type are enums, they return <class 'aenum._enum.EnumType'>, which are equal!
        if desired_type == MatterIntEnum:
            all_attributes_of_type = [attribute for attribute in all_attributes if type(
                attribute.attribute_type.Type) == type(ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type).Type)]
        elif desired_type == IntFlag:
            try:
                feature_map = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.FeatureMap)
            except signals.TestFailure:
                print(f"{cluster} does not support Attributes.FeatureMap")
                return []
            if feature_map >= 1:
                return [cluster.Attributes.FeatureMap]
        else:
            all_attributes_of_type = [attribute for attribute in all_attributes if attribute.attribute_type ==
                                      ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type)]
        return all_attributes_of_type

    async def check_attribute_read_for_type(self, attribute_type: type, return_objects: bool = False) -> None:
        # Get all clusters from device
        self.fprint(f"self.device_clusters: {self.device_clusters}", "red", 2)
        for cluster in self.device_clusters:

            # TEMPORARY: if cluster is SmokeCoAlarm skip, as it returns INVALID_ACTION when trying
            # to subscribe to its TestInProgress attribute
            # if cluster.id == 92:
            #     continue

            all_types = await self.all_type_attributes_for_cluster(cluster, attribute_type)
            self.fprint(f"all_types: {all_types}", "green", 2)

            if all_types:
                chosen_attribute = all_types[0]
                chosen_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_attribute.cluster_id]
                break
        else:
            print(f"Attribute type not found on device: {attribute_type}")
            chosen_cluster = None

        endpoint = None
        for endpoint in self.endpoints:
            if (chosen_cluster in self.endpoints[endpoint]) and (chosen_attribute in self.endpoints[endpoint][chosen_cluster]):
                break

        if chosen_cluster and (endpoint is not None):
            output = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                dev_ctrl=self.default_controller,
                cluster=chosen_cluster,
                attribute=chosen_attribute)
            return chosen_cluster, chosen_attribute, output if return_objects else output
        return

    current_report_data_time = 0
    previous_report_data_time = 0
    attr_update_report_data_time = 0

    min_interval_floor_sec: int = 1
    max_interval_ceiling_sec: int = 3

    empty_report_time = None
    report_data_received = False

    root_node_endpoint = 0

    @async_test_body
    async def test_TC_IDM_4_3(self):

        await self.setup_class_helper(allow_pase=False)

        # all_clusters = [cluster for cluster in Clusters.ClusterObjects.ALL_ATTRIBUTES]
        # server_list_attr = Clusters.Descriptor.Attributes.ServerList
        # attribute_list = Clusters.Descriptor.Attributes.AttributeList
        # descriptor_obj = Clusters.Descriptor
        # server_list_attr_path = [(0, server_list_attr)]
        # descriptor_obj_path = [(0, descriptor_obj)]
        # attribute_list_path = [0, attribute_list]
        self.device_clusters = self.all_device_clusters()
        self.all_supported_clusters = [cluster for cluster in Clusters.__dict__.values(
        ) if inspect.isclass(cluster) and issubclass(cluster, ClusterObjects.Cluster)]

        # Test setup
        # Mandatory writable attributes
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel

        # bc = Clusters.GeneralCommissioning.Attributes.Breadcrumb
        # Event
        # acl = Clusters.AccessControl.Events.

        node_label_attr_path = [(self.root_node_endpoint, node_label_attr)]
        TH: ChipDeviceController = self.default_controller

        # # *** Step 1a ***
        # # DUT and TH activate the subscription.
        # self.step("1a")

        # # Subscribe to attribute
        # sub_th_step1ab = await TH.ReadAttribute(
        #     nodeid=self.dut_node_id,
        #     attributes=node_label_attr_path,
        #     reportInterval=(self.min_interval_floor_sec, self.max_interval_ceiling_sec),
        #     keepSubscriptions=False
        # )

        # sub_th_step1ab.SetNotifySubscriptionStillActiveCallback(self.on_notify_subscription_still_active)

        # secs = 3
        # print(f"\n\n\n\n\nTime to sleep {secs} second(s)")
        # time.sleep(secs)
        # print(f"Rise and shine after {secs} second(s)\n\n\n\n\n")

        # # Verify that the subscription is activated between TH and DUT
        # # Verify on the TH, a report data message is received.
        # asserts.assert_true(sub_th_step1ab.subscriptionId, "Subscription not activated")

        # # Verify subscriptionId field is present
        # asserts.assert_is_not_none(sub_th_step1ab.subscriptionId, "SubscriptionId field not present")

        # # Verify MaxInterval field is present
        # sub_th_step1ab_min_interval_sec, sub_th_step1ab_max_interval_sec = sub_th_step1ab.GetReportingIntervalsSeconds()
        # asserts.assert_is_not_none(sub_th_step1ab_max_interval_sec, "MaxInterval field not present")

        # *** Step 1b ***
        # Change the value of the attribute which has been subscribed on the DUT by manually changing some
        # settings on the device. Example: Temperature sensor may update the value of the room temperature.
        # Turning on/off on a light bulb.
        # self.step("1b")

        # # Set Attribute Update Callback
        # node_label_update_cb = AttributeChangeCallback(node_label_attr)
        # sub_th_step1ab.SetAttributeUpdateCallback(node_label_update_cb)

        # # Update attribute value
        # new_node_label_write = "NewNodeLabel_11001100"
        # await TH.WriteAttribute(
        #     self.dut_node_id,
        #     [(0, node_label_attr(value=new_node_label_write))]
        # )

        # self.wait_for_attribute_update_report(node_label_attr, node_label_update_cb._output)

        # # Number of seconds elapsed between the last report data event
        # # and the arrival of the attribute update report data
        # elapsed_time_since_report = self.attr_update_report_data_time - self.previous_report_data_time

        # # Convert the current time to a datetime object
        # update_time = datetime.fromtimestamp(self.attr_update_report_data_time)
        # previous_time = datetime.fromtimestamp(self.previous_report_data_time)

        # # Format the datetime object into the desired string format
        # update_time_f = update_time.strftime("%H:%M:%S.%f")
        # previous_time_f = previous_time.strftime("%H:%M:%S.%f")

        # self.fprint(f"\n\n\t\elapsed_time_since_report: {elapsed_time_since_report}s\n\t\tattr_update_report_data_time: {update_time_f}s\n\t\tprevious_report_data_time: {previous_time_f}s\n\n", "green")

        # # Verify that the attribute update report data is sent
        # # after MinInterval time and before MaxInterval time
        # asserts.assert_greater(elapsed_time_since_report, self.min_interval_floor_sec,
        #                        f"Attribute update report data must be sent after the MinInterval")
        # asserts.assert_less(elapsed_time_since_report, self.max_interval_ceiling_sec,
        #                 f"Attribute update report data must be sent before the MaxInterval")

        # sub_th_step1ab.Shutdown()

        # DUT and TH activate the subscription. Change the value of the attribute which has been
        # subscribed on the DUT by sending an IMWrite or Invoke message to the DUT from the TH.
        # Verify that there is a report data message sent from the DUT for the changed value of
        # the attribute. Verify that the Report Data is sent when the minimum interval time is
        # reached and before the MaxInterval time.
        # self.step(2)

        # DUT and TH activate the subscription for an attribute. Do not change the value of the
        # attribute which has been subscribed. Verify that there is an empty report data message
        # sent from the DUT to the TH after the MinInterval time and no later than the
        # MaxInterval time plus an additional duration equal to the total retransmission time
        # according to negotiated MRP parameters.
        # self.step(3)

        # *** Step 1 ***
        # DUT and TH activate the subscription for an attribute. Do not change the value of the
        # attribute which has been subscribed. Verify that there is an empty report data message
        # sent from the DUT to the TH after the MinInterval time and no later than the MaxInterval
        # time plus an additional duration equal to the total retransmission time according to
        # negotiated MRP parameters.
        self.step(1)

        # Subscribe to attribute
        sub_th_step1: SubscriptionTransaction = await TH.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(self.min_interval_floor_sec, self.max_interval_ceiling_sec),
            keepSubscriptions=False,
            fabricFiltered=False
        )

        # Record time after subscription
        sub_time = time.time()

        # Get subscription timeout
        sub_timeout_sec = sub_th_step1.GetSubscriptionTimeoutMs() / 1000

        # Records the time the first empty report after subscription arrives
        sub_th_step1.SetNotifySubscriptionStillActiveCallback(self.on_notify_subscription_still_active_empty_report)

        # Wait for empty report data
        wait_increments = self.min_interval_floor_sec / 10
        while not self.report_data_received:
            time.sleep(wait_increments)
            self.fprint(f"Time: {time.time()}", "blue")
            self.fprint(f"Empty Rport time: {self.empty_report_time}", "green")
            if self.report_data_received:
                break

        # Elapsed time between subscription established and first report data
        sub_report_data_elapsed_time = self.empty_report_time - sub_time

        self.fprint(f"min_interval_floor_sec: {self.min_interval_floor_sec}", "red")
        self.fprint(f"sub_report_data_elapsed_time: {sub_report_data_elapsed_time}", "red")
        self.fprint(f"sub_timeout_sec: {sub_timeout_sec}", "red")

        # Verify that the empty report data message from the DUT to the TH was sent
        # after the MinInterval time and no later than the MaxInterval time plus an
        # additional duration equal to the total retransmission time according to
        # negotiated MRP parameters
        asserts.assert_greater(sub_report_data_elapsed_time, self.min_interval_floor_sec,
                               "Empty report not received after the MinInterval time")
        asserts.assert_less(sub_report_data_elapsed_time, sub_timeout_sec, "Empty report not received before the MaxInterval time")

        sub_th_step1.Shutdown()

        # Activate the subscription between the DUT and the TH for an attribute of
        # data type bool. If no such attribute exists, skip this step. Verify the
        # subscription was successfully activated and a priming data report was sent
        self.step(2)

        # Check for attribute of type bool
        out = await self.check_attribute_read_for_type(
            attribute_type=bool,
            return_objects=True
        )

        # If found subscribe to attribute
        if out:
            self.fprint(f"out: {out}", "green", 5)
            cluster, attribute, value = out
            attr_path = [(self.root_node_endpoint, attribute)]

            # Subscribe to attribute
            self.fprint(f"Will sub TO: {attr_path}", "black", 7)
            logging.info("Attribute of type 'bool' was found")
            sub_th_step2: SubscriptionTransaction = await TH.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=attr_path,
                reportInterval=(self.min_interval_floor_sec, self.max_interval_ceiling_sec),
                keepSubscriptions=False,
                fabricFiltered=False
            )

            # Verify the subscription was successfully activated and a priming
            # data report was sent
            asserts.assert_is_not_none(sub_th_step2.subscriptionId,
                                       "Subscription activation to attribute of type 'bool' was unsuccessful")
            logging.info("Subscription activation to attribute of type 'bool' successful")

            sub_th_step2.Shutdown()
        else:
            logging.info("No attribute of type 'bool' was found, skipping step")


if __name__ == "__main__":
    default_matter_test_main()
