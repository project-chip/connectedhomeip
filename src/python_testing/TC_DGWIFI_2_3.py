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
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from chip.clusters.Types import Nullable, NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGWIFI_2_3(MatterBaseTest):
    """
    [TC-DGWIFI-2.3] Wi-Fi Network Diagnostics Cluster - Command Verification

    This test case verifies the ResetCounts command and subsequent behavior.
    """

    @staticmethod
    def is_valid_uint32(value):
        """Validates a uint32 value."""
        return isinstance(value, int) and 0 <= value <= 0xFFFFFFFF

    async def send_reset_counts_command(self, endpoint):
        """Sends the ResetCounts command to the DUT."""
        cluster = Clusters.Objects.WiFiNetworkDiagnostics
        await self.send_single_cmd(cluster.Commands.ResetCounts(), endpoint=endpoint)

    async def read_attribute_and_validate(self, endpoint, attribute, validation_func, field_name):
        """Reads an attribute and validates it using the provided function."""
        value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.WiFiNetworkDiagnostics, attribute=attribute)
        if value is None:
            return
        asserts.assert_true(isinstance(value, Nullable), f"{field_name} must be of type 'Nullable' when not None.")
        if value == NullValue:
            return
        asserts.assert_true(validation_func(value.Value), f"{field_name} has an invalid value: {value.Value}")

    def desc_TC_DGWIFI_2_3(self) -> str:
        """Returns a description of this test."""
        return "[TC-DGWIFI-2.3] Wi-Fi Diagnostics Command Verification with Server as DUT"

    def pics_TC_DGWIFI_2_3(self) -> list[str]:
        return ["DGWIFI.S"]

    def steps_TC_DGWIFI_2_3(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep("2", "Send ResetCounts command to DUT"),
            TestStep("2a", "Verify BeaconLostCount attribute after reset"),
            TestStep("2b", "Verify BeaconRxCount attribute after reset"),
            TestStep("2c", "Verify PacketMulticastRxCount attribute after reset"),
            TestStep("2d", "Verify PacketMulticastTxCount attribute after reset"),
            TestStep("2e", "Verify PacketUnicastRxCount attribute after reset"),
            TestStep("2f", "Verify PacketUnicastTxCount attribute after reset"),
        ]
        return steps

    @async_test_body
    async def test_TC_DGWIFI_2_3(self):
        endpoint = self.get_endpoint(default=0)

        # STEP 1: Commission DUT (already done)
        self.step("1")
        # Typically, we assume commissioning was performed by harness scripts.
        attributes = Clusters.WiFiNetworkDiagnostics.Attributes

        # STEP 2: Send ResetCounts command
        self.step("2")
        await self.send_reset_counts_command(endpoint)

        # STEP 2a: Verify BeaconLostCount attribute
        self.step("2a")
        await self.read_attribute_and_validate(
            endpoint,
            attributes.BeaconLostCount,
            self.is_valid_uint32,
            "BeaconLostCount"
        )

        # STEP 2b: Verify BeaconRxCount attribute
        self.step("2b")
        await self.read_attribute_and_validate(
            endpoint,
            attributes.BeaconRxCount,
            self.is_valid_uint32,
            "BeaconRxCount"
        )

        # STEP 2c: Verify PacketMulticastRxCount attribute
        self.step("2c")
        await self.read_attribute_and_validate(
            endpoint,
            attributes.PacketMulticastRxCount,
            self.is_valid_uint32,
            "PacketMulticastRxCount"
        )

        # STEP 2d: Verify PacketMulticastTxCount attribute
        self.step("2d")
        await self.read_attribute_and_validate(
            endpoint,
            attributes.PacketMulticastTxCount,
            self.is_valid_uint32,
            "PacketMulticastTxCount"
        )

        # STEP 2e: Verify PacketUnicastRxCount attribute
        self.step("2e")
        await self.read_attribute_and_validate(
            endpoint,
            attributes.PacketUnicastRxCount,
            self.is_valid_uint32,
            "PacketUnicastRxCount"
        )

        # STEP 2f: Verify PacketUnicastTxCount attribute
        self.step("2f")
        await self.read_attribute_and_validate(
            endpoint,
            attributes.PacketUnicastTxCount,
            self.is_valid_uint32,
            "PacketUnicastTxCount"
        )


if __name__ == "__main__":
    default_matter_test_main()
