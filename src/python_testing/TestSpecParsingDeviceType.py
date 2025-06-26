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
import re
import xml.etree.ElementTree as ElementTree

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.conformance import conformance_allowed
from chip.testing.matter_testing import MatterBaseTest, default_matter_test_main
from chip.testing.spec_parsing import (PrebuiltDataModelDirectory, XmlDeviceType, build_xml_clusters, build_xml_device_types,
                                       parse_single_device_type)
from chip.tlv import uint
from fake_device_builder import create_minimal_dt
from jinja2 import Template
from mobly import asserts
from TC_DeviceConformance import DeviceConformanceTests, get_supersets


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

        # Let's just build a dictionary of device types to IDs becasue I need them and we don't have codegen
        self.dt_ids = {re.sub('[ -/]*', '', dt.name.lower()): id for id, dt in self.xml_device_types.items()}

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

        endpoint = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id, is_tlv_endpoint=False)
        endpoint_tlv = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id, is_tlv_endpoint=True)

        self.test.endpoints = {1: endpoint}
        self.test.endpoints_tlv = {1: endpoint_tlv}

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
        one_three, one_three_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_3)
        one_four, one_four_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4)
        one_four_one, one_four_one_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_1)
        one_four_two, one_four_two_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_2)

        asserts.assert_equal(len(one_three_problems), 0, "Problems found when parsing 1.3 spec")
        asserts.assert_equal(len(one_four_problems), 0, "Problems found when parsing 1.4 spec")
        asserts.assert_equal(len(one_four_one_problems), 0, "Problems found when parsing 1.4.1 spec")

        # Current ballot has a bunch of problems related to IDs being allocated for closures and TBR. These should all
        # mention ID-TBD as the id, so let's pull those out for now and make sure there are no UNKNOWN problems.
        filtered_ballot_problems = [p for p in one_four_two_problems if 'ID-TBD' not in p.problem]
        asserts.assert_equal(len(filtered_ballot_problems), 0, "Problems found when parsing master spec")

        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_three.keys())),
                               0, "Master dir does not contain any device types not in 1.3")
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_four.keys())),
                               0, "Master dir does not contain any device types not in 1.4")
        asserts.assert_greater(len(set(one_four.keys()) - set(one_three.keys())),
                               0, "1.4 dir does not contain any clusters not in 1.3")
        asserts.assert_equal(len(one_four.keys()), len(one_four_one.keys()),
                             "Number of device types in 1.4 and 1.4.1 does not match")
        asserts.assert_equal(set(one_three.keys()) - set(one_four.keys()),
                             set(), "There are some 1.3 device types that are unexpectedly not included in the 1.4 spec")
        asserts.assert_equal(set(one_four.keys())-set(one_four_two.keys()),
                             set(), "There are some 1.4 device types that are unexpectedly not included in the 1.4.2 spec")
        asserts.assert_equal(set(one_three.keys())-set(one_four_two.keys()),
                             set(), "There are some 1.3 device types that are unexpectedly not included in the 1.4.2 spec")

    def test_application_device_type_on_root(self):
        self.test = DeviceConformanceTests()
        self.test.xml_device_types = self.xml_device_types
        self.test.xml_clusters = self.xml_clusters

        desc = Clusters.Descriptor

        root_dt = self.dt_ids['rootnode']
        lock_dt = self.dt_ids['doorlock']
        power_dt = self.dt_ids['powersource']

        # Root only, should be fine
        dt_list = [desc.Structs.DeviceTypeStruct(root_dt, 1)]
        attrs = {desc: {desc.Attributes.DeviceTypeList: dt_list}}
        self.test.endpoints = {0: attrs}

        problems = self.test.check_root_endpoint_for_application_device_types()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found on root note endpoint")

        # Utility device type - should be fine
        self.test.endpoints[0][desc][desc.Attributes.DeviceTypeList].append(desc.Structs.DeviceTypeStruct(power_dt, 1))
        problems = self.test.check_root_endpoint_for_application_device_types()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found on root note endpoint")

        # Application device type - not fine
        self.test.endpoints[0][desc][desc.Attributes.DeviceTypeList].append(desc.Structs.DeviceTypeStruct(lock_dt, 1))
        problems = self.test.check_root_endpoint_for_application_device_types()
        asserts.assert_equal(len(problems), 1, "Did not get expected problem on root node with application device type")

    def test_superset_parsing_released_spec(self):
        # Testing superset parsing from the spec since it requires multiple device types to be parsed together
        for d in self.xml_device_types.values():
            if d.superset_of_device_type_name:
                print(f'{d.name}: {d.superset_of_device_type_name} {d.superset_of_device_type_id}')
        supersets = get_supersets(self.xml_device_types)

        expected_light_superset = set([self.dt_ids['extendedcolorlight'],
                                       self.dt_ids['colortemperaturelight'], self.dt_ids['dimmablelight'], self.dt_ids['onofflight']])
        expected_dimmer_switch = set([self.dt_ids['colordimmerswitch'],
                                     self.dt_ids['dimmerswitch'], self.dt_ids['onofflightswitch']])

        asserts.assert_in(expected_light_superset, supersets, "Did not find expected light superset")
        asserts.assert_in(expected_dimmer_switch, supersets, "Did not find expected switch superset")

        # 1.4.2 has some interesting stuff where we have equivalent device types, so check that explicitly
        xml_device_types_1_4_2, _ = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_2)
        supersets = get_supersets(xml_device_types_1_4_2)

        expected_onoff_plugin = set([self.dt_ids['mountedonoffcontrol'], self.dt_ids['onoffpluginunit']])
        expected_dimmable_plugin = set([self.dt_ids['mounteddimmableloadcontrol'], self.dt_ids['dimmablepluginunit']])

        asserts.assert_in(expected_light_superset, supersets, "Did not find expected light superset")
        asserts.assert_in(expected_dimmer_switch, supersets, "Did not find expected switch superset")
        asserts.assert_in(expected_onoff_plugin, supersets, "Did not find expected mounted on off superset")
        asserts.assert_in(expected_dimmable_plugin, supersets, "Did not find expected mounted dimmable superset")

    def _build_superset_tree(self) -> dict[int, XmlDeviceType]:
        # Builds the tree as follows
        # 1 -> 2 -> 3
        # 4 -> 3
        # 5 - all alone
        # 6 - utility endpoint
        one = XmlDeviceType('one', 1, [], [], 'simple', 'endpoint',
                            superset_of_device_type_name='two', superset_of_device_type_id=2)
        two = XmlDeviceType('two', 1, [], [], 'simple', 'endpoint',
                            superset_of_device_type_name='three', superset_of_device_type_id=3)
        three = XmlDeviceType('three', 1, [], [], 'simple', 'endpoint',
                              superset_of_device_type_name=None, superset_of_device_type_id=0)
        four = XmlDeviceType('four', 1, [], [], 'simple', 'endpoint',
                             superset_of_device_type_name='three', superset_of_device_type_id=3)
        five = XmlDeviceType('five', 1, [], [], 'simple', 'endpoint',
                             superset_of_device_type_name=None, superset_of_device_type_id=0)
        six = XmlDeviceType('six', 1, [], [], 'utility', 'endpoint',
                            superset_of_device_type_name=None, superset_of_device_type_id=0)
        return {1: one, 2: two, 3: three, 4: four, 5: five, 6: six}

    def test_superset_parsing_mocks(self):
        # We could in theory have superset trees. We don't currently, but let's be future proof.
        superset_tree = self._build_superset_tree()
        supersets = get_supersets(superset_tree)
        # We should see separate lines
        asserts.assert_in(set([1, 2, 3]), supersets, "Did not find expected superset in tree")
        asserts.assert_in(set([4, 3]), supersets, "Did not find expected superset in tree")

    def test_supersets_on_endpoints_mocks(self):
        superset_tree = self._build_superset_tree()

        self.test = DeviceConformanceTests()
        self.test.xml_device_types = superset_tree

        desc = Clusters.Descriptor
        attrs = {desc: {desc.Attributes.DeviceTypeList: []}}
        self.test.endpoints = {1: attrs}

        # Non-superset application endpoint
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(5, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "unexpected problem found on endpoint with one non-superset application endpoint")

        # utility endpoint only
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(6, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with only utility endpoint")

        # 1, 2, and 3 (all one subset) - should be fine
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset")

        # 1 and 3 (skips middle)
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(1, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset (skips middle)")

        # 2 and 3 (skips top)
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset (skips highest)")

        # 1 and 2 (skips bottom)
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(1, 1), desc.Structs.DeviceTypeStruct(2, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset (skips lowest)")

        # 1, 2 and 3 (all one subsets) with utility
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(3, 1), desc.Structs.DeviceTypeStruct(6, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset with utility")

        # non-subset with utility - 5 and 6
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(5, 1), desc.Structs.DeviceTypeStruct(6, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types non-subset with utility")

        # subset with non-subset - 2, 3, 5 - should fail
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            2, 1), desc.Structs.DeviceTypeStruct(3, 1), desc.Structs.DeviceTypeStruct(5, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find expected problem when testing with subset and non-subset devices")

        # mixed subsets without connector - 1, 2 and 4 - these are all supersets of 3, but shouldn't be allowed on the same endpoint
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(4, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find expected problem when testing with mixed subsets")

        # mixed subsets with connector - 1, 2, 3 and 4 - these are all supersets of 3, but shouldn't be allowed on the same endpoint
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(4, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find expected problem when testing with mixed subsets")

    def test_supersets_on_endpoints_spec_ids(self):
        self.test = DeviceConformanceTests()
        self.test.xml_device_types = self.xml_device_types

        # create mock devices using the real spec IDs
        desc = Clusters.Descriptor
        attrs0 = {desc: {desc.Attributes.DeviceTypeList: [desc.Structs.DeviceTypeStruct(self.dt_ids['rootnode'], 1)]}}
        attrs1 = {desc: {desc.Attributes.DeviceTypeList: []}}
        attrs2 = {desc: {desc.Attributes.DeviceTypeList: []}}
        self.test.endpoints = {0: attrs0, 1: attrs1, 2: attrs2}

        door_lock = desc.Structs.DeviceTypeStruct(self.dt_ids['doorlock'], 1)
        light_sensor = desc.Structs.DeviceTypeStruct(self.dt_ids['lightsensor'], 1)
        power_source = desc.Structs.DeviceTypeStruct(self.dt_ids['powersource'], 1)
        onoff_light = desc.Structs.DeviceTypeStruct(self.dt_ids['onofflight'], 1)
        dimmable_light = desc.Structs.DeviceTypeStruct(self.dt_ids['dimmablelight'], 1)

        # two of the same simple application device types on different endpoints
        self.test.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [door_lock]
        self.test.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [door_lock]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found with two endpoints with the same device type")

        # two different simple application device types on different endpoints
        self.test.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [light_sensor]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(
            len(problems), 0, "Unexpected problems found with two endpoints with the different non-subset device types")

        # two subset application device types on one endpoint
        self.test.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [onoff_light, dimmable_light]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found with endpoint with permitted subsets")

        # two subset application device types on one endpoint with an application device type
        self.test.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [onoff_light, dimmable_light, power_source]
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found with endpoint with permitted subsets and utility")

        # bad - two non-subset device types on one endpoint
        self.test.endpoints[2][desc][desc.Attributes.DeviceTypeList].append(door_lock)
        problems = self.test.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find problem with non-permitted application device types")


if __name__ == "__main__":
    default_matter_test_main()
