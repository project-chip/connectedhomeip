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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, pics, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_EPALM_2_3(MatterBaseTest):

    @property
    def default_endpoint(self) -> int:
        return 1

    @pics('EPALM.S')
    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalProtectionAlarm))
    async def test_TC_EPALM_2_3(self):
        """[TC-EPALM-2.3] FeatureMap and Supported Attribute Consistency with DUT as Server

        Verify bidirectional consistency between the EPALM FeatureMap (bits 20-26
        for SHORT / OL / OV / SP / RC / ARC / SELFTEST) and the inherited Alarm
        Base Supported attribute (AlarmBitmap bits 0-6): every feature bit set
        in FeatureMap MUST have its corresponding bit set in Supported, and
        every bit set in Supported MUST correspond to a feature bit set in
        FeatureMap (no orphan bits).
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.ElectricalProtectionAlarm
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        alarm_bits = cluster.Bitmaps.AlarmBitmap

        self.step(1, "Commissioning, already done", is_commissioning=True)

        self.step(2, "TH reads the FeatureMap attribute from DUT")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.FeatureMap
        )
        log.info(f"FeatureMap: 0x{feature_map:08X}")

        self.step(3, "TH reads the Supported attribute from DUT")
        supported_val = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.Supported
        )
        log.info(f"Supported: 0x{supported_val:08X}")

        # Feature-to-AlarmBitmap mapping
        feature_alarm_map = [
            (features.kShortCircuit, alarm_bits.kShortCircuitFault, "ShortCircuit"),
            (features.kOverLoad, alarm_bits.kOverLoadFault, "OverLoad"),
            (features.kOverVoltage, alarm_bits.kOverVoltageFault, "OverVoltage"),
            (features.kSurgeProtection, alarm_bits.kVoltageSurgeFault, "SurgeProtection"),
            (features.kResidualCurrent, alarm_bits.kResidualCurrentFault, "ResidualCurrent"),
            (features.kArcFault, alarm_bits.kArcFault, "ArcFault"),
            (features.kSelfTest, alarm_bits.kSelfTest, "SelfTest"),
        ]

        step_num = 4
        for feature_bit, alarm_bit, feature_name in feature_alarm_map:
            self.step(step_num, f"TH verifies {feature_name} feature/supported consistency")
            if feature_map & feature_bit:
                asserts.assert_true(
                    bool(supported_val & alarm_bit),
                    f"Feature {feature_bit.name} is set in FeatureMap but "
                    f"corresponding alarm bit {alarm_bit.name} is NOT set in Supported"
                )
                log.info(f"Feature {feature_bit.name}: present, alarm bit {alarm_bit.name} verified in Supported")
            else:
                log.info(f"Feature {feature_bit.name}: not present, skipping")
            step_num += 1

        self.step(11, "TH verifies no orphan bits in Supported and no Supported bit without its feature")
        all_alarm_bits = (alarm_bits.kShortCircuitFault | alarm_bits.kOverLoadFault |
                          alarm_bits.kOverVoltageFault | alarm_bits.kVoltageSurgeFault |
                          alarm_bits.kResidualCurrentFault | alarm_bits.kArcFault |
                          alarm_bits.kSelfTest)
        orphan_bits = supported_val & ~all_alarm_bits
        asserts.assert_equal(orphan_bits, 0,
                             f"Supported has bits 0x{orphan_bits:08X} not mapped to any known alarm")

        for feature_bit, alarm_bit, _ in feature_alarm_map:
            if (supported_val & alarm_bit) and not (feature_map & feature_bit):
                asserts.fail(
                    f"Alarm bit {alarm_bit.name} is set in Supported but "
                    f"feature {feature_bit.name} is NOT set in FeatureMap"
                )

        log.info("FeatureMap/Supported consistency verified - no orphan bits")


if __name__ == "__main__":
    default_matter_test_main()
