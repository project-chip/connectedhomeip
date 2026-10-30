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
from matter.testing.decorators import has_cluster, pics, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# Script Function Call Example
# rm -rf admin_storage1.json && rm -rf kvs1 &&
# python3 ./scripts/tests/run_python_test.py --app out/linux-x64-all-devices-clang/all-devices-app
# --factory-reset --app-args "--device ambient-context-sensor:1 --KVS kvs1 --discriminator 1234 --app-pipe /tmp/asu_fifo"
# --script src/python_testing/TC_ASU_2_2.py
# --script-args "--storage-path admin_storage1.json --discriminator 1234 --passcode 20202021
#   --commissioning-method on-network --endpoint 1 --app-pipe /tmp/asu_fifo"


class TC_ASU_2_2(MatterTestCommissionedDevice):

    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_ambientsensing', False)

    @pics('ASU.S')
    @run_if_endpoint_matches(has_cluster(Clusters.AmbientSensingUnion))
    async def test_TC_ASU_2_2(self):
        """[TC-ASU-2.2] Event Reporting with DUT as a server"""
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientSensingUnion
        attr = Clusters.AmbientSensingUnion.Attributes

        self.step("1", "Commissioning, already done")

        self.step("2", "TH establishes a wildcard subscription to all attributes on Ambient Sensing Union Cluster on the endpoint under test.")
        attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
        await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        event_listener = EventSubscriptionHandler(expected_cluster=cluster)
        await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)

        # CI test values — contributor without name
        contnode_str = "0x123456789ABCDEF"
        contend = 1234
        contstatus = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline

        # CI test values — contributor with name
        named_contnode_str = "0xFEDCBA9876543210"
        named_contend = 5678
        named_contname = "PrimarySensor"
        named_contstatus = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline

        self.step("3", "Add new contributors to UnionContributorList attribute. In CI: add one Matter contributor without a name and one Matter contributor with a name.")
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

        self.step("4", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute. Verify that the new contributor(s) added to the UnionContributorList attribute is same as the one from step 3. For Matter contributors, ContributorName MAY be NULL or MAY contain a valid string.")
        subscription_reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(subscription_reports is not None and len(subscription_reports) > 0,
                            "No subscription report received for UnionContributorList after adding contributors.")
        reported_list = subscription_reports[-1].value
        asserts.assert_true(len(reported_list) > 0,
                            "UnionContributorList subscription report is empty after adding contributors.")

        # Step 3 added at least one contributor without a ContributorName (NULL) and one with a
        # ContributorName (non-empty string); verify both are reflected in the report.
        unnamed_count = 0
        named_count = 0
        for contributor in reported_list:
            if contributor.contributorNodeID == Clusters.Types.NullValue:
                continue
            if contributor.contributorName == Clusters.Types.NullValue:
                unnamed_count += 1
            else:
                asserts.assert_true(
                    isinstance(contributor.contributorName, str) and len(contributor.contributorName) > 0,
                    "ContributorName for a Matter contributor must be NULL or a non-empty string in UnionContributorList.")
                named_count += 1
        asserts.assert_true(
            unnamed_count > 0, "No contributor without a ContributorName (NULL) found in UnionContributorList.")
        asserts.assert_true(
            named_count > 0, "No contributor with a ContributorName found in UnionContributorList.")

        attrib_listener.reset()

        self.step("5", "TH receives UnionContributorAdded event and reads the AddedContributor field. Verify that the AddedContributor event field contains the same struct type data (ContributorNodeID, ContributorEndpointID, ContributorName, and ContributorStatus) as the added contributor(s) from step 3. For Matter contributors, ContributorName MAY be NULL or MAY contain a valid string.")
        # Step 3 added two contributors (one without and one with a ContributorName) in both CI and
        # manual runs; collect both events indexed by (ContributorNodeID, ContributorEndpointID) so
        # each can be matched against a distinct entry in step 4's UnionContributorList report.
        added_by_key = {}
        for _ in range(2):
            added_data = event_listener.wait_for_event_report(cluster.Events.UnionContributorAdded, timeout_sec=10.0)
            added_list = list(added_data.addedContributor)
            asserts.assert_true(len(added_list) > 0, "addedContributor field is empty in UnionContributorAdded event.")
            added = added_list[0]
            asserts.assert_false(added.contributorNodeID == Clusters.Types.NullValue,
                                 "ContributorNodeID should not be NULL for a Matter contributor.")
            asserts.assert_false(added.contributorEndpointID == Clusters.Types.NullValue,
                                 "ContributorEndpointID should not be NULL for a Matter contributor.")
            # Per spec, ContributorName MAY be NULL or a valid string for Matter contributors.
            asserts.assert_true(
                added.contributorName == Clusters.Types.NullValue or isinstance(added.contributorName, str),
                "ContributorName for a Matter contributor must be NULL or a valid string in UnionContributorAdded event.")

            # Verify the AddedContributor matches an entry found in the UnionContributorList report from step 4.
            match_found = any(
                contributor.contributorNodeID == added.contributorNodeID and
                contributor.contributorEndpointID == added.contributorEndpointID and
                contributor.contributorStatus == added.contributorStatus and
                contributor.contributorName == added.contributorName
                for contributor in reported_list
            )
            asserts.assert_true(
                match_found,
                "AddedContributor in UnionContributorAdded event does not match any entry in step 4's "
                "UnionContributorList subscription report.")

            added_by_key[(added.contributorNodeID, added.contributorEndpointID)] = added

        # Verify the two received events correspond to two distinct contributors, and that, together,
        # they cover both the unnamed (ContributorName == NULL) and the named (non-empty string)
        # contributors required to have been added in step 3.
        asserts.assert_equal(
            len(added_by_key), 2,
            "Expected two distinct UnionContributorAdded events (one per contributor added in step 3), "
            "but received duplicate or overlapping ContributorNodeID/ContributorEndpointID pairs.")
        unnamed_event_count = sum(
            1 for added in added_by_key.values() if added.contributorName == Clusters.Types.NullValue)
        named_event_count = sum(
            1 for added in added_by_key.values() if added.contributorName != Clusters.Types.NullValue)
        asserts.assert_true(
            unnamed_event_count > 0,
            "No UnionContributorAdded event received for a contributor without a ContributorName.")
        asserts.assert_true(
            named_event_count > 0,
            "No UnionContributorAdded event received for a contributor with a ContributorName.")

        event_listener.reset()

        self.step("6", "Remove one of existing contributors from UnionContributorList attribute.")
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
                prompt_msg="Remove the contributor without ContributorName added in step 3 from UnionContributorList, then type any letter and press ENTER.")

        self.step("7", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute. Verify that the removed contributor(s) from step 6 is not included in the UnionContributorList attribute.")
        subscription_reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(subscription_reports is not None and len(subscription_reports) > 0,
                            "No subscription report received for UnionContributorList after removing contributor.")
        list_after_remove = subscription_reports[-1].value
        asserts.assert_true(
            len(list_after_remove) < len(reported_list),
            "UnionContributorList did not shrink after removing a contributor.")
        attrib_listener.reset()

        self.step("8", "TH receives UnionContributorRemoved event and reads the RemovedContributor field. Verify that the RemovedContributor event field contains the same struct type data (ContributorNodeID, ContributorEndpointID, ContributorName, and ContributorStatus) as the removed contributor(s) from step 6. For Matter contributors, ContributorName MAY be NULL or MAY contain a valid string.")
        removed_data = event_listener.wait_for_event_report(cluster.Events.UnionContributorRemoved, timeout_sec=10.0)
        removed_list = list(removed_data.removedContributor)
        asserts.assert_true(len(removed_list) > 0, "removedContributor field is empty in UnionContributorRemoved event.")
        removed = removed_list[0]
        asserts.assert_false(removed.contributorNodeID == Clusters.Types.NullValue,
                             "ContributorNodeID should not be NULL in UnionContributorRemoved event.")
        asserts.assert_false(removed.contributorEndpointID == Clusters.Types.NullValue,
                             "ContributorEndpointID should not be NULL in UnionContributorRemoved event.")
        # Per spec, ContributorName MAY be NULL or MAY contain a valid string for a Matter contributor.
        asserts.assert_true(
            removed.contributorName == Clusters.Types.NullValue or isinstance(removed.contributorName, str),
            "ContributorName for a Matter contributor must be NULL or a valid string in UnionContributorRemoved event.")

        # Verify the removed contributor was present in step 4's report and is absent from step 7's report.
        was_present_before = any(
            contributor.contributorNodeID == removed.contributorNodeID and
            contributor.contributorEndpointID == removed.contributorEndpointID and
            contributor.contributorName == removed.contributorName and
            contributor.contributorStatus == removed.contributorStatus
            for contributor in reported_list
        )
        asserts.assert_true(
            was_present_before,
            "RemovedContributor in UnionContributorRemoved event was not present in step 4's "
            "UnionContributorList subscription report.")
        still_present_after = any(
            contributor.contributorNodeID == removed.contributorNodeID and
            contributor.contributorEndpointID == removed.contributorEndpointID
            for contributor in list_after_remove
        )
        asserts.assert_false(
            still_present_after,
            "RemovedContributor in UnionContributorRemoved event is still present in UnionContributorList "
            "after removal.")

        event_listener.reset()

        self.step("9", "Change the ContributorStatus value of one contributor from UnionContributorList attribute, and save its ContributorStatus value before the change.")
        current_status = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOffline

        # Capture every contributor's ContributorStatus before the change, for both CI and manual runs.
        # Key by (ContributorNodeID, ContributorEndpointID).
        unionlist_before = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.UnionContributorList)
        status_before = {
            (c.contributorNodeID, c.contributorEndpointID): c.contributorStatus
            for c in unionlist_before
            if c.contributorNodeID != Clusters.Types.NullValue
        }
        asserts.assert_true(len(status_before) > 0, "UnionContributorList is empty before the status change.")

        attrib_listener.reset()
        event_listener.reset()

        if self.is_ci:
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

        self.step("10", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute. Verify that the ContributorStatus value of the contributor changed from step 9 is updated accordingly.")
        subscription_reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(subscription_reports is not None and len(subscription_reports) > 0,
                            "No subscription report received for UnionContributorList after status change.")
        list_after_status = subscription_reports[-1].value
        asserts.assert_true(len(list_after_status) > 0,
                            "UnionContributorList subscription report is empty after status change.")
        status_after = {
            (c.contributorNodeID, c.contributorEndpointID): c.contributorStatus
            for c in list_after_status
            if c.contributorNodeID != Clusters.Types.NullValue
        }
        changed_contributors = {
            key for key, status in status_after.items()
            if key in status_before and status_before[key] != status
        }
        asserts.assert_true(
            len(changed_contributors) > 0,
            "No contributor's ContributorStatus changed in the UnionContributorList subscription report.")
        attrib_listener.reset()

        self.step("11", "TH receives UnionContributorStatusChanged event and reads the ContributorStatusChange field. Verify that the ContributorStatusChange field contains ContributorNodeID, ContributorEndpointID, ContributorName, PreviousContributorStatus, and CurrentContributorStatus and the field values match to the field value changes occurred in step 9.")
        changed_data = event_listener.wait_for_event_report(cluster.Events.UnionContributorStatusChanged, timeout_sec=10.0)
        changed_list = list(changed_data.contributorStatusChange)
        asserts.assert_true(len(changed_list) > 0, "contributorStatusChange field is empty in UnionContributorStatusChanged event.")
        changed = changed_list[0]
        asserts.assert_false(changed.contributorNodeID == Clusters.Types.NullValue,
                             "ContributorNodeID shall not be NULL for a Matter contributor status change.")
        asserts.assert_false(changed.contributorEndpointID == Clusters.Types.NullValue,
                             "ContributorEndpointID shall not be NULL for a Matter contributor status change.")
        asserts.assert_true(
            changed.previousContributorStatus != changed.currentContributorStatus,
            "PreviousContributorStatus and CurrentContributorStatus should differ in UnionContributorStatusChanged event.")

        # Verify the event matches the contributor's status before (step 9) and after (step 10) the change.
        changed_key = (changed.contributorNodeID, changed.contributorEndpointID)
        asserts.assert_in(
            changed_key, status_before,
            "(ContributorNodeID, ContributorEndpointID) in UnionContributorStatusChanged event was not found "
            "in the UnionContributorList snapshot taken before the status change in step 9.")
        asserts.assert_equal(
            changed.previousContributorStatus, status_before[changed_key],
            "PreviousContributorStatus in UnionContributorStatusChanged event does not match the "
            "ContributorStatus observed before the change in step 9.")
        asserts.assert_in(
            changed_key, status_after,
            "(ContributorNodeID, ContributorEndpointID) in UnionContributorStatusChanged event was not found "
            "in step 10's UnionContributorList subscription report.")
        asserts.assert_equal(
            changed.currentContributorStatus, status_after[changed_key],
            "CurrentContributorStatus in UnionContributorStatusChanged event does not match the "
            "ContributorStatus observed in step 10's UnionContributorList subscription report.")

        event_listener.reset()


if __name__ == "__main__":
    default_matter_test_main()
