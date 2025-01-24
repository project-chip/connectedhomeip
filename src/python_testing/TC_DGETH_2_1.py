#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
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
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGETH_2_1(MatterBaseTest):
    """
    [TC-DGETH-2.1] Ethernet Diagnostics Cluster - Attribute Read Verification

    This test case verifies the behavior of the attributes of the Ethernet Diagnostics cluster server.
    See the test plan steps for details on each attribute read and expected outcome.

    Requirements:
    - The Test Harness and DUT must be running on different physical devices.
    - Communication between the Test Harness and DUT should occur via Ethernet.    
    """

    async def read_dgeth_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.EthernetNetworkDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_DGETH_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGETH-2.1] Attributes with Server as DUT"

    def pics_TC_DGETH_2_1(self) -> list[str]:
        return ["DGETH.S"]

    def steps_TC_DGETH_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read PHYRate attribute"),
            TestStep(3, "Read FullDuplex attribute"),
            TestStep(4, "Read PacketRxCount attribute"),
            TestStep(5, "Read PacketTxCount attribute"),
            TestStep(6, "Read TxErrCount attribute"),
            TestStep(7, "Read CollisionCount attribute"),
            TestStep(8, "Read OverrunCount attribute"),
            TestStep(9, "Read CarrierDetect attribute"),
            TestStep(10, "Read TimeSinceReset attribute"),
        ]
        return steps

    @async_test_body
    async def test_TC_DGETH_2_1(self):

        endpoint = self.get_endpoint(0)

        # STEP 1: Commission DUT (already done)
        self.step(1)

        attributes = Clusters.EthernetNetworkDiagnostics.Attributes

        # STEP 2: TH reads from the DUT the PHYRate attribute
        self.step(2)
        phy_rate_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.PHYRate)
        if phy_rate_attr is not None:
            if phy_rate_attr is not NullValue:
                matter_asserts.assert_valid_enum(phy_rate_attr, "PHYRate", Clusters.EthernetNetworkDiagnostics.Enums.PHYRateEnum)

        # STEP 3: TH reads from the DUT the FullDuplex attribute
        self.step(3)
        full_duplex_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.FullDuplex)
        if full_duplex_attr is not None:
            if full_duplex_attr is not NullValue:
                matter_asserts.assert_valid_bool(full_duplex_attr, "FullDuplex")

        # STEP 4: TH reads from the DUT the PacketRxCount attribute
        self.step(4)
        packet_rx_count_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketRxCount)
        if packet_rx_count_attr is not None:
            matter_asserts.assert_valid_uint64(packet_rx_count_attr, "PacketRxCount")
            if not self.is_pics_sdk_ci_only:
                asserts.assert_true(packet_rx_count_attr > 0, f"PacketRxCount ({packet_rx_count_attr}) should be > 0)")

        # STEP 5: TH reads from the DUT the PacketTxCount attribute
        self.step(5)
        packet_tx_count_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketTxCount)
        if packet_tx_count_attr is not None:
            matter_asserts.assert_valid_uint64(packet_tx_count_attr, "PacketTxCount")
            if not self.is_pics_sdk_ci_only:
                asserts.assert_true(packet_tx_count_attr > 0, f"PacketTxCount ({packet_tx_count_attr}) should be > 0)")

        # STEP 6: TH reads from the DUT the TxErrCount attribute
        self.step(6)
        tx_err_count_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.TxErrCount)
        if tx_err_count_attr is not None:
            matter_asserts.assert_valid_uint64(tx_err_count_attr, "TxErrCount")

        # STEP 7: TH reads from the DUT the CollisionCount attribute
        self.step(7)
        collision_count_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.CollisionCount)
        if collision_count_attr is not None:
            matter_asserts.assert_valid_uint64(collision_count_attr, "CollisionCount")

        # STEP 8: TH reads from the DUT the OverrunCount attribute
        self.step(8)
        overrun_count_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverrunCount)
        if overrun_count_attr is not None:
            matter_asserts.assert_valid_uint64(overrun_count_attr, "OverrunCount")

        # STEP 9: TH reads from the DUT the CarrierDetect attribute
        self.step(9)
        carrier_detect_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.CarrierDetect)
        if carrier_detect_attr is not None:
            if carrier_detect_attr is not NullValue:
                matter_asserts.assert_valid_bool(carrier_detect_attr, "CarrierDetect")

        # STEP 10: TH reads from the DUT the TimeSinceReset attribute
        self.step(10)
        time_since_reset_attr = await self.read_dgeth_attribute_expect_success(endpoint=endpoint, attribute=attributes.TimeSinceReset)
        if time_since_reset_attr is not None:
            matter_asserts.assert_valid_uint32(time_since_reset_attr, "TimeSinceReset")


if __name__ == "__main__":
    default_matter_test_main()
