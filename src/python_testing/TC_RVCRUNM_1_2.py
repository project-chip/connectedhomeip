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
#     app: ${rvc}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS examples/rvc-app/rvc-common/pics/rvc-app-pics-values
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts
from modebase_cluster_check import ModeBaseClusterChecks

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main

cluster_rvcrunm_mode = Clusters.RvcRunMode


class TC_RVCRUNM_1_2(MatterBaseTest, ModeBaseClusterChecks):
    def __init__(self, *args):
        MatterBaseTest.__init__(self, *args)
        ModeBaseClusterChecks.__init__(self,
                                       modebase_derived_cluster=cluster_rvcrunm_mode)

    def pics_TC_RVCRUNM_1_2(self) -> list[str]:
        return ["RVCRUNM.S"]

    @async_test_body
    async def test_TC_RVCRUNM_1_2(self):
        self.endpoint = self.get_endpoint()
        supported_modes = []

        self.print_step(1, "Commissioning, already done")
        if self.check_pics("RVCRUNM.S.A0000"):

            self.print_step(2, "Read SupportedModes attribute")
            # Verify common checks for Mode Base as described in the TC-RVCRUNM-1.2
            supported_modes = await self.check_supported_modes_and_labels(self.endpoint)

            self.check_tags_in_lists(supported_modes)

            # Verify that at least one ModeOptionsStruct entry includes the Idle(0x4000)
            # mode tag in the ModeTags field
            at_least_one_idle_mode_tag = False
            # Verify that at least one ModeOptionsStruct entry includes the Cleaning(0x4001)
            # mode tag in the ModeTags field
            at_least_one_cleaning_mode_tag = False
            for m in supported_modes:
                # Verify that each ModeOptionsStruct entry includes at most one of the following
                # mode tags: Idle(0x4000), Cleaning(0x4001), _Mapping(0x4002)
                count_of_idle_cleaning_or_mapping_mode_tags = 0
                for t in m.modeTags:
                    if t.value == Clusters.RvcRunMode.Enums.ModeTag.kIdle:
                        at_least_one_idle_mode_tag = True
                        count_of_idle_cleaning_or_mapping_mode_tags += 1

                    if t.value == Clusters.RvcRunMode.Enums.ModeTag.kCleaning:
                        at_least_one_cleaning_mode_tag = True
                        count_of_idle_cleaning_or_mapping_mode_tags += 1

                    if t.value == Clusters.RvcRunMode.Enums.ModeTag.kMapping:
                        count_of_idle_cleaning_or_mapping_mode_tags += 1

                if count_of_idle_cleaning_or_mapping_mode_tags > 1:
                    asserts.fail("A ModeOptionsStruct entry includes more than one of the following "
                                 "mode tags: Idle(0x4000), Cleaning(0x4001), Mapping(0x4002)!")

            asserts.assert_true(at_least_one_idle_mode_tag,
                                "The Supported Modes does not have an entry of Idle(0x4000)")
            asserts.assert_true(at_least_one_cleaning_mode_tag,
                                "The Supported Modes does not have an entry of Cleaning(0x4001)")

        if self.check_pics("RVCRUNM.S.A0001"):
            self.print_step(3, "Read CurrentMode attribute")
            mode = self.cluster.Attributes.CurrentMode
            await self.read_and_check_mode(endpoint=self.endpoint, mode=mode, supported_modes=supported_modes)


if __name__ == "__main__":
    default_matter_test_main()
