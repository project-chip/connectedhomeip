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
#     app: ${ELECTRICAL_PROTECTION_APP}
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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_EPALM_2_3(MatterBaseTest):

    @property
    def default_endpoint(self) -> int:
        return 1

    def _check_feature(self, feature_map: int, supported: int, feature_bit, alarm_bit) -> None:
        """Assert the alarm bit is set in Supported when its feature is declared; skip if not."""
        if not feature_map & feature_bit:
            log.info("Feature %s: not present, skipping", feature_bit.name)
            self.mark_current_step_skipped()
            return
        asserts.assert_true(
            bool(supported & alarm_bit),
            f"Feature {feature_bit.name} is set in FeatureMap but corresponding alarm bit "
            f"{alarm_bit.name} is NOT set in Supported")
        log.info("Feature %s: present, alarm bit %s verified in Supported",
                 feature_bit.name, alarm_bit.name)

    @pics('EPALM.S')
    @async_test_body
    async def test_TC_EPALM_2_3(self):
        """[TC-EPALM-2.3] FeatureMap and Supported Attribute Consistency with DUT as Server

        This test case verifies that the bits set in the inherited Alarm Base Supported attribute
        are consistent with the alarm-condition feature bits declared in the EPALM FeatureMap. The
        check is bidirectional: (a) every feature bit set in the FeatureMap MUST have its
        corresponding bit set in Supported, and (b) every bit set in Supported MUST correspond to a
        feature bit set in the FeatureMap (no "orphan" Supported bits, no bits outside the seven
        spec-defined alarm bits).
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.ElectricalProtectionAlarm
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        alarm_bits = cluster.Bitmaps.AlarmBitmap

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        self.step(2, "TH reads from the DUT the FeatureMap attribute. TH records the value as FM.",
                  expectation="Verify that the DUT response contains a map32 value.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.FeatureMap)
        log.info("FeatureMap: 0x%08X", feature_map)

        self.step(3, "TH reads from the DUT the Supported attribute. TH records the value as SUP.",
                  expectation="Verify that the DUT response contains a map32 AlarmBitmap value.")
        supported_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Supported)
        log.info("Supported: 0x%08X", supported_val)

        self.step(4, "TH verifies SHORT (FM bit 20) / ShortCircuitFault (SUP bit 0) consistency: "
                      "if EPALM.S.F20(SHORT) is true, SUP bit 0 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F20(SHORT) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kShortCircuit, alarm_bits.kShortCircuitFault)

        self.step(5, "TH verifies OL (FM bit 21) / OverLoadFault (SUP bit 1) consistency: "
                      "if EPALM.S.F21(OL) is true, SUP bit 1 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F21(OL) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kOverLoad, alarm_bits.kOverLoadFault)

        self.step(6, "TH verifies OV (FM bit 22) / OverVoltageFault (SUP bit 2) consistency: "
                      "if EPALM.S.F22(OV) is true, SUP bit 2 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F22(OV) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kOverVoltage, alarm_bits.kOverVoltageFault)

        self.step(7, "TH verifies SP (FM bit 23) / VoltageSurgeFault (SUP bit 3) consistency: "
                      "if EPALM.S.F23(SP) is true, SUP bit 3 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F23(SP) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kSurgeProtection, alarm_bits.kVoltageSurgeFault)

        self.step(8, "TH verifies RC (FM bit 24) / ResidualCurrentFault (SUP bit 4) consistency: "
                      "if EPALM.S.F24(RC) is true, SUP bit 4 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F24(RC) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kResidualCurrent, alarm_bits.kResidualCurrentFault)

        self.step(9, "TH verifies ARC (FM bit 25) / ArcFault (SUP bit 5) consistency: "
                      "if EPALM.S.F25(ARC) is true, SUP bit 5 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F25(ARC) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kArcFault, alarm_bits.kArcFault)

        self.step(10, "TH verifies SELFTEST (FM bit 26) / SelfTest (SUP bit 6) consistency: "
                      "if EPALM.S.F26(SELFTEST) is true, SUP bit 6 MUST be set.",
                  expectation="Consistency holds. If EPALM.S.F26(SELFTEST) is false, the step is skipped.")
        self._check_feature(feature_map, supported_val, features.kSelfTest, alarm_bits.kSelfTest)

        self.step(11, "TH verifies the reverse direction and orphan-bit absence: (a) no bits MAY be "
                      "set in SUP outside positions 0-6 (the seven spec-defined alarm bits); (b) for "
                      "each bit n in 0..6 that is set in SUP, the corresponding FM feature bit "
                      "(20 + n) MUST also be set.",
                  expectation="Both invariants hold. No orphan SUP bits exist outside 0-6, and every "
                              "set SUP bit has its corresponding feature bit set in FM.")
        all_alarm_bits = (alarm_bits.kShortCircuitFault | alarm_bits.kOverLoadFault |
                          alarm_bits.kOverVoltageFault | alarm_bits.kVoltageSurgeFault |
                          alarm_bits.kResidualCurrentFault | alarm_bits.kArcFault |
                          alarm_bits.kSelfTest)
        # int() is load-bearing: AlarmBitmap is an IntFlag covering exactly bits 0..6, so
        # ~all_alarm_bits would complement within that mask and the assertion would never fail.
        orphan_bits = int(supported_val) & ~int(all_alarm_bits)
        asserts.assert_equal(orphan_bits, 0,
                             f"Supported has bits 0x{orphan_bits:08X} not mapped to any known alarm")

        for feature_bit, alarm_bit in (
                (features.kShortCircuit, alarm_bits.kShortCircuitFault),
                (features.kOverLoad, alarm_bits.kOverLoadFault),
                (features.kOverVoltage, alarm_bits.kOverVoltageFault),
                (features.kSurgeProtection, alarm_bits.kVoltageSurgeFault),
                (features.kResidualCurrent, alarm_bits.kResidualCurrentFault),
                (features.kArcFault, alarm_bits.kArcFault),
                (features.kSelfTest, alarm_bits.kSelfTest)):
            if (supported_val & alarm_bit) and not (feature_map & feature_bit):
                asserts.fail(f"Alarm bit {alarm_bit.name} is set in Supported but feature "
                             f"{feature_bit.name} is NOT set in FeatureMap")

        log.info("FeatureMap/Supported consistency verified - no orphan bits")


if __name__ == "__main__":
    default_matter_test_main()
