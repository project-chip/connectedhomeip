import os
import sys
import unittest
from dataclasses import replace
from pathlib import Path
from unittest.mock import AsyncMock
from xml.etree import ElementTree

from matter.clusters import Objects as Clusters
from matter.testing.conformance import mandatory
from matter.testing.spec_parsing import Constraints, SpecParsingException, XmlDataTypeComponent, build_xml_clusters

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from support_modules.idm_support import CommandFieldInfo, IDMBaseTest  # noqa: E402 isort:skip


class TestUnsupportedConstraints(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.checker = IDMBaseTest.__new__(IDMBaseTest)
        self.constraints = Constraints(max_value=10, unsupported_expressions=['maxOf(Lower, Floor)'])
        self.field = XmlDataTypeComponent(
            value=0, name='Value', conformance=mandatory(), type_info='int16', constraints=self.constraints)

    def test_attribute_violation_rejects_unsupported(self):
        with self.assertRaises(SpecParsingException):
            self.checker.generate_constraint_violations(None, self.constraints)

    def test_command_violation_rejects_unsupported(self):
        with self.assertRaises(SpecParsingException):
            self.checker.generate_command_field_violations(self.field, self.constraints)

    def test_sibling_value_rejects_unsupported(self):
        with self.assertRaises(SpecParsingException):
            self.checker._generate_valid_command_field_value(self.field)

    def test_enum_sibling_rejects_unsupported(self):
        with self.assertRaises(SpecParsingException):
            self.checker._generate_valid_command_field_value(
                self.field, Clusters.ColorControl.Enums.StepModeEnum, frozenset({1, 3}))

    def test_supported_enum_sibling_remains_available(self):
        self.field.constraints = None
        value = self.checker._generate_valid_command_field_value(
            self.field, Clusters.ColorControl.Enums.StepModeEnum, frozenset({1, 3}))
        self.assertEqual(value, Clusters.ColorControl.Enums.StepModeEnum.kUp)

    async def test_attribute_check_rejects_before_io(self):
        with self.assertRaises(SpecParsingException):
            await self.checker.check_attribute_constraint(None, self.constraints)

    async def test_command_resolution_rejects_before_io(self):
        info = CommandFieldInfo(endpoint_id=0, cluster_id=0, cluster_name='Synthetic', command_id=0,
                                command_name='Synthetic', command_class=None, cluster_class=None,
                                field=self.field, all_fields={0: self.field}, enum_values_by_field={})
        with self.assertRaises(SpecParsingException):
            await self.checker._resolved_command_field_constraints(info)

    async def test_absent_command_constraints_remain_available(self):
        self.field.constraints = None
        info = CommandFieldInfo(endpoint_id=0, cluster_id=0, cluster_name='Synthetic', command_id=0,
                                command_name='Synthetic', command_class=None, cluster_class=None,
                                field=self.field, all_fields={0: self.field}, enum_values_by_field={})
        self.assertEqual(await self.checker._resolved_command_field_constraints(info), Constraints())

    def test_supported_numeric_violations_remain_available(self):
        constraints = Constraints(min_value=0, max_value=10)
        constraints.require_supported()
        violations = self.checker.generate_command_field_violations(self.field, constraints)
        self.assertEqual({value for _, value in violations}, {-1, 11})


class TestUnsupportedDEMConstraints(unittest.IsolatedAsyncioTestCase):

    @classmethod
    def setUpClass(cls):
        root = Path(__file__).resolve().parents[3]
        clusters, _ = build_xml_clusters(root / 'data_model/1.7/clusters')
        cls.cluster = Clusters.DeviceEnergyManagement
        cls.command = cls.cluster.Commands.PowerRangeAdjustRequest
        cls.xml_command = clusters[cls.cluster.id].accepted_commands[cls.command.command_id]

    def setUp(self):
        self.checker = IDMBaseTest.__new__(IDMBaseTest)
        self.checker.resolve_dynamic_constraint = AsyncMock(side_effect=AssertionError('Unexpected attribute read'))
        self.info = CommandFieldInfo(
            endpoint_id=1, cluster_id=self.cluster.id, cluster_name='Device Energy Management',
            command_id=self.command.command_id, command_name=self.xml_command.name,
            command_class=self.command, cluster_class=self.cluster, field=self.xml_command.fields[1],
            all_fields=self.xml_command.fields, enum_values_by_field={})
        self.expression = self.info.field.constraints.unsupported_expressions[0]

    async def _assert_unsupported(self, info):
        original = list(info.field.constraints.unsupported_expressions)
        with self.assertRaises(SpecParsingException):
            await self.checker._resolved_command_field_constraints(info)
        self.assertEqual(info.field.constraints.unsupported_expressions, original)
        self.checker.resolve_dynamic_constraint.assert_not_awaited()

    async def test_unsupported_dem_grammar_rejects_before_io(self):
        for mutation in ('operator', 'attribute', 'field', 'extra_operand', 'missing_operand',
                         'nested_operand', 'bound', 'bound_attribute', 'operator_attribute',
                         'operand_attribute', 'operand_text', 'bound_text'):
            with self.subTest(mutation=mutation):
                bound = ElementTree.fromstring(self.expression)
                operation = bound[0]
                if mutation == 'operator':
                    operation.tag = 'sumOf'
                elif mutation == 'attribute':
                    operation.find('attribute').set('name', 'AbsMaxPower')
                elif mutation == 'field':
                    operation.find('field').set('name', 'OtherPower')
                elif mutation == 'extra_operand':
                    ElementTree.SubElement(operation, 'literal', {'value': '0'})
                elif mutation == 'missing_operand':
                    operation.remove(operation[0])
                elif mutation == 'nested_operand':
                    ElementTree.SubElement(operation[0], 'maxOf')
                elif mutation == 'bound':
                    bound.tag = 'to'
                elif mutation == 'bound_attribute':
                    bound.set('value', '0')
                elif mutation == 'operator_attribute':
                    operation.set('value', '0')
                elif mutation == 'operand_attribute':
                    operation[0].set('value', '0')
                elif mutation == 'operand_text':
                    operation[0].text = '0'
                else:
                    bound.text = '0'
                constraints = replace(self.info.field.constraints,
                                      unsupported_expressions=[ElementTree.tostring(bound, encoding='unicode')])
                await self._assert_unsupported(replace(self.info, field=replace(self.info.field, constraints=constraints)))

    async def test_malformed_and_multiple_expressions_stay_unsupported(self):
        for expressions in (['maxOf(AbsMinPower, MinPower)'], ['<from>'], [self.expression, self.expression],
                            [self.expression, '<from><unknown/></from>'], [' ' * 1025 + self.expression]):
            with self.subTest(expressions=expressions):
                constraints = replace(self.info.field.constraints, unsupported_expressions=expressions)
                await self._assert_unsupported(replace(self.info, field=replace(self.info.field, constraints=constraints)))

    async def test_same_expression_in_non_dem_context_stays_unsupported(self):
        for changes in ({'cluster_id': Clusters.OnOff.id}, {'command_id': 0},
                        {'cluster_class': Clusters.OnOff}, {'command_class': Clusters.OnOff.Commands.On},
                        {'field': replace(self.info.field, value=0)},
                        {'field': replace(self.info.field, name='OtherPower')}):
            with self.subTest(changes=changes):
                await self._assert_unsupported(replace(self.info, **changes))


class TestUnsupportedNumericConstraints(unittest.IsolatedAsyncioTestCase):

    @classmethod
    def setUpClass(cls):
        root = Path(__file__).resolve().parents[3]
        clusters, _ = build_xml_clusters(root / 'data_model/1.7/clusters')
        cls.cluster = Clusters.AudioControl
        cls.command = cls.cluster.Commands.SetVolume
        cls.xml_command = clusters[cls.cluster.id].accepted_commands[cls.command.command_id]

    def setUp(self):
        self.checker = IDMBaseTest.__new__(IDMBaseTest)
        self.checker.resolve_dynamic_constraint = AsyncMock(side_effect=AssertionError('Unexpected attribute read'))
        self.info = CommandFieldInfo(
            endpoint_id=1, cluster_id=self.cluster.id, cluster_name='Audio Control',
            command_id=self.command.command_id, command_name=self.xml_command.name,
            command_class=self.command, cluster_class=self.cluster, field=self.xml_command.fields[0],
            all_fields=self.xml_command.fields, enum_values_by_field={})
        self.expression = self.info.field.constraints.unsupported_expressions[0]

    async def _assert_unsupported(self, expressions, datatype='uint16'):
        constraints = replace(self.info.field.constraints, unsupported_expressions=expressions)
        info = replace(self.info, field=replace(self.info.field, constraints=constraints, type_info=datatype))
        with self.assertRaises(SpecParsingException):
            await self.checker._resolved_command_field_constraints(info)
        self.assertEqual(constraints.unsupported_expressions, expressions)
        self.checker.resolve_dynamic_constraint.assert_not_awaited()

    async def test_unknown_numeric_grammar_rejects_before_io(self):
        for mutation in ('operator', 'unknown_attribute', 'field', 'extra_operand', 'missing_operand',
                         'nested_operand', 'bound', 'bound_attribute', 'operator_attribute',
                         'operand_attribute', 'operand_text', 'bound_text', 'tail', 'noninteger_attribute'):
            with self.subTest(mutation=mutation):
                bound = ElementTree.fromstring(self.expression)
                operation = bound[0]
                if mutation == 'operator':
                    operation.tag = 'sumOf'
                elif mutation == 'unknown_attribute':
                    operation[1].set('name', 'UnknownVolume')
                elif mutation == 'field':
                    operation[1].tag = 'field'
                elif mutation == 'extra_operand':
                    ElementTree.SubElement(operation, 'literal', {'value': '0'})
                elif mutation == 'missing_operand':
                    operation.remove(operation[0])
                elif mutation == 'nested_operand':
                    ElementTree.SubElement(operation[0], 'minOf')
                elif mutation == 'bound':
                    bound.tag = 'max'
                elif mutation == 'bound_attribute':
                    bound.set('value', '0')
                elif mutation == 'operator_attribute':
                    operation.set('value', '0')
                elif mutation == 'operand_attribute':
                    operation[0].set('value', '0')
                elif mutation == 'operand_text':
                    operation[0].text = '0'
                elif mutation == 'bound_text':
                    bound.text = '0'
                elif mutation == 'tail':
                    operation[0].tail = '0'
                else:
                    operation[0].set('name', 'Muted')
                await self._assert_unsupported([ElementTree.tostring(bound, encoding='unicode')])

    async def test_malformed_oversized_and_duplicate_bounds_reject_before_io(self):
        for expressions in (['minOf(MaxDeviceVolume, MaxUserVolume)'], ['<to>'],
                            [self.expression, self.expression], [self.expression, '<from><unknown/></from>'],
                            [self.expression] * 3, [' ' * 1025 + self.expression],
                            ['<!DOCTYPE to [<!ENTITY value "1">]>' + self.expression]):
            with self.subTest(expressions=expressions):
                await self._assert_unsupported(expressions)

    async def test_invalid_literals_reject_before_io_in_either_operand(self):
        for value in ('', 'NULL', 'true', '1.0', '1.5', 'nan', 'inf', '1 + 2', '-1', '65536', '0x10000'):
            for literal_first in (False, True):
                with self.subTest(value=value, literal_first=literal_first):
                    operands = ['<attribute name="MaxDeviceVolume"/>', f'<literal value="{value}"/>']
                    if literal_first:
                        operands.reverse()
                    await self._assert_unsupported(['<to><minOf>' + ''.join(operands) + '</minOf></to>'])

    async def test_signed_literal_type_limits_reject_before_io(self):
        for value in ('-32769', '32768', '-0x8001', '0x8000'):
            with self.subTest(value=value):
                expression = f'<to><minOf><literal value="{value}"/><literal value="0"/></minOf></to>'
                await self._assert_unsupported([expression], datatype='int16')

    async def test_nonnumeric_target_types_remain_unsupported(self):
        for datatype in ('bool', 'string', 'octstr', 'UnmutePolicyEnum', 'list[uint16]'):
            with self.subTest(datatype=datatype):
                await self._assert_unsupported([self.expression], datatype=datatype)


if __name__ == '__main__':
    unittest.main()
