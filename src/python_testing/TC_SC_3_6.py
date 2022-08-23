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
from chip.clusters.Attribute import TypedAttributePath, SubscriptionTransaction
import queue
import asyncio
from threading import Event
import time

# TODO: Overall, we need to add validation that session IDs have not changed throughout to be agnostic
#       to some internal behavior assumptions of the SDK we are making relative to the write to
#       the trigger the subscriptions not re-opening a new CASE session
#


class AttributeChangeAccumulator:
    def __init__(self, name: str, expected_attribute: Clusters.ClusterAttributeDescriptor, output: queue.Queue):
        self._name = name
        self._output = output
        self._expected_attribute = expected_attribute

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        if path.AttributeType == self._expected_attribute:
            data = transaction.GetAttribute(path)

            value = {
                'name': self._name,
                'endpoint': path.Path.EndpointId,
                'attribute': path.AttributeType,
                'value': data
            }
            logging.info("Got subscription report on client %s for %s: %s" % (self.name, path.AttributeType, data))
            self._output.put(value)

    @property
    def name(self) -> str:
        return self._name


class ResubscriptionCatcher:
    def __init__(self, name):
        self._name = name
        self._got_resubscription_event = Event()

    def __call__(self, transaction: SubscriptionTransaction, terminationError, nextResubscribeIntervalMsec):
        self._got_resubscription_event.set()
        logging.info("Got resubscription on client %s" % self.name)

    @property
    def name(self) -> str:
        return self._name

    @property
    def caught_resubscription(self) -> bool:
        return self._got_resubscription_event.is_set()


class TC_SC_3_6(MatterBaseTest):
    def setup_class(self):
        self._subscriptions = []

    def teardown_class(self):
        logging.info("Teardown: shutting down all subscription to avoid racy callbacks")
        for subscription in self._subscriptions:
            subscription.Shutdown()

    @async_test_body
    async def test_TC_SC_3_6(self):
        dev_ctrl = self.default_controller

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

        BEFORE_LABEL = "Before Subscriptions"
        AFTER_LABEL = "After Subscriptions"

        # Generate list of all clients names
        all_names = []
        for fabric_idx in range(num_fabrics_to_commission):
            for controller_idx in range(num_controllers_per_fabric):
                all_names.append("RD%d%s" % (fabric_idx + 1, chr(ord('A') + controller_idx)))
        logging.info("Client names that will be used: %s" % all_names)
        client_list = []

        logging.info("Pre-conditions: validate CapabilityMinima.CaseSessionsPerFabric >= 3")

        capability_minima = await self.read_single_attribute(dev_ctrl, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.Basic.Attributes.CapabilityMinima)
        asserts.assert_greater_equal(capability_minima.caseSessionsPerFabric, 3)

        logging.info("Pre-conditions: use existing fabric to configure new fabrics so that total is %d fabrics" %
                     num_fabrics_to_commission)

        # Generate Node IDs for subsequent controllers start at 200, follow 200, 300, ...
        node_ids = [200 + (i * 100) for i in range(num_controllers_per_fabric - 1)]

        # Prepare clients for first fabric, that includes the default controller
        dev_ctrl.name = all_names.pop(0)
        client_list.append(dev_ctrl)

        if num_controllers_per_fabric > 1:
            new_controllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(fabricAdmin=dev_ctrl.fabricAdmin, adminDevCtrl=dev_ctrl, controllerNodeIds=node_ids, privilege=Clusters.AccessControl.Enums.Privilege.kAdminister, targetNodeId=self.dut_node_id)
            for controller in new_controllers:
                controller.name = all_names.pop(0)
            client_list.extend(new_controllers)

        # Prepare clients for subsequent fabrics
        for i in range(num_fabrics_to_commission - 1):
            admin_index = 2 + i
            logging.info("Commissioning fabric %d/%d" % (admin_index, num_fabrics_to_commission))
            new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
            new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
            new_admin_ctrl = new_fabric_admin.NewController(nodeId=dev_ctrl.nodeId)
            new_admin_ctrl.name = all_names.pop(0)
            client_list.append(new_admin_ctrl)
            await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=new_admin_ctrl, existingNodeId=self.dut_node_id, newNodeId=self.dut_node_id)

            if num_controllers_per_fabric > 1:
                new_controllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(fabricAdmin=new_fabric_admin, adminDevCtrl=new_admin_ctrl,
                                                                                              controllerNodeIds=node_ids, privilege=Clusters.AccessControl.Enums.Privilege.kAdminister, targetNodeId=self.dut_node_id)
                for controller in new_controllers:
                    controller.name = all_names.pop(0)

                client_list.extend(new_controllers)

        asserts.assert_equal(len(client_list), num_fabrics_to_commission *
                             num_controllers_per_fabric, "Must have the right number of clients")

        # Before subscribing, set the NodeLabel to "Before Subscriptions"
        logging.info("Pre-conditions: writing initial value of NodeLabel, so that we can control for change of attribute detection")
        await client_list[0].WriteAttribute(self.dut_node_id, [(0, Clusters.Basic.Attributes.NodeLabel(value=BEFORE_LABEL))])

        # Subscribe with all clients to NodeLabel attribute
        sub_handlers = []
        resub_catchers = []
        output_queue = queue.Queue()

        logging.info("Step 1 (first part): Establish subscription with all %d clients" % len(client_list))
        for sub_idx, client in enumerate(client_list):
            logging.info("Establishing subscription %d/%d from controller node %s" % (sub_idx + 1, len(client_list), client.name))

            sub = await client.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.Basic.Attributes.NodeLabel)],
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

        # Trigger a change on NodeLabel
        logging.info(
            "Step 1 (second part): Change attribute with one client, await all attributes changed within time")
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

        logging.info("Validation of results")
        failed = False

        for catcher in resub_catchers:
            if catcher.caught_resubscription:
                logging.error("Client %s saw a resubscription" % catcher.name)
                failed = True
            else:
                logging.info("Client %s correctly did not see a resubscription" % catcher.name)

        all_reports_gotten = all(all_changes.values())
        if not all_reports_gotten:
            logging.error("Missing reports from the following clients: %s" %
                          ", ".join([name for name, value in all_changes.items() if value is False]))
            failed = True
        else:
            logging.info("Got successful reports from all clients, meaning all concurrent CASE sessions worked")

        # Determine final result
        if failed:
            asserts.fail("Failed test !")

        # Pass is implicit if not failed


if __name__ == "__main__":
    default_matter_test_main()
