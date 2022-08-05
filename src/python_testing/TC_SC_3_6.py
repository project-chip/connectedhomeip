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

from unicodedata import name
from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
from chip.interaction_model import Status
import chip.clusters as Clusters
import chip.FabricAdmin
import logging
from mobly import asserts
from chip.utils import CommissioningBuildingBlocks
from chip.clusters.Attribute import TypedAttributePath, SubscriptionTransaction
import asyncio
import queue
from threading import Event
import time


class ResubscriptionCatcher:
    def __init__(self, name):
        self._name = name
        self._got_resubscription_event = Event()

    async def __call__(self, transaction: SubscriptionTransaction, terminationError, nextResubscribeIntervalMsec):
        self._got_resubscription_event.set()
        logging.info("Got resubscription on client %s" % self.name)

    @property
    def name(self) -> str:
        return self._name

    @property
    def caught_resubscription(self) -> bool:
        return self._got_resubscription_event.is_set()


class AttributeChangeAccumulator:
    def __init__(self, name):
        self._name = name
        self._all_data = queue.Queue()

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        data = transaction.GetAttribute(path)
        value = {
            'sub_name': self._name,
            'endpoint': path.Path.EndpointId,
            'attribute': path.AttributeType,
            'value': data
        }
        self._all_data.put(value)
        logging.info("Got subscription report on client %s: %s" % (self.name, value))

    @property
    def all_data(self):
        data = []
        while True:
            try:
                data.append(self._all_data.get(block=False))
            except queue.Empty:
                break
        return data

    @property
    def name(self) -> str:
        return self._name


class TC_SC_3_6(MatterBaseTest):
    @async_test_body
    async def test_TC_SC_3_6(self):
        dev_ctrl = self.default_controller

        # Get overrides for debugging the test
        num_fabrics_to_commission = self.user_params.get("num_fabrics_to_commission", 5)
        num_controllers_per_fabric = self.user_params.get("num_controllers_per_fabric", 3)
        min_report_interval_sec = self.user_params.get("min_report_interval_sec", 1)
        max_report_interval_sec = self.user_params.get("max_report_interval_sec", 30)
        # Time to wait after changing NodeLabel for subscriptions to all hit
        stabilization_delay_sec = self.user_params.get("stabilization_delay_sec", 10)
        sub_liveness_override_ms = self.user_params.get("sub_liveness_override_ms", None)

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

        # Generate Node IDs for subsequent for subsequent controllers start at 200, follow 200, 300, ...
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
            new_fabric_admin = chip.FabricAdmin.FabricAdmin(vendorId=0xFFF1, adminIndex=admin_index)
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

        # Before subscribing, set the NodeBabel to "Before Subscriptions"
        logging.info("Pre-conditions: writing initial value of NodeLabel, so that we can control for change of attribute detection")
        await client_list[0].WriteAttribute(self.dut_node_id, [(0, Clusters.Basic.Attributes.NodeLabel(value=BEFORE_LABEL))])

        # Subscribe with all clients to NodeLabel attribute
        subscriptions = []
        sub_handlers = []
        resub_catchers = []

        logging.info("Step 1 (first part): Establish subscription with all 15 clients")
        for client in client_list:
            logging.info("Establishing subscription from controller node %s" % client.name)
            sub = await client.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.Basic.Attributes.NodeLabel)],
                                             reportInterval=(min_report_interval_sec, max_report_interval_sec), keepSubscriptions=False)
            subscriptions.append(sub)

            attribute_handler = AttributeChangeAccumulator(name=client.name)
            sub.SetAttributeUpdateCallback(attribute_handler)
            sub_handlers.append(attribute_handler)

            resub_catcher = ResubscriptionCatcher(name=client.name)
            sub.SetResubscriptionAttemptedCallback(resub_catcher, isAsync=True)
            resub_catchers.append(resub_catcher)

            if sub_liveness_override_ms is not None:
                logging.warning("Overriding subscription liveness to check %dms! NOT FOR CERTIFICATION!" % sub_liveness_override_ms)
                sub.OverrideLivenessTimeoutMs(sub_liveness_override_ms)

        asserts.assert_equal(len(subscriptions), num_fabrics_to_commission *
                             num_controllers_per_fabric, "Must have the right number of subscriptions")

        # Trigger a change on NodeLabel
        logging.info("Step 1 (second part): Change attribute with one client, and validate all clients observe the change on same session")
        await asyncio.sleep(1)
        await client_list[0].WriteAttribute(self.dut_node_id, [(0, Clusters.Basic.Attributes.NodeLabel(value=AFTER_LABEL))])

        # Await a stabilization delay in increments to let the coroutines run
        start_time = time.time()
        while (time.time() - start_time) < stabilization_delay_sec:
            await asyncio.sleep(0.05)

        # After stabilization, validate no resubscriptions and all nodes have seen an update

        logging.info("Validation of results")
        # First check: all subs seeing update
        failed = False
        for handler in sub_handlers:
            data_update_count = 0
            for item in handler.all_data:
                if item['value'] == AFTER_LABEL:
                    data_update_count += 1

            if data_update_count == 0:
                logging.error("Client %s did not see subscription update" % handler.name)
                failed = True
            elif data_update_count > 1:
                logging.error("Client %s saw %d updates instead of 1" % (handler.name, data_update_count))
                failed = True
            else:
                logging.info("Client %s successfully saw 1 update" % handler.name)

        # Second check: no resubscriptions
        for catcher in resub_catchers:
            if catcher.caught_resubscription:
                logging.error("Client %s saw a resubscription" % catcher.name)
                failed = True
            else:
                logging.error("Client %s correctly did not see a resubscription" % catcher.name)

        # Determine final result
        if failed:
            asserts.fail("Failed test !")

        # Pass is implicit if not failed


if __name__ == "__main__":
    default_matter_test_main()
