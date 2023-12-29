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

import glob
import logging
import os
import typing
import xml.etree.ElementTree as ElementTree
from copy import deepcopy
from dataclasses import dataclass
from enum import Enum, auto
from typing import Callable

import chip.clusters as Clusters
from chip.tlv import uint
from conformance_support import (DEPRECATE_CONFORM, DISALLOW_CONFORM, MANDATORY_CONFORM, OPTIONAL_CONFORM, OTHERWISE_CONFORM,
                                 PROVISIONAL_CONFORM, ConformanceDecision, ConformanceException, ConformanceParseParameters,
                                 feature, optional, or_operation, parse_callable_from_xml)
from matter_testing_support import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, EventPathLocation,
                                    FeaturePathLocation, ProblemNotice, ProblemSeverity)


@dataclass
class XmlFeature:
    code: str
    name: str
    conformance: Callable[[uint], ConformanceDecision]


@dataclass
class XmlAttribute:
    name: str
    datatype: str
    conformance: Callable[[uint], ConformanceDecision]


@dataclass
class XmlCommand:
    name: str
    conformance: Callable[[uint], ConformanceDecision]


@dataclass
class XmlEvent:
    name: str
    conformance: Callable[[uint], ConformanceDecision]


@dataclass
class XmlCluster:
    name: str
    revision: int
    derived: str
    feature_map: dict[str, uint]
    attribute_map: dict[str, uint]
    command_map: dict[str, uint]
    # mask to XmlFeature
    features: dict[uint, XmlFeature]
    # IDs to class
    attributes: dict[uint, XmlAttribute]
    accepted_commands: dict[uint, XmlCommand]
    generated_commands: dict[uint, XmlCommand]
    events: dict[uint, XmlEvent]


class CommandType(Enum):
    ACCEPTED = auto()
    GENERATED = auto()


class ClusterParser:
    def __init__(self, cluster, cluster_id, name):
        self._problems: list[ProblemNotice] = []
        self._cluster = cluster
        self._cluster_id = cluster_id
        self._name = name

        self._derived = None
        try:
            classification = next(cluster.iter('classification'))
            hierarchy = classification.attrib['hierarchy']
            if hierarchy.lower() == 'derived':
                self._derived = classification.attrib['baseCluster']
        except (KeyError, StopIteration):
            self._derived = None

        self.feature_elements = self.get_all_feature_elements()
        self.attribute_elements = self.get_all_attribute_elements()
        self.command_elements = self.get_all_command_elements()
        self.event_elements = self.get_all_event_elements()
        self.params = ConformanceParseParameters(feature_map=self.create_feature_map(), attribute_map=self.create_attribute_map(),
                                                 command_map=self.create_command_map())

    def get_conformance(self, element: ElementTree.Element) -> ElementTree.Element:
        for sub in element:
            if sub.tag == OTHERWISE_CONFORM or sub.tag == MANDATORY_CONFORM or sub.tag == OPTIONAL_CONFORM or sub.tag == PROVISIONAL_CONFORM or sub.tag == DEPRECATE_CONFORM or sub.tag == DISALLOW_CONFORM:
                return sub

        # Conformance is missing, so let's record the problem and treat it as optional for lack of a better choice
        if element.tag == 'feature':
            location = FeaturePathLocation(endpoint_id=0, cluster_id=self._cluster_id, feature_code=element.attrib['code'])
        elif element.tag == 'command':
            location = CommandPathLocation(endpoint_id=0, cluster_id=self._cluster_id, command_id=int(element.attrib['id'], 0))
        elif element.tag == 'attribute':
            location = AttributePathLocation(endpoint_id=0, cluster_id=self._cluster_id, attribute_id=int(element.attrib['id'], 0))
        elif element.tag == 'event':
            location = EventPathLocation(endpoint_id=0, cluster_id=self._cluster_id, event_id=int(element.attrib['id'], 0))
        else:
            location = ClusterPathLocation(endpoing_id=0, cluster_id=self._cluster_id)
        self._problems.append(ProblemNotice(test_name='Spec XML parsing', location=location,
                                            severity=ProblemSeverity.WARNING, problem='Unable to find conformance element'))

        return ElementTree.Element(OPTIONAL_CONFORM)

    def get_all_type(self, type_container: str, type_name: str, key_name: str) -> list[tuple[ElementTree.Element, ElementTree.Element]]:
        ret = []
        container_tags = self._cluster.iter(type_container)
        for container in container_tags:
            elements = container.iter(type_name)
            for element in elements:
                try:
                    element.attrib[key_name]
                except KeyError:
                    # This is a conformance tag, which uses the same name
                    continue
                conformance = self.get_conformance(element)
                ret.append((element, conformance))
        return ret

    def get_all_feature_elements(self) -> list[tuple[ElementTree.Element, ElementTree.Element]]:
        ''' Returns a list of features and their conformances'''
        return self.get_all_type('features', 'feature', 'code')

    def get_all_attribute_elements(self) -> list[tuple[ElementTree.Element, ElementTree.Element]]:
        ''' Returns a list of attributes and their conformances'''
        return self.get_all_type('attributes', 'attribute', 'id')

    def get_all_command_elements(self) -> list[tuple[ElementTree.Element, ElementTree.Element]]:
        ''' Returns a list of commands and their conformances '''
        return self.get_all_type('commands', 'command', 'id')

    def get_all_event_elements(self) -> list[tuple[ElementTree.Element, ElementTree.Element]]:
        ''' Returns a list of events and their conformances'''
        return self.get_all_type('events', 'event', 'id')

    def create_feature_map(self) -> dict[str, uint]:
        features = {}
        for element, conformance in self.feature_elements:
            features[element.attrib['code']] = 1 << int(element.attrib['bit'], 0)
        return features

    def create_attribute_map(self) -> dict[str, uint]:
        attributes = {}
        for element, conformance in self.attribute_elements:
            attributes[element.attrib['name']] = int(element.attrib['id'], 0)
        return attributes

    def create_command_map(self) -> dict[str, uint]:
        commands = {}
        for element, conformance in self.command_elements:
            commands[element.attrib['name']] = int(element.attrib['id'], 0)
        return commands

    def parse_conformance(self, conformance_xml: ElementTree.Element) -> Callable:
        try:
            return parse_callable_from_xml(conformance_xml, self.params)
        except ConformanceException as ex:
            # Just point to the general cluster, because something is mismatched, but it's not clear what
            location = ClusterPathLocation(endpoint_id=0, cluster_id=self._cluster_id)
            self._problems.append(ProblemNotice(test_name='Spec XML parsing', location=location,
                                                severity=ProblemSeverity.WARNING, problem=str(ex)))
            return None

    def parse_features(self) -> dict[uint, XmlFeature]:
        features = {}
        for element, conformance_xml in self.feature_elements:
            mask = 1 << int(element.attrib['bit'], 0)
            conformance = self.parse_conformance(conformance_xml)
            if conformance is None:
                continue
            features[mask] = XmlFeature(code=element.attrib['code'], name=element.attrib['name'],
                                        conformance=conformance)
        return features

    def parse_attributes(self) -> dict[uint, XmlAttribute]:
        attributes = {}
        for element, conformance_xml in self.attribute_elements:
            code = int(element.attrib['id'], 0)
            # Some deprecated attributes don't have their types included, for now, lets just fallback to UNKNOWN
            try:
                datatype = element.attrib['type']
            except KeyError:
                datatype = 'UNKNOWN'
            conformance = self.parse_conformance(conformance_xml)
            if conformance is None:
                continue
            if code in attributes:
                # This is one of those fun ones where two different rows have the same id and name, but differ in conformance and ranges
                # I don't have a good way to relate the ranges to the conformance, but they're both acceptable, so let's just or them.
                conformance = or_operation([conformance, attributes[code].conformance])
            attributes[code] = XmlAttribute(name=element.attrib['name'], datatype=datatype,
                                            conformance=conformance)
        return attributes

    def parse_commands(self, command_type: CommandType) -> dict[uint, XmlAttribute]:
        commands = {}
        for element, conformance_xml in self.command_elements:
            code = int(element.attrib['id'], 0)
            dir = CommandType.ACCEPTED
            try:
                if element.attrib['direction'].lower() == 'responsefromserver':
                    dir = CommandType.GENERATED
            except KeyError:
                pass
            if dir != command_type:
                continue
            code = int(element.attrib['id'], 0)
            conformance = self.parse_conformance(conformance_xml)
            if conformance is None:
                continue
            if code in commands:
                conformance = or_operation([conformance, commands[code].conformance])
            commands[code] = XmlCommand(name=element.attrib['name'], conformance=conformance)
        return commands

    def parse_events(self) -> dict[uint, XmlAttribute]:
        events = {}
        for element, conformance_xml in self.event_elements:
            code = int(element.attrib['id'], 0)
            conformance = self.parse_conformance(conformance_xml)
            if conformance is None:
                continue
            if code in events:
                conformance = or_operation([conformance, events[code].conformance])
            events[code] = XmlEvent(name=element.attrib['name'], conformance=conformance)
        return events

    def create_cluster(self) -> XmlCluster:
        try:
            revision = int(self._cluster.attrib['revision'], 0)
        except ValueError:
            revision = 0
        return XmlCluster(revision=revision, derived=self._derived,
                          name=self._name, feature_map=self.params.feature_map,
                          attribute_map=self.params.attribute_map, command_map=self.params.command_map,
                          features=self.parse_features(),
                          attributes=self.parse_attributes(),
                          accepted_commands=self.parse_commands(CommandType.ACCEPTED),
                          generated_commands=self.parse_commands(CommandType.GENERATED),
                          events=self.parse_events())

    def get_problems(self) -> list[ProblemNotice]:
        return self._problems


def build_xml_clusters() -> tuple[list[XmlCluster], list[ProblemNotice]]:
    dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'data_model', 'clusters')
    clusters: dict[int, XmlCluster] = {}
    derived_clusters: dict[str, XmlCluster] = {}
    ids_by_name = {}
    problems = []
    for xml in glob.glob(f"{dir}/*.xml"):
        logging.info(f'Parsing file {xml}')
        tree = ElementTree.parse(f'{xml}')
        root = tree.getroot()
        cluster = root.iter('cluster')
        for c in cluster:
            name = c.attrib['name']
            if not c.attrib['id']:
                # Fully derived clusters have no id, but also shouldn't appear on a device.
                # We do need to keep them, though, because we need to update the derived
                # clusters. We keep them in a special dict by name, so they can be thrown
                # away later.
                cluster_id = None
            else:
                cluster_id = int(c.attrib['id'], 0)
                ids_by_name[name] = cluster_id

            parser = ClusterParser(c, cluster_id, name)
            new = parser.create_cluster()
            problems = problems + parser.get_problems()

            if cluster_id:
                clusters[cluster_id] = new
            else:
                derived_clusters[name] = new

    # There are a few clusters where the conformance columns are listed as desc. These clusters need specific, targeted tests
    # to properly assess conformance. Here, we list them as Optional to allow these for the general test. Targeted tests are described below.
    # Descriptor - TagList feature - this feature is mandated when the duplicate condition holds for the endpoint. It is tested in DESC-2.2
    # Actions cluster - all commands - these need to be listed in the ActionsList attribute to be supported.
    #                                  We do not currently have a test for this. Please see https://github.com/CHIP-Specifications/chip-test-plans/issues/3646.
    def remove_problem(location: typing.Union[CommandPathLocation, FeaturePathLocation]):
        nonlocal problems
        problems = [p for p in problems if p.location != location]

    descriptor_id = Clusters.Descriptor.id
    code = 'TAGLIST'
    mask = clusters[descriptor_id].feature_map[code]
    clusters[descriptor_id].features[mask].conformance = optional()
    remove_problem(FeaturePathLocation(endpoint_id=0, cluster_id=descriptor_id, feature_code=code))
    action_id = Clusters.Actions.id
    for c in Clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[action_id]:
        clusters[action_id].accepted_commands[c].conformance = optional()
        remove_problem(CommandPathLocation(endpoint_id=0, cluster_id=action_id, command_id=c))

    # We have the information now about which clusters are derived, so we need to fix them up. Apply first the base cluster,
    # then add the specific cluster overtop
    for id, c in clusters.items():
        if c.derived:
            base_name = c.derived
            if base_name in ids_by_name:
                base = clusters[ids_by_name[c.derived]]
            else:
                base = derived_clusters[base_name]

            feature_map = deepcopy(base.feature_map)
            feature_map.update(c.feature_map)
            attribute_map = deepcopy(base.attribute_map)
            attribute_map.update(c.attribute_map)
            command_map = deepcopy(base.command_map)
            command_map.update(c.command_map)
            features = deepcopy(base.features)
            features.update(c.features)
            attributes = deepcopy(base.attributes)
            attributes.update(c.attributes)
            accepted_commands = deepcopy(base.accepted_commands)
            accepted_commands.update(c.accepted_commands)
            generated_commands = deepcopy(base.generated_commands)
            generated_commands.update(c.generated_commands)
            events = deepcopy(base.events)
            events.update(c.events)
            new = XmlCluster(revision=c.revision, derived=c.derived, name=c.name,
                             feature_map=feature_map, attribute_map=attribute_map, command_map=command_map,
                             features=features, attributes=attributes, accepted_commands=accepted_commands,
                             generated_commands=generated_commands, events=events)
            clusters[id] = new

    # workaround for aliased clusters not appearing in the xml. Remove this once https://github.com/csa-data-model/projects/issues/373 is addressed
    conc_clusters = {0x040C: 'Carbon Monoxide Concentration Measurement',
                     0x040D: 'Carbon Dioxide Concentration Measurement',
                     0x0413: 'Nitrogen Dioxide Concentration Measurement',
                     0x0415: 'Ozone Concentration Measurement',
                     0x042A: 'PM2.5 Concentration Measurement',
                     0x042B: 'Formaldehyde Concentration Measurement',
                     0x042C: 'PM1 Concentration Measurement',
                     0x042D: 'PM10 Concentration Measurement',
                     0x042E: 'Total Volatile Organic Compounds Concentration Measurement',
                     0x042F: 'Radon Concentration Measurement'}
    conc_base_name = 'Concentration Measurement Clusters'
    resource_clusters = {0x0071: 'HEPA Filter Monitoring',
                         0x0072: 'Activated Carbon Filter Monitoring'}
    resource_base_name = 'Resource Monitoring Clusters'
    water_clusters = {0x0405: 'Relative Humidity Measurement',
                      0x0407: 'Leaf Wetness Measurement',
                      0x0408: 'Soil Moisture Measurement'}
    water_base_name = 'Water Content Measurement Clusters'
    aliases = {conc_base_name: conc_clusters, resource_base_name: resource_clusters, water_base_name: water_clusters}
    for alias_base_name, aliased_clusters in aliases.items():
        for id, alias_name in aliased_clusters.items():
            base = derived_clusters[alias_base_name]
            new = deepcopy(base)
            new.derived = alias_base_name
            new.name = alias_name
            clusters[id] = new

    # Workaround for temp control cluster - this is parsed incorrectly in the DM XML and is missing all its attributes
    # Remove this workaround when https://github.com/csa-data-model/projects/issues/330 is fixed
    temp_control_id = Clusters.TemperatureControl.id
    if temp_control_id in clusters and not clusters[temp_control_id].attributes:
        clusters[temp_control_id].attributes = {
            0x00: XmlAttribute(name='TemperatureSetpoint', datatype='temperature', conformance=feature(0x01, 'TN')),
            0x01: XmlAttribute(name='MinTemperature', datatype='temperature', conformance=feature(0x01, 'TN')),
            0x02: XmlAttribute(name='MaxTemperature', datatype='temperature', conformance=feature(0x01, 'TN')),
            0x03: XmlAttribute(name='Step', datatype='temperature', conformance=feature(0x04, 'STEP')),
            0x04: XmlAttribute(name='SelectedTemperatureLevel', datatype='uint8', conformance=feature(0x02, 'TL')),
            0x05: XmlAttribute(name='SupportedTemperatureLevels', datatype='list', conformance=feature(0x02, 'TL')),
        }

    return clusters, problems
