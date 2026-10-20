import os
import sys
import unittest

from matter.clusters import Objects as Clusters
from matter.testing.conformance import mandatory
from matter.testing.spec_parsing import Constraints, SpecParsingException, XmlDataTypeComponent

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


if __name__ == '__main__':
    unittest.main()
