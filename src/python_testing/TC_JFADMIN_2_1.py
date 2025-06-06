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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

from support_modules import jfadmin_support as JFADMIN


class TC_JFADMIN_2_1(JFADMIN.JointFabricController):
    def __init__(self, app):
        super().__init__(app)
        
    def desc_TC_JFADMIN_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-JFADMIN-2.1] Validate AdministratorFabricIndex Attribute"

    def pics_TC_JFADMIN_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["JFADMIN.S"]

    def steps_TC_JFADMIN_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT if not already done"),
            TestStep("2", "TH reads from the DUT the AdministratorFabricIndex attribute.",
                     " Verify that the DUT response contains AdministratorFabricIndex with a value in range 1..255."),
            TestStep("3", "TH reads from the DUT the Fabrics attribute from Operation Cluster on EP0.",
                     "Verify that the DUT response contains a FabricDescriptorStruct with FabricID equal to AdministratorFabricIndex from step 1."),
        ]

        return steps
    
    @async_test_body
    async def test_TC_JFADMIN_2_1(self):
        
        self.step("1")
        self.commission_device()

        self.step("2")
        self.jfc_app.stdin.write(f"jointfabricadministrator read administratorfabricindex {node_id} {endpoint}\n".encode())
        admin_fabric_index = self.jfc_app.stdout.readlines()
        assert admin_fabric_index in range(1,255)

        self.step("3")
        self.jfc_app.stdin.write(f"operationalcredentials read fabrics {node_id} 0\n".encode())
        fabrics_list = self.jfc_app.stdout.readlines()
        fabric_found = False
        for fabric in fabrics_list:
            if fabric.fabric_id == admin_fabric_index:
                fabric_found = True
                break
        assert fabric_found is True
