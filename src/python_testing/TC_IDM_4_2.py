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

import inspect
import logging
from dataclasses import dataclass

import chip.clusters as Clusters
import chip.discovery as Discovery
from chip import ChipUtility
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

from chip.clusters import ClusterObjects as ClustersObjects
from chip.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from chip.utils import CommissioningBuildingBlocks
import queue

class AttributeChangeAccumulator:
    def __init__(self, name: str, expected_attribute: ClustersObjects.ClusterAttributeDescriptor, output: queue.Queue):
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

    @async_test_body
    async def test_TC_IDM_4_2(self):
        
        SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 0
        
        # Controller 1 Setup
        CR1 = self.default_controller
        CR1_node_id = self.matter_test_config.controller_node_id
        
        CR1_ace_full_access = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[CR1_node_id],
            targets=[])
        
        acl = [CR1_ace_full_access]
        await self.write_acl(acl)
        
        # Read ServerList attribute
        self.print_step("0a", "CR1 reads the Descriptor cluster ServerList attribute from EP0")
        ep0_servers = await self.read_descriptor_server_list_expect_success(CR1)
        
        # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        if Clusters.IcdManagement.id in ep0_servers:
            # Read the IdleModeDuration attribute from the DUT
            self.print_step("0b", "CR1 reads from the DUT the IdleModeDuration attribute and sets SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = IdleModeDuration")
            logging.info("debux - Setting IcdManagement cluster...")
            cluster = Clusters.Objects.IcdManagement
            
            logging.info("debux - Setting IdleModeDuration attribute...")
            idle_mode_duration_attr = Clusters.IcdManagement.Attributes.IdleModeDuration
        
            logging.info("debux - Getting idleModeDuration attribute value...")
            idleModeDuration = await self.read_single_attribute_check_success(
                cluster=cluster, 
                attribute=idle_mode_duration_attr, 
                dev_ctrl=CR1, 
                node_id=self.dut_node_id, 
                endpoint=0)
            
            logging.info('debux - idleModeDuration: ' + idleModeDuration)
            
            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = idleModeDuration
        else:
            # Defaulting SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT to 60 minutes
            self.print_step("0b", "Set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 60 mins")
            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 3600
        
        # Step 1
        self.print_step(1, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT. DUT sends a report data action to the TH. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
        min_interval_floor_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT - 60
        max_interval_ceiling_sec = SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT + 60
        
        sub = await CR1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(0, Clusters.BasicInformation.Attributes.NodeLabel)],
            reportInterval=(min_interval_floor_sec, max_interval_ceiling_sec),
            keepSubscriptions=False
        )
        
        output_queue = queue.Queue()
        attribute_handler = AttributeChangeAccumulator(
            name=CR1.name, 
            expected_attribute=Clusters.BasicInformation.Attributes.NodeLabel, 
            output=output_queue)
        
        sub.SetAttributeUpdateCallback(attribute_handler)









    
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
