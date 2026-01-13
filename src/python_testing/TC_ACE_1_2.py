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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
from matter.exceptions import ChipStackError
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TC_ACE_1_2(MatterBaseTest):

    def setup_class(self):
        super().setup_class()
        self.breadcrumb = 1

    async def write_acl(self, acl):
        # This returns an attribute status
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

    async def steps_subscribe_breadcrumb(self, print_steps: bool):
        """
        Step to subscribe to the Breadcrumb attribute from the DUT.

        This function starts a subscription to the Breadcrumb attribute of the GeneralCommissioning cluster and sets up an update callback for later verification.

        Parameters:
            print_steps (bool): If True, prints step descriptions.

        Returns:
            AttributeSubscriptionHandler: The callback object associated with the subscription.
        """

        if print_steps:
            self.print_step(3, "TH2 subscribes to the Breadcrumb attribute")
        subscription_breadcrumb = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id, attributes=[(0, Clusters.GeneralCommissioning.Attributes.Breadcrumb)],
            reportInterval=(1, 5), keepSubscriptions=False, autoResubscribe=False)
        breadcrumb_cb = AttributeSubscriptionHandler(expected_cluster=Clusters.GeneralCommissioning,
                                                     expected_attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)
        subscription_breadcrumb.SetAttributeUpdateCallback(breadcrumb_cb)
        return breadcrumb_cb

    async def steps_receive_breadcrumb(self, breadcrumb_cb: AttributeSubscriptionHandler, print_steps: bool):
        """
        Step that triggers a change to the Breadcrumb attribute and waits for the subscription report.

        This function writes a new valaue to the Breadcrumb attribute and waits for the AttributeSubscriptionHandler to receive and report the update.

        Parameters:
            breadcrumb_cb (AttributeSubscriptionHandler): The callback previously set up to track Breadcrumb updates.
            print_steps (bool): If True, prints step descriptions.
        """

        if print_steps:
            self.print_step(9, "TH1 writes the breadcrumb attribute")
        await self.default_controller.WriteAttribute(nodeId=self.dut_node_id, attributes=[(0, Clusters.GeneralCommissioning.Attributes.Breadcrumb(self.breadcrumb))])

        if print_steps:
            self.print_step(10, "TH2 waits for a subscription report from the DUT for breadcrumb")
        breadcrumb_cb.wait_for_attribute_report()
        self.breadcrumb = self.breadcrumb + 1

    async def steps_admin_subscription_error(self, print_steps: bool):
        """
        Step that validates error handling when subscribing to attributes/events with invalid permissions.

        This function attempts to subscribe to ACL and AccessControlEntryChanged attributes with a user that lacks the required permissions.
        It asserts that the expected ChipStackError is raised (INVALID_ACTION)

        Parameters:
            print_steps (bool): If True, prints step descriptions.
        """

        if print_steps:
            self.print_step(13, "Subscribe to the ACL attribute, expect INVALID_ACTION")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(nodeId=self.dut_node_id,
                                         attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
                                         reportInterval=(1, 5),
                                         fabricFiltered=False,
                                         keepSubscriptions=False,
                                         autoResubscribe=False)
        asserts.assert_equal(cm.exception.err, 0x580, "Incorrectly subscribed to attribute with invalid permissions")

        if print_steps:
            self.print_step(14, "Subscribe to the AccessControlEntryChanged event, expect INVALID_ACTION")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadEvent(nodeId=self.dut_node_id,
                                     events=[(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
                                     reportInterval=(1, 5),
                                     fabricFiltered=False,
                                     keepSubscriptions=False,
                                     autoResubscribe=False)
        asserts.assert_equal(cm.exception.err, 0x580, "Incorrectly subscribed to attribute with invalid permissions")

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
        breadcrumb_cb = await self.steps_subscribe_breadcrumb(print_steps=True)

        self.print_step(4, "TH2 subscribes to ACL attribute")
        subscription_acl = await self.TH2.ReadAttribute(nodeId=self.dut_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl)], reportInterval=(1, 5), fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        acl_cb = AttributeSubscriptionHandler(expected_cluster=Clusters.AccessControl,
                                              expected_attribute=Clusters.AccessControl.Attributes.Acl)
        subscription_acl.SetAttributeUpdateCallback(acl_cb)

        self.print_step(5, "TH2 subscribes to the AccessControlEntryChanged event")
        urgent = 1
        subscription_ace = await self.TH2.ReadEvent(nodeId=self.dut_node_id, events=[(0, Clusters.AccessControl.Events.AccessControlEntryChanged, urgent)], reportInterval=(1, 5), fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        event = Clusters.AccessControl.Events.AccessControlEntryChanged
        ace_cb = EventSubscriptionHandler(expected_cluster_id=event.cluster_id, expected_event_id=event.event_id)
        subscription_ace.SetEventUpdateCallback(ace_cb)

        self.print_step(6, "TH1 writes ACL attribute")
        acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[TH1_nodeid, TH2_nodeid, TH2_nodeid + 1],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0)])
        await self.write_acl([acl])

        self.print_step(7, "TH2 waits for subscription report for ACL")
        acl_cb.wait_for_attribute_report()

        self.print_step(8, "TH2 waits for subscription report for access control entry changed event")
        ace_cb.wait_for_event_report(Clusters.AccessControl.Events.AccessControlEntryChanged)

        # this function prints the steps for 9 and 10
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=True)

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
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=False)

        # step 13 and 14 - printed in the function
        await self.steps_admin_subscription_error(print_steps=True)

        self.print_step(15, "TH2 subscribes to breadcrumb attribute")
        breadcrumb_cb = await self.steps_subscribe_breadcrumb(print_steps=False)

        self.print_step(16, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=False)

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
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=False)

        self.print_step(19, "TH2 repeats subscriptions to Admin attribute and event to ensure it still errors")
        await self.steps_admin_subscription_error(print_steps=False)

        self.print_step(20, "TH2 subscribes to breadcrumb attribute")
        breadcrumb_cb = await self.steps_subscribe_breadcrumb(print_steps=False)

        self.print_step(21, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=False)

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
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=False)

        self.print_step(24, "TH2 repeats subscriptions to Admin attribute and event to ensure it still errors")
        await self.steps_admin_subscription_error(print_steps=False)

        self.print_step(25, "TH2 subscribes to breadcrumb attribute")
        breadcrumb_cb = await self.steps_subscribe_breadcrumb(print_steps=False)

        self.print_step(26, "TH2 Repeats steps to change breadcrumb and receive subscription report")
        await self.steps_receive_breadcrumb(breadcrumb_cb, print_steps=False)

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

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(nodeId=self.dut_node_id,
                                         attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
                                         reportInterval=(1, 5),
                                         fabricFiltered=False,
                                         keepSubscriptions=False,
                                         autoResubscribe=False)
        asserts.assert_equal(cm.exception.err, 0x580, "Incorrectly subscribed to attribute with invalid permissions")


if __name__ == "__main__":
    default_matter_test_main()
