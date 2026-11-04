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
from pathlib import Path
from types import MethodType, SimpleNamespace
from unittest.mock import AsyncMock, Mock

from mobly import signals

import matter.clusters as Clusters
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.problem_notices import UnknownProblemLocation
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters
from matter.tlv import TLVReader

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.idm_support import (SPEC_VERSION_1_7, CommandFieldInfo, IDMBaseTest, _codegen_enum_values,  # noqa: E402
                                         enum_in_type, smallest_legal_enum_value, spec_enum_values, undefined_enum_values)


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


class TestDiagnosticConstraintCoverage(unittest.IsolatedAsyncioTestCase):
    """Exercise the real TC with mocked transport, not DUT enforcement."""

    @classmethod
    def setUpClass(cls):
        cls.xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)

    def setUp(self):
        diagnostics = Clusters.GeneralDiagnostics
        self.checker = SimpleNamespace(
            ROOT_NODE_ENDPOINT_ID=0, xml_clusters=self.xml_clusters,
            endpoints_tlv={0: {
                Clusters.BasicInformation.id: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id: SPEC_VERSION_1_7,
                    GlobalAttributeIds.ATTRIBUTE_LIST_ID: []},
                diagnostics.id: {
                    GlobalAttributeIds.FEATURE_MAP_ID: diagnostics.Bitmaps.Feature.kDataModelTest,
                    GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [diagnostics.Commands.TestEventTrigger.command_id,
                                                                  diagnostics.Commands.PayloadTestRequest.command_id],
                    GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}}},
            default_controller=SimpleNamespace(SendCommand=AsyncMock(side_effect=InteractionModelError(Status.ConstraintError))),
            dut_node_id=1,
            current_test_info=SimpleNamespace(name='test_TC_IDM_9_1'),
            matter_test_config=SimpleNamespace(endpoint=0, global_test_params={}),
            setup_class_helper=AsyncMock(), build_spec_xmls=Mock(), step=Mock(),
            record_warning=Mock(), record_accepted_violation=Mock(),
            resolve_dynamic_constraint=AsyncMock(return_value=None))
        for name in ('_allowed_numeric_values', '_allowed_lengths', '_command_field_label', '_command_field_enum_type'):
            setattr(self.checker, name, getattr(IDMBaseTest, name))
        for name in ('_generate_valid_command_field_value', '_resolved_command_field_constraints',
                     'generate_command_field_violations', 'check_command_constraint', 'discover_constrained_command_fields',
                     'dut_spec_version', 'enforces_constraints_strictly', 'enforce_constraint_policy', 'checkable_attributes'):
            setattr(self.checker, name, MethodType(getattr(IDMBaseTest, name), self.checker))

    async def test_all_excluded_is_unsupported_not_pass(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        with self.assertRaisesRegex(signals.TestSkip, 'constraint enforcement not demonstrated'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_awaited()
        problems = [call.kwargs['problem'] for call in self.checker.record_warning.call_args_list]
        self.assertEqual(len(problems), 3)
        for field in ('TestEventTrigger.EnableKey', 'PayloadTestRequest.EnableKey', 'PayloadTestRequest.Count'):
            self.assertTrue(any(field in problem and 'safe baseline' in problem for problem in problems))
        self.checker.record_accepted_violation.assert_not_called()

    def _ordinary_command(self, include_diagnostics: bool) -> CommandFieldInfo:
        if not include_diagnostics:
            self.checker.endpoints_tlv[0].pop(Clusters.GeneralDiagnostics.id)
        cluster = Clusters.Thermostat
        command = cluster.Commands.SetpointRaiseLower
        self.checker.endpoints_tlv[1] = {cluster.id: {
            GlobalAttributeIds.FEATURE_MAP_ID: 0,
            GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [command.command_id],
            GlobalAttributeIds.ATTRIBUTE_LIST_ID: []}}
        return next(info for info in self.checker.discover_constrained_command_fields() if info.cluster_id == cluster.id)

    async def test_mixed_coverage_stays_unsupported_after_ordinary_probes(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._ordinary_command(include_diagnostics=True)
        self.checker.enforce_constraint_policy = Mock(wraps=self.checker.enforce_constraint_policy)
        with self.assertRaisesRegex(signals.TestSkip, 'PayloadTestRequest.Count'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        calls = self.checker.default_controller.SendCommand.await_args_list
        self.assertGreater(len(calls), 0)
        self.assertTrue(all(isinstance(call.kwargs['payload'], Clusters.Thermostat.Commands.SetpointRaiseLower) for call in calls))
        self.checker.enforce_constraint_policy.assert_any_call(
            subject='Command field', accepted_paths=[], rejected_count=len(calls), probed_count=len(calls))

    async def test_ordinary_only_can_qualify(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        expectation = TC_IDM_9_1.steps_TC_IDM_9_1(self.checker)[1].expectation
        self.assertIn('GeneralDiagnostics.TestEventTrigger', expectation)
        self.assertIn('GeneralDiagnostics.PayloadTestRequest', expectation)
        self.assertIn('unsupported, not as demonstrated constraint enforcement', expectation)
        self._ordinary_command(include_diagnostics=False)
        await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.assertGreater(self.checker.default_controller.SendCommand.await_count, 0)
        self.checker.record_warning.assert_not_called()
        self.checker.record_accepted_violation.assert_not_called()

    async def test_exclusions_do_not_mask_ordinary_non_enforcement(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        self._ordinary_command(include_diagnostics=True)
        self.checker.default_controller.SendCommand.side_effect = None
        self.checker.default_controller.SendCommand.return_value = None
        with self.assertRaisesRegex(signals.TestFailure, 'Command field constraints not enforced'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.record_accepted_violation.assert_called()

    async def test_diagnostics_never_send_with_missing_wrong_or_present_pixit(self):
        cluster = Clusters.GeneralDiagnostics
        for command in (cluster.Commands.TestEventTrigger, cluster.Commands.PayloadTestRequest):
            fields = self.xml_clusters[cluster.id].accepted_commands[command.command_id].fields
            for key in (None, '', b'', bytes(15), bytes(16), bytes(range(16))):
                self.checker.matter_test_config.global_test_params = (
                    {} if key is None else {'PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY': key})
                for field in fields.values():
                    with self.subTest(command=command.__name__, field=field.name, key=key):
                        info = CommandFieldInfo(
                            endpoint_id=0, cluster_id=cluster.id, cluster_name='GeneralDiagnostics',
                            command_id=command.command_id, command_name=command.__name__, command_class=command,
                            cluster_class=cluster, field=field, all_fields=fields, enum_values_by_field={})
                        result = await self.checker.check_command_constraint(info)
                        self.assertEqual((result.probed, result.rejected, result.accepted, result.other_error), (0, 0, 0, 0))
        self.checker.default_controller.SendCommand.assert_not_awaited()
        self.checker.record_accepted_violation.assert_not_called()

    async def test_unrelated_errors_do_not_credit_ordinary_enforcement(self):
        info = self._ordinary_command(include_diagnostics=False)
        for status in (Status.InvalidCommand, Status.ResourceExhausted):
            with self.subTest(status=status):
                self.checker.default_controller.SendCommand.side_effect = InteractionModelError(status)
                result = await self.checker.check_command_constraint(info)
                self.assertGreater(result.other_error, 0)
                self.assertEqual((result.rejected, result.accepted), (0, 0))
        self.checker.default_controller.SendCommand.side_effect = ChipStackError(0x0b)
        with self.assertRaises(ChipStackError):
            await self.checker.check_command_constraint(info)
        self.checker.record_accepted_violation.assert_not_called()

    async def test_valid_baseline_invalid_target_rejects_but_accepted_target_fails(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        info = self._ordinary_command(include_diagnostics=False)
        result = await self.checker.check_command_constraint(info)
        self.assertGreater(result.rejected, 0)
        self.assertEqual((result.accepted, result.other_error), (0, 0))
        self.assertEqual(info.field.name, 'Mode')
        calls = self.checker.default_controller.SendCommand.await_args_list
        self.assertEqual(len(calls), result.rejected)
        for call in calls:
            payload = call.kwargs['payload']
            self.assertIsInstance(payload, Clusters.Thermostat.Commands.SetpointRaiseLower)
            self.assertEqual(payload.amount, 0, 'The sibling must retain its valid baseline')
            self.assertNotIn(int(payload.mode), (0, 1, 2))
            encoded = TLVReader(payload.ToTLV()).get()['Any']
            self.assertEqual(encoded[1], 0)
            self.assertEqual(encoded[0], int(payload.mode))
        self.checker.default_controller.SendCommand.side_effect = None
        self.checker.default_controller.SendCommand.return_value = None
        with self.assertRaisesRegex(signals.TestFailure, 'constraints not enforced'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.record_accepted_violation.assert_called()

    async def test_invalid_baseline_with_valid_or_invalid_target_is_not_enforcement(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        matching_key = bytes(range(16))
        command = Clusters.GeneralDiagnostics.Commands.PayloadTestRequest
        for enforce_count in (False, True):
            for key in (bytes(16), matching_key):
                for count in (1, 2049):
                    with self.subTest(enforce_count=enforce_count, valid_baseline=key == matching_key, count=count):
                        payload = command(enableKey=key, value=0, count=count)
                        encoded = TLVReader(payload.ToTLV()).get()['Any']
                        model_status = (Status.ConstraintError if encoded[0] != matching_key
                                        or (enforce_count and encoded[2] > 2048) else Status.Success)
                        if key != matching_key:
                            self.assertEqual(model_status, Status.ConstraintError)
                        elif count == 2049:
                            self.assertEqual(model_status, Status.ConstraintError if enforce_count else Status.Success)
                        else:
                            self.assertEqual(model_status, Status.Success)
                        self.checker.matter_test_config.global_test_params = {'PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY': key}
                        self.checker.default_controller.SendCommand.side_effect = (
                            InteractionModelError(model_status) if model_status != Status.Success else None)
                        with self.assertRaisesRegex(signals.TestSkip, 'constraint enforcement not demonstrated'):
                            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_awaited()
        self.checker.record_accepted_violation.assert_not_called()

    async def test_writable_attribute_checks_precede_unsupported_outcome(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        cluster = Clusters.OnOff
        attribute = cluster.Attributes.StartUpOnOff
        self.checker.endpoints_tlv[1] = {cluster.id: {
            GlobalAttributeIds.FEATURE_MAP_ID: 0,
            GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
            GlobalAttributeIds.ATTRIBUTE_LIST_ID: [attribute.attribute_id]}}
        for name in ('generate_constraint_violations', 'check_attribute_constraint'):
            setattr(self.checker, name, MethodType(getattr(IDMBaseTest, name), self.checker))
        self.checker.read_single_attribute_check_success = AsyncMock(return_value=0)
        self.checker.read_single_attribute = AsyncMock(return_value=0)
        self.checker.default_controller.WriteAttribute = AsyncMock(return_value=[SimpleNamespace(Status=Status.ConstraintError)])
        with self.assertRaisesRegex(signals.TestSkip, 'constraint enforcement not demonstrated'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        calls = self.checker.default_controller.WriteAttribute.await_args_list
        self.assertGreater(len(calls), 0)
        for call in calls:
            endpoint, written = call.kwargs['attributes'][0]
            self.assertEqual(endpoint, 1)
            self.assertIsInstance(written, attribute)
            self.assertNotIn(int(written.value), (0, 1, 2, 255))
        self.checker.default_controller.WriteAttribute.reset_mock()
        self.checker.default_controller.WriteAttribute.return_value = [SimpleNamespace(Status=Status.Success)]
        with self.assertRaisesRegex(signals.TestFailure, 'Writable attribute constraints not enforced'):
            await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.assertGreater(self.checker.default_controller.WriteAttribute.await_count, 0)
        self.checker.record_accepted_violation.assert_called()
        self.checker.default_controller.SendCommand.assert_not_awaited()

    async def test_exclusion_is_not_qualification_in_either_policy_era(self):
        from TC_IDM_9_1 import TC_IDM_9_1

        basic = self.checker.endpoints_tlv[0][Clusters.BasicInformation.id]
        for version in (SPEC_VERSION_1_7, SPEC_VERSION_1_7 - 1, None):
            with self.subTest(version=version):
                basic[Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id] = version or 0
                with self.assertRaises(signals.TestSkip):
                    await TC_IDM_9_1.test_TC_IDM_9_1.__wrapped__(self.checker)
        self.checker.default_controller.SendCommand.assert_not_awaited()


if __name__ == "__main__":
    unittest.main()
