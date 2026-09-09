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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device ambient-context-sensor:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/asu_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --app-pipe /tmp/asu_fifo
#       --bool-arg simulate_ambientsensing:true
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Script Function Call Example
# rm -rf admin_storage1.json && rm -rf kvs1 &&
# python3 ./scripts/tests/run_python_test.py --app out/linux-x64-all-devices-clang/all-devices-app
# --factory-reset --app-args "--device ambient-context-sensor:1 --KVS kvs1 --discriminator 1234 --app-pipe /tmp/asu_fifo"
# --script src/python_testing/TC_ASU_2_2.py
# --script-args "--storage-path admin_storage1.json --discriminator 1234 --passcode 20202021
#   --commissioning-method on-network --endpoint 1 --app-pipe /tmp/asu_fifo"


class TC_ASU_2_2(MatterBaseTest):
    def desc_TC_ASU_2_2(self) -> str:
        return "[TC-ASU-2.2] Event Reporting with DUT as a server"

    def pics_TC_ASU_2_2(self):
        return ["ASU.S"]

    def steps_TC_ASU_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH establishes a wildcard subscription to all attributes on Ambient Sensing Union Cluster on the endpoint under test."),
            TestStep("3", "Add new contributors to UnionContributorList attribute. "
                     "In CI: add one Matter contributor without a name and one Matter contributor with a name."),
            TestStep("4", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute.",
                     "Verify that the new contributor(s) added to the UnionContributorList attribute is same as the one from step 3. "
                     "For Matter contributors, ContributorName MAY be NULL or MAY contain a valid string."),
            TestStep("5", "TH receives UnionContributorAdded event and reads the AddedContributor field.",
                     "Verify that the AddedContributor event field contains the same struct type data (ContributorNodeID, ContributorEndpointID, ContributorName, and ContributorStatus) as the added contributor(s) from step 3. "
                     "For Matter contributors, ContributorName MAY be NULL or MAY contain a valid string."),
            TestStep("6", "Remove one of existing contributors from UnionContributorList attribute."),
            TestStep("7", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute.",
                     "Verify that the removed contributor(s) from step 6 is not included in the UnionContributorList attribute."),
            TestStep("8", "TH receives UnionContributorRemoved event and reads the RemovedContributor field.",
                     "Verify that the RemovedContributor event field contains the same struct type data (ContributorNodeID, ContributorEndpointID, ContributorName, and ContributorStatus) as the removed contributor(s) from step 6. "
                     "For Matter contributors, ContributorName MAY be NULL or MAY contain a valid string."),
            TestStep("9", "Change the ContributorStatus value of one contributor from UnionContributorList attribute, and save the contributor's list index value and the ContributorStatus value before the change."),
            TestStep("10", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute.",
                     "Verify that the ContributorStatus value of the contributor changed from step 9 is updated accordingly."),
            TestStep("11", "TH receives UnionContributorStatusChanged event and reads the ContributorStatusChange field.",
                     "Verify that the ContributorStatusChange field contains ContributorNodeID, ContributorEndpointID, ContributorName, PreviousContributorStatus, and CurrentContributorStatus and the field values match to the field value changes occurred in step 9.")
        ]

    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_ambientsensing', False)

    @run_if_endpoint_matches(has_cluster(Clusters.AmbientSensingUnion))
    async def test_TC_ASU_2_2(self):
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientSensingUnion
        attr = Clusters.AmbientSensingUnion.Attributes

        self.step("1")

        self.step("2")
        attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
        await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        event_listener = EventSubscriptionHandler(expected_cluster=cluster)
        await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)

        # CI test values — contributor without name
        contnode_str = "0x123456789ABCDEF"
        contnode = int(contnode_str, 16)
        contend = 1234
        contstatus = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline

        # CI test values — contributor with name
        named_contnode_str = "0xFEDCBA9876543210"
        named_contnode = int(named_contnode_str, 16)
        named_contend = 5678
        named_contname = "PrimarySensor"
        named_contstatus = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline

        self.step("3")
        if self.is_ci:
            # Add a Matter contributor without a name (ContributorName will be NULL)
            self.write_to_app_pipe({
                "Name": "AddAmbientSensingContributor",
                "EndpointId": endpoint,
                "NodeId": contnode_str,
                "ContributorEndpointId": contend,
                "Status": contstatus.value,
            })
            await asyncio.sleep(1)
            # Add a Matter contributor with a name
            self.write_to_app_pipe({
                "Name": "AddAmbientSensingContributor",
                "EndpointId": endpoint,
                "NodeId": named_contnode_str,
                "ContributorEndpointId": named_contend,
                "Status": named_contstatus.value,
                "ContributorName": named_contname,
            })
            await asyncio.sleep(1)
        else:
            self.wait_for_user_input(
                prompt_msg="Add contributors to UnionContributorList (including at least one with and one without a ContributorName), then type any letter and press ENTER.")

        self.step("4")
        subscription_reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(subscription_reports is not None and len(subscription_reports) > 0,
                            "No subscription report received for UnionContributorList after adding contributors.")
        reported_list = subscription_reports[-1].value

        unnamed_found = False
        named_found = False
        for contributor in reported_list:
            if contributor.contributorNodeID == Clusters.Types.NullValue:
                continue
            if contributor.contributorNodeID == contnode:
                asserts.assert_equal(contributor.contributorEndpointID, contend,
                                     "ContributorEndpointID does not match the unnamed contributor.")
                asserts.assert_equal(contributor.contributorStatus, contstatus,
                                     "ContributorStatus does not match the unnamed contributor.")
                # Per spec, ContributorName MAY be NULL or MAY contain a valid string for a Matter contributor.
                asserts.assert_true(
                    contributor.contributorName == Clusters.Types.NullValue or isinstance(contributor.contributorName, str),
                    "ContributorName for a Matter contributor must be NULL or a valid string in UnionContributorList.")
                unnamed_found = True
            elif contributor.contributorNodeID == named_contnode:
                asserts.assert_equal(contributor.contributorEndpointID, named_contend,
                                     "ContributorEndpointID does not match the named contributor.")
                asserts.assert_equal(contributor.contributorStatus, named_contstatus,
                                     "ContributorStatus does not match the named contributor.")
                asserts.assert_true(
                    contributor.contributorName != Clusters.Types.NullValue and contributor.contributorName == named_contname,
                    f"ContributorName in UnionContributorList does not match '{named_contname}'.")
                named_found = True

        if self.is_ci:
            asserts.assert_true(unnamed_found, "Unnamed contributor not found in UnionContributorList subscription report.")
            asserts.assert_true(named_found, "Named contributor not found in UnionContributorList subscription report.")

        attrib_listener.reset()

        self.step("5")
        # Collect all UnionContributorAdded events and index them by contributor NodeID.
        # In CI, two contributors were added, so both NodeIDs must be present.
        # Drain all queued events by polling until the queue is empty.
        all_added_events = []
        while True:
            try:
                ev = event_listener.get_event_from_queue(block=False, timeout=0)
                all_added_events.append(ev)
            except Exception:
                break
        if not all_added_events:
            last = event_listener.get_last_event()
            if last is not None:
                all_added_events.append(last)

        added_by_node = {}
        for event in all_added_events:
            if event is None:
                continue
            asserts.assert_equal(event.Header.EventId, cluster.Events.UnionContributorAdded.event_id,
                                 f"Wrong event ID: got {event.Header.EventId}, expected UnionContributorAdded.")
            added_list = list(event.Data.addedContributor)
            asserts.assert_true(len(added_list) > 0, "addedContributor field is empty in UnionContributorAdded event.")
            added = added_list[0]
            asserts.assert_false(added.contributorNodeID == Clusters.Types.NullValue,
                                 "ContributorNodeID should not be NULL for a Matter contributor.")
            asserts.assert_false(added.contributorEndpointID == Clusters.Types.NullValue,
                                 "ContributorEndpointID should not be NULL for a Matter contributor.")
            added_by_node[added.contributorNodeID] = added

        if self.is_ci:
            asserts.assert_in(contnode, added_by_node,
                              f"No UnionContributorAdded event received for unnamed contributor (NodeID={contnode_str}).")
            asserts.assert_in(named_contnode, added_by_node,
                              f"No UnionContributorAdded event received for named contributor (NodeID={named_contnode_str}).")

            # Validate unnamed contributor fields.
            unnamed = added_by_node[contnode]
            asserts.assert_equal(unnamed.contributorEndpointID, contend,
                                 "ContributorEndpointID does not match for unnamed contributor in UnionContributorAdded event.")
            asserts.assert_equal(unnamed.contributorStatus, contstatus,
                                 "ContributorStatus does not match for unnamed contributor in UnionContributorAdded event.")
            # Per spec, ContributorName MAY be NULL or a valid string for Matter contributors.
            asserts.assert_true(
                unnamed.contributorName == Clusters.Types.NullValue or isinstance(unnamed.contributorName, str),
                "ContributorName for unnamed Matter contributor must be NULL or a valid string in UnionContributorAdded event.")

            # Validate named contributor fields.
            named = added_by_node[named_contnode]
            asserts.assert_equal(named.contributorEndpointID, named_contend,
                                 "ContributorEndpointID does not match for named contributor in UnionContributorAdded event.")
            asserts.assert_equal(named.contributorStatus, named_contstatus,
                                 "ContributorStatus does not match for named contributor in UnionContributorAdded event.")
            asserts.assert_true(
                named.contributorName != Clusters.Types.NullValue and named.contributorName == named_contname,
                f"ContributorName in UnionContributorAdded event does not match '{named_contname}'.")

        event_listener.reset()

        self.step("6")
        if self.is_ci:
            self.write_to_app_pipe({
                "Name": "RemoveAmbientSensingContributor",
                "EndpointId": endpoint,
                "NodeId": contnode_str,
                "ContributorEndpointId": contend,
            })
            await asyncio.sleep(1)
        else:
            self.wait_for_user_input(
                prompt_msg="Remove a contributor from UnionContributorList, then type any letter and press ENTER.")

        self.step("7")
        subscription_reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(subscription_reports is not None and len(subscription_reports) > 0,
                            "No subscription report received for UnionContributorList after removing contributor.")
        reported_list = subscription_reports[-1].value

        for contributor in reported_list:
            if contributor.contributorNodeID != Clusters.Types.NullValue and contributor.contributorNodeID == contnode:
                asserts.fail(
                    f"Removed contributor (NodeID={contnode_str}) is still found in UnionContributorList subscription report.")

        attrib_listener.reset()

        self.step("8")
        event = event_listener.get_last_event()
        asserts.assert_is_not_none(event, "No UnionContributorRemoved event received.")
        asserts.assert_equal(event.Header.EventId, cluster.Events.UnionContributorRemoved.event_id,
                             f"Wrong event ID: got {event.Header.EventId}, expected UnionContributorRemoved.")

        removed_list = list(event.Data.removedContributor)
        asserts.assert_true(len(removed_list) > 0, "removedContributor field is empty in UnionContributorRemoved event.")
        removed = removed_list[0]
        asserts.assert_equal(removed.contributorNodeID, contnode, "Wrong ContributorNodeID in UnionContributorRemoved event.")
        asserts.assert_equal(removed.contributorEndpointID, contend,
                             "Wrong ContributorEndpointID in UnionContributorRemoved event.")
        # Per spec, ContributorName MAY be NULL or MAY contain a valid string for a Matter contributor.
        asserts.assert_true(
            removed.contributorName == Clusters.Types.NullValue or isinstance(removed.contributorName, str),
            "ContributorName for a Matter contributor must be NULL or a valid string in UnionContributorRemoved event.")
        asserts.assert_equal(removed.contributorStatus, contstatus, "Wrong ContributorStatus in UnionContributorRemoved event.")
        event_listener.reset()

        self.step("9")
        prev_status = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline
        current_status = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOffline

        if self.is_ci:
            # Verify the named contributor is still present before triggering the status change.
            unionlist_before = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.UnionContributorList)
            found_before = any(
                c.contributorNodeID != Clusters.Types.NullValue and c.contributorNodeID == named_contnode
                for c in unionlist_before
            )
            asserts.assert_true(found_before, "Could not find named contributor in UnionContributorList before status change.")
            attrib_listener.reset()
            event_listener.reset()

            self.write_to_app_pipe({
                "Name": "UpdateAmbientSensingContributorStatus",
                "EndpointId": endpoint,
                "NodeId": named_contnode_str,
                "ContributorEndpointId": named_contend,
                "Status": current_status.value,
            })
            await asyncio.sleep(1)
        else:
            self.wait_for_user_input(
                prompt_msg="Change the contributor's ContributorStatus of the one with ContributorName added in previous step in UnionContributorList, then type any letter and press ENTER.")

        self.step("10")
        subscription_reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(subscription_reports is not None and len(subscription_reports) > 0,
                            "No subscription report received for UnionContributorList after status change.")
        reported_list = subscription_reports[-1].value

        found_updated = False
        for contributor in reported_list:
            if contributor.contributorNodeID != Clusters.Types.NullValue and contributor.contributorNodeID == named_contnode:
                asserts.assert_equal(contributor.contributorStatus, current_status,
                                     "ContributorStatus was not updated in UnionContributorList subscription report.")
                found_updated = True

        asserts.assert_true(
            found_updated, "Could not find contributor in UnionContributorList subscription report to verify status change.")
        attrib_listener.reset()

        self.step("11")
        event = event_listener.get_last_event()
        asserts.assert_is_not_none(event, "No UnionContributorStatusChanged event received.")
        asserts.assert_equal(event.Header.EventId, cluster.Events.UnionContributorStatusChanged.event_id,
                             f"Wrong event ID: got {event.Header.EventId}, expected UnionContributorStatusChanged.")

        changed_list = list(event.Data.contributorStatusChange)
        asserts.assert_true(len(changed_list) > 0, "contributorStatusChange field is empty in UnionContributorStatusChanged event.")
        changed = changed_list[0]
        # ContributorNodeID and ContributorEndpointID identify the contributor that changed status.
        asserts.assert_false(changed.contributorNodeID == Clusters.Types.NullValue,
                             "ContributorNodeID shall not be NULL for a Matter contributor status change.")
        asserts.assert_equal(changed.contributorNodeID, named_contnode,
                             "Wrong ContributorNodeID in UnionContributorStatusChanged event.")
        asserts.assert_false(changed.contributorEndpointID == Clusters.Types.NullValue,
                             "ContributorEndpointID shall not be NULL for a Matter contributor status change.")
        asserts.assert_equal(changed.contributorEndpointID, named_contend,
                             "Wrong ContributorEndpointID in UnionContributorStatusChanged event.")
        asserts.assert_equal(changed.previousContributorStatus, prev_status,
                             "Wrong PreviousContributorStatus in UnionContributorStatusChanged event.")
        asserts.assert_equal(changed.currentContributorStatus, current_status,
                             "Wrong CurrentContributorStatus in UnionContributorStatusChanged event.")
        event_listener.reset()


if __name__ == "__main__":
    default_matter_test_main()
