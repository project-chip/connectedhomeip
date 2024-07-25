#
#    Copyright (c) 2023 Project CHIP Authors
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

import xml.etree.ElementTree as ElementTree
from typing import Callable

from chip.tlv import uint
from conformance_support import (Choice, Conformance, ConformanceDecision, ConformanceException, ConformanceParseParameters,
                                 deprecated, disallowed, mandatory, optional, parse_basic_callable_from_xml,
                                 parse_callable_from_xml, parse_device_type_callable_from_xml, provisional, zigbee)
from matter_testing_support import MatterBaseTest, default_matter_test_main
from mobly import asserts


def basic_test(xml: str, cls: Callable) -> None:
    et = ElementTree.fromstring(xml)
    xml_callable = parse_basic_callable_from_xml(et)
    asserts.assert_true(isinstance(xml_callable, cls), "Unexpected class parsed from basic conformance")


class TestConformanceSupport(MatterBaseTest):
    def setup_class(self):
        super().setup_class()
        # a small feature map
        self.feature_names_to_bits = {'AB': 0x01, 'CD': 0x02}

        # none, AB, CD, AB&CD
        self.feature_maps = [0x00, 0x01, 0x02, 0x03]
        self.has_ab = [False, True, False, True]
        self.has_cd = [False, False, True, True]

        self.attribute_names_to_values = {'attr1': 0x00, 'attr2': 0x01}
        self.attribute_lists = [[], [0x00], [0x01], [0x00, 0x01]]
        self.has_attr1 = [False, True, False, True]
        self.has_attr2 = [False, False, True, True]

        self.command_names_to_values = {'cmd1': 0x00, 'cmd2': 0x01}
        self.cmd_lists = [[], [0x00], [0x01], [0x00, 0x01]]
        self.has_cmd1 = [False, True, False, True]
        self.has_cmd2 = [False, False, True, True]
        self.params = ConformanceParseParameters(
            feature_map=self.feature_names_to_bits, attribute_map=self.attribute_names_to_values, command_map=self.command_names_to_values)

    def test_conformance_mandatory(self):
        xml = '<mandatoryConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
        asserts.assert_equal(str(xml_callable), 'M')

    def test_conformance_optional(self):
        xml = '<optionalConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
        asserts.assert_equal(str(xml_callable), 'O')

    def test_conformance_disallowed(self):
        xml = '<disallowConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.DISALLOWED)
        asserts.assert_equal(str(xml_callable), 'X')

        xml = '<deprecateConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.DISALLOWED)
        asserts.assert_equal(str(xml_callable), 'D')

    def test_conformance_provisional(self):
        xml = '<provisionalConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.PROVISIONAL)
        asserts.assert_equal(str(xml_callable), 'P')

    def test_conformance_zigbee(self):
        xml = '<condition name="Zigbee"/>'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'Zigbee')

    def test_conformance_mandatory_on_condition(self):
        xml = ('<mandatoryConform>'
               '<feature name="AB" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB')

        xml = ('<mandatoryConform>'
               '<feature name="CD" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'CD')

        # single attribute mandatory
        xml = ('<mandatoryConform>'
               '<attribute name="attr1" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr1[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'attr1')

        xml = ('<mandatoryConform>'
               '<attribute name="attr2" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'attr2')

        # test command in optional and in boolean - this is the same as attribute essentially, so testing every permutation is overkill

    def test_conformance_optional_on_condition(self):
        # single feature optional
        xml = ('<optionalConform>'
               '<feature name="AB" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[AB]')

        xml = ('<optionalConform>'
               '<feature name="CD" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[CD]')

        # single attribute optional
        xml = ('<optionalConform>'
               '<attribute name="attr1" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr1[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[attr1]')

        xml = ('<optionalConform>'
               '<attribute name="attr2" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[attr2]')

        # single command optional
        xml = ('<optionalConform>'
               '<command name="cmd1" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, c in enumerate(self.cmd_lists):
            if self.has_cmd1[i]:
                asserts.assert_equal(xml_callable(0x00, [], c).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, [], c).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[cmd1]')

        xml = ('<optionalConform>'
               '<command name="cmd2" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, c in enumerate(self.cmd_lists):
            if self.has_cmd2[i]:
                asserts.assert_equal(xml_callable(0x00, [], c).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, [], c).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[cmd2]')

    def test_conformance_not_term_mandatory(self):
        # single feature not mandatory
        xml = ('<mandatoryConform>'
               '<notTerm>'
               '<feature name="AB" />'
               '</notTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if not self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '!AB')

        xml = ('<mandatoryConform>'
               '<notTerm>'
               '<feature name="CD" />'
               '</notTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if not self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '!CD')

        # single attribute not mandatory
        xml = ('<mandatoryConform>'
               '<notTerm>'
               '<attribute name="attr1" />'
               '</notTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if not self.has_attr1[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '!attr1')

        xml = ('<mandatoryConform>'
               '<notTerm>'
               '<attribute name="attr2" />'
               '</notTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if not self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '!attr2')

    def test_conformance_not_term_optional(self):
        # single feature not optional
        xml = ('<optionalConform>'
               '<notTerm>'
               '<feature name="AB" />'
               '</notTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if not self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!AB]')

        xml = ('<optionalConform>'
               '<notTerm>'
               '<feature name="CD" />'
               '</notTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if not self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!CD]')

    def test_conformance_and_term(self):
        # and term for features only
        xml = ('<mandatoryConform>'
               '<andTerm>'
               '<feature name="AB" />'
               '<feature name="CD" />'
               '</andTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i] and self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB & CD')

        # and term for attributes only
        xml = ('<mandatoryConform>'
               '<andTerm>'
               '<attribute name="attr1" />'
               '<attribute name="attr2" />'
               '</andTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr1[i] and self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'attr1 & attr2')

        # and term for feature and attribute
        xml = ('<mandatoryConform>'
               '<andTerm>'
               '<feature name="AB" />'
               '<attribute name="attr2" />'
               '</andTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            for j, a in enumerate(self.attribute_lists):
                if self.has_ab[i] and self.has_attr2[j]:
                    asserts.assert_equal(xml_callable(f, a, []).decision, ConformanceDecision.MANDATORY)
                else:
                    asserts.assert_equal(xml_callable(f, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB & attr2')

    def test_conformance_or_term(self):
        # or term feature only
        xml = ('<mandatoryConform>'
               '<orTerm>'
               '<feature name="AB" />'
               '<feature name="CD" />'
               '</orTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i] or self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB | CD')

        # or term attribute only
        xml = ('<mandatoryConform>'
               '<orTerm>'
               '<attribute name="attr1" />'
               '<attribute name="attr2" />'
               '</orTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr1[i] or self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'attr1 | attr2')

        # or term feature and attribute
        xml = ('<mandatoryConform>'
               '<orTerm>'
               '<feature name="AB" />'
               '<attribute name="attr2" />'
               '</orTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            for j, a in enumerate(self.attribute_lists):
                if self.has_ab[i] or self.has_attr2[j]:
                    asserts.assert_equal(xml_callable(f, a, []).decision, ConformanceDecision.MANDATORY)
                else:
                    asserts.assert_equal(xml_callable(f, a, []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB | attr2')

    def test_conformance_and_term_with_not(self):
        # and term with not
        xml = ('<optionalConform>'
               '<andTerm>'
               '<notTerm>'
               '<feature name="AB" />'
               '</notTerm>'
               '<feature name="CD" />'
               '</andTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if not self.has_ab[i] and self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!AB & CD]')

    def test_conformance_or_term_with_not(self):
        # or term with not on second feature
        xml = ('<mandatoryConform>'
               '<orTerm>'
               '<feature name="AB" />'
               '<notTerm>'
               '<feature name="CD" />'
               '</notTerm>'
               '</orTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i] or not self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB | !CD')

        # not around or term with
        xml = ('<optionalConform>'
               '<notTerm>'
               '<orTerm>'
               '<feature name="AB" />'
               '<feature name="CD" />'
               '</orTerm>'
               '</notTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if not (self.has_ab[i] or self.has_cd[i]):
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!(AB | CD)]')

    def test_conformance_and_term_with_three_terms(self):
        # and term with three features
        xml = ('<optionalConform>'
               '<andTerm>'
               '<feature name="AB" />'
               '<feature name="CD" />'
               '<feature name="EF" />'
               '</andTerm>'
               '</optionalConform>')
        self.feature_names_to_bits['EF'] = 0x04
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        # no features
        asserts.assert_equal(xml_callable(0x00, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        # one feature
        asserts.assert_equal(xml_callable(0x01, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        # all features
        asserts.assert_equal(xml_callable(0x07, [], []).decision, ConformanceDecision.OPTIONAL)
        asserts.assert_equal(str(xml_callable), '[AB & CD & EF]')

        # and term with one of each
        xml = ('<optionalConform>'
               '<andTerm>'
               '<feature name="AB" />'
               '<attribute name="attr1" />'
               '<command name="cmd1" />'
               '</andTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            for j, a in enumerate(self.attribute_lists):
                for k, c in enumerate(self.cmd_lists):
                    if self.has_ab[i] and self.has_attr1[j] and self.has_cmd1[k]:
                        asserts.assert_equal(xml_callable(f, a, c).decision, ConformanceDecision.OPTIONAL)
                    else:
                        asserts.assert_equal(xml_callable(f, a, c).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[AB & attr1 & cmd1]')

    def test_conformance_or_term_with_three_terms(self):
        # or term with three features
        xml = ('<optionalConform>'
               '<orTerm>'
               '<feature name="AB" />'
               '<feature name="CD" />'
               '<feature name="EF" />'
               '</orTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        # no features
        asserts.assert_equal(xml_callable(0x00, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        # one feature
        asserts.assert_equal(xml_callable(0x01, [], []).decision, ConformanceDecision.OPTIONAL)
        # all features
        asserts.assert_equal(xml_callable(0x07, [], []).decision, ConformanceDecision.OPTIONAL)
        asserts.assert_equal(str(xml_callable), '[AB | CD | EF]')

        # or term with one of each
        xml = ('<optionalConform>'
               '<orTerm>'
               '<feature name="AB" />'
               '<attribute name="attr1" />'
               '<command name="cmd1" />'
               '</orTerm>'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            for j, a in enumerate(self.attribute_lists):
                for k, c in enumerate(self.cmd_lists):
                    if self.has_ab[i] or self.has_attr1[j] or self.has_cmd1[k]:
                        asserts.assert_equal(xml_callable(f, a, c).decision, ConformanceDecision.OPTIONAL)
                    else:
                        asserts.assert_equal(xml_callable(f, a, c).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[AB | attr1 | cmd1]')

    def test_conformance_otherwise(self):
        # AB, O
        xml = ('<otherwiseConform>'
               '<mandatoryConform>'
               '<feature name="AB" />'
               '</mandatoryConform>'
               '<optionalConform />'
               '</otherwiseConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
        asserts.assert_equal(str(xml_callable), 'AB, O')

        # AB, [CD]
        xml = ('<otherwiseConform>'
               '<mandatoryConform>'
               '<feature name="AB" />'
               '</mandatoryConform>'
               '<optionalConform>'
               '<feature name="CD" />'
               '</optionalConform>'
               '</otherwiseConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            elif self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB, [CD]')

        # AB & !CD, P
        xml = ('<otherwiseConform>'
               '<mandatoryConform>'
               '<andTerm>'
               '<feature name="AB" />'
               '<notTerm>'
               '<feature name="CD" />'
               '</notTerm>'
               '</andTerm>'
               '</mandatoryConform>'
               '<provisionalConform />'
               '</otherwiseConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i] and not self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []).decision, ConformanceDecision.PROVISIONAL)
        asserts.assert_equal(str(xml_callable), 'AB & !CD, P')

    def test_conformance_greater(self):
        # AB, [CD]
        xml = ('<mandatoryConform>'
               '<greaterTerm>'
               '<attribute name="attr1" />'
               '<literal value="1" />'
               '</greaterTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        # TODO: switch this to check greater than once the update to the base is done (#33422)
        asserts.assert_equal(xml_callable(0x00, [], []).decision, ConformanceDecision.OPTIONAL)
        asserts.assert_equal(str(xml_callable), 'attr1 > 1')

        # Ensure that we can only have greater terms with exactly 2 value
        xml = ('<mandatoryConform>'
               '<greaterTerm>'
               '<attribute name="attr1" />'
               '<attribute name="attr2" />'
               '<literal value="1" />'
               '</greaterTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        try:
            xml_callable = parse_callable_from_xml(et, self.params)
            asserts.fail("Incorrectly parsed bad greaterTerm XML with > 2 values")
        except ConformanceException:
            pass

        xml = ('<mandatoryConform>'
               '<greaterTerm>'
               '<attribute name="attr1" />'
               '</greaterTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        try:
            xml_callable = parse_callable_from_xml(et, self.params)
            asserts.fail("Incorrectly parsed bad greaterTerm XML with < 2 values")
        except ConformanceException:
            pass

        # Only attributes and literals allowed because arithmetic operations require values
        xml = ('<mandatoryConform>'
               '<greaterTerm>'
               '<feature name="AB" />'
               '<literal value="1" />'
               '</greaterTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        try:
            xml_callable = parse_callable_from_xml(et, self.params)
            asserts.fail("Incorrectly parsed greater term with feature value")
        except ConformanceException:
            pass

    def test_basic_conformance(self):
        basic_test('<mandatoryConform />', mandatory)
        basic_test('<optionalConform />', optional)
        basic_test('<disallowConform />', disallowed)
        basic_test('<deprecateConform />', deprecated)
        basic_test('<provisionalConform />', provisional)
        basic_test('<condition name="zigbee" />', zigbee)

        # feature is not basic so we should get an exception
        xml = '<feature name="CD" />'
        et = ElementTree.fromstring(xml)
        try:
            parse_basic_callable_from_xml(et)
            asserts.fail("Unexpected success parsing non-basic conformance")
        except ConformanceException:
            pass

        # mandatory tag is basic, but this one is a wrapper, so we should get a TypeError
        xml = ('<mandatoryConform>'
               '<andTerm>'
               '<feature name="AB" />'
               '<notTerm>'
               '<feature name="CD" />'
               '</notTerm>'
               '</andTerm>'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        try:
            parse_basic_callable_from_xml(et)
            asserts.fail("Unexpected success parsing mandatory wrapper")
        except ConformanceException:
            pass

    def test_device_type_conformance(self):
        msg = "Unexpected conformance returned for device type"
        xml = ('<mandatoryConform>'
               '<condition name="zigbee" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_device_type_callable_from_xml(et)
        asserts.assert_equal(str(xml_callable), 'Zigbee', msg)
        asserts.assert_equal(xml_callable(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg)

        xml = ('<optionalConform>'
               '<condition name="zigbee" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_device_type_callable_from_xml(et)
        # expect no exception here
        asserts.assert_equal(str(xml_callable), '[Zigbee]', msg)
        asserts.assert_equal(xml_callable(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg)

        # otherwise conforms are allowed
        xml = ('<otherwiseConform>'
               '<condition name="zigbee" />'
               '<provisionalConform />'
               '</otherwiseConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_device_type_callable_from_xml(et)
        # expect no exception here
        asserts.assert_equal(str(xml_callable), 'Zigbee, P', msg)
        asserts.assert_equal(xml_callable(0, [], []).decision, ConformanceDecision.PROVISIONAL, msg)

        # Device type conditions or features don't correspond to anything in the spec, so the XML takes a best
        # guess as to what they are. We should be able to parse features, conditions, attributes as the same
        # thing.
        # TODO: allow querying conformance for conditional device features
        # TODO: adjust conformance call function to accept a list of features and evaluate based on that
        xml = ('<mandatoryConform>'
               '<feature name="CD" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_device_type_callable_from_xml(et)
        asserts.assert_equal(str(xml_callable), 'CD', msg)
        # Device features are always optional (at least for now), even though we didn't pass this feature in
        asserts.assert_equal(xml_callable(0, [], []).decision, ConformanceDecision.OPTIONAL)

        xml = ('<otherwiseConform>'
               '<feature name="CD" />'
               '<condition name="testy" />'
               '</otherwiseConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_device_type_callable_from_xml(et)
        asserts.assert_equal(str(xml_callable), 'CD, testy', msg)
        asserts.assert_equal(xml_callable(0, [], []).decision, ConformanceDecision.OPTIONAL)

    def check_good_choice(self, xml: str, conformance_str: str) -> Conformance:
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        asserts.assert_equal(str(xml_callable), conformance_str, 'Bad choice conformance string')
        return xml_callable

    def check_decision(self, more_expected: bool, conformance: Conformance, feature_map: uint, attr_list: list[uint], cmd_list: list[uint]):
        decision = conformance(feature_map, attr_list, cmd_list)
        asserts.assert_true(decision.choice, 'Expected choice conformance on decision, but did not get one')
        asserts.assert_equal(decision.choice.marker, 'a', 'Unexpected conformance string returned')
        asserts.assert_equal(decision.choice.more, more_expected, "Unexpected more on choice")

    def test_choice_conformance(self):
        # Choice conformances can appear on:
        # - base optional O.a
        # - base optional feature [AB].a
        # - base optional attribute [attr1].a
        # - base optional command [cmd1].a
        # - optional wrapper of complex feature [AB | CD].a, [!attr1].a
        # - otherwise conformance attr1, [AB], O.a / attr1, [AB].a, O
        # - multiple in otherwise [AB].a, [CD].b
        #
        # Choice conformances are disallowed on:
        # - mandatory M.a
        # - mandatory feature AB.a
        # - mandatory attribute attr1.a
        # - mandatory command cmd1.a
        # - AND expressions (attr1 & O.a)
        # - OR expressions (attr1 | O.a)
        # - NOT expressions (!O.a)
        # - internal expressions [AB.a], [attr1.a], [cmd1.a]
        # - provisional P.a
        # - disallowed X.a
        # - deprecated D.a

        choices = [('a+', 'choice="a" more="true"', True), ('a', 'choice="a"', False)]
        for suffix, xml_attrs, more in choices:

            AB = self.feature_names_to_bits['AB']
            attr1 = [self.attribute_names_to_values['attr1']]
            cmd1 = [self.command_names_to_values['cmd1']]

            msg_not_applicable = "Expected NOT_APPLICABLE conformance"
            xml = f'<optionalConform {xml_attrs} />'
            conformance = self.check_good_choice(xml, f'O.{suffix}')
            self.check_decision(more, conformance, 0, [], [])

            xml = (f'<optionalConform {xml_attrs}>'
                   '<feature name="AB" />'
                   '</optionalConform>')
            conformance = self.check_good_choice(xml, f'[AB].{suffix}')
            asserts.assert_equal(conformance(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg_not_applicable)
            self.check_decision(more, conformance, AB, [], [])

            xml = (f'<optionalConform {xml_attrs}>'
                   '<attribute name="attr1" />'
                   '</optionalConform>')
            conformance = self.check_good_choice(xml, f'[attr1].{suffix}')
            asserts.assert_equal(conformance(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg_not_applicable)
            self.check_decision(more, conformance, 0, attr1, [])

            xml = (f'<optionalConform {xml_attrs}>'
                   '<command name="cmd1" />'
                   '</optionalConform>')
            conformance = self.check_good_choice(xml, f'[cmd1].{suffix}')
            asserts.assert_equal(conformance(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg_not_applicable)
            self.check_decision(more, conformance, 0, [], cmd1)

            xml = (f'<optionalConform {xml_attrs}>'
                   '<orTerm>'
                   '<feature name="AB" />'
                   '<feature name="CD" />'
                   '</orTerm>'
                   '</optionalConform>')
            conformance = self.check_good_choice(xml, f'[AB | CD].{suffix}')
            asserts.assert_equal(conformance(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg_not_applicable)
            self.check_decision(more, conformance, AB, [], [])

            xml = (f'<optionalConform {xml_attrs}>'
                   '<notTerm>'
                   '<attribute name="attr1" />'
                   '</notTerm>'
                   '</optionalConform>')
            conformance = self.check_good_choice(xml, f'[!attr1].{suffix}')
            asserts.assert_equal(conformance(0, attr1, []).decision, ConformanceDecision.NOT_APPLICABLE, msg_not_applicable)
            self.check_decision(more, conformance, 0, [], [])

            xml = ('<otherwiseConform>'
                   '<attribute name="attr1" />'
                   f'<optionalConform>'
                   '<feature name="AB" />'
                   '</optionalConform>'
                   f'<optionalConform {xml_attrs} />'
                   '</otherwiseConform>')
            conformance = self.check_good_choice(xml, f'attr1, [AB], O.{suffix}')
            # with no features or attributes, this should end up as O.a, so there should be a choice
            self.check_decision(more, conformance, 0, [], [])
            # when we have this attribute, we should not have a choice
            asserts.assert_equal(conformance(0, attr1, []).decision, ConformanceDecision.MANDATORY, 'Unexpected conformance')
            asserts.assert_equal(conformance(0, attr1, []).choice, None, 'Unexpected choice in conformance')
            # when we have only this feature, we should not have a choice
            asserts.assert_equal(conformance(AB, [], []).decision, ConformanceDecision.OPTIONAL, 'Unexpected conformance')
            asserts.assert_equal(conformance(AB, [], []).choice, None, 'Unexpected choice in conformance')

            # - multiple in otherwise [AB].a, [CD].b
            xml = ('<otherwiseConform>'
                   '<attribute name="attr1" />'
                   f'<optionalConform {xml_attrs}>'
                   '<feature name="AB" />'
                   '</optionalConform>'
                   f'<optionalConform choice="b">'
                   '<feature name="CD" />'
                   '</optionalConform>'
                   '</otherwiseConform>')
            conformance = self.check_good_choice(xml, f'attr1, [AB].{suffix}, [CD].b')
            asserts.assert_equal(conformance(0, [], []).decision, ConformanceDecision.NOT_APPLICABLE, msg_not_applicable)
            # when we have this attribute, we should not have a choice
            asserts.assert_equal(conformance(0, attr1, []).decision, ConformanceDecision.MANDATORY, 'Unexpected conformance')
            asserts.assert_equal(conformance(0, attr1, []).choice, None, 'Unexpected choice in conformance')
            # When it's just AB, we should have a choice
            self.check_decision(more, conformance, AB, [], [])
            # When we have both the attribute and AB, we should not have a choice
            asserts.assert_equal(conformance(0, attr1, []).decision, ConformanceDecision.MANDATORY, 'Unexpected conformance')
            asserts.assert_equal(conformance(0, attr1, []).choice, None, 'Unexpected choice in conformance')
            # When we have AB and CD, we should be using the AB choice
            CD = self.feature_names_to_bits['CD']
            ABCD = AB | CD
            self.check_decision(more, conformance, ABCD, [], [])
            # When we just have CD, we still have a choice, but the string should be b
            asserts.assert_equal(conformance(CD, [], []).decision, ConformanceDecision.OPTIONAL, 'Unexpected conformance')
            asserts.assert_equal(conformance(CD, [], []).choice, Choice('b', False), 'Unexpected choice in conformance')

            # Ones that should throw exceptions

            def check_bad_choice(xml: str):
                msg = f'Choice conformance string should cause exception, but did not: {xml}'
                et = ElementTree.fromstring(xml)
                try:
                    parse_callable_from_xml(et, self.params)
                    asserts.fail(msg)
                except ConformanceException:
                    pass
            xml = f'<mandatoryConform {xml_attrs} />'
            check_bad_choice(xml)

            xml = f'<feature name="AB" {xml_attrs} />'
            check_bad_choice(xml)

            xml = f'<attribute name="attr1" {xml_attrs} />'
            check_bad_choice(xml)

            xml = f'<command name="cmd1" {xml_attrs} />'
            check_bad_choice(xml)

            xml = ('<mandatoryConform>'
                   '<andTerm>'
                   '<attribute name="attr1"/>'
                   f'<optionalConform {xml_attrs} />'
                   '</andTerm>'
                   '</mandatoryConform>')
            check_bad_choice(xml)

            xml = ('<mandatoryConform>'
                   '<orTerm>'
                   '<attribute name="attr1"/>'
                   f'<optionalConform {xml_attrs} />'
                   '</orTerm>'
                   '</mandatoryConform>')
            check_bad_choice(xml)

            xml = ('<mandatoryConform>'
                   '<notTerm>'
                   f'<optionalConform {xml_attrs} />'
                   '</notTerm>'
                   '</mandatoryConform>')
            check_bad_choice(xml)

            xml = ('<optionalConform>'
                   f'<feature name="AB" {xml_attrs}/>'
                   '</optionalConform>')
            check_bad_choice(xml)

            xml = ('<optionalConform>'
                   f'<attribute name="attr1" {xml_attrs}/>'
                   '</optionalConform>')
            check_bad_choice(xml)

            xml = ('<optionalConform>'
                   f'<command name="cmd1" {xml_attrs}/>'
                   '</optionalConform>')
            check_bad_choice(xml)

            xml = (f'<provisionalConform {xml_attrs}/>')
            check_bad_choice(xml)

            xml = (f'<disallowConform {xml_attrs}/>')
            check_bad_choice(xml)

            xml = (f'<deprecateConform {xml_attrs}/>')
            check_bad_choice(xml)


if __name__ == "__main__":
    default_matter_test_main()
