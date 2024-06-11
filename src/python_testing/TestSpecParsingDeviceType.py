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

from jinja2 import Template
from matter_testing_support import MatterBaseTest, default_matter_test_main
from mobly import asserts
from spec_parsing_support import build_xml_device_types, parse_single_device_type


class TestSpecParsingDeviceType(MatterBaseTest):

    # This just tests that the current spec can be parsed without failures
    def test_spec_device_parsing(self):
        device_types, problems = build_xml_device_types()
        self.problems += problems
        for id, d in device_types.items():
            print(str(d))

    def test_device_parsing_clusters(self):
        device_type_id = 0xBBEF
        revision = 2
        classification_class = "simple"
        classification_scope = "endpoint"
        clusters = {0x0003: "Identify", 0x0004: "Groups"}
        # Conformance support tests the different types of conformance for clusters, so here we just want to ensure that we're correctly parsing the XML into python
        # adds the same attributes and features to every cluster. This is fine for testing.
        template = Template('<deviceType xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd devicetype devicetype.xsd" id="{{ device_type_id }}" name="Test Device Type" revision="{{ revision }}">\n'
                            '<revisionHistory>\n'
                            '{% for i in range(revision) %}'
                            '<revision revision="{{ i }}" summary="Rev"/>\n'
                            '{% endfor %}'
                            '</revisionHistory>\n'
                            '<classification {% if classification_class %} class="{{ classification_class }}" {% endif %} {% if classification_scope %} scope="{{ classification_scope }}" {% endif %}/>\n'
                            '<conditions/>\n'
                            '<clusters>\n'
                            '{% for k,v in clusters.items() %}'
                            '<cluster id="{{ k }}" name="{{ v }}" side="server">\n'
                            '<mandatoryConform/>\n'
                            '</cluster>\n'
                            '{% endfor %}'
                            '</clusters>\n'
                            '</deviceType>\n')

        xml = template.render(device_type_id=device_type_id, revision=revision, classification_class=classification_class,
                              classification_scope=classification_scope, clusters=clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(device_type_id in device_type.keys(), "device type id not found in returned data")
        asserts.assert_equal(device_type[device_type_id].revision, revision, "Unexpected revision")
        asserts.assert_equal(len(device_type[device_type_id].server_clusters), len(clusters), "Unexpected number of clusters")
        for id, name in clusters.items():
            asserts.assert_equal(device_type[device_type_id].server_clusters[id].name, name, "Incorrect cluster name")
            asserts.assert_equal(str(device_type[device_type_id].server_clusters[id].conformance),
                                 'M', 'Incorrect cluster conformance')

        clusters = {}
        xml = template.render(device_type_id=device_type_id, revision=revision, classification_class=classification_class,
                              classification_scope=classification_scope, clusters=clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(device_type_id in device_type.keys(), "device type id not found in returned data")
        asserts.assert_equal(device_type[device_type_id].revision, revision, "Unexpected revision")
        asserts.assert_equal(len(device_type[device_type_id].server_clusters), len(clusters), "Unexpected number of clusters")

        # Bad device type ID
        xml = template.render(device_type_id="", revision=revision, classification_class=classification_class,
                              classification_scope=classification_scope, clusters=clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 1, "Device with blank ID did not generate a problem notice")

        # Bad class
        xml = template.render(device_type_id=device_type_id, revision=revision, classification_class="",
                              classification_scope=classification_scope, clusters=clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 1, "Device with no class did not generate a problem notice")

        # Bad scope
        xml = template.render(device_type_id=device_type_id, revision=revision, classification_class=classification_class,
                              classification_scope="", clusters=clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 1, "Device with no scope did not generate a problem notice")


if __name__ == "__main__":
    default_matter_test_main()
