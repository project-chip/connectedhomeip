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
from global_attribute_ids import GlobalAttributeIds
from matter_testing_support import MatterBaseTest, default_matter_test_main
from mobly import asserts
from spec_parsing_support import ClusterParser, XmlCluster

# TODO: improve the test coverage here
# https://github.com/project-chip/connectedhomeip/issues/30958

CLUSTER_ID = 0x0BEE
CLUSTER_NAME = "TestCluster"
ATTRIBUTE_NAME = "TestAttribute"
ATTRIBUTE_ID = 0x0000


def single_attribute_cluster_xml(read_access: str, write_access: str):
    xml_cluster = f'<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="{CLUSTER_ID}" name="{CLUSTER_NAME}" revision="3">'
    revision_table = ('<revisionHistory>'
                      '<revision revision="1" summary="Initial Release"/>'
                      '<revision revision="2" summary="Some other revision"/>'
                      '<revision revision="3" summary="another revision"/>'
                      '</revisionHistory>')
    classification = '<classification hierarchy="base" role="utility" picsCode="TEST" scope="Node"/>'
    read_access_str = f'read="true" readPrivilege="{read_access}"' if read_access is not None else ""
    write_access_str = f'write="true" writePrivilege="{write_access}"' if write_access is not None else ""
    attribute = ('<attributes>'
                 f'<attribute id="{ATTRIBUTE_ID}" name="{ATTRIBUTE_NAME}" type="uint16" default="MS">'
                 f'<access {read_access_str} {write_access_str}/>'
                 '<quality changeOmitted="false" nullable="false" scene="false" persistence="fixed" reportable="false"/>'
                 '<mandatoryConform/>'
                 '</attribute>'
                 '</attributes>')

    return (f'{xml_cluster}'
            f'{revision_table}'
            f'{classification}'
            f'{attribute}'
            '</cluster>')


def parse_cluster(xml: str) -> XmlCluster:
    cluster = ElementTree.fromstring(xml)
    parser = ClusterParser(cluster, CLUSTER_ID, CLUSTER_NAME, False)
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


class TestSpecParsingSupport(MatterBaseTest):
    def test_spec_parsing_access(self):
        strs = [None, 'view', 'operate', 'manage', 'admin']
        for read in strs:
            for write in strs:
                xml = single_attribute_cluster_xml(read, write)
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


if __name__ == "__main__":
    default_matter_test_main()
