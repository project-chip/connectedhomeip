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

from DeviceConformanceTests import DeviceConformanceTests, get_supersets
from fake_device_builder import create_minimal_dt
from jinja2 import Template
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.conformance import EMPTY_CLUSTER_GLOBAL_ATTRIBUTES, conformance_allowed
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import (PrebuiltDataModelDirectory, XmlDeviceType, build_xml_clusters, build_xml_device_types,
                                         parse_single_device_type)
from matter.tlv import uint


def run_against_all_spec_revisions(body):
    def runner(self: "DeviceConformanceTests", *args, **kwargs):
        for revision in PrebuiltDataModelDirectory:
            self._create_xmls(revision)
            body(self, *args, **kwargs)
    return runner


class TestSpecParsingDeviceType(DeviceConformanceTests):
    def _create_xmls(self, revision: PrebuiltDataModelDirectory):
        print(f"-------------- Testing against spec revision {revision.dirname}")
        self.xml_clusters, self.xml_cluster_problems = build_xml_clusters(revision)
        self.xml_device_types, self.xml_device_types_problems = build_xml_device_types(revision)
        # Let's just build a dictionary of device types to IDs because I need them and we don't have codegen
        self.dt_ids = {re.sub('[ -/]*', '', dt.name.lower()): id for id, dt in self.xml_device_types.items()}

    # This just tests that the prebuilt specs can be parsed without failures

    @run_against_all_spec_revisions
    def test_spec_device_parsing(self):
        for id, d in self.xml_device_types.items():
            print(str(d))

    def teardown_test(self):
        super().teardown_test()
        if self.problems:
            print(self.problems)

    def setup_class(self):
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
                                    {% if features and k in features.keys() %}
                                    <features>
                                    {% for fname in features[k] %}
                                    <feature name="{{ fname }}">
                                    <mandatoryConform/>
                                    </feature>
                                    {% endfor %}
                                    </features>
                                    {% endif %}
                                    {% if attributes and k in attributes.keys() %}
                                    <attributes>
                                    {% for name in attributes[k] %}
                                    <attribute name="{{ name }}">
                                    <mandatoryConform/>
                                    </attribute>
                                    {% endfor %}
                                    </attributes>
                                    {% endif %}
                                    {% if commands and k in commands.keys() %}
                                    <commands>
                                    {% for name in commands[k] %}
                                    <command name="{{ name }}">
                                    <mandatoryConform/>
                                    </command>
                                    {% endfor %}
                                    </commands>
                                    {% endif %}
                                    </cluster>
                                    {% endfor %}
                                    </clusters>
                                    </deviceType>""")
        # We're going to use the real cluster stuff so I don't need to write new XML. Device type uses Identify and Groups.
        super().setup_class()

    def test_device_type_clusters(self):
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(self.device_type_id in device_type, "device type id not found in returned data")
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
        device_type, problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing device type conformance")
        asserts.assert_equal(len(device_type.keys()), 1, "Unexpected number of device types returned")
        asserts.assert_true(self.device_type_id in device_type, "device type id not found in returned data")
        asserts.assert_equal(device_type[self.device_type_id].revision, self.revision, "Unexpected revision")
        asserts.assert_equal(len(device_type[self.device_type_id].server_clusters), len(clusters), "Unexpected number of clusters")

    def test_bad_device_type_id(self):
        xml = self.template.render(device_type_id="", revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_equal(len(problems), 1, "Device with blank ID did not generate a problem notice")

    def test_bad_class(self):
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class="",
                                   classification_scope=self.classification_scope, clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_equal(len(problems), 1, "Device with no class did not generate a problem notice")

    def test_bad_scope(self):
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope="", clusters=self.clusters)
        et = ElementTree.fromstring(xml)
        device_type, problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_equal(len(problems), 1, "Device with no scope did not generate a problem notice")

    def test_feature_overrides_good(self):
        # Groups cluster with GroupNames feature
        groups_id = Clusters.Groups.id
        group_name_mask = Clusters.Groups.Bitmaps.Feature.kGroupNames
        features = {groups_id: ['GroupNames']}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters, features=features)
        et = ElementTree.fromstring(xml)

        device_type, self.problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_in(groups_id, device_type[self.device_type_id].server_clusters,
                          "Groups Cluster not found in device type parse")
        asserts.assert_in(Clusters.Identify.id,
                          device_type[self.device_type_id].server_clusters, "Identify Cluster not found in device type parse")
        asserts.assert_in(
            group_name_mask, device_type[self.device_type_id].server_clusters[groups_id].feature_overrides, "GroupName override not found in groups cluster")
        asserts.assert_equal(str(
            device_type[self.device_type_id].server_clusters[groups_id].feature_overrides[group_name_mask]), "M", "Improper conformance override for GroupName feature")
        asserts.assert_equal(len(self.problems), 0, "Found problems when parsing good feature override")

    def test_feature_overrides_bad(self):
        # Identify cluster with GroupNames feature (does not exist on that cluster)
        groups_id = Clusters.Groups.id
        identify_id = Clusters.Identify.id
        group_name_mask = Clusters.Groups.Bitmaps.Feature.kGroupNames
        features = {identify_id: ['GroupNames']}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters, features=features)
        et = ElementTree.fromstring(xml)

        device_type, self.problems = parse_single_device_type(et, self.xml_clusters)
        # The two clusters should still appear there
        asserts.assert_in(groups_id, device_type[self.device_type_id].server_clusters,
                          "Groups Cluster not found in device type parse")
        asserts.assert_in(identify_id,
                          device_type[self.device_type_id].server_clusters, "Identify Cluster not found in device type parse")
        asserts.assert_not_in(
            group_name_mask, device_type[self.device_type_id].server_clusters[groups_id].feature_overrides, "GroupName override found in groups cluster")
        asserts.assert_not_in(
            group_name_mask, device_type[self.device_type_id].server_clusters[identify_id].feature_overrides, "GroupName override found in identify cluster")
        asserts.assert_equal(len(self.problems), 1, "Unexpected number of problems parsing bad feature conformance")
        # If we get here, the problems are as expected, don't need to report the problems on teardown
        self.problems = []

    def test_attribute_overrides_good(self):
        # Groups cluster with GroupNames feature
        groups_id = Clusters.Groups.id
        name_support_attr = Clusters.Groups.Attributes.NameSupport.attribute_id
        attributes = {groups_id: ['NameSupport']}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters, attributes=attributes)
        et = ElementTree.fromstring(xml)

        device_type, self.problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_in(groups_id, device_type[self.device_type_id].server_clusters,
                          "Groups Cluster not found in device type parse")
        asserts.assert_in(Clusters.Identify.id,
                          device_type[self.device_type_id].server_clusters, "Identify Cluster not found in device type parse")
        asserts.assert_in(
            name_support_attr, device_type[self.device_type_id].server_clusters[groups_id].attribute_overrides, "NameSupport override not found in groups cluster")
        asserts.assert_equal(str(
            device_type[self.device_type_id].server_clusters[groups_id].attribute_overrides[name_support_attr]), "M", "Improper conformance override")
        asserts.assert_equal(len(self.problems), 0, "Found problems when parsing good attribute override")

    def test_attribute_overrides_bad(self):
        # Identify cluster with NameSupport Attribute (does not exist on that cluster)
        groups_id = Clusters.Groups.id
        identify_id = Clusters.Identify.id
        name_support_attr = Clusters.Groups.Attributes.NameSupport.attribute_id
        attributes = {identify_id: ['NameSupport']}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters, attributes=attributes)
        et = ElementTree.fromstring(xml)

        device_type, self.problems = parse_single_device_type(et, self.xml_clusters)
        # The two clusters should still appear there
        asserts.assert_in(groups_id, device_type[self.device_type_id].server_clusters,
                          "Groups Cluster not found in device type parse")
        asserts.assert_in(identify_id,
                          device_type[self.device_type_id].server_clusters, "Identify Cluster not found in device type parse")
        asserts.assert_not_in(
            name_support_attr, device_type[self.device_type_id].server_clusters[groups_id].attribute_overrides, "NameSupport override found in groups cluster")
        asserts.assert_not_in(
            name_support_attr, device_type[self.device_type_id].server_clusters[identify_id].attribute_overrides, "NameSupport override not found in identify cluster")
        asserts.assert_equal(len(self.problems), 1, "Unexpected number of problems parsing bad attribute conformance")
        # If we get here, the problems are as expected, don't need to report the problems on teardown
        self.problems = []

    def test_command_overrides_good(self):
        # Groups cluster with GroupNames feature
        groups_id = Clusters.Groups.id
        add_group_cmd = Clusters.Groups.Commands.AddGroup.command_id
        commands = {groups_id: ['AddGroup']}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters, commands=commands)
        et = ElementTree.fromstring(xml)

        device_type, self.problems = parse_single_device_type(et, self.xml_clusters)
        asserts.assert_in(groups_id, device_type[self.device_type_id].server_clusters,
                          "Groups Cluster not found in device type parse")
        asserts.assert_in(Clusters.Identify.id,
                          device_type[self.device_type_id].server_clusters, "Identify Cluster not found in device type parse")
        asserts.assert_in(
            add_group_cmd, device_type[self.device_type_id].server_clusters[groups_id].command_overrides, "AddGroup override not found in groups cluster")
        asserts.assert_equal(str(
            device_type[self.device_type_id].server_clusters[groups_id].command_overrides[add_group_cmd]), "M", "Improper conformance override")
        asserts.assert_equal(len(self.problems), 0, "Found problems when parsing good attribute override")

    def test_command_overrides_bad(self):
        # Identify cluster with AddGroup Command (does not exist on that cluster)
        groups_id = Clusters.Groups.id
        identify_id = Clusters.Identify.id
        add_group_cmd = Clusters.Groups.Commands.AddGroup.command_id
        commands = {identify_id: ['AddGroup']}
        xml = self.template.render(device_type_id=self.device_type_id, revision=self.revision, classification_class=self.classification_class,
                                   classification_scope=self.classification_scope, clusters=self.clusters, commands=commands)
        et = ElementTree.fromstring(xml)

        device_type, self.problems = parse_single_device_type(et, self.xml_clusters)
        # The two clusters should still appear there
        asserts.assert_in(groups_id, device_type[self.device_type_id].server_clusters,
                          "Groups Cluster not found in device type parse")
        asserts.assert_in(identify_id,
                          device_type[self.device_type_id].server_clusters, "Identify Cluster not found in device type parse")
        asserts.assert_not_in(
            add_group_cmd, device_type[self.device_type_id].server_clusters[groups_id].command_overrides, "NameSupport override found in groups cluster")
        asserts.assert_not_in(
            add_group_cmd, device_type[self.device_type_id].server_clusters[identify_id].command_overrides, "NameSupport override not found in identify cluster")
        asserts.assert_equal(len(self.problems), 1, "Unexpected number of problems parsing bad command conformance")
        # If we get here, the problems are as expected, don't need to report the problems on teardown
        self.problems = []

    @run_against_all_spec_revisions
    def test_overrides_real(self):
        # Check that some known overrides are present
        # Onoff light has element requirements with conformance for commands and features, and attribute requirements for constraints, which should be ignored for now.
        on_off_light_device_type = 0x0100
        identify = Clusters.Identify.id
        trigger_effect = Clusters.Identify.Commands.TriggerEffect.command_id
        asserts.assert_in(trigger_effect, self.xml_device_types[on_off_light_device_type].server_clusters[identify].command_overrides.keys(
        ), "Did not find expected command override for TriggerEffect")
        asserts.assert_equal(str(self.xml_device_types[on_off_light_device_type].server_clusters[identify].command_overrides[trigger_effect]),
                             "M", "Unexpected command override for TriggerEffect")
        onoff = Clusters.OnOff.id
        lighting = Clusters.OnOff.Bitmaps.Feature.kLighting
        asserts.assert_in(lighting, self.xml_device_types[on_off_light_device_type].server_clusters[onoff].feature_overrides.keys(
        ), "Did not find expected feature override for Lighting")
        asserts.assert_equal(str(self.xml_device_types[on_off_light_device_type].server_clusters[onoff].feature_overrides[lighting]),
                             "M", "Unexpected feature override for Lighting")
        asserts.assert_equal(len(self.xml_device_types[on_off_light_device_type].server_clusters[onoff]
                             .attribute_overrides), 0, "Unexpected attribute override in on off device type")

        # Color temperature light DOES have an attribute override
        color_temp_device_type = 0x010C
        color_control = Clusters.ColorControl.id
        remaining_time = Clusters.ColorControl.Attributes.RemainingTime.attribute_id
        asserts.assert_in(remaining_time, self.xml_device_types[color_temp_device_type].server_clusters[color_control].attribute_overrides.keys(
        ), "Did not find expected attribute override for RemainingTime")
        asserts.assert_equal(str(self.xml_device_types[color_temp_device_type].server_clusters[color_control].attribute_overrides[remaining_time]),
                             "M", "Unexpected attribute override for RemainingTime")

    # All these tests are based on the temp sensor device type because it is very simple
    # it requires temperature measurement, identify and the base devices.
    # Right now I'm not testing for binding condition.
    # The test is entirely based on the descriptor cluster so that's all I'm populating here
    # because it makes the test less complex to write.
    def create_test(self, server_list: list[uint], no_descriptor: bool = False, bad_device_id: bool = False):
        # Start with a good device, then add or remove clusters as necessary
        if no_descriptor:
            self.endpoints = {1: {}}
            self.endpoints_tlv = {1: {}}
        else:
            # build the temperature sensor device type
            attrs = create_minimal_dt(xml_device_types=self.xml_device_types, xml_clusters=self.xml_clusters,
                                      device_type_id=0x302, is_tlv_endpoint=False, server_override=server_list)
            attrs_tlv = create_minimal_dt(
                xml_device_types=self.xml_device_types, xml_clusters=self.xml_clusters, device_type_id=0x302, is_tlv_endpoint=True, server_override=server_list)
            # override with the desired device type id
            if bad_device_id:
                known_ids = list(self.xml_device_types.keys())
                device_type_id = [a for a in range(min(known_ids), max(known_ids)) if a not in known_ids][0]
                device_type_list = [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=device_type_id, revision=2)]
                attrs[Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList] = device_type_list
                # note - not populating the TLV for this since it's not used.
            self.endpoints = {1: attrs}
            self.endpoints_tlv = {1: attrs_tlv}

    def create_good_device(self, device_type_id: int):
        endpoint = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id, is_tlv_endpoint=False)
        endpoint_tlv = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id, is_tlv_endpoint=True)

        self.endpoints = {1: endpoint}
        self.endpoints_tlv = {1: endpoint_tlv}

    # Test with temp sensor with temp sensor, identify and descriptor
    @run_against_all_spec_revisions
    def test_ts_minimal_clusters(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.Descriptor.id])
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_true(success, "Failure on Temperature Sensor device type test")

    # Temp sensor with temp sensor, identify, descriptor, binding
    @run_against_all_spec_revisions
    def test_ts_minimal_with_binding(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.Binding.id, Clusters.Descriptor.id])
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_true(success, "Failure on Temperature Sensor device type test")
        asserts.assert_false(problems, "Found problems on Temperature sensor device type test")

    # Temp sensor with temp sensor, identify, descriptor, fixed label
    @run_against_all_spec_revisions
    def test_ts_minimal_with_label(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.FixedLabel.id, Clusters.Descriptor.id])
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_true(success, "Failure on Temperature Sensor device type test")
        asserts.assert_false(problems, "Found problems on Temperature sensor device type test")

    # Temp sensor with temp sensor, descriptor
    @run_against_all_spec_revisions
    def test_ts_missing_identify(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Descriptor.id])
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

    # endpoint 1 empty
    @run_against_all_spec_revisions
    def test_endpoint_missing_descriptor(self):
        self.create_test([], no_descriptor=True)
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

    # Temp sensor with temp sensor, descriptor, identify, onoff
    @run_against_all_spec_revisions
    def test_ts_extra_cluster(self):
        self.create_test([Clusters.TemperatureMeasurement.id, Clusters.Identify.id, Clusters.Descriptor.id, Clusters.OnOff.id])
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

        success, problems = self.check_device_type(fail_on_extra_clusters=False)
        asserts.assert_equal(len(problems), 1, "Did not receive expected warning for extra clusters")
        asserts.assert_true(success, "Unexpected failure")

    @run_against_all_spec_revisions
    def test_bad_device_type_id_device_type_test(self):
        self.create_test([], bad_device_id=True)
        success, problems = self.check_device_type(fail_on_extra_clusters=True)
        if problems:
            print(problems)
        asserts.assert_equal(len(problems), 1, "Unexpected number of problems")
        asserts.assert_false(success, "Unexpected success running test that should fail")

    @run_against_all_spec_revisions
    def test_all_device_types(self):
        for id in self.xml_device_types:
            self.create_good_device(id)
            success, problems = self.check_device_type(fail_on_extra_clusters=True)
            for p in problems:
                print(p)
            asserts.assert_false(problems, f"Unexpected problems on device type {id}")
            asserts.assert_true(success, f"Unexpected failure on device type {id}")

    @run_against_all_spec_revisions
    def test_disallowed_cluster(self):
        for id, dt in self.xml_device_types.items():
            expected_problems = 0
            self.create_good_device(id)
            for cluster_id, cluster in dt.server_clusters.items():
                if not conformance_allowed(cluster.conformance(EMPTY_CLUSTER_GLOBAL_ATTRIBUTES), False):
                    self.endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList].append(cluster_id)
                    expected_problems += 1
            if expected_problems == 0:
                continue
            success, problems = self.check_device_type(fail_on_extra_clusters=True)
            for p in problems:
                print(p)
            asserts.assert_equal(len(problems), expected_problems, "Unexpected number of problems")
            asserts.assert_false(success, "Unexpected success running test that should fail")

    def test_spec_files(self):
        one_two, one_two_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_2)
        one_three, one_three_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_3)
        one_four, one_four_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4)
        one_four_one, one_four_one_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_1)
        one_four_two, one_four_two_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4_2)
        one_five, one_five_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_5)
        one_five_one, one_five_one_problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_5_1)
        self.problems.extend(one_two_problems)
        self.problems.extend(one_three_problems)
        self.problems.extend(one_four_problems)
        self.problems.extend(one_four_one_problems)
        self.problems.extend(one_four_two_problems)
        self.problems.extend(one_five_problems)
        self.problems.extend(one_five_one_problems)

        asserts.assert_equal(len(one_two_problems), 0, "Problems found when parsing 1.2 spec")
        asserts.assert_equal(len(one_three_problems), 0, "Problems found when parsing 1.3 spec")
        asserts.assert_equal(len(one_four_problems), 0, "Problems found when parsing 1.4 spec")
        asserts.assert_equal(len(one_four_one_problems), 0, "Problems found when parsing 1.4.1 spec")
        asserts.assert_equal(len(one_four_two_problems), 0, "Problems found when parsing 1.4.2 spec")
        asserts.assert_equal(len(one_five_problems), 0, "Problems found when parsing 1.5 spec")
        asserts.assert_equal(len(one_five_one_problems), 0, "Problems found when parsing 1.5.1 spec")

        # Current ballot has a bunch of problems related to IDs being allocated for closures and TBR. These should all
        # mention ID-TBD as the id, so let's pull those out for now and make sure there are no UNKNOWN problems.
        filtered_ballot_problems = [p for p in one_four_two_problems if 'ID-TBD' not in p.problem]
        asserts.assert_equal(len(filtered_ballot_problems), 0, "Problems found when parsing master spec")

        asserts.assert_greater(len(set(one_five.keys()) - set(one_four_two.keys())), 0,
                               '1.5 does not contain any device types not in 1.4.2')
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_three.keys())),
                               0, "Master dir does not contain any device types not in 1.3")
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_four.keys())),
                               0, "Master dir does not contain any device types not in 1.4")
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_three.keys())),
                               0, "1.4 dir does not contain any device types not in 1.3")
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_two.keys())),
                               0, "Master does not contain any device types not in 1.2")

        # Heating/cooling unit was provisional, then removed in 1.3
        # https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/5541
        asserts.assert_equal(set(one_two.keys() - set(one_three.keys())),
                             {0x0300}, "1.2 contains unexpected device types not in 1.3")
        asserts.assert_equal(len(one_four.keys()), len(one_four_one.keys()),
                             "Number of device types in 1.4 and 1.4.1 does not match")
        asserts.assert_equal(set(one_three.keys()) - set(one_four.keys()),
                             set(), "There are some 1.3 device types that are unexpectedly not included in the 1.4 spec")
        asserts.assert_equal(set(one_four.keys())-set(one_four_two.keys()),
                             set(), "There are some 1.4 device types that are unexpectedly not included in the 1.4.2 spec")
        asserts.assert_equal(set(one_three.keys())-set(one_four_two.keys()),
                             set(), "There are some 1.3 device types that are unexpectedly not included in the 1.4.2 spec")
        asserts.assert_equal(set(one_four_two.keys())-set(one_five.keys()),
                             set(), "There are some 1.4.2 device types that are unexpectedly not included in the 1.5 spec")
        asserts.assert_equal(set(one_five.keys())-set(one_five_one.keys()),
                             set(), "There are some 1.5 device types that are unexpectedly not included in the 1.5.1 spec")

    @run_against_all_spec_revisions
    def test_application_device_type_on_root(self):
        desc = Clusters.Descriptor

        root_dt = self.dt_ids['rootnode']
        lock_dt = self.dt_ids['doorlock']
        power_dt = self.dt_ids['powersource']

        # Root only, should be fine
        dt_list = [desc.Structs.DeviceTypeStruct(root_dt, 1)]
        attrs = {desc: {desc.Attributes.DeviceTypeList: dt_list}}
        self.endpoints = {0: attrs}

        problems = self.check_root_endpoint_for_application_device_types()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found on root note endpoint")

        # Utility device type - should be fine
        self.endpoints[0][desc][desc.Attributes.DeviceTypeList].append(desc.Structs.DeviceTypeStruct(power_dt, 1))
        problems = self.check_root_endpoint_for_application_device_types()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found on root note endpoint")

        # Application device type - not fine
        self.endpoints[0][desc][desc.Attributes.DeviceTypeList].append(desc.Structs.DeviceTypeStruct(lock_dt, 1))
        problems = self.check_root_endpoint_for_application_device_types()
        asserts.assert_equal(len(problems), 1, "Did not get expected problem on root node with application device type")

    @run_against_all_spec_revisions
    def test_superset_parsing_released_spec(self):
        # Testing superset parsing from the spec since it requires multiple device types to be parsed together
        for d in self.xml_device_types.values():
            if d.superset_of_device_type_name:
                print(f'{d.name}: {d.superset_of_device_type_name} {d.superset_of_device_type_id}')
        supersets = get_supersets(self.xml_device_types)

        expected_light_superset = {self.dt_ids['extendedcolorlight'],
                                   self.dt_ids['colortemperaturelight'], self.dt_ids['dimmablelight'], self.dt_ids['onofflight']}
        expected_dimmer_switch = {self.dt_ids['colordimmerswitch'],
                                  self.dt_ids['dimmerswitch'], self.dt_ids['onofflightswitch']}

        asserts.assert_in(expected_light_superset, supersets, "Did not find expected light superset")
        asserts.assert_in(expected_dimmer_switch, supersets, "Did not find expected switch superset")

    def test_superset_parsing_mounted(self):
        # 1.4.2 has some interesting stuff where we have equivalent device types, so check that explicitly
        self._create_xmls(PrebuiltDataModelDirectory.k1_4_2)
        supersets = get_supersets(self.xml_device_types)

        expected_onoff_plugin = {self.dt_ids['mountedonoffcontrol'], self.dt_ids['onoffpluginunit']}
        expected_dimmable_plugin = {self.dt_ids['mounteddimmableloadcontrol'], self.dt_ids['dimmablepluginunit']}
        expected_light_superset = {self.dt_ids['extendedcolorlight'],
                                   self.dt_ids['colortemperaturelight'], self.dt_ids['dimmablelight'], self.dt_ids['onofflight']}
        expected_dimmer_switch = {self.dt_ids['colordimmerswitch'],
                                  self.dt_ids['dimmerswitch'], self.dt_ids['onofflightswitch']}

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
        asserts.assert_in({1, 2, 3}, supersets, "Did not find expected superset in tree")
        asserts.assert_in({4, 3}, supersets, "Did not find expected superset in tree")

    @run_against_all_spec_revisions
    def test_supersets_on_endpoints_mocks(self):
        superset_tree = self._build_superset_tree()
        self.xml_device_types = superset_tree

        desc = Clusters.Descriptor
        attrs = {desc: {desc.Attributes.DeviceTypeList: []}}
        self.endpoints = {1: attrs}

        # Non-superset application endpoint
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(5, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "unexpected problem found on endpoint with one non-superset application endpoint")

        # utility endpoint only
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(6, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with only utility endpoint")

        # 1, 2, and 3 (all one subset) - should be fine
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset")

        # 1 and 3 (skips middle)
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(1, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset (skips middle)")

        # 2 and 3 (skips top)
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset (skips highest)")

        # 1 and 2 (skips bottom)
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(1, 1), desc.Structs.DeviceTypeStruct(2, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset (skips lowest)")

        # 1, 2 and 3 (all one subsets) with utility
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(3, 1), desc.Structs.DeviceTypeStruct(6, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types all subset with utility")

        # non-subset with utility - 5 and 6
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [
            desc.Structs.DeviceTypeStruct(5, 1), desc.Structs.DeviceTypeStruct(6, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problem found on endpoint with device types non-subset with utility")

        # subset with non-subset - 2, 3, 5 - should fail
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            2, 1), desc.Structs.DeviceTypeStruct(3, 1), desc.Structs.DeviceTypeStruct(5, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find expected problem when testing with subset and non-subset devices")

        # mixed subsets without connector - 1, 2 and 4 - these are all supersets of 3, but shouldn't be allowed on the same endpoint
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(4, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find expected problem when testing with mixed subsets")

        # mixed subsets with connector - 1, 2, 3 and 4 - these are all supersets of 3, but shouldn't be allowed on the same endpoint
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [desc.Structs.DeviceTypeStruct(
            1, 1), desc.Structs.DeviceTypeStruct(2, 1), desc.Structs.DeviceTypeStruct(4, 1), desc.Structs.DeviceTypeStruct(3, 1)]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find expected problem when testing with mixed subsets")

    @run_against_all_spec_revisions
    def test_supersets_on_endpoints_spec_ids(self):

        # create mock devices using the real spec IDs
        desc = Clusters.Descriptor
        attrs0 = {desc: {desc.Attributes.DeviceTypeList: [desc.Structs.DeviceTypeStruct(self.dt_ids['rootnode'], 1)]}}
        attrs1 = {desc: {desc.Attributes.DeviceTypeList: []}}
        attrs2 = {desc: {desc.Attributes.DeviceTypeList: []}}
        self.endpoints = {0: attrs0, 1: attrs1, 2: attrs2}

        door_lock = desc.Structs.DeviceTypeStruct(self.dt_ids['doorlock'], 1)
        light_sensor = desc.Structs.DeviceTypeStruct(self.dt_ids['lightsensor'], 1)
        power_source = desc.Structs.DeviceTypeStruct(self.dt_ids['powersource'], 1)
        onoff_light = desc.Structs.DeviceTypeStruct(self.dt_ids['onofflight'], 1)
        dimmable_light = desc.Structs.DeviceTypeStruct(self.dt_ids['dimmablelight'], 1)

        # two of the same simple application device types on different endpoints
        self.endpoints[1][desc][desc.Attributes.DeviceTypeList] = [door_lock]
        self.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [door_lock]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found with two endpoints with the same device type")

        # two different simple application device types on different endpoints
        self.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [light_sensor]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(
            len(problems), 0, "Unexpected problems found with two endpoints with the different non-subset device types")

        # two subset application device types on one endpoint
        self.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [onoff_light, dimmable_light]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found with endpoint with permitted subsets")

        # two subset application device types on one endpoint with an application device type
        self.endpoints[2][desc][desc.Attributes.DeviceTypeList] = [onoff_light, dimmable_light, power_source]
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 0, "Unexpected problems found with endpoint with permitted subsets and utility")

        # bad - two non-subset device types on one endpoint
        self.endpoints[2][desc][desc.Attributes.DeviceTypeList].append(door_lock)
        problems = self.check_all_application_device_types_superset()
        asserts.assert_equal(len(problems), 1, "Did not find problem with non-permitted application device types")


if __name__ == "__main__":
    default_matter_test_main()
