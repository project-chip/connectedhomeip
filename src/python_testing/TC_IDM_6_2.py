#
#    Copyright (c) 2026 Project CHIP Authors
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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --enable-spec-errata-ci-only-disallowed-for-certification
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time

from mobly import asserts
from support_modules.icd_support import assert_subscription_heartbeat_received
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# Events are read back from AccessControl rather than from a cluster only test applications
# implement, so this test stays runnable against any DUT. AccessControlEntryChanged is
# mandatory on every device, is emitted by commissioning, and can be emitted again on demand
# by rewriting the ACL.
EVENT_CLUSTER = Clusters.AccessControl
EVENT = Clusters.AccessControl.Events.AccessControlEntryChanged

# Reading AccessControlEntryChanged requires the administer privilege, so a controller holding
# only view on an unrelated cluster is refused it. BasicInformation is that unrelated cluster:
# it is mandatory on every DUT and its events are readable at view.
UNPRIVILEGED_CLUSTER = Clusters.BasicInformation

# Offset added to the largest observed event number to build an EventMin no event can satisfy.
EVENT_MIN_BEYOND_LATEST = 1000


class TC_IDM_6_2(IDMBaseTest):
    """Events subscribe interaction from TH to DUT. [{DUT_Server}]"""

    # Factory reset in step 19 drops the session. The background wildcard
    # subscription does not survive that and fails the test.
    disable_wildcard_subscription = True

    # Floor used when a step checks that a report is held until MinInterval.
    min_interval_floor_sec = 5
    max_interval_ceiling_sec = 10
    # Short ceiling so the keepalive step does not sit for the spec maximum.
    keepalive_max_interval_sec = 3

    @property
    def default_timeout(self) -> int:
        return 600

    @pics("MCORE.IDM.S")
    @async_test_body
    async def test_TC_IDM_6_2(self) -> None:
        endpoint = self.ROOT_NODE_ENDPOINT_ID

        self.step(0, "Commissioning, already done", is_commissioning=True)
        await self.setup_class_helper(allow_pase=False)

        th: ChipDeviceController = self.default_controller
        dut_acl_original = await self.get_dut_acl(th)

        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        cr2_nodeid = self.matter_test_config.controller_node_id + 1
        cr2: ChipDeviceController = fabric_admin.NewController(
            nodeId=cr2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        # Guarantee the DUT holds at least one readable event before the path shapes below
        # are exercised, rather than relying on the events commissioning happened to leave.
        await self.emit_access_control_entry_changed(ctrl=th)

        paths = self.wildcard_event_paths(endpoint=endpoint, cluster=EVENT_CLUSTER, event=EVENT)
        subscription_ids: set[int] = set()
        mrp_timeout_sec = self.get_mrp_retransmission_timeout_sec(th)
        log.info("Calculated MRP retransmission timeout: %.2fs", mrp_timeout_sec)

        self.step(1, "TH sends Subscribe Request Message to DUT with EventRequests set to a specific event from a specific cluster "
                  "on a specific endpoint on a specific node that is, [Node = Specific, Endpoint = Specific, Cluster = Specific, Event = Specific].",
                  expectation="Verify TH receives Report Data Message with SubscriptionId which uniquely identifies this subscription on the publisher and data for specific event in Subscribe Request Message.")
        await self.subscribe_event_path(th, paths[0], EVENT, EVENT_CLUSTER, subscription_ids)

        self.step(2, "TH sends Subscribe Request Message to DUT with EventRequests set to all events from a specific cluster "
                  "on a specific endpoint on a specific node that is, [Node = Specific, Endpoint = Specific, Cluster = Specific, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with SubscriptionId which uniquely identifies this subscription on the publisher and data for events in Subscribe Request Message.")
        await self.subscribe_event_path(th, paths[1], EVENT, EVENT_CLUSTER, subscription_ids)

        self.step(3, "TH sends Subscribe Request Message to DUT with EventRequests set to all events from all clusters on a specific endpoint "
                  "on a specific node that is, [Node = Specific, Endpoint = Specific, Cluster = Wildcard, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with SubscriptionId which uniquely identifies this subscription on the publisher and data for events in Subscribe Request Message.")
        await self.subscribe_event_path(th, paths[2], EVENT, EVENT_CLUSTER, subscription_ids)

        self.step(4, "TH sends Subscribe Request Message to DUT with EventRequests set to a specific event from a specific cluster "
                  "on all endpoints on a specific node that is, [Node = Specific, Endpoint = Wildcard, Cluster = Specific, Event = Specific].",
                  expectation="Verify TH receives Report Data Message with SubscriptionId which uniquely identifies this subscription on the publisher and data for specific event in Subscribe Request Message.")
        await self.subscribe_event_path(th, paths[3], EVENT, EVENT_CLUSTER, subscription_ids)

        self.step(5, "TH sends Subscribe Request Message to DUT with EventRequests set to all events from a specific cluster "
                  "on all endpoints on a specific node that is, [Node = Specific, Endpoint = Wildcard, Cluster = Specific, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with SubscriptionId which uniquely identifies this subscription on the publisher and data for events in Subscribe Request Message.")
        await self.subscribe_event_path(th, paths[4], EVENT, EVENT_CLUSTER, subscription_ids)

        self.step(6, "TH sends Subscribe Request Message to DUT with EventRequests set to all events from all clusters "
                  "on all endpoints on a specific node that is, [Node = Specific, Endpoint = Wildcard, Cluster = Wildcard, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with SubscriptionId which uniquely identifies this subscription on the publisher and data for events in Subscribe Request Message.")
        await self.subscribe_event_path(th, paths[5], EVENT, EVENT_CLUSTER, subscription_ids)

        self.step(7, "Activate Event subscription from TH to DUT and trigger the subscribed event on the DUT before minimum interval.",
                  expectation="Verify TH receives Report Data Message for event only after the minimum interval has expired.")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
        ) as (handler, sub):
            started_at = time.time()
            await self.emit_access_control_entry_changed(ctrl=th)
            await self.await_reports_held_until_min_interval(
                handler, sub, started_at, mrp_timeout_sec,
                min_interval_sec=self.min_interval_floor_sec,
            )

        self.step(8, "Activate Event subscription from TH to DUT and and trigger the subscribed event on the DUT before minimum interval. "
                  "'IsUrgent' flag is set to True for a particular event path in the EventPathIB in the SubscribeRequestMessage.",
                  expectation="Verify TH receives Report Data Message for all subscribed events, including any queued events leading up to (and including) the urgent event, after the minimum interval time.")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, True)],
            event=EVENT,
            min_interval_sec=self.min_interval_floor_sec,
            max_interval_sec=self.max_interval_ceiling_sec,
        ) as (handler, sub):
            started_at = time.time()
            # Two ACL updates land in the MinInterval window so the urgent
            # report includes the queued event and the later one. A non-urgent
            # report waits until MaxInterval, so this one must beat that ceiling.
            await self.emit_access_control_entry_changed(ctrl=th)
            await self.emit_access_control_entry_changed(ctrl=th)
            await self.await_reports_held_until_min_interval(
                handler, sub, started_at, mrp_timeout_sec,
                min_interval_sec=self.min_interval_floor_sec,
                minimum_events=2,
                arrival_limit_sec=self.max_interval_ceiling_sec - 1,
            )

        self.step(9, "Activate Event subscription from TH to DUT with no change in event triggering attribute values before maximum interval.",
                  expectation="Verify TH receives Report Data Message every maximum interval or possibly more frequently to keep the subscription alive.")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.keepalive_max_interval_sec,
        ) as (_handler, sub):
            await assert_subscription_heartbeat_received(
                sub, self.negotiated_max_interval_sec(sub), buffer_s=mrp_timeout_sec)

        self.step(10, "With an active Event subscription from TH to DUT, TH sends another Subscribe Request Message to DUT with KeepSubscriptions as False.",
                  expectation="All existing or pending subscriptions on the publisher for this subscriber shall be terminated. "
                  "Verify TH receives report data message for any event changes for the second subscribe request sent by the TH.")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
            keep_subscriptions=False,
        ) as (_, first_sub), self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
            keep_subscriptions=False,
        ) as (second_handler, second_sub):
            asserts.assert_not_equal(
                first_sub.subscriptionId, second_sub.subscriptionId,
                "KeepSubscriptions=False did not allocate a new SubscriptionId")
            await self.emit_access_control_entry_changed(ctrl=th)
            self.collect_event_reports(second_handler, second_sub, mrp_timeout_sec, minimum=1)

        self.step(11, "TH sends Subscribe Request Message to DUT + DUT sends Report Data message to DUT. "
                  "TH sends Status Response Message with a success Status code.",
                  expectation="Verify TH receives Subscribe Response action to activate the subscription with fields "
                  "SubscriptionId - identifies the subscription (uint32 type). "
                  "MaxInterval - the final maximum interval for the subscription in seconds (uint16 type).")
        # ReadEvent returns only after the stack has acked the priming report with
        # StatusResponse(Success) and the DUT has sent SubscribeResponse.
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
        ) as (_handler, sub):
            asserts.assert_true(self.successful_event_reports(sub.GetEvents(), EVENT),
                                "Priming report did not include the subscribed event")
            asserts.assert_true(self.is_valid_uint32_value(sub.subscriptionId),
                                f"SubscriptionId {sub.subscriptionId} is not a uint32")
            _min_interval, max_interval = sub.GetReportingIntervalsSeconds()
            asserts.assert_true(self.is_valid_uint16_value(max_interval),
                                f"MaxInterval {max_interval} is not a uint16")

        self.step(12, "TH sends Subscribe Request Message to DUT. "
                  "DUT sends Report Data message to DUT + TH sends Status Response Message with an error Status.",
                  expectation="Verify DUT terminates the Subscribe interaction and no further Report Data messages are received.")
        # ReadClient::ProcessReportData always sends StatusResponse(Success).
        # The Python controller uses that same client and has no API to send an error status.
        # https://github.com/project-chip/connectedhomeip/issues/23990
        self.mark_current_step_skipped()

        self.step(13, "TH sends Subscribe Request Message to DUT with EventRequests set to path which indicates a cluster event that is not supported.",
                  expectation="Verify TH receives Report Data Message for the event with EventStatusIB generated with the UNSUPPORTED_EVENT Status Code 0xC7.")
        # AccessControlCluster::EventInfo succeeds for any event id on a present
        # cluster, so the priming report has no EventStatusIB. The manual plan
        # marks this step not testable.
        # https://github.com/project-chip/connectedhomeip/issues/23990
        self.mark_current_step_skipped()

        self.step(14, "TH sends Subscribe Request Message to DUT with EventRequests set to path which requires a privilege that is not granted for the cluster in the path.",
                  expectation="Verify TH receives Report Data Message for the event with EventStatusIB generated with the UNSUPPORTED_ACCESS Status Code 0x7E.")
        cr2_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=UNPRIVILEGED_CLUSTER.id)],
            subjects=[cr2_nodeid],
        )
        try:
            await self.add_ace_to_dut_acl(th, cr2_ace, dut_acl_original)
            # A subscribe whose only path is inaccessible is rejected with InvalidAction
            # before any EventStatusIB. BasicInformation is readable at view, so the
            # subscription is accepted and AccessControlEntryChanged comes back as
            # UnsupportedAccess.
            async with self.event_subscription(
                cr2,
                [(endpoint, EVENT, False), (endpoint, UNPRIVILEGED_CLUSTER, False)],
                event=EVENT,
                min_interval_sec=0,
                max_interval_sec=self.max_interval_ceiling_sec,
            ) as (_handler, sub):
                self.assert_event_status(sub.GetEvents(), Status.UnsupportedAccess)

            self.step(15, "TH sends Subscribe Request Message to DUT with EventRequests set to Wildcard path where reading an event in the path requires a privilege that is not granted for the cluster in the path.",
                      expectation="Verify TH receives Report Data Message with no entry for that event in EventReports list.")
            async with self.event_subscription(
                cr2,
                ['*'],
                cluster=EVENT_CLUSTER,
                min_interval_sec=0,
                max_interval_sec=self.max_interval_ceiling_sec,
            ) as (_handler, sub):
                reports = sub.GetEvents()
                asserts.assert_equal(
                    self.successful_event_reports(reports, EVENT), [],
                    "Wildcard report included AccessControlEntryChanged, which CR2 cannot read")
                asserts.assert_true(
                    self.successful_event_reports(reports),
                    "Wildcard report had no events CR2 is allowed to read")
        finally:
            try:
                await self.write_dut_acl(th, dut_acl_original)
            finally:
                cr2.Shutdown()

        self.step(16, "TH sends Subscribe Request Message to DUT with EventFilterIB Node information field matching the node indicated in the path and EventMin field.",
                  expectation="Verify TH receives Report Data Message with EventReports for event numbers higher than the EventMin field. "
                  "Verify that the subsequent ReportData actions as part of the subscription include the latest EventNo associated with each node generating new events.")
        await self.emit_access_control_entry_changed(ctrl=th)
        await self.emit_access_control_entry_changed(ctrl=th)
        event_numbers = await self.read_event_numbers(th, endpoint, EVENT)
        asserts.assert_greater_equal(len(event_numbers), 2, "Need at least two events to set EventMin")
        event_min = event_numbers[-1]
        asserts.assert_true(
            any(number < event_min for number in event_numbers),
            "Log has no event number below EventMin")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
            event_number_filter=event_min,
        ) as (handler, sub):
            priming = self.successful_event_reports(sub.GetEvents(), EVENT)
            asserts.assert_true(priming, "EventMin filter dropped every event at or above EventMin")
            asserts.assert_false(
                any(report.Header.EventNumber < event_min for report in priming),
                "Priming report included an event number below EventMin")
            latest = max(report.Header.EventNumber for report in priming)
            await self.emit_access_control_entry_changed(ctrl=th)
            later = self.collect_event_reports(handler, sub, mrp_timeout_sec, minimum=1)
            new_number = max(report.Header.EventNumber for report in later)
            asserts.assert_greater(
                new_number, latest,
                "Subsequent report did not include a new EventNumber")

        self.step(17, "TH sends Subscribe Request Message to DUT with EventFilterIB Node information field matching the node indicated in the path and the event number is less than the EventMin field.",
                  expectation="Verify TH receives Report Data Message with empty EventReports. "
                  "Verify that the subsequent ReportData actions as part of the subscription include the latest EventNo associated with each node generating new events.")
        event_numbers = await self.read_event_numbers(th, endpoint, EVENT)
        latest = event_numbers[-1]
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
            event_number_filter=latest + EVENT_MIN_BEYOND_LATEST,
        ) as (_handler, sub):
            asserts.assert_equal(
                self.successful_event_reports(sub.GetEvents()), [],
                "EventMin past every stored event still returned event data")
        # EventMin just above the log is empty now, and the next generated event clears it.
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
            event_number_filter=latest + 1,
        ) as (handler, sub):
            asserts.assert_equal(
                self.successful_event_reports(sub.GetEvents(), EVENT), [],
                "Priming report was not empty for EventMin above the log")
            await self.emit_access_control_entry_changed(ctrl=th)
            later = self.collect_event_reports(handler, sub, mrp_timeout_sec, minimum=1)
            new_number = min(report.Header.EventNumber for report in later)
            asserts.assert_greater_equal(
                new_number, latest + 1,
                "Subsequent report did not include the new EventNumber")

        self.step(18, "With an active Event subscription from TH to DUT, trigger events on DUT multiple times.",
                  expectation="Verify that each event record is assigned a number that is exactly 1 greater than the last created event record on that Node. "
                  "Verify that for all the events that it has either System/Epoch timestamp or Delta System/Epoch Timestamp if this is not first event in current report.")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
        ) as (handler, sub):
            await self.emit_access_control_entry_changed(ctrl=th)
            await self.emit_access_control_entry_changed(ctrl=th)
            reports = self.collect_event_reports(handler, sub, mrp_timeout_sec, minimum=2)
            await self.verify_node_event_numbers(th, reports)

        self.step(19, "With an active Event subscription from TH to DUT, change attribute values on DUT to create events multiple times. "
                  "Perform a factory data reset on DUT, re-subscribe to events and continue to change attribute values on DUT to create events multiple times.",
                  expectation="Verify that each event record is assigned a number that is exactly 1 greater than the last created event record on that Node before factory data reset. "
                  "Verify that this monotonicity is not preserved across factory data reset.")
        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
        ) as (handler, sub):
            await self.emit_access_control_entry_changed(ctrl=th)
            await self.emit_access_control_entry_changed(ctrl=th)
            before_reset = self.collect_event_reports(handler, sub, mrp_timeout_sec, minimum=2)
            before_log = await self.verify_node_event_numbers(th, before_reset)
            last_before_reset = max(report.Header.EventNumber for report in before_log)

        await self.request_device_factory_reset()
        commissioned = await self.commission_devices()
        asserts.assert_true(commissioned, "Commissioning after factory reset failed")

        async with self.event_subscription(
            th,
            [(endpoint, EVENT, False)],
            event=EVENT,
            min_interval_sec=0,
            max_interval_sec=self.max_interval_ceiling_sec,
        ) as (handler, sub):
            await self.emit_access_control_entry_changed(ctrl=th)
            await self.emit_access_control_entry_changed(ctrl=th)
            after_reset = self.collect_event_reports(handler, sub, mrp_timeout_sec, minimum=2)
            after_log = await self.verify_node_event_numbers(th, after_reset)
            first_after_reset = min(report.Header.EventNumber for report in after_log)
            asserts.assert_less(
                first_after_reset, last_before_reset,
                f"Event numbers continued across factory reset ({first_after_reset} after {last_before_reset})")


if __name__ == "__main__":
    default_matter_test_main()
