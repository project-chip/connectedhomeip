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
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

# Feature bitmap bit positions (from electrical-alarm-cluster.xml)
_FEAT_RESET = 1 << 0
_FEAT_ADJUST = 1 << 20
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

cluster = Clusters.ElectricalAlarm


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

        self.step(2, "TH reads FeatureMap attribute", "DUT returns uint32. Store as FeatureMap.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.FeatureMap)
        has_reset = bool(feature_map & _FEAT_RESET)
        has_overvolt = bool(feature_map & _FEAT_OVERVOLT)
        has_undervolt = bool(feature_map & _FEAT_UNDERVOLT)
        has_overfreq = bool(feature_map & _FEAT_OVERFREQ)
        has_underfreq = bool(feature_map & _FEAT_UNDERFREQ)
        has_overpower = bool(feature_map & _FEAT_OVERPOWER)
        has_underpower = bool(feature_map & _FEAT_UNDERPOWER)
        has_overcur = bool(feature_map & _FEAT_OVERCUR)
        has_undercur = bool(feature_map & _FEAT_UNDERCUR)
        has_powerimp = bool(feature_map & _FEAT_POWERIMP)
        has_powerexp = bool(feature_map & _FEAT_POWEREXP)

        self.step(3, "TH reads Supported attribute",
                  "DUT returns AlarmBitmap. Every bit set in Supported has a corresponding feature bit in FeatureMap.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        asserts.assert_true(isinstance(supported, int), "Supported must be an integer bitmap")
        if has_overvolt:
            asserts.assert_true(supported & 0x1, "OVERVOLT feature set but OverVoltage bit (0x1) missing from Supported")
        if has_undervolt:
            asserts.assert_true(supported & 0x2, "UNDERVOLT feature set but UnderVoltage bit (0x2) missing from Supported")
        if has_overfreq:
            asserts.assert_true(supported & 0x4, "OVERFREQ feature set but OverFrequency bit (0x4) missing from Supported")
        if has_underfreq:
            asserts.assert_true(supported & 0x8, "UNDERFREQ feature set but UnderFrequency bit (0x8) missing from Supported")
        if has_overpower:
            asserts.assert_true(supported & 0x10, "OVERPOWER feature set but OverPower bit (0x10) missing from Supported")
        if has_underpower:
            asserts.assert_true(supported & 0x20, "UNDERPOWER feature set but UnderPower bit (0x20) missing from Supported")
        if has_overcur:
            asserts.assert_true(supported & 0x40, "OVERCUR feature set but OverCurrent bit (0x40) missing from Supported")
        if has_undercur:
            asserts.assert_true(supported & 0x80, "UNDERCUR feature set but UnderCurrent bit (0x80) missing from Supported")
        if has_powerimp:
            asserts.assert_true(supported & 0x2000, "POWERIMP feature set but PowerImported bit (0x2000) missing from Supported")
        if has_powerexp:
            asserts.assert_true(supported & 0x4000, "POWEREXP feature set but PowerExported bit (0x4000) missing from Supported")

        self.step(4, "TH reads Mask attribute", "DUT returns AlarmBitmap. Every bit set in Mask is also set in Supported.")
        mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        asserts.assert_true(isinstance(mask, int), "Mask must be an integer bitmap")
        asserts.assert_equal(mask & ~supported, 0, "Mask contains bits not set in Supported")

        self.step(5, "TH reads Latch attribute (if RESET supported)",
                  "DUT returns AlarmBitmap. Every bit set in Latch is also set in Supported.")
        if has_reset:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)
            asserts.assert_true(isinstance(latch, int), "Latch must be an integer bitmap")
            asserts.assert_equal(latch & ~supported, 0, "Latch contains bits not set in Supported")
        else:
            self.mark_current_step_skipped()

        self.step(6, "TH reads State attribute", "DUT returns AlarmBitmap. Every bit set in State is also set in Supported.")
        state = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        asserts.assert_true(isinstance(state, int), "State must be an integer bitmap")
        asserts.assert_equal(state & ~supported, 0, "State contains bits not set in Supported")

        over_voltage = None
        under_voltage = None
        over_frequency = None
        under_frequency = None
        over_power = None
        under_power = None
        over_current = None
        under_current = None

        self.step(7, "TH reads OverVoltageThreshold (if OVERVOLT supported)",
                  "DUT returns int64. Store as OverVoltageThreshold.")
        if has_overvolt:
            over_voltage = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverVoltageThreshold)
            asserts.assert_true(isinstance(over_voltage, int), "OverVoltageThreshold must be int64")
        else:
            self.mark_current_step_skipped()

        self.step(8, "TH reads UnderVoltageThreshold (if UNDERVOLT supported)",
                  "DUT returns int64. Store as UnderVoltageThreshold. If both supported: OverVoltage >= UnderVoltage + 1.")
        if has_undervolt:
            under_voltage = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderVoltageThreshold)
            asserts.assert_true(isinstance(under_voltage, int), "UnderVoltageThreshold must be int64")
        else:
            self.mark_current_step_skipped()
        if has_overvolt and has_undervolt:
            asserts.assert_greater_equal(over_voltage, under_voltage + 1,
                                         "OverVoltageThreshold must be >= UnderVoltageThreshold + 1")

        self.step(9, "TH reads OverFrequencyThreshold (if OVERFREQ supported)",
                  "DUT returns int64. Store as OverFrequencyThreshold.")
        if has_overfreq:
            over_frequency = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverFrequencyThreshold)
            asserts.assert_true(isinstance(over_frequency, int), "OverFrequencyThreshold must be int64")
        else:
            self.mark_current_step_skipped()

        self.step(10, "TH reads UnderFrequencyThreshold (if UNDERFREQ supported)",
                  "DUT returns int64. Store as UnderFrequencyThreshold. If both supported: OverFreq >= UnderFreq + 1.")
        if has_underfreq:
            under_frequency = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderFrequencyThreshold)
            asserts.assert_true(isinstance(under_frequency, int), "UnderFrequencyThreshold must be int64")
        else:
            self.mark_current_step_skipped()
        if has_overfreq and has_underfreq:
            asserts.assert_greater_equal(over_frequency, under_frequency + 1,
                                         "OverFrequencyThreshold must be >= UnderFrequencyThreshold + 1")

        self.step(11, "TH reads OverPowerThreshold (if OVERPOWER supported)",
                  "DUT returns int64. Store as OverPowerThreshold.")
        if has_overpower:
            over_power = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverPowerThreshold)
            asserts.assert_true(isinstance(over_power, int), "OverPowerThreshold must be int64")
        else:
            self.mark_current_step_skipped()

        self.step(12, "TH reads UnderPowerThreshold (if UNDERPOWER supported)",
                  "DUT returns int64. If both supported: OverPower >= UnderPower + 1.")
        if has_underpower:
            under_power = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderPowerThreshold)
            asserts.assert_true(isinstance(under_power, int), "UnderPowerThreshold must be int64")
        else:
            self.mark_current_step_skipped()
        if has_overpower and has_underpower:
            asserts.assert_greater_equal(over_power, under_power + 1,
                                         "OverPowerThreshold must be >= UnderPowerThreshold + 1")

        self.step(13, "TH reads OverCurrentThreshold (if OVERCUR supported)",
                  "DUT returns int64. Store as OverCurrentThreshold.")
        if has_overcur:
            over_current = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverCurrentThreshold)
            asserts.assert_true(isinstance(over_current, int), "OverCurrentThreshold must be int64")
        else:
            self.mark_current_step_skipped()

        self.step(14, "TH reads UnderCurrentThreshold (if UNDERCUR supported)",
                  "DUT returns int64. If both supported: OverCurrent >= UnderCurrent + 1.")
        if has_undercur:
            under_current = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderCurrentThreshold)
            asserts.assert_true(isinstance(under_current, int), "UnderCurrentThreshold must be int64")
        else:
            self.mark_current_step_skipped()
        if has_overcur and has_undercur:
            asserts.assert_greater_equal(over_current, under_current + 1,
                                         "OverCurrentThreshold must be >= UnderCurrentThreshold + 1")

        self.step(15, "TH reads PowerImportThreshold (if POWERIMP supported)",
                  "DUT returns int64 >= 0.")
        if has_powerimp:
            power_import = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerImportThreshold)
            asserts.assert_true(isinstance(power_import, int), "PowerImportThreshold must be int64")
            asserts.assert_greater_equal(power_import, 0, "PowerImportThreshold must be >= 0")
        else:
            self.mark_current_step_skipped()

        self.step(16, "TH reads PowerExportThreshold (if POWEREXP supported)",
                  "DUT returns int64 <= 0.")
        if has_powerexp:
            power_export = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerExportThreshold)
            asserts.assert_true(isinstance(power_export, int), "PowerExportThreshold must be int64")
            asserts.assert_less_equal(power_export, 0, "PowerExportThreshold must be <= 0")
        else:
            self.mark_current_step_skipped()

        self.step(17, "TH attempts write to OverVoltageThreshold (if OVERVOLT supported)",
                  "DUT returns UNSUPPORTED_WRITE (attribute is read-only).")
        if has_overvolt:
            write_val = attrs.OverVoltageThreshold(over_voltage if over_voltage is not None else 0)
            status = await self.write_single_attribute(
                attribute_value=write_val, endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.UnsupportedWrite,
                                 "Expected UNSUPPORTED_WRITE for read-only OverVoltageThreshold")
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
