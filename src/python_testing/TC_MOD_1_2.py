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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import random
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.clusters.Types import Nullable
from chip.exceptions import ChipStackError
from chip.native import PyChipError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.tlv import uint
from mobly import asserts

class TC_MOD_1_2(MatterBaseTest):

    def steps_TC_MOD_1_2(self) -> list[TestStep]:
        return [
            TestStep(1, "TH reads the SupportedModes attribute from DUT", is_commissioning=True),
            TestStep(2, "TH reads the CurrentMode attribute from the DUT", "Verify that the DUT response is an integer that is in the list of modes returned in step 1"),
            TestStep(3, "TH reads the OnMode attribute from the DUT", "Verify that the DUT response is an integer that is in the list of modes returned in step 1 ()"),
            TestStep(4, "TH reads the StartUpMode attribute from the DUT", "Verify that the DUT response is an integer that is in the list of modes returned in step 1"),
            TestStep(5, "TH reads the Description attribute from the DUT", "Verify that the DUT response provides human readable text string that describes the purpose of the mode select server."),
            TestStep(6, "TH reads the StandardNamespace attribute from the DUT", "Verify that the DUT response provides a 16 bit enum (null is also acceptable)."),
        ]

    def pics_TC_MOD_1_2(self) -> list[str]:
        return ["MOD.S"]

    @async_test_body
    async def test_TC_MOD_1_2(self):
        self.step(1)
        # Establishing TH1
        self.endpoint = self.get_endpoint()
        self.th1 = self.default_controller
        MOD_cluster = Clusters.ModeSelect
        attributes = MOD_cluster.Attributes        

        if await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.SupportedModes):
            mode_options = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=self.endpoint, attribute=attributes.SupportedModes)            
            modes = [option.mode for option in mode_options]

        self.step(2)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.CurrentMode):
            Current_Mode_ID = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=self.endpoint, attribute=attributes.CurrentMode)            
            asserts.assert_in(Current_Mode_ID, modes, "Current Mode ID should have been in supported modes")

        self.step(3)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.OnMode):
            # This returns Nullable and not an int 
            # yaml script shows that this is expected to return a null value as the default value according to spec 
            On_Mode_ID = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=self.endpoint, attribute=attributes.OnMode)            
            self.print_step("On Mode ID", f"On Mode ID is of {type(On_Mode_ID)} and its value is {On_Mode_ID}")
            if not isinstance(On_Mode_ID, (Clusters.Types.Nullable, int)):
                raise ValueError("Invalid return value: must be Nullable or an int")

        self.step(4)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.StartUpMode):
            Start_Up_Mode_ID = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=self.endpoint, attribute=attributes.StartUpMode)            
            asserts.assert_in(Start_Up_Mode_ID, modes, "Current Mode ID should have been in supported modes")

        self.step(5)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.Description):
            Description = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=self.endpoint, attribute=attributes.Description)            
            assert isinstance(Description, str), "Description was not a human readable string"

        self.step(6)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.StandardNamespace):
            Standard_Namespace = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=self.endpoint, attribute=attributes.StandardNamespace)            
            if not isinstance(Standard_Namespace, (uint, int)):
                raise ValueError("Standard Namespace value: must be of type uint or an int")

if __name__ == "__main__":
    default_matter_test_main()
