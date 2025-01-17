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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --passcode 20202021 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --commissioning-method on-network
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio

from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CreateNewController(MatterBaseTest):
    # Using get_code and a modified version of setup_class_helper functions from chip.testing.basic_composition module
    def get_code(self, dev_ctrl):
        created_codes = []
        for idx, discriminator in enumerate(self.matter_test_config.discriminators):
            created_codes.append(dev_ctrl.CreateManualCode(discriminator, self.matter_test_config.setup_passcodes[idx]))

        setup_codes = self.matter_test_config.qr_code_content + self.matter_test_config.manual_code + created_codes
        if not setup_codes:
            return None
        asserts.assert_equal(len(setup_codes), 1,
                             "Require exactly one of either --qr-code, --manual-code or (--discriminator and --passcode).")
        return setup_codes[0]

    async def setup_class_helper(self, allow_pase: bool = True):
        dev_ctrl = self.default_controller
        self.problems = []

        node_id = self.dut_node_id

        task_list = []
        if allow_pase and self.get_code(dev_ctrl):
            setup_code = self.get_code(dev_ctrl)
            pase_future = dev_ctrl.EstablishPASESession(setup_code, self.dut_node_id)
            task_list.append(asyncio.create_task(pase_future))

        case_future = dev_ctrl.GetConnectedDevice(nodeid=node_id, allowPASE=False)
        task_list.append(asyncio.create_task(case_future))

        for task in task_list:
            asyncio.ensure_future(task)

        done, pending = await asyncio.wait(task_list, return_when=asyncio.FIRST_COMPLETED)

        for task in pending:
            try:
                task.cancel()
                await task
            except asyncio.CancelledError:
                pass

        wildcard_read = (await dev_ctrl.Read(node_id, [()]))

        # ======= State kept for use by all tests =======
        # All endpoints in "full object" indexing format
        self.endpoints = wildcard_read.attributes

    def steps_TC_CreateNewController(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Checking create new controller on new fabric"),
            TestStep(3, "Remove new fabric from TH to verify it can be removed after creation"),
            TestStep(4, "Checking create new controller on existing fabric"),
            TestStep(5, "Remove fabric from TH to verify it can be removed after creation")
        ]

    def TC_TestAttrAvail(self) -> list[str]:
        return ["RVCOPSTATE.S"]

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()

    # ======= START OF ACTUAL TESTS =======
    @async_test_body
    async def test_TC_CreateNewController(self):
        self.step(1)
        self.endpoint = self.get_endpoint()
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")
        self.th1 = self.default_controller

        self.step(2)
        self.th2 = self.certificate_authority_manager.create_new_controller(fabricId=self.th1.fabricId + 1, CaList=1, nodeid=2)

        self.step(3)
        self.th2.Shutdown()

        self.step(4)
        self.th3 = self.certificate_authority_manager.create_new_controller(CaList=0, new_fabric=False, nodeid=2)

        self.step(5)
        self.th3.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
