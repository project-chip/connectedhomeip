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
import xml.etree.ElementTree as ElementTree

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.conformance import conformance_allowed
from chip.testing.matter_testing import MatterBaseTest, default_matter_test_main
from chip.testing.spec_parsing import (PrebuiltDataModelDirectory, build_xml_clusters, build_xml_device_types,
                                       parse_single_device_type)
from chip.tlv import uint
from jinja2 import Template
from mobly import asserts
from TC_DeviceConformance import DeviceConformanceTests


class TestSpecParsingDeviceType(MatterBaseTest):
    # This just tests that the current spec can be parsed without failures
    def test_spec_device_parsing(self):
        for id, d in self.xml_device_types.items():
            print(str(d))

    def setup_class(self):
        # Latest fully qualified release
        self.xml_clusters, self.xml_cluster_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        self.xml_device_types, self.xml_device_types_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4)

        self.device_type_id = 0xBBEF
        self.revision = 2
        self.classification_class = "simple"
        self.classification_scope = "endpoint"
        self.clusters = {0x0003: "Identify", 0x0004: "Groups"}

        # Conformance support tests the different types of conformance for clusters, so here we just want to ensure that we're correctly parsing the XML into python
        # adds the same attributes and features to every cluster. This is fine for testing.
        self.template = Template("""<deviceType xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd devicetype devicetype.xsd" id="{{ device_type_id }}" name="Test Device Type" revision="{{ revision }}">
                                    <revisionHistory>
                                    {% for i in range(revision) %}
                                    <revision revision="{{ i }}" summary="Rev"/>
                                    {% endfor %}
                                    </revisionHistory>
                                    <classification {% if classification_class %} class="{{ classification_class }}" {% endif %} {% if classification_scope %} scope="{{ classification_scope }}" {% endif %}/>
                                    <conditions/>
                                    <clusters>
                                    {% for k,v in clusters.items() %}
                                    <cluster id="{{ k }}" name="{{ v }}" side="server">
                                    <mandatoryConform/>
                                    </cluster>
                                    {% endfor %}
                                    </clusters>
                                    </deviceType>""")

    def test_device_type_clusters(self):
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(self.device_type_id in device_type.keys(), "device type id not found in returned data")
        asserts.assert_equal(device_type[self.device_type_id].revision, self.revision, "Unexpected revision")
        asserts.assert_equal(len(device_type[self.device_type_id].server_clusters),
                             len(self.clusters), "Unexpected number of clusters")
        for id, name in self.clusters.items():
            asserts.assert_equal(device_type[self.device_type_id].server_clusters[id].name, name, "Incorrect cluster name")
            asserts.assert_equal(str(device_type[self.device_type_id].server_clusters[id].conformance),
                                 'M', 'Incorrect cluster conformance')

    def test_no_clusters(self):
        clusters = {}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(self.device_type_id in device_type.keys(), "device type id not found in returned data")
        asserts.assert_equal(device_type[self.device_type_id].revision, self.revision, "Unexpected revision")
        asserts.assert_equal(len(device_type[self.device_type_id].server_clusters), len(clusters), "Unexpected number of clusters")

    def test_bad_device_type_id(self):
        xml = self.template.render(device_type_id="", revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 1, "Device with blank ID did not generate a problem notice")

    def test_bad_class(self):
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class="",
                                   classification_scope=self.classification_scope, clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 1, "Device with no class did not generate a problem notice")

    def test_bad_scope(self):
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope="", clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et)
        asserts.assert_equal(len(problems), 1, "Device with no scope did not generate a problem notice")

    # All these tests are based on the temp sensor device type because it is very simple
    # it requires temperature measurement, identify and the base devices.
    # Right now I'm not testing for binding condition.
    # The test is entirely based on the descriptor cluster so that's all I'm populating here
    # because it makes the test less complex to write.
    def create_test(self, server_list: list[uint], no_descriptor: bool = False, bad_device_id: bool = False) -> DeviceConformanceTests:
        self.test = DeviceConformanceTests()
        self.test.xml_device_types = self.xml_device_types
        self.test.xml_clusters = self.xml_clusters

        if bad_device_id:
            known_ids = list(self.test.xml_device_types.keys())
            device_type_id = [a for a in range(min(known_ids), max(known_ids)) if a not in known_ids][0]
        else:
            device_type_id = 0x0302

        resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
        if no_descriptor:
            resp.attributes = {1: {}}
        else:
            desc = Clusters.Descriptor
            server_list_attr = Clusters.Descriptor.Attributes.ServerList
            device_type_list_attr = Clusters.Descriptor.Attributes.DeviceTypeList
            device_type_list = [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=device_type_id, revision=2)]
            resp.attributes = {1: {desc: {device_type_list_attr: device_type_list, server_list_attr: server_list}}}
        self.test.endpoints = resp.attributes

    def create_good_device(self, device_type_id: int) -> DeviceConformanceTests:
        self.test = DeviceConformanceTests()
        self.test.xml_device_types = self.xml_device_types
        self.test.xml_clusters = self.xml_clusters

        resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
        desc = Clusters.Descriptor
        server_list_attr = Clusters.Descriptor.Attributes.ServerList
        device_type_list_attr = Clusters.Descriptor.Attributes.DeviceTypeList
        device_type_list = [Clusters.Descriptor.Structs.DeviceTypeStruct(
            deviceType=device_type_id, revision=self.xml_device_types[device_type_id].revision)]
        server_list = [k for k, v in self.xml_device_types[device_type_id].server_clusters.items(
        ) if conformance_allowed(v.conformance(0, [], []), False)]
        resp.attributes = {1: {desc: {device_type_list_attr: device_type_list, server_list_attr: server_list}}}

        self.test.endpoints = resp.attributes

    # Test with temp sensor with temp sensor, identify and descriptor
    def test_ts_minimal_clusters(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.Descriptor.id])
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_true(success, "Failure on Temperature Sensor device type test")

    # Temp sensor with temp sensor, identify, descriptor, binding
    def test_ts_minimal_with_binding(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.Binding.id, Clusters.Descriptor.id])
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_true(success, "Failure on Temperature Sensor device type test")
        asserts.assert_false(problems, "Found problems on Temperature sensor device type test")

    # Temp sensor with temp sensor, identify, descriptor, fixed label
    def test_ts_minimal_with_label(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.FixedLabel.id, Clusters.Descriptor.id])
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_true(success, "Failure on Temperature Sensor device type test")
        asserts.assert_false(problems, "Found problems on Temperature sensor device type test")

    # Temp sensor with temp sensor, descriptor
    def test_ts_missing_identify(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Descriptor.id])
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

    # endpoint 1 empty
    def test_endpoint_missing_descriptor(self):
        self.create_test([], no_descriptor=True)
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

    # Temp sensor with temp sensor, descriptor, identify, onoff
    def test_ts_extra_cluster(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.Descriptor.id, Clusters.OnOff.id])
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

        success, problems = self.test.check_device_type(fail_on_extra_clusters=False)
        asserts.assert_equal(len(problems), 1, "Did not receive expected warning for extra clusters")
        asserts.assert_true(success, "Unexpected failure")

    def test_bad_device_type_id_device_type_test(self):
        self.create_test([], bad_device_id=True)
        success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

    def test_all_device_types(self):
        for id in self.xml_device_types.keys():
            self.create_good_device(id)
            success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
            if problems:
                print(problems)
            asserts.assert_false(problems, f"Unexpected problems on device type {id}")
            asserts.assert_true(success, f"Unexpected failure on device type {id}")

    def test_disallowed_cluster(self):
        for id, dt in self.xml_device_types.items():
            expected_problems = 0
            self.create_good_device(id)
            for cluster_id, cluster in dt.server_clusters.items():
                if not conformance_allowed(cluster.conformance(0, [], []), False):
                    self.test.endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList].append(cluster_id)
                    expected_problems += 1
            if expected_problems == 0:
                continue
            success, problems = self.test.check_device_type(fail_on_extra_clusters=True)
            if problems:
                print(problems)
            asserts.assert_equal(len(problems), expected_problems, "Unexpected number of problems")
            asserts.assert_false(success, "Unexpected success running test that should fail")

    def test_spec_files(self):
        one_three, _ = build_xml_device_types(PrebuiltDataModelDirectory.k1_3)
        one_four, one_four_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4)
        one_four_one, one_four_one_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_1)
        tot, tot_problems = build_xml_device_types(PrebuiltDataModelDirectory.kMaster)
        # 1.3 has a couple of problems related to proxy clusters and a random file from the DM editor.
        # Some of these should be fixed with the move to alchemy. For now, let's just make sure 1.4
        # and the current pull don't introduce NEW problems.
        asserts.assert_equal(len(one_four_problems), 0, "Problems found when parsing 1.4 spec")
        asserts.assert_equal(len(one_four_one_problems), 0, "Problems found when parsing 1.4.1 spec")
        # TOT has a bunch of problems related to IDs being allocated for closures and TBR. These should all
        # mention ID-TBD as the id, so let's pull those out for now and make sure there are no UNKNOWN problems.
        filtered_tot_problems = [p for p in tot_problems if 'ID-TBD' not in p.problem]
        asserts.assert_equal(len(filtered_tot_problems), 0, "Problems found when parsing master spec")

        asserts.assert_greater(len(set(tot.keys()) - set(one_three.keys())),
                               0, "Master dir does not contain any device types not in 1.3")
        asserts.assert_greater(len(set(tot.keys()) - set(one_four.keys())),
                               0, "Master dir does not contain any device types not in 1.4")
        asserts.assert_greater(len(set(one_four.keys()) - set(one_three.keys())),
                               0, "1.4 dir does not contain any clusters not in 1.3")
        asserts.assert_equal(len(one_four.keys()), len(one_four_one.keys()),
                             "Number of device types in 1.4 and 1.4.1 does not match")
        asserts.assert_equal(set(one_three.keys()) - set(one_four.keys()),
                             set(), "There are some 1.3 device types that are unexpectedly not included in the 1.4 spec")
        asserts.assert_equal(set(one_four.keys())-set(tot.keys()),
                             set(), "There are some 1.4 device types that are unexpectedly not included in the TOT spec")
        asserts.assert_equal(set(one_three.keys())-set(tot.keys()),
                             set(), "There are some 1.3 device types that are unexpectedly not included in the TOT spec")


if __name__ == "__main__":
    default_matter_test_main()
