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
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

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


class TC_ESALM_2_2(MatterBaseTest):

    async def _send_set_thresholds(self, endpoint, **kwargs):
        cmd = cluster.Commands.SetElectricalAlarmThresholds(**kwargs)
        await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

    async def _send_set_thresholds_expect_error(self, endpoint, expected_status, **kwargs):
        cmd = cluster.Commands.SetElectricalAlarmThresholds(**kwargs)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.fail("Expected %s but command succeeded" % expected_status)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Expected %s, got %s" % (expected_status, e.status))

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_ESALM_2_2(self):
        """[TC-ESALM-2.2] SetElectricalAlarmThresholds Command with Server as DUT

        Verify that the SetElectricalAlarmThresholds command correctly sets threshold
        attributes, enforces cross-field constraints, and that written values persist
        across a device reboot (Non-Volatile).
        """
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes

        self.step(1, "Commission DUT to TH", is_commissioning=True)

        self.step(2, "TH reads FeatureMap; reads all supported threshold attributes and stores originals",
                  "SUCCESS for each read.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.FeatureMap)
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

        orig_over_voltage = None
        orig_under_voltage = None
        orig_over_frequency = None
        orig_under_frequency = None
        orig_over_power = None
        orig_under_power = None
        orig_over_current = None
        orig_under_current = None
        orig_power_import = None
        orig_power_export = None

        if has_overvolt:
            orig_over_voltage = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverVoltageThreshold)
        if has_undervolt:
            orig_under_voltage = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderVoltageThreshold)
        if has_overfreq:
            orig_over_frequency = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverFrequencyThreshold)
        if has_underfreq:
            orig_under_frequency = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderFrequencyThreshold)
        if has_overpower:
            orig_over_power = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverPowerThreshold)
        if has_underpower:
            orig_under_power = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderPowerThreshold)
        if has_overcur:
            orig_over_current = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverCurrentThreshold)
        if has_undercur:
            orig_under_current = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderCurrentThreshold)
        if has_powerimp:
            orig_power_import = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerImportThreshold)
        if has_powerexp:
            orig_power_export = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerExportThreshold)

        new_over_voltage = None
        new_under_voltage = None

        self.step(3, "TH sends SetElectricalAlarmThresholds with OverVoltageThreshold = ORIG + 1000; reads back",
                  "SUCCESS. Read-back equals ORIG + 1000.")
        if has_overvolt:
            new_over_voltage = orig_over_voltage + 1000
            await self._send_set_thresholds(endpoint, overVoltageThreshold=new_over_voltage)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverVoltageThreshold)
            asserts.assert_equal(readback, new_over_voltage, "OverVoltageThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(4, "TH sends SetElectricalAlarmThresholds with UnderVoltageThreshold = ORIG - 1000; reads back",
                  "SUCCESS. Read-back equals ORIG - 1000.")
        if has_undervolt:
            new_under_voltage = orig_under_voltage - 1000
            await self._send_set_thresholds(endpoint, underVoltageThreshold=new_under_voltage)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderVoltageThreshold)
            asserts.assert_equal(readback, new_under_voltage, "UnderVoltageThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(5, "TH sends SetElectricalAlarmThresholds with OverVoltageThreshold = current UnderVoltageThreshold (constraint violation)",
                  "DUT returns CONSTRAINT_ERROR. OverVoltageThreshold unchanged from step 3.")
        if has_overvolt and has_undervolt:
            current_under = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderVoltageThreshold)
            await self._send_set_thresholds_expect_error(
                endpoint, Status.ConstraintError, overVoltageThreshold=current_under)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverVoltageThreshold)
            asserts.assert_equal(readback, new_over_voltage, "OverVoltageThreshold changed after CONSTRAINT_ERROR")
        else:
            self.mark_current_step_skipped()

        self.step(6, "TH sends SetElectricalAlarmThresholds with OverFrequencyThreshold = ORIG + 1000; reads back",
                  "SUCCESS. Read-back equals ORIG + 1000.")
        if has_overfreq:
            new_over_freq = orig_over_frequency + 1000
            await self._send_set_thresholds(endpoint, overFrequencyThreshold=new_over_freq)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverFrequencyThreshold)
            asserts.assert_equal(readback, new_over_freq, "OverFrequencyThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(7, "TH sends SetElectricalAlarmThresholds with UnderFrequencyThreshold = ORIG - 1000; reads back",
                  "SUCCESS. Read-back equals ORIG - 1000.")
        if has_underfreq:
            new_under_freq = orig_under_frequency - 1000
            await self._send_set_thresholds(endpoint, underFrequencyThreshold=new_under_freq)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderFrequencyThreshold)
            asserts.assert_equal(readback, new_under_freq, "UnderFrequencyThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(8, "TH sends SetElectricalAlarmThresholds with OverPowerThreshold = ORIG + 1000; reads back",
                  "SUCCESS. Read-back equals ORIG + 1000.")
        if has_overpower:
            new_over_power = orig_over_power + 1000
            await self._send_set_thresholds(endpoint, overPowerThreshold=new_over_power)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverPowerThreshold)
            asserts.assert_equal(readback, new_over_power, "OverPowerThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(9, "TH sends SetElectricalAlarmThresholds with UnderPowerThreshold = ORIG - 1000; reads back",
                  "SUCCESS. Read-back equals ORIG - 1000.")
        if has_underpower:
            new_under_power = orig_under_power - 1000
            await self._send_set_thresholds(endpoint, underPowerThreshold=new_under_power)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderPowerThreshold)
            asserts.assert_equal(readback, new_under_power, "UnderPowerThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(10, "TH sends SetElectricalAlarmThresholds with OverCurrentThreshold = ORIG + 1000; reads back",
                  "SUCCESS. Read-back equals ORIG + 1000.")
        if has_overcur:
            new_over_current = orig_over_current + 1000
            await self._send_set_thresholds(endpoint, overCurrentThreshold=new_over_current)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverCurrentThreshold)
            asserts.assert_equal(readback, new_over_current, "OverCurrentThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(11, "TH sends SetElectricalAlarmThresholds with UnderCurrentThreshold = ORIG - 1000; reads back",
                  "SUCCESS. Read-back equals ORIG - 1000.")
        if has_undercur:
            new_under_current = orig_under_current - 1000
            await self._send_set_thresholds(endpoint, underCurrentThreshold=new_under_current)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderCurrentThreshold)
            asserts.assert_equal(readback, new_under_current, "UnderCurrentThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        new_power_import = None
        self.step(12, "TH sends SetElectricalAlarmThresholds with PowerImportThreshold = 1000; reads back",
                  "SUCCESS. Read-back equals 1000.")
        if has_powerimp:
            new_power_import = 1000
            await self._send_set_thresholds(endpoint, powerImportThreshold=new_power_import)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerImportThreshold)
            asserts.assert_equal(readback, new_power_import, "PowerImportThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(13, "TH sends SetElectricalAlarmThresholds with PowerImportThreshold = -1 (violates min 0)",
                  "DUT returns CONSTRAINT_ERROR. PowerImportThreshold unchanged from step 12.")
        if has_powerimp:
            await self._send_set_thresholds_expect_error(endpoint, Status.ConstraintError, powerImportThreshold=-1)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerImportThreshold)
            asserts.assert_equal(readback, new_power_import, "PowerImportThreshold changed after CONSTRAINT_ERROR")
        else:
            self.mark_current_step_skipped()

        new_power_export = None
        self.step(14, "TH sends SetElectricalAlarmThresholds with PowerExportThreshold = -1000; reads back",
                  "SUCCESS. Read-back equals -1000.")
        if has_powerexp:
            new_power_export = -1000
            await self._send_set_thresholds(endpoint, powerExportThreshold=new_power_export)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerExportThreshold)
            asserts.assert_equal(readback, new_power_export, "PowerExportThreshold read-back mismatch after write")
        else:
            self.mark_current_step_skipped()

        self.step(15, "TH sends SetElectricalAlarmThresholds with PowerExportThreshold = 1 (violates max 0)",
                  "DUT returns CONSTRAINT_ERROR. PowerExportThreshold unchanged from step 14.")
        if has_powerexp:
            await self._send_set_thresholds_expect_error(endpoint, Status.ConstraintError, powerExportThreshold=1)
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerExportThreshold)
            asserts.assert_equal(readback, new_power_export, "PowerExportThreshold changed after CONSTRAINT_ERROR")
        else:
            self.mark_current_step_skipped()

        self.step(16, "TH reboots DUT and re-commissions; reads all threshold attributes written in steps 3-14",
                  "Each attribute returns the value written before reboot (Non-Volatile).")
        await self.request_device_reboot()

        if has_overvolt:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverVoltageThreshold)
            asserts.assert_equal(readback, new_over_voltage, "OverVoltageThreshold not persisted across reboot")
        if has_undervolt:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderVoltageThreshold)
            asserts.assert_equal(readback, new_under_voltage, "UnderVoltageThreshold not persisted across reboot")
        if has_overfreq:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverFrequencyThreshold)
            asserts.assert_equal(readback, new_over_freq, "OverFrequencyThreshold not persisted across reboot")
        if has_underfreq:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderFrequencyThreshold)
            asserts.assert_equal(readback, new_under_freq, "UnderFrequencyThreshold not persisted across reboot")
        if has_overpower:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverPowerThreshold)
            asserts.assert_equal(readback, new_over_power, "OverPowerThreshold not persisted across reboot")
        if has_underpower:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderPowerThreshold)
            asserts.assert_equal(readback, new_under_power, "UnderPowerThreshold not persisted across reboot")
        if has_overcur:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.OverCurrentThreshold)
            asserts.assert_equal(readback, new_over_current, "OverCurrentThreshold not persisted across reboot")
        if has_undercur:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.UnderCurrentThreshold)
            asserts.assert_equal(readback, new_under_current, "UnderCurrentThreshold not persisted across reboot")
        if has_powerimp:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerImportThreshold)
            asserts.assert_equal(readback, new_power_import, "PowerImportThreshold not persisted across reboot")
        if has_powerexp:
            readback = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.PowerExportThreshold)
            asserts.assert_equal(readback, new_power_export, "PowerExportThreshold not persisted across reboot")

        self.step(17, "TH sends SetElectricalAlarmThresholds to restore all attributes to original values",
                  "SUCCESS for each restore.")
        restore_kwargs = {}
        if has_overvolt:
            restore_kwargs['overVoltageThreshold'] = orig_over_voltage
        if has_undervolt:
            restore_kwargs['underVoltageThreshold'] = orig_under_voltage
        if has_overfreq:
            restore_kwargs['overFrequencyThreshold'] = orig_over_frequency
        if has_underfreq:
            restore_kwargs['underFrequencyThreshold'] = orig_under_frequency
        if has_overpower:
            restore_kwargs['overPowerThreshold'] = orig_over_power
        if has_underpower:
            restore_kwargs['underPowerThreshold'] = orig_under_power
        if has_overcur:
            restore_kwargs['overCurrentThreshold'] = orig_over_current
        if has_undercur:
            restore_kwargs['underCurrentThreshold'] = orig_under_current
        if has_powerimp:
            restore_kwargs['powerImportThreshold'] = orig_power_import
        if has_powerexp:
            restore_kwargs['powerExportThreshold'] = orig_power_export
        if restore_kwargs:
            await self._send_set_thresholds(endpoint, **restore_kwargs)


if __name__ == "__main__":
    default_matter_test_main()
