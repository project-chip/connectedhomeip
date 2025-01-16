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

    @staticmethod
    def is_valid_uint_value(value, bit_count=64):
        """
        Checks if 'value' is a non-negative integer fitting into 'bit_count' bits.
        For example, bit_count=32 => must fit within 0 <= value <= 0xFFFFFFFF
        """
        if not isinstance(value, int):
            return False
        if value < 0:
            return False
        return value < 2**bit_count

    def assert_valid_bssid(self, value, field_name):
        """Asserts that the value is a valid BSSID (MAC address), None, or NullValue."""
        if isinstance(value, Nullable):
            if value == NullValue:
                return
            value = value.Value

        if value is not None:
            asserts.assert_true(self.is_valid_bssid(value),
                                f"{field_name} should be a valid BSSID string (e.g., '00:11:22:33:44:55') or None/NullValue.")

    def assert_valid_uint64(self, value, field_name):
        """Asserts that the value is a valid uint64 or None (if attribute can return NULL)."""
        asserts.assert_true(value is None or self.is_valid_uint_value(value, bit_count=64),
                            f"{field_name} should be a uint64 or NULL.")

    def assert_valid_uint32(self, value, field_name):
        """Asserts that the value is a valid uint32 or None (if attribute can return NULL)."""
        asserts.assert_true(value is None or self.is_valid_uint_value(value, bit_count=32),
                            f"{field_name} should be a uint32 or NULL.")

    def assert_valid_uint16(self, value, field_name):
        """Asserts that the value is a valid uint16 or None (if attribute can return NULL)."""
        asserts.assert_true(value is None or self.is_valid_uint_value(value, bit_count=16),
                            f"{field_name} should be a uint16 or NULL.")

    def assert_valid_uint8(self, value, field_name):
        """Asserts that the value is a valid uint16 or None (if attribute can return NULL)."""
        asserts.assert_true(value is None or self.is_valid_uint_value(value, bit_count=8),
                            f"{field_name} should be a uint8 or NULL.")

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
        if security_type is not None:
            asserts.assert_true(isinstance(security_type, Nullable),
                                "SecurityType must be of type 'Nullable' when not None.")

            if security_type is not NullValue:
                security_type_value = security_type.Value
                self.assert_valid_uint8(security_type_value, "SecurityType")

                # Check if the security_type is a valid SecurityTypeEnum member
                self.assert_true(
                    security_type_value in [item.value for item in Clusters.Objects.WiFiNetworkDiagnostics.Enums.SecurityTypeEnum],
                    f"SecurityType {security_type_value} is not a valid SecurityTypeEnum value"
                )

                # Additional check that it's not kUnknownEnumValue:
                self.assert_true(
                    security_type_value != Clusters.Objects.WiFiNetworkDiagnostics.Enums.SecurityTypeEnum.kUnknownEnumValue.value,
                    f"SecurityType should not be kUnknownEnumValue "
                    f"({Clusters.Objects.WiFiNetworkDiagnostics.Enums.SecurityTypeEnum.kUnknownEnumValue.value})"
                )

        #
        # STEP 4: TH reads WiFiVersion attribute
        #
        self.step(4)
        wifi_version = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.WiFiVersion)
        # WiFiVersion is an enum. If not configured or operational, might be NULL.
        if wifi_version is not None:
            asserts.assert_true(isinstance(wifi_version, Nullable),
                                "WiFiVersion must be of type 'Nullable' when not None.")

            if wifi_version is not NullValue:
                wifi_version_value = wifi_version.Value
                self.assert_valid_uint8(wifi_version_value, "WiFiVersion")

                # Check if the wifi_version is a valid WiFiVersionEnum member
                self.assert_true(wifi_version_value in [item.value for item in Clusters.Objects.WiFiNetworkDiagnostics.Enums.WiFiVersionEnum],
                                 f"WiFiVersion {wifi_version_value} is not a valid WiFiVersionEnum value")

                # Additional check that it's not kUnknownEnumValue:
                self.assert_true(wifi_version_value != Clusters.Objects.WiFiNetworkDiagnostics.Enums.WiFiVersionEnum.kUnknownEnumValue.value,
                                 f"WiFiVersion should not be kUnknownEnumValue ({Clusters.Objects.WiFiNetworkDiagnostics.Enums.WiFiVersionEnum.kUnknownEnumValue.value})")

        #
        # STEP 5: TH reads ChannelNumber attribute
        #
        self.step(5)
        channel_number = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.ChannelNumber)
        # If not operational, might be NULL. Else we expect an unsigned integer channel.
        if channel_number is not None:
            asserts.assert_true(isinstance(channel_number, Nullable),
                                "ChannelNumber must be of type 'Nullable' when not None.")

            if channel_number is not NullValue:
                self.assert_valid_uint16(channel_number.Value, "ChannelNumber")

        #
        # STEP 6: TH reads RSSI attribute
        #
        self.step(6)
        rssi = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.Rssi)
        # RSSI is typically a signed integer (dB). If not operational, might be NULL.
        if rssi is not None:
            asserts.assert_true(isinstance(rssi, Nullable),
                                "RSSI must be of type 'Nullable' when not None.")

            if rssi is not NullValue:
                rssi_value = rssi.Value
                asserts.assert_true(isinstance(rssi_value, int) and -120 <= rssi_value <= 0,
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
                self.assert_valid_uint32(beacon_lost_count.Value, "BeaconLostCount")

        #
        # STEP 8: TH reads BeaconRxCount attribute
        #
        self.step(8)
        beacon_rx_count = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.BeaconRxCount)
        if beacon_rx_count is not None:
            asserts.assert_true(isinstance(beacon_rx_count, Nullable),
                                "BeaconRxCount must be of type 'Nullable' when not None.")

            if beacon_rx_count is not NullValue:
                self.assert_valid_uint32(beacon_rx_count.Value, "BeaconRxCount")

        #
        # STEP 9: TH reads PacketMulticastRxCount attribute
        #
        self.step(9)
        pkt_multi_rx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketMulticastRxCount)
        if pkt_multi_rx is not None:
            asserts.assert_true(isinstance(pkt_multi_rx, Nullable),
                                "PacketMulticastRxCount must be of type 'Nullable' when not None.")

            if pkt_multi_rx is not NullValue:
                self.assert_valid_uint32(pkt_multi_rx.Value, "PacketMulticastRxCount")

        #
        # STEP 10: TH reads PacketMulticastTxCount attribute
        #
        self.step(10)
        pkt_multi_tx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketMulticastTxCount)
        if pkt_multi_tx is not None:
            asserts.assert_true(isinstance(pkt_multi_tx, Nullable),
                                "PacketMulticastTxCount must be of type 'Nullable' when not None.")

            if pkt_multi_tx is not NullValue:
                self.assert_valid_uint32(pkt_multi_tx.Value, "PacketMulticastTxCount")

        #
        # STEP 11: TH reads PacketUnicastRxCount attribute
        #
        self.step(11)
        pkt_uni_rx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketUnicastRxCount)
        if pkt_uni_rx is not None:
            asserts.assert_true(isinstance(pkt_uni_rx, Nullable),
                                "PacketUnicastRxCount must be of type 'Nullable' when not None.")

            if pkt_uni_rx is not NullValue:
                self.assert_valid_uint32(pkt_uni_rx.Value, "PacketUnicastRxCount")

        #
        # STEP 12: TH reads PacketUnicastTxCount attribute
        #
        self.step(12)
        pkt_uni_tx = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.PacketUnicastTxCount)
        if pkt_uni_tx is not None:
            asserts.assert_true(isinstance(pkt_uni_tx, Nullable),
                                "PacketUnicastTxCount must be of type 'Nullable' when not None.")

            if pkt_uni_tx is not NullValue:
                self.assert_valid_uint32(pkt_uni_tx.Value, "PacketUnicastTxCount")

        #
        # STEP 13: TH reads CurrentMaxRate attribute
        #
        self.step(13)
        current_max_rate = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentMaxRate)
        # According to the spec, this is bytes per second (uint).
        if current_max_rate is not None:
            asserts.assert_true(isinstance(current_max_rate, Nullable),
                                "CurrentMaxRate must be of type 'Nullable' when not None.")

            if current_max_rate is not NullValue:
                self.assert_valid_uint64(current_max_rate.Value, "CurrentMaxRate")

        #
        # STEP 14: TH reads OverrunCount attribute
        #
        self.step(14)
        overrun_count = await self.read_dgwifi_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverrunCount)
        # This is a uint and may reset to 0 after node reboot.
        if overrun_count is not None:
            asserts.assert_true(isinstance(overrun_count, Nullable),
                                "OverrunCount must be of type 'Nullable' when not None.")

            if overrun_count is not NullValue:
                self.assert_valid_uint64(overrun_count.Value, "OverrunCount")


if __name__ == "__main__":
    default_matter_test_main()
