#
#    Copyright (c) 2025 Project CHIP Authors
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

import os
import sys

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, default_matter_test_main
from chip.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_device_types

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

try:
    import matter_device_graph
except ModuleNotFoundError:
    sys.path.append(os.path.join(DEFAULT_CHIP_ROOT, 'src', 'tools', 'device-graph'))
    import matter_device_graph

try:
    import TestMatterTestingSupport
except ModuleNotFoundError:
    sys.path.append(os.path.join(DEFAULT_CHIP_ROOT, 'src', 'python_testing'))
    import TestMatterTestingSupport


class TestMatterDeviceGraph(MatterBaseTest):
    def test_graph_script(self):
        # Use the example topology from the TestMatterTestingSupport script
        # The topology used there is a 21-endpoint system with multiple trees and an aggregator
        endpoints = TestMatterTestingSupport.TestMatterTestingSupport.create_example_topology()

        # The topology has descriptors on every endpoint, but doesn't include any server or client clusters
        # Let's add some.
        # We can just add two of the same on every endpoint
        for e in endpoints.keys():
            endpoints[e][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList] = [
                Clusters.ColorControl.id, Clusters.OnOff.id]
            endpoints[e][Clusters.Descriptor][Clusters.Descriptor.Attributes.ClientList] = [Clusters.Identify.id]

        # Run the script to ensure it doesn't throw any errors
        xml_device_types, _ = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_1)
        matter_device_graph.create_graph(endpoints, xml_device_types)


if __name__ == "__main__":
    default_matter_test_main()
