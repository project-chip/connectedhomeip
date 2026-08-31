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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

cluster = Clusters.ElectricalAlarm
_F = cluster.Bitmaps.Feature

# AlarmBitmap bit positions (from electrical-alarm-cluster.xml)
_ALARM_OVERVOLTAGE = 0x0001    # bit 0
_ALARM_UNDERVOLTAGE = 0x0002   # bit 1
_ALARM_OVERFREQUENCY = 0x0004  # bit 2
_ALARM_UNDERFREQUENCY = 0x0008  # bit 3
_ALARM_OVERPOWER = 0x0010      # bit 4
_ALARM_UNDERPOWER = 0x0020     # bit 5
_ALARM_OVERCURRENT = 0x0040    # bit 6
_ALARM_UNDERCURRENT = 0x0080   # bit 7
_ALARM_FREQUENCYQUALITY = 0x0100  # bit 8
_ALARM_VOLTAGEQUALITY = 0x0200    # bit 9
_ALARM_SWAPPEDPOLARITY = 0x0400   # bit 10
_ALARM_LOSSOFMEASUREMENT = 0x0800  # bit 11
_ALARM_LOSSOFGRIDPOWER = 0x1000   # bit 12
_ALARM_POWERIMPORTED = 0x2000  # bit 13
_ALARM_POWEREXPORTED = 0x4000  # bit 14

# All bits defined in AlarmBitmap — used to verify no orphan bits in Supported
_ALL_DEFINED_ALARM_BITS = (
    _ALARM_OVERVOLTAGE | _ALARM_UNDERVOLTAGE |
    _ALARM_OVERFREQUENCY | _ALARM_UNDERFREQUENCY |
    _ALARM_OVERPOWER | _ALARM_UNDERPOWER |
    _ALARM_OVERCURRENT | _ALARM_UNDERCURRENT |
    _ALARM_FREQUENCYQUALITY | _ALARM_VOLTAGEQUALITY |
    _ALARM_SWAPPEDPOLARITY | _ALARM_LOSSOFMEASUREMENT |
    _ALARM_LOSSOFGRIDPOWER |
    _ALARM_POWERIMPORTED | _ALARM_POWEREXPORTED
)


class TC_ESALM_2_4(MatterBaseTest):

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_ESALM_2_4(self):
        """[TC-ESALM-2.4] FeatureMap and Supported attribute consistency with Server as DUT

        Verify that each alarm bit in the Supported attribute has a corresponding feature
        bit set in FeatureMap (and vice versa), ensuring the two attributes are consistent
        with the spec-defined feature-to-alarm-bit mapping.
        """
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes

        self.step(1, "Commission DUT to TH", is_commissioning=True)

        self.step(2, "TH reads FeatureMap attribute", expectation="DUT returns map32. Store as FM.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.FeatureMap)

        self.step(3, "TH reads Supported attribute", expectation="DUT returns AlarmBitmap. Store as SUP.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)

        self.step(4, "Verify OVERVOLT / OverVoltage (SUP bit 0) consistency",
                  expectation="If OVERVOLT set in FM, SUP bit 0 must be set.")
        if feature_map & _F.kOverVoltage:
            asserts.assert_true(supported & _ALARM_OVERVOLTAGE,
                                "OVERVOLT feature set but OverVoltage (bit 0) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(5, "Verify UNDERVOLT / UnderVoltage (SUP bit 1) consistency",
                  expectation="If UNDERVOLT set in FM, SUP bit 1 must be set.")
        if feature_map & _F.kUnderVoltage:
            asserts.assert_true(supported & _ALARM_UNDERVOLTAGE,
                                "UNDERVOLT feature set but UnderVoltage (bit 1) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(6, "Verify OVERFREQ / OverFrequency (SUP bit 2) consistency",
                  expectation="If OVERFREQ set in FM, SUP bit 2 must be set.")
        if feature_map & _F.kOverFrequency:
            asserts.assert_true(supported & _ALARM_OVERFREQUENCY,
                                "OVERFREQ feature set but OverFrequency (bit 2) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(7, "Verify UNDERFREQ / UnderFrequency (SUP bit 3) consistency",
                  expectation="If UNDERFREQ set in FM, SUP bit 3 must be set.")
        if feature_map & _F.kUnderFrequency:
            asserts.assert_true(supported & _ALARM_UNDERFREQUENCY,
                                "UNDERFREQ feature set but UnderFrequency (bit 3) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(8, "Verify OVERPOWER / OverPower (SUP bit 4) consistency",
                  expectation="If OVERPOWER set in FM, SUP bit 4 must be set.")
        if feature_map & _F.kOverPower:
            asserts.assert_true(supported & _ALARM_OVERPOWER,
                                "OVERPOWER feature set but OverPower (bit 4) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(9, "Verify UNDERPOWER / UnderPower (SUP bit 5) consistency",
                  expectation="If UNDERPOWER set in FM, SUP bit 5 must be set.")
        if feature_map & _F.kUnderPower:
            asserts.assert_true(supported & _ALARM_UNDERPOWER,
                                "UNDERPOWER feature set but UnderPower (bit 5) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(10, "Verify OVERCUR / OverCurrent (SUP bit 6) consistency",
                  expectation="If OVERCUR set in FM, SUP bit 6 must be set.")
        if feature_map & _F.kOverCurrent:
            asserts.assert_true(supported & _ALARM_OVERCURRENT,
                                "OVERCUR feature set but OverCurrent (bit 6) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(11, "Verify UNDERCUR / UnderCurrent (SUP bit 7) consistency",
                  expectation="If UNDERCUR set in FM, SUP bit 7 must be set.")
        if feature_map & _F.kUnderCurrent:
            asserts.assert_true(supported & _ALARM_UNDERCURRENT,
                                "UNDERCUR feature set but UnderCurrent (bit 7) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(12, "Verify POWERIMP / PowerImported (SUP bit 13) consistency",
                  expectation="If POWERIMP set in FM, SUP bit 13 must be set.")
        if feature_map & _F.kPowerImport:
            asserts.assert_true(supported & _ALARM_POWERIMPORTED,
                                "POWERIMP feature set but PowerImported (bit 13) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(13, "Verify POWEREXP / PowerExported (SUP bit 14) consistency",
                  expectation="If POWEREXP set in FM, SUP bit 14 must be set.")
        if feature_map & _F.kPowerExport:
            asserts.assert_true(supported & _ALARM_POWEREXPORTED,
                                "POWEREXP feature set but PowerExported (bit 14) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(14, "Verify reverse direction: every SUP bit has a corresponding FM feature bit; no orphan SUP bits outside defined positions",
                  expectation="Both invariants hold.")
        asserts.assert_equal(int(supported) & ~int(_ALL_DEFINED_ALARM_BITS), 0,
                             "Supported contains bits outside the spec-defined alarm bit positions (0-14)")

        feature_alarm_map = [
            (_ALARM_OVERVOLTAGE, _F.kOverVoltage, "OverVoltage (bit 0)"),
            (_ALARM_UNDERVOLTAGE, _F.kUnderVoltage, "UnderVoltage (bit 1)"),
            (_ALARM_OVERFREQUENCY, _F.kOverFrequency, "OverFrequency (bit 2)"),
            (_ALARM_UNDERFREQUENCY, _F.kUnderFrequency, "UnderFrequency (bit 3)"),
            (_ALARM_OVERPOWER, _F.kOverPower, "OverPower (bit 4)"),
            (_ALARM_UNDERPOWER, _F.kUnderPower, "UnderPower (bit 5)"),
            (_ALARM_OVERCURRENT, _F.kOverCurrent, "OverCurrent (bit 6)"),
            (_ALARM_UNDERCURRENT, _F.kUnderCurrent, "UnderCurrent (bit 7)"),
            (_ALARM_POWERIMPORTED, _F.kPowerImport, "PowerImported (bit 13)"),
            (_ALARM_POWEREXPORTED, _F.kPowerExport, "PowerExported (bit 14)"),
        ]
        for alarm_bit, feat_bit, name in feature_alarm_map:
            if int(supported) & alarm_bit:
                asserts.assert_true(int(feature_map) & int(feat_bit),
                                    f"{name} set in Supported but corresponding feature bit not set in FeatureMap")


if __name__ == "__main__":
    default_matter_test_main()
