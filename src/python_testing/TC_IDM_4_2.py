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

import copy
import logging
import time

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters.Attribute import AttributePath, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
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

    ROOT_NODE_ENDPOINT_ID = 0

    async def write_acl(self, ctrl, acl, ep=ROOT_NODE_ENDPOINT_ID):
        result = await ctrl.WriteAttribute(self.dut_node_id, [(ep, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[ep].Status, Status.Success, "ACL write failed")

    async def get_descriptor_server_list(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
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
            Path=AttributePath(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    async def get_dut_acl(self, ep=ROOT_NODE_ENDPOINT_ID):
        sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ep, Clusters.AccessControl.Attributes.Acl)],
            keepSubscriptions=False,
            fabricFiltered=True
        )

        acl_list = sub[ep][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        return acl_list

    async def add_ace_to_dut_acl(self, ctrl, ace):
        dut_acl_original = await self.get_dut_acl()
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        await self.write_acl(ctrl=ctrl, acl=dut_acl)

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
        node_label_attr_typed_path = self.get_typed_attribute_path(node_label_attr)
        SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = 0
        INVALID_ACTION_ERROR_CODE = 0x580

        # Controller 1 setup
        # Subscriber/client with admin access to the DUT
        # Will write ACL for controller 2 and validate success/error codes
        CR1: ChipDeviceController = self.default_controller

        # Controller 2 setup
        # Subscriber/client with limited access to the DUT
        # Will validate error status codes
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        CR2_nodeid = self.matter_test_config.controller_node_id + 1
        CR2: ChipDeviceController = fabric_admin.NewController(
            nodeId=CR2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        # Read ServerList attribute
        self.print_step("0a", "CR1 reads the Descriptor cluster ServerList attribute from EP0")
        ep0_servers = await self.get_descriptor_server_list(CR1)

        # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        if Clusters.IcdManagement.id in ep0_servers:
            # Read the IdleModeDuration attribute value from the DUT
            logging.info(
                "CR1 reads from the DUT the IdleModeDuration attribute and sets SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = IdleModeDuration")

            idleModeDuration = await self.get_idle_mode_duration_sec(CR1)

            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = idleModeDuration
        else:
            # Defaulting SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC to 60 minutes
            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = 60 * 60

        logging.info(
            f"Set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC to {SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC} seconds")

        # *** Step 1 ***
        self.print_step(1, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC. DUT sends a report data action to the TH. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = 1
        max_interval_ceiling_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC + 5
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
        self.print_step(2, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value less than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC. DUT sends a report data action to the CR1. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = 1
        max_interval_ceiling_sec = max(2, SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC - 5)
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
        asserts.assert_less_equal(sub_cr1_step2_max_interval_ceiling_sec, SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC,
                                  "MaxInterval is not less than or equal to SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC")

        sub_cr1_step2.Shutdown()

        # *** Step 3 ***
        self.print_step(3, "Setup CR2 such that it does not have access to a specific cluster. CR2 sends a subscription message to subscribe to an attribute on that cluster for which it does not have access.")

        # Limited ACE for controller 2 with single cluster access
        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.BasicInformation.id)],
            subjects=[CR2_nodeid])

        self.add_ace_to_dut_acl(CR1, CR2_limited_ace)

        # Controller 2 tries to subscribe an attribute from a cluster
        # it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                nodeid=self.dut_node_id,
                # Attribute from a cluster controller 2 has no access to
                attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
                keepSubscriptions=False,
                reportInterval=[3, 3],
                autoResubscribe=False
            )
            asserts.fail("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed cluster")

        # *** Step 4 ***
        self.print_step(4, "Setup CR2 such that it does not have access to all attributes on a specific cluster and endpoint. CR2 sends a subscription request to subscribe to all attributes for which it does not have access.")

        # Limited ACE for controller 2 with single cluster access and specific endpoint
        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=1,
                cluster=Clusters.BasicInformation.id)],
            subjects=[CR2_nodeid])

        self.add_ace_to_dut_acl(CR1, CR2_limited_ace)

        # Controller 2 tries to subscribe to all attributes from a cluster
        # it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                nodeid=self.dut_node_id,
                # Cluster controller 2 has no access to
                attributes=[(0, Clusters.BasicInformation)],
                keepSubscriptions=False,
                reportInterval=[3, 3],
                autoResubscribe=False
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed cluster")

        # *** Step 5 ***
        self.print_step(5, "Setup CR2 such that it does not have access to an Endpoint. CR2 sends a subscription request to subscribe to all attributes on all clusters on a specific Endpoint for which it does not have access.")

        # Limited ACE for controller 2 with endpoint 1 access only to all clusters and all attributes
        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=1)],
            subjects=[CR2_nodeid])

        self.add_ace_to_dut_acl(CR1, CR2_limited_ace)

        # Controller 2 tries to subscribe to all attributes from all clusters
        # on an endpoint it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                nodeid=self.dut_node_id,
                # Endpoint controller 2 has no access to
                attributes=[(0)],
                keepSubscriptions=False,
                reportInterval=[3, 3],
                autoResubscribe=False
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed endpoint")

        # *** Step 6 ***
        self.print_step(6, "Setup CR2 such that it does not have access to the Node. CR2 sends a subscription request to subscribe to all attributes on all clusters on all endpoints on a Node for which it does not have access.")

        # Skip setting an ACE for controller 2 so
        # the DUT node rejects subscribing to it

        # Write original DUT ACL into DUT
        dut_acl_original = await self.get_dut_acl()
        await self.write_acl(ctrl=CR1, acl=dut_acl_original)

        # Controller 2 tries to subscribe to all attributes from all clusters
        # from all endpoints on a node it doesn't have access to
        # "INVALID_ACTION" status response expected
        try:
            await CR2.ReadAttribute(
                # Node controller 2 has no access to
                nodeid=self.dut_node_id,
                attributes=[],
                keepSubscriptions=False,
                reportInterval=[3, 3],
                autoResubscribe=False
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "INVALID_ACTION" status response
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription to unallowed node")

        # *** Step 7 ***
        self.print_step(7, "CR1 sends a subscription request action for an attribute with an empty DataVersionFilters field. DUT sends a report data action with the data of the attribute along with the data version. Tear down the subscription for that attribute. Start another subscription with the DataVersionFilter field set to the data version received above.")

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
        sub_cr1_provided_dvf = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(10, 20),
            keepSubscriptions=False,
            dataVersionFilters=data_version_filter
        )

        # Verify that the subscription is activated between CR1 and DUT
        asserts.assert_true(sub_cr1_provided_dvf.subscriptionId, "Subscription not activated")

        sub_cr1_provided_dvf.Shutdown()

        # *** Step 8 ***
        self.print_step(8, "CR1 sends a subscription request action for an attribute and sets the MinIntervalFloor value to be same as MaxIntervalCeiling. Activate the Subscription between CR1 and DUT. Modify the attribute which has been subscribed to on the DUT.")

        # Subscribe to attribute
        same_min_max_interval_sec = 3
        sub_cr1_update_value = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(same_min_max_interval_sec, same_min_max_interval_sec),
            keepSubscriptions=False
        )

        # Modify attribute value
        new_node_label_write = "NewNodeLabel_011235813"
        await CR1.WriteAttribute(
            self.dut_node_id,
            [(0, node_label_attr(value=new_node_label_write))]
        )

        # Wait MinIntervalFloor seconds before reading updated attribute value
        time.sleep(same_min_max_interval_sec)
        new_node_label_read = sub_cr1_update_value.GetAttribute(node_label_attr_typed_path)

        # Verify new attribute value after MinIntervalFloor time
        asserts.assert_equal(new_node_label_read, new_node_label_write, "Attribute value not updated after write operation.")

        sub_cr1_update_value.Shutdown()

        # *** Step 9 ***
        self.print_step(
            9, "CR1 sends a subscription request action for an attribute and set the MinIntervalFloor value to be greater than MaxIntervalCeiling.")

        # Subscribe to attribute with invalid reportInterval arguments, expect and exception
        sub_cr1_invalid_intervals = None
        try:
            sub_cr1_invalid_intervals = await CR1.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=node_label_attr_path,
                reportInterval=(20, 10),
                keepSubscriptions=False
            )
        except ChipStackError:
            # Verify no subscription is established
            with asserts.assert_raises(AttributeError):
                sub_cr1_invalid_intervals.subscriptionId
        except Exception:
            asserts.fail("Expected exception was not thrown")

        # *** Step 10 ***
        self.print_step(
            10, "CR1 sends a subscription request to subscribe to a specific global attribute from all clusters on all endpoints.")

        # Omitting endpoint to indicate endpoint wildcard
        cluster_rev_attr_path = [(cluster_rev_attr)]

        # Subscribe to global attribute
        sub_cr1_step10 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=cluster_rev_attr_path,
            reportInterval=(3, 3),
            keepSubscriptions=False
        )

        # Verify that the subscription is activated between CR1 and DUT
        asserts.assert_true(sub_cr1_step10.subscriptionId, "Subscription not activated")

        # Verify attribute came back
        self.verify_attribute_exists(
            sub=sub_cr1_step10,
            cluster=Clusters.BasicInformation,
            attribute=cluster_rev_attr
        )

        # Verify DUT sends back the attribute values for the global attribute
        cluster_revision_attr_value = sub_cr1_step10.GetAttribute(cluster_rev_attr_typed_path)

        # Verify ClusterRevision is of uint16 type
        asserts.assert_true(self.is_valid_uint16_value(cluster_revision_attr_value), "ClusterRevision is not of uint16 type.")

        # Verify valid ClusterRevision value
        asserts.assert_greater_equal(cluster_revision_attr_value, 0, "Invalid ClusterRevision value.")

        sub_cr1_step10.Shutdown()

        # *** Step 11 ***
        self.print_step(11, "CR1 sends a subscription request to subscribe to a global attribute on an endpoint on all clusters.")

        # Specifying single endpoint 0
        requested_ep = 0
        cluster_rev_attr_path = [(requested_ep, cluster_rev_attr)]

        # Subscribe to global attribute
        sub_cr1_step11 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=cluster_rev_attr_path,
            reportInterval=(3, 3),
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

        # Verify no data from other endpoints is sent back
        attributes = sub_cr1_step11.GetAttributes()
        ep_keys = list(attributes.keys())
        asserts.assert_true(len(ep_keys) == 1, "More than one endpoint returned, exactly 1 was expected")

        # Verify DUT sends back the attribute values for the global attribute
        cluster_rev_attr_typed_path = self.get_typed_attribute_path(cluster_rev_attr)
        cluster_revision_attr_value = sub_cr1_step11.GetAttribute(cluster_rev_attr_typed_path)

        # Verify ClusterRevision is of uint16 type
        asserts.assert_true(self.is_valid_uint16_value(cluster_revision_attr_value), "ClusterRevision is not of uint16 type.")

        sub_cr1_step11.Shutdown()

        # *** Step 12 ***
        self.print_step(12, "CR1 sends a subscription request to the DUT with both AttributeRequests and EventRequests as empty.")

        # Attempt a subscription with both AttributeRequests and EventRequests as empty
        sub_cr1_step12 = None
        try:
            sub_cr1_step12 = await CR1.Read(
                nodeid=self.dut_node_id,
                attributes=[],
                events=[],
                reportInterval=(3, 3)
            )
            raise ValueError("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT sends back a Status Response Action with the INVALID_ACTION Status Code
            asserts.assert_equal(e.err, INVALID_ACTION_ERROR_CODE,
                                 "Incorrect error response for subscription with empty AttributeRequests and EventRequests")

            # Verify no subscription is established
            with asserts.assert_raises(AttributeError):
                sub_cr1_step12.subscriptionId
        except Exception:
            asserts.fail("Expected exception was not thrown")


if __name__ == "__main__":
    default_matter_test_main()
