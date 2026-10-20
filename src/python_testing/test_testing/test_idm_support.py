#!/usr/bin/env -S python3 -B
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

"""Unit tests for the IDM support helpers TC-IDM-9.1 builds and judges its constraint probes with.

A value the enum helpers wrongly treat as undefined is sent to the DUT as a violation, and a
conformant DUT that accepts it fails the test. Several cases below use generated enums whose real
members share the kUnknown prefix with the codegen sentinel, which is where that goes wrong.
"""

import sys
import unittest
from dataclasses import replace
from pathlib import Path
from types import MethodType, SimpleNamespace
from unittest.mock import AsyncMock, Mock, PropertyMock, patch
from xml.etree import ElementTree

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing import spec_parsing
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.problem_notices import UnknownProblemLocation
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters
from matter.tlv import TLVReader

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.idm_support import (SPEC_VERSION_1_7, CommandFieldInfo, IDMBaseTest, _codegen_enum_values,  # noqa: E402
                                         enum_in_type, smallest_legal_enum_value, spec_enum_values, undefined_enum_values)


class TestDEMPowerRangeConstraints(unittest.IsolatedAsyncioTestCase):

    @classmethod
    def setUpClass(cls):
        cls.xml_clusters, _ = build_xml_clusters(_CHIP_ROOT / 'data_model/1.7/clusters')
        cls.cluster = Clusters.DeviceEnergyManagement
        cls.command_class = cls.cluster.Commands.PowerRangeAdjustRequest
        cls.xml_cluster = cls.xml_clusters[cls.cluster.id]
        cls.xml_command = cls.xml_cluster.accepted_commands[cls.command_class.command_id]

    def setUp(self):
        self.checker = IDMBaseTest.__new__(IDMBaseTest)
        self.checker.endpoints_tlv = {1: {self.cluster.id: {
            self.cluster.Attributes.AbsMinPower.attribute_id: -5000,
            self.cluster.Attributes.AbsMaxPower.attribute_id: -1000,
        }}}
        self.checker.current_test_info = SimpleNamespace(name='DEM constraint regression')
        self.checker.read_single_attribute_check_success = AsyncMock(
            side_effect=lambda **kwargs: self.checker.endpoints_tlv[kwargs['endpoint']][kwargs['cluster'].id].get(
                kwargs['attribute'].attribute_id))
        self.checker.record_warning = Mock()
        self.checker.record_accepted_violation = Mock()

    def _info(self, field_id):
        return CommandFieldInfo(
            endpoint_id=1, cluster_id=self.cluster.id, cluster_name=self.xml_cluster.name,
            command_id=self.command_class.command_id, command_name=self.xml_command.name,
            command_class=self.command_class, cluster_class=self.cluster,
            field=self.xml_command.fields[field_id], all_fields=self.xml_command.fields,
            enum_values_by_field={field_id: spec_enum_values(self.xml_cluster, field.type_info)
                                  for field_id, field in self.xml_command.fields.items()})

    def test_signed_power_alias_generates_encodable_violations(self):
        info = self._info(0)
        violations = self.checker.generate_command_field_violations(
            info.field, spec_parsing.Constraints(min_value=-5000, max_value=-1000))
        self.assertEqual({value for _, value in violations}, {-5001, -999})
        for _, value in violations:
            payload = self.command_class(minPower=value, duration=1)
            self.assertEqual(TLVReader(payload.ToTLV()).get()['Any'][0], value)

    async def test_min_power_only_resolves_absolute_bounds(self):
        request = self.command_class(minPower=-3000, duration=1)
        self.assertIsNone(request.maxPower)
        constraints = await self.checker._resolved_command_field_constraints(self._info(0), request)
        self.assertEqual((constraints.min_value, constraints.max_value), (-5000, -1000))

    async def test_max_power_only_resolves_xml_max_of(self):
        info = self._info(1)
        self.assertTrue(info.field.constraints.unsupported_expressions)
        request = self.command_class(maxPower=-2000, duration=1)
        self.assertIsNone(request.minPower)
        constraints = await self.checker._resolved_command_field_constraints(info, request)
        self.assertEqual((constraints.min_value, constraints.max_value), (-5000, -1000))
        self.assertFalse(constraints.unsupported_expressions)
        self.assertTrue(info.field.constraints.unsupported_expressions)

    async def test_both_powers_resolve_against_constructed_request(self):
        request = self.command_class(minPower=-3000, maxPower=-2000, duration=1)
        constraints = await self.checker._resolved_command_field_constraints(self._info(1), request)
        self.assertEqual((constraints.min_value, constraints.max_value), (-3000, -1000))
        self.assertEqual({value for _, value in self.checker.generate_command_field_violations(
            self._info(1).field, constraints)}, {-3001, -999})

    async def test_max_of_accepts_attribute_first_order(self):
        info = self._info(1)
        bound = ElementTree.fromstring(info.field.constraints.unsupported_expressions[0])
        bound[0][:] = list(reversed(bound[0]))
        constraints = replace(info.field.constraints, unsupported_expressions=[ElementTree.tostring(bound, encoding='unicode')])
        info = replace(info, field=replace(info.field, constraints=constraints))
        resolved = await self.checker._resolved_command_field_constraints(
            info, self.command_class(minPower=-2500, duration=1))
        self.assertEqual((resolved.min_value, resolved.max_value), (-2500, -1000))

    async def test_signed_absolute_bound_matrix(self):
        for lower, upper in ((-5000, -1000), (-5000, 5000), (1000, 5000), (0, 0), (-(1 << 63), (1 << 63) - 1)):
            for field_id in (0, 1):
                with self.subTest(lower=lower, upper=upper, field_id=field_id):
                    self.checker.endpoints_tlv[1][self.cluster.id].update({
                        self.cluster.Attributes.AbsMinPower.attribute_id: lower,
                        self.cluster.Attributes.AbsMaxPower.attribute_id: upper,
                    })
                    constraints = await self.checker._resolved_command_field_constraints(self._info(field_id))
                    self.assertEqual((constraints.min_value, constraints.max_value), (lower, upper))
                    violations = self.checker.generate_command_field_violations(self._info(field_id).field, constraints)
                    expected = {value for value in (lower - 1, upper + 1) if -(1 << 63) <= value < (1 << 63)}
                    self.assertEqual({value for _, value in violations}, expected)

    async def test_bad_absolute_bounds_skip_probes(self):
        for lower, upper in ((None, -1000), (-5000, None), (True, 5000), (-5000, 1.5),
                             (NullValue, 5000), (-1000, -5000), (-(1 << 63) - 1, 0), (0, 1 << 63)):
            for field_id in range(4):
                with self.subTest(lower=lower, upper=upper, field_id=field_id):
                    self.checker.endpoints_tlv[1][self.cluster.id].update({
                        self.cluster.Attributes.AbsMinPower.attribute_id: lower,
                        self.cluster.Attributes.AbsMaxPower.attribute_id: upper,
                    })
                    with patch.object(IDMBaseTest, 'default_controller', new_callable=PropertyMock) as controller:
                        result = await self.checker.check_command_constraint(self._info(field_id))
                        self.assertEqual(result.probed, 0)
                        controller.assert_not_called()

    async def test_max_of_does_not_invent_missing_or_fractional_operands(self):
        for lower in (None, True, 1.5, NullValue):
            with self.subTest(lower=lower):
                self.checker.endpoints_tlv[1][self.cluster.id][self.cluster.Attributes.AbsMinPower.attribute_id] = lower
                with self.assertRaises(spec_parsing.SpecParsingException):
                    await self.checker._resolved_command_field_constraints(self._info(1))

    async def test_max_of_rejects_invalid_present_companion(self):
        for value in (NullValue, True, -3000.5, -5001, -999):
            with self.subTest(value=value), self.assertRaises(spec_parsing.SpecParsingException):
                await self.checker._resolved_command_field_constraints(
                    self._info(1), self.command_class(minPower=value, duration=1))

    async def _probe(self, field_id):
        sent = []
        lower = self.checker.endpoints_tlv[1][self.cluster.id][self.cluster.Attributes.AbsMinPower.attribute_id]
        upper = self.checker.endpoints_tlv[1][self.cluster.id][self.cluster.Attributes.AbsMaxPower.attribute_id]

        async def reject(**kwargs):
            payload = kwargs['payload']
            self.assertIsInstance(payload, self.command_class)
            decoded = TLVReader(payload.ToTLV()).get()['Any']
            self.assertTrue(0 in decoded or 1 in decoded)
            if field_id not in (0, 1):
                for tag in (0, 1):
                    if tag in decoded:
                        self.assertLessEqual(lower, decoded[tag])
                        self.assertLessEqual(decoded[tag], upper)
                if 0 in decoded and 1 in decoded:
                    self.assertLessEqual(decoded[0], decoded[1])
            if field_id != 2:
                self.assertLessEqual(1, decoded[2])
                self.assertLessEqual(decoded[2], 86400)
            if field_id != 3:
                self.assertIn(decoded[3], {int(value) for value in self._info(3).enum_values_by_field[3]})
            sent.append(decoded)
            raise InteractionModelError(Status.ConstraintError)

        controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=reject))
        with patch.object(IDMBaseTest, 'default_controller', new_callable=PropertyMock, return_value=controller), \
                patch.object(IDMBaseTest, 'dut_node_id', new_callable=PropertyMock, return_value=1):
            result = await self.checker.check_command_constraint(self._info(field_id))
        self.assertEqual(result.probed, len(sent))
        self.assertEqual(result.rejected, len(sent))
        self.assertEqual(result.accepted + result.other_error, 0)
        return sent

    async def test_min_power_probe_payloads(self):
        sent = await self._probe(0)
        self.assertEqual({payload[0] for payload in sent}, {-5001, -999})
        self.assertTrue(all(1 not in payload for payload in sent))

    async def test_max_power_probe_payloads(self):
        sent = await self._probe(1)
        self.assertEqual({payload[1] for payload in sent}, {-5001, -999})
        self.assertTrue(all(payload.get(0, -5000) == -5000 for payload in sent))

    async def test_duration_probe_has_valid_companion_power(self):
        sent = await self._probe(2)
        self.assertEqual({payload[2] for payload in sent}, {0, 86401})

    async def test_cause_probe_has_valid_companion_power(self):
        sent = await self._probe(3)
        self.assertTrue(sent)
        legal = self._info(3).enum_values_by_field[3]
        self.assertTrue(all(payload[3] not in legal for payload in sent))

    async def test_duration_and_cause_companions_use_signed_bounds(self):
        for lower, upper in ((-5000, -1000), (-5000, 5000), (1000, 5000), (0, 0)):
            for field_id in (2, 3):
                with self.subTest(lower=lower, upper=upper, field_id=field_id):
                    self.checker.endpoints_tlv[1][self.cluster.id].update({
                        self.cluster.Attributes.AbsMinPower.attribute_id: lower,
                        self.cluster.Attributes.AbsMaxPower.attribute_id: upper,
                    })
                    sent = await self._probe(field_id)
                    self.assertTrue(sent)
                    self.assertTrue(all(payload[0] == lower for payload in sent))

    async def test_non_dem_bounded_sibling_skip_is_unchanged(self):
        cluster = Clusters.OnOff
        command = cluster.Commands.OnWithTimedOff
        xml_cluster = self.xml_clusters[cluster.id]
        xml_command = xml_cluster.accepted_commands[command.command_id]
        info = CommandFieldInfo(
            endpoint_id=1, cluster_id=cluster.id, cluster_name=xml_cluster.name,
            command_id=command.command_id, command_name=xml_command.name,
            command_class=command, cluster_class=cluster,
            field=xml_command.fields[1], all_fields=xml_command.fields, enum_values_by_field={})
        controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=InteractionModelError(Status.ConstraintError)))
        with patch.object(IDMBaseTest, 'default_controller', new_callable=PropertyMock, return_value=controller), \
                patch.object(IDMBaseTest, 'dut_node_id', new_callable=PropertyMock, return_value=1):
            result = await self.checker.check_command_constraint(info)
        controller.SendCommand.assert_not_awaited()
        self.assertEqual(result.probed, 0)
        self.checker.read_single_attribute_check_success.assert_not_awaited()


class TestAudioVolumeConstraints(unittest.IsolatedAsyncioTestCase):

    @classmethod
    def setUpClass(cls):
        clusters, _ = build_xml_clusters(_CHIP_ROOT / 'data_model/1.7/clusters')
        cls.cluster = Clusters.AudioControl
        cls.command_class = cls.cluster.Commands.SetVolume
        cls.xml_cluster = clusters[cls.cluster.id]
        cls.xml_command = cls.xml_cluster.accepted_commands[cls.command_class.command_id]

    def setUp(self):
        self.checker = IDMBaseTest.__new__(IDMBaseTest)
        self.checker.endpoints_tlv = {1: {self.cluster.id: {
            self.cluster.Attributes.MinDeviceVolume.attribute_id: 1,
            self.cluster.Attributes.MaxDeviceVolume.attribute_id: 100,
            self.cluster.Attributes.MaxUserVolume.attribute_id: 100,
        }}}
        self.values = dict(self.checker.endpoints_tlv[1][self.cluster.id])
        self.checker.current_test_info = SimpleNamespace(name='Audio volume constraint regression')
        self.checker.read_single_attribute_check_success = AsyncMock(
            side_effect=lambda **kwargs: self.values[kwargs['attribute'].attribute_id])
        self.checker.record_warning = Mock()
        self.checker.record_accepted_violation = Mock()
        self.info = CommandFieldInfo(
            endpoint_id=1, cluster_id=self.cluster.id, cluster_name=self.xml_cluster.name,
            command_id=self.command_class.command_id, command_name=self.xml_command.name,
            command_class=self.command_class, cluster_class=self.cluster,
            field=self.xml_command.fields[0], all_fields=self.xml_command.fields, enum_values_by_field={})

    async def test_actual_audio_xml_unequal_caps_use_read_values(self):
        self.assertEqual(self.info.field.name, 'NewVolume')
        self.assertEqual(self.checker._command_field_label(self.command_class, 0), 'newVolume')
        self.assertTrue(self.info.field.constraints.unsupported_expressions)
        for device, user in ((80, 40), (40, 80)):
            with self.subTest(device=device, user=user):
                self.values.update({self.cluster.Attributes.MaxDeviceVolume.attribute_id: device,
                                    self.cluster.Attributes.MaxUserVolume.attribute_id: user})
                self.checker.read_single_attribute_check_success.reset_mock()
                constraints = await self.checker._resolved_command_field_constraints(self.info)
                self.assertEqual((constraints.min_value, constraints.max_value), (1, 40))
                self.assertFalse(constraints.unsupported_expressions)
                self.assertTrue(self.info.field.constraints.unsupported_expressions)
                reads = self.checker.read_single_attribute_check_success.await_args_list
                self.assertEqual([read.kwargs['attribute'] for read in reads],
                                 [self.cluster.Attributes.MinDeviceVolume,
                                  self.cluster.Attributes.MaxDeviceVolume, self.cluster.Attributes.MaxUserVolume])
                self.assertTrue(all(read.kwargs['endpoint'] == 1 and read.kwargs['cluster'] is self.cluster
                                    and read.kwargs['assert_on_error'] is False for read in reads))

    async def test_actual_audio_equal_caps(self):
        for value in (1, 40, 65534, 65535):
            with self.subTest(value=value):
                self.values.update({self.cluster.Attributes.MaxDeviceVolume.attribute_id: value,
                                    self.cluster.Attributes.MaxUserVolume.attribute_id: value})
                constraints = await self.checker._resolved_command_field_constraints(self.info)
                self.assertEqual((constraints.min_value, constraints.max_value), (1, value))
                self.assertEqual(constraints.allowed, ['0'])

    async def test_zero_cap_is_not_treated_as_missing(self):
        self.values[self.cluster.Attributes.MinDeviceVolume.attribute_id] = 0
        for device, user in ((0, 40), (40, 0), (0, 0)):
            with self.subTest(device=device, user=user):
                self.values.update({self.cluster.Attributes.MaxDeviceVolume.attribute_id: device,
                                    self.cluster.Attributes.MaxUserVolume.attribute_id: user})
                constraints = await self.checker._resolved_command_field_constraints(self.info)
                self.assertEqual((constraints.min_value, constraints.max_value), (0, 0))
                self.assertEqual({value for _, value in self.checker.generate_command_field_violations(
                    self.info.field, constraints)}, {1})

    async def test_invalid_attribute_values_reject_in_either_operand(self):
        for attribute in (self.cluster.Attributes.MaxDeviceVolume, self.cluster.Attributes.MaxUserVolume):
            for value in (None, NullValue, True, False, 1.0, 1.5, '40', -1, 65536):
                with self.subTest(attribute=attribute.__name__, value=value):
                    self.values.update({self.cluster.Attributes.MaxDeviceVolume.attribute_id: 80,
                                        self.cluster.Attributes.MaxUserVolume.attribute_id: 40,
                                        attribute.attribute_id: value})
                    with self.assertRaises(spec_parsing.SpecParsingException):
                        await self.checker._resolved_command_field_constraints(self.info)
                    self.assertTrue(self.info.field.constraints.unsupported_expressions)

    async def test_missing_optional_attribute_rejects_instead_of_using_other_cap(self):
        del self.checker.endpoints_tlv[1][self.cluster.id][self.cluster.Attributes.MaxUserVolume.attribute_id]
        with self.assertRaises(spec_parsing.SpecParsingException):
            await self.checker._resolved_command_field_constraints(self.info)
        self.checker.record_warning.assert_called_once()
        self.assertNotIn(self.cluster.Attributes.MaxUserVolume,
                         [read.kwargs['attribute'] for read in self.checker.read_single_attribute_check_success.await_args_list])

    async def test_invalid_lower_reference_rejects(self):
        for value in (None, NullValue, True, 1.0, -1, 65536):
            with self.subTest(value=value):
                self.values[self.cluster.Attributes.MinDeviceVolume.attribute_id] = value
                with self.assertRaises(spec_parsing.SpecParsingException):
                    await self.checker._resolved_command_field_constraints(self.info)

    async def test_reversed_bounds_reject(self):
        self.values[self.cluster.Attributes.MinDeviceVolume.attribute_id] = 101
        with self.assertRaises(spec_parsing.SpecParsingException):
            await self.checker._resolved_command_field_constraints(self.info)

    async def test_existing_upper_bound_is_not_weakened(self):
        self.values[self.cluster.Attributes.MaxUserVolume.attribute_id] = 40
        for upper, expected in ((30, 30), (60, 40)):
            with self.subTest(upper=upper):
                constraints = replace(self.info.field.constraints, max_value=upper)
                info = replace(self.info, field=replace(self.info.field, constraints=constraints))
                resolved = await self.checker._resolved_command_field_constraints(info)
                self.assertEqual((resolved.min_value, resolved.max_value), (1, expected))

    async def test_dynamic_references_preserve_stricter_existing_bounds(self):
        self.values[self.cluster.Attributes.MaxUserVolume.attribute_id] = 40
        constraints = replace(
            self.info.field.constraints, min_value=30, max_value=35,
            max_value_ref=spec_parsing.ConstraintReference(attribute='MaxUserVolume'))
        info = replace(self.info, field=replace(self.info.field, constraints=constraints))
        resolved = await self.checker._resolved_command_field_constraints(info)
        self.assertEqual((resolved.min_value, resolved.max_value), (30, 35))
        self.assertEqual((constraints.min_value, constraints.max_value), (30, 35))
        self.assertTrue(constraints.unsupported_expressions)

    async def test_both_expression_bounds_and_existing_lower_are_combined(self):
        for lower, expected in ((5, 20), (30, 30)):
            with self.subTest(lower=lower):
                constraints = replace(self.info.field.constraints, min_value=lower, min_value_ref=None,
                                      unsupported_expressions=[
                                          '<from><maxOf><literal value="10"/><literal value="20"/></maxOf></from>',
                                          self.info.field.constraints.unsupported_expressions[0]])
                resolved = await self.checker._resolved_command_field_constraints(
                    replace(self.info, field=replace(self.info.field, constraints=constraints)))
                self.assertEqual((resolved.min_value, resolved.max_value), (expected, 100))

    async def test_max_of_attribute_operands_uses_larger_cap(self):
        self.values[self.cluster.Attributes.MaxUserVolume.attribute_id] = 40
        expression = self.info.field.constraints.unsupported_expressions[0].replace('minOf', 'maxOf')
        constraints = replace(self.info.field.constraints, unsupported_expressions=[expression])
        resolved = await self.checker._resolved_command_field_constraints(
            replace(self.info, field=replace(self.info.field, constraints=constraints)))
        self.assertEqual((resolved.min_value, resolved.max_value), (1, 100))

    async def test_integer_literals_mix_with_attribute_reads(self):
        for literal in ('40', '0x28'):
            for attribute_first in (False, True):
                with self.subTest(literal=literal, attribute_first=attribute_first):
                    operands = ['<attribute name="MaxDeviceVolume"/>', f'<literal value="{literal}"/>']
                    if not attribute_first:
                        operands.reverse()
                    expression = '<to><minOf>' + ''.join(operands) + '</minOf></to>'
                    constraints = replace(self.info.field.constraints, unsupported_expressions=[expression])
                    self.checker.read_single_attribute_check_success.reset_mock()
                    resolved = await self.checker._resolved_command_field_constraints(
                        replace(self.info, field=replace(self.info.field, constraints=constraints)))
                    self.assertEqual((resolved.min_value, resolved.max_value), (1, 40))
                    self.assertEqual(self.checker.read_single_attribute_check_success.await_count, 2)

    async def test_signed_integer_attributes_and_literals(self):
        for operation, expected in (('minOf', -40), ('maxOf', -20)):
            with self.subTest(operation=operation):
                attribute = self.cluster.Attributes.MinCorrection
                self.checker.endpoints_tlv[1][self.cluster.id][attribute.attribute_id] = 100
                self.values[attribute.attribute_id] = -40
                expression = f'<to><{operation}><attribute name="MinCorrection"/><literal value="-20"/></{operation}></to>'
                constraints = spec_parsing.Constraints(min_value=-100, unsupported_expressions=[expression])
                field = replace(self.info.field, type_info='int16', constraints=constraints)
                resolved = await self.checker._resolved_command_field_constraints(replace(self.info, field=field))
                self.assertEqual((resolved.min_value, resolved.max_value), (-100, expected))
                self.assertEqual({value for _, value in self.checker.generate_command_field_violations(field, resolved)},
                                 {-101, expected + 1})

    async def test_signed_integer_reference_values_are_strict(self):
        attribute = self.cluster.Attributes.MinCorrection
        self.checker.endpoints_tlv[1][self.cluster.id][attribute.attribute_id] = 0
        expression = '<to><minOf><attribute name="MinCorrection"/><literal value="0"/></minOf></to>'
        constraints = spec_parsing.Constraints(min_value=-32768, unsupported_expressions=[expression])
        info = replace(self.info, field=replace(self.info.field, type_info='int16', constraints=constraints))
        for value in (None, NullValue, True, -1.0, -32769, 32768):
            with self.subTest(value=value):
                self.values[attribute.attribute_id] = value
                with self.assertRaises(spec_parsing.SpecParsingException):
                    await self.checker._resolved_command_field_constraints(info)

    async def test_actual_audio_probe_executes_generated_payload(self):
        self.values[self.cluster.Attributes.MaxDeviceVolume.attribute_id] = 80
        self.values[self.cluster.Attributes.MaxUserVolume.attribute_id] = 40

        async def reject(**kwargs):
            payload = kwargs['payload']
            self.assertIsInstance(payload, self.command_class)
            self.assertEqual(payload.newVolume, 41)
            self.assertEqual(TLVReader(payload.ToTLV()).get()['Any'], {0: 41})
            return SimpleNamespace(status=Status.ConstraintError)

        controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=reject))
        with patch.object(IDMBaseTest, 'default_controller', new_callable=PropertyMock, return_value=controller), \
                patch.object(IDMBaseTest, 'dut_node_id', new_callable=PropertyMock, return_value=1):
            result = await self.checker.check_command_constraint(self.info)
        controller.SendCommand.assert_awaited_once()
        self.assertEqual((result.probed, result.rejected, result.accepted, result.other_error), (1, 1, 0, 0))

    async def test_invalid_cap_aborts_probe_before_send(self):
        self.values[self.cluster.Attributes.MaxUserVolume.attribute_id] = NullValue
        controller = SimpleNamespace(SendCommand=AsyncMock())
        with patch.object(IDMBaseTest, 'default_controller', new_callable=PropertyMock, return_value=controller), \
                self.assertRaises(spec_parsing.SpecParsingException):
            await self.checker.check_command_constraint(self.info)
        controller.SendCommand.assert_not_awaited()


class TestCodegenEnumValues(unittest.TestCase):

    def test_real_member_with_unknown_prefix_is_defined(self):
        # A global enum: the spec XML contributes no values, so codegen is the only source.
        end_reason = Clusters.Globals.Enums.WebRTCEndReasonEnum
        self.assertIn(end_reason.kUnknownReason.value, _codegen_enum_values(end_reason))
        self.assertNotIn(end_reason.kUnknownReason.value,
                         [int(value) for value in undefined_enum_values(end_reason, frozenset())])

    def test_real_member_named_unknown_is_legal_filler(self):
        ac_type = Clusters.Thermostat.Enums.ACTypeEnum
        self.assertEqual(smallest_legal_enum_value(ac_type, frozenset()), ac_type.kUnknown)

    def test_sentinel_is_not_defined(self):
        end_reason = Clusters.Globals.Enums.WebRTCEndReasonEnum
        self.assertNotIn(end_reason.kUnknownEnumValue.value, _codegen_enum_values(end_reason))

    def test_grafted_placeholders_are_not_defined(self):
        # StepModeEnum defines {1, 3}; 0 is its kUnknownEnumValue, so probing the gap at 2
        # grafts a kUnknownPlaceholder member onto the enum.
        step_mode = Clusters.ColorControl.Enums.StepModeEnum
        probed = [int(value) for value in undefined_enum_values(step_mode, frozenset())]
        self.assertIn(2, probed)
        self.assertTrue(any(member.name.startswith('kUnknownPlaceholder') for member in step_mode))
        self.assertEqual(_codegen_enum_values(step_mode), frozenset({1, 3}))


class TestSpecEnumValues(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)

    def test_obsolete_items_are_not_legal(self):
        joint_fabric = self.xml_clusters[Clusters.JointFabricDatastore.id]
        # ProxyView (2) is obsolete, so it is neither legal nor a filler candidate.
        self.assertEqual(spec_enum_values(joint_fabric, 'DatastoreAccessControlEntryPrivilegeEnum'),
                         frozenset({1, 3, 4, 5}))

    def test_obsolete_values_codegen_keeps_are_not_probed(self):
        fan_mode = Clusters.FanControl.Enums.FanModeEnum
        spec_values = spec_enum_values(self.xml_clusters[Clusters.FanControl.id], 'FanModeEnum')
        self.assertNotIn(fan_mode.kOn.value, spec_values)
        probed = [int(value) for value in undefined_enum_values(fan_mode, spec_values)]
        self.assertNotIn(fan_mode.kOn.value, probed)
        self.assertNotIn(fan_mode.kSmart.value, probed)

    def test_filler_comes_from_spec_values(self):
        # The DUT's declared spec decides what is legal; codegen's 0 is not consulted.
        ac_type = Clusters.Thermostat.Enums.ACTypeEnum
        self.assertEqual(smallest_legal_enum_value(ac_type, frozenset({2, 3})), 2)


class TestUndefinedEnumValueEncoding(unittest.TestCase):

    def test_plain_int_encodes_as_sentinel(self):
        # The encoder converts a field value to its generated enum type, which maps an
        # undefined plain int to kUnknownEnumValue: the reason probe values are grafted.
        effect = Clusters.Identify.Enums.EffectIdentifierEnum
        # 0x80 is not a value undefined_enum_values probes, so nothing has grafted it.
        self.assertIs(effect(0x80), effect.kUnknownEnumValue)
        command = Clusters.Identify.Commands.TriggerEffect(effectIdentifier=0x80, effectVariant=0)
        self.assertEqual(TLVReader(command.ToTLV()).get()['Any'][0], effect.kUnknownEnumValue.value)

    def test_probe_value_survives_encoding(self):
        effect = Clusters.Identify.Enums.EffectIdentifierEnum
        for value in undefined_enum_values(effect, frozenset()):
            command = Clusters.Identify.Commands.TriggerEffect(effectIdentifier=value, effectVariant=0)
            self.assertEqual(TLVReader(command.ToTLV()).get()['Any'][0], int(value))


class TestEnumInType(unittest.TestCase):

    def test_nullable_optional_enum(self):
        attribute = Clusters.Thermostat.Attributes.ACType
        self.assertIs(enum_in_type(attribute.attribute_type.Type), Clusters.Thermostat.Enums.ACTypeEnum)
        fields = {field.Label: field for field in Clusters.WebRTCTransportProvider.Commands.EndSession.descriptor.Fields}
        self.assertIs(enum_in_type(fields['reason'].Type), Clusters.Globals.Enums.WebRTCEndReasonEnum)

    def test_list_of_enums_is_not_an_enum(self):
        fields = Clusters.CameraAvStreamManagement.Commands.SetStreamPriorities.descriptor.Fields
        self.assertIsNone(enum_in_type(fields[0].Type))

    def test_non_enum(self):
        self.assertIsNone(enum_in_type(Clusters.Identify.Attributes.IdentifyTime.attribute_type.Type))


class TestRecordAcceptedViolation(unittest.TestCase):

    @staticmethod
    def _severities_recorded(spec_version: int | None) -> list[str]:
        """Run the real era logic against a stand-in composition and collect what gets recorded."""
        basic_information = {}
        if spec_version is not None:
            basic_information[Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id] = spec_version
        recorded: list[str] = []
        test = SimpleNamespace(
            ROOT_NODE_ENDPOINT_ID=IDMBaseTest.ROOT_NODE_ENDPOINT_ID,
            endpoints_tlv={IDMBaseTest.ROOT_NODE_ENDPOINT_ID: {Clusters.BasicInformation.id: basic_information}},
            current_test_info=SimpleNamespace(name='test_TC_IDM_9_1'),
            record_error=lambda **kwargs: recorded.append('error'),
            record_warning=lambda **kwargs: recorded.append('warning'),
        )
        test.dut_spec_version = MethodType(IDMBaseTest.dut_spec_version, test)
        test.enforces_constraints_strictly = MethodType(IDMBaseTest.enforces_constraints_strictly, test)
        IDMBaseTest.record_accepted_violation(test, location=UnknownProblemLocation(), problem='accepted')
        return recorded

    def test_strict_dut_records_error(self):
        self.assertEqual(self._severities_recorded(SPEC_VERSION_1_7), ['error'])

    def test_pre_1_7_dut_records_warning(self):
        self.assertEqual(self._severities_recorded(SPEC_VERSION_1_7 - 1), ['warning'])

    def test_dut_without_spec_version_records_warning(self):
        self.assertEqual(self._severities_recorded(None), ['warning'])


class TestFeatureGatedCommandFields(unittest.TestCase):
    """A field the DUT ignores for lack of a feature accepts any value, so it must not be probed."""

    @classmethod
    def setUpClass(cls):
        cls.xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)

    def _discovered(self, cluster, feature_map: int, commands: list) -> set[tuple[str, str]]:
        """Run the real discovery against a one-cluster stand-in composition."""
        test = SimpleNamespace(
            endpoints_tlv={1: {cluster.id: {
                GlobalAttributeIds.FEATURE_MAP_ID: feature_map,
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [command.command_id for command in commands],
            }}},
            xml_clusters=self.xml_clusters,
            _command_field_enum_type=IDMBaseTest._command_field_enum_type,
        )
        return {(info.command_name, info.field.name) for info in IDMBaseTest.discover_constrained_command_fields(test)}

    def test_field_whose_conformance_names_the_feature(self):
        # Step.Speed is [SP] in the data model, so conformance evaluation alone decides it.
        feature = Clusters.ClosureDimension.Bitmaps.Feature
        commands = [Clusters.ClosureDimension.Commands.Step]
        without_speed = self._discovered(Clusters.ClosureDimension, feature.kPositioning, commands)
        self.assertIn(('Step', 'NumberOfSteps'), without_speed)
        self.assertNotIn(('Step', 'Speed'), without_speed)
        self.assertIn(('Step', 'Speed'),
                      self._discovered(Clusters.ClosureDimension, feature.kPositioning | feature.kSpeed, commands))

    def test_closure_dimension_fields_gated_by_spec_text(self):
        # SetTarget/GroupedSetTarget/GroupedStep Speed are plain optional in the data model.
        feature = Clusters.ClosureDimension.Bitmaps.Feature
        commands = [Clusters.ClosureDimension.Commands.SetTarget, Clusters.ClosureDimension.Commands.GroupedSetTarget,
                    Clusters.ClosureDimension.Commands.GroupedStep]
        speed_fields = {('SetTarget', 'Speed'), ('GroupedSetTarget', 'Speed'), ('GroupedStep', 'Speed')}
        without_speed = self._discovered(Clusters.ClosureDimension, feature.kPositioning, commands)
        self.assertFalse(speed_fields & without_speed)
        self.assertIn(('GroupedStep', 'NumberOfSteps'), without_speed)
        self.assertLessEqual(speed_fields,
                             self._discovered(Clusters.ClosureDimension, feature.kPositioning | feature.kSpeed, commands))

    def test_closure_control_fields_gated_by_spec_text(self):
        feature = Clusters.ClosureControl.Bitmaps.Feature
        commands = [Clusters.ClosureControl.Commands.MoveTo, Clusters.ClosureControl.Commands.GroupedMoveTo]
        gated_fields = {('MoveTo', 'Position'), ('MoveTo', 'Speed'), ('GroupedMoveTo', 'Position'), ('GroupedMoveTo', 'Speed')}
        self.assertFalse(gated_fields & self._discovered(Clusters.ClosureControl, feature.kMotionLatching, commands))
        self.assertEqual(gated_fields, self._discovered(
            Clusters.ClosureControl, feature.kPositioning | feature.kMotionLatching | feature.kSpeed, commands))

    def test_ungated_field_is_kept(self):
        # ViewGroup has no side effects, so it is unlikely to be added to COMMAND_CONSTRAINT_DENIED_COMMANDS.
        commands = [Clusters.Groups.Commands.ViewGroup]
        self.assertIn(('ViewGroup', 'GroupID'), self._discovered(Clusters.Groups, 0, commands))


if __name__ == "__main__":
    unittest.main()
