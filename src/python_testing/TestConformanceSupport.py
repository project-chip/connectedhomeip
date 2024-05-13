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

from conformance_support import ConformanceDecision, ConformanceParseParameters, parse_callable_from_xml
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TestConformanceSupport(MatterBaseTest):
    @async_test_body
    async def setup_class(self):
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

    @async_test_body
    async def test_conformance_mandatory(self):
        xml = '<mandatoryConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
        asserts.assert_equal(str(xml_callable), 'M')

    @async_test_body
    async def test_conformance_optional(self):
        xml = '<optionalConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
        asserts.assert_equal(str(xml_callable), 'O')

    @async_test_body
    async def test_conformance_disallowed(self):
        xml = '<disallowConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.DISALLOWED)
        asserts.assert_equal(str(xml_callable), 'X')

        xml = '<deprecateConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.DISALLOWED)
        asserts.assert_equal(str(xml_callable), 'D')

    @async_test_body
    async def test_conformance_provisional(self):
        xml = '<provisionalConform />'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.PROVISIONAL)
        asserts.assert_equal(str(xml_callable), 'P')

    @async_test_body
    async def test_conformance_zigbee(self):
        xml = '<condition name="Zigbee"/>'
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for f in self.feature_maps:
            asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'Zigbee')

    @async_test_body
    async def test_conformance_mandatory_on_condition(self):
        xml = ('<mandatoryConform>'
               '<feature name="AB" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB')

        xml = ('<mandatoryConform>'
               '<feature name="CD" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'CD')

        # single attribute mandatory
        xml = ('<mandatoryConform>'
               '<attribute name="attr1" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr1[i]:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'attr1')

        xml = ('<mandatoryConform>'
               '<attribute name="attr2" />'
               '</mandatoryConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'attr2')

        # test command in optional and in boolean - this is the same as attribute essentially, so testing every permutation is overkill

    @async_test_body
    async def test_conformance_optional_on_condition(self):
        # single feature optional
        xml = ('<optionalConform>'
               '<feature name="AB" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_ab[i]:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[AB]')

        xml = ('<optionalConform>'
               '<feature name="CD" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, f in enumerate(self.feature_maps):
            if self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[CD]')

        # single attribute optional
        xml = ('<optionalConform>'
               '<attribute name="attr1" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr1[i]:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[attr1]')

        xml = ('<optionalConform>'
               '<attribute name="attr2" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, a in enumerate(self.attribute_lists):
            if self.has_attr2[i]:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[attr2]')

        # single command optional
        xml = ('<optionalConform>'
               '<command name="cmd1" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, c in enumerate(self.cmd_lists):
            if self.has_cmd1[i]:
                asserts.assert_equal(xml_callable(0x00, [], c), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, [], c), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[cmd1]')

        xml = ('<optionalConform>'
               '<command name="cmd2" />'
               '</optionalConform>')
        et = ElementTree.fromstring(xml)
        xml_callable = parse_callable_from_xml(et, self.params)
        for i, c in enumerate(self.cmd_lists):
            if self.has_cmd2[i]:
                asserts.assert_equal(xml_callable(0x00, [], c), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(0x00, [], c), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[cmd2]')

    @async_test_body
    async def test_conformance_not_term_mandatory(self):
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '!attr2')

    @async_test_body
    async def test_conformance_not_term_optional(self):
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!CD]')

    @async_test_body
    async def test_conformance_and_term(self):
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
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
                    asserts.assert_equal(xml_callable(f, a, []), ConformanceDecision.MANDATORY)
                else:
                    asserts.assert_equal(xml_callable(f, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB & attr2')

    @async_test_body
    async def test_conformance_or_term(self):
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)
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
                    asserts.assert_equal(xml_callable(f, a, []), ConformanceDecision.MANDATORY)
                else:
                    asserts.assert_equal(xml_callable(f, a, []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), 'AB | attr2')

    @async_test_body
    async def test_conformance_and_term_with_not(self):
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!AB & CD]')

    @async_test_body
    async def test_conformance_or_term_with_not(self):
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[!(AB | CD)]')

    @async_test_body
    async def test_conformance_and_term_with_three_terms(self):
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
        asserts.assert_equal(xml_callable(0x00, [], []), ConformanceDecision.NOT_APPLICABLE)
        # one feature
        asserts.assert_equal(xml_callable(0x01, [], []), ConformanceDecision.NOT_APPLICABLE)
        # all features
        asserts.assert_equal(xml_callable(0x07, [], []), ConformanceDecision.OPTIONAL)
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
                        asserts.assert_equal(xml_callable(f, a, c), ConformanceDecision.OPTIONAL)
                    else:
                        asserts.assert_equal(xml_callable(f, a, c), ConformanceDecision.NOT_APPLICABLE)
        asserts.assert_equal(str(xml_callable), '[AB & attr1 & cmd1]')

    @async_test_body
    async def test_conformance_or_term_with_three_terms(self):
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
        asserts.assert_equal(xml_callable(0x00, [], []), ConformanceDecision.NOT_APPLICABLE)
        # one feature
        asserts.assert_equal(xml_callable(0x01, [], []), ConformanceDecision.OPTIONAL)
        # all features
        asserts.assert_equal(xml_callable(0x07, [], []), ConformanceDecision.OPTIONAL)
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
                        asserts.assert_equal(xml_callable(f, a, c), ConformanceDecision.OPTIONAL)
                    else:
                        asserts.assert_equal(xml_callable(f, a, c), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            elif self.has_cd[i]:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)
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
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(xml_callable(f, [], []), ConformanceDecision.PROVISIONAL)
        asserts.assert_equal(str(xml_callable), 'AB & !CD, P')


if __name__ == "__main__":
    default_matter_test_main()
