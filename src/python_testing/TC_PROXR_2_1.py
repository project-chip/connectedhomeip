#
#    Copyright (c) 2025 Project CHIP Authors
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
#     app: ${ALL_DEVICES_APP}
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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PROXR_2_1(MatterBaseTest):

    def desc_TC_PROXR_2_1(self) -> str:
        return "[TC-PROXR-2.1] Attributes with DUT as Server"

    def pics_TC_PROXR_2_1(self) -> list[str]:
        return ["PROXR.S"]

    def steps_TC_PROXR_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read RangingCapabilities attribute - verify WiFi technology entries (WFUSDPD feature)"),
            TestStep("2b", "Read RangingCapabilities attribute - verify BT CS technology entries (BLTCS feature)"),
            TestStep("2c", "Read RangingCapabilities attribute - verify BLE RSSI technology entries (BLERBC feature)"),
            TestStep("3a", "Read WiFiDevIK attribute (WFUSDPD feature)"),
            TestStep("3b", "Read BLTDevIK, BLTCSSecurityLevel, BLTCSModeCapability attributes (BLTCS feature)"),
            TestStep("3c", "Read BLEDeviceId attribute (BLERBC feature)"),
            TestStep(4, "Read SessionIDs attribute - verify empty list"),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ProximityRanging))
    async def test_TC_PROXR_2_1(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ProximityRanging
        attributes = cluster.Attributes

        self.step(1)

        # Read FeatureMap to determine supported features
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.FeatureMap)
        has_wfusdpd = (feature_map & cluster.Bitmaps.Feature.kWiFiUsdProximityDetection) != 0
        has_bltcs = (feature_map & cluster.Bitmaps.Feature.kBluetoothChannelSounding) != 0
        has_blerbc = (feature_map & cluster.Bitmaps.Feature.kBleBeaconRssi) != 0

        # Step 2a: Read RangingCapabilities - verify WiFi technology entries
        self.step("2a")
        if has_wfusdpd:
            ranging_capabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.RangingCapabilities)
            wifi_techs = {cluster.Enums.RangingTechEnum.kWiFiRoundTripTimeRanging,
                          cluster.Enums.RangingTechEnum.kWiFiNextGenerationRanging}
            found = any(entry.technology in wifi_techs for entry in ranging_capabilities)
            asserts.assert_true(found, "RangingCapabilities does not contain a WiFi technology entry")
        else:
            self.mark_current_step_skipped()

        # Step 2b: Read RangingCapabilities - verify BT CS technology entries
        self.step("2b")
        if has_bltcs:
            ranging_capabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.RangingCapabilities)
            found = any(entry.technology == cluster.Enums.RangingTechEnum.kBluetoothChannelSounding
                        for entry in ranging_capabilities)
            asserts.assert_true(found, "RangingCapabilities does not contain a BluetoothChannelSounding entry")
        else:
            self.mark_current_step_skipped()

        # Step 2c: Read RangingCapabilities - verify BLE RSSI technology entries
        self.step("2c")
        if has_blerbc:
            ranging_capabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.RangingCapabilities)
            found = any(entry.technology == cluster.Enums.RangingTechEnum.kBLEBeaconRSSIRanging
                        for entry in ranging_capabilities)
            asserts.assert_true(found, "RangingCapabilities does not contain a BLEBeaconRSSIRanging entry")
        else:
            self.mark_current_step_skipped()

        # Step 3a: Read WiFiDevIK (WFUSDPD feature)
        self.step("3a")
        if has_wfusdpd:
            wifi_dev_ik = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.WiFiDevIK)
            asserts.assert_true(isinstance(wifi_dev_ik, bytes), "WiFiDevIK is not an octet string")
            asserts.assert_equal(len(wifi_dev_ik), 16, "WiFiDevIK must be 16 bytes")
        else:
            self.mark_current_step_skipped()

        # Step 3b: Read BLTDevIK, BLTCSSecurityLevel, BLTCSModeCapability (BLTCS feature)
        self.step("3b")
        if has_bltcs:
            blt_dev_ik = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLTDevIK)
            asserts.assert_true(isinstance(blt_dev_ik, bytes), "BLTDevIK is not an octet string")
            asserts.assert_equal(len(blt_dev_ik), 16, "BLTDevIK must be 16 bytes")

            blt_cs_security_level = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLTCSSecurityLevel)
            asserts.assert_true(
                isinstance(blt_cs_security_level, cluster.Enums.BLTCSSecurityLevelEnum),
                "BLTCSSecurityLevel is not a BLTCSSecurityLevelEnum value")

            blt_cs_mode_capability = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLTCSModeCapability)
            asserts.assert_true(
                isinstance(blt_cs_mode_capability, cluster.Enums.BLTCSModeEnum),
                "BLTCSModeCapability is not a BLTCSModeEnum value")
        else:
            self.mark_current_step_skipped()

        # Step 3c: Read BLEDeviceId (BLERBC feature)
        self.step("3c")
        if has_blerbc:
            ble_device_id = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLEDeviceID)
            asserts.assert_true(isinstance(ble_device_id, int), "BLEDeviceId is not a uint64")
            asserts.assert_greater(ble_device_id, 0, "BLEDeviceId is not a non-zero identifier")
        else:
            self.mark_current_step_skipped()

        # Step 4: Read SessionIDs attribute - verify empty list
        self.step(4)
        session_ids = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.SessionIDList)
        asserts.assert_true(isinstance(session_ids, list), "SessionIDs is not a list")
        asserts.assert_equal(len(session_ids), 0, "SessionIDs should be an empty list")


if __name__ == "__main__":
    default_matter_test_main()
