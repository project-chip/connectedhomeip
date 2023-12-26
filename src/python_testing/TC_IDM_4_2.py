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
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects as ClustersObjects
from chip.clusters.Attribute import SubscriptionTransaction, TypedAttributePath, AttributePath
from mobly import asserts

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

    def is_uint32(self, var):
        return isinstance(var, int) and 0 <= var <= 4294967295

    @async_test_body
    async def test_TC_IDM_4_2(self):
        
        SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 0
        CR1: ChipDeviceController = self.default_controller # Is admin by default
        icd_mgmt_cluster = Clusters.IcdManagement
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
        
        # Step 1
        self.print_step(1, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT. DUT sends a report data action to the TH. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 60
        max_interval_ceiling_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT + 60
        
        # Subscribe to attribute
        sub_cr1 = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(0, node_label_attr)],
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )
        
        # TODO: Correct way to get data of the attribute requested in the subscription and compare
        # Get data of the attribute requested in the subscription
        sub_cr1_attr = sub_cr1.GetAttribute(typed_attribute_path)
        logging.info("debux - Attribute data: " + str(sub_cr1_attr))
        # asserts.assert_equal( node_label_attr, sub_cr1_attr, "Report data action attribute mismatch or missing")
        
        # Verify uint32 type
        asserts.assert_true(self.is_uint32(sub_cr1.subscriptionId), "subscriptionId is not of uint32 type.")
        
        # Verify uint32 type
        sub_cr1_intervals = sub_cr1.GetReportingIntervalsSeconds()
        sub_cr1_min_interval_floor_sec, sub_cr1_max_interval_ceiling_sec = sub_cr1_intervals
        asserts.assert_true(self.is_uint32(sub_cr1_max_interval_ceiling_sec), "MaxInterval is not of uint32 type.")

        # Verify MaxInterval is less than or equal to MaxIntervalCeiling
        asserts.assert_true(sub_cr1_max_interval_ceiling_sec <= max_interval_ceiling_sec, "MaxInterval is not less than or equal to MaxIntervalCeiling")


        
        
        
        
        
        
        
        

            
        
      
        
     
        


        # data = sub.GetAttributes()
        # if data:
        #     attributes = sub.GetAttributes()[0]
        #     basic_information = (sub.GetAttributes()[0])[Clusters.Objects.BasicInformation]
        # else:

        
        
        # if node_label_attr in sub_basic_information[Clusters.Objects.BasicInformation]
        
        
        # logging.info("debux - sub_intervals: " + str(sub_intervals))
        # logging.info("debux - sub_1_attr: " + str(sub_1_attr))
        # logging.info("debux - sub_basic_information_attribute: " + str(sub_basic_information_attribute))
        # logging.info("debux - sub_subscription_id: " + str(sub_subscription_id))

        
        
        # data = {
        #     0: {
        #         clusters.Objects.BasicInformation: {
        #             clusters.Attribute.DataVersion: 2884903808, 
        #             clusters.Objects.BasicInformation.Attributes.NodeLabel: ''
        #         }
        #     }
        # }

        # # Assuming 'chip' module is already imported and classes are available

        # # Iterate through the first level of the dictionary
        # for key, value in data.items():
        #     logging.info(f"debux - Key: {key}")
            
        #     # 'value' is another dictionary, so iterate through it
        #     for class_key, inner_dict in value.items():
        #         logging.info(f"debux -   Class Key: {class_key.__name__}")

        #         # 'inner_dict' is also a dictionary, iterate through it to log each item
        #         for class_attr_key, attr_value in inner_dict.items():
        #             logging.info(f"debux -     Attribute Key: {class_attr_key.__name__}, Value: {attr_value}")

        
        
        
        
        # output_queue = queue.Queue()
        # attribute_handler = AttributeChangeAccumulator(
        #     name=CR1.name, 
        #     expected_attribute=Clusters.BasicInformation.Attributes.NodeLabel, 
        #     output=output_queue)
        
        # sub.SetAttributeUpdateCallback(attribute_handler)









    
        # Controller 2 Setup
        # fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]

        # CR2_nodeid = self.matter_test_config.controller_node_id + 1
        # CR2 = fabric_admin.NewController(nodeId=CR2_nodeid,
        #                                  paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        
        # CR2_limited_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
        #     privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
        #     authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
        #     subjects=[CR2_nodeid],
        #     targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])        
        
        logging.info("debux - test_TC_IDM_4_2 end")

if __name__ == "__main__":
    default_matter_test_main()
