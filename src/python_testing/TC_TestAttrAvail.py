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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --manual-code 10054912339
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
#   run2:
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
#   run3:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --endpoint 1
#       --discriminator 1234
#       --passcode 20202021
#     factory-reset: false
#     quiet: true
# === END CI TEST ARGUMENTS ===

# Run 1: Tests PASE connection using manual code
# Run 2: Tests CASE connection using manual discriminator and passcode
# Run 3: Tests without factory reset

import asyncio

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TestAttrAvail(MatterBaseTest):
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

    def steps_TC_TestAttrAvail(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Checking OperationalState attribute is available on endpoint"),
            TestStep(3, "Checking Operational Resume command is available on endpoint"),
            TestStep(4, "Checking Timezone feature is available on endpoint"),
        ]

    def TC_TestAttrAvail(self) -> list[str]:
        return ["RVCOPSTATE.S"]

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()

    # ======= START OF ACTUAL TESTS =======
    @async_test_body
    async def test_TC_TestAttrAvail(self):
        self.step(1)

        if self.matter_test_config.endpoint is None or self.matter_test_config.endpoint == 0:
            asserts.fail("--endpoint must be set and not set to 0 for this test to run correctly.")
        self.endpoint = self.get_endpoint()
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")

        cluster = Clusters.RvcOperationalState
        attributes = cluster.Attributes
        commands = cluster.Commands
        self.th1 = self.default_controller

        self.step(2)
        attr_should_be_there = await self.attribute_guard(endpoint=self.endpoint, attribute=attributes.OperationalState)
        asserts.assert_true(attr_should_be_there, True)
        self.print_step("Operational State Attr", attr_should_be_there)

        self.step(3)
        cmd_should_be_there = await self.command_guard(endpoint=self.endpoint, command=commands.Resume)
        asserts.assert_true(cmd_should_be_there, True)
        self.print_step("Operational Resume Command available ", cmd_should_be_there)

        self.step(4)
        feat_should_be_there = await self.feature_guard(endpoint=self.endpoint, cluster=Clusters.BooleanStateConfiguration, feature_int=Clusters.BooleanStateConfiguration.Bitmaps.Feature.kAudible)
        asserts.assert_true(feat_should_be_there, True)
        self.print_step("Boolean State Config Audio Feature available ", feat_should_be_there)


if __name__ == "__main__":
    default_matter_test_main()
