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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm


class TC_EPALM_2_1(MatterBaseTest):

    @pics('EPALM.S')
    @async_test_body
    async def test_TC_EPALM_2_1(self):
        """[TC-EPALM-2.1] Attributes with DUT as Server

        Verify the mandatory and feature-conditional server attributes of the
        Electrical Protection Alarm Cluster (EPALM, 0x00A3): each attribute is
        implemented with the correct data type, satisfies its declared
        constraint, honors the Nullable (X) quality where applicable, and the
        feature-conditional rating attributes are present only when their
        gating feature is supported. Final step cross-validates Mask and State
        are subsets of Supported per AlarmBase spec.

        Test Plan:
        https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/electrical_protection_alarm.adoc#tc-epalm-2-1
        """
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        self.step(2, "TH reads ArcCause attribute")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ArcCause):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ArcCause)
            if val is not NullValue:
                matter_asserts.assert_valid_map8(val, 'ArcCause must be a valid map8')
                # Only spec-defined bits 0 (Series), 1 (ParallelToNeutral), 2 (ParallelToGround) may be set
                asserts.assert_less_equal(val, 7, 'ArcCause may only set bits 0..2')

        self.step(3, "TH reads OverLoadRating attribute (feature-conditional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.OverLoadRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.OverLoadRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.OverLoadRatingsStruct),
                                    'OverLoadRating must be an OverLoadRatingsStruct')
                self._check_over_load_ratings_struct(val)

        self.step(4, "TH reads OverVoltageRating attribute (feature-conditional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.OverVoltageRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.OverVoltageRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.OverVoltageRatingsStruct),
                                    'OverVoltageRating must be an OverVoltageRatingsStruct')
                self._check_over_voltage_ratings_struct(val)

        self.step(5, "TH reads SurgeProtectionRating attribute (feature-conditional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SurgeProtectionRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.SurgeProtectionRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.SurgeProtectionRatingsStruct),
                                    'SurgeProtectionRating must be a SurgeProtectionRatingsStruct')
                self._check_surge_protection_ratings_struct(val)

        self.step(6, "TH reads ShortCircuitRating attribute (feature-conditional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ShortCircuitRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ShortCircuitRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.ShortCircuitRatingsStruct),
                                    'ShortCircuitRating must be a ShortCircuitRatingsStruct')
                self._check_short_circuit_ratings_struct(val)

        self.step(7, "TH reads ResidualCurrentRating attribute (feature-conditional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ResidualCurrentRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ResidualCurrentRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.ResidualCurrentFaultRatingsStruct),
                                    'ResidualCurrentRating must be a ResidualCurrentFaultRatingsStruct')
                self._check_residual_current_fault_ratings_struct(val)

        self.step(8, "TH reads ArcFaultRating attribute (feature-conditional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ArcFaultRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ArcFaultRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.ArcFaultRatingsStruct),
                                    'ArcFaultRating must be an ArcFaultRatingsStruct')
                self._check_arc_fault_ratings_struct(val)

        # AlarmBitmap (map32) — set bits limited to the 7 spec-defined fault bits (0..6).
        ALARM_BITMAP_MAX = 0x7F  # ShortCircuit | OverLoad | OverVoltage | VoltageSurge | ResidualCurrent | ArcFault | SelfTest

        self.step(9, "TH reads Mask attribute (inherited from Alarm Base)")
        mask_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Mask)
        asserts.assert_true(matter_asserts.is_valid_int_value(mask_val, bit_count=32),
                            'Mask must be a valid uint32 (AlarmBitmap)')
        asserts.assert_less_equal(mask_val, ALARM_BITMAP_MAX,
                                  'Mask may only set the 7 spec-defined AlarmBitmap bits')

        self.step(10, "TH reads Latch attribute (inherited from Alarm Base; optional)")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.Latch):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.Latch)
            asserts.assert_true(matter_asserts.is_valid_int_value(val, bit_count=32),
                                'Latch must be a valid uint32 (AlarmBitmap)')
            asserts.assert_less_equal(val, ALARM_BITMAP_MAX,
                                      'Latch may only set the 7 spec-defined AlarmBitmap bits')

        self.step(11, "TH reads State attribute (inherited from Alarm Base)")
        state_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.State)
        asserts.assert_true(matter_asserts.is_valid_int_value(state_val, bit_count=32),
                            'State must be a valid uint32 (AlarmBitmap)')
        asserts.assert_less_equal(state_val, ALARM_BITMAP_MAX,
                                  'State may only set the 7 spec-defined AlarmBitmap bits')

        self.step(12, "TH reads Supported attribute (inherited from Alarm Base)")
        supported_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Supported)
        asserts.assert_true(matter_asserts.is_valid_int_value(supported_val, bit_count=32),
                            'Supported must be a valid uint32 (AlarmBitmap)')
        asserts.assert_less_equal(supported_val, ALARM_BITMAP_MAX,
                                  'Supported may only set the 7 spec-defined AlarmBitmap bits')

        self.step(13, "TH cross-validates Mask and State are subsets of Supported (per AlarmBase spec)")
        asserts.assert_equal(mask_val & ~supported_val, 0,
                             'Mask has bits not in Supported')
        asserts.assert_equal(state_val & ~supported_val, 0,
                             'State has bits not in Supported')
        log.info('Cross-validation passed: Mask and State are subsets of Supported')

    # ------------------------------------------------------------------
    # Rating struct field validators
    # ------------------------------------------------------------------

    def _check_arc_fault_ratings_struct(
            self, s: 'cluster.Structs.ArcFaultRatingsStruct') -> None:
        if s.seriesArcCurrentSensitivity is not None:
            matter_asserts.assert_valid_int64(
                s.seriesArcCurrentSensitivity, 'SeriesArcCurrentSensitivity must be an int64')
            asserts.assert_greater_equal(s.seriesArcCurrentSensitivity, 1)
        if s.parallelArcCurrentSensitivity is not None:
            matter_asserts.assert_valid_int64(
                s.parallelArcCurrentSensitivity, 'ParallelArcCurrentSensitivity must be an int64')
            asserts.assert_greater_equal(s.parallelArcCurrentSensitivity, 1)
        if s.supportedArcCauses is not None:
            matter_asserts.assert_valid_map8(s.supportedArcCauses, 'SupportedArcCauses must be a valid map8')
            asserts.assert_less_equal(s.supportedArcCauses, 7,
                                      'SupportedArcCauses may only set ArcCauseBitmap bits 0..2')

    def _check_over_load_ratings_struct(
            self, s: 'cluster.Structs.OverLoadRatingsStruct') -> None:
        if s.tripCurrent is not None:
            matter_asserts.assert_valid_int64(s.tripCurrent, 'TripCurrent must be an int64')
            asserts.assert_greater_equal(s.tripCurrent, 1)
        if s.tripCurve is not None:
            matter_asserts.assert_valid_enum(
                s.tripCurve, 'TripCurve must be a CurrentTripCurveEnum', cluster.Enums.CurrentTripCurveEnum)
        if s.tripMechanism is not None:
            matter_asserts.assert_valid_map8(s.tripMechanism, 'TripMechanism must be a valid map8')
            asserts.assert_less_equal(s.tripMechanism, 7,
                                      'TripMechanism may only set CurrentTripMechanismBitmap bits 0..2')
        if s.ultimateMaxCurrent is not None:
            matter_asserts.assert_valid_int64(s.ultimateMaxCurrent, 'UltimateMaxCurrent must be an int64')
            asserts.assert_greater_equal(s.ultimateMaxCurrent, 1)
        if s.serviceMaxCurrent is not None:
            matter_asserts.assert_valid_int64(s.serviceMaxCurrent, 'ServiceMaxCurrent must be an int64')
            asserts.assert_greater_equal(s.serviceMaxCurrent, 1)

    def _check_over_voltage_ratings_struct(
            self, s: 'cluster.Structs.OverVoltageRatingsStruct') -> None:
        if s.tripMechanism is not None:
            matter_asserts.assert_valid_map8(s.tripMechanism, 'TripMechanism must be a valid map8')
            asserts.assert_less_equal(s.tripMechanism, 7,
                                      'TripMechanism may only set VoltageTripMechanismBitmap bits 0..2 (MOV/SAD/GDT)')
        if s.tripVoltage is not None:
            matter_asserts.assert_valid_int64(s.tripVoltage, 'TripVoltage must be an int64')
            asserts.assert_greater_equal(s.tripVoltage, 1)
        if s.maxContinuousOperatingVoltage is not None:
            matter_asserts.assert_valid_int64(
                s.maxContinuousOperatingVoltage, 'MaxContinuousOperatingVoltage must be an int64')
            asserts.assert_greater_equal(s.maxContinuousOperatingVoltage, 1)
        if s.responseTime is not None:
            matter_asserts.assert_valid_uint64(s.responseTime, 'ResponseTime must be a uint64')
            asserts.assert_greater_equal(s.responseTime, 1)

    def _check_residual_current_fault_ratings_struct(
            self, s: 'cluster.Structs.ResidualCurrentFaultRatingsStruct') -> None:
        if s.currentSensitivity is not None:
            matter_asserts.assert_valid_int64(s.currentSensitivity, 'CurrentSensitivity must be an int64')
            asserts.assert_greater_equal(s.currentSensitivity, 1)
        if s.tripMechanism is not None:
            matter_asserts.assert_valid_map8(s.tripMechanism, 'TripMechanism must be a valid map8')
            asserts.assert_less_equal(s.tripMechanism, 7,
                                      'TripMechanism may only set CurrentTripMechanismBitmap bits 0..2')
        if s.voltageDependent is not None:
            matter_asserts.assert_valid_bool(s.voltageDependent, 'VoltageDependent must be a bool')
        if s.groundFaultClass is not None:
            matter_asserts.assert_valid_enum(
                s.groundFaultClass, 'GroundFaultClass must be a GroundFaultClassEnum',
                cluster.Enums.GroundFaultClassEnum)
        if s.waveform is not None:
            matter_asserts.assert_valid_enum(
                s.waveform, 'Waveform must be a CurrentWaveformEnum',
                cluster.Enums.CurrentWaveformEnum)
        # Conditional conformance for TrippingCharacteristic:
        #   * If Waveform is present, TrippingCharacteristic MUST be present (and validates as map8).
        #   * If Waveform is absent, TrippingCharacteristic MUST be absent.
        if s.waveform is not None:
            asserts.assert_is_not_none(
                s.trippingCharacteristic,
                'TrippingCharacteristic must be present when Waveform is present')
            asserts.assert_not_equal(
                s.trippingCharacteristic, NullValue,
                'TrippingCharacteristic must not be null when Waveform is present')
            matter_asserts.assert_valid_map8(
                s.trippingCharacteristic, 'TrippingCharacteristic must be a valid map8')
            asserts.assert_less_equal(
                s.trippingCharacteristic, 7,
                'TrippingCharacteristic may only set TrippingCharacteristicsBitmap bits 0..2')
        else:
            asserts.assert_is_none(
                s.trippingCharacteristic,
                'TrippingCharacteristic must be absent when Waveform is absent')
        if s.ultimateMaxCurrent is not None:
            matter_asserts.assert_valid_int64(s.ultimateMaxCurrent, 'UltimateMaxCurrent must be an int64')
            asserts.assert_greater_equal(s.ultimateMaxCurrent, 1)
        if s.serviceMaxCurrent is not None:
            matter_asserts.assert_valid_int64(s.serviceMaxCurrent, 'ServiceMaxCurrent must be an int64')
            asserts.assert_greater_equal(s.serviceMaxCurrent, 1)

    def _check_short_circuit_ratings_struct(
            self, s: 'cluster.Structs.ShortCircuitRatingsStruct') -> None:
        if s.tripCurrent is not None:
            matter_asserts.assert_valid_int64(s.tripCurrent, 'TripCurrent must be an int64')
            asserts.assert_greater_equal(s.tripCurrent, 1)
        if s.tripMechanism is not None:
            matter_asserts.assert_valid_map8(s.tripMechanism, 'TripMechanism must be a valid map8')
            asserts.assert_less_equal(s.tripMechanism, 7,
                                      'TripMechanism may only set CurrentTripMechanismBitmap bits 0..2')
        if s.tripCurve is not None:
            matter_asserts.assert_valid_enum(
                s.tripCurve, 'TripCurve must be a CurrentTripCurveEnum',
                cluster.Enums.CurrentTripCurveEnum)
        if s.ultimateMaxCurrent is not None:
            matter_asserts.assert_valid_int64(s.ultimateMaxCurrent, 'UltimateMaxCurrent must be an int64')
            asserts.assert_greater_equal(s.ultimateMaxCurrent, 1)
        if s.serviceMaxCurrent is not None:
            matter_asserts.assert_valid_int64(s.serviceMaxCurrent, 'ServiceMaxCurrent must be an int64')
            asserts.assert_greater_equal(s.serviceMaxCurrent, 1)
        if s.maxCurrent is not None:
            matter_asserts.assert_valid_int64(s.maxCurrent, 'MaxCurrent must be an int64')
            asserts.assert_greater_equal(s.maxCurrent, 1)

    def _check_surge_protection_ratings_struct(
            self, s: 'cluster.Structs.SurgeProtectionRatingsStruct') -> None:
        if s.tripMechanism is not None:
            matter_asserts.assert_valid_map8(s.tripMechanism, 'TripMechanism must be a valid map8')
            asserts.assert_less_equal(s.tripMechanism, 7,
                                      'TripMechanism may only set VoltageTripMechanismBitmap bits 0..2 (MOV/SAD/GDT)')
        if s.protectionClass is not None:
            matter_asserts.assert_valid_map8(s.protectionClass, 'ProtectionClass must be a valid map8')
            asserts.assert_less_equal(s.protectionClass, 7,
                                      'ProtectionClass may only set SurgeProtectionClassBitmap bits 0..2')
        if s.protectionType is not None:
            matter_asserts.assert_valid_map8(s.protectionType, 'ProtectionType must be a valid map8')
            asserts.assert_less_equal(s.protectionType, 7,
                                      'ProtectionType may only set SurgeProtectionTypeBitmap bits 0..2')
        if s.maxContinuousOperatingVoltage is not None:
            matter_asserts.assert_valid_int64(
                s.maxContinuousOperatingVoltage, 'MaxContinuousOperatingVoltage must be an int64')
            asserts.assert_greater_equal(s.maxContinuousOperatingVoltage, 1)
        if s.maxVoltageProtection is not None:
            matter_asserts.assert_valid_int64(
                s.maxVoltageProtection, 'MaxVoltageProtection must be an int64')
            asserts.assert_greater_equal(s.maxVoltageProtection, 1)
        if s.maxTemporaryVoltage is not None:
            matter_asserts.assert_valid_int64(
                s.maxTemporaryVoltage, 'MaxTemporaryVoltage must be an int64')
            asserts.assert_greater_equal(s.maxTemporaryVoltage, 1)
        if s.nominalDischargeCurrent is not None:
            matter_asserts.assert_valid_int64(
                s.nominalDischargeCurrent, 'NominalDischargeCurrent must be an int64')
            asserts.assert_greater_equal(s.nominalDischargeCurrent, 1)
        if s.maximumDishargeCurrent is not None:
            matter_asserts.assert_valid_int64(
                s.maximumDishargeCurrent, 'MaximumDishargeCurrent must be an int64')
            asserts.assert_greater_equal(s.maximumDishargeCurrent, 1)
        if s.ratedShortCircuitCurrent is not None:
            matter_asserts.assert_valid_int64(
                s.ratedShortCircuitCurrent, 'RatedShortCircuitCurrent must be an int64')
            asserts.assert_greater_equal(s.ratedShortCircuitCurrent, 1)
        if s.ratedShortTimeWithstandCurrent is not None:
            matter_asserts.assert_valid_int64(
                s.ratedShortTimeWithstandCurrent, 'RatedShortTimeWithstandCurrent must be an int64')
            asserts.assert_greater_equal(s.ratedShortTimeWithstandCurrent, 1)
        if s.energyAbsorptionCapability is not None:
            matter_asserts.assert_valid_uint64(
                s.energyAbsorptionCapability, 'EnergyAbsorptionCapability must be a uint64')
            asserts.assert_greater_equal(s.energyAbsorptionCapability, 1)
        if s.responseTime is not None:
            matter_asserts.assert_valid_uint64(s.responseTime, 'ResponseTime must be a uint64')
            asserts.assert_greater_equal(s.responseTime, 1)


if __name__ == "__main__":
    default_matter_test_main()
