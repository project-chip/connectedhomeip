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
from collections.abc import Sequence
from enum import IntFlag

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm

# amperage-mA and voltage-mV fields in this cluster are declared min 1, max 0x3FFFFFFFFFFFFFFF.
# matter_asserts.assert_valid_int64 alone permits up to 0x7FFFFFFFFFFFFFFF.
MEASUREMENT_MAX = 0x3FFFFFFFFFFFFFFF

# Alarm Base bit 0. EPALM sets RESET to disallowConform, so Alchemy omits it from the generated
# Feature IntFlag and the literal is the only way to name it.
RESET_FEATURE_BIT = 0x1

# Alarm Base attribute id for Latch, likewise absent from the generated bindings.
LATCH_ATTRIBUTE_ID = 0x0001


def _defined_bits(bitmap: type[IntFlag]) -> int:
    """Union of every bit the generated bitmap defines, as a plain int.

    int() is deliberate. These are IntFlag, so `value & ~SomeFlag` masks back to the class's own
    bits and is always 0, which silently turns a reserved-bit assertion into a no-op.
    """
    return sum(int(bit) for bit in bitmap)


def _check_bitmap(value: int, name: str, bitmap: type[IntFlag], min_one_bit: bool = False) -> None:
    """Validate a bitmap value: only spec-defined bits set, optionally at least one.

    matter_asserts has assert_valid_map8 but no map32 equivalent, so the reserved-bit check is
    expressed here once rather than open-coded per attribute.
    """
    defined = _defined_bits(bitmap)
    asserts.assert_equal(int(value) & ~defined, 0,
                         f'{name} sets bits outside {bitmap.__name__} (defined mask 0x{defined:X})')
    if min_one_bit:
        asserts.assert_not_equal(int(value), 0, f'{name} must have at least one bit set (constraint min 1)')


def _check_measurement(value: int, name: str) -> None:
    """amperage-mA / voltage-mV field: int64 constrained to [1, MEASUREMENT_MAX]."""
    matter_asserts.assert_valid_int64(value, f'{name} must be an int64')
    matter_asserts.assert_int_in_range(value, 1, MEASUREMENT_MAX, name)


def _check_uint_min_one(value: int, name: str) -> None:
    """uint64 field declared min 1 with no maximum."""
    matter_asserts.assert_valid_uint64(value, f'{name} must be a uint64')
    asserts.assert_greater_equal(value, 1, f'{name} must be at least 1')


def _check_choice_group(struct, fields: Sequence[str], struct_name: str) -> None:
    """O.<group>+ choice conformance: at least one field of the group must be present."""
    asserts.assert_true(any(getattr(struct, f) is not None for f in fields),
                        f'{struct_name} has no fields present; choice conformance requires at least one')


def _check_current_trip_mechanism(value: int) -> None:
    _check_bitmap(value, 'TripMechanism', cluster.Bitmaps.CurrentTripMechanismBitmap, min_one_bit=True)


def _check_voltage_trip_mechanism(value: int) -> None:
    _check_bitmap(value, 'TripMechanism', cluster.Bitmaps.VoltageTripMechanismBitmap, min_one_bit=True)


def _check_trip_curve(value) -> None:
    matter_asserts.assert_valid_enum(value, 'TripCurve must be a CurrentTripCurveEnum',
                                     cluster.Enums.CurrentTripCurveEnum)


def _check_arc_fault_ratings_struct(s) -> None:
    _check_choice_group(s, ('seriesArcCurrentSensitivity', 'parallelArcCurrentSensitivity',
                            'supportedArcCauses'), 'ArcFaultRatingsStruct')
    if s.seriesArcCurrentSensitivity is not None:
        _check_measurement(s.seriesArcCurrentSensitivity, 'SeriesArcCurrentSensitivity')
    if s.parallelArcCurrentSensitivity is not None:
        _check_measurement(s.parallelArcCurrentSensitivity, 'ParallelArcCurrentSensitivity')
    if s.supportedArcCauses is not None:
        _check_bitmap(s.supportedArcCauses, 'SupportedArcCauses', cluster.Bitmaps.ArcCauseBitmap,
                      min_one_bit=True)


def _check_over_load_ratings_struct(s) -> None:
    _check_choice_group(s, ('tripCurrent', 'tripCurve', 'tripMechanism', 'ultimateMaxCurrent',
                            'serviceMaxCurrent'), 'OverLoadRatingsStruct')
    if s.tripCurrent is not None:
        _check_measurement(s.tripCurrent, 'TripCurrent')
    if s.tripCurve is not None:
        _check_trip_curve(s.tripCurve)
    if s.tripMechanism is not None:
        _check_current_trip_mechanism(s.tripMechanism)
    if s.ultimateMaxCurrent is not None:
        _check_measurement(s.ultimateMaxCurrent, 'UltimateMaxCurrent')
    if s.serviceMaxCurrent is not None:
        _check_measurement(s.serviceMaxCurrent, 'ServiceMaxCurrent')


def _check_over_voltage_ratings_struct(s) -> None:
    _check_choice_group(s, ('tripMechanism', 'tripVoltage', 'maxContinuousOperatingVoltage',
                            'responseTime'), 'OverVoltageRatingsStruct')
    if s.tripMechanism is not None:
        _check_voltage_trip_mechanism(s.tripMechanism)
    if s.tripVoltage is not None:
        _check_measurement(s.tripVoltage, 'TripVoltage')
    if s.maxContinuousOperatingVoltage is not None:
        _check_measurement(s.maxContinuousOperatingVoltage, 'MaxContinuousOperatingVoltage')
    if s.responseTime is not None:
        _check_uint_min_one(s.responseTime, 'ResponseTime')


def _check_residual_current_fault_ratings_struct(s) -> None:
    _check_choice_group(s, ('currentSensitivity', 'tripMechanism', 'voltageDependent',
                            'groundFaultClass', 'waveform', 'ultimateMaxCurrent',
                            'serviceMaxCurrent'), 'ResidualCurrentFaultRatingsStruct')
    if s.currentSensitivity is not None:
        _check_measurement(s.currentSensitivity, 'CurrentSensitivity')
    if s.tripMechanism is not None:
        _check_current_trip_mechanism(s.tripMechanism)
    if s.voltageDependent is not None:
        matter_asserts.assert_valid_bool(s.voltageDependent, 'VoltageDependent must be a bool')
    if s.groundFaultClass is not None:
        matter_asserts.assert_valid_enum(s.groundFaultClass, 'GroundFaultClass must be a GroundFaultClassEnum',
                                         cluster.Enums.GroundFaultClassEnum)
    if s.waveform is not None:
        matter_asserts.assert_valid_enum(s.waveform, 'Waveform must be a CurrentWaveformEnum',
                                         cluster.Enums.CurrentWaveformEnum)
    # Conditional conformance: TrippingCharacteristic is present exactly when Waveform is.
    if s.waveform is not None:
        asserts.assert_is_not_none(s.trippingCharacteristic,
                                   'TrippingCharacteristic must be present when Waveform is present')
        _check_bitmap(s.trippingCharacteristic, 'TrippingCharacteristic',
                      cluster.Bitmaps.TrippingCharacteristicsBitmap)
    else:
        asserts.assert_is_none(s.trippingCharacteristic,
                               'TrippingCharacteristic must be absent when Waveform is absent')
    if s.ultimateMaxCurrent is not None:
        _check_measurement(s.ultimateMaxCurrent, 'UltimateMaxCurrent')
    if s.serviceMaxCurrent is not None:
        _check_measurement(s.serviceMaxCurrent, 'ServiceMaxCurrent')


def _check_short_circuit_ratings_struct(s) -> None:
    _check_choice_group(s, ('tripCurrent', 'tripMechanism', 'tripCurve', 'ultimateMaxCurrent',
                            'serviceMaxCurrent', 'maxCurrent'), 'ShortCircuitRatingsStruct')
    # These amperage-mA fields carry constraint "all" in the spec, unlike every other amperage-mA
    # field in this cluster, which is "min 1". Raised with the test team as a possible spec defect;
    # validated as int64 only until that is settled.
    for name, value in (('TripCurrent', s.tripCurrent),
                        ('UltimateMaxCurrent', s.ultimateMaxCurrent),
                        ('ServiceMaxCurrent', s.serviceMaxCurrent),
                        ('MaxCurrent', s.maxCurrent)):
        if value is not None:
            matter_asserts.assert_valid_int64(value, f'{name} must be an int64')
            matter_asserts.assert_int_in_range(value, -MEASUREMENT_MAX - 1, MEASUREMENT_MAX, name)
    if s.tripMechanism is not None:
        _check_current_trip_mechanism(s.tripMechanism)
    if s.tripCurve is not None:
        _check_trip_curve(s.tripCurve)


def _check_surge_protection_ratings_struct(s) -> None:
    _check_choice_group(s, ('tripMechanism', 'protectionClass', 'protectionType',
                            'maxContinuousOperatingVoltage', 'maxVoltageProtection',
                            'maxTemporaryVoltage', 'nominalDischargeCurrent',
                            'maximumDischargeCurrent', 'ratedShortCircuitCurrent',
                            'ratedShortTimeWithstandCurrent', 'energyAbsorptionCapability',
                            'responseTime'), 'SurgeProtectionRatingsStruct')
    if s.tripMechanism is not None:
        _check_voltage_trip_mechanism(s.tripMechanism)
    if s.protectionClass is not None:
        _check_bitmap(s.protectionClass, 'ProtectionClass', cluster.Bitmaps.SurgeProtectionClassBitmap)
    if s.protectionType is not None:
        _check_bitmap(s.protectionType, 'ProtectionType', cluster.Bitmaps.SurgeProtectionTypeBitmap)
    for name, value in (('MaxContinuousOperatingVoltage', s.maxContinuousOperatingVoltage),
                        ('MaxVoltageProtection', s.maxVoltageProtection),
                        ('MaxTemporaryVoltage', s.maxTemporaryVoltage),
                        ('NominalDischargeCurrent', s.nominalDischargeCurrent),
                        ('MaximumDischargeCurrent', s.maximumDischargeCurrent),
                        ('RatedShortCircuitCurrent', s.ratedShortCircuitCurrent),
                        ('RatedShortTimeWithstandCurrent', s.ratedShortTimeWithstandCurrent)):
        if value is not None:
            _check_measurement(value, name)
    if s.energyAbsorptionCapability is not None:
        _check_uint_min_one(s.energyAbsorptionCapability, 'EnergyAbsorptionCapability')
    if s.responseTime is not None:
        _check_uint_min_one(s.responseTime, 'ResponseTime')


class TC_EPALM_2_1(MatterBaseTest):

    @pics('EPALM.S')
    @async_test_body
    async def test_TC_EPALM_2_1(self):
        """[TC-EPALM-2.1] Attributes with Server as DUT

        This test case verifies the mandatory and feature-conditional server attributes of the
        Electrical Protection Alarm Cluster: each attribute is implemented with the correct data
        type, satisfies its declared constraint, and (where applicable) the conditional rating
        attributes are present only when their gating feature is supported. The Alarm Base
        inherited attributes (Mask, Latch, State, Supported) are also verified.
        """
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        self.step(2, "TH reads the FeatureMap attribute",
                  expectation="DUT responds with a map32 value. Bit 0 (RESET) MUST NOT be set, "
                              "because RESET is disallowConform on EPALM.")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.FeatureMap)
        log.info('FeatureMap: 0x%08X', feature_map)
        # Choice conformance across the seven alarm-class bits belongs to TC-EPALM-2.3, which
        # checks FeatureMap against Supported bidirectionally. Only the EPALM-specific RESET
        # prohibition is asserted here. IDM-10.5 would cover it generically, but only once a 1.7
        # data-model XML exists; the prebuilt directories stop at 1.6.1 and carry no EPALM.
        asserts.assert_equal(feature_map & RESET_FEATURE_BIT, 0,
                             'FeatureMap must not set bit 0: RESET is disallowConform on EPALM')

        self.step(3, "TH reads the Mask attribute (inherited from Alarm Base)",
                  expectation="DUT responds with a map32 value (AlarmBitmap) indicating which "
                              "alarms are enabled by the application.")
        mask_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Mask)
        _check_bitmap(mask_val, 'Mask', cluster.Bitmaps.AlarmBitmap)

        self.step(4, "TH reads the State attribute (inherited from Alarm Base)",
                  expectation="DUT responds with a map32 value (AlarmBitmap). In the no-fault "
                              "baseline established by the test setup this value is 0.")
        state_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.State)
        _check_bitmap(state_val, 'State', cluster.Bitmaps.AlarmBitmap)
        asserts.assert_equal(int(state_val), 0,
                             'State must be 0 in the no-fault baseline established by the test setup')

        self.step(5, "TH reads the Supported attribute (inherited from Alarm Base)",
                  expectation="DUT responds with a map32 value (AlarmBitmap).")
        supported_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Supported)
        # Supported against FeatureMap, per bit and in both directions, is TC-EPALM-2.3's subject.
        _check_bitmap(supported_val, 'Supported', cluster.Bitmaps.AlarmBitmap)

        self.step(6, "TH reads the AttributeList attribute and verifies Latch is absent",
                  expectation="Latch (attribute 0x0001) is absent from AttributeList. Its Alarm "
                              "Base conformance is RESET, and RESET is disallowConform for EPALM, "
                              "so the attribute MUST NOT be implemented.")
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.AttributeList)
        asserts.assert_not_in(LATCH_ATTRIBUTE_ID, attribute_list,
                              'Latch must not be implemented: it is RESET-conditional and EPALM '
                              'disallows RESET')

        self.step(7, "TH reads the ArcCause attribute",
                  expectation="DUT responds with a map8 value (ArcCauseBitmap), OR null. Only "
                              "spec-defined bits 0 (Series), 1 (ParallelToNeutral), "
                              "2 (ParallelToGround) may be set; reserved bits are 0.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ArcCause):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ArcCause)
            if val is not NullValue:
                matter_asserts.assert_valid_map8(val, 'ArcCause must be a valid map8')
                _check_bitmap(val, 'ArcCause', cluster.Bitmaps.ArcCauseBitmap)

        self.step(8, "TH reads the OverLoadRating attribute",
                  expectation="DUT responds with an OverLoadRatingsStruct value, OR null. Each "
                              "present field conforms to its declared type and constraint.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.OverLoadRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.OverLoadRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.OverLoadRatingsStruct),
                                    'OverLoadRating must be an OverLoadRatingsStruct')
                _check_over_load_ratings_struct(val)

        self.step(9, "TH reads the OverVoltageRating attribute",
                  expectation="DUT responds with an OverVoltageRatingsStruct value, OR null. Each "
                              "present field conforms to its declared type and constraint.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.OverVoltageRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.OverVoltageRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.OverVoltageRatingsStruct),
                                    'OverVoltageRating must be an OverVoltageRatingsStruct')
                _check_over_voltage_ratings_struct(val)

        self.step(10, "TH reads the SurgeProtectionRating attribute",
                  expectation="DUT responds with a SurgeProtectionRatingsStruct value, OR null. "
                              "Each present field conforms to its declared type and constraint.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SurgeProtectionRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.SurgeProtectionRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.SurgeProtectionRatingsStruct),
                                    'SurgeProtectionRating must be a SurgeProtectionRatingsStruct')
                _check_surge_protection_ratings_struct(val)

        self.step(11, "TH reads the ShortCircuitRating attribute",
                  expectation="DUT responds with a ShortCircuitRatingsStruct value, OR null. Each "
                              "present field conforms to its declared type and constraint.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ShortCircuitRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ShortCircuitRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.ShortCircuitRatingsStruct),
                                    'ShortCircuitRating must be a ShortCircuitRatingsStruct')
                _check_short_circuit_ratings_struct(val)

        self.step(12, "TH reads the ResidualCurrentRating attribute",
                  expectation="DUT responds with a ResidualCurrentFaultRatingsStruct value, OR "
                              "null. If Waveform is present, TrippingCharacteristic MUST be "
                              "present; if Waveform is absent, it MUST be absent.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ResidualCurrentRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ResidualCurrentRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.ResidualCurrentFaultRatingsStruct),
                                    'ResidualCurrentRating must be a ResidualCurrentFaultRatingsStruct')
                _check_residual_current_fault_ratings_struct(val)

        self.step(13, "TH reads the ArcFaultRating attribute",
                  expectation="DUT responds with an ArcFaultRatingsStruct value, OR null. Each "
                              "present field conforms to its declared type and constraint.")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ArcFaultRating):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ArcFaultRating)
            if val is not NullValue:
                asserts.assert_true(isinstance(val, cluster.Structs.ArcFaultRatingsStruct),
                                    'ArcFaultRating must be an ArcFaultRatingsStruct')
                _check_arc_fault_ratings_struct(val)

        self.step(14, "TH verifies the Alarm Base subset invariants across the values recorded in "
                      "steps 3, 4 and 5",
                  expectation="Every bit set in Mask is also set in Supported, and every bit set "
                              "in State is also set in Supported. An alarm cannot be masked or "
                              "active unless it is supported.")
        asserts.assert_equal(int(mask_val) & ~int(supported_val), 0, 'Mask has bits not in Supported')
        asserts.assert_equal(int(state_val) & ~int(supported_val), 0, 'State has bits not in Supported')


if __name__ == "__main__":
    default_matter_test_main()
