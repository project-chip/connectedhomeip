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

import time
from support_modules import jfadmin_support as JFADMIN


class TC_JFADMIN_2_2(JFADMIN.JointFabricController):
    def __init__(self, app):
        super().__init__(app)
        
    def desc_TC_JFADMIN_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-JFADMIN-2.1] Validate AdministratorFabricIndex Attribute"

    def pics_TC_JFADMIN_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["JFADMIN.S"]

    def steps_TC_JFADMIN_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT if not already done"),
            TestStep("2", "TH sends ICACCSRRequest command to DUT.",
                     " DUT response contains status code FAILSAFE_REQUIRED."),
            TestStep("3", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            TestStep("4", "TH sends ICACCSRRequest command to DUT.",
                    "DUT response contains status code CONSTRAINT_ERROR."),
            TestStep("4", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                "DUT respond with ArmFailSafeResponse Command."),
            TestStep("5", "TWait for ArmFailSafe to expire.", ""),
            TestStep("6", "TH sends AddICAC command to DUT using icac1 as parameter.",
                "DUT response contains status code FAILSAFE_REQUIRED."),
            TestStep("7", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                "DUT respond with ArmFailSafeResponse Command."),
            TestStep("8", "TH sends AddICAC command to DUT using icac1 as parameter.",
                "DUT ICACResponse contains status 2 (InvalidICAC)."),
        ]

        return steps
    
    @async_test_body
    async def test_TC_JFADMIN_2_2(self):
        self.step("1")
        self.commission_device()

        self.step("2")
        self.jfc_app.stdin.write(f"jointfabricadministrator icaccsrrequest\n".encode())
        response = self.jfc_app.stdout.readlines()
        assert "FAILSAFE_REQUIRED" in response

        self.step("3")
        self.jfc_app.stdin.write(f"armfailsafe\n".encode())

        self.step("4")
        self.jfc_app.stdin.write(f"jointfabricadministrator icaccsrrequest\n".encode())
        response = self.jfc_app.stdout.readlines()
        assert "CONSTRAINT_ERROR" in response

        self.step("5")
        #TODO: Use ExpiryLengthSeconds for timeout
        time.sleep(10)

        self.step("6")
        self.jfc_app.stdin.write(f"jointfabricadministrator addicac\n".encode())
        response = self.jfc_app.stdout.readlines()
        assert "FAILSAFE_REQUIRED" in response

        self.step("7")
        self.jfc_app.stdin.write(f"armfailsafe\n".encode())

        self.step("8")
        self.jfc_app.stdin.write(f"jointfabricadministrator addicac\n".encode())
        response = self.jfc_app.stdout.readlines()
        assert "Status: 2" in response