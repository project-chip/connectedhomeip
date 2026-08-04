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


from modebase_cluster_check import ModeBaseClusterChecks

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

CLUSTER = Clusters.ThermostatMode


class TC_TSTATM_1_2(MatterBaseTest, ModeBaseClusterChecks):

    def __init__(self, *args):
        MatterBaseTest.__init__(self, *args)
        ModeBaseClusterChecks.__init__(self,
                                       modebase_derived_cluster=CLUSTER)

    def desc_TC_TSTATM_1_2(self) -> str:
        return "[TC-TSTATM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_TSTATM_1_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute."),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute."),
            TestStep(5, "TH reads from the DUT the StartUpMode attribute.",
                "Verify that the DUT response contains an integer from supported_modes_dut or null"),
        ]

    def pics_TC_TSTATM_1_2(self) -> list[str]:
        return [
            "TSTATM.S"
        ]

    @async_test_body
    async def test_TC_TSTATM_1_2(self):

        # Setup common mode check
        endpoint = self.get_endpoint()

        self.step(1)

        self.step(2)
        # Verify common checks for Mode Base as described in the TC-TSTATM-1.2
        # TH reads from the DUT the SupportedModes attribute.
        # Verify that the DUT response contains a list of ModeOptionsStruct entries
        # Verify that the list has at least 2 and at most 255 entries
        # Verify that each ModeOptionsStruct entry has a unique Mode field value
        # Verify that each ModeOptionsStruct entry has a unique Label field value
        # Verify that each ModeOptionsStruct entry’s ModeTags field has: 
        #   - at least one entry
        #   - no duplicates in ModeTag list
        #   - the values of the Value fields that are not larger than 16 bits
        #   - for each Value field:
        #       - Is the mode tag value a defined common tag value (Auto(0x0000), Quick(0x0001),
        #         Quiet(0x0002), LowNoise(0x0003), LowEnergy(0x0004), Vacation(0x0005), Min(0x0006),
        #         Max(0x0007), Night(0x0008), Day(0x0009)) or a defined derived cluster tag value 
        #         Off(0x4000), Cool(0x4001), Heat(0x4002), EmergencyHeat(0x4003)
        #       - or in the MfgTags (0x8000 to 0xBFFF) range
        supported_modes = await self.check_supported_modes_and_labels(endpoint=endpoint)
        #   - for at least one Value field: Is the mode tag value the common tag value Auto(0x0000)
        #     or a derived cluster value (Off(0x4000), Cool(0x4001), Heat(0x4002), EmergencyHeat(0x4003))
        additional_tags = [CLUSTER.Enums.ModeTag.kOff,
                           CLUSTER.Enums.ModeTag.kCool,
                           CLUSTER.Enums.ModeTag.kHeat,
                           CLUSTER.Enums.ModeTag.kEmergencyHeat]
        self.check_tags_in_lists(supported_modes=supported_modes, required_tags=additional_tags)
        # For the SupportedModes attribute: 
        #   (1) Verify that each entry contains exactly one of Off(0x4000), Cool(0x4001), Heat(0x4002), or Auto(0x0000) mode tag.
        #   (2) Verify that the Off(0x4000) tag appears in only one SupportedModes entry.
        #   (3) If the EmergencyHeat(0x4003) tag is included in a list:
        #       (3a) Verify that the Heat(0x4002) tag appears in the same list as the EmergencyHeat(0x4003) tag.
        #       (3b) One or more manufacturer tags MAY be included in the same list as the EmergencyHeat(0x4003) tag.
        #       (3c) The Heat(0x4002) SHALL appear without the EmergencyHeat(0x4003) tag in a separate list from
        #            the list which includes the Heat(0x4002) and EmergencyHeat(0x4003) tags.
        #   (4) The Heat(0x4002), Cool(0x4001) and Auto(0x0000) tags MAY appear in a list by themselves or with one
        #       or more manufacturer tag.
        #   (5) The Heat(0x4002), Cool(0x4001) or Auto(0x0000) SHALL NOT appear in a list together or in
        #       a list with any two of these tags.
        #   (6) If a manufacturer tag appears in a list, a standard tag SHALL also appear in the same list.
        # Save the Mode field values as supported_modes_dut

        self.step(3)
        # Verify that the CurrentMode attribute has a valid value.
        mode = self.cluster.Attributes.CurrentMode
        await self.read_and_check_mode(endpoint=endpoint, mode=mode, supported_modes=supported_modes)

        self.step(5)
        # TH reads from the DUT the StartUpMode attribute.
        # Verify that the DUT response contains an integer from supported_modes_dut or null
        mode = self.cluster.Attributes.StartUpMode
        await self.read_and_check_mode(endpoint=endpoint, mode=mode, supported_modes=supported_modes)

if __name__ == "__main__":
    default_matter_test_main()
