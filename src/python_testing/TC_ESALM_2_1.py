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
#     app: ${ELECTRICAL_PROTECTION_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 2
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

cluster = Clusters.ElectricalAlarm
_F = cluster.Bitmaps.Feature
_A = cluster.Bitmaps.AlarmBitmap


class TC_ESALM_2_1(MatterBaseTest):

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_ESALM_2_1(self):
        """[TC-ESALM-2.1] Attributes with Server as DUT

        Verify that all mandatory and optional attributes of the Electrical Alarm cluster
        have valid values and satisfy the constraints defined in the spec.
        """
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes

        self.step(1, "Commission DUT to TH", is_commissioning=True)

        self.step(2, "TH reads from the DUT the FeatureMap attribute.",
                  expectation="Verify that the DUT response contains a uint32 value. Store the value as FeatureMap.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.FeatureMap)
        has_overvolt = bool(feature_map & _F.kOverVoltage)
        has_undervolt = bool(feature_map & _F.kUnderVoltage)
        has_overfreq = bool(feature_map & _F.kOverFrequency)
        has_underfreq = bool(feature_map & _F.kUnderFrequency)
        has_overpower = bool(feature_map & _F.kOverPower)
        has_underpower = bool(feature_map & _F.kUnderPower)
        has_overcur = bool(feature_map & _F.kOverCurrent)
        has_undercur = bool(feature_map & _F.kUnderCurrent)
        has_powerimp = bool(feature_map & _F.kPowerImport)
        has_powerexp = bool(feature_map & _F.kPowerExport)

        # The Latch attribute is optional (PICS_S.A0001), so step 5 is gated on its presence in
        # the AttributeList rather than on any feature bit.
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        has_latch = attrs.Latch.attribute_id in attribute_list

        self.step(3, "TH reads from the DUT the Supported attribute.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value. Store the "
                              "value as Supported. Verify bidirectional consistency with FeatureMap: every "
                              "bit set in Supported has a corresponding feature bit set in FeatureMap, and "
                              "every feature bit set in FeatureMap has its corresponding bit set in Supported. "
                              "Mapping: OverVoltage (bit 0) ↔ OVERVOLT, UnderVoltage (bit 1) ↔ UNDERVOLT, "
                              "OverFrequency (bit 2) ↔ OVERFREQ, UnderFrequency (bit 3) ↔ UNDERFREQ, OverPower "
                              "(bit 4) ↔ OVERPOWER, UnderPower (bit 5) ↔ UNDERPOWER, OverCurrent (bit 6) ↔ "
                              "OVERCUR, UnderCurrent (bit 7) ↔ UNDERCUR, PowerImported (bit 13) ↔ POWERIMP, "
                              "PowerExported (bit 14) ↔ POWEREXP.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        matter_asserts.assert_valid_uint32(supported, "Supported (map32 AlarmBitmap)")
        if has_overvolt:
            asserts.assert_true(supported & _A.kOverVoltage, "OVERVOLT feature set but OverVoltage bit missing from Supported")
        if has_undervolt:
            asserts.assert_true(supported & _A.kUnderVoltage, "UNDERVOLT feature set but UnderVoltage bit missing from Supported")
        if has_overfreq:
            asserts.assert_true(supported & _A.kOverFrequency, "OVERFREQ feature set but OverFrequency bit missing from Supported")
        if has_underfreq:
            asserts.assert_true(supported & _A.kUnderFrequency,
                                "UNDERFREQ feature set but UnderFrequency bit missing from Supported")
        if has_overpower:
            asserts.assert_true(supported & _A.kOverPower, "OVERPOWER feature set but OverPower bit missing from Supported")
        if has_underpower:
            asserts.assert_true(supported & _A.kUnderPower, "UNDERPOWER feature set but UnderPower bit missing from Supported")
        if has_overcur:
            asserts.assert_true(supported & _A.kOverCurrent, "OVERCUR feature set but OverCurrent bit missing from Supported")
        if has_undercur:
            asserts.assert_true(supported & _A.kUnderCurrent, "UNDERCUR feature set but UnderCurrent bit missing from Supported")
        if has_powerimp:
            asserts.assert_true(supported & _A.kPowerImported, "POWERIMP feature set but PowerImported bit missing from Supported")
        if has_powerexp:
            asserts.assert_true(supported & _A.kPowerExported, "POWEREXP feature set but PowerExported bit missing from Supported")
        # Reverse direction: every bit set in Supported must map to a set feature bit, and there
        # must be no bits outside the defined alarm positions. Together with the checks above this
        # verifies the bidirectional FeatureMap<->Supported consistency the test plan calls for.
        if supported & _A.kOverVoltage:
            asserts.assert_true(has_overvolt, "OverVoltage bit set in Supported but OVERVOLT feature absent from FeatureMap")
        if supported & _A.kUnderVoltage:
            asserts.assert_true(
                has_undervolt, "UnderVoltage bit set in Supported but UNDERVOLT feature absent from FeatureMap")
        if supported & _A.kOverFrequency:
            asserts.assert_true(has_overfreq, "OverFrequency bit set in Supported but OVERFREQ feature absent from FeatureMap")
        if supported & _A.kUnderFrequency:
            asserts.assert_true(
                has_underfreq, "UnderFrequency bit set in Supported but UNDERFREQ feature absent from FeatureMap")
        if supported & _A.kOverPower:
            asserts.assert_true(has_overpower, "OverPower bit set in Supported but OVERPOWER feature absent from FeatureMap")
        if supported & _A.kUnderPower:
            asserts.assert_true(
                has_underpower, "UnderPower bit set in Supported but UNDERPOWER feature absent from FeatureMap")
        if supported & _A.kOverCurrent:
            asserts.assert_true(has_overcur, "OverCurrent bit set in Supported but OVERCUR feature absent from FeatureMap")
        if supported & _A.kUnderCurrent:
            asserts.assert_true(has_undercur, "UnderCurrent bit set in Supported but UNDERCUR feature absent from FeatureMap")
        if supported & _A.kPowerImported:
            asserts.assert_true(
                has_powerimp, "PowerImported bit set in Supported but POWERIMP feature absent from FeatureMap")
        if supported & _A.kPowerExported:
            asserts.assert_true(
                has_powerexp, "PowerExported bit set in Supported but POWEREXP feature absent from FeatureMap")
        # Every AlarmBitmap bit is P, O.b+ with no feature conformance, so a DUT may declare any
        # of them, including the five without a feature (FrequencyQuality, VoltageQuality,
        # SwappedPolarity, LossOfMeasurement, LossOfGridPower). Any bit outside the defined
        # AlarmBitmap is reserved. The mask is derived from the cluster enum rather than hardcoded.
        defined_supported_bits = 0
        for _alarm_bit in _A:
            defined_supported_bits |= _alarm_bit
        asserts.assert_equal(int(supported) & ~int(defined_supported_bits), 0,
                             "Supported has reserved bits set outside the defined ESALM AlarmBitmap")

        self.step(4, "TH reads from the DUT the Mask attribute.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value, where any "
                              "bit set in Mask is also set in Supported.")
        mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        matter_asserts.assert_valid_uint32(mask, "Mask (map32 AlarmBitmap)")
        asserts.assert_equal(int(mask) & ~int(supported), 0, "Mask contains bits not set in Supported")

        self.step(5, "TH reads from the DUT the Latch attribute.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value, where any "
                              "bit set in Latch is also set in Supported.")
        if has_latch:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)
            matter_asserts.assert_valid_uint32(latch, "Latch (map32 AlarmBitmap)")
            asserts.assert_equal(int(latch) & ~int(supported), 0, "Latch contains bits not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(6, "TH reads from the DUT the State attribute.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value, where any "
                              "bit set in State is also set in Supported.")
        state = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        matter_asserts.assert_valid_uint32(state, "State (map32 AlarmBitmap)")
        asserts.assert_equal(int(state) & ~int(supported), 0, "State contains bits not set in Supported")

        over_voltage = None
        under_voltage = None
        over_frequency = None
        under_frequency = None
        over_power = None
        under_power = None
        over_current = None
        under_current = None

        self.step(7, "TH reads from the DUT the OverVoltageThreshold attribute.",
                  expectation="Verify that the DUT response contains a voltage-mV (int64) value.")
        if has_overvolt:
            over_voltage = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverVoltageThreshold)
            matter_asserts.assert_valid_int64(over_voltage, "OverVoltageThreshold")
        else:
            self.mark_current_step_skipped()

        self.step(8, "TH reads from the DUT the UnderVoltageThreshold attribute.",
                  expectation="Verify that the DUT response contains a voltage-mV (int64) value. If OVERVOLT "
                              "is also supported, value is less than or equal to (OverVoltageThreshold - 1).")
        if has_undervolt:
            under_voltage = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderVoltageThreshold)
            matter_asserts.assert_valid_int64(under_voltage, "UnderVoltageThreshold")
        else:
            self.mark_current_step_skipped()
        if has_overvolt and has_undervolt:
            asserts.assert_less_equal(under_voltage, over_voltage - 1,
                                      "UnderVoltageThreshold must be <= OverVoltageThreshold - 1")

        self.step(9, "TH reads from the DUT the OverFrequencyThreshold attribute.",
                  expectation="Verify that the DUT response contains an int64 value in millihertz.")
        if has_overfreq:
            over_frequency = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverFrequencyThreshold)
            matter_asserts.assert_valid_int64(over_frequency, "OverFrequencyThreshold")
        else:
            self.mark_current_step_skipped()

        self.step(10, "TH reads from the DUT the UnderFrequencyThreshold attribute.",
                  expectation="Verify that the DUT response contains an int64 value in millihertz. If OVERFREQ is also supported, "
                              "value is less than or equal to (OverFrequencyThreshold - 1).")
        if has_underfreq:
            under_frequency = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderFrequencyThreshold)
            matter_asserts.assert_valid_int64(under_frequency, "UnderFrequencyThreshold")
        else:
            self.mark_current_step_skipped()
        if has_overfreq and has_underfreq:
            asserts.assert_less_equal(under_frequency, over_frequency - 1,
                                      "UnderFrequencyThreshold must be <= OverFrequencyThreshold - 1")

        self.step(11, "TH reads from the DUT the OverPowerThreshold attribute.",
                  expectation="Verify that the DUT response contains a power-mW (int64) value.")
        if has_overpower:
            over_power = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverPowerThreshold)
            matter_asserts.assert_valid_int64(over_power, "OverPowerThreshold")
        else:
            self.mark_current_step_skipped()

        self.step(12, "TH reads from the DUT the UnderPowerThreshold attribute.",
                  expectation="Verify that the DUT response contains a power-mW (int64) value. If OVERPOWER "
                              "is also supported, value is less than or equal to (OverPowerThreshold - 1).")
        if has_underpower:
            under_power = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderPowerThreshold)
            matter_asserts.assert_valid_int64(under_power, "UnderPowerThreshold")
        else:
            self.mark_current_step_skipped()
        if has_overpower and has_underpower:
            asserts.assert_less_equal(under_power, over_power - 1,
                                      "UnderPowerThreshold must be <= OverPowerThreshold - 1")

        self.step(13, "TH reads from the DUT the OverCurrentThreshold attribute.",
                  expectation="Verify that the DUT response contains an amperage-mA (int64) value.")
        if has_overcur:
            over_current = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverCurrentThreshold)
            matter_asserts.assert_valid_int64(over_current, "OverCurrentThreshold")
        else:
            self.mark_current_step_skipped()

        self.step(14, "TH reads from the DUT the UnderCurrentThreshold attribute.",
                  expectation="Verify that the DUT response contains an amperage-mA (int64) value. If OVERCUR is also supported, "
                              "value is less than or equal to (OverCurrentThreshold - 1).")
        if has_undercur:
            under_current = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderCurrentThreshold)
            matter_asserts.assert_valid_int64(under_current, "UnderCurrentThreshold")
        else:
            self.mark_current_step_skipped()
        if has_overcur and has_undercur:
            asserts.assert_less_equal(under_current, over_current - 1,
                                      "UnderCurrentThreshold must be <= OverCurrentThreshold - 1")

        self.step(15, "TH reads from the DUT the PowerImportThreshold attribute.",
                  expectation="Verify that the DUT response contains a power-mW (int64) value. Value is greater "
                              "than or equal to 0.")
        if has_powerimp:
            power_import = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerImportThreshold)
            matter_asserts.assert_valid_int64(power_import, "PowerImportThreshold")
            asserts.assert_greater_equal(power_import, 0, "PowerImportThreshold must be >= 0")
        else:
            self.mark_current_step_skipped()

        self.step(16, "TH reads from the DUT the PowerExportThreshold attribute.",
                  expectation="Verify that the DUT response contains a power-mW (int64) value. Value is less than or equal to 0. "
                              "If POWERIMP is also supported, value is less than or equal to "
                              "(PowerImportThreshold - 1).")
        if has_powerexp:
            power_export = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerExportThreshold)
            matter_asserts.assert_valid_int64(power_export, "PowerExportThreshold")
            asserts.assert_less_equal(power_export, 0, "PowerExportThreshold must be <= 0")
        else:
            self.mark_current_step_skipped()

        # PowerExportThreshold is constrained to max minOf(0, PowerImportThreshold - 1), so export
        # must stay below import, matching the retained step 16 wording.
        if has_powerimp and has_powerexp:
            asserts.assert_less_equal(power_export, power_import - 1,
                                      "PowerExportThreshold must be <= PowerImportThreshold - 1")


if __name__ == "__main__":
    default_matter_test_main()
