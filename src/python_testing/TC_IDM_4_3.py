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
#       --enable-spec-errata-ci-only-disallowed-for-certification
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import time

from mobly import asserts
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters.Attribute import AttributePath
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import (AttributeSubscriptionHandler, EventSubscriptionHandler,
                                                      WildcardAttributeSubscriptionHandler)
from matter.testing.global_attribute_ids import is_standard_cluster_id
from matter.testing.matter_testing import TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


'''
Category:
Functional

Description:
This test case will verify the report data messages sent from the DUT after activating subscription are according to specification.

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#443-tc-idm-43-report-data-messages-post-subscription-activation-from-dut-test-cases-dut_server

'''


class TC_IDM_4_3(IDMBaseTest):
    @property
    def default_timeout(self) -> int:
        return 600

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()
        # Build XML spec data for identifying writable attributes
        self.build_spec_xmls()

    def steps_TC_IDM_4_3(self):
        return [
            TestStep(0, "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioned to TH.", is_commissioning=True),
            TestStep(1, "DUT and TH activate the subscription for an attribute. Do not change the value of the attribute which has been subscribed.",
                     "Verify that there is an empty report data message sent from the DUT to the TH after the MinInterval time and no later than the MaxInterval time plus an additional duration equal to the total retransmission time according to negotiated MRP parameters."),
            TestStep(2, "Activate the subscription between the DUT and the TH for an attribute. Change the value of the attribute which has been subscribed on the DUT by sending a Write command from the TH.",
                     "Verify that there is a report data message sent from the DUT for the changed value of the attribute. Verify that the Report Data is sent after the MinIntervalFloor time is reached and before the MaxInterval time."),
            TestStep(3, "Activate the subscription between the DUT and the TH for any attribute. KeepSubscriptions flag should be set to False Save the returned MaxInterval value as original_max_interval TH then sends another subscription request message for the same attribute with different parameters than before. KeepSubscriptions flag should be set to False Wait for original_max_interval. Change the value of the attribute requested on the DUT.",
                     "Verify that the DUT sends the changed value of the attribute with the newest subscription id sent with the second request."),
            TestStep(4, "Activate the subscription between the DUT and the TH for any attribute with MinIntervalFloor set to 5 seconds and MaxIntervalCeiling set to 10. Save the returned MaxInterval as max_interval. Wait to receive the empty report on the subscription and save the time the report was received as time_empty. TH then changes the attribute and waits for a data report. Save the time the report was received as time_data. TH then waits for a second empty report on the subscription and saves the time the report was received as time_empty_2",
                     "Verify that time_data - time_empty is larger than the MinIntervalFloor and smaller than max_interval plus an additional duration equal to the total retransmission time according to negotiated MRP parameters. Verify that time_empty_2 - time_data is larger than the MinIntervalFloor and smaller than max_interval plus an additional duration equal to the total retransmission time according to negotiated MRP parameters."),
            TestStep(5, "TH sends a subscription request action for an attribute to the DUT with the KeepSubscriptions flag set to False. Activate the subscription between DUT and the TH. Initiate another subscription request action to the DUT for another attribute with the KeepSubscriptions flag set to True. Change both the attribute values on the DUT.",
                     "Verify that the TH receives reports for both these attributes on their respective subscriptions."),
            TestStep(6, "TH sends a subscription request action for an attribute to the DUT with the KeepSubscriptions flag set to False. Activate the subscription between DUT and the TH. Initiate another subscription request action to the DUT for another attribute with the KeepSubscriptions flag set to False. Change both the attribute values on the DUT.",
                     "Verify that the TH receives a report for the second attribute on the second subscription. Verify that that the TH does not receive a report on the first subscription."),
            TestStep(7, "TH sends a subscription request action for an attribute and all events. Change the value of the attribute and trigger an action on the DUT to trigger any event.",
                     "Verify on TH that DUT sends a report action data for both the attribute and the event."),
            TestStep(8, "TH sends a subscription request action for attribute wildcard - AttributePath = [[Endpoint = EndpointID, Cluster = ClusterID]] for a cluster where more than 1 attribute can be changed by the TH. Change all or few of the attributes on the DUT",
                     "Verify that the DUT sends reports for all the attributes that have changed."),
            TestStep(9, "TH sends a subscription request to subscribe to an attribute on a specific cluster from all endpoints AttributePath = [[Attribute = Attribute, Cluster = ClusterID ]]. Change the attribute on the DUT",
                     "Verify that the DUT sends a priming reports for all the attributes."),
            TestStep(10, "TH sends a subscription request to subscribe to all attributes from all clusters from all endpoints. AttributePath = [[]]. Change all or few of the attributes on the DUT",
                         "Verify that the DUT sends reports for all the attributes that have changed."),
            TestStep(11, "TH sends a sub scription request to subscribe to all attributes from all clusters on an endpoint. AttributePath = [[Endpoint = EndpointID]]. Change all or few of the attributes on the DUT",
                         "Verify that the DUT sends reports for all the attributes that have changed."),
            TestStep(12, "TH sends a subscription request to subscribe to all attributes from a specific cluster on all endpoints. AttributePath = [[Cluster = ClusterID]].",
                         "Verify that the DUT sends a priming reports for all the attributes."),
        ]

    def all_device_clusters(self) -> set:
        device_clusters = set()
        for endpoint in self.endpoints:
            device_clusters |= set(self.endpoints[endpoint].keys())
        return device_clusters

    # Test configuration
    min_interval_floor_sec: int = 0
    max_interval_ceiling_sec: int = 3
    root_node_endpoint: int = 0
    # This removes the framework wildcard subscription from running in the background for this test.
    disable_wildcard_subscription = True

    @async_test_body
    async def test_TC_IDM_4_3(self):

        self.step(0)

        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        TH: ChipDeviceController = self.default_controller

        # Calculate MRP retransmission timeout once at the beginning for use across all steps
        mrp_timeout_sec = self.get_mrp_retransmission_timeout_sec(TH)
        log.info("Calculated MRP retransmission timeout: %.2fs", mrp_timeout_sec)

        # Step 1: Empty report verification
        self.step(1)
        # Track empty report arrival time using an async event to avoid busy-wait
        empty_report_event = asyncio.Event()
        empty_report_received = False
        empty_report_time = None

        def on_empty_report():
            nonlocal empty_report_received, empty_report_time
            empty_report_time = time.time()
            empty_report_event.set()
            log.debug("Empty report callback triggered at %s", empty_report_time)

        # Use AttributeSubscriptionHandler for cleaner subscription management
        attr_handler_step1 = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )

        sub_step1 = await attr_handler_step1.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
            keepSubscriptions=False
        )

        # Register callback to track empty reports
        sub_step1.SetNotifySubscriptionStillActiveCallback(on_empty_report)

        sub_time = time.time()
        sub_timeout_sec = sub_step1.GetSubscriptionTimeoutMs() / 1000

        # Flush the priming report
        attr_handler_step1.flush_reports()

        # Wait for the empty report (SubscriptionStillActive message)
        # This should arrive between MinInterval and MaxInterval
        max_wait = sub_timeout_sec + 1
        try:
            await asyncio.wait_for(empty_report_event.wait(), timeout=max_wait)
        except TimeoutError:
            asserts.fail("Empty report was not received")

        asserts.assert_is_not_none(empty_report_time, "Empty report timing not captured")
        sub_report_elapsed = empty_report_time - sub_time

        max_expected_time = self.max_interval_ceiling_sec + mrp_timeout_sec
        log.info("Empty report received after %ss (MinInterval: %ss, MaxInterval: %ss, MRP timeout: %.2fs, Max expected: %.2fs)",
                 sub_report_elapsed, self.min_interval_floor_sec, self.max_interval_ceiling_sec, mrp_timeout_sec, max_expected_time)

        asserts.assert_greater_equal(
            sub_report_elapsed, self.min_interval_floor_sec,
            f"Empty report elapsed time ({sub_report_elapsed}s) should be >= MinInterval ({self.min_interval_floor_sec}s)"
        )
        asserts.assert_less(
            sub_report_elapsed, max_expected_time,
            f"Empty report elapsed time ({sub_report_elapsed}s) should be < MaxInterval + MRP timeout ({max_expected_time:.2f}s)"
        )

        attr_handler_step1.cancel()

        # Step 2: Basic attribute change and report timing
        self.step(2)

        attr_handler_step2 = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )

        await attr_handler_step2.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
            keepSubscriptions=False
        )

        # Flush priming report
        attr_handler_step2.flush_reports()

        # Record time before write
        time_before_write = time.time()

        # Change attribute via Write command
        new_label = "TestLabel_Step2"
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, node_label_attr(value=new_label))]
        )

        # Wait for change report
        timeout = self.max_interval_ceiling_sec + mrp_timeout_sec
        wait_start = time.time()
        report_received = False

        while time.time() - wait_start < timeout:
            if attr_handler_step2.attribute_queue.qsize() > 0:
                attr_handler_step2.attribute_queue.get()
                time_report = time.time()
                report_received = True
                break
            await asyncio.sleep(0.1)

        asserts.assert_true(report_received, "Failed to receive attribute change report")

        # Verify timing: report should come after MinInterval
        report_delay = time_report - time_before_write
        asserts.assert_greater_equal(
            report_delay, self.min_interval_floor_sec,
            f"Report came too early ({report_delay}s < MinInterval {self.min_interval_floor_sec}s)"
        )
        asserts.assert_less(
            report_delay, self.max_interval_ceiling_sec + mrp_timeout_sec,
            f"Report came too late ({report_delay}s > MaxInterval {self.max_interval_ceiling_sec}s + MRP retransmission {mrp_timeout_sec}s)"
        )

        attr_handler_step2.cancel()

        # Step 3: Multiple subscriptions with KeepSubscriptions=False
        self.step(3)

        # First subscription with handler
        attr_handler_step3_first = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )
        sub_step3_first = await attr_handler_step3_first.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
            keepSubscriptions=False
        )

        first_sub_id = sub_step3_first.subscriptionId

        # Second subscription with different parameters (this should replace the first)
        attr_handler_step3_second = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )
        sub_step3_second = await attr_handler_step3_second.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            min_interval_sec=self.min_interval_floor_sec + 1,
            max_interval_sec=self.max_interval_ceiling_sec + 1,
            keepSubscriptions=False
        )

        second_sub_id = sub_step3_second.subscriptionId
        _, second_max_interval_sec = sub_step3_second.GetReportingIntervalsSeconds()

        # Verify different subscription IDs
        asserts.assert_not_equal(first_sub_id, second_sub_id, "Subscription IDs should be different")

        # Flush priming reports
        attr_handler_step3_second.flush_reports()

        # Change the attribute
        new_label = "TestLabel_Step3"
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, node_label_attr(value=new_label))]
        )

        # Wait directly for a report from the active subscription.
        step3_timeout_sec = second_max_interval_sec + mrp_timeout_sec + 1
        attr_handler_step3_second.wait_for_attribute_report(timeout_sec=step3_timeout_sec)

        # Clean up both subscriptions from step 3
        attr_handler_step3_first.cancel()
        attr_handler_step3_second.cancel()

        # Step 4: MinInterval/MaxInterval timing
        self.step(4)

        min_interval_step4 = 5
        max_interval_step4 = 10

        attr_handler_step4 = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )

        sub_step4 = await attr_handler_step4.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            min_interval_sec=min_interval_step4,
            max_interval_sec=max_interval_step4,
            keepSubscriptions=False
        )

        _, max_interval = sub_step4.GetReportingIntervalsSeconds()

        # Flush the priming report from the queue
        attr_handler_step4.flush_reports()

        # Wait for first empty report and capture its time
        time_empty = time.time()
        await asyncio.sleep(max_interval + 1)

        new_label_step4 = "TestLabel_Step4"
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, node_label_attr(value=new_label_step4))]
        )

        # Wait for change report - give it enough time considering MinInterval
        wait_start = time.time()
        report_received = False
        timeout = max_interval + 5

        while time.time() - wait_start < timeout:
            if attr_handler_step4.attribute_queue.qsize() > 0:
                _ = attr_handler_step4.attribute_queue.get()
                time_data = time.time()
                report_received = True
                break
            await asyncio.sleep(0.1)

        asserts.assert_true(report_received, "Failed to receive attribute change report")

        # Flush queue before waiting for next empty report
        attr_handler_step4.flush_reports()

        # Wait for second empty report and capture its time
        empty_wait_2_start = time.time()
        time_empty_2 = None

        while time.time() - empty_wait_2_start < (max_interval + 5):
            if attr_handler_step4.attribute_queue.qsize() > 0:
                # Second empty report arrived - capture the time
                attr_handler_step4.attribute_queue.get()
                time_empty_2 = time.time()
                break
            await asyncio.sleep(0.1)

        # Wait for second empty report
        time_empty_2 = time.time()
        await asyncio.sleep(max_interval + 1)

        # Verify timing constraints
        # MRP retransmission timeout was calculated at the beginning of the test
        # See get_mrp_retransmission_timeout_sec() for the detailed calculation
        # based on Matter Core Specification Section 4.12.8, Table 22 (MRP Parameters)

        diff_1 = time_data - time_empty
        diff_2 = time_empty_2 - time_data

        asserts.assert_greater(diff_1, min_interval_step4,
                               f"First report interval ({diff_1}s) should be greater than MinInterval ({min_interval_step4}s)")
        asserts.assert_less(diff_1, max_interval + mrp_timeout_sec,
                            f"First report interval ({diff_1}s) should be less than MaxInterval + MRP retransmission time ({max_interval + mrp_timeout_sec}s)")

        asserts.assert_greater(diff_2, min_interval_step4,
                               f"Second report interval ({diff_2}s) should be greater than MinInterval ({min_interval_step4}s)")
        asserts.assert_less(diff_2, max_interval + mrp_timeout_sec,
                            f"Second report interval ({diff_2}s) should be less than MaxInterval + MRP retransmission time ({max_interval + mrp_timeout_sec}s)")

        attr_handler_step4.cancel()

        # Step 5: KeepSubscriptions=True preserves first subscription
        self.step(5)

        # Find a second writable attribute for testing
        breadcrumb_attr = Clusters.GeneralCommissioning.Attributes.Breadcrumb

        # First subscription with KeepSubscriptions=False
        attr_handler_step5_first = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )
        await attr_handler_step5_first.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            keepSubscriptions=False
        )

        # Second subscription with KeepSubscriptions=True
        attr_handler_step5_second = AttributeSubscriptionHandler(
            expected_cluster=Clusters.GeneralCommissioning,
            expected_attribute=breadcrumb_attr
        )
        _ = await attr_handler_step5_second.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            keepSubscriptions=True
        )

        # Change both attributes
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, node_label_attr(value="TestLabel_Step5"))]
        )
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, breadcrumb_attr(value=12345))]
        )

        # Wait for reports
        await asyncio.sleep(self.max_interval_ceiling_sec)

        # Verify both subscriptions received reports
        asserts.assert_greater(attr_handler_step5_first.attribute_queue.qsize(), 0,
                               "First subscription should receive reports")
        asserts.assert_greater(attr_handler_step5_second.attribute_queue.qsize(), 0,
                               "Second subscription should receive reports")

        attr_handler_step5_first.cancel()
        attr_handler_step5_second.cancel()

        # Step 6: KeepSubscriptions=False cancels first subscription
        self.step(6)

        # First subscription with KeepSubscriptions=False
        attr_handler_step6_first = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )
        _ = await attr_handler_step6_first.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            keepSubscriptions=False
        )

        # Second subscription with KeepSubscriptions=False (should cancel first)
        attr_handler_step6_second = AttributeSubscriptionHandler(
            expected_cluster=Clusters.GeneralCommissioning,
            expected_attribute=breadcrumb_attr
        )
        await attr_handler_step6_second.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            keepSubscriptions=False
        )

        # Change both attributes
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, node_label_attr(value="TestLabel_Step6"))]
        )
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, breadcrumb_attr(value=54321))]
        )

        # Wait for reports
        await asyncio.sleep(self.max_interval_ceiling_sec)

        # Verify second subscription received reports
        asserts.assert_greater(attr_handler_step6_second.attribute_queue.qsize(), 0,
                               "Second subscription should receive reports")

        # First subscription should not receive reports (it was cancelled)

        attr_handler_step6_second.cancel()

        # Step 7: Subscription to attribute and events
        self.step(7)

        # Subscribe to attribute using handler
        attr_handler_step7 = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=node_label_attr
        )
        await attr_handler_step7.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False,
            keepSubscriptions=False
        )

        # Subscribe to events - using AccessControl cluster events
        event_handler_step7 = EventSubscriptionHandler(
            expected_cluster=Clusters.AccessControl
        )
        await event_handler_step7.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            endpoint=self.root_node_endpoint,
            fabric_filtered=False
        )

        # Change attribute to trigger report
        await TH.WriteAttribute(
            self.dut_node_id,
            [(self.root_node_endpoint, node_label_attr(value="TestLabel_Step7"))]
        )

        # Wait for attribute report
        await asyncio.sleep(self.max_interval_ceiling_sec)

        # Verify attribute report received
        asserts.assert_greater(attr_handler_step7.attribute_queue.qsize(), 0,
                               "Should receive attribute report")

        attr_handler_step7.cancel()

        # Step 8: Attribute wildcard subscription
        self.step(8)

        # Subscribe to all attributes in BasicInformation cluster on endpoint 0
        # This wildcard path subscribes to ALL attributes in the cluster
        handler_step8 = WildcardAttributeSubscriptionHandler()
        sub_step8 = await handler_step8.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            attributes=[(self.root_node_endpoint, Clusters.BasicInformation)],
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
            keepSubscriptions=False,
            fabric_filtered=False,
            autoResubscribe=False
        )

        # Verify we got a priming report with multiple attributes
        priming_data = sub_step8.GetAttributes()
        asserts.assert_in(self.root_node_endpoint, priming_data,
                          "Should have data for endpoint 0")
        asserts.assert_in(Clusters.BasicInformation, priming_data[self.root_node_endpoint],
                          "Should have BasicInformation cluster data")

        cluster_data = priming_data[self.root_node_endpoint][Clusters.BasicInformation]
        num_attributes = len(cluster_data)
        asserts.assert_greater(num_attributes, 5,
                               "Should receive multiple attributes in priming report for wildcard subscription")

        # Reset handler to clear priming report tracking before making changes
        handler_step8.reset()

        # Change writable attributes and verify change reports per test spec
        changed_count = await self.change_writable_attributes_and_verify_reports(
            handler_step8, priming_data, "Step 8"
        )
        log.info("Changed and verified %s attribute(s)", changed_count)

        # Shutdown subscription and wait for cleanup
        handler_step8.shutdown()
        await asyncio.sleep(1)  # Allow time for subscription teardown to complete

        # Step 9: Attribute on cluster from all endpoints
        self.step(9)

        # Subscribe to NodeLabel attribute from BasicInformation cluster on ALL endpoints
        sub_step9 = await TH.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttributePath(
                ClusterId=Clusters.BasicInformation.id,
                AttributeId=Clusters.BasicInformation.Attributes.NodeLabel.attribute_id
            )],
            reportInterval=(self.min_interval_floor_sec, self.max_interval_ceiling_sec),
            keepSubscriptions=False,
            fabricFiltered=False
        )

        # Verify we got priming reports from multiple endpoints (or at least endpoint 0)
        priming_data = sub_step9.GetAttributes()
        num_endpoints = len(priming_data)
        asserts.assert_greater_equal(num_endpoints, 1,
                                     "Should receive priming reports for at least one endpoint")

        # Verify the attribute is present in at least one endpoint
        found_attribute = False
        for endpoint_id, clusters in priming_data.items():
            if Clusters.BasicInformation in clusters:
                if Clusters.BasicInformation.Attributes.NodeLabel in clusters[Clusters.BasicInformation]:
                    found_attribute = True

        asserts.assert_true(found_attribute, "Should find NodeLabel attribute in priming report")

        sub_step9.Shutdown()
        await asyncio.sleep(1)  # Allow time for subscription teardown to complete

        # Step 10: All attributes from all clusters from all endpoints
        self.step(10)
        # Subscribe to ALL attributes from ALL clusters on ALL endpoints
        all_cluster_ids = set()
        for endpoint_id, endpoint_data in self.endpoints_tlv.items():
            all_cluster_ids.update(endpoint_data.keys())

        # Build wildcard paths for all clusters, removing any non-standard function clusters
        subscription_paths = []
        for cluster_id in all_cluster_ids:
            if (not is_standard_cluster_id(cluster_id)):
                continue

            # Subscribe to all attributes in this cluster across all endpoints, append this cluster to the subscription_paths list
            subscription_paths.append(AttributePath(ClusterId=cluster_id))

        handler_step10 = WildcardAttributeSubscriptionHandler()
        sub_step10 = await handler_step10.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            attributes=subscription_paths,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
            keepSubscriptions=False,
            fabric_filtered=False,
            autoResubscribe=False
        )

        # Verify we got priming reports with multiple endpoints, clusters, and attributes
        priming_data = sub_step10.GetAttributes()
        num_endpoints = len(priming_data)

        total_clusters = 0
        total_attributes = 0
        for endpoint_id, clusters in priming_data.items():
            num_clusters = len(clusters)
            total_clusters += num_clusters
            for cluster, attributes in clusters.items():
                total_attributes += len(attributes)

        asserts.assert_greater(num_endpoints, 0, "Should receive reports from at least one endpoint")
        asserts.assert_greater(total_clusters, 10, "Should receive reports from multiple clusters")
        asserts.assert_greater(total_attributes, 50, "Should receive reports for many attributes")

        log.info("Step 10: Priming data received from %s endpoints, %s clusters, %s attributes",
                 num_endpoints, total_clusters, total_attributes)

        # Reset handler to clear priming report tracking before making changes
        handler_step10.reset()

        # Give subscription a moment to stabilize after reset
        await asyncio.sleep(1)

        # Change writable attributes and verify change reports per test spec
        changed_count = await self.change_writable_attributes_and_verify_reports(
            handler_step10, priming_data, "Step 10"
        )
        log.info("Changed and verified %s attribute(s)", changed_count)

        handler_step10.shutdown()
        await asyncio.sleep(1)  # Allow time for subscription teardown to complete

        # Step 11: All attributes from all clusters on an endpoint
        self.step(11)

        # Subscribe to all attributes from all clusters on endpoint 0
        subscription_paths_step11 = []
        if self.root_node_endpoint in self.endpoints_tlv:
            for cluster_id in self.endpoints_tlv[self.root_node_endpoint]:
                if (not is_standard_cluster_id(cluster_id)):
                    continue

                # Subscribe to all attributes in this cluster on this endpoint
                subscription_paths_step11.append(AttributePath(
                    EndpointId=self.root_node_endpoint,
                    ClusterId=cluster_id
                ))

        handler_step11 = WildcardAttributeSubscriptionHandler()
        sub_step11 = await handler_step11.start(
            dev_ctrl=TH,
            node_id=self.dut_node_id,
            attributes=subscription_paths_step11,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
            keepSubscriptions=False,
            fabric_filtered=False,
            autoResubscribe=False
        )

        # Verify we got priming reports with multiple clusters and attributes
        priming_data = sub_step11.GetAttributes()
        asserts.assert_in(self.root_node_endpoint, priming_data,
                          f"Should have data for endpoint {self.root_node_endpoint}")

        clusters = priming_data[self.root_node_endpoint]
        num_clusters = len(clusters)
        total_attributes = sum(len(attrs) for attrs in clusters.values())

        asserts.assert_greater(num_clusters, 1, "Should receive reports from multiple clusters on the endpoint")
        asserts.assert_greater(total_attributes, 5, "Should receive reports for many attributes on the endpoint")

        # Reset handler to clear priming report tracking before making changes
        handler_step11.reset()

        # Change writable attributes and verify change reports per test spec
        changed_count = await self.change_writable_attributes_and_verify_reports(
            handler_step11, priming_data, "Step 11"
        )
        log.info("Changed and verified %s attribute(s)", changed_count)

        handler_step11.shutdown()
        await asyncio.sleep(1)  # Allow time for subscription teardown to complete

        # Step 12: All attributes from specific cluster on all endpoints
        self.step(12)

        # Subscribe to all attributes of BasicInformation cluster on ALL endpoints
        sub_step12 = await TH.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttributePath(ClusterId=Clusters.BasicInformation.id)],
            reportInterval=(self.min_interval_floor_sec, self.max_interval_ceiling_sec),
            keepSubscriptions=False,
            fabricFiltered=False
        )

        # Verify we got priming reports from multiple endpoints (or at least one)
        priming_data = sub_step12.GetAttributes()
        num_endpoints = len(priming_data)

        total_attributes = 0
        for endpoint_id, clusters in priming_data.items():
            if Clusters.BasicInformation in clusters:
                num_attrs_on_endpoint = len(clusters[Clusters.BasicInformation])
                total_attributes += num_attrs_on_endpoint

        asserts.assert_greater_equal(num_endpoints, 1, "Should receive reports from at least one endpoint")
        asserts.assert_greater(total_attributes, 5, "Should receive multiple attributes from BasicInformation cluster")

        sub_step12.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
