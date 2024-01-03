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

import queue
import logging
import chip.clusters as Clusters
from chip.interaction_model import Status, InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects as ClustersObjects
from chip.clusters.Attribute import SubscriptionTransaction, TypedAttributePath, AttributePath, AttributeCache
from mobly import asserts
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main


class TC_IDM_4_2(MatterBaseTest):
    
    async def write_acl(self, acl):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        logging.info("debux - write_acl Status result: " + str(result[0].Status))
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        print(result)    
    
    async def read_descriptor_server_list_expect_success(self, th):
        cluster = Clusters.Objects.Descriptor
        attribute = Clusters.Descriptor.Attributes.ServerList
        return await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
    
    async def read_basic_expect_success(self, th):
        cluster = Clusters.Objects.BasicInformation
        attribute = Clusters.BasicInformation.Attributes.VendorID
        return await self.read_single_attribute_check_success(
            dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        
    # async def read_single_attribute_expect_error(
    #         self, cluster: object, attribute: object,
    #         error: Status, dev_ctrl: ChipDeviceCtrl = None, node_id: int = None, endpoint: int = None) -> object:
    #     if dev_ctrl is None:
    #         dev_ctrl = self.default_controller
    #     if node_id is None:
    #         node_id = self.dut_node_id
    #     if endpoint is None:
    #         endpoint = self.matter_test_config.endpoint

    #     result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)])
    #     attr_ret = result[endpoint][cluster][attribute]
    #     err_msg = "Did not see expected error when reading {}:{}".format(str(cluster), str(attribute))
    #     asserts.assert_true(attr_ret is not None, err_msg)
    #     asserts.assert_true(isinstance(attr_ret, Clusters.Attribute.ValueDecodeFailure), err_msg)
    #     asserts.assert_true(isinstance(attr_ret.Reason, InteractionModelError), err_msg)
    #     asserts.assert_equal(attr_ret.Reason.status, error, err_msg)
    #     return attr_ret

    def is_uint32(self, var):
        return isinstance(var, int) and 0 <= var <= 4294967295

    @async_test_body
    async def test_TC_IDM_4_2(self):
        
        SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 0
        CR1: ChipDeviceController = self.default_controller # Is admin by default
        icd_mgmt_cluster = Clusters.Objects.IcdManagement
        idle_mode_duration_attr = icd_mgmt_cluster.Attributes.IdleModeDuration
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        typed_attribute_path: TypedAttributePath = TypedAttributePath(
            Path=AttributePath(
                EndpointId=0,
                Attribute=node_label_attr
            )
        )
        
        # Read ServerList attribute
        self.print_step("0a", "CR1 reads the Descriptor cluster ServerList attribute from EP0")
        ep0_servers = await self.read_descriptor_server_list_expect_success(CR1)
        
        # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        if icd_mgmt_cluster.id in ep0_servers:
            # Read the IdleModeDuration attribute from the DUT
            logging.info("CR1 reads from the DUT the IdleModeDuration attribute and sets SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = IdleModeDuration")
        
            idleModeDuration = await self.read_single_attribute_check_success(
                cluster=icd_mgmt_cluster, 
                attribute=idle_mode_duration_attr, 
                dev_ctrl=CR1, 
                node_id=self.dut_node_id, 
                endpoint=0
            )
            
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
        
        read_contents = [
            Clusters.BasicInformation.Attributes.VendorName,
            Clusters.BasicInformation.Attributes.ProductName,
            Clusters.BasicInformation.Attributes.NodeLabel,
        ]
        read_paths = [(0, attrib) for attrib in read_contents]

        # Subscribe to attribute
        sub_cr1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=read_paths,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )
        
        sub_cr1_requested_attributes = sub_cr1.GetAttributes()

        # Verify attribute data came back
        asserts.assert_true(0 in sub_cr1_requested_attributes, "Must have read endpoint 0 data")
        asserts.assert_true(Clusters.BasicInformation in sub_cr1_requested_attributes[0], "Must have read Basic Information cluster data")
        for attribute in read_contents:
            asserts.assert_true(attribute in sub_cr1_requested_attributes[0][Clusters.BasicInformation],
                                "Must have read back attribute %s" % (attribute.__name__))

        # Verify subscriptionId is of uint32 type
        asserts.assert_true(self.is_uint32(sub_cr1.subscriptionId), "subscriptionId is not of uint32 type.")
        
        # Verify MaxInterval is of uint32 type
        sub_cr1_intervals = sub_cr1.GetReportingIntervalsSeconds()
        sub_cr1_min_interval_floor_sec, sub_cr1_max_interval_ceiling_sec = sub_cr1_intervals
        asserts.assert_true(self.is_uint32(sub_cr1_max_interval_ceiling_sec), "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_true(sub_cr1_max_interval_ceiling_sec <= max_interval_ceiling_sec, "MaxInterval is not less than or equal to MaxIntervalCeiling")

        sub_cr1.Shutdown()

        ''' 
        ##########
        Step 2
        ##########
        '''
        self.print_step(2, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value less than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT. DUT sends a report data action to the CR1. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 60
        max_interval_ceiling_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 30
        
        # Subscribe to attribute
        sub_cr1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=read_paths,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )
        
        sub_cr1_requested_attributes = sub_cr1.GetAttributes()

        # Verify attribute data came back
        asserts.assert_true(0 in sub_cr1_requested_attributes, "Must have read endpoint 0 data")
        asserts.assert_true(Clusters.BasicInformation in sub_cr1_requested_attributes[0], "Must have read Basic Information cluster data")
        for attribute in read_contents:
            asserts.assert_true(attribute in sub_cr1_requested_attributes[0][Clusters.BasicInformation],
                                "Must have read back attribute %s" % (attribute.__name__))
        
        # Verify subscriptionId is of uint32 type
        asserts.assert_true(self.is_uint32(sub_cr1.subscriptionId), "subscriptionId is not of uint32 type.")
        
        # Verify MaxInterval is of uint32 type
        sub_cr1_intervals = sub_cr1.GetReportingIntervalsSeconds()
        sub_cr1_min_interval_floor_sec, sub_cr1_max_interval_ceiling_sec = sub_cr1_intervals
        asserts.assert_true(self.is_uint32(sub_cr1_max_interval_ceiling_sec), "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_true(sub_cr1_max_interval_ceiling_sec <= max_interval_ceiling_sec, "MaxInterval is not less than or equal to MaxIntervalCeiling")

        sub_cr1.Shutdown()
        
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
        # cluster = Clusters.Objects.Descriptor
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
        read_contents = [
            Clusters.BasicInformation.Attributes.ProductName
        ]
        read_paths = [(0, attrib) for attrib in read_contents]
        
        # Subscribe to attribute with empty dataVersionFilters
        sub_cr1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=read_paths,
            keepSubscriptions=False
        )

        # Verify attribute data came back
        asserts.assert_true(0 in sub_cr1, "Must have read endpoint 0 data")
        asserts.assert_true(Clusters.BasicInformation in sub_cr1[0], "Must have read Basic Information cluster data")
        for attribute in read_contents:
            asserts.assert_true(Clusters.Attribute.DataVersion in sub_cr1[0][Clusters.BasicInformation],
                                "Must have read back attribute %s" % (attribute.__name__))

        # Get DataVersion and generate a dataVersionFilters type
        data_version = sub_cr1[0][Clusters.Objects.BasicInformation][Clusters.Attribute.DataVersion]
        data_version_filter = [(0, Clusters.BasicInformation, data_version)]

        # Subscribe to attribute with provided dataVersionFilters
        sub_cr1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=read_paths,
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False,
            dataVersionFilters=data_version_filter
        )
        
        # Verify that the subscription is activated between CR1 and DUT
        asserts.assert_true(sub_cr1.subscriptionId, "Subscription not activated")
        
        
        
        
        
        
        
        


        logging.info("debux - test_TC_IDM_4_2 end")

if __name__ == "__main__":
    default_matter_test_main()
