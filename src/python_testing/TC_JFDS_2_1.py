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


class TC_JFDS_2_1(JFADMIN.JointFabricController):
    def __init__(self, app):
        super().__init__(app)
        
    def desc_TC_JFDS_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-JFADMIN-2.1] Validate AdministratorFabricIndex Attribute"

    def pics_TC_JFDS_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["JFADMIN.S"]

    def steps_TC_JFDS_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "TH reads AnchorRootCA attribute from DUT", 
                     "Verify that a RootCA is returned"),
            TestStep("2", "TH reads AnchorNodeID attribute from DUT",
                     "Verify that the DUT NodeId is returned"),
            TestStep("3", "TH reads AnchorVendorID attribute from DUT",
                     "Verify that the VendorId of the DUT is returned"),
            TestStep("4", "TH reads FriendlyName from DUT",
                     "Verify that the a valid string is returned"),
        ]

        return steps
    
    @async_test_body
    async def test_TC_JFDS_2_1(self):
        
        self.step("1")

        self.step("2")

        self.step("3")

        self.step("4")