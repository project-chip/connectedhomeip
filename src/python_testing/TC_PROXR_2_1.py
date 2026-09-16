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
from mobly import asserts
from TC_PROXRTestBase import (DEVICE_IDENTITY_KEY_LEN, WIFI_TECHNOLOGIES, BLTCSModeEnum, BLTCSSecurityLevelEnum, Feature,
                              ProximityRangingTestBase, RangingTechEnum)

import matter.clusters as Clusters
import matter.testing.matter_asserts as matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

_PROXR = Clusters.ProximityRanging


class TC_PROXR_2_1(MatterTestCommissionedDevice, ProximityRangingTestBase):

    def desc_TC_PROXR_2_1(self) -> str:
        return "[TC-PROXR-2.1] Attributes (DUT as Server)"

    def pics_TC_PROXR_2_1(self) -> list[str]:
        return ["PROXR.S"]

    def steps_TC_PROXR_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2a", test_plan_support.read_attribute("RangingCapabilities"),
                     "Verify a list of RangingCapabilitiesStruct; exactly one element has Technology "
                     "WiFiRoundTripTimeRanging or WiFiNextGenerationRanging; MaxConcurrentSessions is present and "
                     "greater than 0."),
            TestStep("2b", test_plan_support.read_attribute("RangingCapabilities"),
                     "Verify exactly one element has Technology BluetoothChannelSounding; "
                     "MaxConcurrentSessions is present and greater than 0."),
            TestStep("2c", test_plan_support.read_attribute("RangingCapabilities"),
                     "Verify exactly one element has Technology BLEBeaconRSSIRanging; "
                     "MaxConcurrentSessions is present and greater than 0."),
            TestStep("3a", test_plan_support.read_attribute("WiFiDevIK"), "Verify an octstr value of length 16."),
            TestStep("3b", "TH reads the BLTDevIK, BLTCSSecurityLevel and BLTCSModeCapability attributes from DUT",
                     "Verify BLTDevIK is an octstr of length 16, BLTCSSecurityLevel is a BLTCSSecurityLevelEnum, "
                     "BLTCSModeCapability is a BLTCSModeEnum."),
            TestStep("3c", test_plan_support.read_attribute("BLEDeviceID"), "Verify a valid uint64 value."),
            TestStep(4, test_plan_support.read_attribute("SessionIDList"), "Verify the list has zero elements."),
            TestStep(5, test_plan_support.read_attribute("RangingConstraints"),
                     "If present, verify each element has a Technology (RangingTechEnum) and Role (RangingRoleEnum); "
                     "BluetoothChannelSounding elements include a BLTCSMode (BLTCSModeEnum); elements with Enabled=false "
                     "do not include MinRangingInterval, MaxSessionDuration or MaxRangingInstances."),
        ]

    def _verify_single_capability(self, capabilities, technologies):
        matter_asserts.assert_list(capabilities, "RangingCapabilities")
        matching = [c for c in capabilities if c.technology in technologies]
        asserts.assert_equal(len(matching), 1,
                             f"Expected exactly one RangingCapabilities element for {technologies}, got {len(matching)}")
        cap = matching[0]
        # MaxConcurrentSessions is treated as MANDATORY here: the field is planned to
        # change from optional to mandatory in the spec, so this test holds devices to
        # that now. NOTE the deliberate divergence from the current SDK data model,
        # which still declares `optional int8u maxConcurrentSessions = 6` in
        # RangingCapabilitiesStruct - do not relax this assertion back to a
        # present-only check; it is ahead of the data model on purpose.
        # The bound is the plan's own wording for steps 2a/2b/2c ("Verify the
        # MaxConcurrentSessions field is greater than 0"), so a device supporting
        # exactly one concurrent session is conformant.
        asserts.assert_is_not_none(cap.maxConcurrentSessions,
                                   "MaxConcurrentSessions must be present (mandatory)")
        matter_asserts.assert_valid_uint8(cap.maxConcurrentSessions, "MaxConcurrentSessions")
        asserts.assert_greater(cap.maxConcurrentSessions, 0, "MaxConcurrentSessions must be greater than 0")
        return cap

    @run_if_endpoint_matches(has_cluster(Clusters.ProximityRanging))
    async def test_TC_PROXR_2_1(self):
        endpoint = self.get_endpoint()

        self.step(1)

        self.step("2a")
        if await self.feature_guard(endpoint, _PROXR, Feature.kWiFiUsdProximityDetection):
            caps = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities)
            self._verify_single_capability(caps, WIFI_TECHNOLOGIES)

        self.step("2b")
        if await self.feature_guard(endpoint, _PROXR, Feature.kBluetoothChannelSounding):
            caps = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities)
            self._verify_single_capability(caps, (RangingTechEnum.kBluetoothChannelSounding,))

        self.step("2c")
        if await self.feature_guard(endpoint, _PROXR, Feature.kBleBeaconRssi):
            caps = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities)
            self._verify_single_capability(caps, (RangingTechEnum.kBLEBeaconRSSIRanging,))

        self.step("3a")
        if await self.feature_guard(endpoint, _PROXR, Feature.kWiFiUsdProximityDetection):
            wifi_ik = await self.read_proxr_attribute(_PROXR.Attributes.WiFiDevIK)
            matter_asserts.assert_is_octstr(wifi_ik, "WiFiDevIK")
            asserts.assert_equal(len(wifi_ik), DEVICE_IDENTITY_KEY_LEN, "WiFiDevIK must be 16 bytes")

        self.step("3b")
        if await self.feature_guard(endpoint, _PROXR, Feature.kBluetoothChannelSounding):
            blt_ik = await self.read_proxr_attribute(_PROXR.Attributes.BLTDevIK)
            matter_asserts.assert_is_octstr(blt_ik, "BLTDevIK")
            asserts.assert_equal(len(blt_ik), DEVICE_IDENTITY_KEY_LEN, "BLTDevIK must be 16 bytes")
            sec_level = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSSecurityLevel)
            matter_asserts.assert_valid_enum(sec_level, "BLTCSSecurityLevel", BLTCSSecurityLevelEnum)
            mode = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSModeCapability)
            matter_asserts.assert_valid_enum(mode, "BLTCSModeCapability", BLTCSModeEnum)

        self.step("3c")
        if await self.feature_guard(endpoint, _PROXR, Feature.kBleBeaconRssi):
            ble_id = await self.read_proxr_attribute(_PROXR.Attributes.BLEDeviceID)
            matter_asserts.assert_valid_uint64(ble_id, "BLEDeviceID")

        self.step(4)
        session_ids = await self.read_session_id_list()
        matter_asserts.assert_list(session_ids, "SessionIDList", max_length=0)

        self.step(5)
        if await self.attribute_guard(endpoint, _PROXR.Attributes.RangingConstraints):
            constraints = await self.read_proxr_attribute(_PROXR.Attributes.RangingConstraints)
            if constraints is not None:
                for c in constraints:
                    matter_asserts.assert_valid_enum(c.technology, "RangingConstraint.Technology", RangingTechEnum)
                    matter_asserts.assert_valid_enum(c.role, "RangingConstraint.Role",
                                                     _PROXR.Enums.RangingRoleEnum)
                    # The plan asks to verify a BLTCSMode field on BluetoothChannelSounding
                    # constraint elements, but RangingConstraintStruct has no BLTCSMode field
                    # in the cluster definition (see DECISIONS.md), so that sub-check is omitted.
                    if c.enabled is False:
                        asserts.assert_is_none(c.minRangingInterval,
                                               "Disabled RangingConstraint must not include MinRangingInterval")
                        asserts.assert_is_none(c.maxSessionDuration,
                                               "Disabled RangingConstraint must not include MaxSessionDuration")
                        asserts.assert_is_none(c.maxRangingInstances,
                                               "Disabled RangingConstraint must not include MaxRangingInstances")


if __name__ == "__main__":
    default_matter_test_main()
