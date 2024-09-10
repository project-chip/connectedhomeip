#
#    Copyright (c) 2022 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import math
import queue
import random
import string
import time
from typing import Any, Dict, List, Set

import chip.clusters as Clusters
from chip.interaction_model import Status as StatusEnum
from chip.utils import CommissioningBuildingBlocks
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts
from TC_SC_3_6 import AttributeChangeAccumulator, ResubscriptionCatcher

# TODO: Overall, we need to add validation that session IDs have not changed throughout to be agnostic
#       to some internal behavior assumptions of the SDK we are making relative to the write to
#       the trigger the subscriptions not re-opening a new CASE session
#


def generate_controller_name(fabric_index: int, controller_index: int):
    return f"RD{fabric_index}{string.ascii_uppercase[controller_index]}"


class TC_RR_1_1(MatterBaseTest):
    def setup_class(self):
        self._pseudo_random_generator = random.Random(1234)
        self._subscriptions = []

    def teardown_class(self):
        logging.info("Teardown: shutting down all subscription to avoid racy callbacks")
        for subscription in self._subscriptions:
            subscription.Shutdown()

    @async_test_body
    async def test_TC_RR_1_1(self):
        dev_ctrl = self.default_controller

        # Debug/test arguments

        # Get overrides for debugging the test
        num_fabrics_to_commission = self.user_params.get("num_fabrics_to_commission", 5)
        num_controllers_per_fabric = self.user_params.get("num_controllers_per_fabric", 3)
        # Immediate reporting
        min_report_interval_sec = self.user_params.get("min_report_interval_sec", 0)
        # 10 minutes max reporting interval --> We don't care about keep-alives per-se and
        # want to avoid resubscriptions
        max_report_interval_sec = self.user_params.get("max_report_interval_sec", 10 * 60)
        # Time to wait after changing NodeLabel for subscriptions to all hit. This is dependant
        # on MRP params of subscriber and on actual min_report_interval.
        # TODO: Determine the correct max value depending on target. Test plan doesn't say!
        timeout_delay_sec = self.user_params.get("timeout_delay_sec", max_report_interval_sec * 2)
        # Whether to skip filling the UserLabel clusters
        skip_user_label_cluster_steps = self.user_params.get("skip_user_label_cluster_steps", False)
        # Whether to do the local session ID comparison checks to prove new sessions have not been established.
        check_local_session_id_unchanged = self.user_params.get("check_local_session_id_unchanged", False)
        # Whether to check heap statistics. Add `--bool-arg check_heap_watermarks:true` to command line to enable
        check_heap_watermarks = self.user_params.get("check_heap_watermarks", False)

        BEFORE_LABEL = "Before Subscriptions 12345678912"
        AFTER_LABEL = "After Subscriptions 123456789123"

        # Pre-conditions

        # Do a read-out of heap statistics before the test begins
        if check_heap_watermarks:
            logging.info("Read Heap info before stress test")
            high_watermark_before, current_usage_before = await self.read_heap_statistics(dev_ctrl)

        # Make sure all certificates are installed with maximal size
        dev_ctrl.fabricAdmin.certificateAuthority.maximizeCertChains = True

        # TODO: Do from PICS list. The reflection approach here what a real client would do,
        #       and it respects what the test says: "TH writes 4 entries per endpoint where LabelList is supported"
        logging.info("Pre-condition: determine whether any endpoints have UserLabel cluster (ULABEL.S.A0000(LabelList))")
        endpoints_with_user_label_list = await dev_ctrl.ReadAttribute(self.dut_node_id, [Clusters.UserLabel.Attributes.LabelList])
        has_user_labels = len(endpoints_with_user_label_list) > 0
        if has_user_labels:
            logging.info("--> User label cluster present on endpoints %s" %
                         ", ".join(["%d" % ep for ep in endpoints_with_user_label_list.keys()]))
        else:
            logging.info("--> User label cluster not present on any endpoitns")

        # Generate list of all clients names
        client_list = []

        # TODO: Shall we also verify SupportedFabrics attribute, and the CapabilityMinima attribute?
        logging.info("Pre-conditions: validate CapabilityMinima.CaseSessionsPerFabric >= 3")

        capability_minima = await self.read_single_attribute(dev_ctrl,
                                                             node_id=self.dut_node_id,
                                                             endpoint=0,
                                                             attribute=Clusters.BasicInformation.Attributes.CapabilityMinima)
        asserts.assert_greater_equal(capability_minima.caseSessionsPerFabric, 3)

        # Step 1: Commission 5 fabrics with maximized NOC chains. 1a and 1b have already been completed at this time.
        logging.info(f"Step 1: use existing fabric to configure new fabrics so that total is {num_fabrics_to_commission} fabrics")

        # Generate Node IDs for subsequent controllers start at 200, follow 200, 300, ...
        node_ids = [200 + (i * 100) for i in range(num_controllers_per_fabric - 1)]

        # Prepare clients for first fabric, that includes the default controller
        fabric_index = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, dev_ctrl=dev_ctrl)
        dev_ctrl.name = generate_controller_name(fabric_index, 0)
        client_list.append(dev_ctrl)

        if num_controllers_per_fabric > 1:
            new_controllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(
                fabricAdmin=dev_ctrl.fabricAdmin,
                adminDevCtrl=dev_ctrl,
                controllerNodeIds=node_ids,
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                targetNodeId=self.dut_node_id, catTags=[0x0001_0001]
            )
            for idx, controller in enumerate(new_controllers):
                controller.name = generate_controller_name(fabric_index, idx+1)
            client_list.extend(new_controllers)

        # Step 1c - Ensure there are no leftover fabrics from another process.
        commissioned_fabric_count: int = await self.read_single_attribute(
            dev_ctrl, node_id=self.dut_node_id,
            endpoint=0, attribute=Clusters.OperationalCredentials.Attributes.CommissionedFabrics)

        # Insert a fabric to self-test the next step.
        # This is not hidden behind a flag to avoid potential undetected bugs.
        if commissioned_fabric_count == 1:
            logging.info("Commissioning fabric for TH test.")
            new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
            new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)

            new_admin_ctrl = new_fabric_admin.NewController(nodeId=dev_ctrl.nodeId, catTags=[0x0001_0001])
            new_admin_ctrl.name = "THTF"
            await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
                commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=new_admin_ctrl,
                existingNodeId=self.dut_node_id, newNodeId=self.dut_node_id)

        commissioned_fabric_count = await self.read_single_attribute(
            dev_ctrl, node_id=self.dut_node_id,
            endpoint=0, attribute=Clusters.OperationalCredentials.Attributes.CommissionedFabrics)
        asserts.assert_not_equal(commissioned_fabric_count, 1, "TH Error: failed to add fabric for testing TH.")

        # Step 1c - perform removal.
        if commissioned_fabric_count > 1:
            logging.info("Removing extra fabrics from device.")
            fabrics: List[Clusters.OperationalCredentials.Structs.FabricDescriptorStruct] = await self.read_single_attribute(
                dev_ctrl, node_id=self.dut_node_id, endpoint=0,
                attribute=Clusters.OperationalCredentials.Attributes.Fabrics, fabricFiltered=False)
            current_fabric_index = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)
            for fabric in fabrics:
                if fabric.fabricIndex == current_fabric_index:
                    continue
                # This is not the test client's fabric, so remove it.
                await dev_ctrl.SendCommand(
                    self.dut_node_id, 0, Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=fabric.fabricIndex))

        commissioned_fabric_count = await self.read_single_attribute(
            dev_ctrl, node_id=self.dut_node_id,
            endpoint=0, attribute=Clusters.OperationalCredentials.Attributes.CommissionedFabrics)
        asserts.assert_equal(commissioned_fabric_count, 1, "Failed to remove extra fabrics from DUT.")

        # Prepare clients for subsequent fabrics (step 1d)
        for i in range(num_fabrics_to_commission - 1):
            admin_index = 2 + i
            logging.info("Commissioning fabric %d/%d" % (admin_index, num_fabrics_to_commission))
            new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
            new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=admin_index)

            new_admin_ctrl = new_fabric_admin.NewController(nodeId=dev_ctrl.nodeId, catTags=[0x0001_0001])
            await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(commissionerDevCtrl=dev_ctrl,
                                                                             newFabricDevCtrl=new_admin_ctrl,
                                                                             existingNodeId=self.dut_node_id,
                                                                             newNodeId=self.dut_node_id)
            fabric_index = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, dev_ctrl=new_admin_ctrl)
            new_admin_ctrl.name = generate_controller_name(fabric_index, 0)
            client_list.append(new_admin_ctrl)
            if num_controllers_per_fabric > 1:
                new_controllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(
                    fabricAdmin=new_fabric_admin,
                    adminDevCtrl=new_admin_ctrl,
                    controllerNodeIds=node_ids,
                    privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                    targetNodeId=self.dut_node_id,
                    catTags=[0x0001_0001]
                )
                for idx, controller in enumerate(new_controllers):
                    controller.name = generate_controller_name(fabric_index, idx+1)

                client_list.extend(new_controllers)

        asserts.assert_equal(len(client_list), num_fabrics_to_commission *
                             num_controllers_per_fabric, "Must have the right number of clients")

        commissioned_fabric_count = await self.read_single_attribute(
            dev_ctrl, node_id=self.dut_node_id,
            endpoint=0, attribute=Clusters.OperationalCredentials.Attributes.CommissionedFabrics)
        asserts.assert_equal(commissioned_fabric_count, num_fabrics_to_commission,
                             "Must have the right number of fabrics commissioned.")
        fabric_table: List[Clusters.OperationalCredentials.Structs.FabricDescriptorStruct] = await self.read_single_attribute(
            dev_ctrl, node_id=self.dut_node_id,
            endpoint=0, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, fabricFiltered=False)

        client_by_name = {client.name: client for client in client_list}
        local_session_id_by_client_name = {client.name: client.GetConnectedDeviceSync(
            self.dut_node_id).localSessionId for client in client_list}

        # Step 2: Set the Label field for each fabric and BasicInformation.NodeLabel to 32 characters
        logging.info("Step 2: Setting the Label field for each fabric and BasicInformation.NodeLabel to 32 characters")

        for fabric in fabric_table:
            client_name = generate_controller_name(fabric.fabricIndex, 0)
            client = client_by_name[client_name]

            # Send the UpdateLabel command
            label = ("%d" % fabric.fabricIndex) * 32
            logging.info("Step 2a: Setting fabric label on fabric %d to '%s' using client %s" %
                         (fabric.fabricIndex, label, client_name))
            await client.SendCommand(self.dut_node_id, 0, Clusters.OperationalCredentials.Commands.UpdateFabricLabel(label))

            # Read back
            fabric_metadata = await self.read_single_attribute(client,
                                                               node_id=self.dut_node_id,
                                                               endpoint=0,
                                                               attribute=Clusters.OperationalCredentials.Attributes.Fabrics)
            print(fabric_metadata)
            asserts.assert_equal(fabric_metadata[0].label, label, "Fabrics[x].label must match what was written")

        # Before subscribing, set the NodeLabel to "Before Subscriptions"
        logging.info(f"Step 2b: Set BasicInformation.NodeLabel to {BEFORE_LABEL}")
        await client_list[0].WriteAttribute(self.dut_node_id,
                                            [(0, Clusters.BasicInformation.Attributes.NodeLabel(value=BEFORE_LABEL))])

        node_label = await self.read_single_attribute(client,
                                                      node_id=self.dut_node_id,
                                                      endpoint=0,
                                                      attribute=Clusters.BasicInformation.Attributes.NodeLabel)
        asserts.assert_equal(node_label, BEFORE_LABEL, "NodeLabel must match what was written")

        # Step 3: Add 4 Access Control entries on DUT with a list of 4 Subjects and 3 Targets with the following parameters (...)
        await self.send_acl(test_step=3, client_by_name=client_by_name, enable_access_to_group_cluster=False, fabric_table=fabric_table)

        # Step 4 and 5 (the operations cannot be separated): establish all CASE sessions and subscriptions

        # Subscribe with all clients to NodeLabel attribute and 2 more paths
        sub_handlers = []
        resub_catchers = []
        output_queue = queue.Queue()
        subscription_contents = [
            (0, Clusters.BasicInformation.Attributes.NodeLabel),  # Single attribute
            (0, Clusters.OperationalCredentials),  # Wildcard all of opcreds attributes on EP0
            Clusters.Descriptor  # All descriptors on all endpoints
        ]

        logging.info("Step 4 and 5 (first part): Establish subscription with all %d clients" % len(client_list))
        for sub_idx, client in enumerate(client_list):
            logging.info("Establishing subscription %d/%d from controller node %s" % (sub_idx + 1, len(client_list), client.name))

            sub = await client.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=subscription_contents,
                reportInterval=(min_report_interval_sec, max_report_interval_sec),
                keepSubscriptions=False
            )
            self._subscriptions.append(sub)

            attribute_handler = AttributeChangeAccumulator(
                name=client.name, expected_attribute=Clusters.BasicInformation.Attributes.NodeLabel, output=output_queue)
            sub.SetAttributeUpdateCallback(attribute_handler)
            sub_handlers.append(attribute_handler)

            # TODO: Replace resubscription catcher with API to disable re-subscription on failure
            resub_catcher = ResubscriptionCatcher(name=client.name)
            sub.SetResubscriptionAttemptedCallback(resub_catcher)
            resub_catchers.append(resub_catcher)

        asserts.assert_equal(len(self._subscriptions), len(client_list), "Must have the right number of subscriptions")

        # Step 6: Read 9 paths and validate success
        logging.info("Step 6: Read 9 paths (first 9 attributes of Basic Information cluster) and validate success")

        large_read_contents = [
            Clusters.BasicInformation.Attributes.DataModelRevision,
            Clusters.BasicInformation.Attributes.VendorName,
            Clusters.BasicInformation.Attributes.VendorID,
            Clusters.BasicInformation.Attributes.ProductName,
            Clusters.BasicInformation.Attributes.ProductID,
            Clusters.BasicInformation.Attributes.NodeLabel,
            Clusters.BasicInformation.Attributes.Location,
            Clusters.BasicInformation.Attributes.HardwareVersion,
            Clusters.BasicInformation.Attributes.HardwareVersionString,
        ]
        large_read_paths = [(0, attrib) for attrib in large_read_contents]
        basic_info = await dev_ctrl.ReadAttribute(self.dut_node_id, large_read_paths)

        # Make sure everything came back from the read that we expected
        asserts.assert_true(0 in basic_info.keys(), "Must have read endpoint 0 data")
        asserts.assert_true(Clusters.BasicInformation in basic_info[0].keys(), "Must have read Basic Information cluster data")
        for attribute in large_read_contents:
            asserts.assert_true(attribute in basic_info[0][Clusters.BasicInformation],
                                "Must have read back attribute %s" % (attribute.__name__))

        # Step 7: Trigger a change on NodeLabel
        logging.info(
            "Step 7: Change attribute with one client, await all attributes changed successfully without loss of subscriptions")
        await asyncio.sleep(1)
        await client_list[0].WriteAttribute(self.dut_node_id,
                                            [(0, Clusters.BasicInformation.Attributes.NodeLabel(value=AFTER_LABEL))])

        all_changes = {client.name: False for client in client_list}

        # Await a stabilization delay in increments to let the event loops run
        start_time = time.time()
        elapsed = 0
        time_remaining = timeout_delay_sec

        while time_remaining > 0:
            try:
                item = output_queue.get(block=True, timeout=time_remaining)
                client_name, endpoint, attribute, value = item['name'], item['endpoint'], item['attribute'], item['value']

                # Record arrival of an expected subscription change when seen
                if endpoint == 0 and attribute == Clusters.BasicInformation.Attributes.NodeLabel and value == AFTER_LABEL:
                    if not all_changes[client_name]:
                        logging.info("Got expected attribute change for client %s" % client_name)
                        all_changes[client_name] = True

                # We are done waiting when we have accumulated all results
                if all(all_changes.values()):
                    logging.info("All clients have reported, done waiting.")
                    break
            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = timeout_delay_sec - elapsed

        logging.info("Step 7: Validation of results")
        sub_test_failed = False

        for catcher in resub_catchers:
            if catcher.caught_resubscription:
                logging.error("Client %s saw a resubscription" % catcher.name)
                sub_test_failed = True
            else:
                logging.info("Client %s correctly did not see a resubscription" % catcher.name)

        all_reports_gotten = all(all_changes.values())
        if not all_reports_gotten:
            logging.error("Missing reports from the following clients: %s" %
                          ", ".join([name for name, value in all_changes.items() if value is False]))
            sub_test_failed = True
        else:
            logging.info("Got successful reports from all clients, meaning all concurrent CASE sessions worked")

        # Determine result of Step 7
        if sub_test_failed:
            asserts.fail("Failed step 7 !")

        # Step 8: Validate sessions have not changed by doing a read on NodeLabel from all clients
        logging.info("Step 8a: Read back NodeLabel directly from all clients")
        for sub_idx, client in enumerate(client_list):
            logging.info("Reading NodeLabel (%d/%d) from controller node %s" % (sub_idx + 1, len(client_list), client.name))

            label_readback = await self.read_single_attribute(client,
                                                              node_id=self.dut_node_id,
                                                              endpoint=0,
                                                              attribute=Clusters.BasicInformation.Attributes.NodeLabel)
            asserts.assert_equal(label_readback, AFTER_LABEL)

        # On each client, read back the local session id for the CASE session to the DUT and ensure
        # it's the same as that of the session established right at the beginning of the test.
        # In tandem with checking that the number of sessions to the DUT is exactly one,
        # this ensures we have not established any new CASE
        # sessions in this test.
        if check_local_session_id_unchanged:
            logging.info("Step 8b: Validate that the local CASE session ID hasn't changed")
            num_failed_clients = 0

            for client in client_list:
                beginning_session_id = local_session_id_by_client_name[client.name]
                end_session_id = client.GetConnectedDeviceSync(self.dut_node_id).localSessionId
                total_sessions = client.GetConnectedDeviceSync(self.dut_node_id).numTotalSessions

                if (beginning_session_id != end_session_id):
                    logging.error(
                        f"Test ended with a different session ID created from what we had before for {client.name} "
                        f"(total sessions = {total_sessions})")
                    num_failed_clients = num_failed_clients + 1
                elif (total_sessions != 1):
                    logging.error(f"Test ended with more than 1 session for {client.name}")
                    num_failed_clients = num_failed_clients + 1

            if (num_failed_clients > 0):
                asserts.fail(f"Failed Step 8b: ({num_failed_clients} / {len(client_list)} failed)")

        # Step 9: Fill user label list
        if has_user_labels and not skip_user_label_cluster_steps:
            await self.fill_user_label_list(dev_ctrl, self.dut_node_id)
        else:
            logging.info("Step 9: Skipped due to no UserLabel cluster instances")
        # Step 10: Reconfig ACL to allow test runner access to Groups clusters on all endpoints.
        logging.info("Step 10: Reconfiguring ACL to allow access to Groups Clusters")
        await self.send_acl(test_step=10, client_by_name=client_by_name, enable_access_to_group_cluster=True, fabric_table=fabric_table)
        # Step 11: Count all group cluster instances
        # and ensure MaxGroupsPerFabric >= 4 * counted_groups_clusters.
        logging.info("Step 11: Validating groups support minimums")
        groups_cluster_endpoints: Dict[int, Any] = await dev_ctrl.ReadAttribute(self.dut_node_id, [Clusters.Groups])
        counted_groups_clusters: int = len(groups_cluster_endpoints)

        # The test for Step 11 and all of Steps 12 to 15 are only performed if Groups cluster instances are found.
        if counted_groups_clusters > 0:
            indicated_max_groups_per_fabric: int = await self.read_single_attribute(
                dev_ctrl,
                node_id=self.dut_node_id,
                endpoint=0,
                attribute=Clusters.GroupKeyManagement.Attributes.MaxGroupsPerFabric)
            logging.info(
                f"MaxGroupsPerFabric value: {indicated_max_groups_per_fabric}, number of endpoints with Groups clusters cluster: {counted_groups_clusters}, which are: {list(groups_cluster_endpoints.keys())}")
            if indicated_max_groups_per_fabric < 4 * counted_groups_clusters:
                asserts.fail("Failed Step 11: MaxGroupsPerFabric < 4 * counted_groups_clusters")

            # Step 12: Confirm MaxGroupKeysPerFabric meets the minimum requirement of 3.
            indicated_max_group_keys_per_fabric: int = await self.read_single_attribute(
                dev_ctrl,
                node_id=self.dut_node_id,
                endpoint=0,
                attribute=Clusters.GroupKeyManagement.Attributes.MaxGroupKeysPerFabric)
            if indicated_max_group_keys_per_fabric < 3:
                asserts.fail("Failed Step 12: MaxGroupKeysPerFabric < 3")

            # Create a list of per-fabric clients to use for filling group resources accross all fabrics.
            fabric_unique_clients: List[Any] = []

            for fabric in fabric_table:
                client_name = generate_controller_name(fabric.fabricIndex, 0)
                fabric_unique_clients.append(client_by_name[client_name])

            # Step 13: Write and verify indicated_max_group_keys_per_fabric group keys to all fabrics.
            group_keys: List[List[
                Clusters.GroupKeyManagement.Structs.GroupKeySetStruct]] = await self.fill_and_validate_group_key_sets(
                num_fabrics_to_commission, fabric_unique_clients, indicated_max_group_keys_per_fabric)

            # Step 14: Write and verify indicated_max_groups_per_fabric group/key mappings for all fabrics.
            # First, Generate list of unique group/key mappings
            group_key_map: List[Dict[int, int]] = [{} for _ in range(num_fabrics_to_commission)]
            for fabric_list_idx in range(num_fabrics_to_commission):
                for group_idx in range(indicated_max_groups_per_fabric):
                    group_id: int = fabric_list_idx * indicated_max_groups_per_fabric + group_idx + 1
                    group_key_idx: int = group_idx % len(group_keys[fabric_list_idx])
                    group_key_map[fabric_list_idx][group_id] = group_keys[fabric_list_idx][group_key_idx].groupKeySetID

            await self.fill_and_validate_group_key_map(
                num_fabrics_to_commission, fabric_unique_clients, group_key_map, fabric_table)

            # Step 15: Add all the groups to the discovered groups-supporting endpoints and verify GroupTable
            group_table_written: List[
                Dict[int, Clusters.GroupKeyManagement.Structs.GroupInfoMapStruct]] = await self.add_all_groups(
                num_fabrics_to_commission, fabric_unique_clients, group_key_map,
                groups_cluster_endpoints, indicated_max_groups_per_fabric, fabric_table)
            await self.validate_group_table(num_fabrics_to_commission, fabric_unique_clients, group_table_written, fabric_table)

        # Read heap watermarks after the test
        if check_heap_watermarks:
            logging.info("Read Heap info after stress test")
            high_watermark_after, current_usage_after = await self.read_heap_statistics(dev_ctrl)
            logging.info("=== Heap Usage Diagnostics ===\nHigh watermark: {} (before) / {} (after)\n"
                         "Current usage: {} (before) / {} (after)".format(high_watermark_before, high_watermark_after,
                                                                          current_usage_before, current_usage_after))

    def random_string(self, length) -> str:
        rnd = self._pseudo_random_generator
        return "".join([rnd.choice("abcdef0123456789") for _ in range(length)])[:length]

    async def fill_user_label_list(self, dev_ctrl, target_node_id):
        logging.info("Step 9: Fill UserLabel clusters on each endpoint")
        user_labels = await dev_ctrl.ReadAttribute(target_node_id, [Clusters.UserLabel])

        # Build 4 sets of maximized labels
        random_label = self.random_string(16)
        random_value = self.random_string(16)
        labels = [Clusters.UserLabel.Structs.LabelStruct(label=random_label, value=random_value) for _ in range(4)]

        for endpoint_id in user_labels:
            clusters = user_labels[endpoint_id]
            for cluster in clusters:
                if cluster == Clusters.UserLabel:
                    logging.info("Step 9a: Filling UserLabel cluster on endpoint %d" % endpoint_id)
                    statuses = await dev_ctrl.WriteAttribute(target_node_id,
                                                             [(endpoint_id, Clusters.UserLabel.Attributes.LabelList(labels))])
                    asserts.assert_equal(statuses[0].Status, StatusEnum.Success, "Label write must succeed")

                    logging.info("Step 9b: Validate UserLabel cluster contents after write on endpoint %d" % endpoint_id)
                    read_back_labels = await self.read_single_attribute(dev_ctrl,
                                                                        node_id=target_node_id,
                                                                        endpoint=endpoint_id,
                                                                        attribute=Clusters.UserLabel.Attributes.LabelList)
                    print(read_back_labels)

                    asserts.assert_equal(read_back_labels, labels, "LabelList attribute must match what was written")

    async def fill_and_validate_group_key_sets(self,
                                               fabrics: int,
                                               clients: List[Any],
                                               keys_per_fabric: int) -> List[List[
                                                   Clusters.GroupKeyManagement.Structs.GroupKeySetStruct]]:
        # Step 12: Write indicated_max_group_keys_per_fabric group keys to all fabrics.
        group_keys: List[List[Clusters.GroupKeyManagement.Structs.GroupKeySetStruct]] = [[] for _ in range(fabrics)]
        for client_idx in range(fabrics):
            client: Any = clients[client_idx]

            # Write, skip the IPK key set.
            for group_key_cluster_idx in range(1, keys_per_fabric):
                group_key_list_idx: int = group_key_cluster_idx - 1

                logging.info("Step 13: Setting group key on fabric %d at index '%d'" % (client_idx+1, group_key_cluster_idx))
                group_keys[client_idx].append(self.build_group_key(client_idx, group_key_cluster_idx, keys_per_fabric))
                await client.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(
                    group_keys[client_idx][group_key_list_idx]))

        # Step 12 verification: After all the key sets were written, read all the information back.
        for client_idx in range(fabrics):
            client: Any = clients[client_idx]

            logging.info("Step 13: Reading back group keys on fabric %d" % (client_idx+1))
            resp = await client.SendCommand(self.dut_node_id, 0,
                                            Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices(),
                                            responseType=Clusters.GroupKeyManagement.Commands.KeySetReadAllIndicesResponse)

            read_group_key_ids: List[int] = resp.groupKeySetIDs
            known_group_key_ids: List[int] = [key_set.groupKeySetID for key_set in group_keys[client_idx]]
            ipk_group_key_id: Set[int] = set(read_group_key_ids) - set(known_group_key_ids)

            asserts.assert_equal(keys_per_fabric, len(read_group_key_ids),
                                 "KeySetReadAllIndicesResponse length does "
                                 "not match the key support indicated: %d." % (keys_per_fabric))

            asserts.assert_equal(len(ipk_group_key_id), 1,
                                 "Read more than 1 key ID that did not match written values after IPK (only expected 1 for IPK).")

        return group_keys

    async def fill_and_validate_group_key_map(self,
                                              fabrics: int,
                                              clients: List[Any],
                                              group_key_map: List[Dict[int, int]],
                                              fabric_table: List[
                                                  Clusters.OperationalCredentials.Structs.FabricDescriptorStruct]) -> None:
        # Step 14: Write and verify indicated_max_groups_per_fabric group/key mappings for all fabrics.
        mapping_structs: List[List[Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct]] = [[] for _ in range(fabrics)]
        for client_idx in range(fabrics):
            client: Any = clients[client_idx]
            fabric_idx: int = fabric_table[client_idx].fabricIndex

            for group in group_key_map[client_idx]:
                mapping_structs[client_idx].append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                    groupId=group,
                    groupKeySetID=group_key_map[client_idx][group],
                    fabricIndex=fabric_idx))

            logging.info("Step 14: Setting group key map on fabric %d" % (fabric_idx))
            await client.WriteAttribute(
                self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping_structs[client_idx]))])

        # Step 14 verification: After all the group key maps were written, read all the information back.
        for client_idx in range(fabrics):
            client: Any = clients[client_idx]
            fabric_idx: int = fabric_table[client_idx].fabricIndex

            logging.info("Step 14: Reading group key map on fabric %d" % (fabric_idx))
            group_key_map_readback = await self.read_single_attribute(
                client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupKeyMap)

            found_entry: int = 0
            for read_entry in group_key_map_readback:
                if read_entry.fabricIndex != fabric_idx:
                    continue

                written_entry = next(entry for entry in mapping_structs[client_idx] if entry.groupId == read_entry.groupId)
                found_entry += 1
                asserts.assert_equal(written_entry.groupId, read_entry.groupId)
                asserts.assert_equal(written_entry.groupKeySetID, read_entry.groupKeySetID)
                asserts.assert_equal(written_entry.fabricIndex, read_entry.fabricIndex)

            asserts.assert_equal(found_entry, len(mapping_structs[client_idx]),
                                 "GroupKeyMap does not match the length of written data.")

    async def add_all_groups(self,
                             fabrics: int,
                             clients: List[Any],
                             group_key_map: List[Dict[int, int]],
                             group_endpoints: Dict[int, Any],
                             groups_per_fabric: int,
                             fabric_table: List[
                                 Clusters.OperationalCredentials.Structs.FabricDescriptorStruct]) -> List[
            Dict[int, Clusters.GroupKeyManagement.Structs.GroupInfoMapStruct]]:
        # Step 14: Add indicated_max_groups_per_fabric to each fabric through the Groups clusters on supporting endpoints.
        written_group_table_map: List[Dict[int, Clusters.GroupKeyManagement.Structs.GroupInfoMapStruct]] = [
            {} for _ in range(fabrics)]
        for client_idx in range(fabrics):
            client: Any = clients[client_idx]
            fabric_idx: int = fabric_table[client_idx].fabricIndex

            base_groups_per_endpoint: int = math.floor(groups_per_fabric / len(group_endpoints))
            groups_remainder: int = groups_per_fabric % len(group_endpoints)

            group_id_list_copy = list(group_key_map[client_idx])
            # This is just a sanity check that the number of IDs provided matches how many group IDs we are told to
            # write to each fabric.
            asserts.assert_equal(len(group_id_list_copy), groups_per_fabric)

            for endpoint_id in group_endpoints:
                groups_to_add: int = base_groups_per_endpoint
                if groups_remainder:
                    groups_to_add += 1
                    groups_remainder -= 1

                feature_map: int = await self.read_single_attribute(client,
                                                                    node_id=self.dut_node_id,
                                                                    endpoint=endpoint_id,
                                                                    attribute=Clusters.Groups.Attributes.FeatureMap)
                name_featrure_bit: int = 0
                name_supported: bool = (feature_map & (1 << name_featrure_bit)) != 0

                # Write groups to cluster
                for _ in range(groups_to_add):
                    group_id = group_id_list_copy.pop()
                    group_name: str = self.random_string(16) if name_supported else ""
                    command: Clusters.Groups.Commands.AddGroup = Clusters.Groups.Commands.AddGroup(
                        groupID=group_id, groupName=group_name)
                    written_group_table_map[client_idx][group_id] = Clusters.GroupKeyManagement.Structs.GroupInfoMapStruct(
                        groupId=group_id,
                        groupName=group_name,
                        fabricIndex=fabric_idx,
                        endpoints=[endpoint_id])
                    add_response: Clusters.Groups.Commands.AddGroupResponse = await client.SendCommand(
                        self.dut_node_id, endpoint_id, command, responseType=Clusters.Groups.Commands.AddGroupResponse)
                    asserts.assert_equal(StatusEnum.Success, add_response.status)
                    asserts.assert_equal(group_id, add_response.groupID)

        return written_group_table_map

    async def validate_group_table(self,
                                   fabrics: int,
                                   clients: List[Any],
                                   group_table_written: List[Dict[int, Clusters.GroupKeyManagement.Structs.GroupInfoMapStruct]],
                                   fabric_table: List[Clusters.OperationalCredentials.Structs.FabricDescriptorStruct]) -> None:
        for client_idx in range(fabrics):
            client: Any = clients[client_idx]
            fabric_idx: int = fabric_table[client_idx].fabricIndex

            group_table_read: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
                client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)

            found_groups: int = 0
            for read_entry in group_table_read:
                if read_entry.fabricIndex != fabric_idx:
                    continue

                found_groups += 1
                asserts.assert_in(read_entry.groupId, group_table_written[client_idx], "Group missing from group map")
                written_entry: Clusters.GroupKeyManagement.Structs.GroupInfoMapStruct = group_table_written[
                    client_idx][read_entry.groupId]
                asserts.assert_equal(written_entry.groupId, read_entry.groupId)
                asserts.assert_equal(written_entry.endpoints, read_entry.endpoints)
                asserts.assert_equal(written_entry.groupName, read_entry.groupName)
                asserts.assert_equal(written_entry.fabricIndex, read_entry.fabricIndex)

            asserts.assert_equal(found_groups, len(group_table_written[client_idx]),
                                 "Found group count does not match written value.")

    async def send_acl(self,
                       test_step: int,
                       client_by_name,
                       enable_access_to_group_cluster: bool,
                       fabric_table: List[
                           Clusters.OperationalCredentials.Structs.FabricDescriptorStruct]):
        for fabric in fabric_table:
            client_name = generate_controller_name(fabric.fabricIndex, 0)
            client = client_by_name[client_name]

            acl = self.build_acl(enable_access_to_group_cluster)

            logging.info(f"Step {test_step}a: Writing ACL entry for fabric {fabric.fabricIndex}")
            await client.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

            logging.info(f"Step {test_step}b: Validating ACL entry for fabric {fabric.fabricIndex}")
            acl_readback = await self.read_single_attribute(
                client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.AccessControl.Attributes.Acl)
            fabric_index = 9999
            for entry in acl_readback:
                asserts.assert_equal(entry.fabricIndex, fabric.fabricIndex, "Fabric Index of response entries must match")
                fabric_index = entry.fabricIndex

            for entry in acl:
                # Fix-up the original ACL list items (that all had fabricIndex of 0 on write, since ignored)
                # so that they match incoming fabric index. Allows checking by equality of the structs
                entry.fabricIndex = fabric_index
            asserts.assert_equal(acl_readback, acl, "ACL must match what was written")

    def build_acl(self, enable_access_to_group_cluster: bool):
        acl = []

        # Test says:
        #
        # . struct
        # - Privilege field: Administer (5)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0xFFFF_FFFD_0001_0001, 0x2000_0000_0000_0001, 0x2000_0000_0000_0002, 0x2000_0000_0000_0003]
        # - Targets field: [
        #                   {Endpoint: 0},
        #                   {Cluster: 0xFFF1_FC00, DeviceType: 0xFFF1_BC30},
        #                   {Cluster: 0xFFF1_FC00, DeviceType: 0xFFF1_BC31}
        #                 ]
        # . struct
        # - Privilege field: Manage (4)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0x1000_0000_0000_0001, 0x1000_0000_0000_0002, 0x1000_0000_0000_0003, 0x1000_0000_0000_0004]
        # - Targets field: [
        #                   {Cluster: 0xFFF1_FC00, DeviceType: 0xFFF1_BC20},
        #                   {Cluster: 0xFFF1_FC01, DeviceType: 0xFFF1_BC21},
        #                   {Cluster: 0xFFF1_FC02, DeviceType: 0xFFF1_BC22}
        #                 ]
        # . struct
        # - Privilege field: Operate (3)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0x3000_0000_0000_0001, 0x3000_0000_0000_0002, 0x3000_0000_0000_0003, 0x3000_0000_0000_0004]
        # - Targets field: [{Cluster: 0xFFF1_FC40, DeviceType: 0xFFF1_BC20},
        #                  {Cluster: 0xFFF1_FC41, DeviceType: 0xFFF1_BC21},
        #                  {Cluster: 0xFFF1_FC02, DeviceType: 0xFFF1_BC42}]
        # . struct
        # - Privilege field: View (1)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0x4000_0000_0000_0001, 0x4000_0000_0000_0002, 0x4000_0000_0000_0003, 0x4000_0000_0000_0004]
        # - Targets field: [{Cluster: 0xFFF1_FC80, DeviceType: 0xFFF1_BC20},
        #                  {Cluster: 0xFFF1_FC81, DeviceType: 0xFFF1_BC21},
        #                  {Cluster: 0xFFF1_FC82, DeviceType: 0xFFF1_BC22}]

        # Administer ACL entry
        admin_subjects = [0xFFFF_FFFD_0001_0001, 0x2000_0000_0000_0001, 0x2000_0000_0000_0002, 0x2000_0000_0000_0003]

        admin_target_field_2 = Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC00, deviceType=0xFFF1_BC30)
        if enable_access_to_group_cluster:
            admin_target_field_2 = Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0x0000_0004)

        admin_targets = [
            Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0),
            admin_target_field_2,
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC01, deviceType=0xFFF1_BC31)
        ]
        admin_acl_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=admin_subjects,
            targets=admin_targets
        )
        acl.append(admin_acl_entry)

        # Manage ACL entry
        manage_subjects = [0x1000_0000_0000_0001, 0x1000_0000_0000_0002, 0x1000_0000_0000_0003, 0x1000_0000_0000_0004]
        manage_targets = [
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC00, deviceType=0xFFF1_BC20),
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC01, deviceType=0xFFF1_BC21),
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC02, deviceType=0xFFF1_BC22)
        ]

        manage_acl_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=manage_subjects,
            targets=manage_targets
        )
        acl.append(manage_acl_entry)

        # Operate ACL entry
        operate_subjects = [0x3000_0000_0000_0001, 0x3000_0000_0000_0002, 0x3000_0000_0000_0003, 0x3000_0000_0000_0004]
        operate_targets = [
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC40, deviceType=0xFFF1_BC20),
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC41, deviceType=0xFFF1_BC21),
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC42, deviceType=0xFFF1_BC42)
        ]

        operate_acl_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=operate_subjects,
            targets=operate_targets
        )
        acl.append(operate_acl_entry)

        # View ACL entry
        view_subjects = [0x4000_0000_0000_0001, 0x4000_0000_0000_0002, 0x4000_0000_0000_0003, 0x4000_0000_0000_0004]
        view_targets = [
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC80, deviceType=0xFFF1_BC20),
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC81, deviceType=0xFFF1_BC21),
            Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0xFFF1_FC82, deviceType=0xFFF1_BC22)
        ]

        view_acl_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=view_subjects,
            targets=view_targets)
        acl.append(view_acl_entry)

        return acl

    def build_group_key(self, fabric_index: int,
                        group_key_index: int, keys_per_fabric: int) -> Clusters.GroupKeyManagement.Structs.GroupKeySetStruct:
        asserts.assert_not_equal(group_key_index, 0, "TH Internal Error: IPK key set index (0) should not be re-generated.")

        # groupKeySetID is definted as uint16 in the Matter specification.
        # To easily test that the stored values are unique, unique values are created accross all fabrics.
        # However, it is only required that values be unique within a fabric according to the specifiction.
        # If a device ever provides over 65535 total key sets, then this will need to be updated.
        set_id: int = fabric_index*keys_per_fabric + group_key_index
        asserts.assert_less_equal(
            set_id, 0xFFFF, "Invalid Key Set ID. This may be a limitation of the test harness, not the device under test.")
        return Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=self.random_string(16).encode(),
            epochStartTime0=(set_id * 4),
            epochKey1=self.random_string(16).encode(),
            epochStartTime1=(set_id * 4 + 1),
            epochKey2=self.random_string(16).encode(),
            epochStartTime2=(set_id * 4 + 2))

    async def read_heap_statistics(self, dev_ctrl):
        diagnostics_contents = [
            Clusters.SoftwareDiagnostics.Attributes.CurrentHeapHighWatermark,
            Clusters.SoftwareDiagnostics.Attributes.CurrentHeapUsed,
        ]
        diagnostics_paths = [(0, attrib) for attrib in diagnostics_contents]
        swdiag_info = await dev_ctrl.ReadAttribute(self.dut_node_id, diagnostics_paths)

        # Make sure everything came back from the read that we expected
        asserts.assert_true(0 in swdiag_info.keys(), "Must have read endpoint 0 data")
        asserts.assert_true(Clusters.SoftwareDiagnostics in swdiag_info[0].keys(
        ), "Must have read Software Diagnostics cluster data")
        for attribute in diagnostics_contents:
            asserts.assert_true(attribute in swdiag_info[0][Clusters.SoftwareDiagnostics],
                                "Must have read back attribute %s" % (attribute.__name__))
        high_watermark = swdiag_info[0][Clusters.SoftwareDiagnostics][
            Clusters.SoftwareDiagnostics.Attributes.CurrentHeapHighWatermark]
        current_usage = swdiag_info[0][Clusters.SoftwareDiagnostics][
            Clusters.SoftwareDiagnostics.Attributes.CurrentHeapUsed]
        return high_watermark, current_usage


if __name__ == "__main__":
    default_matter_test_main()
