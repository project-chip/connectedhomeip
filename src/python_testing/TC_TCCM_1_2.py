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
#     app: ${ENERGY_MANAGEMENT_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --application water-heater
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 2
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from tc_mode_base import TC_MODE_BASE

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TCCM_1_2(MatterBaseTest, TC_MODE_BASE):

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

        endpoint = self.get_endpoint(default=1)

        attributes = Clusters.RefrigeratorAndTemperatureControlledCabinetMode.Attributes

        self.initialize_tc_base(requested_cluster=Clusters.RefrigeratorAndTemperatureControlledCabinetMode,
                                cluster_objects=Clusters.Objects.RefrigeratorAndTemperatureControlledCabinetMode,
                                endpoint=endpoint, attributes=attributes)

        self.step(1)

        self.step(2)
        await self.check_supported_modes_and_labels()
        additional_tags = [Clusters.RefrigeratorAndTemperatureControlledCabinetMode.Enums.ModeTag.kRapidCool,
                           Clusters.RefrigeratorAndTemperatureControlledCabinetMode.Enums.ModeTag.kRapidFreeze]
        await self.check_if_labels_in_lists(requiredtags=additional_tags)

        self.step(3)
        await self.read_and_check_mode(mode=attributes.CurrentMode)

        self.step(4)
        await self.read_and_check_mode(mode=attributes.OnMode, is_nullable=True)

        self.step(5)
        await self.read_and_check_mode(mode=attributes.StartUpMode, is_nullable=True)


if __name__ == "__main__":
    default_matter_test_main()
