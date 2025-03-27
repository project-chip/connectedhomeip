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

import chip.clusters as Clusters
import jinja2
from chip.testing.global_attribute_ids import GlobalAttributeIds
from chip.testing.matter_testing import MatterBaseTest, ProblemNotice, default_matter_test_main
from chip.testing.spec_parsing import (ClusterParser, DataModelLevel, PrebuiltDataModelDirectory, XmlCluster,
                                       add_cluster_data_from_xml, build_xml_clusters, check_clusters_for_unknown_commands,
                                       combine_derived_clusters_with_base, get_data_model_directory)
from mobly import asserts

# TODO: improve the test coverage here
# https://github.com/project-chip/connectedhomeip/issues/30958

CLUSTER_ID = 0x0BEE
CLUSTER_NAME = "TestCluster"
ATTRIBUTE_NAME = "TestAttribute"
ATTRIBUTE_ID = 0x0000


def single_attribute_cluster_xml(read_access: str, write_access: str, write_supported: str):
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

    return (f'{xml_cluster}'
            f'{revision_table}'
            f'{id_table}'
            f'{classification}'
            f'{attribute}'
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


class TestSpecParsingSupport(MatterBaseTest):
    def setup_class(self):
        super().setup_class()
        # Latest fully certified build
        self.spec_xml_clusters, self.spec_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        self.all_spec_clusters = set([(id, c.name, c.pics) for id, c in self.spec_xml_clusters.items()])

    def test_build_xml_override(self):
        # checks that the 1.3 spec (default) does not contain in-progress clusters and the TOT does
        tot_xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.kMaster)
        one_three_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_3)
        one_four_clusters, one_four_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        one_four_one_clusters, one_four_one_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)

        # We know 1.4 and 1.4.1 are clear of errors, ensure it stays that way.
        asserts.assert_equal(len(one_four_problems), 0, "Unexpected problems found on 1.4 cluster parsing")
        asserts.assert_equal(len(one_four_one_problems), 0, "Unexpected problems found on 1.4.1 cluster parsing")

        asserts.assert_greater(len(set(tot_xml_clusters.keys()) - set(one_three_clusters.keys())),
                               0, "Master dir does not contain any clusters not in 1.3")
        asserts.assert_greater(len(set(tot_xml_clusters.keys()) - set(one_four_clusters.keys())),
                               0, "Master dir does not contain any clusters not in 1.4")
        asserts.assert_greater(len(set(one_four_clusters.keys()) - set(one_three_clusters.keys())),
                               0, "1.4 dir does not contain any clusters not in 1.3")
        asserts.assert_equal(len(one_four_clusters.keys()), len(one_four_one_clusters.keys()),
                             "1.4 and 1.4.1 do not contain the same clusters")
        # only the pulse width modulation cluster was removed post 1.3
        one_four_removed = set([Clusters.PulseWidthModulation.id])
        asserts.assert_equal(set(one_three_clusters.keys()) - set(one_four_clusters.keys()),
                             one_four_removed, "There are some 1.3 clusters that are unexpectedly not included in the 1.4 spec")
        # Ballast and all the proxy clusters are being removed in 1.5
        one_five_removed = set([Clusters.BallastConfiguration.id, Clusters.ProxyConfiguration.id,
                               Clusters.ProxyDiscovery.id, Clusters.ProxyValid.id])
        asserts.assert_equal(set(one_four_clusters.keys())-set(tot_xml_clusters.keys()),
                             one_five_removed, "There are some 1.4 clusters that are unexpectedly not included in the TOT spec")
        asserts.assert_equal(set(one_three_clusters.keys())-set(tot_xml_clusters.keys()),
                             one_four_removed.union(one_five_removed), "There are some 1.3 clusters that are unexpectedly not included in the TOT spec")

        str_path = get_data_model_directory(PrebuiltDataModelDirectory.k1_4, DataModelLevel.kCluster)
        string_override_check, problems = build_xml_clusters(str_path)

        asserts.assert_count_equal(string_override_check.keys(), self.spec_xml_clusters.keys(), "Mismatched cluster generation")

    def test_spec_parsing_access(self):
        strs = [None, 'view', 'operate', 'manage', 'admin']
        for read in strs:
            for write in strs:
                xml = single_attribute_cluster_xml(read, write, "true")
                xml_cluster = parse_cluster(xml)
                asserts.assert_is_not_none(xml_cluster.attributes, "No attributes found in cluster")
                asserts.assert_is_not_none(xml_cluster.attribute_map, "No attribute map found in cluster")
                asserts.assert_equal(len(xml_cluster.attributes), len(GlobalAttributeIds) + 1, "Unexpected number of attributes")
                asserts.assert_true(ATTRIBUTE_ID in xml_cluster.attributes.keys(),
                                    "Did not find test attribute in XmlCluster.attributes")
                asserts.assert_equal(xml_cluster.attributes[ATTRIBUTE_ID].read_access,
                                     get_access_enum_from_string(read), "Unexpected read access")
                asserts.assert_equal(xml_cluster.attributes[ATTRIBUTE_ID].write_access,
                                     get_access_enum_from_string(write), "Unexpected write access")

    def test_write_optional(self):
        for write_support in ['true', 'optional']:
            xml = single_attribute_cluster_xml('view', 'view', write_support)
            xml_cluster = parse_cluster(xml)
            asserts.assert_is_not_none(xml_cluster.attributes, "No attributes found in cluster")
            asserts.assert_is_not_none(xml_cluster.attribute_map, "No attribute map found in cluster")
            asserts.assert_equal(len(xml_cluster.attributes), len(GlobalAttributeIds) + 1, "Unexpected number of attributes")
            asserts.assert_true(ATTRIBUTE_ID in xml_cluster.attributes.keys(),
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
        asserts.assert_equal(set(clusters[0xFFFF].accepted_commands.keys()), set([]), "Unexpected accepted commands")
        asserts.assert_equal(set(clusters[0xFFFF].generated_commands.keys()), set([]), "Unexpected generated commands")

        asserts.assert_true("Test Base" in pure_base_clusters, "Base ID not found in derived clusters")
        asserts.assert_equal(set(pure_base_clusters["Test Base"].attributes.keys()), set(
            [0, 1, 2, 3] + expected_global_attrs), "Unexpected attribute list")
        asserts.assert_equal(set(pure_base_clusters["Test Base"].accepted_commands.keys()),
                             set([0]), "Unexpected accepted commands")
        asserts.assert_equal(set(pure_base_clusters["Test Base"].generated_commands.keys()),
                             set([1]), "Unexpected generated commands")
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
                            set([0]), "Unexpected accepted command list after merge")
        asserts.assert_true(set(clusters[0xFFFF].generated_commands.keys()), set([1]),
                            "Unexpected generated command list after merge")
        asserts.assert_equal(str(clusters[0xFFFF].accepted_commands[0].conformance),
                             "X", "Unexpected conformance on accepted commands")
        asserts.assert_equal(str(clusters[0xFFFF].generated_commands[1].conformance),
                             "X", "Unexpected conformance on generated commands")
        asserts.assert_equal(len(clusters[0xFFFF].unknown_commands), 0, "Unexpected number of unknown commands after merge")

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

        ids = [(id, c.name) for id, c in clusters.items()]
        asserts.assert_true((0xFFFE, 'Test Alias1') in ids, "Unable to find Test Alias1 cluster in parsed clusters")
        asserts.assert_true((0xFFFD, 'Test Alias2') in ids, "Unable to find Test Alias2 cluster in parsed clusters")

        # Test Alias2 is marked as provisional, and TestAlias1 is not
        asserts.assert_false(clusters[0xFFFE].is_provisional, "Test Alias1 is marked as provisional and should not be")
        asserts.assert_true(clusters[0xFFFD].is_provisional, "Test Alias2 is not marked as provisional and should be")

    def test_known_aliased_clusters(self):
        known_aliased_clusters = set([(0x040C, 'Carbon Monoxide Concentration Measurement', 'CMOCONC'),
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
                                      (0x0405, 'Relative Humidity Measurement', 'RH')])

        missing_clusters = known_aliased_clusters - self.all_spec_clusters
        asserts.assert_equal(len(missing_clusters), 0, f"Missing aliased clusters from DM XML - {missing_clusters}")

    def test_known_derived_clusters(self):
        known_derived_clusters = set([(0x0048, 'Oven Cavity Operational State', 'OVENOPSTATE'),
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
                                      (0x0061, 'RVC Operational State', 'RVCOPSTATE')])

        missing_clusters = known_derived_clusters - self.all_spec_clusters
        asserts.assert_equal(len(missing_clusters), 0, f"Missing aliased clusters from DM XML - {missing_clusters}")
        for d in known_derived_clusters:
            asserts.assert_true(self.spec_xml_clusters is not None, "Derived cluster with no base cluster marker")

    def test_provisional_clusters(self):
        clusters: dict[int, XmlCluster] = {}
        pure_base_clusters: dict[str, XmlCluster] = {}
        ids_by_name: dict[str, int] = {}
        problems: list[ProblemNotice] = []
        id = 0x0001

        environment = jinja2.Environment()
        template = environment.from_string(PROVISIONAL_CLUSTER_TEMPLATE)

        provisional = template.render(provisional=True, id=id)
        cluster_xml = ElementTree.fromstring(provisional)
        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)

        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing provisional cluster")
        asserts.assert_in(id, clusters.keys(), "Provisional cluster not parsed")
        asserts.assert_true(clusters[id].is_provisional, "Provisional cluster not marked as provisional")

        non_provisional = template.render(provisional=False, id=id)
        cluster_xml = ElementTree.fromstring(non_provisional)
        add_cluster_data_from_xml(cluster_xml, clusters, pure_base_clusters, ids_by_name, problems)

        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing non-provisional cluster")
        asserts.assert_in(id, clusters.keys(), "Non-provisional cluster not parsed")
        asserts.assert_false(clusters[id].is_provisional, "Non-provisional cluster marked as provisional")

    def test_atomic_thermostat(self):
        tot_xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.kMaster)
        one_three_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_3)
        one_four_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)

        asserts.assert_in("Atomic Request", tot_xml_clusters[Clusters.Thermostat.id].command_map,
                          "Atomic request not found on thermostat command map")
        request_id = tot_xml_clusters[Clusters.Thermostat.id].command_map["Atomic Request"]
        asserts.assert_in(request_id, tot_xml_clusters[Clusters.Thermostat.id].accepted_commands.keys(),
                          "Atomic request not found in thermostat accepted command list")

        asserts.assert_in("Atomic Response", one_four_clusters[Clusters.Thermostat.id].command_map,
                          "Atomic response not found in the thermostat command map")
        response_id = tot_xml_clusters[Clusters.Thermostat.id].command_map["Atomic Response"]
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


if __name__ == "__main__":
    default_matter_test_main()
