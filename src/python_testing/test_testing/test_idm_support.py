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
from copy import deepcopy
from dataclasses import replace
from pathlib import Path
from types import MethodType, SimpleNamespace
from unittest.mock import AsyncMock, Mock

from mobly import signals

import matter.clusters as Clusters
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.problem_notices import UnknownProblemLocation
from matter.testing.spec_parsing import ConstraintReference, Constraints, PrebuiltDataModelDirectory, build_xml_clusters
from matter.tlv import TLVReader

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.idm_support import (_NUMERIC_TYPE_RANGES, SPEC_VERSION_1_7, CommandFieldInfo, IDMBaseTest,  # noqa: E402
                                         _codegen_enum_values, enum_in_type, smallest_legal_enum_value, spec_enum_values,
                                         undefined_enum_values)


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


class TestValidCommandFieldValues(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)

    def setUp(self):
        self.checker = SimpleNamespace(_allowed_numeric_values=IDMBaseTest._allowed_numeric_values,
                                       _allowed_lengths=IDMBaseTest._allowed_lengths)
        self.field = self.xml_clusters[Clusters.ColorControl.id].accepted_commands[
            Clusters.ColorControl.Commands.MoveToHue.command_id].fields[0]

    def _value(self, constraints, datatype='uint8', **kwargs):
        field = replace(self.field, constraints=constraints, type_info=datatype)
        return IDMBaseTest._generate_valid_command_field_value(self.checker, field, **kwargs)

    def test_real_max_only_numeric_fields(self):
        for cluster, command, field_id, datatype, maximum in (
                (Clusters.ColorControl, Clusters.ColorControl.Commands.MoveToHue, 0, 'uint8', 254),
                (Clusters.ColorControl, Clusters.ColorControl.Commands.MoveToHue, 2, 'uint16', 65534),
                (Clusters.GeneralDiagnostics, Clusters.GeneralDiagnostics.Commands.PayloadTestRequest, 2, 'uint16', 2048),
                (Clusters.ScenesManagement, Clusters.ScenesManagement.Commands.AddScene, 1, 'uint8', 254),
                (Clusters.ScenesManagement, Clusters.ScenesManagement.Commands.AddScene, 2, 'uint32', 60000000)):
            with self.subTest(cluster=cluster.id, command=command.command_id, field=field_id):
                field = self.xml_clusters[cluster.id].accepted_commands[command.command_id].fields[field_id]
                self.assertEqual(field.type_info, datatype)
                self.assertFalse(field.is_optional)
                self.assertEqual(field.constraints, Constraints(max_value=maximum))
                self.assertEqual(IDMBaseTest._generate_valid_command_field_value(self.checker, field), 0)

    def test_numeric_type_boundaries(self):
        for datatype, (minimum, maximum) in _NUMERIC_TYPE_RANGES.items():
            for bound in (0, 1, maximum):
                for nullable in (False, True):
                    with self.subTest(datatype=datatype, maximum=bound, nullable=nullable):
                        field = replace(self.field, type_info=datatype, is_nullable=nullable,
                                        constraints=Constraints(max_value=bound))
                        value = IDMBaseTest._generate_valid_command_field_value(self.checker, field)
                        self.assertEqual(value, 0)
                        self.assertLessEqual(minimum, value)
                        self.assertLessEqual(value, maximum)

    def test_existing_minimum_negative_maximum_and_allowed_selection(self):
        for datatype, constraints, expected in (
                ('uint8', Constraints(min_value=5, max_value=10), 5),
                ('int8', Constraints(min_value=-10, max_value=-2), -10),
                ('int8', Constraints(max_value=-1), -1),
                ('uint64', Constraints(min_value=2**64 - 2, max_value=2**64 - 1), 2**64 - 2),
                ('int64', Constraints(min_value=-(2**63), max_value=2**63 - 1), -(2**63)),
                ('uint8', Constraints(min_value=0, max_value=0), 0),
                ('uint8', Constraints(max_value=20, allowed=['7']), 7)):
            with self.subTest(datatype=datatype, constraints=constraints):
                self.assertEqual(self._value(constraints, datatype), expected)
        self.assertIn(self._value(Constraints(max_value=20, allowed=['0x02', '7'])), {2, 7})

    def test_unresolved_references_on_real_fields(self):
        for cluster, command, field_ids in (
                (Clusters.ColorControl, Clusters.ColorControl.Commands.MoveToHue, (0, 2)),
                (Clusters.GeneralDiagnostics, Clusters.GeneralDiagnostics.Commands.PayloadTestRequest, (2,)),
                (Clusters.ScenesManagement, Clusters.ScenesManagement.Commands.AddScene, (1, 2))):
            fields = self.xml_clusters[cluster.id].accepted_commands[command.command_id].fields
            for field_id in field_ids:
                for reference in ('min_value_ref', 'max_value_ref'):
                    with self.subTest(cluster=cluster.id, field=field_id, reference=reference):
                        field = fields[field_id]
                        constraints = replace(field.constraints, **{
                            reference: ConstraintReference(attribute='MinDeviceVolume')})
                        self.assertIsNone(IDMBaseTest._generate_valid_command_field_value(
                            self.checker, replace(field, constraints=constraints)))

    def test_other_constraints_do_not_enable_zero_fallback(self):
        reference = ConstraintReference(attribute='MinDeviceVolume')
        for other in (Constraints(min_length=1), Constraints(max_length=2), Constraints(min_count=1),
                      Constraints(max_count=2), Constraints(min_count_ref=reference), Constraints(max_count_ref=reference),
                      Constraints(allowed=[]), Constraints(allowed=['unresolved']),
                      Constraints(min_value_ref=reference), Constraints(max_value_ref=reference)):
            with self.subTest(constraints=other):
                self.assertIsNone(self._value(replace(other, max_value=254)))
        self.assertIsNone(self._value(Constraints(max_value=float('nan'))))

    def test_unsupported_types_and_unconstrained_defaults(self):
        for datatype in ('group-id', 'TLSCAID', 'TLSCCDID', 'enum8', 'bitmap8', 'list', 'UnknownStruct',
                         'single', 'double', 'bool', 'int', 'uint', None):
            with self.subTest(datatype=datatype):
                self.assertIsNone(self._value(Constraints(max_value=254), datatype))
        for constraints in (None, Constraints()):
            self.assertIsNone(self._value(constraints))
        command = Clusters.ColorControl.Commands.MoveToHue()
        self.assertEqual(command.hue, 0)
        self.assertEqual(command.transitionTime, 0)

    def test_enum_and_string_selection_is_unchanged(self):
        enum_type = Clusters.ColorControl.Enums.StepModeEnum
        self.assertEqual(self._value(None, enum_type=enum_type, enum_values=frozenset({1, 3})), 1)
        self.assertEqual(self._value(Constraints(min_length=2), 'string'), 'aa')
        self.assertEqual(self._value(Constraints(min_length=2), 'octstr'), b'\x00\x00')


class TestValidCommandSiblingSetup(unittest.IsolatedAsyncioTestCase):

    @classmethod
    def setUpClass(cls):
        cls.xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)

    def setUp(self):
        self.checker = SimpleNamespace(
            default_controller=SimpleNamespace(SendCommand=AsyncMock(return_value=None)),
            dut_node_id=1, current_test_info=SimpleNamespace(name='test_TC_IDM_9_1'),
            record_accepted_violation=Mock(), record_warning=Mock())
        for name in ('_allowed_numeric_values', '_allowed_lengths', '_command_field_label', '_command_field_enum_type'):
            setattr(self.checker, name, getattr(IDMBaseTest, name))
        for name in ('_generate_valid_command_field_value', '_resolved_command_field_constraints',
                     'generate_command_field_violations'):
            setattr(self.checker, name, MethodType(getattr(IDMBaseTest, name), self.checker))

    def _info(self, target=0):
        cluster = Clusters.ColorControl
        command = cluster.Commands.MoveToHue
        xml_cluster = self.xml_clusters[cluster.id]
        fields = xml_cluster.accepted_commands[command.command_id].fields
        return CommandFieldInfo(
            endpoint_id=1, cluster_id=cluster.id, cluster_name=xml_cluster.name,
            command_id=command.command_id, command_name='MoveToHue', command_class=command,
            cluster_class=cluster, field=fields[target], all_fields=dict(fields),
            enum_values_by_field={field_id: spec_enum_values(xml_cluster, field.type_info)
                                  for field_id, field in fields.items()})

    def _prepare_tc_idm_9_1(self):
        self.checker.ROOT_NODE_ENDPOINT_ID = 0
        self.checker.xml_clusters = deepcopy(self.xml_clusters)
        self.checker.endpoints_tlv = {
            0: {Clusters.BasicInformation.id: {
                Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id: SPEC_VERSION_1_7,
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}},
            1: {Clusters.ColorControl.id: {
                GlobalAttributeIds.FEATURE_MAP_ID: Clusters.ColorControl.Bitmaps.Feature.kHueAndSaturation,
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [Clusters.ColorControl.Commands.MoveToHue.command_id],
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}}}
        self.checker.matter_test_config = SimpleNamespace(endpoint=1)
        self.checker.setup_class_helper = AsyncMock()
        self.checker.build_spec_xmls = Mock()
        self.checker.step = Mock()
        self.checker.resolve_dynamic_constraint = AsyncMock(return_value=None)
        for name in ('check_command_constraint', 'discover_constrained_command_fields', 'dut_spec_version',
                     'enforces_constraints_strictly', 'enforce_constraint_policy', 'checkable_attributes'):
            setattr(self.checker, name, MethodType(getattr(IDMBaseTest, name), self.checker))

    async def test_real_consumer_encodes_only_the_target_violation(self):
        for target, sibling in ((0, 2), (2, 0)):
            with self.subTest(target=target):
                self.checker.default_controller.SendCommand.reset_mock()
                info = self._info(target)
                result = await IDMBaseTest.check_command_constraint(self.checker, info)
                self.checker.default_controller.SendCommand.assert_awaited_once()
                payload = self.checker.default_controller.SendCommand.call_args.kwargs['payload']
                encoded = TLVReader(payload.ToTLV()).get()['Any']
                self.assertEqual(encoded[target], info.field.constraints.max_value + 1)
                self.assertEqual(encoded[sibling], 0)
                self.assertLessEqual(encoded[sibling], info.all_fields[sibling].constraints.max_value)
                self.assertIn(encoded[1], info.enum_values_by_field[1])
                self.assertEqual((result.accepted, result.rejected, result.other_error, result.probed), (1, 0, 0, 1))

    async def test_unusable_required_sibling_never_sends_or_claims_rejection(self):
        for datatype, constraints in (
                ('uint16', Constraints(max_value=65534, min_value_ref=ConstraintReference(attribute='MinDeviceVolume'))),
                ('uint16', Constraints(max_value=65534, max_value_ref=ConstraintReference(attribute='MaxDeviceVolume'))),
                ('group-id', Constraints(max_value=65534)),
                ('TLSCAID', Constraints(max_value=65534)),
                ('uint16', Constraints(max_value=65534, min_count=1))):
            with self.subTest(datatype=datatype, constraints=constraints):
                info = self._info()
                info.all_fields[2] = replace(info.all_fields[2], type_info=datatype, constraints=constraints)
                result = await IDMBaseTest.check_command_constraint(self.checker, info)
                self.checker.default_controller.SendCommand.assert_not_awaited()
                self.checker.record_accepted_violation.assert_not_called()
                self.assertEqual((result.accepted, result.rejected, result.other_error, result.probed), (0, 0, 0, 0))

    async def test_tc_idm_9_1_does_not_credit_simulated_acceptance_as_enforcement(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_tc_idm_9_1()
        with self.assertRaisesRegex(signals.TestFailure, 'Command field constraints not enforced'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.assertGreater(self.checker.default_controller.SendCommand.await_count, 0)
        self.checker.record_accepted_violation.assert_called()

    async def test_tc_idm_9_1_unresolved_setup_is_not_a_dut_rejection(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_tc_idm_9_1()
        fields = self.checker.xml_clusters[Clusters.ColorControl.id].accepted_commands[
            Clusters.ColorControl.Commands.MoveToHue.command_id].fields
        fields[2] = replace(fields[2], constraints=Constraints(
            max_value_ref=ConstraintReference(attribute='MaxDeviceVolume')))
        with self.assertRaisesRegex(signals.TestFailure, 'no constraint violation could be exercised'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_awaited()
        self.checker.record_accepted_violation.assert_not_called()


if __name__ == "__main__":
    unittest.main()
