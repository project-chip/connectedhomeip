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

class TC_IDM_4_2(MatterBaseTest):

    @async_test_body
    async def test_TC_IDM_4_2(self):
        
        SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 0

        # Checks if ICDM.S PICS is not set
        if not self.check_pics("ICDM.S"):
            self.print_step("0a", "SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 60 mins")
            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = 3600
        else:
            logging.info("debux")
            # Get IdleModeDuration attribute (IdleModeInterval?)
            self.print_step("0b", "CR1 reads from the DUT the IdleModeDuration attribute.")

            logging.info("debux - Setting enpoint...")
            endpoint = self.user_params.get("endpoint", 0)

            logging.info("debux - Setting cluster...")
            cluster = Clusters.Objects.IcdManagement
            
            logging.info("debux - Setting attributes...")
            attributes = Clusters.IcdManagement.Attributes
            
            idleModeDuration = 0

            logging.info("debux - Getting idleModeDuration...")
            idleModeDuration = await self.read_single_attribute_check_success(
                endpoint=endpoint, 
                cluster=cluster, 
                attribute=attributes.IdleModeDuration)
            
            logging.info('debux - idleModeDuration: ' + idleModeDuration)
            
            SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = idleModeDuration

        # self.print_step(1, "CR1 sends a subscription message to the DUT with MaxIntervalCeiling set to a value greater than SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT. DUT sends a report data action to the TH. CR1 sends a success status response to the DUT. DUT sends a Subscribe Response Message to the CR1 to activate the subscription.")
    
        # # Controllers Setup
        # fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]

        # CR1_nodeid = self.matter_test_config.controller_node_id
        # CR2_nodeid = self.matter_test_config.controller_node_id + 1
        
        # CR1 = fabric_admin.NewController(nodeId=CR1_nodeid,
        #                                  paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        # CR2 = fabric_admin.NewController(nodeId=CR2_nodeid,
        #                                  paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        
        # CR1_admin_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
        #     privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
        #     authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
        #     subjects=[CR1_nodeid],
        #     targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=0x001f)])
        
        # CR2_limited_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
        #     privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
        #     authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
        #     subjects=[CR2_nodeid],
        #     targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])        
        
        # acl = [CR1_admin_acl, CR2_limited_acl]
        # await self.write_acl(acl)
        
        
        
        





if __name__ == "__main__":
    default_matter_test_main()
