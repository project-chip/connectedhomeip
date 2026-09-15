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
#     app: ${ALL_DEVICES_APP}
#     app-args: --device proximity-ranger:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import test_plan_support
from TC_PROXRTestBase import (DEVICE_IDENTITY_KEY_LEN, LTK_LEN, PMK_LEN, SESSION_KEY_LEN, BLTCSModeEnum, BLTCSSecurityLevelEnum,
                              Feature, ProximityRangingTestBase, RangingRoleEnum, StatusCodeEnum)

import matter.clusters as Clusters
import matter.testing.matter_asserts as matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

_PROXR = Clusters.ProximityRanging

_INFEASIBLE = StatusCodeEnum.kRejectedInfeasibleRanging


class TC_PROXR_2_2(MatterTestCommissionedDevice, ProximityRangingTestBase):

    def desc_TC_PROXR_2_2(self) -> str:
        return "[TC-PROXR-2.2] Processing Infeasible Proximity Ranging Configuration (DUT as Server)"

    def pics_TC_PROXR_2_2(self) -> list[str]:
        return ["PROXR.S"]

    def steps_TC_PROXR_2_2(self) -> list[TestStep]:
        reject = "Verify DUT responds with status RejectedInfeasibleRanging."
        return [
            TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep(2, test_plan_support.read_attribute("RangingCapabilities"),
                     "Verify a list of RangingCapabilitiesStruct and save the elements."),
            TestStep(3, test_plan_support.read_attribute("WiFiDevIK"),
                     "Verify an octstr value and save it."),
            TestStep(4, "TH reads the BLTDevIK, BLTCSSecurityLevel and BLTCSModeCapability attributes from DUT",
                     "Verify BLTDevIK octstr, BLTCSSecurityLevel (BLTCSSecurityLevelEnum), "
                     "BLTCSModeCapability (BLTCSModeEnum); save them."),
            TestStep(5, test_plan_support.read_attribute("BLEDeviceID"),
                     "Verify a uint64 value and save it."),
            TestStep("6a", "If WFUSDPD is not supported, TH sends StartRangingRequest (WiFiRoundTripTimeRanging, "
                     "WiFiSubscriberRole, random PeerWiFiDevIK/PMK, StartTime 0, EndTime 6).", reject),
            TestStep("6b", "If WFUSDPD is not supported, TH sends StartRangingRequest (WiFiRoundTripTimeRanging, "
                     "WiFiPublisherRole, random PeerWiFiDevIK/PMK, StartTime 0, EndTime 6).", reject),
            TestStep("7a", "If BLTCS is not supported, TH sends StartRangingRequest (BluetoothChannelSounding, "
                     "BLTInitiatorRole, random PeerBLTDevIK/LTK, highest supported security level and mode, "
                     "StartTime 0, EndTime 6).", reject),
            TestStep("7b", "If BLTCS is not supported, TH sends StartRangingRequest (BluetoothChannelSounding, "
                     "BLTReflectorRole, random PeerBLTDevIK/LTK, highest supported security level and mode, "
                     "StartTime 0, EndTime 6).", reject),
            TestStep("8a", "If BLERBC is not supported, TH sends StartRangingRequest (BLEBeaconRSSIRanging, "
                     "BLEScanningRole, random PeerBLEDeviceID, BLEDeviceIDObfuscation, octstr SessionKey, "
                     "StartTime 0, EndTime 6).", reject),
            TestStep("8b", "If BLERBC is not supported, TH sends StartRangingRequest (BLEBeaconRSSIRanging, "
                     "BLEBeaconRole, random PeerBLEDeviceID, BLEDeviceIDObfuscation, octstr SessionKey, "
                     "StartTime 0, EndTime 6).", reject),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ProximityRanging))
    async def test_TC_PROXR_2_2(self):
        endpoint = self.get_endpoint()
        feature_map = await self.read_feature_map()
        wifi = bool(feature_map & Feature.kWiFiUsdProximityDetection)
        blt = bool(feature_map & Feature.kBluetoothChannelSounding)
        ble = bool(feature_map & Feature.kBleBeaconRssi)

        self.step(1)

        self.step(2)
        caps = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities)
        matter_asserts.assert_list(caps, "RangingCapabilities", min_length=1)

        self.step(3)
        if await self.feature_guard(endpoint, _PROXR, Feature.kWiFiUsdProximityDetection):
            wifi_ik = await self.read_proxr_attribute(_PROXR.Attributes.WiFiDevIK)
            matter_asserts.assert_is_octstr(wifi_ik, "WiFiDevIK")

        self.step(4)
        if await self.feature_guard(endpoint, _PROXR, Feature.kBluetoothChannelSounding):
            blt_ik = await self.read_proxr_attribute(_PROXR.Attributes.BLTDevIK)
            matter_asserts.assert_is_octstr(blt_ik, "BLTDevIK")
            matter_asserts.assert_valid_enum(
                await self.read_proxr_attribute(_PROXR.Attributes.BLTCSSecurityLevel), "BLTCSSecurityLevel",
                BLTCSSecurityLevelEnum)
            matter_asserts.assert_valid_enum(
                await self.read_proxr_attribute(_PROXR.Attributes.BLTCSModeCapability), "BLTCSModeCapability",
                BLTCSModeEnum)

        self.step(5)
        if await self.feature_guard(endpoint, _PROXR, Feature.kBleBeaconRssi):
            matter_asserts.assert_valid_uint64(
                await self.read_proxr_attribute(_PROXR.Attributes.BLEDeviceID), "BLEDeviceID")

        # Steps 6-8 are gated on the NEGATED feature PICS: they run only when the DUT
        # does NOT support the corresponding feature (so the technology-specific role
        # config is infeasible). The all-devices-app enables all three features, so on
        # that app every one of these steps is correctly skipped (see DECISIONS.md).
        rnd = self.random_secret

        for step, role in [("6a", RangingRoleEnum.kWiFiSubscriberRole), ("6b", RangingRoleEnum.kWiFiPublisherRole)]:
            if wifi:
                self.skip_step(step)
            else:
                self.step(step)
                await self.expect_start_ranging_status(
                    self.build_wifi_request(role=role, peer_wifi_ik=rnd(DEVICE_IDENTITY_KEY_LEN), pmk=rnd(PMK_LEN)),
                    _INFEASIBLE, endpoint=endpoint)

        for step, role in [("7a", RangingRoleEnum.kBLTInitiatorRole), ("7b", RangingRoleEnum.kBLTReflectorRole)]:
            if blt:
                self.skip_step(step)
            else:
                self.step(step)
                await self.expect_start_ranging_status(
                    self.build_blt_request(role=role, peer_blt_ik=rnd(DEVICE_IDENTITY_KEY_LEN), ltk=rnd(LTK_LEN),
                                           security_level=BLTCSSecurityLevelEnum.kBLTCSSecurityLevelThree,
                                           mode=BLTCSModeEnum.kBoth),
                    _INFEASIBLE, endpoint=endpoint)

        for step, role in [("8a", RangingRoleEnum.kBLEScanningRole), ("8b", RangingRoleEnum.kBLEBeaconRole)]:
            if ble:
                self.skip_step(step)
            else:
                self.step(step)
                await self.expect_start_ranging_status(
                    self.build_ble_request(role=role, peer_ble_device_id=0x1122334455667788, session_key=rnd(SESSION_KEY_LEN)),
                    _INFEASIBLE, endpoint=endpoint)


if __name__ == "__main__":
    default_matter_test_main()
