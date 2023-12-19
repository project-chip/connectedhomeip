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
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts
from spec_parsing_support import build_xml_device_types, parse_single_device_type
from jinja2 import Template
import xml.etree.ElementTree as ElementTree


class TestSpecParsingDeviceType(MatterBaseTest):

    # This just tests that the current spec can be parsed without failures
    def test_spec_device_parsing(self):
        problems, device_types = build_xml_device_types()
        for id, d in device_types.items():
            print(str(d))

    def test_device_parsing_clusters(self):
        device_type_id = 0xBBEF
        revision = 2
        classification_class = "simple"
        classification_scope = "endpoint"
        clusters = {0x0003: "Identify", 0x0004: "Groups"}
        # Conformance support tests the different types of conformance for clusters, so here we just want to ensure that we're correctly parsing the XML into python
        template = Template('<deviceType xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd devicetype devicetype.xsd" id="{{ device_type_id }}" name="Test Device Type" revision="{{ revision }}">\n'
                            '<revisionHistory>\n'
                            '{% for i in range(revision) %}'
                            '<revision revision="{{ i }}" summary="Rev"/>\n'
                            '{% endfor %}'
                            '</revisionHistory>\n'
                            '<classification class="{{ classification_class }}" scope="{{ classification_scope }}"/>\n'
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
        print(xml)
        et = ElementTree.fromstring(xml)
        problems, device_type = parse_single_device_type(et)
        print(device_type.keys())
        for id, d in device_type.items():
            print(device_type_id)
            print(id)
            print(str(d))
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(device_type_id in device_type.keys(), "device type id not found in returned data")
        asserts.assert_equal(device_type[device_type_id].revision, {revision}, "Unexpected revision")


if __name__ == "__main__":
    default_matter_test_main()
