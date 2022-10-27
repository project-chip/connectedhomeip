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

from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
import chip.clusters as Clusters
import chip.FabricAdmin
import chip.CertificateAuthority
import logging
from mobly import asserts
from chip.utils import CommissioningBuildingBlocks
from chip.clusters.Attribute import TypedAttributePath, SubscriptionTransaction, AttributeStatus
from chip.interaction_model import Status as StatusEnum
import queue
import asyncio
from binascii import hexlify
from threading import Event
import time
import random

from TC_SC_3_6 import AttributeChangeAccumulator, ResubscriptionCatcher

# TODO: Overall, we need to add validation that session IDs have not changed throughout to be agnostic
#       to some internal behavior assumptions of the SDK we are making relative to the write to
#       the trigger the subscriptions not re-opening a new CASE session
#


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

        BEFORE_LABEL = "Before Subscriptions 12345678912"
        AFTER_LABEL = "After Subscriptions 123456789123"

        # Pre-conditions

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
        all_names = []
        for fabric_idx in range(num_fabrics_to_commission):
            for controller_idx in range(num_controllers_per_fabric):
                all_names.append("RD%d%s" % (fabric_idx + 1, chr(ord('A') + controller_idx)))
        logging.info(f"Client names that will be used: {all_names}")
        client_list = []

        # TODO: Shall we also verify SupportedFabrics attribute, and the CapabilityMinima attribute?
        logging.info("Pre-conditions: validate CapabilityMinima.CaseSessionsPerFabric >= 3")

        capability_minima = await self.read_single_attribute(dev_ctrl, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.Basic.Attributes.CapabilityMinima)
        asserts.assert_greater_equal(capability_minima.caseSessionsPerFabric, 3)

        # Step 1: Commission 5 fabrics with maximized NOC chains
        logging.info(f"Step 1: use existing fabric to configure new fabrics so that total is {num_fabrics_to_commission} fabrics")

        # Generate Node IDs for subsequent controllers start at 200, follow 200, 300, ...
        node_ids = [200 + (i * 100) for i in range(num_controllers_per_fabric - 1)]

        # Prepare clients for first fabric, that includes the default controller
        dev_ctrl.name = all_names.pop(0)
        client_list.append(dev_ctrl)

        if num_controllers_per_fabric > 1:
            new_controllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(fabricAdmin=dev_ctrl.fabricAdmin, adminDevCtrl=dev_ctrl, controllerNodeIds=node_ids, privilege=Clusters.AccessControl.Enums.Privilege.kAdminister, targetNodeId=self.dut_node_id, catTags=[0x0001_0001])
            for controller in new_controllers:
                controller.name = all_names.pop(0)
            client_list.extend(new_controllers)

        # Prepare clients for subsequent fabrics
        for i in range(num_fabrics_to_commission - 1):
            admin_index = 2 + i
            logging.info("Commissioning fabric %d/%d" % (admin_index, num_fabrics_to_commission))
            new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
            new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=admin_index)

            new_admin_ctrl = new_fabric_admin.NewController(nodeId=dev_ctrl.nodeId, catTags=[0x0001_0001])
            new_admin_ctrl.name = all_names.pop(0)
            client_list.append(new_admin_ctrl)
            await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=new_admin_ctrl, existingNodeId=self.dut_node_id, newNodeId=self.dut_node_id)

            if num_controllers_per_fabric > 1:
                new_controllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(fabricAdmin=new_fabric_admin, adminDevCtrl=new_admin_ctrl,
                                                                                              controllerNodeIds=node_ids, privilege=Clusters.AccessControl.Enums.Privilege.kAdminister, targetNodeId=self.dut_node_id, catTags=[0x0001_0001])
                for controller in new_controllers:
                    controller.name = all_names.pop(0)

                client_list.extend(new_controllers)

        asserts.assert_equal(len(client_list), num_fabrics_to_commission *
                             num_controllers_per_fabric, "Must have the right number of clients")

        client_by_name = {client.name: client for client in client_list}
        local_session_id_by_client_name = {client.name: client.GetConnectedDeviceSync(
            self.dut_node_id).localSessionId for client in client_list}

        # Step 2: Set the Label field for each fabric and BasicInformation.NodeLabel to 32 characters
        logging.info("Step 2: Setting the Label field for each fabric and BasicInformation.NodeLabel to 32 characters")

        for idx in range(num_fabrics_to_commission):
            fabric_number = idx + 1
            # Client is client A for each fabric to set the Label field
            client_name = "RD%dA" % fabric_number
            client = client_by_name[client_name]

            # Send the UpdateLabel command
            label = ("%d" % fabric_number) * 32
            logging.info("Step 2a: Setting fabric label on fabric %d to '%s' using client %s" % (fabric_number, label, client_name))
            await client.SendCommand(self.dut_node_id, 0, Clusters.OperationalCredentials.Commands.UpdateFabricLabel(label))

            # Read back
            fabric_metadata = await self.read_single_attribute(client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.OperationalCredentials.Attributes.Fabrics)
            print(fabric_metadata)
            asserts.assert_equal(fabric_metadata[0].label, label, "Fabrics[x].label must match what was written")

        # Before subscribing, set the NodeLabel to "Before Subscriptions"
        logging.info(f"Step 2b: Set BasicInformation.NodeLabel to {BEFORE_LABEL}")
        await client_list[0].WriteAttribute(self.dut_node_id, [(0, Clusters.Basic.Attributes.NodeLabel(value=BEFORE_LABEL))])

        node_label = await self.read_single_attribute(client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.Basic.Attributes.NodeLabel)
        asserts.assert_equal(node_label, BEFORE_LABEL, "NodeLabel must match what was written")

        # Step 3: Add 3 Access Control entries on DUT with a list of 4 Subjects and 3 Targets with the following parameters (...)
        logging.info("Step 3: Fill ACL table so that all minimas are reached")

        for idx in range(num_fabrics_to_commission):
            fabric_number = idx + 1
            # Client is client A for each fabric
            client_name = "RD%dA" % fabric_number
            client = client_by_name[client_name]

            acl = self.build_acl(fabric_number, client_by_name, num_controllers_per_fabric)

            logging.info(f"Step 3a: Writing ACL entry for fabric {fabric_number}")
            await client.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

            logging.info(f"Step 3b: Validating ACL entry for fabric {fabric_number}")
            acl_readback = await self.read_single_attribute(client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.AccessControl.Attributes.Acl)
            fabric_index = 9999
            for entry in acl_readback:
                asserts.assert_equal(entry.fabricIndex, fabric_number, "Fabric Index of response entries must match")
                fabric_index = entry.fabricIndex

            for entry in acl:
                # Fix-up the original ACL list items (that all had fabricIndex of 0 on write, since ignored)
                # so that they match incoming fabric index. Allows checking by equality of the structs
                entry.fabricIndex = fabric_index
            asserts.assert_equal(acl_readback, acl, "ACL must match what was written")

        # Step 4 and 5 (the operations cannot be separated): establish all CASE sessions and subscriptions

        # Subscribe with all clients to NodeLabel attribute and 2 more paths
        sub_handlers = []
        resub_catchers = []
        output_queue = queue.Queue()
        subscription_contents = [
            (0, Clusters.Basic.Attributes.NodeLabel),  # Single attribute
            (0, Clusters.OperationalCredentials),  # Wildcard all of opcreds attributes on EP0
            Clusters.Descriptor  # All descriptors on all endpoints
        ]

        logging.info("Step 4 and 5 (first part): Establish subscription with all %d clients" % len(client_list))
        for sub_idx, client in enumerate(client_list):
            logging.info("Establishing subscription %d/%d from controller node %s" % (sub_idx + 1, len(client_list), client.name))

            sub = await client.ReadAttribute(nodeid=self.dut_node_id, attributes=subscription_contents,
                                             reportInterval=(min_report_interval_sec, max_report_interval_sec), keepSubscriptions=False)
            self._subscriptions.append(sub)

            attribute_handler = AttributeChangeAccumulator(
                name=client.name, expected_attribute=Clusters.Basic.Attributes.NodeLabel, output=output_queue)
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
            Clusters.Basic.Attributes.DataModelRevision,
            Clusters.Basic.Attributes.VendorName,
            Clusters.Basic.Attributes.VendorID,
            Clusters.Basic.Attributes.ProductName,
            Clusters.Basic.Attributes.ProductID,
            Clusters.Basic.Attributes.NodeLabel,
            Clusters.Basic.Attributes.Location,
            Clusters.Basic.Attributes.HardwareVersion,
            Clusters.Basic.Attributes.HardwareVersionString,
        ]
        large_read_paths = [(0, attrib) for attrib in large_read_contents]
        basic_info = await dev_ctrl.ReadAttribute(self.dut_node_id, large_read_paths)

        # Make sure everything came back from the read that we expected
        asserts.assert_true(0 in basic_info.keys(), "Must have read endpoint 0 data")
        asserts.assert_true(Clusters.Basic in basic_info[0].keys(), "Must have read Basic Information cluster data")
        for attribute in large_read_contents:
            asserts.assert_true(attribute in basic_info[0][Clusters.Basic],
                                "Must have read back attribute %s" % (attribute.__name__))

        # Step 7: Trigger a change on NodeLabel
        logging.info(
            "Step 7: Change attribute with one client, await all attributes changed successfully without loss of subscriptions")
        await asyncio.sleep(1)
        await client_list[0].WriteAttribute(self.dut_node_id, [(0, Clusters.Basic.Attributes.NodeLabel(value=AFTER_LABEL))])

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
                if endpoint == 0 and attribute == Clusters.Basic.Attributes.NodeLabel and value == AFTER_LABEL:
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

            label_readback = await self.read_single_attribute(client, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.Basic.Attributes.NodeLabel)
            asserts.assert_equal(label_readback, AFTER_LABEL)

        # On each client, read back the local session id for the CASE session to the DUT and ensure it's the same as that of the session established right at the
        # beginning of the test. In tandem with checking that the number of sessions to the DUT is exactly one, this ensures we have not established any new CASE
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
                        f"Test ended with a different session ID created from what we had before for {client.name} (total sessions = {total_sessions})")
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
                    statuses = await dev_ctrl.WriteAttribute(target_node_id, [(endpoint_id, Clusters.UserLabel.Attributes.LabelList(labels))])
                    asserts.assert_equal(statuses[0].Status, StatusEnum.Success, "Label write must succeed")

                    logging.info("Step 9b: Validate UserLabel cluster contents after write on endpoint %d" % endpoint_id)
                    read_back_labels = await self.read_single_attribute(dev_ctrl, node_id=target_node_id, endpoint=endpoint_id, attribute=Clusters.UserLabel.Attributes.LabelList)
                    print(read_back_labels)

                    asserts.assert_equal(read_back_labels, labels, "LabelList attribute must match what was written")

    def build_acl(self, fabric_number, client_by_name, num_controllers_per_fabric):
        acl = []

        # Test says:
        #
        # . struct
        # - Privilege field: Administer (5)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0xFFFF_FFFD_0001_0001, 0x2000_0000_0000_0001, 0x2000_0000_0000_0002, 0x2000_0000_0000_0003]
        # - Targets field: [{Endpoint: 0}, {Cluster: 0xFFF1_FC00, DeviceType: 0xFFF1_FC30}, {Cluster: 0xFFF1_FC00, DeviceType: 0xFFF1_FC31}]
        # . struct
        # - Privilege field: Manage (4)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0x1000_0000_0000_0001, 0x1000_0000_0000_0002, 0x1000_0000_0000_0003, 0x1000_0000_0000_0004]
        # - Targets field: [{Cluster: 0xFFF1_FC00, DeviceType: 0xFFF1_FC20}, {Cluster: 0xFFF1_FC01, DeviceType: 0xFFF1_FC21}, {Cluster: 0xFFF1_FC02, DeviceType: 0xFFF1_FC22}]
        # . struct
        # - Privilege field: Operate (3)
        # - AuthMode field: CASE (2)
        # - Subjects field: [0x3000_0000_0000_0001, 0x3000_0000_0000_0002, 0x3000_0000_0000_0003, 0x3000_0000_0000_0004]
        # - Targets field: [{Cluster: 0xFFF1_FC40, DeviceType: 0xFFF1_FC20}, {Cluster: 0xFFF1_FC41, DeviceType: 0xFFF1_FC21}, {Cluster: 0xFFF1_FC02, DeviceType: 0xFFF1_FC42}]

        # Administer ACL entry
        admin_subjects = [0xFFFF_FFFD_0001_0001, 0x2000_0000_0000_0001, 0x2000_0000_0000_0002, 0x2000_0000_0000_0003]

        admin_targets = [
            Clusters.AccessControl.Structs.Target(endpoint=0),
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC00, deviceType=0xFFF1_BC30),
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC01, deviceType=0xFFF1_BC31)
        ]
        admin_acl_entry = Clusters.AccessControl.Structs.AccessControlEntry(privilege=Clusters.AccessControl.Enums.Privilege.kAdminister,
                                                                            authMode=Clusters.AccessControl.Enums.AuthMode.kCase,
                                                                            subjects=admin_subjects,
                                                                            targets=admin_targets)
        acl.append(admin_acl_entry)

        # Manage ACL entry
        manage_subjects = [0x1000_0000_0000_0001, 0x1000_0000_0000_0002, 0x1000_0000_0000_0003, 0x1000_0000_0000_0004]
        manage_targets = [
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC00, deviceType=0xFFF1_BC20),
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC01, deviceType=0xFFF1_BC21),
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC02, deviceType=0xFFF1_BC22)
        ]

        manage_acl_entry = Clusters.AccessControl.Structs.AccessControlEntry(privilege=Clusters.AccessControl.Enums.Privilege.kManage,
                                                                             authMode=Clusters.AccessControl.Enums.AuthMode.kCase,
                                                                             subjects=manage_subjects,
                                                                             targets=manage_targets)
        acl.append(manage_acl_entry)

        # Operate ACL entry
        operate_subjects = [0x3000_0000_0000_0001, 0x3000_0000_0000_0002, 0x3000_0000_0000_0003, 0x3000_0000_0000_0004]
        operate_targets = [
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC40, deviceType=0xFFF1_BC20),
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC41, deviceType=0xFFF1_BC21),
            Clusters.AccessControl.Structs.Target(cluster=0xFFF1_FC42, deviceType=0xFFF1_BC42)
        ]

        operate_acl_entry = Clusters.AccessControl.Structs.AccessControlEntry(privilege=Clusters.AccessControl.Enums.Privilege.kOperate,
                                                                              authMode=Clusters.AccessControl.Enums.AuthMode.kCase,
                                                                              subjects=operate_subjects,
                                                                              targets=operate_targets)
        acl.append(operate_acl_entry)

        return acl


if __name__ == "__main__":
    default_matter_test_main(maximize_cert_chains=True, controller_cat_tags=[0x0001_0001])
