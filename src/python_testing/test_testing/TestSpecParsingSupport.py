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
from collections import Counter
from copy import deepcopy
from unittest.mock import patch

import jinja2
from mobly import asserts, signals

import matter.clusters as Clusters
from matter.testing.conformance import TOP_LEVEL_CONFORMANCE_TAGS
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.matter_testing import CertificationUnitTestNoDevice
from matter.testing.problem_notices import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, EventPathLocation,
                                            FeaturePathLocation, ProblemNotice, ProblemSeverity)
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import (ClusterParser, DataModelLevel, PrebuiltDataModelDirectory, SpecParsingException,
                                         XmlCluster, add_cluster_data_from_xml, build_xml_clusters,
                                         check_clusters_for_unknown_commands, combine_derived_clusters_with_base,
                                         get_data_model_directory)

# TODO: improve the test coverage here
# https://github.com/project-chip/connectedhomeip/issues/30958

CLUSTER_ID = 0x0BEE
CLUSTER_NAME = "TestCluster"
ATTRIBUTE_NAME = "TestAttribute"
ATTRIBUTE_ID = 0x0000
COMMAND_ID = 0x0F


def single_attribute_cluster_xml(read_access: str, write_access: str, write_supported: str, invoke_access: str):
    xml_cluster = f'<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="{CLUSTER_ID}" name="{CLUSTER_NAME}" revision="3">'
    revision_table = ('<revisionHistory>'
                      '<revision revision="1" summary="Initial Release"/>'
                      '<revision revision="2" summary="Some other revision"/>'
                      '<revision revision="3" summary="another revision"/>'
                      '</revisionHistory>')
    id_table = ('<clusterIds>'
                f'<clusterId id="{CLUSTER_ID}" name="{CLUSTER_NAME}"/>'
                '</clusterIds>')
    classification = '<classification hierarchy="base" role="utility" picsCode="TEST" scope="Node"/>'
    read_access_str = f'read="true" readPrivilege="{read_access}"' if read_access is not None else ""
    write_access_str = f'write="{write_supported}" writePrivilege="{write_access}"' if write_access is not None else ""
    attribute = ('<attributes>'
                 f'<attribute id="{ATTRIBUTE_ID}" name="{ATTRIBUTE_NAME}" type="uint16" default="MS">'
                 f'<access {read_access_str} {write_access_str}/>'
                 '<quality changeOmitted="false" nullable="false" scene="false" persistence="fixed" reportable="false"/>'
                 '<mandatoryConform/>'
                 '</attribute>'
                 '</attributes>')
    invoke_access_str = f'invokePrivilege="{invoke_access}"' if invoke_access is not None else ""
    command = ('<commands>'
               f'<command id="{COMMAND_ID}" name="Cmd" direction="commandToServer" response="Y">'
               f'<access {invoke_access_str}/>'
               '<mandatoryConform/>'
               '</command>'
               '</commands>')

    return (f'{xml_cluster}'
            f'{revision_table}'
            f'{id_table}'
            f'{classification}'
            f'{attribute}'
            f'{command}'
            '</cluster>')


def parse_cluster(xml: str) -> XmlCluster:
    cluster = ElementTree.fromstring(xml)
    parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME)
    return parser.create_cluster()


def get_access_enum_from_string(access_str: str) -> Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum:
    if access_str == 'view':
        return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView
    if access_str == 'operate':
        return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate
    if access_str == 'manage':
        return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage
    if access_str == 'admin':
        return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister
    if access_str is None:
        return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
    asserts.fail("Unknown access string")
    return None


BASE_CLUSTER_XML_STR = (
    '<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="" name="Test Base" revision="1">'
    '  <revisionHistory>'
    '    <revision revision="1" summary="Initial version"/>'
    '  </revisionHistory>'
    '  <clusterIds>'
    '    <clusterId name="Test Base"/>'
    '  </clusterIds>'
    '  <classification hierarchy="base" role="application" picsCode="BASE" scope="Endpoint"/>'
    '  <features>'
    '    <feature bit="0" code="DEPONOFF" name="OnOff" summary="Dependency with the OnOff cluster">'
    '      <optionalConform/>'
    '    </feature>'
    '  </features>'
    '  <attributes>'
    '    <attribute id="0x0000" name="SupportedModes" type="list" default="MS">'
    '      <entry type="ModeOptionStruct"/>'
    '      <access read="true" readPrivilege="view"/>'
    '      <quality changeOmitted="false" nullable="false" scene="false" persistence="fixed" reportable="false"/>'
    '      <mandatoryConform/>'
    '      <constraint type="countBetween" from="2" to="255"/>'
    '    </attribute>'
    '    <attribute id="0x0001" name="CurrentMode" type="uint8" default="MS">'
    '      <access read="true" readPrivilege="view"/>'
    '      <quality changeOmitted="false" nullable="false" scene="true" persistence="nonVolatile" reportable="false"/>'
    '      <mandatoryConform/>'
    '      <constraint type="desc"/>'
    '   </attribute>'
    '    <attribute id="0x0002" name="StartUpMode" type="uint8" default="MS">'
    '      <access read="true" write="true" readPrivilege="view" writePrivilege="operate"/>'
    '      <quality changeOmitted="false" nullable="true" scene="false" persistence="nonVolatile" reportable="false"/>'
    '      <optionalConform/>'
    '      <constraint type="desc"/>'
    '    </attribute>'
    '    <attribute id="0x0003" name="OnMode" type="uint8" default="null">'
    '      <access read="true" write="true" readPrivilege="view" writePrivilege="operate"/>'
    '      <quality changeOmitted="false" nullable="true" scene="false" persistence="nonVolatile" reportable="false"/>'
    '      <mandatoryConform>'
    '        <feature name="DEPONOFF"/>'
    '      </mandatoryConform>'
    '      <constraint type="desc"/>'
    '    </attribute>'
    '  </attributes>'
    '  <commands>'
    '    <command id="0x00" name="ChangeToMode" response="ChangeToModeResponse" direction="commandToServer">'
    '      <access invokePrivilege="operate"/>'
    '      <mandatoryConform/>'
    '      <field id="0" name="NewMode" type="uint8">'
    '        <mandatoryConform/>'
    '        <constraint type="desc"/>'
    '      </field>'
    '    </command>'
    '    <command id="0x01" name="ChangeToModeResponse" direction="responseFromServer">'
    '      <access invokePrivilege="operate"/>'
    '      <mandatoryConform/>'
    '      <field id="0" name="Status" type="enum8">'
    '        <enum>'
    '          <item from="0x00" to="0x3F" name="CommonCodes" summary="Common standard values defined in the generic Mode Base cluster specification.">'
    '            <mandatoryConform/>'
    '          </item>'
    '        </enum>'
    '        <mandatoryConform/>'
    '        <constraint type="desc"/>'
    '      </field>'
    '      <field id="1" name="StatusText" type="string">'
    '        <constraint type="maxLength" value="64"/>'
    '      </field>'
    '    </command>'
    '  </commands>'
    '</cluster>')

DERIVED_CLUSTER_XML_STR = (
    '<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="0xFFFF" name="Test Derived" revision="1">'
    '  <revisionHistory>'
    '    <revision revision="1" summary="Initial Release"/>'
    '  </revisionHistory>'
    '  <clusterIds>'
    '    <clusterId id="0xFFFF" name="Test Derived"/>'
    '  </clusterIds>'
    '  <classification hierarchy="derived" baseCluster="Test Base" role="application" picsCode="MWOM" scope="Endpoint"/>'
    '  <attributes>'
    '    <attribute id="0x0000" name="SupportedModes">'
    '      <mandatoryConform/>'
    '    </attribute>'
    '    <attribute id="0x0002" name="StartUpMode">'
    '      <disallowConform/>'
    '    </attribute>'
    '    <attribute id="0x0003" name="OnMode">'
    '      <disallowConform/>'
    '    </attribute>'
    '  </attributes>'
    '  <commands>'
    '    <command id="0x00" name="ChangeToMode">'
    '      <access invokePrivilege="operate"/>'
    '      <disallowConform/>'
    '    </command>'
    '    <command id="0x01" name="ChangeToModeResponse">'
    '      <access invokePrivilege="operate"/>'
    '      <disallowConform/>'
    '    </command>'
    '  </commands>'
    '</cluster>'
)

CLUSTER_WITH_UNKNOWN_COMMAND = (
    '<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="0xFFFE" name="Test Unknown Command" revision="1">'
    '  <revisionHistory>'
    '    <revision revision="1" summary="Initial version"/>'
    '  </revisionHistory>'
    '  <clusterIds>'
    '    <clusterId id="0xFFFE" name="Test Unknown Command"/>'
    '  </clusterIds>'
    '  <classification hierarchy="base" role="application" picsCode="BASE" scope="Endpoint"/>'
    '  <commands>'
    '    <command id="0x00" name="ChangeToMode" direction="commandToClient">'
    '      <access invokePrivilege="operate"/>'
    '      <mandatoryConform/>'
    '    </command>'
    '  </commands>'
    '</cluster>'
)

ALIASED_CLUSTERS = (
    '<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="" name="Test Aliases" revision="1">'
    '  <revisionHistory>'
    '    <revision revision="1" summary="Initial version"/>'
    '  </revisionHistory>'
    '  <clusterIds>'
    '    <clusterId id="0xFFFE" name="Test Alias1"/>'
    '    <clusterId id="0xFFFD" name="Test Alias2">'
    '      <provisionalConform/>'
    '    </clusterId>'
    '  </clusterIds>'
    '  <classification hierarchy="base" role="application" picsCode="BASE" scope="Endpoint"/>'
    '  <commands>'
    '    <command id="0x00" name="ChangeToMode" direction="commandToServer">'
    '      <access invokePrivilege="operate"/>'
    '      <mandatoryConform/>'
    '    </command>'
    '  </commands>'
    '</cluster>'
)

PROVISIONAL_CLUSTER_TEMPLATE = """
<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="{{ id }}" name="Test Provisional" revision="1">
  <revisionHistory>
    <revision revision="1" summary="Initial revision"/>
  </revisionHistory>
  <clusterIds>
    <clusterId id="{{ id }}" name="Test Provisional">
    {% if provisional %}
      <provisionalConform/>
    {% endif %}
    </clusterId>
  </clusterIds>
  <classification hierarchy="base" role="utility" picsCode="PROVISIONAL" scope="Node"/>
  <commands>
    <command id="0x00" name="My command" direction="commandToServer">
      <access invokePrivilege="operate"/>
      <mandatoryConform/>
    </command>
  </commands>
</cluster>
"""


class TestSpecParsingSupport(CertificationUnitTestNoDevice):

    def setup_class(self):
        super().setup_class()
        # Latest fully certified build
        self.spec_xml_clusters, self.spec_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        self.all_spec_clusters = {(_id, c.name, c.pics) for _id, c in self.spec_xml_clusters.items()}

    @staticmethod
    def assert_historical_parser_diagnostics(version: PrebuiltDataModelDirectory,
                                             problems: list[ProblemNotice]) -> None:
        """Check visible parser limits against the packaged XML, not just a notice total.

        Field/value predicates are not evaluated, multi-bit bitmap members are omitted,
        and derived attributes without conformance still fall back to optional rather
        than inheriting. Older XML also lacks some conformance and command-field IDs.
        These remain returned warnings, not evidence that those constructs are supported.
        Any other diagnostic, severity change, or source mismatch must fail this check.
        """
        expected = {
            '1.2': (5, 0, 5, 0, 0),
            '1.3': (11, 6, 5, 0, 0),
            '1.4': (12, 15, 5, 7, 1),
            '1.4.1': (12, 15, 5, 7, 1),
            '1.4.2': (29, 14, 5, 0, 0),
            '1.5': (50, 14, 5, 0, 0),
            '1.5.1': (55, 14, 3, 0, 0),
            '1.6': (55, 14, 3, 0, 0),
            '1.6.1': (55, 14, 3, 0, 0),
            '1.7': (67, 18, 5, 0, 0),
        }
        categories = ('unsupported predicate', 'derived attribute fallback', 'bitmap range',
                      'missing conformance', 'malformed command field')
        sources = {}
        for path in get_data_model_directory(version).iterdir():
            if not path.name.endswith('.xml'):
                continue
            with path.open('r') as source:
                root = ElementTree.parse(source).getroot()
            for cluster in root.iter('cluster'):
                ids = {int(element.get('id'), 0) for element in cluster.findall('./clusterIds/clusterId')
                       if element.get('id')}
                if cluster.get('id'):
                    ids.add(int(cluster.get('id'), 0))
                for cluster_id in ids or {0}:
                    sources.setdefault(cluster_id, []).append(cluster)

        counts = Counter()
        for problem in problems:
            asserts.assert_is_instance(problem, ProblemNotice)
            asserts.assert_equal(problem.severity, ProblemSeverity.WARNING, str(problem))
            asserts.assert_is_instance(problem.location, ClusterPathLocation)
            asserts.assert_equal(problem.location.endpoint_id, 0, str(problem))
            asserts.assert_in(problem.test_name, ('Spec XML parsing', 'Spec XML Parsing'))
            asserts.assert_equal(problem.spec_location, '', str(problem))
            asserts.assert_in(problem.location.cluster_id, sources, str(problem))
            matching = set()
            for cluster in sources[problem.location.cluster_id]:
                classification = cluster.find('classification')
                derived = classification is not None and classification.get('hierarchy') == 'derived'
                for element in cluster.iter():
                    if element.tag in TOP_LEVEL_CONFORMANCE_TAGS:
                        for leaf in element.iter():
                            if leaf.tag in ('field', 'value') and not len(leaf):
                                message = ('ConformanceException(Unexpected xml conformance element with no children '
                                           f'{leaf.tag} {leaf.attrib})')
                                if type(problem.location) is ClusterPathLocation and problem.problem == message:
                                    matching.add('unsupported predicate')
                    if element.tag == 'attribute' and derived and element.get('id') is not None:
                        message = ('Derived attribute omits conformance; parser uses optional conformance '
                                   'instead of inheriting from base')
                        if (type(problem.location) is AttributePathLocation
                                and problem.location.attribute_id == int(element.get('id'), 0)
                                and not any(child.tag in TOP_LEVEL_CONFORMANCE_TAGS for child in element)
                                and problem.problem == message):
                            matching.add('derived attribute fallback')
                    if element.tag == 'bitmap':
                        for bitfield in element.findall('bitfield'):
                            if (bitfield.get('name') and bitfield.get('bit') is None
                                    and bitfield.get('from') is not None and bitfield.get('to') is not None
                                    and int(bitfield.get('from'), 0) < int(bitfield.get('to'), 0)):
                                message = (f"Unsupported bitmap bit range {bitfield.get('from')}..{bitfield.get('to')} "
                                           f"for {bitfield.get('name')} in {element.get('name')}; field omitted")
                                if type(problem.location) is ClusterPathLocation and problem.problem == message:
                                    matching.add('bitmap range')
                    locations = {
                        'attribute': (AttributePathLocation, 'attribute_id', 'id'),
                        'command': (CommandPathLocation, 'command_id', 'id'),
                        'event': (EventPathLocation, 'event_id', 'id'),
                        'feature': (FeaturePathLocation, 'feature_code', 'code'),
                    }
                    if (element.tag in locations and problem.problem == 'Unable to find conformance element'
                            and not (derived and element.tag in ('attribute', 'command'))
                            and not any(child.tag in TOP_LEVEL_CONFORMANCE_TAGS for child in element)):
                        location_type, location_key, source_key = locations[element.tag]
                        source_id = element.get(source_key)
                        if source_id is not None and type(problem.location) is location_type:
                            source_id = source_id if source_key == 'code' else int(source_id, 0)
                            if getattr(problem.location, location_key) == source_id:
                                matching.add('missing conformance')
                    if element.tag == 'command':
                        message = f"Field in command {element.get('name', '?')} with no id or name"
                        if (type(problem.location) is ClusterPathLocation and problem.problem == message
                                and any('id' not in child.attrib or 'name' not in child.attrib
                                        for child in element.findall('field'))):
                            matching.add('malformed command field')
            asserts.assert_equal(len(matching), 1, f'Unclassified parser diagnostic: {problem}')
            counts.update(matching)
        expected_counts = dict(zip(categories, expected[version.dirname]))
        asserts.assert_equal(counts, Counter(expected_counts), f'{version.dirname}: diagnostic categories changed')
        asserts.assert_equal(len(problems), sum(expected_counts.values()), f'{version.dirname}: diagnostic total changed')

    def test_historical_diagnostics_reject_unknown_notice(self):
        version = PrebuiltDataModelDirectory.k1_4
        self.assert_historical_parser_diagnostics(version, self.spec_problems)
        unexpected = ProblemNotice('Spec XML parsing', ClusterPathLocation(0, Clusters.AccessControl.id),
                                   ProblemSeverity.WARNING, 'Unknown access type invalid')
        with asserts.assert_raises_regex(signals.TestFailure, 'Unclassified parser diagnostic'):
            self.assert_historical_parser_diagnostics(version, [*self.spec_problems, unexpected])
        unexpected.severity = ProblemSeverity.ERROR
        with asserts.assert_raises(signals.TestFailure):
            self.assert_historical_parser_diagnostics(version, [*self.spec_problems, unexpected])

    def test_build_xml_actions_workaround_retains_access_warning(self):
        action_id = Clusters.Actions.id
        command_id = Clusters.Actions.Commands.InstantAction.command_id
        location = CommandPathLocation(endpoint_id=0, cluster_id=action_id, command_id=command_id)
        missing_conformance = 'Unable to find conformance element'
        invalid_access = 'Unknown access type invalid'
        original_parse = ElementTree.parse
        injected = []

        def parse_with_invalid_action(source):
            tree = original_parse(source)
            for cluster in tree.getroot().iter('cluster'):
                if not any(int(element.get('id'), 0) == action_id
                           for element in cluster.findall('./clusterIds/clusterId') if element.get('id')):
                    continue
                command = next(element for element in cluster.findall('./commands/command')
                               if int(element.get('id'), 0) == command_id)
                access = command.find('access')
                asserts.assert_is_not_none(access)
                access.set('invokePrivilege', 'invalid')
                for child in list(command):
                    if child.tag in TOP_LEVEL_CONFORMANCE_TAGS:
                        command.remove(child)
                parser = ClusterParser(cluster, action_id, 'Actions')
                parser.create_cluster()
                injected.extend(problem.problem for problem in parser.get_problems() if problem.location == location)
            return tree

        with patch('matter.testing.spec_parsing.ElementTree.parse', side_effect=parse_with_invalid_action):
            clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)

        asserts.assert_count_equal(injected, [missing_conformance, invalid_access])
        retained = [problem for problem in problems if problem.location == location]
        asserts.assert_equal([problem.problem for problem in retained], [invalid_access])
        asserts.assert_equal(retained[0].severity, ProblemSeverity.WARNING)
        asserts.assert_equal(clusters[action_id].accepted_commands[command_id].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue)
        asserts.assert_equal(str(clusters[action_id].accepted_commands[command_id].conformance), 'O')

    def test_build_xml_override(self):
        one_two_clusters, one_two_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_2)
        one_three_clusters, one_three_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_3)
        one_four_clusters, one_four_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        one_four_one_clusters, one_four_one_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        one_four_two_xml_clusters, one_four_two_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_2)
        one_five_xml_clusters, one_five_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_5)
        one_five_one_xml_clusters, one_five_one_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_5_1)
        one_six_xml_clusters, one_six_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_6)
        one_six_one_xml_clusters, one_six_one_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_6_1)
        one_seven_xml_clusters, one_seven_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)

        for version, problems in (
                (PrebuiltDataModelDirectory.k1_2, one_two_problems),
                (PrebuiltDataModelDirectory.k1_3, one_three_problems),
                (PrebuiltDataModelDirectory.k1_4, one_four_problems),
                (PrebuiltDataModelDirectory.k1_4_1, one_four_one_problems),
                (PrebuiltDataModelDirectory.k1_4_2, one_four_two_problems),
                (PrebuiltDataModelDirectory.k1_5, one_five_problems),
                (PrebuiltDataModelDirectory.k1_5_1, one_five_one_problems),
                (PrebuiltDataModelDirectory.k1_6, one_six_problems),
                (PrebuiltDataModelDirectory.k1_6_1, one_six_one_problems),
                (PrebuiltDataModelDirectory.k1_7, one_seven_problems)):
            self.assert_historical_parser_diagnostics(version, problems)

        asserts.assert_greater(len(set(one_four_two_xml_clusters.keys()) - set(one_two_clusters.keys())),
                               0, "1.2.2 dir does not contain any clusters not in 1.3")
        asserts.assert_greater(len(set(one_four_two_xml_clusters.keys()) - set(one_three_clusters.keys())),
                               0, "1.4.2 dir does not contain any clusters not in 1.3")
        asserts.assert_equal(len(set(one_four_two_xml_clusters.keys()) - set(one_four_clusters.keys())),
                             0, "1.4.2 contains clusters not in 1.4")
        asserts.assert_greater(len(set(one_four_clusters.keys()) - set(one_three_clusters.keys())),
                               0, "1.4 dir does not contain any clusters not in 1.3")
        asserts.assert_equal(len(one_four_clusters.keys()), len(one_four_one_clusters.keys()),
                             "1.4 and 1.4.1 do not contain the same clusters")
        asserts.assert_greater(len(set(one_five_xml_clusters.keys()) - set(one_four_two_xml_clusters.keys())),
                               0, "1.5 dir does not contain any clusters not in 1.4.2")
        asserts.assert_greater_equal(len(set(one_five_one_xml_clusters.keys()) - set(one_five_xml_clusters.keys())),
                                     0, "1.5.1 has fewer clusters than 1.5")
        asserts.assert_greater_equal(len(set(one_six_xml_clusters.keys()) - set(one_five_one_xml_clusters.keys())),
                                     0, "1.6 has fewer clusters than 1.5.1")
        asserts.assert_greater_equal(len(set(one_six_one_xml_clusters.keys()) - set(one_six_xml_clusters.keys())),
                                     0, "1.6.1 has fewer clusters than 1.6")
        asserts.assert_greater_equal(len(set(one_seven_xml_clusters.keys()) - set(one_six_one_xml_clusters.keys())),
                                     0, "1.7 has fewer clusters than 1.6.1")

        # The following clusters were removed in 1.3: Scenes, Leaf Wetness Measurement, Soil Moisture Measurement
        one_two_removed = {0x0005, 0x0407, 0x0408}
        asserts.assert_equal(set(one_two_clusters.keys()) - set(one_three_clusters.keys()),
                             one_two_removed, "There are some 1.3 clusters that are unexpectedly not included in the 1.4 spec")
        # only the pulse width modulation cluster was removed post 1.3
        one_four_removed = {Clusters.PulseWidthModulation.id}
        asserts.assert_equal(set(one_three_clusters.keys()) - set(one_four_clusters.keys()),
                             one_four_removed, "There are some 1.3 clusters that are unexpectedly not included in the 1.4 spec")
        # Ballast and all the proxy clusters are being removed in 1.4.2
        one_four_two_removed = {Clusters.BallastConfiguration.id, Clusters.ProxyConfiguration.id,
                                Clusters.ProxyDiscovery.id, Clusters.ProxyValid.id}
        asserts.assert_equal(set(one_four_clusters.keys())-set(one_four_two_xml_clusters.keys()),
                             one_four_two_removed, "There are some 1.4 clusters that are unexpectedly not included in the 1.4.2 spec")
        asserts.assert_equal(set(one_three_clusters.keys())-set(one_four_two_xml_clusters.keys()),
                             one_four_removed.union(one_four_two_removed), "There are some 1.3 clusters that are unexpectedly not included in the 1.4.2 spec")
        asserts.assert_equal(set(one_four_two_xml_clusters.keys())-set(one_five_xml_clusters.keys()), set(),
                             "There are some 1.4.2 cluster that are unexpectedly not included in the 1.5 spec")

        str_path = get_data_model_directory(PrebuiltDataModelDirectory.k1_4, DataModelLevel.kCluster)
        string_override_check, problems = build_xml_clusters(str_path)

        asserts.assert_count_equal(string_override_check.keys(), self.spec_xml_clusters.keys(), "Mismatched cluster generation")

    def test_spec_parsing_access(self):
        strs = [None, 'view', 'operate', 'manage', 'admin']
        for read in strs:
            for write in strs:
                xml = single_attribute_cluster_xml(read, write, "true", None)
                xml_cluster = parse_cluster(xml)
                asserts.assert_is_not_none(xml_cluster.attributes, "No attributes found in cluster")
                asserts.assert_is_not_none(xml_cluster.attribute_map, "No attribute map found in cluster")
                asserts.assert_equal(len(xml_cluster.attributes), len(GlobalAttributeIds) + 1, "Unexpected number of attributes")
                asserts.assert_true(ATTRIBUTE_ID in xml_cluster.attributes,
                                    "Did not find test attribute in XmlCluster.attributes")
                asserts.assert_equal(xml_cluster.attributes[ATTRIBUTE_ID].read_access,
                                     get_access_enum_from_string(read), "Unexpected read access")
                asserts.assert_equal(xml_cluster.attributes[ATTRIBUTE_ID].write_access,
                                     get_access_enum_from_string(write), "Unexpected write access")
        for invoke in strs:
            xml = single_attribute_cluster_xml(None, None, "true", invoke)
            xml_cluster = parse_cluster(xml)
            asserts.assert_is_not_none(xml_cluster.accepted_commands, "No commands found in cluster")
            asserts.assert_is_not_none(xml_cluster.command_map, "No command map found in cluster")
            asserts.assert_true(COMMAND_ID in xml_cluster.accepted_commands,
                                "Did not find test command in XmlCluster.accepted_commands")
            asserts.assert_equal(xml_cluster.accepted_commands[COMMAND_ID].privilege,
                                 get_access_enum_from_string(invoke), "Unexpected invoke privilege")

    def test_write_optional(self):
        for write_support in ['true', 'optional']:
            xml = single_attribute_cluster_xml('view', 'view', write_support, None)
            xml_cluster = parse_cluster(xml)
            asserts.assert_is_not_none(xml_cluster.attributes, "No attributes found in cluster")
            asserts.assert_is_not_none(xml_cluster.attribute_map, "No attribute map found in cluster")
            asserts.assert_equal(len(xml_cluster.attributes), len(GlobalAttributeIds) + 1, "Unexpected number of attributes")
            asserts.assert_true(ATTRIBUTE_ID in xml_cluster.attributes,
                                "Did not find test attribute in XmlCluster.attributes")
            asserts.assert_equal(xml_cluster.attributes[ATTRIBUTE_ID].write_optional,
                                 write_support == 'optional', "Unexpected write_optional value")

    def test_derived_clusters(self):
        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        base_cluster_xml = ElementTree.fromstring(BASE_CLUSTER_XML_STR)
        derived_cluster_xml = ElementTree.fromstring(DERIVED_CLUSTER_XML_STR)
        expected_global_attrs = [GlobalAttributeIds.FEATURE_MAP_ID, GlobalAttributeIds.ATTRIBUTE_LIST_ID,
                                 GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID, GlobalAttributeIds.GENERATED_COMMAND_LIST_ID, GlobalAttributeIds.CLUSTER_REVISION_ID]

        add_cluster_data_from_xml(base_cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)
        add_cluster_data_from_xml(derived_cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)

        asserts.assert_equal(len(clusters), 1, "Unexpected number of clusters")
        asserts.assert_equal(len(pure_base_clusters), 1, "Unexpected number of pure base clusters")
        asserts.assert_equal(len(ids_by_name), 1, "Unexpected number of IDs per name")
        asserts.assert_equal(len(problems), 0, "Unexpected number of problems")
        asserts.assert_equal(ids_by_name["Test Derived"], 0xFFFF, "Test derived name not added to IDs")

        asserts.assert_true(0xFFFF in clusters, "Derived ID not found in clusters")
        asserts.assert_equal(set(clusters[0xFFFF].attributes.keys()), set(
            [0, 2, 3] + expected_global_attrs), "Unexpected attribute list")
        asserts.assert_equal(set(clusters[0xFFFF].accepted_commands.keys()), set(), "Unexpected accepted commands")
        asserts.assert_equal(set(clusters[0xFFFF].generated_commands.keys()), set(), "Unexpected generated commands")

        asserts.assert_true("Test Base" in pure_base_clusters, "Base ID not found in derived clusters")
        asserts.assert_equal(set(pure_base_clusters["Test Base"].attributes.keys()), set(
            [0, 1, 2, 3] + expected_global_attrs), "Unexpected attribute list")
        asserts.assert_equal(set(pure_base_clusters["Test Base"].accepted_commands.keys()),
                             {0}, "Unexpected accepted commands")
        asserts.assert_equal(set(pure_base_clusters["Test Base"].generated_commands.keys()),
                             {1}, "Unexpected generated commands")
        asserts.assert_equal(str(pure_base_clusters["Test Base"].accepted_commands[0].conformance),
                             "M", "Unexpected conformance on base accepted command")
        asserts.assert_equal(str(pure_base_clusters["Test Base"].generated_commands[1].conformance),
                             "M", "Unexpected conformance on base generated command")

        asserts.assert_equal(len(pure_base_clusters["Test Base"].unknown_commands),
                             0, "Unexpected number of unknown commands in base")
        asserts.assert_equal(len(clusters[0xFFFF].unknown_commands), 2, "Unexpected number of unknown commands in derived cluster")

        combine_derived_clusters_with_base(clusters, pure_base_clusters, ids_by_name, problems)
        # Ensure the base-only attribute (1) was added to the derived cluster
        asserts.assert_equal(set(clusters[0xFFFF].attributes.keys()), set(
            [0, 1, 2, 3] + expected_global_attrs), "Unexpected attribute list")
        # Ensure the conformance overrides from the derived cluster are on the attributes
        asserts.assert_equal(str(clusters[0xFFFF].attributes[0].conformance), "M", "Unexpected conformance on attribute 0")
        asserts.assert_equal(str(clusters[0xFFFF].attributes[1].conformance), "M", "Unexpected conformance on attribute 1")
        asserts.assert_equal(str(clusters[0xFFFF].attributes[2].conformance), "X", "Unexpected conformance on attribute 2")
        asserts.assert_equal(str(clusters[0xFFFF].attributes[3].conformance), "X", "Unexpected conformance on attribute 3")

        # Ensure both the accepted and generated command overrides work
        asserts.assert_true(set(clusters[0xFFFF].accepted_commands.keys()),
                            {0}, "Unexpected accepted command list after merge")
        asserts.assert_true(set(clusters[0xFFFF].generated_commands.keys()), {1},
                            "Unexpected generated command list after merge")
        asserts.assert_equal(str(clusters[0xFFFF].accepted_commands[0].conformance),
                             "X", "Unexpected conformance on accepted commands")
        asserts.assert_equal(str(clusters[0xFFFF].generated_commands[1].conformance),
                             "X", "Unexpected conformance on generated commands")
        asserts.assert_equal(len(clusters[0xFFFF].unknown_commands), 0, "Unexpected number of unknown commands after merge")

    def test_derived_command_conformance_inheritance(self):
        cases = [('', 'M'), ('<optionalConform/>', 'O'), ('<disallowConform/>', 'X'),
                 ('<mandatoryConform><feature name="DEPONOFF"/></mandatoryConform>', 'DEPONOFF')]
        for explicit_direction in (False, True):
            for conformance_xml, expected_conformance in cases:
                for privilege in (None, 'manage'):
                    clusters: dict[int, XmlCluster] = {}
                    pure_base_clusters: dict[str, XmlCluster] = {}
                    ids_by_name: dict[str, int] = {}
                    problems: list[ProblemNotice] = []
                    base = ElementTree.fromstring(BASE_CLUSTER_XML_STR)
                    derived = ElementTree.fromstring(DERIVED_CLUSTER_XML_STR)
                    derived.append(deepcopy(base.find('features')))
                    for command in derived.findall('./commands/command'):
                        for child in list(command):
                            command.remove(child)
                        if conformance_xml:
                            command.append(ElementTree.fromstring(conformance_xml))
                        if privilege is not None:
                            ElementTree.SubElement(command, 'access', invokePrivilege=privilege)
                        if explicit_direction:
                            command.set('direction', 'commandToServer' if command.get('id') == '0x00'
                                        else 'responseFromServer')
                    add_cluster_data_from_xml(base, clusters, pure_base_clusters, ids_by_name, problems)
                    add_cluster_data_from_xml(derived, clusters, pure_base_clusters, ids_by_name, problems)
                    combine_derived_clusters_with_base(clusters, pure_base_clusters, ids_by_name, problems)
                    context = f'direction={explicit_direction}, conformance={conformance_xml}, privilege={privilege}'
                    for command in (clusters[0xFFFF].accepted_commands[0], clusters[0xFFFF].generated_commands[1]):
                        asserts.assert_equal(str(command.conformance), expected_conformance, context)
                        asserts.assert_equal(command.privilege, get_access_enum_from_string(privilege or 'operate'), context)

    def test_derived_command_inherits_feature_conformance(self):
        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        base = ElementTree.fromstring(BASE_CLUSTER_XML_STR)
        derived = ElementTree.fromstring(DERIVED_CLUSTER_XML_STR)
        for command in base.findall('./commands/command'):
            command.remove(command.find('mandatoryConform'))
            otherwise = ElementTree.SubElement(command, 'otherwiseConform')
            ElementTree.SubElement(otherwise, 'provisionalConform')
            optional = ElementTree.SubElement(otherwise, 'optionalConform')
            ElementTree.SubElement(optional, 'feature', name='DEPONOFF')
        for command in derived.findall('./commands/command'):
            command.remove(command.find('disallowConform'))
        add_cluster_data_from_xml(base, clusters, pure_base_clusters, ids_by_name, problems)
        add_cluster_data_from_xml(derived, clusters, pure_base_clusters, ids_by_name, problems)
        combine_derived_clusters_with_base(clusters, pure_base_clusters, ids_by_name, problems)
        for command in (clusters[0xFFFF].accepted_commands[0], clusters[0xFFFF].generated_commands[1]):
            asserts.assert_equal(str(command.conformance), 'P, [DEPONOFF]', 'Missing overlay must inherit feature condition')

    def test_loader_preserves_parser_problems(self):
        cluster_xml = ElementTree.fromstring(ALIASED_CLUSTERS)
        cluster_xml.find('./commands/command/access').set('invokePrivilege', 'invalid')
        existing = ProblemNotice('Caller', ClusterPathLocation(0, CLUSTER_ID), ProblemSeverity.WARNING, 'Existing notice')
        problems = [existing]
        clusters: dict[int, XmlCluster] = {}
        add_cluster_data_from_xml(cluster_xml, clusters, {}, {}, problems)

        asserts.assert_is(problems[0], existing, 'The caller notice must remain in place')
        asserts.assert_equal(len(problems), 3, 'Each alias must append its parser notice to the caller list')
        asserts.assert_equal([problem.location.cluster_id for problem in problems[1:]],
                             [0xFFFE, 0xFFFD], 'Each notice must identify its alias')
        for problem in problems[1:]:
            asserts.assert_equal(problem.severity, ProblemSeverity.WARNING)
            asserts.assert_equal(problem.problem, 'Unknown access type invalid')

    def test_data_type_reference_does_not_replace_definition(self):
        cluster = ElementTree.fromstring(BASE_CLUSTER_XML_STR)
        data_types = ElementTree.SubElement(cluster, 'dataTypes')
        definition = ElementTree.SubElement(data_types, 'enum', name='StateEnum')
        item = ElementTree.SubElement(definition, 'item', name='Ready', value='1')
        ElementTree.SubElement(item, 'mandatoryConform')
        struct = ElementTree.SubElement(data_types, 'struct', name='Record')
        field = ElementTree.SubElement(struct, 'field', id='0', name='State', type='StateEnum')
        conformance = ElementTree.SubElement(field, 'mandatoryConform')
        condition = ElementTree.SubElement(conformance, 'equal')
        ElementTree.SubElement(condition, 'field', name='Other')
        ElementTree.SubElement(condition, 'enum', name='StateEnum', value='Ready')
        parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME)
        parsed = parser.create_cluster()
        asserts.assert_equal(set(parsed.enums['StateEnum'].components), {1}, 'A reference must not overwrite its definition')
        asserts.assert_equal(len(parser.get_problems()), 1, 'Unsupported field conformance must remain visible')

        data_types.append(deepcopy(definition))
        parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME)
        parser.create_cluster()
        asserts.assert_in("Duplicate enum name 'StateEnum' - overwriting previous entry",
                          [problem.problem for problem in parser.get_problems()], 'Real duplicate definitions must still warn')

        bitmap = ElementTree.SubElement(data_types, 'bitmap', name='StatusBitmap')
        ElementTree.SubElement(bitmap, 'bitfield', name='State', **{'from': '0', 'to': '1'})
        parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME)
        parsed = parser.create_cluster()
        asserts.assert_equal(parsed.bitmaps['StatusBitmap'].components, {}, 'Do not approximate a multi-bit field as one bit')
        asserts.assert_in('Unsupported bitmap bit range 0..1 for State in StatusBitmap; field omitted',
                          [problem.problem for problem in parser.get_problems()], 'Describe the unsupported range accurately')

    def test_access_notice_origins(self):
        for direction, expected_count in (('responseFromServer', 0), ('commandToServer', 1)):
            cluster = ElementTree.fromstring(ALIASED_CLUSTERS)
            command = cluster.find('./commands/command')
            command.set('direction', direction)
            command.remove(command.find('access'))
            parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME)
            parser.create_cluster()
            asserts.assert_equal(len(parser.get_problems()), expected_count, direction)
            if expected_count:
                asserts.assert_equal(parser.get_problems()[0].problem, 'Unable to find access element')

        base = ElementTree.fromstring(BASE_CLUSTER_XML_STR)
        for attribute in base.findall('./attributes/attribute'):
            attribute.remove(attribute.find('access'))
        parser = ClusterParser(base, None, 'Test Base')
        pure_base_clusters = {'Test Base': parser.create_cluster()}
        asserts.assert_equal(parser.get_problems(), [], 'An abstract base can leave access to derived clusters')
        derived = ClusterParser(ElementTree.fromstring(DERIVED_CLUSTER_XML_STR), 0xFFFF, 'Test Derived').create_cluster()
        problems: list[ProblemNotice] = []
        combine_derived_clusters_with_base({0xFFFF: derived}, pure_base_clusters, {}, problems)
        asserts.assert_true(problems, 'Unresolved access must still be reported after inheritance')
        asserts.assert_true(all('unknown read and write access' in problem.problem for problem in problems),
                            'Only unresolved access notices are expected')

    def test_conformance_notice_origins(self):
        for derived in (False, True):
            cluster = ElementTree.fromstring(ALIASED_CLUSTERS)
            if derived:
                classification = cluster.find('classification')
                classification.set('hierarchy', 'derived')
                classification.set('baseCluster', 'Test Base')
            command = cluster.find('./commands/command')
            command.remove(command.find('mandatoryConform'))
            parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME)
            parser.create_cluster()
            asserts.assert_equal(len(parser.get_problems()), 0 if derived else 1)
            if not derived:
                asserts.assert_equal(parser.get_problems()[0].problem, 'Unable to find conformance element')

        cluster = ElementTree.fromstring(DERIVED_CLUSTER_XML_STR)
        attribute = cluster.find('./attributes/attribute')
        attribute.remove(attribute.find('mandatoryConform'))
        parser = ClusterParser(cluster, 0xFFFF, 'Test Derived')
        parsed = parser.create_cluster()
        asserts.assert_equal(str(parsed.attributes[0].conformance), 'O', 'Do not change the existing attribute fallback')
        asserts.assert_equal([problem.problem for problem in parser.get_problems()],
                             ['Derived attribute omits conformance; parser uses optional conformance instead of inheriting from base'])

        base = ClusterParser(ElementTree.fromstring(BASE_CLUSTER_XML_STR), None, 'Test Base').create_cluster()
        cluster = ElementTree.fromstring(DERIVED_CLUSTER_XML_STR)
        command = cluster.find('./commands/command')
        command.set('id', '0xFE')
        command.set('direction', 'commandToServer')
        command.remove(command.find('disallowConform'))
        parser = ClusterParser(cluster, 0xFFFF, 'Test Derived')
        parsed = parser.create_cluster()
        problems: list[ProblemNotice] = []
        combine_derived_clusters_with_base({0xFFFF: parsed}, {'Test Base': base}, {}, problems)
        asserts.assert_equal([problem.location.command_id for problem in problems], [0xFE],
                             'An unmatched derived command must not silently lose its missing-conformance notice')

    def test_missing_command_direction(self):
        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        cluster_xml = ElementTree.fromstring(CLUSTER_WITH_UNKNOWN_COMMAND)

        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)
        check_clusters_for_unknown_commands(clusters, problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems found")
        asserts.assert_equal(problems[0].location.cluster_id, 0xFFFE, "Unexpected problem location (cluster id)")
        asserts.assert_equal(problems[0].location.command_id, 0, "Unexpected problem location (command id)")

    def test_unsupported_constraint_expressions(self):
        expressions = [
            '<maxOf><field name="Lower"/><attribute name="Floor"/></maxOf>',
            '<minOf><attribute name="Limit"/><maxOf><attribute name="Floor"/><value value="1"/></maxOf></minOf>',
            '<attribute name="Floor"/><attribute name="Limit"/>',
            '<attribute name="Floor"><maxOf><value value="1"/></maxOf></attribute>',
        ]
        for expression in expressions:
            for bound in ('min', 'max', 'minCount', 'maxCount'):
                parser = ClusterParser(ElementTree.fromstring(BASE_CLUSTER_XML_STR), None, 'Test Base')
                element = ElementTree.fromstring(f'<field><constraint><{bound}>{expression}</{bound}></constraint></field>')
                constraints = parser.parse_attribute_constraints(element)
                asserts.assert_true(constraints.has_constraints(), 'Unsupported must not look unconstrained')
                asserts.assert_true(hasattr(constraints, 'unsupported_expressions'), 'Unsupported state must survive parsing')
                asserts.assert_true(constraints.unsupported_expressions, expression)
                with asserts.assert_raises(SpecParsingException):
                    constraints.require_supported()
        parser = ClusterParser(ElementTree.fromstring(BASE_CLUSTER_XML_STR), None, 'Test Base')
        element = ElementTree.fromstring(
            '<field><constraint><between><from>' + expressions[0] + '</from>'
            '<to><attribute name="Limit"/></to></between></constraint>'
            '<constraint><min value="0"/></constraint></field>')
        constraints = parser.parse_attribute_constraints(element)
        asserts.assert_equal(constraints.max_value_ref.attribute, 'Limit', 'Keep representable upper bound')
        asserts.assert_true(constraints.unsupported_expressions, 'Sibling constraints must retain unsupported state')
        asserts.assert_in('maxOf', str(constraints), 'String diagnostics must expose unsupported expression')

    def test_unsupported_text_constraints(self):
        for value in ('maxOf(Lower, Floor)', 'Floor - 1', 'Floor + 1', 'Floor.Field'):
            parser = ClusterParser(ElementTree.fromstring(BASE_CLUSTER_XML_STR), None, 'Test Base')
            element = ElementTree.fromstring(f'<field><constraint><min value="{value}"/></constraint></field>')
            constraints = parser.parse_attribute_constraints(element)
            asserts.assert_is_none(constraints.min_value_ref, 'Must not approximate an expression as one attribute')
            with asserts.assert_raises(SpecParsingException):
                constraints.require_supported()

    def test_supported_constraint_controls(self):
        parser = ClusterParser(ElementTree.fromstring(BASE_CLUSTER_XML_STR), None, 'Test Base')
        for value, expected in (('0', 0), ('-2', -2), ('1.5', 1.5), ('0x20', 32), ('1e2', 100)):
            element = ElementTree.fromstring(f'<field><constraint><min value="{value}"/></constraint></field>')
            constraints = parser.parse_attribute_constraints(element)
            asserts.assert_equal(constraints.min_value, expected, value)
        for value in ('Floor', ' Floor '):
            element = ElementTree.fromstring(f'<field><constraint><min value="{value}"/></constraint></field>')
            constraints = parser.parse_attribute_constraints(element)
            asserts.assert_equal(constraints.min_value_ref.attribute, 'Floor', value)
        element = ElementTree.fromstring(
            '<field><constraint><min><attribute name="Floor"><field name="Value"/></attribute></min></constraint></field>')
        constraints = parser.parse_attribute_constraints(element)
        asserts.assert_equal(constraints.min_value_ref.field, 'Value', 'Simple attribute-field references remain supported')

    def test_numeric_constraint_precision(self):
        parser = ClusterParser(ElementTree.fromstring(BASE_CLUSTER_XML_STR), None, 'Test Base')
        for value, expected in (
            ('9007199254740991', 2**53 - 1),
            ('9007199254740992', 2**53),
            ('9007199254740993', 2**53 + 1),
            ('-9007199254740993', -(2**53 + 1)),
            ('-9223372036854775808', -(2**63)),
            ('-9223372036854775807', -(2**63) + 1),
            ('9223372036854775806', 2**63 - 2),
            ('9223372036854775807', 2**63 - 1),
            ('+9007199254740993', 2**53 + 1),
            ('009007199254740993', 2**53 + 1),
            ('0x20000000000001', 2**53 + 1),
            ('0X7FFFFFFFFFFFFFFF', 2**63 - 1),
            ('1.5', 1.5),
            ('-2.5', -2.5),
            ('2.0', 2),
            ('1e2', 100),
        ):
            for bounds in (
                f'<min value="{value}"/><max value="{value}"/>',
                f'<between><from value="{value}"/><to value="{value}"/></between>',
            ):
                element = ElementTree.fromstring(f'<field><constraint>{bounds}</constraint></field>')
                constraints = parser.parse_attribute_constraints(element)
                constraints.require_supported()
                for actual in (constraints.min_value, constraints.max_value):
                    asserts.assert_equal(actual, expected, value)
                    asserts.assert_equal(type(actual), type(expected), value)

        for value in ('0xGG', '-0x20', '+0x20', '1.2.3', '1e', 'Floor - 1'):
            element = ElementTree.fromstring(f'<field><constraint><min value="{value}"/></constraint></field>')
            constraints = parser.parse_attribute_constraints(element)
            asserts.assert_is_none(constraints.min_value, value)
            asserts.assert_is_none(constraints.min_value_ref, value)
            with asserts.assert_raises(SpecParsingException):
                constraints.require_supported()

    def test_aliased_clusters(self):
        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        cluster_xml = ElementTree.fromstring(ALIASED_CLUSTERS)

        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)
        asserts.assert_equal(len(problems), 0, "Unexpected problem parsing aliased clusters")
        asserts.assert_equal(len(clusters), 2, "Unexpected number of clusters when parsing aliased cluster set")
        asserts.assert_equal(len(pure_base_clusters), 0, "Unexpected number of pure base clusters")
        asserts.assert_equal(len(ids_by_name), 2, "Unexpected number of ids by name")

        ids = [(_id, c.name) for _id, c in clusters.items()]
        asserts.assert_true((0xFFFE, 'Test Alias1') in ids, "Unable to find Test Alias1 cluster in parsed clusters")
        asserts.assert_true((0xFFFD, 'Test Alias2') in ids, "Unable to find Test Alias2 cluster in parsed clusters")

        # Test Alias2 is marked as provisional, and TestAlias1 is not
        asserts.assert_false(clusters[0xFFFE].is_provisional, "Test Alias1 is marked as provisional and should not be")
        asserts.assert_true(clusters[0xFFFD].is_provisional, "Test Alias2 is not marked as provisional and should be")

    def test_known_aliased_clusters(self):
        known_aliased_clusters = {
            (0x040C, 'Carbon Monoxide Concentration Measurement', 'CMOCONC'),
            (0x040D, 'Carbon Dioxide Concentration Measurement', 'CDOCONC'),
            (0x0413, 'Nitrogen Dioxide Concentration Measurement', 'NDOCONC'),
            (0x0415, 'Ozone Concentration Measurement', 'OZCONC'),
            (0x042A, 'PM2.5 Concentration Measurement', 'PMICONC'),
            (0x042B, 'Formaldehyde Concentration Measurement', 'FLDCONC'),
            (0x042C, 'PM1 Concentration Measurement', 'PMHCONC'),
            (0x042D, 'PM10 Concentration Measurement', 'PMKCONC'),
            (0x042E, 'Total Volatile Organic Compounds Concentration Measurement', 'TVOCCONC'),
            (0x042F, 'Radon Concentration Measurement', 'RNCONC'),
            (0x0071, 'HEPA Filter Monitoring', 'HEPAFREMON'),
            (0x0072, 'Activated Carbon Filter Monitoring', 'ACFREMON'),
            (0x0079, 'Water Tank Level Monitoring', 'WTLREPMON'),
            (0x0405, 'Relative Humidity Measurement', 'RH'),
        }

        missing_clusters = known_aliased_clusters - self.all_spec_clusters
        asserts.assert_equal(len(missing_clusters), 0, f"Missing aliased clusters from DM XML - {missing_clusters}")

    def test_known_derived_clusters(self):
        known_derived_clusters = {
            (0x0048, 'Oven Cavity Operational State', 'OVENOPSTATE'),
            (0x0049, 'Oven Mode', 'OTCCM'),
            (0x0051, 'Laundry Washer Mode', 'LWM'),
            (0x0052, 'Refrigerator And Temperature Controlled Cabinet Mode', 'TCCM'),
            (0x0054, 'RVC Run Mode', 'RVCRUNM'),
            (0x0055, 'RVC Clean Mode', 'RVCCLEANM'),
            (0x0057, 'Refrigerator Alarm', 'REFALM'),
            (0x0059, 'Dishwasher Mode', 'DISHM'),
            (0x005c, 'Smoke CO Alarm', 'SMOKECO'),
            (0x005d, 'Dishwasher Alarm', 'DISHALM'),
            (0x005e, 'Microwave Oven Mode', 'MWOM'),
            (0x0061, 'RVC Operational State', 'RVCOPSTATE'),
        }

        missing_clusters = known_derived_clusters - self.all_spec_clusters
        asserts.assert_equal(len(missing_clusters), 0, f"Missing aliased clusters from DM XML - {missing_clusters}")
        for d in known_derived_clusters:
            asserts.assert_true(self.spec_xml_clusters is not None, "Derived cluster with no base cluster marker")

    def test_provisional_clusters(self):
        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        cid = 0x0001

        environment = jinja2.Environment()
        template = environment.from_string(PROVISIONAL_CLUSTER_TEMPLATE)

        provisional = template.render(provisional=True, id=cid)
        cluster_xml = ElementTree.fromstring(provisional)
        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)

        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing provisional cluster")
        asserts.assert_in(cid, clusters.keys(), "Provisional cluster not parsed")
        asserts.assert_true(clusters[cid].is_provisional, "Provisional cluster not marked as provisional")

        non_provisional = template.render(provisional=False, id=cid)
        cluster_xml = ElementTree.fromstring(non_provisional)
        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)

        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing non-provisional cluster")
        asserts.assert_in(cid, clusters.keys(), "Non-provisional cluster not parsed")
        asserts.assert_false(clusters[cid].is_provisional, "Non-provisional cluster marked as provisional")

    def test_atomic_thermostat(self):
        one_four_two_xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_2)
        one_three_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_3)
        one_four_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)

        asserts.assert_in("Atomic Request", one_four_two_xml_clusters[Clusters.Thermostat.id].command_map,
                          "Atomic request not found on thermostat command map")
        request_id = one_four_two_xml_clusters[Clusters.Thermostat.id].command_map["Atomic Request"]
        asserts.assert_in(request_id, one_four_two_xml_clusters[Clusters.Thermostat.id].accepted_commands.keys(),
                          "Atomic request not found in thermostat accepted command list")

        asserts.assert_in("Atomic Response", one_four_clusters[Clusters.Thermostat.id].command_map,
                          "Atomic response not found in the thermostat command map")
        response_id = one_four_two_xml_clusters[Clusters.Thermostat.id].command_map["Atomic Response"]
        asserts.assert_in(response_id, one_four_clusters[Clusters.Thermostat.id].generated_commands.keys(),
                          "Atomic response not found in thermostat generated command list")

        asserts.assert_not_in(
            "Atomic Request", one_three_clusters[Clusters.Thermostat.id].command_map, "Atomic request found on thermostat command map for 1.3")
        asserts.assert_not_in(request_id, one_three_clusters[Clusters.Thermostat.id].accepted_commands.keys(),
                              "Atomic request found in thermostat accepted command list for 1.3")
        asserts.assert_not_in(
            "Atomic Response", one_three_clusters[Clusters.Thermostat.id].command_map, "Atomic response found on thermostat command map for 1.3")
        asserts.assert_not_in(response_id, one_three_clusters[Clusters.Thermostat.id].generated_commands.keys(),
                              "Atomic request found in thermostat generated command list for 1.3")

    def test_is_scene_attribute(self):
        # Each case is (attribute XML, expected scene flag).
        cases = [
            ('<attribute><quality scene="true"/></attribute>', True),
            # Case-insensitive parsing of the attribute value.
            ('<attribute><quality scene="TRUE"/></attribute>', True),
            ('<attribute><quality scene="false"/></attribute>', False),
            # scene attribute absent from the quality element defaults to False.
            ('<attribute><quality nullable="true"/></attribute>', False),
            # No quality element at all defaults to False.
            ('<attribute/>', False),
        ]
        for attribute_xml, expected in cases:
            element = ElementTree.fromstring(attribute_xml)
            asserts.assert_equal(ClusterParser._is_scene_attribute(element), expected,
                                 f"Unexpected scene quality parsed from '{attribute_xml}'")

    def test_command_field_parsing(self):
        # Synthetic cluster with one command exercising the constraint vocabulary
        # used by command fields in the spec XML: exact length via <allowed>,
        # min/max length, between with numeric bounds, and an attribute reference.
        command_xml = (
            f'<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" id="{CLUSTER_ID}" name="{CLUSTER_NAME}" revision="1">'
            '  <revisionHistory>'
            '    <revision revision="1" summary="Initial version"/>'
            '  </revisionHistory>'
            f' <clusterIds><clusterId id="{CLUSTER_ID}" name="{CLUSTER_NAME}"/></clusterIds>'
            '  <classification hierarchy="base" role="application" picsCode="TEST" scope="Endpoint"/>'
            '  <commands>'
            '    <command id="0x00" name="Cmd" direction="commandToServer" response="Y">'
            '      <access invokePrivilege="operate"/>'
            '      <mandatoryConform/>'
            '      <field id="0" name="ExactLengthKey" type="octstr">'
            '        <mandatoryConform/>'
            '        <constraint><allowed value="16"/></constraint>'
            '      </field>'
            '      <field id="1" name="BoundedName" type="string">'
            '        <optionalConform/>'
            '        <constraint><maxLength value="32"/></constraint>'
            '      </field>'
            '      <field id="2" name="BoundedNumber" type="uint16">'
            '        <mandatoryConform/>'
            '        <quality nullable="true"/>'
            '        <constraint><between><from value="1"/><to value="100"/></between></constraint>'
            '      </field>'
            '      <field id="3" name="RefBounded" type="uint8">'
            '        <mandatoryConform/>'
            '        <constraint><between><from value="1"/><to><attribute name="MaxThing"/></to></between></constraint>'
            '      </field>'
            '      <field id="4" name="Unconstrained" type="uint64">'
            '        <mandatoryConform/>'
            '      </field>'
            '    </command>'
            '  </commands>'
            '</cluster>')
        xml_cluster = parse_cluster(command_xml)
        asserts.assert_in(0x00, xml_cluster.accepted_commands, "Cmd not found in accepted commands")
        fields = xml_cluster.accepted_commands[0x00].fields
        asserts.assert_equal(sorted(fields.keys()), [0, 1, 2, 3, 4], "Unexpected command field IDs")

        exact = fields[0]
        asserts.assert_equal(exact.name, "ExactLengthKey", "Unexpected field name")
        asserts.assert_equal(exact.type_info, "octstr", "Unexpected field type")
        asserts.assert_equal(exact.constraints.allowed, ["16"], "Expected exact-length allowed constraint")

        bounded_name = fields[1]
        asserts.assert_true(bounded_name.is_optional, "BoundedName should be optional")
        asserts.assert_equal(bounded_name.constraints.max_length, 32, "Unexpected max_length")

        bounded_number = fields[2]
        asserts.assert_true(bounded_number.is_nullable, "BoundedNumber should be nullable")
        asserts.assert_equal(bounded_number.constraints.min_value, 1, "Unexpected min_value")
        asserts.assert_equal(bounded_number.constraints.max_value, 100, "Unexpected max_value")

        ref_bounded = fields[3]
        asserts.assert_equal(ref_bounded.constraints.min_value, 1, "Unexpected min_value")
        asserts.assert_is_none(ref_bounded.constraints.max_value, "max_value should be unresolved (reference)")
        asserts.assert_equal(ref_bounded.constraints.max_value_ref.attribute, "MaxThing", "Unexpected max_value_ref")

        asserts.assert_is_none(fields[4].constraints, "Unconstrained field should have no constraints")

    def test_command_field_sibling_constraints(self):
        # The spec XML expresses alternatives as repeated <constraint> siblings rather
        # than one element holding several children (e.g. AudioStreamAllocate.BitDepth,
        # SetUser.UserType). Every alternative must survive parsing, and bounds from an
        # earlier sibling must not be lost when an <allowed> sibling follows them
        # (ClearWeekDaySchedule.WeekDayIndex).
        command_xml = (
            f'<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" id="{CLUSTER_ID}" name="{CLUSTER_NAME}" revision="1">'
            '  <revisionHistory><revision revision="1" summary="Initial version"/></revisionHistory>'
            f' <clusterIds><clusterId id="{CLUSTER_ID}" name="{CLUSTER_NAME}"/></clusterIds>'
            '  <classification hierarchy="base" role="application" picsCode="TEST" scope="Endpoint"/>'
            '  <commands>'
            '    <command id="0x00" name="Cmd" direction="commandToServer" response="Y">'
            '      <access invokePrivilege="operate"/>'
            '      <mandatoryConform/>'
            '      <field id="0" name="BitDepth" type="uint8">'
            '        <mandatoryConform/>'
            '        <constraint><allowed value="8"/></constraint>'
            '        <constraint><allowed value="16"/></constraint>'
            '        <constraint><allowed value="24"/></constraint>'
            '        <constraint><allowed value="32"/></constraint>'
            '      </field>'
            '      <field id="1" name="UserType" type="UserTypeEnum">'
            '        <mandatoryConform/>'
            '        <constraint><allowed><enum value="UnrestrictedUser"/></allowed></constraint>'
            '        <constraint><allowed><enum value="NonAccessUser"/></allowed></constraint>'
            '      </field>'
            '      <field id="2" name="IndexOrWildcard" type="uint8">'
            '        <mandatoryConform/>'
            '        <constraint><between><from value="1"/><to value="10"/></between></constraint>'
            '        <constraint><allowed value="0xFE"/></constraint>'
            '      </field>'
            '    </command>'
            '  </commands>'
            '</cluster>')
        fields = parse_cluster(command_xml).accepted_commands[0x00].fields

        asserts.assert_equal(fields[0].constraints.allowed, ["8", "16", "24", "32"],
                             "Every allowed value from the sibling constraints should be retained")
        asserts.assert_equal(fields[1].constraints.allowed, ["UnrestrictedUser", "NonAccessUser"],
                             "Every allowed enum value from the sibling constraints should be retained")

        wildcard = fields[2]
        asserts.assert_equal(wildcard.constraints.min_value, 1, "Unexpected min_value")
        asserts.assert_equal(wildcard.constraints.max_value, 10, "Unexpected max_value")
        asserts.assert_equal(wildcard.constraints.allowed, ["0xFE"],
                             "Sentinel allowed value from the trailing sibling constraint should be retained")

    def test_command_field_parsing_prebuilt_data_model(self):
        # Spot-check two constraints that exist on every root node in the real
        # data model: TestEventTrigger.EnableKey (exact length 16 octstr) and
        # SetRegulatoryConfig.CountryCode (exact length 2 string).
        clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_6_1)

        test_event_trigger = clusters[Clusters.GeneralDiagnostics.id].accepted_commands[
            Clusters.GeneralDiagnostics.Commands.TestEventTrigger.command_id]
        enable_key = next(f for f in test_event_trigger.fields.values() if f.name == "EnableKey")
        asserts.assert_equal(enable_key.type_info, "octstr", "Unexpected EnableKey type")
        asserts.assert_equal(enable_key.constraints.allowed, ["16"], "Unexpected EnableKey constraint")

        set_regulatory_config = clusters[Clusters.GeneralCommissioning.id].accepted_commands[
            Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig.command_id]
        country_code = next(f for f in set_regulatory_config.fields.values() if f.name == "CountryCode")
        asserts.assert_equal(country_code.type_info, "string", "Unexpected CountryCode type")
        asserts.assert_equal(country_code.constraints.allowed, ["2"], "Unexpected CountryCode constraint")

    def test_scene_attribute_end_to_end(self):
        # CurrentMode (0x0001) in the base cluster XML carries scene="true", the
        # other attributes carry scene="false".
        xml_cluster = parse_cluster(BASE_CLUSTER_XML_STR)
        asserts.assert_true(xml_cluster.attributes[0x0001].scene,
                            "CurrentMode should carry the Scene (S) quality")
        for attribute_id in (0x0000, 0x0002, 0x0003):
            asserts.assert_false(xml_cluster.attributes[attribute_id].scene,
                                 f"Attribute {attribute_id:#06x} should not carry the Scene (S) quality")


if __name__ == "__main__":
    default_matter_test_main()
