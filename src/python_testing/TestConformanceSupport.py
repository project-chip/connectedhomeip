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

from chip.tlv import uint
from typing import Callable
from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
from conformance_support import parse_callable_from_xml, ConformanceDecision, ConformanceException, ConformanceParseParameters
import xml.etree.ElementTree as ElementTree
from mobly import asserts


class TestConformanceSupport(MatterBaseTest):
    @async_test_body
    async def test_conformance_from_xml(self):
        # a small feature map
        feature_names_to_bits = {'AB': 0x01, 'CD': 0x02}

        # none, AB, CD, AB&CD
        feature_maps = [0x00, 0x01, 0x02, 0x03]
        has_ab = [False, True, False, True]
        has_cd = [False, False, True, True]

        attribute_names_to_values = {'attr1': 0x00, 'attr2': 0x01}
        attribute_lists = [[], [0x00], [0x01], [0x00, 0x01]]
        has_attr1 = [False, True, False, True]
        has_attr2 = [False, False, True, True]

        command_names_to_values = {'cmd1': 0x00, 'cmd2': 0x01}
        cmd_lists = [[], [0x00], [0x01], [0x00, 0x01]]
        has_cmd1 = [False, True, False, True]
        has_cmd2 = [False, False, True, True]

        params = ConformanceParseParameters(
            feature_map=feature_names_to_bits, attribute_map=attribute_names_to_values, command_map=command_names_to_values)

        # Start simple - mandatory, optional, disallowed
        xml = '<mandatoryConform />'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for f in feature_maps:
            asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)

        xml = '<optionalConform />'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for f in feature_maps:
            asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)

        xml = '<disallowConform />'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for f in feature_maps:
            asserts.assert_equal(callable(f, [], []), ConformanceDecision.DISALLOWED)

        xml = '<deprecateConform />'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for f in feature_maps:
            asserts.assert_equal(callable(f, [], []), ConformanceDecision.DISALLOWED)

        xml = '<provisionalConform />'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for f in feature_maps:
            asserts.assert_equal(callable(f, [], []), ConformanceDecision.PROVISIONAL)

        # single feature mandatory
        xml = '<mandatoryConform>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<mandatoryConform>\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # single attribute mandatory
        xml = '<mandatoryConform>\n'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if has_attr1[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<mandatoryConform>\n'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if has_attr2[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        # test command in optional and in boolean - this is the same as attribute essentially, so testing every permutation is overkill

        # single feature optional
        xml = '<optionalConform>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<optionalConform>\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # single attribute optional
        xml = '<optionalConform>\n'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if has_attr1[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<optionalConform>\n'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if has_attr2[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        # single command optional
        xml = '<optionalConform>\n'
        xml = xml + '<command name="cmd1" />\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, c in enumerate(cmd_lists):
            if has_cmd1[i]:
                asserts.assert_equal(callable(0x00, [], c), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(0x00, [], c), ConformanceDecision.NOT_APPLICABLE)

        xml = '<optionalConform>\n'
        xml = xml + '<command name="cmd2" />\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, c in enumerate(cmd_lists):
            if has_cmd2[i]:
                asserts.assert_equal(callable(0x00, [], c), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(0x00, [], c), ConformanceDecision.NOT_APPLICABLE)

        # single feature not mandatory
        xml = '<mandatoryConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if not has_ab[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<mandatoryConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if not has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # single attribute not mandatory
        xml = '<mandatoryConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if not has_attr1[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<mandatoryConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if not has_attr2[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        # single feature not optional
        xml = '<optionalConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if not has_ab[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        xml = '<optionalConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if not has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # and term for features only
        xml = '<mandatoryConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i] and has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # and term for attributes only
        xml = '<mandatoryConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if has_attr1[i] and has_attr2[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        # and term for feature and attribute
        xml = '<mandatoryConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            for j, a in enumerate(attribute_lists):
                if has_ab[i] and has_attr2[j]:
                    asserts.assert_equal(callable(f, a, []), ConformanceDecision.MANDATORY)
                else:
                    asserts.assert_equal(callable(f, a, []), ConformanceDecision.NOT_APPLICABLE)

        # or term feature only
        xml = '<mandatoryConform>\n'
        xml = xml + '<orTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</orTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i] or has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # or term attribute only
        xml = '<mandatoryConform>\n'
        xml = xml + '<orTerm>\n'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</orTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, a in enumerate(attribute_lists):
            if has_attr1[i] or has_attr2[i]:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(0x00, a, []), ConformanceDecision.NOT_APPLICABLE)

        # or term feature and attribute
        xml = '<mandatoryConform>\n'
        xml = xml + '<orTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<attribute name="attr2" />\n'
        xml = xml + '</orTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            for j, a in enumerate(attribute_lists):
                if has_ab[i] or has_attr2[j]:
                    asserts.assert_equal(callable(f, a, []), ConformanceDecision.MANDATORY)
                else:
                    asserts.assert_equal(callable(f, a, []), ConformanceDecision.NOT_APPLICABLE)

        # and term with not
        xml = '<optionalConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if not has_ab[i] and has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # or term with not on second feature
        xml = '<mandatoryConform>\n'
        xml = xml + '<orTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<notTerm>'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</notTerm>'
        xml = xml + '</orTerm>\n'
        xml = xml + '</mandatoryConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i] or not has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # not around or term with
        xml = '<optionalConform>\n'
        xml = xml + '<notTerm>'
        xml = xml + '<orTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</orTerm>\n'
        xml = xml + '</notTerm>'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if not (has_ab[i] or has_cd[i]):
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # and term with three features
        xml = '<optionalConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '<feature name="EF" />\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</optionalConform>'
        feature_names_to_bits['EF'] = 0x04
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        # no features
        asserts.assert_equal(callable(0x00, [], []), ConformanceDecision.NOT_APPLICABLE)
        # one feature
        asserts.assert_equal(callable(0x01, [], []), ConformanceDecision.NOT_APPLICABLE)
        # all features
        asserts.assert_equal(callable(0x07, [], []), ConformanceDecision.OPTIONAL)

        # and term with one of each
        xml = '<optionalConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '<command name="cmd1" />\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            for j, a in enumerate(attribute_lists):
                for k, c in enumerate(cmd_lists):
                    if has_ab[i] and has_attr1[j] and has_cmd1[k]:
                        asserts.assert_equal(callable(f, a, c), ConformanceDecision.OPTIONAL)
                    else:
                        asserts.assert_equal(callable(f, a, c), ConformanceDecision.NOT_APPLICABLE)

        # or term with three features
        xml = '<optionalConform>\n'
        xml = xml + '<orTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '<feature name="EF" />\n'
        xml = xml + '</orTerm>\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        # no features
        asserts.assert_equal(callable(0x00, [], []), ConformanceDecision.NOT_APPLICABLE)
        # one feature
        asserts.assert_equal(callable(0x01, [], []), ConformanceDecision.OPTIONAL)
        # all features
        asserts.assert_equal(callable(0x07, [], []), ConformanceDecision.OPTIONAL)

        # or term with one of each
        xml = '<optionalConform>\n'
        xml = xml + '<orTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<attribute name="attr1" />\n'
        xml = xml + '<command name="cmd1" />\n'
        xml = xml + '</orTerm>\n'
        xml = xml + '</optionalConform>'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            for j, a in enumerate(attribute_lists):
                for k, c in enumerate(cmd_lists):
                    if has_ab[i] or has_attr1[j] or has_cmd1[k]:
                        asserts.assert_equal(callable(f, a, c), ConformanceDecision.OPTIONAL)
                    else:
                        asserts.assert_equal(callable(f, a, c), ConformanceDecision.NOT_APPLICABLE)

    def test_otherwise_conformance_from_xml(self):
        # a small feature map
        feature_names_to_bits = {'AB': 0x01, 'CD': 0x02}

        # none, AB, CD, AB&CD
        feature_maps = [0x00, 0x01, 0x02, 0x03]
        has_ab = [False, True, False, True]
        has_cd = [False, False, True, True]

        attribute_names_to_values = {'attr1': 0x00, 'attr2': 0x01}
        attribute_lists = [[], [0x00], [0x01], [0x01, 0x02]]
        has_attr1 = [False, True, False, True]
        has_attr2 = [False, False, True, True]

        command_names_to_values = {'cmd1': 0x00, 'cmd2': 0x01}
        cmd_lists = [[], [0x00], [0x01], [0x00, 0x01]]
        has_cmd1 = [False, True, False, True]
        has_cmd2 = [False, False, True, True]

        params = ConformanceParseParameters(
            feature_map=feature_names_to_bits, attribute_map=attribute_names_to_values, command_map=command_names_to_values)

        # AB, O
        xml = '<otherwiseConform>\n'
        xml = xml + '<mandatoryConform>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</mandatoryConform>\n'
        xml = xml + '<optionalConform />\n'
        xml = xml + '</otherwiseConform>\n'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)

        # AB, [CD]
        xml = '<otherwiseConform>\n'
        xml = xml + '<mandatoryConform>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '</mandatoryConform>\n'
        xml = xml + '<optionalConform>\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</optionalConform>\n'
        xml = xml + '</otherwiseConform>\n'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            elif has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.OPTIONAL)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.NOT_APPLICABLE)

        # AB & !CD, P
        xml = '<otherwiseConform>\n'
        xml = xml + '<mandatoryConform>\n'
        xml = xml + '<andTerm>\n'
        xml = xml + '<feature name="AB" />\n'
        xml = xml + '<notTerm>\n'
        xml = xml + '<feature name="CD" />\n'
        xml = xml + '</notTerm>\n'
        xml = xml + '</andTerm>\n'
        xml = xml + '</mandatoryConform>\n'
        xml = xml + '<provisionalConform />\n'
        xml = xml + '</otherwiseConform>\n'
        et = ElementTree.fromstring(xml)
        callable = parse_callable_from_xml(et, params)
        for i, f in enumerate(feature_maps):
            if has_ab[i] and not has_cd[i]:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.MANDATORY)
            else:
                asserts.assert_equal(callable(f, [], []), ConformanceDecision.DISALLOWED)


if __name__ == "__main__":
    default_matter_test_main()
