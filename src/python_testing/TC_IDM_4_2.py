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

import time
import random
import logging
import chip.clusters as Clusters
from mobly import asserts
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters.Attribute import TypedAttributePath, AttributePath
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main


class TC_IDM_4_2(MatterBaseTest):

    async def write_acl(self, acl):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        print(result)

    async def get_descriptor_server_list(self, ctrl):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_idle_mode_duration(self, ctrl):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=ctrl,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.IdleModeDuration
        )

    def verify_attribute_data(self, sub, cluster, attribute, ep=0):
        sub_attrs = sub
        if isinstance(sub, Clusters.Attribute.SubscriptionTransaction):
            sub_attrs = sub.GetAttributes()

        asserts.assert_true(ep in sub_attrs, "Must have read endpoint %s data" % ep)
        asserts.assert_true(cluster in sub_attrs[0], "Must have read %s cluster data" % cluster.__name__)
        asserts.assert_true(attribute in sub_attrs[0][cluster],
                            "Must have read back attribute %s" % attribute.__name__)

    def get_attribute_from_sub_dict(self, sub, cluster, attribute, ep=0):
        return sub[ep][cluster][attribute]

    def get_typed_attribute_path(self, attribute, ep=0):
        return TypedAttributePath(
            Path=AttributePath(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    def is_uint32(self, var):
        return isinstance(var, int) and 0 <= var <= 4294967295

    @async_test_body
    async def test_TC_IDM_4_2(self):

        CR1: ChipDeviceController = self.default_controller
        SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 0
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        node_label_attr_path = [(0, node_label_attr)]

        # Read ServerList attribute
        self.print_step("0a", "CR1 reads the Descriptor cluster ServerList attribute from EP0")
        ep0_servers = await self.get_descriptor_server_list(CR1)

        # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        if Clusters.IcdManagement.id in ep0_servers:
            # Read the IdleModeDuration attribute value from the DUT
            logging.info(
                "CR1 reads from the DUT the IdleModeDuration attribute and sets SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = IdleModeDuration")

            idleModeDuration = await self.get_idle_mode_duration(CR1)

            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = idleModeDuration
            logging.info("SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT: " + idleModeDuration + " sec")
        else:
            # Defaulting SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT to 60 minutes
            logging.info("Set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 60 mins")
            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 3600

        '''
        ##########
        Step 1
        ##########
        '''
        self.print_step(1, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT. DUT sends a report data action to the TH. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 60
        max_interval_ceiling_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT + 60

        # Subscribe to attribute
        sub_cr1_step1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Verify attribute data came back
        self.verify_attribute_data(
            sub=sub_cr1_step1,
            cluster=Clusters.BasicInformation,
            attribute=node_label_attr
        )

        # Verify subscriptionId is of uint32 type
        asserts.assert_true(self.is_uint32(sub_cr1_step1.subscriptionId), "subscriptionId is not of uint32 type.")

        # Verify MaxInterval is of uint32 type
        sub_cr1_step1_intervals = sub_cr1_step1.GetReportingIntervalsSeconds()
        sub_cr1_step1_min_interval_floor_sec, sub_cr1_step1_max_interval_ceiling_sec = sub_cr1_step1_intervals
        asserts.assert_true(self.is_uint32(sub_cr1_step1_max_interval_ceiling_sec), "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_true(sub_cr1_step1_max_interval_ceiling_sec <= max_interval_ceiling_sec,
                            "MaxInterval is not less than or equal to MaxIntervalCeiling")

        sub_cr1_step1.Shutdown()

        '''
        ##########
        Step 2
        ##########
        '''
        self.print_step(2, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value less than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT. DUT sends a report data action to the CR1. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 60
        max_interval_ceiling_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 30

        # Subscribe to attribute
        sub_cr1_step2 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )

        # Verify attribute data came back
        self.verify_attribute_data(
            sub=sub_cr1_step2,
            cluster=Clusters.BasicInformation,
            attribute=node_label_attr
        )

        # Verify subscriptionId is of uint32 type
        asserts.assert_true(self.is_uint32(sub_cr1_step2.subscriptionId), "subscriptionId is not of uint32 type.")

        # Verify MaxInterval is of uint32 type
        sub_cr1_step2_intervals = sub_cr1_step2.GetReportingIntervalsSeconds()
        sub_cr1_step2_min_interval_floor_sec, sub_cr1_step2_max_interval_ceiling_sec = sub_cr1_step2_intervals
        asserts.assert_true(self.is_uint32(sub_cr1_step2_max_interval_ceiling_sec), "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_true(sub_cr1_step2_max_interval_ceiling_sec <= max_interval_ceiling_sec,
                            "MaxInterval is not less than or equal to MaxIntervalCeiling")

        sub_cr1_step2.Shutdown()

        '''
        ##########
        Step 3 - 6
        ##########
        '''
        # # TODO: How to trigger desired Status responses

        # # Controller 2 Setup
        # fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        # CR2_nodeid = self.matter_test_config.controller_node_id + 1
        # CR2 = fabric_admin.NewController(
        #     nodeId=CR2_nodeid,
        #     paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        # )

        # CR2_limited_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
        #     privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
        #     authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
        #     subjects=[CR2_nodeid],
        #     targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.Thermostat.id)]
        # )
        # await self.write_acl([CR2_limited_acl])
        # cluster = Clusters.Descriptor
        # attribute = Clusters.Descriptor.Attributes.DeviceTypeList
        # await self.read_single_attribute_expect_error(
        #     dev_ctrl=CR2,
        #     endpoint=0,
        #     cluster=cluster,
        #     attribute=attribute,
        #     error=Status.InvalidAction
        # )

        '''
        ##########
        Step 7
        ##########
        '''
        self.print_step(7, "CR1 sends a subscription request action for an attribute with an empty DataVersionFilters field. DUT sends a report data action with the data of the attribute along with the data version. Tear down the subscription for that attribute. Start another subscription with the DataVersionFilter field set to the data version received above.")

        # Subscribe to attribute with empty dataVersionFilters
        sub_cr1_empty_dvf = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            keepSubscriptions=False
        )

        # Verify DataVersion attribute data came back
        self.verify_attribute_data(
            sub=sub_cr1_empty_dvf,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.Attribute.DataVersion
        )

        # Get DataVersion
        data_version = self.get_attribute_from_sub_dict(
            sub=sub_cr1_empty_dvf,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.Attribute.DataVersion
        )
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

        '''
        ##########
        Step 8
        ##########
        '''
        self.print_step(8, "CR1 sends a subscription request action for an attribute and sets the MinIntervalFloor value to be same as MaxIntervalCeiling. Activate the Subscription between CR1 and DUT. Modify the attribute which has been subscribed to on the DUT.")
        min_max_interval_sec = 3

        # Subscribe to attribute
        sub_cr1_update_value = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=node_label_attr_path,
            reportInterval=(min_max_interval_sec, min_max_interval_sec),
            keepSubscriptions=False
        )

        # Modify attribute value
        new_node_label_write = "NewNodeLabel_" + str(random.randint(1000, 9999))
        await CR1.WriteAttribute(
            self.dut_node_id,
            [(0, node_label_attr(value=new_node_label_write))]
        )

        # Wait MinIntervalFloor seconds before reading updated attribute value
        time.sleep(min_max_interval_sec)
        node_label_attr_typed_path = self.get_typed_attribute_path(node_label_attr)
        new_node_label_read = sub_cr1_update_value.GetAttribute(node_label_attr_typed_path)

        # Verify new attribute value after MinIntervalFloor time
        asserts.assert_equal(new_node_label_read, new_node_label_write, "Attribute value not updated after write operation.")

        sub_cr1_update_value.Shutdown()

        '''
        ##########
        Step 9
        ##########
        '''
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

        '''
        ##########
        Step 10 - 12
        ##########
        '''
        # TODO: How to subscribe to global attributes
        # TODO: How to subscribe with both AttributeRequests and EventRequests as empty
        # self.print_step(10, "CR1 sends a subscription request to subscribe to a specific global attribute from all clusters on all endpoints.")
        # read_contents = [
        #     Clusters.BasicInformation.Attributes.NodeLabel
        # ]
        # read_paths = [(0, attrib) for attrib in read_contents]

        # # Subscribe to global attribute
        # sub_cr1_invalid_intervals = await CR1.ReadAttribute(
        #     nodeid=self.dut_node_id,
        #     attributes=read_paths,
        #     reportInterval=(10, 10),
        #     keepSubscriptions=False,
        # )


if __name__ == "__main__":
    default_matter_test_main()
