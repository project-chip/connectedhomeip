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
import queue

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class AttributeChangeCallback:
    def __init__(self, expected_attribute: ClusterObjects.ClusterAttributeDescriptor, output: queue.Queue):
        self._output = output
        self._expected_attribute = expected_attribute

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        if path.AttributeType == self._expected_attribute:
            q = (path, transaction)
            logging.info(f'Got subscription report for {path.AttributeType}')
            self._output.put(q)


class EventChangeCallback:
    def __init__(self, expected_event: ClusterObjects.ClusterEvent, output: queue.Queue):
        self._output = output
        self._expected_cluster_id = expected_event.cluster_id
        self._expected_event_id = expected_event.event_id

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster_id and res.Header.EventId == self._expected_event_id:
            logging.info(
                f'Got subscription report for event {self._expected_event_id} on cluster {self._expected_cluster_id}: {res.Data}')
            self._output.put(res)


def WaitForAttributeReport(q: queue.Queue, expected_attribute: ClusterObjects.ClusterAttributeDescriptor):
    try:
        path, transaction = q.get(block=True, timeout=10)
    except queue.Empty:
        asserts.fail("Failed to receive a report for the attribute change for {}".format(expected_attribute))

    asserts.assert_equal(path.AttributeType, expected_attribute, "Received incorrect attribute report")
    try:
        transaction.GetAttribute(path)
    except KeyError:
        asserts.fail("Attribute not found in returned report")


def WaitForEventReport(q: queue.Queue, expected_event: ClusterObjects.ClusterEvent):
    try:
        res = q.get(block=True, timeout=10)
    except queue.Empty:
        asserts.fail("Failed to receive a report for the event {}".format(expected_event))

    asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
    asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")


class TC_ACE_1_2(MatterBaseTest):

    def setup_class(self):
        super().setup_class()
        self.breadcrumb = 1
        self.breadcrumb_queue = queue.Queue()

    async def write_acl(self, acl):
        # This returns an attribute status
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

    async def steps_subscribe_breadcrumb(self, print_steps: bool):
        if print_steps:
            self.print_step(3, "TH2 subscribes to the Breadcrumb attribute")
        subscription_breadcrumb = await self.TH2.ReadAttribute(
            nodeid=self.dut_node_id, attributes=[(0, Clusters.GeneralCommissioning.Attributes.Breadcrumb)],
            reportInterval=(1, 5), keepSubscriptions=False, autoResubscribe=False)
        breadcrumb_cb = AttributeChangeCallback(Clusters.GeneralCommissioning.Attributes.Breadcrumb, self.breadcrumb_queue)
        subscription_breadcrumb.SetAttributeUpdateCallback(breadcrumb_cb)

    async def steps_receive_breadcrumb(self, print_steps: bool):
        if print_steps:
            self.print_step(9, "TH1 writes the breadcrumb attribute")
        await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.GeneralCommissioning.Attributes.Breadcrumb(self.breadcrumb))])

        if print_steps:
            self.print_step(10, "TH2 waits for a subscription report from the DUT for breadcrumb")
        WaitForAttributeReport(self.breadcrumb_queue, Clusters.GeneralCommissioning.Attributes.Breadcrumb)
        self.breadcrumb = self.breadcrumb + 1

    async def steps_admin_subscription_error(self, print_steps: bool):
        if print_steps:
            self.print_step(13, "Subscribe to the ACL attribute, expect INVALID_ACTION")
        try:
            await self.TH2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl)], reportInterval=(1, 5), fabricFiltered=False, keepSubscriptions=False, autoResubscribe=False)
            asserts.fail("Incorrectly subscribed to attribute with invalid permissions")
        except ChipStackError as e:
            asserts.assert_equal(e.err, 0x580, "Incorrect error message received from subscription with no permission")

        if print_steps:
            self.print_step(14, "Subscribe to the AccessControlEntryChanged event, expect INVALID_ACTION")
        try:
            await self.TH2.ReadEvent(nodeid=self.dut_node_id, events=[(0, Clusters.AccessControl.Events.AccessControlEntryChanged)], reportInterval=(
                1, 5), fabricFiltered=False, keepSubscriptions=False, autoResubscribe=False)
            asserts.fail("Incorrectly subscribed to event with invalid permissions")
        except ChipStackError as e:
            asserts.assert_equal(e.err, 0x580, "Incorrect error message received from subscription with no permission")

    @async_test_body
    async def test_TC_ACE_1_2(self):
        self.print_step(1, "Commissioning, already done")

        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]

        TH1_nodeid = self.matter_test_config.controller_node_id
        TH2_nodeid = self.matter_test_config.controller_node_id + 1

        self.TH2 = fabric_admin.NewController(nodeId=TH2_nodeid,
                                              paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

        self.print_step(2, "TH1 writes ACL for admin with two subjects")
        TH1_2_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid, TH2_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        await self.write_acl([TH1_2_admin])

        # Step 3 - subscribe to breadcrumb - print handled in function
        await self.steps_subscribe_breadcrumb(print_steps=True)

        self.print_step(4, "TH2 subscribes to ACL attribute")
        subscription_acl = await self.TH2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl)], reportInterval=(1, 5), fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        acl_queue = queue.Queue()
        acl_cb = AttributeChangeCallback(Clusters.AccessControl.Attributes.Acl, acl_queue)
        subscription_acl.SetAttributeUpdateCallback(acl_cb)

        self.print_step(5, "TH2 subscribes to the AccessControlEntryChanged event")
        urgent = 1
        subscription_ace = await self.TH2.ReadEvent(nodeid=self.dut_node_id, events=[(0, Clusters.AccessControl.Events.AccessControlEntryChanged, urgent)], reportInterval=(1, 5), fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        ace_queue = queue.Queue()
        ace_cb = EventChangeCallback(Clusters.AccessControl.Events.AccessControlEntryChanged, ace_queue)
        subscription_ace.SetEventUpdateCallback(ace_cb)

        self.print_step(6, "TH1 writes ACL attribute")
        acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid, TH2_nodeid, TH2_nodeid + 1],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        await self.write_acl([acl])

        self.print_step(7, "TH2 waits for subscription report for ACL")
        WaitForAttributeReport(acl_queue, Clusters.AccessControl.Attributes.Acl)

        self.print_step(8, "TH2 waits for subscription report for access control entry changed event")
        WaitForEventReport(ace_queue, Clusters.AccessControl.Events.AccessControlEntryChanged)

        # this function prints the steps for 9 and 10
        await self.steps_receive_breadcrumb(print_steps=True)

        self.print_step(11, "TH1 writes ACL attribute")
        acl1 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        acl2 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH2_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        await self.write_acl([acl1, acl2])

        self.print_step(12, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(print_steps=False)

        # step 13 and 14 - printed in the function
        await self.steps_admin_subscription_error(print_steps=True)

        self.print_step(15, "TH2 subscribes to breadcrumb attribute")
        await self.steps_subscribe_breadcrumb(print_steps=False)

        self.print_step(16, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(print_steps=False)

        self.print_step(17, "TH1 writes ACL attribute")
        acl1 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        acl2 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH2_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        await self.write_acl([acl1, acl2])

        self.print_step(18, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(print_steps=False)

        self.print_step(19, "TH2 repeats subscriptions to Admin attribute and event to ensure it still errors")
        await self.steps_admin_subscription_error(print_steps=False)

        self.print_step(20, "TH2 subscribes to breadcrumb attribute")
        await self.steps_subscribe_breadcrumb(print_steps=False)

        self.print_step(21, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(print_steps=False)

        self.print_step(22, "TH1 writes ACL attribute")
        acl1 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        acl2 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH2_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        await self.write_acl([acl1, acl2])

        self.print_step(23, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(print_steps=False)

        self.print_step(24, "TH2 repeats subscriptions to Admin attribute and event to ensure it still errors")
        await self.steps_admin_subscription_error(print_steps=False)

        self.print_step(25, "TH2 subscribes to breadcrumb attribute")
        await self.steps_subscribe_breadcrumb(print_steps=False)

        self.print_step(26, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(print_steps=False)

        self.print_step(27, "TH1 writes ACL attribute")
        acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid],
            targets=[])
        await self.write_acl([acl])

        self.print_step(28, "TH2 repeats subscriptions to Admin attribute and event to ensure it still errors")
        await self.steps_admin_subscription_error(print_steps=False)

        self.print_step(29, "TH2 attempts to subscribe to the breadcrumb attribute - expect error")
        try:
            await self.TH2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.GeneralCommissioning.Attributes.Breadcrumb)], reportInterval=(1, 5), keepSubscriptions=False, autoResubscribe=False)
            asserts.fail("Incorrectly subscribed to attribute with invalid permissions")
        except ChipStackError as e:
            asserts.assert_equal(e.err, 0x580, "Incorrect error message received from subscription with no permission")


if __name__ == "__main__":
    default_matter_test_main()
