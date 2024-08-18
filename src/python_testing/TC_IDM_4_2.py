#
#    Copyright (c) 2023 Project CHIP Authors
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

import copy
import logging
import time

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AttributePath, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from matter_testing_support import AttributeChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

'''
Category:
Functional

Description:
Validates Interaction Data Model (IDM), specifically subscription responses. Some example of tests run:
  - Subscriptions with varying MaxIntervalCeiling
  - Checks for `InvalidAction` results when subscribing to clusters and attributes without access rights
  - Checks that subscription is not established for invalid MinIntervalFloor
  - Validates that only correctly filtered data is received when a subscription is established

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#tc-idm-4-2-subscription-response-messages-from-dut-test-cases-dut_server
'''


class TC_IDM_4_2(MatterBaseTest):

    def steps_TC_IDM_4_2(self):
        return [TestStep(0, "CR1 reads the ServerList attribute from the Descriptor cluster on EP0.",
                         "If the ICD Management cluster ID (70,0x46) is present, set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = IdleModeDuration and min_interval_floor_s to 0, otherwise, set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = 60 mins and min_interval_floor_s to 3."),
                TestStep(1, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than subscription_max_interval_publisher_limit_sec. DUT sends a report data action to the TH. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.",
                         "Verify on the CR1, a report data message is received. Verify it contains the following data Report data - data of the attribute/event requested earlier. Verify on the CR1 the Subscribe Response has the following fields, SubscriptionId - Verify it is of type uint32. MaxInterval - Verify it is of type uint32. Verify that the MaxInterval is less than or equal to MaxIntervalCeiling."),
                TestStep(2, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value less than subscription_max_interval_publisher_limit_sec. DUT sends a report data action to the CR1. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.",
                         "Verify on the CR1, a report data message is received. Verify it contains the following data: Report data - data of the attribute/event requested earlier. Verify on the CR1 the Subscribe Response has the following fields, SubscriptionId - Verify it is of type uint32. MaxInterval - Verify it is of type uint32. Verify that the MaxInterval is less than or equal to SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT."),
                TestStep(3, "Setup CR2 such that it does not have access to a specific cluster. CR2 sends a subscription message to subscribe to an attribute on that cluster for which it does not have access.",
                         "Verify that the DUT returns a \"INVALID_ACTION\" status response."),
                TestStep(4, "Setup CR2 such that it does not have access to all attributes on a specific cluster and endpoint. CR2 sends a subscription request to subscribe to all attributes for which it does not have access.",
                         "Verify that the DUT returns a \"INVALID_ACTION\" status response."),
                TestStep(5, "Setup CR2 such that it does not have access to an Endpoint. CR2 sends a subscription request to subscribe to all attributes on all clusters on a specific Endpoint for which it does not have access.",
                         "Verify that the DUT returns a \"INVALID_ACTION\" status response."),
                TestStep(6, "Setup CR2 such that it does not have access to the Node. CR2 sends a subscription request to subscribe to all attributes on all clusters on all endpoints on a Node for which it does not have access.",
                         "Verify that the DUT returns a \"INVALID_ACTION\" status response."),
                TestStep(7, "CR1 sends a subscription request action for an attribute with an empty DataVersionFilters field. DUT sends a report data action with the data of the attribute along with the data version. Tear down the subscription for that attribute. Start another subscription with the DataVersionFilter field set to the data version received above.",
                         "Verify that the subscription is activated between CR1 and DUT."),
                TestStep(8, "CR1 sends a subscription request action for an attribute and sets the MinIntervalFloor to min_interval_floor_sec and MaxIntervalCeiling to 10. Activate the Subscription between CR1 and DUT and record the time when the priming ReportDataMessage is received as t_report_sec. Save the returned MaxInterval from the SubscribeResponseMessage as max_interval_sec."),
                TestStep(9, "CR1 modifies the attribute which has been subscribed to on the DUT and waits for an incoming ReportDataMessage",
                         "Verify that t_update - t_report is greater than min_interval_floor_s and less than the ReadClient SubscriptionTimeout (calculated by the ReadClient using max_interval_s and the maximum estimated network delay based on the MRP parameters for retries with backoff)"),
                TestStep(10, "CR1 sends a subscription request action for an attribute and set the MinIntervalFloor value to be greater than MaxIntervalCeiling.",
                         "Verify that the DUT sends an error message and the subscription is not established."),
                TestStep(11, "CR1 sends a subscription request to subscribe to a specific global attribute from all clusters on all endpoints.",
                         "Verify that the Subscription succeeds and the DUT sends back the attribute values for the global attribute."),
                TestStep(12, "CR1 sends a subscription request to subscribe to a global attribute on an endpoint on all clusters.",
                         "Verify that the Subscription succeeds and the DUT sends back the attribute values for the global attribute. Verify no data from other endpoints is sent back."),
                TestStep(13, "CR1 sends a subscription request to the DUT with both AttributeRequests and EventRequests as empty.",
                         "Verify that the Subscription does not succeed and the DUT sends back a Status Response Action with the INVALID_ACTION Status Code")
                ]

    ROOT_NODE_ENDPOINT_ID = 0

    async def get_descriptor_server_list(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_descriptor_parts_list(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList
        )

    async def get_idle_mode_duration_sec(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.IdleModeDuration
        )

    @staticmethod
    def verify_attribute_exists(sub, cluster, attribute, ep=ROOT_NODE_ENDPOINT_ID):
        sub_attrs = sub
        if isinstance(sub, Clusters.Attribute.SubscriptionTransaction):
            sub_attrs = sub.GetAttributes()

        asserts.assert_true(ep in sub_attrs, "Must have read endpoint %s data" % ep)
        asserts.assert_true(cluster in sub_attrs[ep], "Must have read %s cluster data" % cluster.__name__)
        asserts.assert_true(attribute in sub_attrs[ep][cluster],
                            "Must have read back attribute %s" % attribute.__name__)

    @staticmethod
    def get_typed_attribute_path(attribute, ep=ROOT_NODE_ENDPOINT_ID):
        return TypedAttributePath(
            Path=AttributePath.from_attribute(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    async def write_dut_acl(self, ctrl, acl, ep=ROOT_NODE_ENDPOINT_ID):
        result = await ctrl.WriteAttribute(self.dut_node_id, [(ep, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[ep].Status, Status.Success, "ACL write failed")

    async def get_dut_acl(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        sub = await ctrl.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ep, Clusters.AccessControl.Attributes.Acl)],
            keepSubscriptions=False,
            fabricFiltered=True
        )

        acl_list = sub[ep][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        return acl_list

    async def add_ace_to_dut_acl(self, ctrl, ace, dut_acl_original):
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        await self.write_dut_acl(ctrl=ctrl, acl=dut_acl)

    @staticmethod
    def is_valid_uint32_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFFFFFFFF

    @staticmethod
    def is_valid_uint16_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFFFF

    @async_test_body
    async def test_TC_IDM_4_2(self):

        # Test setup
        cluster_rev_attr = Clusters.BasicInformation.Attributes.ClusterRevision
        cluster_rev_attr_typed_path = self.get_typed_attribute_path(cluster_rev_attr)
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        node_label_attr_path = [(0, node_label_attr)]
        subscription_max_interval_publisher_limit_sec = 0
        INVALID_ACTION_ERROR_CODE = 0x580

        # Controller 1 setup
        # Subscriber/client with admin access to the DUT
        # Will write ACL for controller 2 and validate success/error codes
        CR1: ChipDeviceController = self.default_controller

        # Original DUT ACL used for reseting the ACL on some steps
        dut_acl_original = await self.get_dut_acl(CR1)

        # Controller 2 setup
        # Subscriber/client with limited access to the DUT
        # Will validate error status codes
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        CR2_nodeid = self.matter_test_config.controller_node_id + 1
        CR2: ChipDeviceController = fabric_admin.NewController(
            nodeId=CR2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        # *** Step 0 ***
        # CR1 reads the ServerList attribute from the Descriptor cluster on EP0. If the ICDManagement cluster ID
        # (70,0x46) is present, set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = IdleModeDuration and
        # min_interval_floor_s to 0, otherwise, set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = 60 mins and
        # min_interval_floor_s to 3.
        self.step(0)

        # Reads the ServerList attribute
        ep0_servers = await self.get_descriptor_server_list(CR1)

        # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        if Clusters.IcdManagement.id in ep0_servers:
            # Read the IdleModeDuration attribute value from the DUT
            logging.info(
                "CR1 reads from the DUT the IdleModeDuration attribute and sets SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = IdleModeDuration")

            idleModeDuration = await self.get_idle_mode_duration_sec(CR1)
            subscription_max_interval_publisher_limit_sec = idleModeDuration
            min_interval_floor_sec = 0
        else:
            # Defaulting SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC to 60 minutes
            subscription_max_interval_publisher_limit_sec = 60 * 60
            min_interval_floor_sec = 3

        asserts.assert_greater_equal(subscription_max_interval_publisher_limit_sec, 1,
                                     "SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC must be at least 1")

        logging.info(
            f"Set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC to {subscription_max_interval_publisher_limit_sec} seconds")

        # *** Step 1 ***
        # CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than
        # SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC. DUT sends a report data action to the TH. CR1 sends
        # a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate
        # the subscription.
        self.step(1)

        max_interval_ceiling_sec = subscription_max_interval_publisher_limit_sec + 5
        asserts.assert_greater(max_interval_ceiling_sec, min_interval_floor_sec,
                               "MaxIntervalCeiling must be greater than MinIntervalFloor")

        # Subscribe to attribute
        sub_cr1_step1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Verify attribute came back
        self.verify_attribute_exists(
            sub=sub_cr1_step1,
            cluster=Clusters.BasicInformation,
            attribute=node_label_attr
        )

        # Verify subscriptionId is of uint32 type
        asserts.assert_true(self.is_valid_uint32_value(sub_cr1_step1.subscriptionId), "subscriptionId is not of uint32 type.")

        # Verify MaxInterval is of uint32 type
        sub_cr1_step1_intervals = sub_cr1_step1.GetReportingIntervalsSeconds()
        sub_cr1_step1_min_interval_floor_sec, sub_cr1_step1_max_interval_ceiling_sec = sub_cr1_step1_intervals
        asserts.assert_true(self.is_valid_uint32_value(sub_cr1_step1_max_interval_ceiling_sec),
                            "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_less_equal(sub_cr1_step1_max_interval_ceiling_sec, max_interval_ceiling_sec,
                                  "MaxInterval is not less than or equal to MaxIntervalCeiling")

        sub_cr1_step1.Shutdown()

        # *** Step 2 ***
        # CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value less than
        # subscription_max_interval_publisher_limit_sec. DUT sends a report data action to the CR1.
        # CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the
        # CR1 to activate the subscription.
        self.step(2)

        min_interval_floor_sec = 0

        max_interval_ceiling_sec = max(1, subscription_max_interval_publisher_limit_sec - 5)
        asserts.assert_greater(max_interval_ceiling_sec, min_interval_floor_sec,
                               "MaxIntervalCeiling must be greater than MinIntervalFloor")

        # Subscribe to attribute
        sub_cr1_step2 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Verify attribute came back
        self.verify_attribute_exists(
            sub=sub_cr1_step2,
            cluster=Clusters.BasicInformation,
            attribute=node_label_attr
        )

        # Verify subscriptionId is of uint32 type
        asserts.assert_true(self.is_valid_uint32_value(sub_cr1_step2.subscriptionId), "subscriptionId is not of uint32 type.")

        # Verify MaxInterval is of uint32 type
        sub_cr1_step2_intervals = sub_cr1_step2.GetReportingIntervalsSeconds()
        sub_cr1_step2_min_interval_floor_sec, sub_cr1_step2_max_interval_ceiling_sec = sub_cr1_step2_intervals
        asserts.assert_true(self.is_valid_uint32_value(sub_cr1_step2_max_interval_ceiling_sec),
                            "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_less_equal(sub_cr1_step2_max_interval_ceiling_sec, subscription_max_interval_publisher_limit_sec,
                                  "MaxInterval is not less than or equal to SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC")

        sub_cr1_step2.Shutdown()

        # *** Step 3 ***
        # Setup CR2 such that it does not have access to a specific cluster. CR2 sends a subscription
        # message to subscribe to an attribute on that cluster for which it does not have access.
        self.step(3)

        # Setting max_interval_ceiling_sec value for steps 3-8
        max_interval_ceiling_sec = 10

        # Limited ACE for controller 2 with single cluster access
        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.BasicInformation.id)],
            subjects=[CR2_nodeid])

        # Add limited ACE
        await self.add_ace_to_dut_acl(CR1, CR2_limited_ace, dut_acl_original)

        # Controller 2 tries to subscribe an attribute from a cluster
        # it doesn't have access to
        # "INVALID_ACTION" status response expected

        try:
            await CR2.ReadAttribute(
                nodeid=self.dut_node_id,
                # Attribute from a cluster controller 2 has no access to
                attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
                keepSubscriptions=False,
                reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
                autoResubscribe=False
            )
            asserts.fail("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed cluster")

        # *** Step 4 ***
        # Setup CR2 such that it does not have access to all attributes on a specific cluster and
        # endpoint. CR2 sends a subscription request to subscribe to all attributes for which it
        # does not have access.
        self.step(4)

        # Limited ACE for controller 2 with single cluster access and specific endpoint
        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=1,
                cluster=Clusters.Descriptor.id)],
            subjects=[CR2_nodeid])

        # Add limited ACE
        await self.add_ace_to_dut_acl(CR1, CR2_limited_ace, dut_acl_original)

        # Controller 2 tries to subscribe to all attributes from a cluster
        # it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                nodeid=self.dut_node_id,
                # Cluster controller 2 has no access to
                attributes=[(0, Clusters.BasicInformation)],
                keepSubscriptions=False,
                reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
                autoResubscribe=False
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed cluster")

        await self.write_dut_acl(CR1, dut_acl_original)
        acl_list = await self.get_dut_acl(CR1)
        print(f'acl_list - reset 4: {acl_list}')

        # *** Step 5 ***
        # Setup CR2 such that it does not have access to an Endpoint. CR2 sends a subscription
        # request to subscribe to all attributes on all clusters on a specific Endpoint for which
        # it does not have access.
        self.step(5)

        # Get first value of parts list for the endpoint
        parts_list = await self.get_descriptor_parts_list(CR1)
        asserts.assert_greater(len(parts_list), 0, "Parts list is empty.")
        endpoint = parts_list[0]

        # Limited ACE for controller 2 with endpoint 1 access only to all clusters and all attributes
        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=endpoint)],
            subjects=[CR2_nodeid])

        # Add limited ACE
        await self.add_ace_to_dut_acl(CR1, CR2_limited_ace, dut_acl_original)

        # Controller 2 tries to subscribe to all attributes from all clusters
        # on an endpoint it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                nodeid=self.dut_node_id,
                # Endpoint controller 2 has no access to
                attributes=[(0)],
                keepSubscriptions=False,
                reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
                autoResubscribe=False
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed endpoint")

        # *** Step 6 ***
        # Setup CR2 such that it does not have access to the Node. CR2 sends a subscription
        # request to subscribe to all attributes on all clusters on all endpoints on a Node
        # for which it does not have access.
        self.step(6)

        # Skip setting an ACE for controller 2 so the DUT node rejects subscribing to it

        # Restore original DUT ACL
        await self.write_dut_acl(CR1, dut_acl_original)

        # Controller 2 tries to subscribe to all attributes from all clusters
        # from all endpoints on a node it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                # Node controller 2 has no access to
                nodeid=self.dut_node_id,
                attributes=[],
                keepSubscriptions=False,
                reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
                autoResubscribe=False
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed node")

        # *** Step 7 ***
        # CR1 sends a subscription request action for an attribute with an empty
        # DataVersionFilters field. DUT sends a report data action with the data
        # of the attribute along with the data version. Tear down the subscription
        # for that attribute. Start another subscription with the DataVersionFilter
        # field set to the data version received above.
        self.step(7)

        # Subscribe to attribute with empty dataVersionFilters
        sub_cr1_empty_dvf = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            keepSubscriptions=False
        )

        # Verify DataVersion attribute came back
        self.verify_attribute_exists(
            sub=sub_cr1_empty_dvf,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.Attribute.DataVersion
        )

        # Get DataVersion
        data_version = sub_cr1_empty_dvf[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        data_version_filter = [(0, Clusters.BasicInformation, data_version)]

        # Subscribe to attribute with provided DataVersion
        sub_cr1_step7 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False,
            dataVersionFilters=data_version_filter
        )

        # Verify that the subscription is activated between CR1 and DUT
        asserts.assert_true(sub_cr1_step7.subscriptionId, "Subscription not activated")

        sub_cr1_step7.Shutdown()

        # *** Step 8 ***
        # CR1 sends a subscription request action for an attribute and sets the
        # MinIntervalFloor to min_interval_floor_sec and MaxIntervalCeiling to 10.
        # Activate the Subscription between CR1 and DUT and record the time when
        # the priming ReportDataMessage is received as t_report_sec. Save the
        # returned MaxInterval from the SubscribeResponseMessage as max_interval_sec.
        self.step(8)

        # Subscribe to attribute
        sub_cr1_update_value = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Record the time when the priming ReportDataMessage is received
        t_report_sec = time.time()

        # *** Step 9 ***
        # CR1 modifies the attribute which has been subscribed to on the DUT
        # and waits for an incoming ReportDataMessage
        self.step(9)

        # Saving the returned MaxInterval from the SubscribeResponseMessage
        min_interval_floor_sec, max_interval_sec = sub_cr1_update_value.GetReportingIntervalsSeconds()

        # Get subscription timeout
        subscription_timeout_sec = sub_cr1_update_value.GetSubscriptionTimeoutMs() / 1000

        # Set Attribute Update Callback
        node_label_update_cb = AttributeChangeCallback(node_label_attr)
        sub_cr1_update_value.SetAttributeUpdateCallback(node_label_update_cb)

        # Update attribute value
        new_node_label_write = "NewNodeLabel_011235813"
        await CR1.WriteAttribute(
            self.dut_node_id,
            [(0, node_label_attr(value=new_node_label_write))]
        )

        node_label_update_cb.wait_for_report()

        # Save the time that the report is received
        t_update_sec = time.time()

        # Elapsed time between attribute subscription and write update
        t_elapsed_sec = t_update_sec - t_report_sec

        # Verify that t_update - t_report is greater than min_interval_floor_s and less than the ReadClient SubscriptionTimeout
        asserts.assert_greater(t_elapsed_sec, min_interval_floor_sec,
                               f"t_update_sec - t_report_sec ({t_elapsed_sec}s) must be greater than min_interval_floor_sec ({min_interval_floor_sec}s)")
        asserts.assert_less(t_elapsed_sec, subscription_timeout_sec,
                            f"t_update_sec  - t_report_sec ({t_elapsed_sec}s) must be less than subscription_timeout_sec ({subscription_timeout_sec}s)")

        sub_cr1_update_value.Shutdown()

        # *** Step 10 ***
        # CR1 sends a subscription request action for an attribute and set the MinIntervalFloor
        # value to be greater than MaxIntervalCeiling.
        self.step(10)

        # Subscribe to attribute with invalid reportInterval arguments, expect an error
        sub_cr1_invalid_intervals = None
        with asserts.assert_raises(ChipStackError, "Expected exception wasn't thrown."):
            sub_cr1_invalid_intervals = await CR1.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=node_label_attr_path,
                reportInterval=(20, 10),
                keepSubscriptions=False
            )

        # Verify no subscription was established
        with asserts.assert_raises(AttributeError):
            sub_cr1_invalid_intervals.subscriptionId

        # *** Step 11 ***
        # CR1 sends a subscription request to subscribe to a specific global attribute from
        # all clusters on all endpoints.
        self.step(11)

        # Setting max_interval_ceiling_sec value for steps 11-13
        max_interval_ceiling_sec = 10

        # Omitting endpoint to indicate endpoint wildcard
        cluster_rev_attr_path = [(cluster_rev_attr)]

        # Subscribe to global attribute
        sub_cr1_step11 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=cluster_rev_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Verify that the subscription is activated between CR1 and DUT
        asserts.assert_true(sub_cr1_step11.subscriptionId, "Subscription not activated")

        # Verify attribute came back
        self.verify_attribute_exists(
            sub=sub_cr1_step11,
            cluster=Clusters.BasicInformation,
            attribute=cluster_rev_attr
        )

        # Verify DUT sends back the attribute values for the global attribute
        cluster_revision_attr_value = sub_cr1_step11.GetAttribute(cluster_rev_attr_typed_path)

        # Verify ClusterRevision is of uint16 type
        asserts.assert_true(self.is_valid_uint16_value(cluster_revision_attr_value), "ClusterRevision is not of uint16 type.")

        # Verify valid ClusterRevision value
        asserts.assert_greater_equal(cluster_revision_attr_value, 0, "Invalid ClusterRevision value.")

        sub_cr1_step11.Shutdown()

        # *** Step 12 ***
        # CR1 sends a subscription request to subscribe to a global attribute on an endpoint on all clusters.
        self.step(12)

        # Specifying single endpoint 0
        requested_ep = 0
        cluster_rev_attr_path = [(requested_ep, cluster_rev_attr)]

        # Subscribe to global attribute
        sub_cr1_step12 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=cluster_rev_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Verify that the subscription is activated between CR1 and DUT
        asserts.assert_true(sub_cr1_step12.subscriptionId, "Subscription not activated")

        # Verify attribute came back
        self.verify_attribute_exists(
            sub=sub_cr1_step12,
            cluster=Clusters.BasicInformation,
            attribute=cluster_rev_attr
        )

        # Verify no data from other endpoints is sent back
        attributes = sub_cr1_step12.GetAttributes()
        ep_keys = list(attributes.keys())
        asserts.assert_true(len(ep_keys) == 1, "More than one endpoint returned, exactly 1 was expected")

        # Verify DUT sends back the attribute values for the global attribute
        cluster_rev_attr_typed_path = self.get_typed_attribute_path(cluster_rev_attr)
        cluster_revision_attr_value = sub_cr1_step12.GetAttribute(cluster_rev_attr_typed_path)

        # Verify ClusterRevision is of uint16 type
        asserts.assert_true(self.is_valid_uint16_value(cluster_revision_attr_value), "ClusterRevision is not of uint16 type.")

        sub_cr1_step12.Shutdown()

        # *** Step 13 ***
        # CR1 sends a subscription request to the DUT with both AttributeRequests and EventRequests as empty.
        self.step(13)

        # Attempt a subscription with both AttributeRequests and EventRequests as empty
        sub_cr1_step13 = None
        try:
            sub_cr1_step13 = await CR1.Read(
                nodeid=self.dut_node_id,
                attributes=[],
                events=[],
                reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT sends back a Status Response Action with the INVALID_ACTION Status Code
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription with empty AttributeRequests and EventRequests")

            # Verify no subscription is established
            with asserts.assert_raises(AttributeError):
                sub_cr1_step13.subscriptionId
        except Exception:
            asserts.fail("Expected exception was not thrown")


if __name__ == "__main__":
    default_matter_test_main()
