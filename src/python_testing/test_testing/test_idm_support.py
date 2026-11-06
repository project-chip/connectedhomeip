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
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.matter_testing import MatterBaseTest
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

    def _info(self, target=0, cluster=Clusters.ColorControl, command=None):
        command = command or cluster.Commands.MoveToHue
        xml_cluster = self.xml_clusters[cluster.id]
        fields = xml_cluster.accepted_commands[command.command_id].fields
        return CommandFieldInfo(
            endpoint_id=1, cluster_id=cluster.id, cluster_name=xml_cluster.name,
            command_id=command.command_id, command_name=command.__name__, command_class=command,
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

    async def test_zero_step_baseline_cannot_credit_or_fail_the_dut(self):
        commands = [(Clusters.LevelControl, Clusters.LevelControl.Commands.Step)]
        commands.extend((Clusters.ColorControl, command) for command in (
            Clusters.ColorControl.Commands.StepHue, Clusters.ColorControl.Commands.StepSaturation,
            Clusters.ColorControl.Commands.EnhancedStepHue, Clusters.ColorControl.Commands.StepColorTemperature))
        for cluster, command in commands:
            for status in (Status.ConstraintError, Status.InvalidCommand, Status.Success):
                with self.subTest(command=command.__name__, status=status):
                    self.setUp()
                    self.checker.default_controller.SendCommand.side_effect = (
                        None if status == Status.Success else InteractionModelError(status))
                    xml_cluster = self.xml_clusters[cluster.id]
                    fields = xml_cluster.accepted_commands[command.command_id].fields
                    info = replace(self._info(), cluster_id=cluster.id, cluster_class=cluster,
                                   cluster_name=xml_cluster.name, command_id=command.command_id,
                                   command_name=command.__name__, command_class=command,
                                   field=fields[0], all_fields=fields,
                                   enum_values_by_field={field_id: spec_enum_values(xml_cluster, field.type_info)
                                                         for field_id, field in fields.items()})
                    result = await IDMBaseTest.check_command_constraint(self.checker, info)
                    self.checker.default_controller.SendCommand.assert_not_called()
                    self.checker.record_accepted_violation.assert_not_called()
                    self.assertEqual((result.attempted, result.probed, result.rejected,
                                      result.accepted, result.other_error, result.execution_error), (0, 0, 0, 0, 0, 0))

    async def test_disabled_color_loop_fields_have_no_enforcement_credit(self):
        for target in (1, 2):
            with self.subTest(target=target):
                self.setUp()
                info = self._info(target, command=Clusters.ColorControl.Commands.ColorLoopSet)
                result = await IDMBaseTest.check_command_constraint(self.checker, info)
                self.checker.default_controller.SendCommand.assert_not_called()
                self.checker.record_accepted_violation.assert_not_called()
                self.assertEqual((result.attempted, result.probed, result.rejected,
                                  result.accepted, result.other_error, result.execution_error), (0, 0, 0, 0, 0, 0))
                self.assertIn('UpdateFlags=0', self.checker.record_warning.call_args.kwargs['problem'])
                self.assertIn('NOT DEMONSTRATED', self.checker.record_warning.call_args.kwargs['problem'])

    async def test_root_certificate_excluded_before_generation_or_invocation(self):
        cluster = Clusters.TlsCertificateManagement
        command = cluster.Commands.ProvisionRootCertificate
        fields = self.xml_clusters[cluster.id].accepted_commands[command.command_id].fields
        certificate_id = next(field_id for field_id, field in fields.items() if field.name == 'Certificate')
        self.checker.generate_command_field_violations = Mock(side_effect=AssertionError('unsafe certificate generated'))
        self.checker._resolved_command_field_constraints = AsyncMock(side_effect=AssertionError('unsafe baseline read'))
        self.checker.default_controller.SendCommand.side_effect = ChipStackError(0x0b)
        result = await IDMBaseTest.check_command_constraint(self.checker, self._info(certificate_id, cluster, command))
        self.checker.generate_command_field_violations.assert_not_called()
        self.checker._resolved_command_field_constraints.assert_not_called()
        self.checker.default_controller.SendCommand.assert_not_called()
        self.checker.record_accepted_violation.assert_not_called()
        self.assertEqual((result.attempted, result.probed, result.rejected,
                          result.accepted, result.other_error, result.execution_error), (0, 0, 0, 0, 0, 0))
        self.assertIn('credential', self.checker.record_warning.call_args.kwargs['problem'])

    async def test_tc_idm_9_1_baseline_limitations_are_discovered_but_skip(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for cluster, command in (
                (Clusters.LevelControl, Clusters.LevelControl.Commands.Step),
                (Clusters.ColorControl, Clusters.ColorControl.Commands.ColorLoopSet),
                (Clusters.TlsCertificateManagement, Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate)):
            with self.subTest(command=command.__name__):
                self.setUp()
                self._prepare_tc_idm_9_1()
                self.checker.endpoints_tlv[1] = {cluster.id: {
                    GlobalAttributeIds.FEATURE_MAP_ID: 0xFFFFFFFF,
                    GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [command.command_id],
                    GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}}
                self.assertTrue(self.checker.discover_constrained_command_fields())
                with self.assertRaisesRegex(signals.TestSkip, 'No qualified constraint violation'):
                    await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                self.checker.default_controller.SendCommand.assert_not_called()
                self.checker.record_accepted_violation.assert_not_called()
                self.assertTrue(all('NOT DEMONSTRATED' in call.kwargs['problem']
                                    for call in self.checker.record_warning.call_args_list))

    async def test_unknown_move_modes_keep_zero_rate_and_strict_wrong_status_failure(self):
        for command in (Clusters.ColorControl.Commands.MoveHue, Clusters.ColorControl.Commands.MoveSaturation,
                        Clusters.ColorControl.Commands.EnhancedMoveHue, Clusters.ColorControl.Commands.MoveColorTemperature):
            with self.subTest(command=command.__name__):
                self.setUp()
                self._prepare_tc_idm_9_1()
                info = self._info(command=command)
                self.checker.default_controller.SendCommand.side_effect = InteractionModelError(Status.InvalidCommand)
                result = await IDMBaseTest.check_command_constraint(self.checker, info)
                self.assertEqual((result.attempted, result.probed, result.rejected, result.other_error), (2, 2, 0, 2))
                encoded = [TLVReader(call.kwargs['payload'].ToTLV()).get()['Any']
                           for call in self.checker.default_controller.SendCommand.call_args_list]
                self.assertEqual([payload[0] for payload in encoded], [2, 4])
                self.assertTrue(all(payload[1] == 0 for payload in encoded))
                with self.assertRaisesRegex(signals.TestFailure, 'CONSTRAINT_ERROR'):
                    self.checker.enforce_constraint_policy('Command field', [], result.rejected, result.probed)

    async def test_remaining_unknown_enums_are_qualified_and_wrong_status_fails(self):
        for cluster, command, target in (
                (Clusters.Thermostat, Clusters.Thermostat.Commands.SetpointRaiseLower, 0),
                (Clusters.ColorControl, Clusters.ColorControl.Commands.MoveToHue, 1),
                (Clusters.ColorControl, Clusters.ColorControl.Commands.EnhancedMoveToHue, 1)):
            with self.subTest(command=command.__name__):
                self.setUp()
                self._prepare_tc_idm_9_1()
                self.checker.default_controller.SendCommand.side_effect = InteractionModelError(Status.InvalidCommand)
                info = self._info(target, cluster, command)
                result = await IDMBaseTest.check_command_constraint(self.checker, info)
                self.assertEqual((result.attempted, result.probed, result.rejected, result.other_error), (1, 1, 0, 1))
                with self.assertRaisesRegex(signals.TestFailure, 'CONSTRAINT_ERROR'):
                    self.checker.enforce_constraint_policy('Command field', [], result.rejected, result.probed)

    async def test_tc_idm_9_1_qualified_unknown_enums_still_fail_wrong_status(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        commands = [(Clusters.Thermostat, Clusters.Thermostat.Commands.SetpointRaiseLower)]
        commands.extend((Clusters.ColorControl, command) for command in (
            Clusters.ColorControl.Commands.MoveHue, Clusters.ColorControl.Commands.MoveSaturation,
            Clusters.ColorControl.Commands.EnhancedMoveHue, Clusters.ColorControl.Commands.MoveColorTemperature,
            Clusters.ColorControl.Commands.MoveToHue, Clusters.ColorControl.Commands.EnhancedMoveToHue))
        for cluster, command in commands:
            for status in (Status.InvalidCommand, Status.ConstraintError):
                with self.subTest(command=command.__name__, status=status):
                    self.setUp()
                    self._prepare_tc_idm_9_1()
                    self.checker.endpoints_tlv[1] = {cluster.id: {
                        GlobalAttributeIds.FEATURE_MAP_ID: 0xFFFFFFFF,
                        GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [command.command_id],
                        GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}}
                    self.checker.default_controller.SendCommand.side_effect = InteractionModelError(status)
                    if status == Status.InvalidCommand:
                        with self.assertRaisesRegex(signals.TestFailure, 'CONSTRAINT_ERROR'):
                            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    else:
                        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertGreater(self.checker.default_controller.SendCommand.await_count, 0)

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
        with self.assertRaisesRegex(signals.TestSkip, 'No qualified constraint violation'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_awaited()
        self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_strict_unrelated_statuses_fail(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for statuses in ((Status.InvalidCommand,), (Status.ResourceExhausted,),
                         (Status.InvalidCommand, Status.ResourceExhausted),
                         (Status.ConstraintError, Status.InvalidCommand)):
            with self.subTest(statuses=statuses):
                self.setUp()
                self._prepare_tc_idm_9_1()
                controller = self.checker.default_controller

                async def send_command(**kwargs):
                    status = statuses[(controller.SendCommand.await_count - 1) % len(statuses)]
                    raise InteractionModelError(status)

                controller.SendCommand.side_effect = send_command
                with self.assertRaisesRegex(signals.TestFailure, 'CONSTRAINT_ERROR'):
                    await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                self.assertGreater(controller.SendCommand.await_count, 1)
                self.checker.record_accepted_violation.assert_not_called()

    async def test_qualified_transport_error_preserves_errno_and_partial_result(self):
        self.checker.default_controller.SendCommand.side_effect = ChipStackError(0x0b)
        with self.assertRaises(ChipStackError) as raised:
            await IDMBaseTest.check_command_constraint(self.checker, self._info())
        self.assertEqual(raised.exception.err, 0x0b)
        result = raised.exception.result
        self.assertEqual((result.attempted, result.execution_error, result.probed,
                          result.rejected, result.accepted, result.other_error), (1, 1, 0, 0, 0, 0))
        self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_unsafe_command_is_not_enforcement(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_tc_idm_9_1()
        self.checker.endpoints_tlv[1][Clusters.ColorControl.id][GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] = []
        cluster = Clusters.OperationalCredentials
        self.checker.endpoints_tlv[2] = {cluster.id: {
            GlobalAttributeIds.FEATURE_MAP_ID: 0,
            GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [cluster.Commands.AddNOC.command_id],
            GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}}
        self.assertEqual(self.checker.discover_constrained_command_fields(), [])
        with self.assertRaisesRegex(signals.TestSkip, 'No qualified constraint violation'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_called()
        self.checker.record_accepted_violation.assert_not_called()

    def _prepare_writable_attribute(self, include_commands=False):
        self._prepare_tc_idm_9_1()
        if not include_commands:
            self.checker.endpoints_tlv[1][Clusters.ColorControl.id][GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] = []
        cluster = Clusters.OnOff
        attribute = cluster.Attributes.StartUpOnOff
        self.checker.endpoints_tlv[2] = {cluster.id: {
            GlobalAttributeIds.FEATURE_MAP_ID: 0,
            GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
            GlobalAttributeIds.ATTRIBUTE_LIST_ID: [attribute.attribute_id]}}
        for name in ('generate_constraint_violations', 'check_attribute_constraint'):
            setattr(self.checker, name, MethodType(getattr(IDMBaseTest, name), self.checker))
        self.checker.read_single_attribute_check_success = AsyncMock(return_value=0)
        self.checker.read_single_attribute = AsyncMock(return_value=0)
        self.checker.default_controller.SendCommand.side_effect = InteractionModelError(Status.ConstraintError)
        self.checker.default_controller.WriteAttribute = AsyncMock(
            return_value=[SimpleNamespace(Status=Status.ConstraintError)])

    async def test_tc_idm_9_1_strict_writable_attribute_statuses(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for status in (Status.InvalidCommand, Status.ResourceExhausted, Status.ConstraintError, Status.Success):
            with self.subTest(status=status):
                self.setUp()
                self._prepare_writable_attribute()
                self.checker.default_controller.WriteAttribute.return_value = [SimpleNamespace(Status=status)]
                if status == Status.ConstraintError:
                    await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.checker.record_accepted_violation.assert_not_called()
                else:
                    message = 'constraints not enforced' if status == Status.Success else 'CONSTRAINT_ERROR'
                    with self.assertRaisesRegex(signals.TestFailure, message):
                        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                self.assertGreater(self.checker.default_controller.WriteAttribute.await_count, 0)
                self.checker.default_controller.SendCommand.assert_not_called()

    async def test_tc_idm_9_1_command_execution_error_never_passes(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_tc_idm_9_1()
        self.checker.default_controller.SendCommand.side_effect = ChipStackError(0x0b)
        with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(
                signals.TestFailure, 'Execution failed'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        calls = self.checker.default_controller.SendCommand.await_count
        self.assertGreater(calls, 0)
        self.assertTrue(any(f'attempted={calls}, classified=0, CONSTRAINT_ERROR=0, accepted=0, '
                            f'other IM or response status=0, execution errors={calls}' in line for line in captured.output))
        self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_partial_command_result_cannot_mask_transport_failure(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for first_status in (Status.ConstraintError, Status.Success):
            with self.subTest(first_status=first_status):
                self.setUp()
                self._prepare_tc_idm_9_1()
                info = self._info()
                info = replace(info, field=replace(info.field, constraints=Constraints(min_value=1, max_value=254)))
                self.checker.discover_constrained_command_fields = Mock(return_value=[info])
                controller = self.checker.default_controller

                async def command_response(**kwargs):
                    if controller.SendCommand.await_count == 1:
                        if first_status == Status.ConstraintError:
                            raise InteractionModelError(first_status)
                        return
                    raise ChipStackError(0x0b)

                controller.SendCommand.side_effect = command_response
                message = 'Execution failed' if first_status == Status.ConstraintError else 'constraints not enforced'
                with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(signals.TestFailure, message):
                    await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                self.assertEqual(controller.SendCommand.await_count, 2)
                rejected = int(first_status == Status.ConstraintError)
                accepted = int(first_status == Status.Success)
                self.assertTrue(any(f'attempted=2, classified=1, CONSTRAINT_ERROR={rejected}, accepted={accepted}, '
                                    'other IM or response status=0, execution errors=1' in line for line in captured.output))
                self.assertEqual(self.checker.record_accepted_violation.call_count, accepted)

    async def test_tc_idm_9_1_writable_execution_error_never_passes(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for operation in ('WriteAttribute', 'read_single_attribute'):
            with self.subTest(operation=operation):
                self.setUp()
                self._prepare_writable_attribute(include_commands=True)
                target = (self.checker.default_controller.WriteAttribute if operation == 'WriteAttribute'
                          else self.checker.read_single_attribute)
                target.side_effect = ChipStackError(0x0b)
                with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(
                        signals.TestFailure, 'Execution failed'):
                    await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                expected_writes = 1 if operation == 'WriteAttribute' else 2
                self.assertEqual(self.checker.default_controller.WriteAttribute.await_count, expected_writes)
                self.assertTrue(any('attempted=1, classified=0, CONSTRAINT_ERROR=0, accepted=0, '
                                    f'other IM or response status=0, execution errors={expected_writes}' in line for line in captured.output))
                self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_accepted_write_precedes_readback_failure(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute()
        self.checker.default_controller.WriteAttribute.return_value = [SimpleNamespace(Status=Status.Success)]
        self.checker.read_single_attribute.side_effect = ChipStackError(0x0b)
        with self.assertRaisesRegex(signals.TestFailure, 'Writable attribute constraints not enforced'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.record_accepted_violation.assert_called_once()

    async def test_tc_idm_9_1_no_qualified_probes_is_not_pass(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute()
        self.checker.read_single_attribute_check_success.side_effect = ChipStackError(0x0b)
        with self.assertRaisesRegex(signals.TestSkip, 'No qualified constraint violation'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.WriteAttribute.assert_not_called()
        self.checker.default_controller.SendCommand.assert_not_called()
        self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_unreadable_baseline_has_no_enforcement_credit(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute()
        cluster = Clusters.OnOff
        attribute = cluster.Attributes.StartUpOnOff
        self.checker.read_single_attribute_check_success = MethodType(
            MatterBaseTest.read_single_attribute_check_success, self.checker)
        self.checker.default_controller.ReadAttribute = AsyncMock(return_value={2: {cluster: {attribute: None}}})
        self.checker.record_error = Mock()
        with self.assertRaisesRegex(signals.TestSkip, 'No qualified constraint violation'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.WriteAttribute.assert_not_called()
        self.checker.default_controller.SendCommand.assert_not_called()
        self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_unqualified_commands_allow_attribute_scope(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute(include_commands=True)
        fields = self.checker.xml_clusters[Clusters.ColorControl.id].accepted_commands[
            Clusters.ColorControl.Commands.MoveToHue.command_id].fields
        fields[2] = replace(fields[2], constraints=Constraints(max_value_ref=ConstraintReference(attribute='MaxDeviceVolume')))
        with self.assertLogs('TC_IDM_9_1', level='INFO') as captured:
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_called()
        self.assertEqual(self.checker.default_controller.WriteAttribute.await_count, 1)
        self.checker.record_accepted_violation.assert_not_called()
        self.assertTrue(any('Command probes: attempted=0, classified=0, CONSTRAINT_ERROR=0' in line for line in captured.output))
        self.assertTrue(any('Writable attribute probes: attempted=1, classified=1, CONSTRAINT_ERROR=1' in line for line in captured.output))

    async def test_tc_idm_9_1_baseline_subscription_mismatch_is_not_skipped(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute(include_commands=True)
        cluster = Clusters.OnOff
        attribute = cluster.Attributes.StartUpOnOff
        self.checker.default_controller.ReadAttribute = AsyncMock(
            return_value={2: {cluster: {attribute: cluster.Enums.StartUpOnOffEnum.kOff}}})
        self.checker.read_single_attribute_check_success = MethodType(
            MatterBaseTest.read_single_attribute_check_success, self.checker)
        self.checker._effective_verify_wildcard_subscription = Mock(return_value=True)
        self.checker.verify_attribute_subscription_value = AsyncMock(side_effect=signals.TestFailure('confirmed cache mismatch'))
        self.checker.record_error = Mock()
        with self.assertRaisesRegex(signals.TestFailure, 'confirmed cache mismatch'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.verify_attribute_subscription_value.assert_awaited_once()
        self.checker.default_controller.WriteAttribute.assert_not_called()
        self.checker.record_error.assert_not_called()

    async def test_tc_idm_9_1_raised_write_statuses_are_classified(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for version in (SPEC_VERSION_1_7, SPEC_VERSION_1_7 - 1):
            for statuses in ((Status.ConstraintError,), (Status.InvalidCommand,), (Status.ResourceExhausted,),
                             (Status.ConstraintError, Status.ResourceExhausted)):
                with self.subTest(version=version, statuses=statuses):
                    self.setUp()
                    self._prepare_writable_attribute()
                    self.checker.endpoints_tlv[3] = deepcopy(self.checker.endpoints_tlv[2])
                    basic = self.checker.endpoints_tlv[0][Clusters.BasicInformation.id]
                    basic[Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id] = version
                    controller = self.checker.default_controller

                    async def write_response(**kwargs):
                        status = statuses[(controller.WriteAttribute.await_count - 1) % len(statuses)]
                        raise InteractionModelError(status)

                    controller.WriteAttribute.side_effect = write_response
                    answers = [statuses[index % len(statuses)] for index in range(2)]
                    rejected = answers.count(Status.ConstraintError)
                    failure = rejected != 2 if version == SPEC_VERSION_1_7 else rejected == 0
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured:
                        if failure:
                            with self.assertRaisesRegex(signals.TestFailure, 'CONSTRAINT_ERROR'):
                                await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                        else:
                            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(controller.WriteAttribute.await_count, 2)
                    self.assertEqual(self.checker.read_single_attribute.await_count, 2)
                    self.assertTrue(any(f'attempted=2, classified=2, CONSTRAINT_ERROR={rejected}, accepted=0, '
                                        f'other IM or response status={2 - rejected}, execution errors=0' in line for line in captured.output))
                    self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_raised_restore_status_retains_accepted_verdict(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for initial_status in (Status.ConstraintError, Status.Success):
            for restore_status in (Status.Failure, Status.ConstraintError, Status.ResourceExhausted):
                with self.subTest(initial_status=initial_status, restore_status=restore_status):
                    self.setUp()
                    self._prepare_writable_attribute()
                    self.checker.read_single_attribute.return_value = 3
                    controller = self.checker.default_controller

                    async def write_response(**kwargs):
                        if controller.WriteAttribute.await_count == 1:
                            return [SimpleNamespace(Status=initial_status)]
                        raise InteractionModelError(restore_status)

                    controller.WriteAttribute.side_effect = write_response
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(
                            signals.TestFailure, 'Writable attribute constraints not enforced'):
                        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(controller.WriteAttribute.await_count, 2)
                    self.checker.record_accepted_violation.assert_called_once()
                    self.assertTrue(any('attempted=1, classified=1, CONSTRAINT_ERROR=0, accepted=1, '
                                        'other IM or response status=0, execution errors=1' in line for line in captured.output))

    async def test_tc_idm_9_1_readback_failure_attempts_original_restoration(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for initial_status in (Status.Success, Status.ConstraintError):
            for restore_status in (Status.Success, Status.ResourceExhausted):
                with self.subTest(initial_status=initial_status, restore_status=restore_status):
                    self.setUp()
                    self._prepare_writable_attribute()
                    controller = self.checker.default_controller
                    controller.WriteAttribute.side_effect = [
                        [SimpleNamespace(Status=initial_status)], [SimpleNamespace(Status=restore_status)]]
                    self.checker.read_single_attribute.side_effect = ChipStackError(0x0b)
                    failure = 'constraints not enforced' if initial_status == Status.Success else 'Execution failed for writable'
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(signals.TestFailure, failure):
                        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(controller.WriteAttribute.await_count, 2)
                    self.assertEqual(controller.WriteAttribute.await_args_list[1].kwargs['attributes'][0][1].value, 0)
                    accepted = int(initial_status == Status.Success)
                    errors = 1 + int(restore_status != Status.Success)
                    self.assertTrue(any(f'attempted=1, classified={accepted}, CONSTRAINT_ERROR=0, accepted={accepted}, '
                                        f'other IM or response status=0, execution errors={errors}' in line for line in captured.output))
                    self.assertEqual(self.checker.record_accepted_violation.call_count, accepted)

    async def test_tc_idm_9_1_stored_violation_retains_any_error_status(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for initial_status in (Status.ConstraintError, Status.InvalidCommand, Status.ResourceExhausted):
            for restore_error in (None, InteractionModelError(Status.ResourceExhausted), ChipStackError(0x0c)):
                with self.subTest(initial_status=initial_status, restore_error=restore_error):
                    self.setUp()
                    self._prepare_writable_attribute()
                    controller = self.checker.default_controller
                    controller.WriteAttribute.side_effect = [
                        [SimpleNamespace(Status=initial_status)], restore_error or [SimpleNamespace(Status=Status.Success)]]
                    self.checker.read_single_attribute.return_value = 3
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(
                            signals.TestFailure, 'Writable attribute constraints not enforced'):
                        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(controller.WriteAttribute.await_count, 2)
                    self.assertEqual(controller.WriteAttribute.await_args_list[1].kwargs['attributes'][0][1].value, 0)
                    self.checker.record_accepted_violation.assert_called_once()
                    self.assertTrue(any('attempted=1, classified=1, CONSTRAINT_ERROR=0, accepted=1, '
                                        f'other IM or response status=0, execution errors={int(restore_error is not None)}'
                                        in line for line in captured.output))

    async def test_tc_idm_9_1_embedded_rejections_are_not_acceptance(self):
        """Synthetic response payloads classify statuses, not real DUT enforcement.

        Strict non-CONSTRAINT_ERROR responses fail; earlier versions need one CONSTRAINT_ERROR.
        """
        from TC_IDM_9_1 import TC_IDM_9_1

        for version in (SPEC_VERSION_1_7, SPEC_VERSION_1_7 - 1):
            for statuses in ((Status.ConstraintError,), (Status.ResourceExhausted,), (Status.InvalidCommand,),
                             (Status.ConstraintError, Status.ResourceExhausted), (Status.Success,)):
                with self.subTest(version=version, statuses=statuses):
                    self.setUp()
                    self._prepare_tc_idm_9_1()
                    basic = self.checker.endpoints_tlv[0][Clusters.BasicInformation.id]
                    basic[Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id] = version
                    controller = self.checker.default_controller

                    async def command_response(**kwargs):
                        return SimpleNamespace(status=statuses[(controller.SendCommand.await_count - 1) % len(statuses)])

                    controller.SendCommand.side_effect = command_response
                    answers = [statuses[index % len(statuses)] for index in range(3)]
                    rejected = answers.count(Status.ConstraintError)
                    accepted = answers.count(Status.Success)
                    other = 3 - rejected - accepted
                    failure = rejected != 3 if version == SPEC_VERSION_1_7 else rejected == 0
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured:
                        if failure:
                            with self.assertRaises(signals.TestFailure):
                                await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                        else:
                            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(controller.SendCommand.await_count, 3)
                    self.assertEqual(self.checker.record_accepted_violation.call_count, accepted)
                    self.assertTrue(any(f'attempted=3, classified=3, CONSTRAINT_ERROR={rejected}, accepted={accepted}, '
                                        f'other IM or response status={other}, execution errors=0' in line for line in captured.output))

    async def test_tc_idm_9_1_attribute_assertion_is_not_swallowed(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute(include_commands=True)
        self.checker.read_single_attribute.side_effect = signals.TestFailure('readback failed')
        with self.assertRaisesRegex(signals.TestFailure, 'readback failed'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.assertEqual(self.checker.default_controller.WriteAttribute.await_count, 2)
        self.assertEqual(self.checker.default_controller.WriteAttribute.await_args_list[1].kwargs['attributes'][0][1].value, 0)

    async def test_tc_idm_9_1_era_policy_and_exact_status_counts(self):
        """Strict test-plan policy requires every classified probe to return CONSTRAINT_ERROR.

        Earlier or missing versions require one such rejection; accepted violations only warn.
        """
        from TC_IDM_9_1 import TC_IDM_9_1

        cases = (
            (SPEC_VERSION_1_7, (Status.ConstraintError,), None),
            (SPEC_VERSION_1_7, (Status.Success,), 'constraints not enforced'),
            (SPEC_VERSION_1_7, (Status.InvalidCommand,), 'CONSTRAINT_ERROR'),
            (SPEC_VERSION_1_7, (Status.ResourceExhausted,), 'CONSTRAINT_ERROR'),
            (SPEC_VERSION_1_7, (Status.ConstraintError, Status.InvalidCommand), 'CONSTRAINT_ERROR'),
            (SPEC_VERSION_1_7 - 1, (Status.ConstraintError,), None),
            (SPEC_VERSION_1_7 - 1, (Status.Success,), 'never returned CONSTRAINT_ERROR'),
            (SPEC_VERSION_1_7 - 1, (Status.InvalidCommand,), 'never returned CONSTRAINT_ERROR'),
            (SPEC_VERSION_1_7 - 1, (Status.ResourceExhausted,), 'never returned CONSTRAINT_ERROR'),
            (SPEC_VERSION_1_7 - 1, (Status.ConstraintError, Status.Success), None),
            (SPEC_VERSION_1_7 - 1, (Status.ConstraintError, Status.InvalidCommand), None),
            (None, (Status.ConstraintError, Status.ResourceExhausted), None),
            (None, (Status.InvalidCommand,), 'never returned CONSTRAINT_ERROR'))
        for channel in ('commands', 'attributes'):
            for version, statuses, failure in cases:
                with self.subTest(channel=channel, version=version, statuses=statuses):
                    self.setUp()
                    if channel == 'commands':
                        self._prepare_tc_idm_9_1()
                        operation = self.checker.default_controller.SendCommand
                        expected_probes = 3

                        async def command_response(**kwargs):
                            status = statuses[(operation.await_count - 1) % len(statuses)]
                            if status != Status.Success:
                                raise InteractionModelError(status)
                            return

                        operation.side_effect = command_response
                    else:
                        self._prepare_writable_attribute()
                        self.checker.endpoints_tlv[3] = deepcopy(self.checker.endpoints_tlv[2])
                        operation = self.checker.default_controller.WriteAttribute
                        expected_probes = 2

                        async def attribute_response(**kwargs):
                            status = statuses[(operation.await_count - 1) % len(statuses)]
                            return [SimpleNamespace(Status=status)]

                        operation.side_effect = attribute_response
                    basic = self.checker.endpoints_tlv[0][Clusters.BasicInformation.id]
                    specification_version = Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id
                    if version is None:
                        basic.pop(specification_version)
                    else:
                        basic[specification_version] = version
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured:
                        if failure:
                            with self.assertRaisesRegex(signals.TestFailure, failure):
                                await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                        else:
                            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(operation.await_count, expected_probes)
                    answers = [statuses[index % len(statuses)] for index in range(expected_probes)]
                    rejected = answers.count(Status.ConstraintError)
                    accepted = answers.count(Status.Success)
                    other = expected_probes - rejected - accepted
                    self.assertTrue(any(
                        f'attempted={expected_probes}, classified={expected_probes}, CONSTRAINT_ERROR={rejected}, '
                        f'accepted={accepted}, other IM or response status={other}, execution errors=0' in line
                        for line in captured.output))
                    self.assertEqual(self.checker.record_accepted_violation.call_count, accepted)

    async def test_tc_idm_9_1_mixed_scope_does_not_mask_wrong_attribute_status(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_writable_attribute(include_commands=True)
        self.checker.default_controller.WriteAttribute.return_value = [SimpleNamespace(Status=Status.InvalidCommand)]
        with self.assertRaisesRegex(signals.TestFailure, 'Writable attribute must return CONSTRAINT_ERROR'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.assertEqual(self.checker.default_controller.SendCommand.await_count, 3)
        self.assertEqual(self.checker.default_controller.WriteAttribute.await_count, 1)
        self.checker.record_accepted_violation.assert_not_called()

    async def test_tc_idm_9_1_stored_violation_precedes_restore_failure(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        for initial_status in (Status.ConstraintError, Status.Success):
            for restore_failure in (ChipStackError(0x0b), Status.Failure):
                with self.subTest(initial_status=initial_status, restore_failure=restore_failure):
                    self.setUp()
                    self._prepare_writable_attribute()
                    self.checker.read_single_attribute.return_value = 3

                    async def write_response(**kwargs):
                        if self.checker.default_controller.WriteAttribute.await_count == 1:
                            return [SimpleNamespace(Status=initial_status)]
                        if isinstance(restore_failure, ChipStackError):
                            raise restore_failure
                        return [SimpleNamespace(Status=restore_failure)]

                    self.checker.default_controller.WriteAttribute.side_effect = write_response
                    with self.assertLogs('TC_IDM_9_1', level='INFO') as captured, self.assertRaisesRegex(
                            signals.TestFailure, 'Writable attribute constraints not enforced'):
                        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
                    self.assertEqual(self.checker.default_controller.WriteAttribute.await_count, 2)
                    self.assertEqual(
                        self.checker.default_controller.WriteAttribute.await_args_list[1].kwargs['attributes'][0][1].value, 0)
                    self.checker.record_accepted_violation.assert_called_once()
                    self.assertTrue(any('attempted=1, classified=1, CONSTRAINT_ERROR=0, accepted=1, '
                                        'other IM or response status=0, execution errors=1' in line for line in captured.output))

    async def test_tc_idm_9_1_empty_scope_is_skip(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._prepare_tc_idm_9_1()
        self.checker.endpoints_tlv[1][Clusters.ColorControl.id][GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] = []
        with self.assertRaisesRegex(signals.TestSkip, 'No qualified constraint violation'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_called()


if __name__ == "__main__":
    unittest.main()
