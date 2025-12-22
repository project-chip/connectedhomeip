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

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import TestStep, default_matter_test_main


class TC_TestAttrAvail(BasicCompositionTests):
    def steps_TC_TestAttrAvail(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Checking OperationalState attribute is available on endpoint"),
            TestStep(3, "Checking Operational Resume command is available on endpoint"),
            TestStep(4, "Checking Boolean State Config Audible feature is available on endpoint"),
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
