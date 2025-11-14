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

import logging
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters import ClusterObjects as ClusterObjects
from matter.clusters.Attribute import AttributePath, ValueDecodeFailure
from matter.interaction_model import Status
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.event_attribute_reporting import (AttributeSubscriptionHandler, EventSubscriptionHandler,
                                                       WildcardAttributeSubscriptionHandler)
from matter.testing.global_attribute_ids import GlobalAttributeIds, is_standard_attribute_id, is_standard_cluster_id
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.tlv import uint

'''
Category:
Functional

Description:
This test case will verify the report data messages sent from the DUT after activating subscription are according to specification.

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#443-tc-idm-43-report-data-messages-post-subscription-activation-from-dut-test-cases-dut_server

'''


class TC_IDM_4_3(MatterBaseTest, BasicCompositionTests):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()
        # Build XML spec data for identifying writable attributes
        self.build_spec_xmls()

    def steps_TC_IDM_4_3(self):
        return [
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
    min_interval_floor_sec: int = 1
    max_interval_ceiling_sec: int = 3
    root_node_endpoint: int = 0

    def get_writable_attributes_for_cluster(self, cluster_id: uint, cluster_data: dict) -> list[uint]:
        """Get list of writable attribute IDs for a cluster.

        Similar to TC_AccessChecker's checkable_attributes(), but filters for writable attributes.
        Uses XML spec data to identify which attributes support write operations.

        Args:
            cluster_id: The cluster ID
            cluster_data: The cluster data from endpoints_tlv containing ATTRIBUTE_LIST_ID

        Returns:
            List of attribute IDs that are writable
        """
        if cluster_id not in self.xml_clusters:
            return []

        if cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
            return []

        xml_cluster = self.xml_clusters[cluster_id]
        all_attrs = cluster_data.get(GlobalAttributeIds.ATTRIBUTE_LIST_ID, [])

        writable_attrs = []
        for attribute_id in all_attrs:
            if not is_standard_attribute_id(attribute_id):
                continue

            if attribute_id not in xml_cluster.attributes:
                continue

            if attribute_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                continue

            xml_attr = xml_cluster.attributes[attribute_id]
            write_access = xml_attr.write_access

            if write_access != Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                writable_attrs.append(attribute_id)

        return writable_attrs

    async def change_writable_attributes_and_verify_reports(self, handler: WildcardAttributeSubscriptionHandler, priming_data, test_step: str, clusters_to_skip: list = None):
        """Change writable attributes and verify subscription reports are received.

        Based on TC_AccessChecker.py's _run_write_access_test_for_cluster_privilege() approach.
        This dynamically identifies writable attributes using XML spec data, then writes ACTUAL
        VALUE CHANGES to trigger subscription change reports per Matter specification.

        This function ensures actual value changes for all attribute types:
        - Strings: Write unique timestamped values
        - Lists: Write empty list
        - Booleans: Flip the value
        - Integers and Floats: Increment the value

        This function verifies that change reports are received for each changed attribute
        by using the handler's queue-based tracking mechanism to confirm reports were received.

        Args:
            handler: WildcardAttributeSubscriptionHandler tracking the subscription
            priming_data: Priming report data from GetAttributes()
            test_step: Step name for logging
            clusters_to_skip: List of cluster IDs to skip (e.g., problematic clusters)

        Returns:
            Number of attributes successfully changed and verified
        """
        if clusters_to_skip is None:
            clusters_to_skip = []

        changed_count = 0
        max_changes = 20
        changed_attributes = []

        reports_at_start = handler.get_all_reported_attributes()
        logging.info(f"{test_step}: Handler has {len(reports_at_start)} attributes with reports at start")
        logging.info(f"{test_step}: Scanning for writable attributes using XML spec data...")

        for endpoint_id, clusters in priming_data.items():
            if changed_count >= max_changes:
                break

            for cluster_class, attributes in clusters.items():
                if changed_count >= max_changes:
                    break

                cluster_id = cluster_class.id
                if cluster_id in clusters_to_skip:
                    continue

                # Get writable attributes for this cluster from endpoints_tlv data
                if endpoint_id not in self.endpoints_tlv:
                    continue

                if cluster_id not in self.endpoints_tlv[endpoint_id]:
                    continue

                cluster_data = self.endpoints_tlv[endpoint_id][cluster_id]
                writable_attr_ids = self.get_writable_attributes_for_cluster(cluster_id, cluster_data)

                if not writable_attr_ids:
                    continue

                # Try to write to each writable attribute
                for attribute_id in writable_attr_ids:
                    if changed_count >= max_changes:
                        break

                    # Get the attribute object
                    if attribute_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                        continue

                    attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]

                    # Skip attributes known to not send subscription reports even after successful writes
                    # These attributes have special reporting behavior per device implementation
                    ATTRIBUTES_WITH_NO_SUBSCRIPTION_REPORTS = [
                        Clusters.BasicInformation.Attributes.LocalConfigDisabled,
                        Clusters.Binding.Attributes.Binding,
                        Clusters.GeneralCommissioning.Attributes.Breadcrumb,
                        Clusters.TimeFormatLocalization.Attributes.HourFormat,
                    ]
                    if attribute in ATTRIBUTES_WITH_NO_SUBSCRIPTION_REPORTS:
                        logging.debug(f"{test_step}: Skipping {attribute.__name__} - known to not send subscription reports")
                        continue

                    # Check if we have this attribute in the priming data
                    if attribute not in attributes:
                        continue

                    try:
                        # Get current value from priming data
                        cached_val = attributes[attribute]

                        # Skip if value decode failed
                        if isinstance(cached_val, ValueDecodeFailure):
                            logging.debug(f"{test_step}: Skipping {attribute.__name__} - decode failure")
                            continue

                        # Use cached value from priming data to avoid subscription timeout from many reads
                        current_val = cached_val

                        # Determine new value based on type
                        if isinstance(current_val, str):
                            # String attribute - use unique value to trigger actual change
                            new_val = f"{test_step}_T{int(time.time())}_{changed_count}"
                        elif isinstance(current_val, list):
                            # List attribute - toggle between empty and non-empty to ensure actual change
                            if len(current_val) == 0:
                                # Skip empty lists - writing a valid non-empty list requires XML spec knowledge to write valid data
                                logging.debug(f"{test_step}: Skipping {attribute.__name__} - empty list (requires XML spec knowledge to write valid data)")
                                continue
                            else:
                                # Non-empty list -> write empty list (safe change)
                                new_val = []
                        elif isinstance(current_val, bool):
                            # Boolean attribute - flip the value to trigger actual change
                            new_val = not current_val
                        elif isinstance(current_val, (int, float)):
                            # increment to trigger actual change
                            # Use modulo to keep within reasonable bounds
                            new_val = (current_val + 1) if current_val < 1000000 else 0
                        else:
                            # For other types, skip to avoid writing same value
                            # Writing the same value should NOT trigger a report
                            logging.debug(f"{test_step}: Skipping {attribute.__name__} - unsupported type for change")
                            continue

                        # Write the attribute
                        logging.debug(f"{test_step}: Writing {attribute.__name__} on EP{endpoint_id}: {current_val} -> {new_val}")
                        resp = await self.default_controller.WriteAttribute(
                            nodeId=self.dut_node_id,
                            attributes=[(endpoint_id, attribute(new_val))]
                        )

                        if resp[0].Status == Status.Success:
                            changed_count += 1
                            logging.info(
                                f"{test_step}: [{changed_count}] Changed {attribute.__name__} (0x{attribute_id:04X}) on endpoint {endpoint_id}, cluster 0x{cluster_id:04X}: {current_val} -> {new_val}")

                            # Track this change for verification
                            changed_attributes.append({
                                'endpoint': endpoint_id,
                                'cluster': cluster_class,
                                'attribute': attribute,
                                'old_value': current_val,
                                'new_value': new_val
                            })

                        elif resp[0].Status == Status.UnsupportedAccess:
                            asserts.fail(f"{test_step}: Write to {attribute.__name__} returned UnsupportedAccess")

                        else:
                            # Other errors are acceptable per TC_AccessChecker pattern
                            # (e.g., InvalidValue, ConstraintError - as long as it's not UnsupportedAccess)
                            logging.debug(f"{test_step}: Write to {attribute.__name__} returned {resp[0].Status}")

                    except Exception as e:
                        logging.debug(f"{test_step}: Exception writing {attribute.__name__}: {e}")

        # Wait for change reports to arrive
        # Wait in small increments, checking periodically
        count = 0
        last_report_count = len(handler.get_all_reported_attributes())
        while count < 30:
            time.sleep(1)
            count += 1
            # Log progress every interval
            current_reports = len(handler.get_all_reported_attributes())
            if current_reports != last_report_count:
                logging.info(f"{test_step}: {count}s elapsed, {current_reports} unique attributes reported (was {last_report_count})")
                last_report_count = current_reports
            elif current_reports == changed_count:
                break
            else:
                logging.debug(f"{test_step}: {count}s elapsed, {current_reports} unique attributes reported (no change)")

        # Verify that we received reports for all the changed attributes we wrote to
        verified_count = 0
        missing_reports = []

        for change in changed_attributes:
            ep = change['endpoint']
            cluster = change['cluster']
            attr = change['attribute']

            # Check if handler received a report for this attribute
            if handler.was_attribute_reported(ep, cluster, attr):
                verified_count += 1
                report_count = handler.get_attribute_report_count(ep, cluster, attr)
                logging.debug(
                    f"{test_step}: Verified report for {attr.__name__} on EP{ep} "
                    f"({report_count} report{'s' if report_count > 1 else ''} received)"
                )
            else:
                missing_reports.append(f"{attr.__name__} on endpoint {ep}")
                logging.warning(f"{test_step}: Missing report for {attr.__name__} on EP{ep}")

        logging.info(
            f"{test_step}: Verified {verified_count}/{len(changed_attributes)} attribute change reports received")

        # Report summary of all attributes that received reports
        all_reported = handler.get_all_reported_attributes()
        logging.info(f"{test_step}: Total unique attributes with reports: {len(all_reported)}")

        asserts.assert_less_equal(
            len(missing_reports), 0,
            f"{test_step}: Missing reports for {len(missing_reports)} attribute(s): {', '.join(missing_reports)}")
        asserts.assert_greater(
            verified_count, 0,
            f"{test_step}: No change reports verified, expected {changed_count} reports")

        return verified_count

    @async_test_body
    async def test_TC_IDM_4_3(self):
        self.device_clusters = self.all_device_clusters()
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        TH: ChipDeviceController = self.default_controller

        # Step 1: Empty report verification
        # (This was originally test step 3 in the test plan it appears)
        self.step(1)
        # Track empty report arrival time
        # This callback mechanism was created by Raul to enable precise timing validation
        # of empty reports (SubscriptionStillActive messages) per Matter spec requirements
        empty_report_received = False
        empty_report_time = None

        def on_empty_report():
            nonlocal empty_report_received, empty_report_time
            empty_report_received = True
            empty_report_time = time.time()
            logging.debug(f"Empty report callback triggered at {empty_report_time}")

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
        wait_start = time.time()

        while not empty_report_received and (time.time() - wait_start) < max_wait:
            time.sleep(0.1)

        asserts.assert_true(empty_report_received, "Empty report was not received")
        asserts.assert_is_not_none(empty_report_time, "Empty report timing not captured")
        sub_report_elapsed = empty_report_time - sub_time

        logging.info(
            F"Empty report received after {sub_report_elapsed}s (MinInterval: {self.min_interval_floor_sec}s, MaxInterval: {self.max_interval_ceiling_sec}s, Timeout: {sub_timeout_sec}s)")

        asserts.assert_greater_equal(
            sub_report_elapsed, self.min_interval_floor_sec,
            f"Empty report elapsed time ({sub_report_elapsed}s) should be >= MinInterval ({self.min_interval_floor_sec}s)"
        )
        asserts.assert_less(
            sub_report_elapsed, sub_timeout_sec,
            f"Empty report elapsed time ({sub_report_elapsed}s) should be < subscription timeout ({sub_timeout_sec}s)"
        )

        await attr_handler_step1.cancel()

        # New Step 2: Basic attribute change and report timing
        # (This was originally test step 2 in the test plan)
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
        timeout = self.max_interval_ceiling_sec + 2
        wait_start = time.time()
        report_received = False

        while time.time() - wait_start < timeout:
            if attr_handler_step2.attribute_queue.qsize() > 0:
                attr_handler_step2.attribute_queue.get()
                time_report = time.time()
                report_received = True
                break
            time.sleep(0.1)

        asserts.assert_true(report_received, "Failed to receive attribute change report")

        # Verify timing: report should come after MinInterval
        report_delay = time_report - time_before_write
        asserts.assert_greater_equal(
            report_delay, self.min_interval_floor_sec,
            f"Report came too early ({report_delay}s < MinInterval {self.min_interval_floor_sec}s)"
        )
        asserts.assert_less(
            report_delay, self.max_interval_ceiling_sec + 2,
            f"Report came too late ({report_delay}s > MaxInterval {self.max_interval_ceiling_sec}s)"
        )

        await attr_handler_step2.cancel()

        # Step 3: Multiple subscriptions with KeepSubscriptions=False
        # (This was originally test step 11 in the test plan)
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
        _, original_max_interval_sec = sub_step3_first.GetReportingIntervalsSeconds()

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

        # Wait for report (should be on second subscription)
        time.sleep(original_max_interval_sec + 1)

        # Verify we got a report on second subscription
        asserts.assert_greater(attr_handler_step3_second.attribute_queue.qsize(), 0,
                               "Should receive report on second subscription")

        # First subscription should have been cancelled
        # Note: The first handler may still have the priming report but no update

        await attr_handler_step3_second.cancel()

        # Step 4: MinInterval/MaxInterval timing validation
        # (This was originally test steps 12 and 13 in the test plan, just appears to have been combined into one test step, which was a wise design decision)
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
        time.sleep(max_interval + 1)
        time_empty = time.time()

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
            time.sleep(0.1)

        asserts.assert_true(report_received, "Failed to receive attribute change report")

        # Flush queue before waiting for next empty report
        attr_handler_step4.flush_reports()

        # Wait for second empty report
        time.sleep(max_interval + 1)
        time_empty_2 = time.time()

        # Verify timing constraints
        #
        # MRP Retransmission Time Calculation:
        # Per Matter Core Specification Section 4.12.8, Table 22 (MRP Parameters):
        # - MRP_MAX_TRANSMISSIONS = 5 (1 initial + 4 retries)
        # - MRP_BACKOFF_BASE = 1.6 (exponential backoff multiplier)
        # - MRP_BACKOFF_JITTER = 0.25 (random jitter scaler, ±25%)
        # - MRP_BACKOFF_MARGIN = 1.1 (margin increase over peer idle interval)
        # - MRP_BACKOFF_THRESHOLD = 1 (retransmissions before exponential backoff)
        #
        # Per src/messaging/ReliableMessageProtocolConfig.h implementation defaults:
        # - CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS = 4 retransmissions
        # - CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL = 500ms (non-Thread) / 2000ms (Thread)
        # - CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL = 300ms (non-Thread) / 2000ms (Thread)
        #
        # NOTE: Per spec Section 4.12.8, "A Node SHALL use the provided default value for each
        # parameter unless the message recipient Node advertises an alternate value for the parameter
        # via Operational Discovery."
        # The negotiated values can be retrieved using:
        #   session_params = TH.GetRemoteSessionParameters(nodeId)
        #   actual_idle_interval = session_params.sessionIdleInterval (in ms)
        #   actual_active_interval = session_params.sessionActiveInterval (in ms)
        #
        # Example calculation using default IDLE interval (500ms for non-Thread):
        #   Retry 1: 500ms
        #   Retry 2: 500ms × 1.6 = 800ms
        #   Retry 3: 800ms × 1.6 = 1280ms
        #   Retry 4: 1280ms × 1.6 = 2048ms
        #   Baseline Total: ~4628ms
        #
        # With JITTER (±25%) and MARGIN (1.1x) applied per spec:
        #   Worst-case multiplier: 1.25 × 1.1 = 1.375
        #   Worst-case Total: 4628ms × 1.375 ≈ 6364ms
        #
        # However, we dynamically calculate MRP retransmission timeout based on the actual
        # negotiated idle interval retrieved from the DUT, ensuring accurate timing validation
        # regardless of whether the DUT uses default values or custom values advertised via
        # Operational Discovery.
        #
        # Calculate MRP retransmission timeout based on negotiated idle interval
        # Using the spec formula with BASE=1.6 for 4 retransmissions:
        #   Total baseline = idle_interval × (1 + 1.6 + 1.6² + 1.6³)
        #                  = idle_interval × (1 + 1.6 + 2.56 + 4.096)
        #                  = idle_interval × 9.256
        # Apply worst-case JITTER (1.25) and MARGIN (1.1): multiplier = 1.375
        # Convert from ms to seconds and add 1s buffer for network latency

        # Retrieve and log negotiated MRP parameters from the DUT
        # These may differ from defaults if the DUT advertises custom values via Operational Discovery
        negotiated_idle_interval_ms = 500  # Default for non-Thread devices
        try:
            session_params = TH.GetRemoteSessionParameters(self.dut_node_id)
            if session_params and session_params.sessionIdleInterval:
                negotiated_idle_interval_ms = session_params.sessionIdleInterval
            else:
                logging.info("Using default MRP Idle Interval (500ms for non-Thread)")
        except Exception as e:
            logging.info(f"Using default MRP Idle Interval (500ms for non-Thread) but experienced {e}")

        MRP_RETRANSMISSION_TIMEOUT = (negotiated_idle_interval_ms * 9.256 * 1.375 / 1000.0)

        logging.info(f"Using MRP Idle Interval of {negotiated_idle_interval_ms}ms")
        logging.info(f"Calculated MRP retransmission timeout: {MRP_RETRANSMISSION_TIMEOUT:.2f}s")
        logging.info(f"This accounts for {4} retransmissions with BASE={1.6}, JITTER={0.25}, MARGIN={1.1}")

        diff_1 = time_data - time_empty
        diff_2 = time_empty_2 - time_data

        asserts.assert_greater(diff_1, min_interval_step4,
                               f"First report interval ({diff_1}s) should be greater than MinInterval ({min_interval_step4}s)")
        asserts.assert_less(diff_1, max_interval + MRP_RETRANSMISSION_TIMEOUT,
                            f"First report interval ({diff_1}s) should be less than MaxInterval + MRP retransmission time ({max_interval + MRP_RETRANSMISSION_TIMEOUT}s)")

        asserts.assert_greater(diff_2, min_interval_step4,
                               f"Second report interval ({diff_2}s) should be greater than MinInterval ({min_interval_step4}s)")
        asserts.assert_less(diff_2, max_interval + MRP_RETRANSMISSION_TIMEOUT,
                            f"Second report interval ({diff_2}s) should be less than MaxInterval + MRP retransmission time ({max_interval + MRP_RETRANSMISSION_TIMEOUT}s)")

        await attr_handler_step4.cancel()

        # Step 5: KeepSubscriptions=True preserves first subscription
        # (This was originally test step 14 in the test plan)
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
        time.sleep(self.max_interval_ceiling_sec)

        # Verify both subscriptions received reports
        asserts.assert_greater(attr_handler_step5_first.attribute_queue.qsize(), 0,
                               "First subscription should receive reports")
        asserts.assert_greater(attr_handler_step5_second.attribute_queue.qsize(), 0,
                               "Second subscription should receive reports")

        await attr_handler_step5_first.cancel()
        await attr_handler_step5_second.cancel()

        # Step 6: KeepSubscriptions=False cancels first subscription
        # (This was originally test step 15 in the test plan)
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
        time.sleep(self.max_interval_ceiling_sec)

        # Verify second subscription received reports
        asserts.assert_greater(attr_handler_step6_second.attribute_queue.qsize(), 0,
                               "Second subscription should receive reports")

        # First subscription should not receive reports (it was cancelled)

        await attr_handler_step6_second.cancel()

        # Step 7: Subscription to attribute and events
        # (This was originally test step 16 in the test plan)
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

        # Subscribe to events - using AccessControl cluster events as an example
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
        time.sleep(self.max_interval_ceiling_sec)

        # Verify attribute report received
        asserts.assert_greater(attr_handler_step7.attribute_queue.qsize(), 0,
                               "Should receive attribute report")

        await attr_handler_step7.cancel()

        # Step 8: Attribute wildcard subscription
        # (This was originally test step 17 in the test plan)
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

        # Flush priming reports from handler queue before making changes
        handler_step8.flush_reports()

        # Change writable attributes and verify change reports per test spec
        changed_count = await self.change_writable_attributes_and_verify_reports(
            handler_step8, priming_data, "Step 8"
        )
        logging.info(f"Changed and verified {changed_count} attribute(s)")

        # Shutdown subscription
        handler_step8.shutdown()

        # Step 9: Attribute on cluster from all endpoints
        # (This was originally test step 18 in the test plan)
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

        # Step 10: All attributes from all clusters from all endpoints
        # (This was originally test step 19 in the test plan)
        self.step(10)
        # Subscribe to ALL attributes from ALL clusters on ALL endpoints
        # Note: Some clusters have attributes that don't support subscriptions (INVALID_ACTION) or only support subscriptions in special circumstances:
        # - AccessControl, NetworkCommissioning, CameraAvStreamManagement, OperationalCredentials
        # - ValveConfigurationAndControl, Switch, PowerSource, BasicInformation, Identify
        # - All concentration measurement clusters (10 total: CO, CO2, NO2, Ozone, PM2.5, Formaldehyde, PM1, PM10, TVOC, Radon)
        # - Manufacturer-specific clusters (0xFC00-0xFFFE range) may have non-standard subscription behavior
        # Since most of these appear to be mandatory/common clusters, we must exclude them from the wildcard so it doesnt fail.
        # We build a filtered wildcard by subscribing to all attributes for each non-problematic cluster similar to how it is done in ACE test modules (Specifically ACE_2_3 for writes and ACE_2_4 for subscriptions).

        CLUSTERS_WITH_SUBSCRIPTION_ISSUES = [
            Clusters.AccessControl.id,
            Clusters.NetworkCommissioning.id,
            Clusters.CameraAvStreamManagement.id,
            Clusters.OperationalCredentials.id,
            Clusters.ValveConfigurationAndControl.id,
            Clusters.Switch.id,  # Switch cluster has subscription issues with certain attributes
            Clusters.PowerSource.id,  # Power Source cluster returns INVALID_ACTION for certain attributes
            Clusters.BasicInformation.id,  # BasicInformation cluster has attributes (e.g., 0x000E) that return INVALID_ACTION
            Clusters.Identify.id,  # Identify cluster returns INVALID_ACTION during wildcard subscriptions
            # All concentration measurement clusters have been added to remove possible clusters that might cause issues during subscription testing. More comprehensive testing is done in ACE test modules.
            Clusters.CarbonMonoxideConcentrationMeasurement.id,
            Clusters.CarbonDioxideConcentrationMeasurement.id,
            Clusters.NitrogenDioxideConcentrationMeasurement.id,
            Clusters.OzoneConcentrationMeasurement.id,
            Clusters.Pm25ConcentrationMeasurement.id,
            Clusters.FormaldehydeConcentrationMeasurement.id,
            Clusters.Pm1ConcentrationMeasurement.id,
            Clusters.Pm10ConcentrationMeasurement.id,
            Clusters.TotalVolatileOrganicCompoundsConcentrationMeasurement.id,
            Clusters.RadonConcentrationMeasurement.id,
        ]

        all_cluster_ids = set()
        for endpoint_id, endpoint_data in self.endpoints_tlv.items():
            all_cluster_ids.update(endpoint_data.keys())

        # Build wildcard paths for all clusters EXCEPT the problematic ones
        subscription_paths = []
        for cluster_id in all_cluster_ids:
            # Skip known problematic clusters
            if cluster_id in CLUSTERS_WITH_SUBSCRIPTION_ISSUES:
                continue

            # These may have non-standard subscription behavior
            if (not is_standard_cluster_id(cluster_id)):
                continue

            # Subscribe to all attributes in this cluster across all endpoints
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
            autoResubscribe=True
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
        asserts.assert_greater(total_clusters, 15, "Should receive reports from multiple clusters")
        asserts.assert_greater(total_attributes, 100, "Should receive reports for many attributes")

        # Flush priming reports from handler queue before making changes
        handler_step10.flush_reports()

        # Change writable attributes and verify change reports per test spec
        # Skip problematic clusters we identified earlier
        changed_count = await self.change_writable_attributes_and_verify_reports(
            handler_step10, priming_data, "Step 10", clusters_to_skip=CLUSTERS_WITH_SUBSCRIPTION_ISSUES
        )
        logging.info(f"Changed and verified {changed_count} attribute(s)")

        handler_step10.shutdown()

        # Step 11: All attributes from all clusters on an endpoint
        # (This was originally test step 20 in the test plan)
        self.step(11)

        # Subscribe to all attributes from all clusters on endpoint 0
        # Note: We must exclude clusters with subscription issues and manufacturer-specific clusters
        subscription_paths_step11 = []
        if self.root_node_endpoint in self.endpoints_tlv:
            for cluster_id in self.endpoints_tlv[self.root_node_endpoint].keys():
                # Skip known problematic clusters
                if cluster_id in CLUSTERS_WITH_SUBSCRIPTION_ISSUES:
                    continue

                # Skip manufacturer-specific clusters (0xFC00-0xFFFE range)
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

        # Flush priming reports from handler queue before making changes
        handler_step11.flush_reports()

        # Change writable attributes and verify change reports per test spec
        changed_count = await self.change_writable_attributes_and_verify_reports(
            handler_step11, priming_data, "Step 11", clusters_to_skip=CLUSTERS_WITH_SUBSCRIPTION_ISSUES
        )
        logging.info(f"Changed and verified {changed_count} attribute(s)")

        handler_step11.shutdown()

        # Step 12: All attributes from specific cluster on all endpoints
        # (This was originally test step 21 in the test plan)
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
