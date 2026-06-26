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

# Feature bitmap bit positions (from electrical-alarm-cluster.xml)
_FEAT_OVERVOLT = 1 << 21
_FEAT_UNDERVOLT = 1 << 22
_FEAT_OVERFREQ = 1 << 23
_FEAT_UNDERFREQ = 1 << 24
_FEAT_OVERPOWER = 1 << 25
_FEAT_UNDERPOWER = 1 << 26
_FEAT_OVERCUR = 1 << 27
_FEAT_UNDERCUR = 1 << 28
_FEAT_POWERIMP = 1 << 29
_FEAT_POWEREXP = 1 << 30

# Corresponding AlarmBitmap bit positions (from electrical-alarm-cluster.xml)
_ALARM_OVERVOLTAGE = 0x0001    # bit 0
_ALARM_UNDERVOLTAGE = 0x0002   # bit 1
_ALARM_OVERFREQUENCY = 0x0004  # bit 2
_ALARM_UNDERFREQUENCY = 0x0008  # bit 3
_ALARM_OVERPOWER = 0x0010      # bit 4
_ALARM_UNDERPOWER = 0x0020     # bit 5
_ALARM_OVERCURRENT = 0x0040    # bit 6
_ALARM_UNDERCURRENT = 0x0080   # bit 7
_ALARM_POWERIMPORTED = 0x2000  # bit 13
_ALARM_POWEREXPORTED = 0x4000  # bit 14

# Mask of all spec-defined AlarmBitmap positions that correspond to features
_DEFINED_FEATURE_ALARM_BITS = (
    _ALARM_OVERVOLTAGE | _ALARM_UNDERVOLTAGE |
    _ALARM_OVERFREQUENCY | _ALARM_UNDERFREQUENCY |
    _ALARM_OVERPOWER | _ALARM_UNDERPOWER |
    _ALARM_OVERCURRENT | _ALARM_UNDERCURRENT |
    _ALARM_POWERIMPORTED | _ALARM_POWEREXPORTED
)

cluster = Clusters.ElectricalAlarm


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

        self.step(2, "TH reads FeatureMap attribute", "DUT returns map32. Store as FM.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.FeatureMap)

        self.step(3, "TH reads Supported attribute", "DUT returns map64 AlarmBitmap. Store as SUP.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)

        self.step(4, "Verify OVERVOLT (FM bit 21) / OverVoltage (SUP bit 0) consistency",
                  "If OVERVOLT set in FM, SUP bit 0 must be set.")
        if feature_map & _FEAT_OVERVOLT:
            asserts.assert_true(supported & _ALARM_OVERVOLTAGE,
                                "OVERVOLT feature set but OverVoltage (bit 0) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(5, "Verify UNDERVOLT (FM bit 22) / UnderVoltage (SUP bit 1) consistency",
                  "If UNDERVOLT set in FM, SUP bit 1 must be set.")
        if feature_map & _FEAT_UNDERVOLT:
            asserts.assert_true(supported & _ALARM_UNDERVOLTAGE,
                                "UNDERVOLT feature set but UnderVoltage (bit 1) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(6, "Verify OVERFREQ (FM bit 23) / OverFrequency (SUP bit 2) consistency",
                  "If OVERFREQ set in FM, SUP bit 2 must be set.")
        if feature_map & _FEAT_OVERFREQ:
            asserts.assert_true(supported & _ALARM_OVERFREQUENCY,
                                "OVERFREQ feature set but OverFrequency (bit 2) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(7, "Verify UNDERFREQ (FM bit 24) / UnderFrequency (SUP bit 3) consistency",
                  "If UNDERFREQ set in FM, SUP bit 3 must be set.")
        if feature_map & _FEAT_UNDERFREQ:
            asserts.assert_true(supported & _ALARM_UNDERFREQUENCY,
                                "UNDERFREQ feature set but UnderFrequency (bit 3) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(8, "Verify OVERPOWER (FM bit 25) / OverPower (SUP bit 4) consistency",
                  "If OVERPOWER set in FM, SUP bit 4 must be set.")
        if feature_map & _FEAT_OVERPOWER:
            asserts.assert_true(supported & _ALARM_OVERPOWER,
                                "OVERPOWER feature set but OverPower (bit 4) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(9, "Verify UNDERPOWER (FM bit 26) / UnderPower (SUP bit 5) consistency",
                  "If UNDERPOWER set in FM, SUP bit 5 must be set.")
        if feature_map & _FEAT_UNDERPOWER:
            asserts.assert_true(supported & _ALARM_UNDERPOWER,
                                "UNDERPOWER feature set but UnderPower (bit 5) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(10, "Verify OVERCUR (FM bit 27) / OverCurrent (SUP bit 6) consistency",
                  "If OVERCUR set in FM, SUP bit 6 must be set.")
        if feature_map & _FEAT_OVERCUR:
            asserts.assert_true(supported & _ALARM_OVERCURRENT,
                                "OVERCUR feature set but OverCurrent (bit 6) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(11, "Verify UNDERCUR (FM bit 28) / UnderCurrent (SUP bit 7) consistency",
                  "If UNDERCUR set in FM, SUP bit 7 must be set.")
        if feature_map & _FEAT_UNDERCUR:
            asserts.assert_true(supported & _ALARM_UNDERCURRENT,
                                "UNDERCUR feature set but UnderCurrent (bit 7) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(12, "Verify POWERIMP (FM bit 29) / PowerImported (SUP bit 13) consistency",
                  "If POWERIMP set in FM, SUP bit 13 must be set.")
        if feature_map & _FEAT_POWERIMP:
            asserts.assert_true(supported & _ALARM_POWERIMPORTED,
                                "POWERIMP feature set but PowerImported (bit 13) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(13, "Verify POWEREXP (FM bit 30) / PowerExported (SUP bit 14) consistency",
                  "If POWEREXP set in FM, SUP bit 14 must be set.")
        if feature_map & _FEAT_POWEREXP:
            asserts.assert_true(supported & _ALARM_POWEREXPORTED,
                                "POWEREXP feature set but PowerExported (bit 14) not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(14, "Verify reverse direction: every SUP bit has a corresponding FM feature bit; no orphan SUP bits outside defined positions",
                  "Both invariants hold.")
        asserts.assert_equal(supported & ~_DEFINED_FEATURE_ALARM_BITS, 0,
                             "Supported contains bits outside the spec-defined alarm bit positions (0-7, 13-14)")

        feature_alarm_map = [
            (_ALARM_OVERVOLTAGE, _FEAT_OVERVOLT, "OverVoltage (bit 0)"),
            (_ALARM_UNDERVOLTAGE, _FEAT_UNDERVOLT, "UnderVoltage (bit 1)"),
            (_ALARM_OVERFREQUENCY, _FEAT_OVERFREQ, "OverFrequency (bit 2)"),
            (_ALARM_UNDERFREQUENCY, _FEAT_UNDERFREQ, "UnderFrequency (bit 3)"),
            (_ALARM_OVERPOWER, _FEAT_OVERPOWER, "OverPower (bit 4)"),
            (_ALARM_UNDERPOWER, _FEAT_UNDERPOWER, "UnderPower (bit 5)"),
            (_ALARM_OVERCURRENT, _FEAT_OVERCUR, "OverCurrent (bit 6)"),
            (_ALARM_UNDERCURRENT, _FEAT_UNDERCUR, "UnderCurrent (bit 7)"),
            (_ALARM_POWERIMPORTED, _FEAT_POWERIMP, "PowerImported (bit 13)"),
            (_ALARM_POWEREXPORTED, _FEAT_POWEREXP, "PowerExported (bit 14)"),
        ]
        for alarm_bit, feat_bit, name in feature_alarm_map:
            if supported & alarm_bit:
                asserts.assert_true(feature_map & feat_bit,
                                    "%s set in Supported but corresponding feature bit not set in FeatureMap" % name)


if __name__ == "__main__":
    default_matter_test_main()
