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
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from modebase_cluster_check import ModeBaseClusterChecks

CLUSTER = Clusters.RefrigeratorAndTemperatureControlledCabinetMode


class TC_TCCM_1_2(MatterBaseTest, ModeBaseClusterChecks):

    def __init__(self, *args):
        MatterBaseTest.__init__(self, *args)
        ModeBaseClusterChecks.__init__(self,
                                       modebase_derived_cluster=CLUSTER)

    def desc_TC_TCCM_1_2(self) -> str:
        return "[TC-TCCM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_TCCM_1_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute."),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(4, "TH reads from the DUT the OnMode attribute."),
            TestStep(5, "TH reads from the DUT the StartUpMode attribute.")
        ]
        return steps

    def pics_TC_TCCM_1_2(self) -> list[str]:
        pics = [
            "TCCM.S"
        ]
        return pics

    @async_test_body
    async def test_TC_TCCM_1_2(self):

        # Setup common mode check
        endpoint = self.get_endpoint(default=1)

        self.step(1)

        self.step(2)
        # Verify common checks for Mode Base as described in the TC-TCCM-1.2
        supported_modes = await self.check_supported_modes_and_labels(endpoint=endpoint)
        # According to the spec, there should be at least one RapidCool or RapidFreeze tag in
        # the ones supported.
        additional_tags = [CLUSTER.Enums.ModeTag.kRapidCool,
                           CLUSTER.Enums.ModeTag.kRapidFreeze]
        self.check_tags_in_lists(supported_modes=supported_modes, required_tags=additional_tags)

        self.step(3)
        # Verify that the CurrentMode attribute has a valid value.
        mode = self.cluster.Attributes.CurrentMode
        await self.read_and_check_mode(endpoint=endpoint, mode=mode, supported_modes=supported_modes)

        self.step(4)
        # Verify that the OnMode attribute has a valid value or null.
        mode = self.cluster.Attributes.OnMode
        await self.read_and_check_mode(endpoint=endpoint, mode=mode,
                                       supported_modes=supported_modes, is_nullable=True)

        self.step(5)
        # Verify that the StartUpMode has a valid value or null
        mode = self.cluster.Attributes.StartUpMode
        await self.read_and_check_mode(endpoint=endpoint, mode=mode,
                                       supported_modes=supported_modes, is_nullable=True)


if __name__ == "__main__":
    default_matter_test_main()
