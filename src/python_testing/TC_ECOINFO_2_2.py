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

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

_DEVICE_TYPE_AGGREGGATOR = 0x000E


class TC_ECOINFO_2_2(MatterBaseTest):

    def steps_TC_ECOINFO_2_2(self) -> list[TestStep]:
        steps = [TestStep(1, "Prepare", is_commissioning=True),
                 TestStep("1a", "Read root endpoint's PartsList"),
                 TestStep("1b", "For each endpoint in 1a read DeviceType list confirming aggregator endpoint exists"),
                 TestStep(2, "Add a bridged device"),
                 TestStep("2a", "(Manual Step) Add a bridged device using method indicated by the manufacturer"),
                 TestStep("2b", "Read root endpoint's PartsList, validate exactly one endpoint added"),
                 TestStep("2c", "On newly added endpoint detected in 2b read DeviceDirectory Ecosystem Information Attribute and validate success"),
                 TestStep("2d", "On newly added endpoint detected in 2b read LocationDirectory Ecosystem Information Attribute and validate success"),
                 TestStep(3, "Remove bridged device"),
                 TestStep("3a", "(Manual Step) Removed bridged device added in step 2a using method indicated by the manufacturer"),
                 TestStep("3b", "Verify that PartsList equals what was read in 1a"),
                 TestStep("3c", "On endpoint detected in 2b, read DeviceDirectory Ecosystem Information Attribute and validate failure"),
                 TestStep("3d", "On endpoint detected in 2b, read LocationDirectory Ecosystem Information Attribute and validate failure")]

        return steps

    # This test has some manual steps, so we need a longer timeout. Test typically runs under 1 mins so 3 mins should
    # be enough time for test to run
    @property
    def default_timeout(self) -> int:
        return 3*60

    @async_test_body
    async def test_TC_ECOINFO_2_2(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning, already done")
        self.step(1)
        self.step("1a")
        root_node_endpoint = 0
        root_part_list = await dev_ctrl.ReadAttribute(dut_node_id, [(root_node_endpoint, Clusters.Descriptor.Attributes.PartsList)])

        self.step("1b")
        set_of_endpoints_step_1 = set(root_part_list[root_node_endpoint]
                                      [Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])
        list_of_aggregator_endpoints = []
        for endpoint in set_of_endpoints_step_1:
            device_type_list_read = await dev_ctrl.ReadAttribute(dut_node_id, [(endpoint, Clusters.Descriptor.Attributes.DeviceTypeList)])
            device_type_list = device_type_list_read[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]
            for device_type in device_type_list:
                if device_type.deviceType == _DEVICE_TYPE_AGGREGGATOR:
                    list_of_aggregator_endpoints.append(endpoint)

        asserts.assert_greater_equal(len(list_of_aggregator_endpoints), 1, "Did not find any Aggregator device types")

        self.step(2)
        self.step("2a")
        self.wait_for_user_input(prompt_msg="Add a bridged device using method indicated by the manufacturer")

        self.step("2b")
        root_part_list_step_2 = await dev_ctrl.ReadAttribute(dut_node_id, [(root_node_endpoint, Clusters.Descriptor.Attributes.PartsList)])
        set_of_endpoints_step_2 = set(
            root_part_list_step_2[root_node_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])

        asserts.assert_true(set_of_endpoints_step_2.issuperset(set_of_endpoints_step_1), "Expected only new endpoints to be added")
        unique_endpoints_set = set_of_endpoints_step_2 - set_of_endpoints_step_1
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")

        self.step("2c")
        newly_added_endpoint = list(unique_endpoints_set)[0]
        await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            node_id=dut_node_id,
            endpoint=newly_added_endpoint,
            cluster=Clusters.EcosystemInformation,
            attribute=Clusters.EcosystemInformation.Attributes.DeviceDirectory,
            fabric_filtered=False)

        self.step("2d")
        await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            node_id=dut_node_id,
            endpoint=newly_added_endpoint,
            cluster=Clusters.EcosystemInformation,
            attribute=Clusters.EcosystemInformation.Attributes.LocationDirectory,
            fabric_filtered=False)

        self.step(3)
        self.step("3a")
        self.wait_for_user_input(prompt_msg="Removed bridged device added in step 2a using method indicated by the manufacturer")

        self.step("3b")
        root_part_list_step_3 = await dev_ctrl.ReadAttribute(dut_node_id, [(root_node_endpoint, Clusters.Descriptor.Attributes.PartsList)])
        set_of_endpoints_step_3 = set(
            root_part_list_step_3[root_node_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])

        asserts.assert_equal(set_of_endpoints_step_3, set_of_endpoints_step_1,
                             "Expected set of endpoints after removal to be identical to when test started")

        self.step("3c")
        newly_added_endpoint = list(unique_endpoints_set)[0]
        await self.read_single_attribute_expect_error(
            dev_ctrl=dev_ctrl,
            node_id=dut_node_id,
            error=Status.UnsupportedEndpoint,
            endpoint=newly_added_endpoint,
            cluster=Clusters.EcosystemInformation,
            attribute=Clusters.EcosystemInformation.Attributes.DeviceDirectory,
            fabric_filtered=False)

        self.step("3d")
        await self.read_single_attribute_expect_error(
            dev_ctrl=dev_ctrl,
            node_id=dut_node_id,
            error=Status.UnsupportedEndpoint,
            endpoint=newly_added_endpoint,
            cluster=Clusters.EcosystemInformation,
            attribute=Clusters.EcosystemInformation.Attributes.LocationDirectory,
            fabric_filtered=False)


if __name__ == "__main__":
    default_matter_test_main()
