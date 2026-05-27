#
#    Copyright (c) 2024 Project CHIP Authors
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

import logging
from xml.etree import ElementTree

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.device_conformance_tests import DeviceConformanceTests
from matter.testing.spec_parsing import parse_single_device_type, XmlCluster, XmlFeature, parse_callable_from_xml, ConformanceParseParameters
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# A mock XML representing a device type with composedDeviceTypes
MOCK_DEVICE_TYPE_XML = """
<deviceType id="0x0001" name="Parent Device" revision="1">
  <classification class="simple" scope="endpoint"/>
  <composedDeviceTypes>
    <deviceType deviceTypeId="0x0002" deviceTypeName="Child Device">
      <mandatoryConform/>
      <constraint>
        <min value="2"/>
      </constraint>
    </deviceType>
    <deviceType deviceTypeId="0x0002" deviceTypeName="Child Device">
      <mandatoryConform/>
      <clusterRequirements>
        <cluster id="0x0090" name="Cluster X">
          <mandatoryConform/>
          <features>
            <feature code="ALTC">
              <mandatoryConform/>
            </feature>
          </features>
        </cluster>
      </clusterRequirements>
    </deviceType>
  </composedDeviceTypes>
</deviceType>
"""

class TestSpecParsingComposedDeviceTypes(DeviceConformanceTests):
    def setup_class(self):
        self.xml_device_types = {}
        self.xml_clusters = {}
        self.problems = []

        # Parse the mock XML
        root = ElementTree.fromstring(MOCK_DEVICE_TYPE_XML)
        
        # We need to mock the cluster definition for the parser to resolve features
        mock_cluster = XmlCluster(
            name="Cluster X",
            revision=1,
            derived=None,
            features={0x01: XmlFeature(code="ALTC", name="Alternate Cluster", conformance=parse_callable_from_xml(ElementTree.Element('mandatoryConform'), ConformanceParseParameters({}, {}, {})))},
            feature_map={"ALTC": 0x01}, # Map code to mask
            attribute_map={},
            command_map={},
            attributes={},
            accepted_commands={},
            generated_commands={},
            unknown_commands=[],
            events={},
            structs={},
            enums={},
            bitmaps={},
            pics="",
            is_provisional=False,
            revision_desc={}
        )
        mock_clusters = {0x0090: mock_cluster}
        
        device_types, problems = parse_single_device_type(root, mock_clusters)
        self.problems.extend(problems)
        self.xml_device_types.update(device_types)
        self.xml_clusters.update(mock_clusters)

    def test_parsing_correctness(self):
        asserts.assert_equal(len(self.problems), 0, f"Unexpected problems during parsing: {self.problems}")
        
        dt_parent_id = 0x0001
        asserts.assert_in(dt_parent_id, self.xml_device_types, "Parent device type not found in parsed output")
        
        xml_device = self.xml_device_types[dt_parent_id]
        asserts.assert_equal(len(xml_device.composed_device_types), 2, "Expected 2 composed device type requirements")
        
        # Check base requirement
        req_base = xml_device.composed_device_types[0]
        asserts.assert_equal(req_base.min_instances, 2, "Expected min_instances = 2 for base requirement")
        
        # Check override requirement
        req_override = xml_device.composed_device_types[1]
        asserts.assert_in(0x0090, req_override.cluster_requirements, "Expected cluster requirement for 0x0090")
        
        cr = req_override.cluster_requirements[0x0090]
        asserts.assert_in(0x01, cr.feature_overrides, "Expected feature override for mask 0x01")

    def test_enforcement_with_fake_device(self):
        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        cluster_x_id = 0x0090
        
        # Scenario: We need at least 2 child devices.
        # One of them must have Cluster X with feature ALTC (mask 0x01).
        
        # Create mock endpoints
        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: [2, 3]
                }
            },
            2: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_x_id]
                },

            }
        }
        
        # Let's use the cluster class as key!
        cluster_class = Clusters.ClustersObjects.ALL_CLUSTERS[cluster_x_id] if hasattr(Clusters, 'ClustersObjects') else None
        # Wait, let's check how to get cluster class in TestConformanceTest!
        # It used `Clusters.ClusterObjects.ALL_CLUSTERS[cid]`!
        
        cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_x_id]
        
        self.endpoints[2][cluster_class] = {
            cluster_class.Attributes.FeatureMap: 0x01, # Has ALTC!
            cluster_class.Attributes.AttributeList: [],
            cluster_class.Attributes.AcceptedCommandList: [],
            cluster_class.Attributes.ClusterRevision: 1
        }
        
        # EP3: Child device without Cluster X (to satisfy min 2 constraint)
        self.endpoints[3] = {
            Clusters.Descriptor: {
                Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                Clusters.Descriptor.Attributes.ServerList: []
            }
        }
        
        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure on enforcement with fake device")

if __name__ == "__main__":
    default_matter_test_main()
