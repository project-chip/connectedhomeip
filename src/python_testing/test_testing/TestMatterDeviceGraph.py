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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import (PrebuiltDataModelDirectory, XmlNamespace, XmlTag, build_xml_device_types,
                                         build_xml_namespaces)

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

try:
    import matter_device_graph
except ModuleNotFoundError:
    sys.path.append(os.path.join(DEFAULT_CHIP_ROOT, 'src', 'tools', 'device-graph'))
    import matter_device_graph

try:
    import TestMatterTestingSupport
except ModuleNotFoundError:
    sys.path.append(os.path.join(DEFAULT_CHIP_ROOT, 'src', 'python_testing', 'test_testing'))
    import TestMatterTestingSupport


SemanticTagStruct = Clusters.Globals.Structs.SemanticTagStruct


class TestMatterDeviceGraph(MatterBaseTest):
    def test_tag_str(self):
        # Build a small in-memory namespace map without touching XML files.
        xml_namespaces = {
            0x0008: XmlNamespace(
                id=0x0008,
                name="Common Position",
                tags={
                    0x0000: XmlTag(id=0x0000, name="Left"),
                    0x0001: XmlTag(id=0x0001, name="Right"),
                },
            ),
        }

        # Fully resolved namespace and tag.
        resolved = SemanticTagStruct(mfgCode=NullValue, namespaceID=0x0008, tag=0x0000)
        asserts.assert_equal(matter_device_graph.tag_str(resolved, xml_namespaces),
                             "(Common Position: Left)")

        # Resolved namespace, unknown tag id.
        unknown_tag = SemanticTagStruct(mfgCode=NullValue, namespaceID=0x0008, tag=0x00FF)
        asserts.assert_equal(matter_device_graph.tag_str(unknown_tag, xml_namespaces),
                             "(Common Position: Unknown Tag 0x00FF)")

        # Unknown namespace id entirely.
        unknown_ns = SemanticTagStruct(mfgCode=NullValue, namespaceID=0x1234, tag=0x0005)
        asserts.assert_equal(matter_device_graph.tag_str(unknown_ns, xml_namespaces),
                             "(Unknown Namespace 0x1234: 0x0005)")

        # Manufacturer-defined namespace - never resolvable from spec XML.
        mfg_tag = SemanticTagStruct(mfgCode=0xFFF1, namespaceID=0x0042, tag=0x0007)
        asserts.assert_equal(matter_device_graph.tag_str(mfg_tag, xml_namespaces),
                             "(Mfg 0xFFF1: 0x0042: 0x0007)")

        # Label is appended in quotes when present.
        labeled = SemanticTagStruct(mfgCode=NullValue, namespaceID=0x0008, tag=0x0000, label="kitchen")
        asserts.assert_equal(matter_device_graph.tag_str(labeled, xml_namespaces),
                             '(Common Position: Left "kitchen")')

        # Empty label is treated as no label.
        empty_label = SemanticTagStruct(mfgCode=NullValue, namespaceID=0x0008, tag=0x0000, label="")
        asserts.assert_equal(matter_device_graph.tag_str(empty_label, xml_namespaces),
                             "(Common Position: Left)")

        # When no namespace map is provided, every entry falls back to hex.
        asserts.assert_equal(matter_device_graph.tag_str(resolved, None),
                             "(Unknown Namespace 0x0008: 0x0000)")

    def test_graph_script(self):
        # Use the example topology from the TestMatterTestingSupport script
        # The topology used there is a 21-endpoint system with multiple trees and an aggregator
        endpoints = TestMatterTestingSupport.TestMatterTestingSupport.create_example_topology()

        # The topology has descriptors on every endpoint, but doesn't include any server or client clusters
        # Let's add some.
        # We can just add two of the same on every endpoint
        for e in endpoints:
            endpoints[e][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList] = [
                Clusters.ColorControl.id, Clusters.OnOff.id]
            endpoints[e][Clusters.Descriptor][Clusters.Descriptor.Attributes.ClientList] = [Clusters.Identify.id]

        # Add the vendor name and product name
        endpoints[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.VendorName] = "TestVendor"
        endpoints[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.ProductName] = "TestProduct"

        xml_namespaces, _ = build_xml_namespaces(PrebuiltDataModelDirectory.k1_6)
        # Pick a real namespace from the parsed map for the resolvable case.
        resolvable_ns_id, resolvable_ns = next(iter(xml_namespaces.items()))
        resolvable_tag_id = next(iter(resolvable_ns.tags))

        # Exercise all three rendering paths (resolved, unknown namespace, mfg-defined) on
        # endpoints that exist in the example topology.
        endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            SemanticTagStruct(mfgCode=NullValue, namespaceID=resolvable_ns_id, tag=resolvable_tag_id),
            SemanticTagStruct(mfgCode=NullValue, namespaceID=0xABCD, tag=0x0001, label="custom"),
        ]
        endpoints[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            SemanticTagStruct(mfgCode=0xFFF1, namespaceID=0x0042, tag=0x0007),
        ]

        # Run the script to ensure it doesn't throw any errors
        xml_device_types, _ = build_xml_device_types(PrebuiltDataModelDirectory.k1_6)
        matter_device_graph.create_graph(endpoints, xml_device_types, xml_namespaces)


if __name__ == "__main__":
    default_matter_test_main()
