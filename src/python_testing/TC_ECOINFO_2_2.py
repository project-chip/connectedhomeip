#
#    Copyright (c) 2024 Project CHIP Authors
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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.tlv import uint
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, type_matches
from mobly import asserts


class TC_ECOINFO_2_2(MatterBaseTest):

    def steps_TC_ECOINFO_2_2(self) -> list[TestStep]:
        steps = [TestStep(1, "Prepare: Collect aggregator information", is_commissioning=True),
                 TestStep(2, "Add a bridged device: Validate new Ecosystem Information cluster accessible on new endpoint"),
                 TestStep(3, "Remove bridged device: Validate Ecosystem Information's Removed on populated")]
        return steps

    @async_test_body
    async def test_TC_ECOINFO_2_2(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning, already done")
        self.step(1)
        root_node_endpoint = 0
        root_part_list = await dev_ctrl.ReadAttribute(dut_node_id, [(root_node_endpoint, Clusters.Descriptor.Attributes.PartsList)])
    
        list_of_endpoints = root_part_list[root_node_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
    
        list_of_aggregator_endpoints = []
        for endpoint in list_of_endpoints:
            device_type_list_read = await dev_ctrl.ReadAttribute(dut_node_id, [(endpoint, Clusters.Descriptor.Attributes.DeviceTypeList)])
            device_type_list = device_type_list_read[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]
            for device_type in device_type_list:
                if device_type.deviceType == 14:
                    list_of_aggregator_endpoints.append(endpoint)
    
        asserts.assert_equal(len(list_of_aggregator_endpoints), 1, "Test currently assumes one Aggregator device type")
        aggregator_endpoint = list_of_aggregator_endpoints[0]
        aggregator_part_list = await dev_ctrl.ReadAttribute(dut_node_id, [(aggregator_endpoint, Clusters.Descriptor.Attributes.PartsList)])
        step_1_set_of_aggregator_endpoints = set(aggregator_part_list[aggregator_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])
    
        # TODO what is the stop and wait for user prompt so we can add the device
        breakpoint()
        aggregator_part_list = await dev_ctrl.ReadAttribute(dut_node_id, [(aggregator_endpoint, Clusters.Descriptor.Attributes.PartsList)])
        step_2_set_of_aggregator_endpoints = set(aggregator_part_list[aggregator_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])
        
        asserts.assert_true(step_2_set_of_aggregator_endpoints.issuperset(step_1_set_of_aggregator_endpoints), "Expected only new endpoints to be added")
        unique_endpoints_set = step_2_set_of_aggregator_endpoints - step_1_set_of_aggregator_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
    
        newly_added_endpoint = list(unique_endpoints_set)[0]
        
        removed_on = await self.read_single_attribute(
            dev_ctrl,
            dut_node_id,
            endpoint=newly_added_endpoint,
            attribute=Clusters.EcosystemInformation.Attributes.RemovedOn)
    
        asserts.assert_true(removed_on is NullValue, "RemovedOn is expected to be null for a newly added device")
    
        # TODO what is the stop and wait for user prompt so we can remove the device
        breakpoint()
    
        aggregator_part_list = await dev_ctrl.ReadAttribute(dut_node_id, [(aggregator_endpoint, Clusters.Descriptor.Attributes.PartsList)])
        step_3_set_of_aggregator_endpoints = set(aggregator_part_list[aggregator_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])
    
        asserts.assert_equal(step_2_set_of_aggregator_endpoints, step_3_set_of_aggregator_endpoints, "Expect number of endpoints to be identical")
    
        removed_on = await self.read_single_attribute(
            dev_ctrl,
            dut_node_id,
            endpoint=newly_added_endpoint,
            attribute=Clusters.EcosystemInformation.Attributes.RemovedOn)
        asserts.assert_true(removed_on is not NullValue, "RemovedOn is expected to have a value")
    
        device_directory = await self.read_single_attribute(
            dev_ctrl,
            dut_node_id,
            endpoint=newly_added_endpoint,
            attribute=Clusters.EcosystemInformation.Attributes.DeviceDirectory,
            fabricFiltered=False)
        asserts.assert_equal(len(device_directory), 0, "Expected device directory to be empty")
    
        location_directory = await self.read_single_attribute(
            dev_ctrl,
            dut_node_id,
            endpoint=newly_added_endpoint,
            attribute=Clusters.EcosystemInformation.Attributes.LocationDirectory,
            fabricFiltered=False)
        asserts.assert_equal(len(location_directory), 0, "Expected location directory to be empty")
