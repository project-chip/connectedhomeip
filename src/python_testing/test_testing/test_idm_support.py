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

import matter.clusters as Clusters
from matter.testing.problem_notices import UnknownProblemLocation
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters
from matter.tlv import TLVReader

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.idm_support import (SPEC_VERSION_1_7, IDMBaseTest, _codegen_enum_values, enum_in_type,  # noqa: E402
                                         smallest_legal_enum_value, spec_enum_values, undefined_enum_values)


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


if __name__ == "__main__":
    unittest.main()
