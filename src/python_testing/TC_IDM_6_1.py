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
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main

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


class TC_IDM_6_1(IDMBaseTest):
    @pics("MCORE.IDM.S")
    @async_test_body
    async def test_TC_IDM_6_1(self) -> None:
        endpoint = self.ROOT_NODE_ENDPOINT_ID

        self.step(0, "Commissioning, already done", is_commissioning=True)
        await self.setup_class_helper(allow_pase=False)

        # Guarantee the DUT holds at least one readable event before the path shapes below
        # are exercised, rather than relying on the events commissioning happened to leave.
        await self.emit_access_control_entry_changed(ctrl=self.default_controller)

        paths = self.wildcard_event_paths(endpoint=endpoint, cluster=EVENT_CLUSTER, event=EVENT)

        self.step(1, "TH sends Read Request Message to DUT with EventRequests set to a specific event from a specific "
                     "cluster on a specific endpoint on a specific node that is, [Node = Specific, Endpoint = Specific, "
                     "Cluster = Specific, Event = Specific].",
                  expectation="Verify TH receives Report Data Message with the data for specific event in Read Request Message.")
        events = await self.read_events(ctrl=self.default_controller, events=paths.specific_endpoint_specific_event)
        self.assert_event_reported(events=events, cluster=EVENT_CLUSTER, event=EVENT, endpoint=endpoint)

        self.step(2, "TH sends Read Request Message to DUT with EventRequests set to all events from a specific cluster "
                     "on a specific endpoint on a specific node that is, [Node = Specific, Endpoint = Specific, "
                     "Cluster = Specific, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with the data for events in Read Request Message.")
        events = await self.read_events(ctrl=self.default_controller, events=paths.specific_endpoint_all_cluster_events)
        self.assert_event_reported(events=events, cluster=EVENT_CLUSTER, event=EVENT, endpoint=endpoint)

        self.step(3, "TH sends Read Request Message to DUT with EventRequests set to all events from all clusters on a "
                     "specific endpoint on a specific node that is, [Node = Specific, Endpoint = Specific, "
                     "Cluster = Wildcard, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with the data for events in Read Request Message.")
        events = await self.read_events(ctrl=self.default_controller, events=paths.specific_endpoint_all_events)
        self.assert_event_reported(events=events, cluster=EVENT_CLUSTER, event=EVENT, endpoint=endpoint)

        self.step(4, "TH sends Read Request Message to DUT with EventRequests set to a specific event from a specific "
                     "cluster on all endpoints on a specific node that is, [Node = Specific, Endpoint = Wildcard, "
                     "Cluster = Specific, Event = Specific].",
                  expectation="Verify TH receives Report Data Message with the data for specific event in Read Request Message.")
        events = await self.read_events(ctrl=self.default_controller, events=paths.all_endpoints_specific_event)
        self.assert_event_reported(events=events, cluster=EVENT_CLUSTER, event=EVENT)

        self.step(5, "TH sends Read Request Message to DUT with EventRequests set to all events from a specific cluster "
                     "on all endpoints on a specific node that is, [Node = Specific, Endpoint = Wildcard, "
                     "Cluster = Specific, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with the data for events in Read Request Message.")
        events = await self.read_events(ctrl=self.default_controller, events=paths.all_endpoints_all_cluster_events)
        self.assert_event_reported(events=events, cluster=EVENT_CLUSTER, event=EVENT)

        self.step(6, "TH sends Read Request Message to DUT with EventRequests set to all events from all clusters on "
                     "all endpoints on a specific node that is, [Node = Specific, Endpoint = Wildcard, "
                     "Cluster = Wildcard, Event = Wildcard].",
                  expectation="Verify TH receives Report Data Message with the data for events in Read Request Message.")
        # The path names nothing to check a reported event against, so any event satisfies it.
        events = await self.read_events(ctrl=self.default_controller, events=paths.all_endpoints_all_events)
        self.assert_events_reported(events=events)

        self.step(7, "TH sends Read Request Message to DUT with EventRequests set to path which indicates a cluster "
                     "event that is not supported.",
                  expectation="Verify TH receives Report Data Message for the event with EventStatusIB generated with "
                              "the UNSUPPORTED_EVENT Status Code.")
        # Not automated. Naming an unsupported event requires an event path carrying an event id
        # the DUT does not implement, and ReadEvent only accepts paths built from generated
        # cluster objects, all of which name events that do exist. The certification test
        # Test_TC_IDM_6_1.yaml marks this same step disabled as out of scope.
        self.mark_current_step_skipped()

        self.step(8, "TH sends Read Request Message to DUT with EventRequests set to path which requires a privilege "
                     "that is not granted for the cluster in the path.",
                  expectation="Verify TH receives Report Data Message for the event with EventStatusIB generated with "
                              "the UNSUPPORTED_ACCESS Status Code 0x7e.")
        view_privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView
        async with self.restricted_privilege_controller(cluster_id=UNPRIVILEGED_CLUSTER.id,
                                                        privilege=view_privilege) as restricted_ctrl:
            # The restricted controller holds view on BasicInformation only, so a concrete path
            # naming an AccessControl event, which requires administer, is refused by name.
            events = await self.read_events(ctrl=restricted_ctrl, events=paths.specific_endpoint_specific_event)
            self.assert_event_status_count(events=events, status=Status.UnsupportedAccess, expected_count=1)

            self.step(9, "TH sends Read Request Message to DUT with EventRequests set to Wildcard path where reading an "
                         "event in the path requires a privilege that is not granted for the cluster in the path.",
                      expectation="Verify TH receives Report Data Message with no entry for that event in EventReports list.")
            # The same denial expands differently under a wildcard: the path the controller may
            # not read is skipped outright rather than reported with a status.
            events = await self.read_events(ctrl=restricted_ctrl, events=paths.all_endpoints_all_events)
            self.assert_cluster_events_absent(events=events, cluster=EVENT_CLUSTER)
            self.assert_event_status_count(events=events, status=Status.UnsupportedAccess, expected_count=0)

        self.step(10, "TH sends a Read Request Message to the DUT to read back events. Note down the largest event "
                      "number received in the report data message sent back from the DUT. TH sends Read Request Message "
                      "to DUT with the EventMin field set to a number less than what was received in the previous step.",
                  expectation="Verify TH receives Report Data Message with EventReports for event numbers higher than or "
                              "equal to the EventMin field.")
        # A second event guarantees two distinct event numbers, so an EventMin strictly below the
        # largest one still selects a non-empty set.
        await self.emit_access_control_entry_changed(ctrl=self.default_controller)
        latest_event_number = await self.read_latest_event_number(ctrl=self.default_controller,
                                                                 events=paths.all_endpoints_all_events)
        asserts.assert_is_not_none(latest_event_number, "DUT reported no events to derive an EventMin from")
        asserts.assert_greater(latest_event_number, 0, "DUT must report an event number above zero to test EventMin below it")
        event_min = latest_event_number - 1
        events = await self.read_events(ctrl=self.default_controller, events=paths.all_endpoints_all_events,
                                       event_number_filter=event_min)
        self.assert_events_reported(events=events)
        self.assert_event_numbers_at_least(events=events, minimum=event_min)

        self.step(11, "TH sends a Read Request Message to the DUT to read back events. Note down the largest event "
                      "number received in the report data message sent back from the DUT. TH sends Read Request Message "
                      "to DUT with the EventMin field set to a number much greater than what was received in the "
                      "previous step.",
                  expectation="Verify TH receives Report Data Message with empty EventReports, or if Events are present, "
                              "then verify that it contains EventReports with event numbers bigger than the one provided.")
        event_min = latest_event_number + EVENT_MIN_BEYOND_LATEST
        events = await self.read_events(ctrl=self.default_controller, events=paths.all_endpoints_all_events,
                                       event_number_filter=event_min)
        self.assert_event_numbers_at_least(events=events, minimum=event_min)

        self.step(12, "[Testing Chunked Event Data] TH sends Read Request Message to DUT with EventRequests set to all "
                      "events so as to create a very large event data. For every chunked data message received, DUT "
                      "sends a status response.",
                  expectation="Verify on the TH that the DUT sends a status response back to the TH on receipt of the "
                              "report data action for every chunked message. The last chunked message should not receive "
                              "any status response from the DUT.")
        # Not automated. This step is stated in terms of individual report data messages and the
        # status response sent for each, but the controller reassembles a chunked report before
        # returning it and exposes no chunk boundaries, so neither the per-chunk status responses
        # nor their absence on the final chunk is observable from a test script. Message-level
        # coverage of chunked event reports lives in src/controller/tests/TestEventChunking.cpp.
        self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
