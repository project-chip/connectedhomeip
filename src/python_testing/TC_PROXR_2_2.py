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
import os

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PROXR_2_2(MatterBaseTest):

    def desc_TC_PROXR_2_2(self) -> str:
        return "[TC-PROXR-2.2] Processing Infeasible Proximity Ranging Configuration with DUT as Server"

    def pics_TC_PROXR_2_2(self) -> list[str]:
        return ["PROXR.S"]

    def steps_TC_PROXR_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read RangingCapabilities attribute from DUT"),
            TestStep(3, "Read WiFiDevIK attribute (WFUSDPD feature)"),
            TestStep(4, "Read BLTDevIK, BLTCSSecurityLevel, BLTCSModeCapability attributes (BLTCS feature)"),
            TestStep(5, "Read BLEDeviceId attribute (BLERBC feature)"),
            TestStep("6a", "Send StartRangingRequest with WiFi technology when WFUSDPD NOT supported"),
            TestStep("6b", "Send StartRangingRequest with BT CS technology when BLTCS NOT supported"),
            TestStep("6c", "Send StartRangingRequest with BLE Beacon technology when BLERBC NOT supported"),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ProximityRanging))
    async def test_TC_PROXR_2_2(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ProximityRanging
        attributes = cluster.Attributes
        commands = cluster.Commands
        enums = cluster.Enums
        structs = cluster.Structs

        self.step(1)

        # Read FeatureMap to determine supported features
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.FeatureMap)
        has_wfusdpd = (feature_map & cluster.Bitmaps.Feature.kWiFiUsdProximityDetection) != 0
        has_bltcs = (feature_map & cluster.Bitmaps.Feature.kBluetoothChannelSounding) != 0
        has_blerbc = (feature_map & cluster.Bitmaps.Feature.kBleBeaconRssi) != 0

        # Step 2: Read RangingCapabilities
        self.step(2)
        ranging_capabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.RangingCapabilities)
        asserts.assert_true(isinstance(ranging_capabilities, list), "RangingCapabilities is not a list")

        # Step 3: Read WiFiDevIK (conditional on WFUSDPD)
        self.step(3)
        if has_wfusdpd:
            await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.WiFiDevIK)
        else:
            self.mark_current_step_skipped()

        # Step 4: Read BLT attributes (conditional on BLTCS)
        self.step(4)
        if has_bltcs:
            await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLTDevIK)
            await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLTCSSecurityLevel)
            await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLTCSModeCapability)
        else:
            self.mark_current_step_skipped()

        # Step 5: Read BLEDeviceId (conditional on BLERBC)
        self.step(5)
        if has_blerbc:
            await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.BLEDeviceID)
        else:
            self.mark_current_step_skipped()

        # Step 6a: Send StartRangingRequest with WiFi when NOT supported
        self.step("6a")
        if not has_wfusdpd:
            cmd = commands.StartRangingRequest(
                technology=enums.RangingTechEnum.kWiFiRoundTripTimeRanging,
                wiFiRangingDeviceRoleConfig=structs.WiFiRangingDeviceRoleConfigStruct(
                    role=enums.RangingRoleEnum.kWiFiSubscriberRole,
                    peerWiFiDevIK=os.urandom(16),
                    pmk=os.urandom(32),
                ),
                securityMode=enums.RangingSecurityEnum.kSecureRanging,
                trigger=structs.RangingTriggerConditionStruct(
                    startTime=0,
                    endTime=10,
                ),
            )
            response = await self.send_single_cmd(cmd, endpoint=endpoint)
            asserts.assert_equal(
                response.resultCode, enums.ResultCodeEnum.kRejectedInfeasibleRanging,
                "Expected RejectedInfeasibleRanging for unsupported WiFi technology")
            asserts.assert_equal(response.sessionID, NullValue, "SessionID should be null for rejected request")
        else:
            self.mark_current_step_skipped()

        # Step 6b: Send StartRangingRequest with BT CS when NOT supported
        self.step("6b")
        if not has_bltcs:
            cmd = commands.StartRangingRequest(
                technology=enums.RangingTechEnum.kBluetoothChannelSounding,
                BLTChannelSoundingDeviceRoleConfig=structs.BLTChannelSoundingDeviceRoleConfigStruct(
                    role=enums.RangingRoleEnum.kBLTInitiatorRole,
                    peerBLTDevIK=os.urandom(16),
                    ltk=os.urandom(16),
                    BLTCSSecurityLevel=enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelOne,
                    BLTCSMode=enums.BLTCSModeEnum.kRTTOnly,
                ),
                securityMode=enums.RangingSecurityEnum.kSecureRanging,
                trigger=structs.RangingTriggerConditionStruct(
                    startTime=0,
                    endTime=10,
                ),
            )
            response = await self.send_single_cmd(cmd, endpoint=endpoint)
            asserts.assert_equal(
                response.resultCode, enums.ResultCodeEnum.kRejectedInfeasibleRanging,
                "Expected RejectedInfeasibleRanging for unsupported BT CS technology")
            asserts.assert_equal(response.sessionID, NullValue, "SessionID should be null for rejected request")
        else:
            self.mark_current_step_skipped()

        # Step 6c: Send StartRangingRequest with BLE Beacon when NOT supported
        self.step("6c")
        if not has_blerbc:
            cmd = commands.StartRangingRequest(
                technology=enums.RangingTechEnum.kBLEBeaconRSSIRanging,
                BLERangingDeviceRoleConfig=structs.BLERangingDeviceRoleConfigStruct(
                    role=enums.RangingRoleEnum.kBLEScanningRole,
                    peerBLEDeviceID=0x1234567890ABCDEF,
                ),
                trigger=structs.RangingTriggerConditionStruct(
                    startTime=0,
                    endTime=10,
                ),
            )
            response = await self.send_single_cmd(cmd, endpoint=endpoint)
            asserts.assert_equal(
                response.resultCode, enums.ResultCodeEnum.kRejectedInfeasibleRanging,
                "Expected RejectedInfeasibleRanging for unsupported BLE Beacon technology")
            asserts.assert_equal(response.sessionID, NullValue, "SessionID should be null for rejected request")
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
