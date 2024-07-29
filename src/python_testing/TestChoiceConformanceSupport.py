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

import itertools
import xml.etree.ElementTree as ElementTree

import jinja2
from choice_conformance_support import (evaluate_attribute_choice_conformance, evaluate_command_choice_conformance,
                                        evaluate_feature_choice_conformance)
from matter_testing_support import MatterBaseTest, ProblemNotice, default_matter_test_main
from mobly import asserts
from spec_parsing_support import XmlCluster, add_cluster_data_from_xml

FEATURE_TEMPLATE = '''\
    <feature bit="{{ id }}" code="{{ name }}" name="{{ name }}" summary="summary">
      <optionalConform choice="{{ choice }}" more="{{ more }}">
      {%- if XXX %}'
       <feature name="XXX" />
      {% endif %}
      </optionalConform>
    </feature>
'''

ATTRIBUTE_TEMPLATE = (
    '    <attribute id="{{ id }}" name="{{ name }}" type="uint16">\n'
    '      <optionalConform choice="{{ choice }}" more="{{ more }}">\n'
    '    {% if XXX %}'
    '        <attribute name="XXX" />\n'
    '    {% endif %}'
    '    </optionalConform>\n'
    '    </attribute>\n'
)

COMMAND_TEMPLATE = (
    '    <command id="{{ id }}" name="{{ name }}" direction="commandToServer" response="Y">\n'
    '      <optionalConform choice="{{ choice }}" more="{{ more }}">\n'
    '    {% if XXX %}'
    '        <command name="XXX" />\n'
    '    {% endif %}'
    '    </optionalConform>\n'
    '    </command>\n'
)

CLUSTER_TEMPLATE = (
    '<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="0x0001" name="Test Base" revision="1">\n'
    '  <revisionHistory>\n'
    '    <revision revision="1" summary="Initial version"/>\n'
    '  </revisionHistory>\n'
    '  <clusterIds>\n'
    '    <clusterId id="0x0001" name="Test Base"/>\n'
    '  </clusterIds>\n'
    '  <classification hierarchy="base" role="application" picsCode="BASE" scope="Endpoint"/>\n'
    '  <features>\n'
    '    {{ feature_string }}\n'
    '  </features>\n'
    '  <attributes>\n'
    '    {{ attribute_string}}\n'
    '  </attributes>\n'
    '  <commands>\n'
    '    {{ command_string }}\n'
    '  </commands>\n'
    '</cluster>\n')


def _create_elements(template_str: str, base_name: str) -> list[str]:
    xml_str = []

    def add_elements(curr_choice: str, starting_id: int, more: str, XXX: bool):
        for i in range(3):
            element_name = f'{base_name}{curr_choice.upper()*(i+1)}'
            environment = jinja2.Environment()
            template = environment.from_string(template_str)
            xml_str.append(template.render(id=(i + starting_id), name=element_name, choice=curr_choice, more=more, XXX=XXX))
    add_elements('a', 1, 'false', False)
    add_elements('b', 4, 'true', False)
    add_elements('c', 7, 'false', True)
    add_elements('d', 10, 'true', True)

    return xml_str

# TODO: this setup makes my life easy because it assumes that choice conformances apply only within one table
# if this is not true (ex, you can have choose 1 of a feature or an attribute), then this gets more complex
# in this case we need to have this test evaluate the same choice conformance value between multiple tables, and all
# the conformances need to be assessed for the entire element set.
# I've done it this way specifically so I can hardcode the choice values and test the features, attributes and commands
# separately, even though I load them all at the start.

# Cluster with choices on all elements
# 3 of each element with O.a
# 3 of each element with O.b+
# 3 of each element with [XXX].c
# 3 of each element with [XXX].d+
# 1 element named XXX


def _create_features():
    xml = _create_elements(FEATURE_TEMPLATE, 'F')
    xxx = ('    <feature bit="13" code="XXX" name="XXX" summary="summary">\n'
           '      <optionalConform />\n'
           '    </feature>\n')
    xml.append(xxx)
    return '\n'.join(xml)


def _create_attributes():
    xml = _create_elements(ATTRIBUTE_TEMPLATE, "attr")
    xxx = ('    <attribute id="13" name="XXX" summary="summary">\n'
           '      <optionalConform />\n'
           '    </attribute>\n')
    xml.append(xxx)
    return '\n'.join(xml)


def _create_commands():
    xml = _create_elements(COMMAND_TEMPLATE, 'cmd')
    xxx = ('    <command id="13" name="XXX" summary="summary" direction="commandToServer" response="Y">\n'
           '      <optionalConform />\n'
           '    </command>\n')
    xml.append(xxx)
    return '\n'.join(xml)


def _create_cluster():
    environment = jinja2.Environment()
    template = environment.from_string(CLUSTER_TEMPLATE)
    return template.render(feature_string=_create_features(), attribute_string=_create_attributes(), command_string=_create_commands())


class TestConformanceSupport(MatterBaseTest):
    def setup_class(self):
        super().setup_class()

        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        cluster_xml = ElementTree.fromstring(_create_cluster())
        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)
        self.clusters = clusters
        # each element type uses 13 IDs from 1-13 (or bits for the features) and we want to test all the combinations
        num_elements = 13
        ids = range(1, num_elements + 1)
        self.all_id_combos = []
        combos = []
        for r in range(1, num_elements + 1):
            combos.extend(list(itertools.combinations(ids, r)))
        for combo in combos:
            # The first three IDs are all O.a, so we need exactly one for the conformance to be valid
            expected_failures = set()
            if len(set([1, 2, 3]) & set(combo)) != 1:
                expected_failures.add('a')
            if len(set([4, 5, 6]) & set(combo)) < 1:
                expected_failures.add('b')
            # For these, we are checking that choice conformance checkers
            # - Correctly report errors and correct cases when the gating feature is ON
            # - Do not report any errors when the gating features is off.
            # Errors where we incorrectly set disallowed features based on the gating feature are checked
            # elsewhere in the cert test in a comprehensive way. We just want to ensure that we are not
            # incorrectly reporting choice conformance error as well
            if 13 in combo and ((len(set([7, 8, 9]) & set(combo)) != 1)):
                expected_failures.add('c')
            if 13 in combo and (len(set([10, 11, 12]) & set(combo)) < 1):
                expected_failures.add('d')

            self.all_id_combos.append((combo, expected_failures))

    def _evaluate_problems(self, problems, expected_failures=list[str]):
        if len(expected_failures) != len(problems):
            print(problems)
        asserts.assert_equal(len(expected_failures), len(problems), 'Unexpected number of choice conformance problems')
        actual_failures = set([p.choice.marker for p in problems])
        asserts.assert_equal(actual_failures, expected_failures, "Mismatch between failures")

    def test_features(self):
        def make_feature_map(combo: tuple[int]) -> int:
            feature_map = 0
            for bit in combo:
                feature_map += pow(2, bit)
            return feature_map

        for combo, expected_failures in self.all_id_combos:
            problems = evaluate_feature_choice_conformance(0, 1, self.clusters, make_feature_map(combo), [], [])
            self._evaluate_problems(problems, expected_failures)

    def test_attributes(self):
        for combo, expected_failures in self.all_id_combos:
            problems = evaluate_attribute_choice_conformance(0, 1, self.clusters, 0, list(combo), [])
            self._evaluate_problems(problems, expected_failures)

    def test_commands(self):
        for combo, expected_failures in self.all_id_combos:
            problems = evaluate_command_choice_conformance(0, 1, self.clusters, 0, [], list(combo))
            self._evaluate_problems(problems, expected_failures)


if __name__ == "__main__":
    default_matter_test_main()
