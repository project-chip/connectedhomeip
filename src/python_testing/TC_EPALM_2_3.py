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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_EPALM_2_3(MatterBaseTest):
    """TC-EPALM-2.3: FeatureMap and Supported Attribute Consistency with DUT as Server

    Verify that the bits set in the Supported attribute are consistent with the
    alarm-condition feature bits declared in the FeatureMap.
    """

    def desc_TC_EPALM_2_3(self) -> str:
        return "[TC-EPALM-2.3] FeatureMap and Supported Attribute Consistency with DUT as Server"

    def pics_TC_EPALM_2_3(self) -> list[str]:
        return ["EPALM.S"]

    def steps_TC_EPALM_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads the FeatureMap attribute from DUT"),
            TestStep(3, "TH reads the Supported attribute from DUT"),
            TestStep(4, "TH verifies ShortCircuit feature/supported consistency"),
            TestStep(5, "TH verifies OverLoad feature/supported consistency"),
            TestStep(6, "TH verifies OverVoltage feature/supported consistency"),
            TestStep(7, "TH verifies SurgeProtection feature/supported consistency"),
            TestStep(8, "TH verifies ResidualCurrent feature/supported consistency"),
            TestStep(9, "TH verifies ArcFault feature/supported consistency"),
            TestStep(10, "TH verifies SelfTest feature/supported consistency"),
            TestStep(11, "TH verifies no orphan bits in Supported"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalProtectionAlarm))
    async def test_TC_EPALM_2_3(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ElectricalProtectionAlarm
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        alarm_bits = cluster.Bitmaps.AlarmBitmap

        self.step(1)

        # Step 2: Read FeatureMap
        self.step(2)
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.FeatureMap
        )
        log.info(f"FeatureMap: 0x{feature_map:08X}")

        # Step 3: Read Supported
        self.step(3)
        supported_val = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.Supported
        )
        log.info(f"Supported: 0x{supported_val:08X}")

        # Feature-to-AlarmBitmap mapping
        feature_alarm_map = {
            features.kShortCircuit: alarm_bits.kShortCircuitFault,
            features.kOverLoad: alarm_bits.kOverLoadFault,
            features.kOverVoltage: alarm_bits.kOverVoltageFault,
            features.kSurgeProtection: alarm_bits.kVoltageSurgeFault,
            features.kResidualCurrent: alarm_bits.kResidualCurrentFault,
            features.kArcFault: alarm_bits.kArcFault,
            features.kSelfTest: alarm_bits.kSelfTest,
        }

        step_num = 4
        expected_supported = 0

        for feature_bit, alarm_bit in feature_alarm_map.items():
            self.step(step_num)
            if feature_map & feature_bit:
                asserts.assert_true(
                    bool(supported_val & alarm_bit),
                    f"Feature {feature_bit.name} is set in FeatureMap but "
                    f"corresponding alarm bit {alarm_bit.name} is NOT set in Supported"
                )
                expected_supported |= alarm_bit
                log.info(f"Feature {feature_bit.name}: present, alarm bit {alarm_bit.name} verified in Supported")
            else:
                log.info(f"Feature {feature_bit.name}: not present, skipping")
            step_num += 1

        # Step 11: Verify no orphan supported bits
        self.step(11)
        all_alarm_bits = (alarm_bits.kShortCircuitFault | alarm_bits.kOverLoadFault |
                          alarm_bits.kOverVoltageFault | alarm_bits.kVoltageSurgeFault |
                          alarm_bits.kResidualCurrentFault | alarm_bits.kArcFault |
                          alarm_bits.kSelfTest)
        orphan_bits = supported_val & ~all_alarm_bits
        asserts.assert_equal(orphan_bits, 0,
                             f"Supported has bits 0x{orphan_bits:08X} not mapped to any known alarm")

        # Also verify no alarm bit in Supported without its feature
        for feature_bit, alarm_bit in feature_alarm_map.items():
            if (supported_val & alarm_bit) and not (feature_map & feature_bit):
                asserts.fail(
                    f"Alarm bit {alarm_bit.name} is set in Supported but "
                    f"feature {feature_bit.name} is NOT set in FeatureMap"
                )

        log.info("FeatureMap/Supported consistency verified - no orphan bits")


if __name__ == "__main__":
    default_matter_test_main()
