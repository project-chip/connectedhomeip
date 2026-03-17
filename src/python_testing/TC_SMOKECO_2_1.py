#
#    Copyright (c) 2026 Project CHIP Authors
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
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_asserts import assert_int_in_range, assert_is_unixtimestamp, assert_valid_bool, assert_valid_uint32
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_1(MatterBaseTest):

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.smoke_cluster = Clusters.SmokeCoAlarm

    async def _read_attribute_check_range(self, attribute, range_low: int, range_high: int):
        """Reads an attribute from the SmokeCluster and validate against a range
        """
        if self.attribute_guard(self.get_endpoint(), attribute=attribute):
            attr = await self.read_single_attribute_check_success(cluster=self.smoke_cluster, attribute=attribute, dev_ctrl=self.default_controller, endpoint=self.get_endpoint())
            # Validate
            assert_int_in_range(attr, range_low, range_high, f"Attritute {attribute} is out of range {attr}")

    async def _read_attribute_check_bool(self, attribute):
        """Reads an attribute from the SmokeCluster and validate againts a boolean value
        """
        if self.attribute_guard(self.get_endpoint(), attribute=attribute):
            attr = await self.read_single_attribute_check_success(cluster=self.smoke_cluster, attribute=attribute, dev_ctrl=self.default_controller, endpoint=self.get_endpoint())
            assert_valid_bool(attr, f"Attritute {attribute} is not a bool instance {attr}")

    async def _read_attribute_check_epoch(self, attribute):
        """Reads an attribute from the SmokeCluster and validate againts a boolean value
        """
        if self.attribute_guard(self.get_endpoint(), attribute=attribute):
            attr = await self.read_single_attribute_check_success(cluster=self.smoke_cluster, attribute=attribute, dev_ctrl=self.default_controller, endpoint=self.get_endpoint())
            assert_valid_uint32(attr, "Attribute is not in uint range")
            assert_is_unixtimestamp(attr, f"Attribute with value: {attribute} ")

    def desc_TC_SMOKECO_2_1(self) -> str:
        return "[TC-SMOKECO-2.7] Unmount Attribute with DUT as Server"

    def steps_TC_SMOKECO_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH"),
            TestStep(2, "TH reads from the DUT the ExpressedState attribute.",
                     "Verify that the DUT response contains a value between 0 and 9"),
            TestStep(3, "TH reads from the DUT the SmokeState attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(4, "TH reads from the DUT the COState attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(5, "TH reads from the DUT the BatteryAlert attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(6, "TH reads from the DUT the DeviceMuted attribute.",
                     "Verify that the DUT response contains a value between 0 and 1"),
            TestStep(7, "TH reads from the DUT the TestInProgress attribute.",
                     "Verify that the DUT response contains a bool value"),
            TestStep(8, "TH reads from the DUT the HardwareFaultAlert attribute.",
                     "Verify that the DUT response contains a bool value"),
            TestStep(9, "TH reads from the DUT the EndOfServiceAlert attribute.",
                     "Verify that the DUT response contains a value between 0 and 1"),
            TestStep(10, "TH reads from the DUT the InterconnectSmokeAlarm attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(11, "TH reads from the DUT the InterconnectCOAlarm attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(12, "TH reads from the DUT the ContaminationState attribute.",
                     "Verify that the DUT response contains a value between 0 and 3"),
            TestStep(13, "TH reads from the DUT the SmokeSensitivityLevel attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(14, "TH reads from the DUT the ExpiryDate attribute.", "Verify that the DUT response contains epoch-s format"),
            TestStep(15, "TH reads from the DUT the Unmounted attribute.", "Verify that the DUT response contains a bool value"),
        ]

    def pics_TC_SMOKECO_2_1(self) -> list[str]:
        return [
            "SMOKECO.S.A000d",
        ]

    @async_test_body
    async def test_TC_SMOKECO_2_1(self):

        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        self.step(2)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.ExpressedState, 0, 9)

        self.step(3)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.SmokeState, 0, 2)

        self.step(4)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.COState, 0, 2)

        self.step(5)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.BatteryAlert, 0, 2)

        self.step(6)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.DeviceMuted, 0, 2)

        self.step(7)
        await self._read_attribute_check_bool(self.smoke_cluster.Attributes.TestInProgress)

        self.step(8)
        await self._read_attribute_check_bool(self.smoke_cluster.Attributes.HardwareFaultAlert)

        self.step(9)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.EndOfServiceAlert, 0, 1)

        self.step(10)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.InterconnectSmokeAlarm, 0, 2)

        self.step(11)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.InterconnectCOAlarm, 0, 2)

        self.step(12)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.ContaminationState, 0, 3)

        self.step(13)
        await self._read_attribute_check_range(self.smoke_cluster.Attributes.SmokeSensitivityLevel, 0, 2)

        self.step(14)
        await self._read_attribute_check_epoch(self.smoke_cluster.Attributes.ExpiryDate)

        self.step(15)
        await self._read_attribute_check_bool(self.smoke_cluster.Attributes.Unmounted)


if __name__ == "__main__":
    default_matter_test_main()
