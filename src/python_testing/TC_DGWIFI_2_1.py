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
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGWIFI_2_1(MatterBaseTest):
    """
    [TC-DGWIFI-2.1] Wi-Fi Network Diagnostics Cluster - Attribute Read Verification

    This test case verifies the behavior of the attributes of the Wi-Fi Diagnostics cluster server.
    See the test plan steps for details on each attribute read and expected outcome.
    """

    @staticmethod
    def is_valid_bssid(bssid) -> bool:
        """
        Checks whether the BSSID is valid or None.

        In Matter, the BSSID attribute might return:
          - None (if no Wi-Fi is connected or attribute is NULL),
          - 6 bytes (raw MAC address),

        Returns True if it is valid, False otherwise.
        """
        if isinstance(bssid, bytes):
            # For raw bytes, we expect exactly 6 bytes for a MAC address
            return len(bssid) == 6

        return False

    def assert_valid_bssid(self, value, field_name):
        """Asserts that the value is a valid BSSID (MAC address), or NullValue."""
        if value is not NullValue:
            asserts.assert_true(self.is_valid_bssid(value),
                                f"{field_name} should be a valid BSSID string (e.g., '00:11:22:33:44:55') or NullValue.")

    async def read_dgwifi_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.WiFiNetworkDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_DGWIFI_2_1(self) -> str:
        """Returns a description of this test."""
        return "[TC-DGWIFI-2.1] Wi-Fi Network Diagnostics Attributes with Server as DUT"

    def pics_TC_DGWIFI_2_1(self) -> list[str]:
        return ["DGWIFI.S"]

    def steps_TC_DGWIFI_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep(2, "Read BSSID attribute"),
            TestStep(3, "Read SecurityType attribute"),
            TestStep(4, "Read WiFiVersion attribute"),
            TestStep(5, "Read ChannelNumber attribute"),
            TestStep(6, "Read RSSI attribute"),
            TestStep(7, "Read BeaconLostCount attribute"),
            TestStep(8, "Read BeaconRxCount attribute"),
            TestStep(9, "Read PacketMulticastRxCount attribute"),
            TestStep(10, "Read PacketMulticastTxCount attribute"),
            TestStep(11, "Read PacketUnicastRxCount attribute"),
            TestStep(12, "Read PacketUnicastTxCount attribute"),
            TestStep(13, "Read CurrentMaxRate attribute"),
            TestStep(14, "Read OverrunCount attribute"),
        ]
        return steps

    # ---------------------------
    # Main Test Routine
    # ---------------------------
    @async_test_body
    async def test_TC_DGWIFI_2_1(self):
        endpoint = self.get_endpoint(default=0)

        #
        # STEP 1: Commissioning (already done)
        #
        self.step(1)
        # Typically, we assume commissioning was performed by harness scripts.
        attributes = Clusters.WiFiNetworkDiagnostics.Attributes

        #
        # STEP 2: TH reads Bssid attribute
        #
        self.step(2)
        bssid = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.Bssid)
        # If the interface is not configured or not operational, a None might be returned
        self.assert_valid_bssid(bssid, "BSSID")

        #
        # STEP 3: TH reads SecurityType attribute
        #
        self.step(3)
        security_type = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.SecurityType)
        # SecurityType is an enum. If the interface is not operational, it could be NULL.
        # If not NULL, we expect an integer in the SecurityType enum range.
        # Just do a minimal check here; you can refine or extend based on the spec.
        if security_type is not NullValue:
            matter_asserts.assert_valid_uint8(security_type, "SecurityType")

            # Check if the security_type is a valid SecurityTypeEnum member
            matter_asserts.assert_valid_enum(security_type, "SecurityType",
                                             Clusters.Objects.WiFiNetworkDiagnostics.Enums.SecurityTypeEnum)

            # Additional check that it's not kUnknownEnumValue:
            asserts.assert_true(
                security_type != Clusters.Objects.WiFiNetworkDiagnostics.Enums.SecurityTypeEnum.kUnknownEnumValue,
                f"SecurityType should not be kUnknownEnumValue "
                f"({Clusters.Objects.WiFiNetworkDiagnostics.Enums.SecurityTypeEnum.kUnknownEnumValue})"
            )

        #
        # STEP 4: TH reads WiFiVersion attribute
        #
        self.step(4)
        wifi_version = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.WiFiVersion)
        # WiFiVersion is an enum. If not configured or operational, might be NULL.
        if wifi_version is not NullValue:
            matter_asserts.assert_valid_uint8(wifi_version, "WiFiVersion")

            # Check if the wifi_version is a valid WiFiVersionEnum member
            matter_asserts.assert_valid_enum(wifi_version, "WiFiVersion",
                                             Clusters.Objects.WiFiNetworkDiagnostics.Enums.WiFiVersionEnum)

            # Additional check that it's not kUnknownEnumValue:
            asserts.assert_true(wifi_version != Clusters.Objects.WiFiNetworkDiagnostics.Enums.WiFiVersionEnum.kUnknownEnumValue,
                                f"WiFiVersion should not be kUnknownEnumValue ({Clusters.Objects.WiFiNetworkDiagnostics.Enums.WiFiVersionEnum.kUnknownEnumValue})")

        #
        # STEP 5: TH reads ChannelNumber attribute
        #
        self.step(5)
        channel_number = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.ChannelNumber)
        # If not operational, might be NULL. Else we expect an unsigned integer channel.
        if channel_number is not NullValue:
            matter_asserts.assert_valid_uint16(channel_number, "ChannelNumber")

        #
        # STEP 6: TH reads RSSI attribute
        #
        self.step(6)
        rssi = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.Rssi)
        # RSSI is typically a signed integer (dB). If not operational, might be NULL.
        if rssi is not NullValue:
            asserts.assert_true(isinstance(rssi, int) and -120 <= rssi <= 0,
                                "rssi_value is not within valid range.")

        #
        # STEP 7: TH reads BeaconLostCount attribute
        #
        self.step(7)
        beacon_lost_count = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.BeaconLostCount)
        # Expect unsigned int. Not to be subscribed.
        if beacon_lost_count is not None:
            asserts.assert_true(isinstance(beacon_lost_count, Nullable),
                                "BeaconLostCount must be of type 'Nullable' when not None.")

            if beacon_lost_count is not NullValue:
                matter_asserts.assert_valid_uint32(beacon_lost_count, "BeaconLostCount")

        #
        # STEP 8: TH reads BeaconRxCount attribute
        #
        self.step(8)
        beacon_rx_count = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.BeaconRxCount)
        if beacon_rx_count is not None:
            if beacon_rx_count is not NullValue:
                matter_asserts.assert_valid_uint32(beacon_rx_count, "BeaconRxCount")

        #
        # STEP 9: TH reads PacketMulticastRxCount attribute
        #
        self.step(9)
        pkt_multi_rx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketMulticastRxCount)
        if pkt_multi_rx is not None:
            if pkt_multi_rx is not NullValue:
                matter_asserts.assert_valid_uint32(pkt_multi_rx, "PacketMulticastRxCount")

        #
        # STEP 10: TH reads PacketMulticastTxCount attribute
        #
        self.step(10)
        pkt_multi_tx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketMulticastTxCount)
        if pkt_multi_tx is not None:
            if pkt_multi_tx is not NullValue:
                matter_asserts.assert_valid_uint32(pkt_multi_tx, "PacketMulticastTxCount")

        #
        # STEP 11: TH reads PacketUnicastRxCount attribute
        #
        self.step(11)
        pkt_uni_rx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketUnicastRxCount)
        if pkt_uni_rx is not None:
            if pkt_uni_rx is not NullValue:
                matter_asserts.assert_valid_uint32(pkt_uni_rx, "PacketUnicastRxCount")

        #
        # STEP 12: TH reads PacketUnicastTxCount attribute
        #
        self.step(12)
        pkt_uni_tx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketUnicastTxCount)
        if pkt_uni_tx is not None:
            if pkt_uni_tx is not NullValue:
                matter_asserts.assert_valid_uint32(pkt_uni_tx, "PacketUnicastTxCount")

        #
        # STEP 13: TH reads CurrentMaxRate attribute
        #
        self.step(13)
        current_max_rate = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentMaxRate)
        # According to the spec, this is bytes per second (uint).
        if current_max_rate is not None:
            if current_max_rate is not NullValue:
                matter_asserts.assert_valid_uint64(current_max_rate, "CurrentMaxRate")

        #
        # STEP 14: TH reads OverrunCount attribute
        #
        self.step(14)
        overrun_count = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverrunCount)
        # This is a uint and may reset to 0 after node reboot.
        if overrun_count is not None:
            if overrun_count is not NullValue:
                matter_asserts.assert_valid_uint64(overrun_count, "OverrunCount")


if __name__ == "__main__":
    default_matter_test_main()
