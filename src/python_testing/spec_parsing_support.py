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
from typing import Callable, Optional

import chip.clusters as Clusters
import conformance_support
from chip.tlv import uint
from conformance_support import (OPTIONAL_CONFORM, TOP_LEVEL_CONFORMANCE_TAGS, ConformanceDecision, ConformanceException,
                                 ConformanceParseParameters, feature, is_disallowed, mandatory, optional, or_operation,
                                 parse_callable_from_xml, parse_device_type_callable_from_xml)
from global_attribute_ids import GlobalAttributeIds
from matter_testing_support import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, DeviceTypePathLocation,
                                    EventPathLocation, FeaturePathLocation, ProblemNotice, ProblemSeverity)

_PRIVILEGE_STR = {
    None: "N/A",
    Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView: "V",
    Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate: "O",
    Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage: "M",
    Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister: "A",
}


def to_access_code(privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum) -> str:
    return _PRIVILEGE_STR.get(privilege, "")


class SpecParsingException(Exception):
    pass


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
    read_access: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum
    write_access: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum
    write_optional: bool

    def access_string(self):
        read_marker = "R" if self.read_access is not Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue else ""
        write_marker = "W" if self.write_access is not Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue else ""
        read_access_marker = f'{to_access_code(self.read_access)}'
        write_access_marker = f'{to_access_code(self.write_access)}'
        return f'{read_marker}{write_marker} {read_access_marker}{write_access_marker}'

    def __str__(self):
        return f'{self.name}: datatype: {self.datatype} conformance: {str(self.conformance)}, access = {self.access_string()}'


@dataclass
class XmlCommand:
    id: int
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
    unknown_commands: list[XmlCommand]
    events: dict[uint, XmlEvent]
    pics: str
    is_provisional: bool


class ClusterSide(Enum):
    SERVER = auto()
    CLIENT = auto()


@dataclass
class XmlDeviceTypeClusterRequirements:
    name: str
    side: ClusterSide
    conformance: Callable[[uint, list[uint], list[uint]], ConformanceDecision]
    # TODO: add element requirements

    def __str__(self):
        return f'{self.name}: {str(self.conformance)}'


@dataclass
class XmlDeviceType:
    name: str
    revision: int
    server_clusters: dict[uint, XmlDeviceTypeClusterRequirements]
    client_clusters: dict[uint, XmlDeviceTypeClusterRequirements]
    # Keeping these as strings for now because the exact definitions are being discussed in DMTT
    classification_class: str
    classification_scope: str

    def __str__(self):
        msg = f'{self.name} - Revision {self.revision}, Class {self.classification_class}, Scope {self.classification_scope}\n'
        msg += '    Server clusters\n'
        for id, c in self.server_clusters.items():
            msg = msg + f'      {id}: {str(c)}\n'
        msg += '    Client clusters\n'
        for id, c in self.client_clusters.items():
            msg = msg + f'      {id}: {str(c)}\n'
        return msg


class CommandType(Enum):
    ACCEPTED = auto()
    GENERATED = auto()
    # This will happen for derived clusters, where the direction isn't noted. On normal clusters, this is a problem.
    UNKNOWN = auto()


# workaround for aliased clusters PICS not appearing in the xml. Remove this once https://github.com/csa-data-model/projects/issues/461 is addressed
ALIAS_PICS = {0x040C: 'CMOCONC',
              0x040D: 'CDOCONC',
              0x0413: 'NDOCONC',
              0x0415: 'OZCONC',
              0x042A: 'PMICONC',
              0x042B: 'FLDCONC',
              0x042C: 'PMHCONC',
              0x042D: 'PMKCONC',
              0x042E: 'TVOCCONC',
              0x042F: 'RNCONC',
              0x0071: 'HEPAFREMON',
              0x0072: 'ACFREMON',
              0x0405: 'RH',
              0x001C: 'PWM'}

CLUSTER_NAME_FIXES = {0x0036: 'WiFi Network Diagnostics', 0x042a: 'PM25 Concentration Measurement', 0x0006: 'On/Off'}
DEVICE_TYPE_NAME_FIXES = {0x010b: 'Dimmable Plug-In Unit', 0x010a: 'On/Off Plug-in Unit'}


def get_location_from_element(element: ElementTree.Element, cluster_id: int):
    if element.tag == 'feature':
        location = FeaturePathLocation(endpoint_id=0, cluster_id=cluster_id, feature_code=element.attrib['code'])
    elif element.tag == 'command':
        location = CommandPathLocation(endpoint_id=0, cluster_id=cluster_id, command_id=int(element.attrib['id'], 0))
    elif element.tag == 'attribute':
        location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=int(element.attrib['id'], 0))
    elif element.tag == 'event':
        location = EventPathLocation(endpoint_id=0, cluster_id=cluster_id, event_id=int(element.attrib['id'], 0))
    else:
        location = ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id)
    return location


def get_conformance(element: ElementTree.Element, cluster_id: int) -> tuple[ElementTree.Element, typing.Optional[ProblemNotice]]:
    for sub in element:
        if sub.tag in TOP_LEVEL_CONFORMANCE_TAGS:
            return sub, None
    location = get_location_from_element(element, cluster_id)
    problem = ProblemNotice(test_name='Spec XML parsing', location=location,
                            severity=ProblemSeverity.WARNING, problem='Unable to find conformance element')
    return ElementTree.Element(OPTIONAL_CONFORM), problem


class ClusterParser:
    def __init__(self, cluster, cluster_id, name):
        self._problems: list[ProblemNotice] = []
        self._cluster = cluster
        self._cluster_id = cluster_id
        self._name = name

        self._derived = None
        self._is_provisional = False
        try:
            classification = next(cluster.iter('classification'))
            hierarchy = classification.attrib['hierarchy']
            if hierarchy.lower() == 'derived':
                self._derived = classification.attrib['baseCluster']
        except (KeyError, StopIteration):
            self._derived = None

        for id in cluster.iter('clusterIds'):
            if list(id.iter('provisionalConform')):
                self._is_provisional = True

        try:
            classification = next(cluster.iter('classification'))
            self._pics = classification.attrib['picsCode']
        except (KeyError, StopIteration):
            self._pics = None

        if self._cluster_id in ALIAS_PICS.keys():
            self._pics = ALIAS_PICS[cluster_id]

        self.feature_elements = self.get_all_feature_elements()
        self.attribute_elements = self.get_all_attribute_elements()
        self.command_elements = self.get_all_command_elements()
        self.event_elements = self.get_all_event_elements()
        self.params = ConformanceParseParameters(feature_map=self.create_feature_map(), attribute_map=self.create_attribute_map(),
                                                 command_map=self.create_command_map())

    def get_conformance(self, element: ElementTree.Element) -> ElementTree.Element:
        element, problem = get_conformance(element, self._cluster_id)
        if problem:
            self._problems.append(problem)
        return element

    def get_access(self, element: ElementTree.Element) -> Optional[ElementTree.Element]:
        for sub in element:
            if sub.tag == 'access':
                return sub
        return None

    def get_all_type(self, type_container: str, type_name: str, key_name: str) -> list[tuple[ElementTree.Element, ElementTree.Element, ElementTree.Element]]:
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
                access = self.get_access(element)
                ret.append((element, conformance, access))
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
        for element, _, _ in self.feature_elements:
            features[element.attrib['code']] = 1 << int(element.attrib['bit'], 0)
        return features

    def create_attribute_map(self) -> dict[str, uint]:
        attributes = {}
        for element, conformance, _ in self.attribute_elements:
            attributes[element.attrib['name']] = int(element.attrib['id'], 0)
        return attributes

    def create_command_map(self) -> dict[str, uint]:
        commands = {}
        for element, _, _ in self.command_elements:
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

    def parse_write_optional(self, element_xml: ElementTree.Element, access_xml: ElementTree.Element) -> bool:
        return access_xml.attrib['write'] == 'optional'

    def parse_access(self, element_xml: ElementTree.Element, access_xml: ElementTree.Element, conformance: Callable) -> tuple[Optional[Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum], Optional[Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum], Optional[Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum]]:
        ''' Returns a tuple of access types for read / write / invoke'''
        def str_to_access_type(privilege_str: str) -> Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum:
            if privilege_str == 'view':
                return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView
            if privilege_str == 'operate':
                return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate
            if privilege_str == 'manage':
                return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage
            if privilege_str == 'admin':
                return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister

            # We don't know what this means, for now, assume no access and mark a warning
            location = get_location_from_element(element_xml, self._cluster_id)
            self._problems.append(ProblemNotice(test_name='Spec XML parsing', location=location,
                                                severity=ProblemSeverity.WARNING, problem=f'Unknown access type {privilege_str}'))
            return Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue

        if access_xml is None:
            # Derived clusters can inherit their access from the base and that's fine, so don't add an error
            # Similarly, pure base clusters can have the access defined in the derived clusters. If neither has it defined,
            # we will determine this at the end when we put these together.
            # Things with deprecated conformance don't get an access element, and that is also fine.
            # If a device properly passes the conformance test, such elements are guaranteed not to appear on the device.
            if self._derived is not None or is_disallowed(conformance):
                return (None, None, None)

            location = get_location_from_element(element_xml, self._cluster_id)
            self._problems.append(ProblemNotice(test_name='Spec XML parsing', location=location,
                                                severity=ProblemSeverity.WARNING, problem='Unable to find access element'))
            return (None, None, None)
        try:
            read_access = str_to_access_type(access_xml.attrib['readPrivilege'])
        except KeyError:
            read_access = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
        try:
            write_access = str_to_access_type(access_xml.attrib['writePrivilege'])
        except KeyError:
            write_access = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
        try:
            invoke_access = str_to_access_type(access_xml.attrib['invokePrivilege'])
        except KeyError:
            invoke_access = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
        return (read_access, write_access, invoke_access)

    def parse_features(self) -> dict[uint, XmlFeature]:
        features = {}
        for element, conformance_xml, _ in self.feature_elements:
            mask = 1 << int(element.attrib['bit'], 0)
            conformance = self.parse_conformance(conformance_xml)
            if conformance is None:
                continue
            features[mask] = XmlFeature(code=element.attrib['code'], name=element.attrib['name'],
                                        conformance=conformance)
        return features

    def parse_attributes(self) -> dict[uint, XmlAttribute]:
        attributes = {}
        for element, conformance_xml, access_xml in self.attribute_elements:
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
            read_access, write_access, _ = self.parse_access(element, access_xml, conformance)
            write_optional = False
            if write_access not in [None, Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue]:
                write_optional = self.parse_write_optional(element, access_xml)
            attributes[code] = XmlAttribute(name=element.attrib['name'], datatype=datatype,
                                            conformance=conformance, read_access=read_access, write_access=write_access, write_optional=write_optional)
        # Add in the global attributes for the base class
        for id in GlobalAttributeIds:
            # TODO: Add data type here. Right now it's unused. We should parse this from the spec.
            attributes[id] = XmlAttribute(name=id.to_name(), datatype="", conformance=mandatory(
            ), read_access=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView, write_access=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue, write_optional=False)
        return attributes

    def get_command_type(self, element: ElementTree.Element) -> CommandType:
        try:
            if element.attrib['direction'].lower() == 'responsefromserver':
                return CommandType.GENERATED
            if element.attrib['direction'].lower() == 'commandtoclient':
                return CommandType.UNKNOWN
            if element.attrib['direction'].lower() == 'commandtoserver':
                return CommandType.ACCEPTED
            if element.attrib['direction'].lower() == 'responsefromclient':
                return CommandType.UNKNOWN
            raise Exception(f"Unknown direction: {element.attrib['direction']}")
        except KeyError:
            return CommandType.UNKNOWN

    def parse_unknown_commands(self) -> list[XmlCommand]:
        commands = []
        for element, conformance_xml, access_xml in self.command_elements:
            if self.get_command_type(element) != CommandType.UNKNOWN:
                continue
            code = int(element.attrib['id'], 0)
            conformance = self.parse_conformance(conformance_xml)
            commands.append(XmlCommand(id=code, name=element.attrib['name'], conformance=conformance))
        return commands

    def parse_commands(self, command_type: CommandType) -> dict[uint, XmlCommand]:
        commands = {}
        for element, conformance_xml, access_xml in self.command_elements:
            if self.get_command_type(element) != command_type:
                continue
            code = int(element.attrib['id'], 0)
            conformance = self.parse_conformance(conformance_xml)
            if conformance is None:
                continue
            if code in commands:
                conformance = or_operation([conformance, commands[code].conformance])
            commands[code] = XmlCommand(id=code, name=element.attrib['name'], conformance=conformance)
        return commands

    def parse_events(self) -> dict[uint, XmlAttribute]:
        events = {}
        for element, conformance_xml, access_xml in self.event_elements:
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
                          unknown_commands=self.parse_unknown_commands(),
                          events=self.parse_events(), pics=self._pics, is_provisional=self._is_provisional)

    def get_problems(self) -> list[ProblemNotice]:
        return self._problems


def add_cluster_data_from_xml(xml: ElementTree.Element, clusters: dict[int, XmlCluster], pure_base_clusters: dict[str, XmlCluster], ids_by_name: dict[str, int], problems: list[ProblemNotice]) -> None:
    ''' Adds cluster data to the supplied dicts as appropriate

        xml: XML element read from from the XML cluster file
        clusters: dict of id -> XmlCluster. This function will append new clusters as appropriate to this dict.
        pure_base_clusters: dict of base name -> XmlCluster. This data structure is used to hold pure base clusters that don't have
                            an ID. This function will append new pure base clusters as appropriate to this dict.
        ids_by_name: dict of cluster name -> ID. This function will append new IDs as appropriate to this dict.
        problems: list of any problems encountered during spec parsing. This function will append problems as appropriate to this list.
    '''
    cluster = xml.iter('cluster')
    for c in cluster:
        ids = c.iter('clusterId')
        for id in ids:
            name = id.get('name')
            cluster_id = id.get('id')
            if cluster_id:
                cluster_id = int(id.get('id'), 0)
                ids_by_name[name] = cluster_id

            parser = ClusterParser(c, cluster_id, name)
            new = parser.create_cluster()
            problems = problems + parser.get_problems()

            if cluster_id:
                clusters[cluster_id] = new
            else:
                # Fully derived clusters have no id, but also shouldn't appear on a device.
                # We do need to keep them, though, because we need to update the derived
                # clusters. We keep them in a special dict by name, so they can be thrown
                # away later.
                pure_base_clusters[name] = new


def check_clusters_for_unknown_commands(clusters: dict[int, XmlCluster], problems: list[ProblemNotice]):
    for id, cluster in clusters.items():
        for cmd in cluster.unknown_commands:
            problems.append(ProblemNotice(test_name="Spec XML parsing", location=CommandPathLocation(
                endpoint_id=0, cluster_id=id, command_id=cmd.id), severity=ProblemSeverity.WARNING, problem="Command with unknown direction"))


class PrebuiltDataModelDirectory(Enum):
    k1_3 = auto()
    k1_4 = auto()
    kMaster = auto()


class DataModelLevel(str, Enum):
    kCluster = 'clusters'
    kDeviceType = 'device_types'


def _get_data_model_directory(data_model_directory: typing.Union[PrebuiltDataModelDirectory, str], data_model_level: DataModelLevel) -> str:
    if data_model_directory == PrebuiltDataModelDirectory.k1_3:
        return os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'data_model', '1.3', data_model_level)
    elif data_model_directory == PrebuiltDataModelDirectory.k1_4:
        return os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'data_model', '1.4', data_model_level)
    elif data_model_directory == PrebuiltDataModelDirectory.kMaster:
        return os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'data_model', 'master', data_model_level)
    else:
        return data_model_directory


def build_xml_clusters(data_model_directory: typing.Union[PrebuiltDataModelDirectory, str] = PrebuiltDataModelDirectory.k1_4) -> tuple[dict[uint, XmlCluster], list[ProblemNotice]]:
    dir = _get_data_model_directory(data_model_directory, DataModelLevel.kCluster)

    clusters: dict[int, XmlCluster] = {}
    pure_base_clusters: dict[str, XmlCluster] = {}
    ids_by_name: dict[str, int] = {}
    problems: list[ProblemNotice] = []
    files = glob.glob(f'{dir}/*.xml')
    if not files:
        raise SpecParsingException(f'No data model files found in specified directory {dir}')

    for xml in files:
        logging.info(f'Parsing file {xml}')
        tree = ElementTree.parse(f'{xml}')
        root = tree.getroot()
        add_cluster_data_from_xml(root, clusters, pure_base_clusters, ids_by_name, problems)

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

    combine_derived_clusters_with_base(clusters, pure_base_clusters, ids_by_name, problems)

    # TODO: All these fixups should be removed BEFORE SVE if at all possible
    # Workaround for Color Control cluster - the spec uses a non-standard conformance. Set all to optional now, will need
    # to implement either arithmetic conformance handling (once spec changes land here) or specific test
    # https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/7808 for spec changes.
    # see 3.2.8. Defined Primaries Information Attribute Set, affects Primary<#>X/Y/Intensity attributes.
    cc_id = Clusters.ColorControl.id
    cc_attr = Clusters.ColorControl.Attributes
    affected_attributes = [cc_attr.Primary1X,
                           cc_attr.Primary1Y,
                           cc_attr.Primary1Intensity,
                           cc_attr.Primary2X,
                           cc_attr.Primary2Y,
                           cc_attr.Primary2Intensity,
                           cc_attr.Primary3X,
                           cc_attr.Primary3Y,
                           cc_attr.Primary3Intensity,
                           cc_attr.Primary4X,
                           cc_attr.Primary4Y,
                           cc_attr.Primary4Intensity,
                           cc_attr.Primary5X,
                           cc_attr.Primary5Y,
                           cc_attr.Primary5Intensity,
                           cc_attr.Primary6X,
                           cc_attr.Primary6Y,
                           cc_attr.Primary6Intensity,
                           ]
    for a in affected_attributes:
        clusters[cc_id].attributes[a.attribute_id].conformance = optional()

    # Workaround for temp control cluster - this is parsed incorrectly in the DM XML and is missing all its attributes
    # Remove this workaround when https://github.com/csa-data-model/projects/issues/330 is fixed
    temp_control_id = Clusters.TemperatureControl.id
    if temp_control_id in clusters and not clusters[temp_control_id].attributes:
        view = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView
        none = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
        clusters[temp_control_id].attributes = {
            0x00: XmlAttribute(name='TemperatureSetpoint', datatype='temperature', conformance=feature(0x01, 'TN'), read_access=view, write_access=none, write_optional=False),
            0x01: XmlAttribute(name='MinTemperature', datatype='temperature', conformance=feature(0x01, 'TN'), read_access=view, write_access=none, write_optional=False),
            0x02: XmlAttribute(name='MaxTemperature', datatype='temperature', conformance=feature(0x01, 'TN'), read_access=view, write_access=none, write_optional=False),
            0x03: XmlAttribute(name='Step', datatype='temperature', conformance=feature(0x04, 'STEP'), read_access=view, write_access=none, write_optional=False),
            0x04: XmlAttribute(name='SelectedTemperatureLevel', datatype='uint8', conformance=feature(0x02, 'TL'), read_access=view, write_access=none, write_optional=False),
            0x05: XmlAttribute(name='SupportedTemperatureLevels', datatype='list', conformance=feature(0x02, 'TL'), read_access=view, write_access=none, write_optional=False),
        }

    # TODO: Need automated parsing for atomic attributes.
    atomic_request_cmd_id = 0xFE
    atomic_response_cmd_id = 0xFD
    atomic_request_name = "Atomic Request"
    atomic_response_name = "Atomic Response"
    presets_name = "Presets"
    schedules_name = "Schedules"
    if clusters[Clusters.Thermostat.id].revision >= 8:
        presents_id = clusters[Clusters.Thermostat.id].attribute_map[presets_name]
        schedules_id = clusters[Clusters.Thermostat.id].attribute_map[schedules_name]
        conformance = or_operation([conformance_support.attribute(presents_id, presets_name),
                                   conformance_support.attribute(schedules_id, schedules_name)])
        clusters[Clusters.Thermostat.id].accepted_commands[atomic_request_cmd_id] = XmlCommand(
            id=atomic_request_cmd_id, name=atomic_request_name, conformance=conformance)
        clusters[Clusters.Thermostat.id].generated_commands[atomic_response_cmd_id] = XmlCommand(
            id=atomic_response_cmd_id, name=atomic_response_name, conformance=conformance)
        clusters[Clusters.Thermostat.id].command_map[atomic_request_name] = atomic_request_cmd_id
        clusters[Clusters.Thermostat.id].command_map[atomic_response_name] = atomic_response_cmd_id

    check_clusters_for_unknown_commands(clusters, problems)

    return clusters, problems


def combine_derived_clusters_with_base(xml_clusters: dict[int, XmlCluster], pure_base_clusters: dict[str, XmlCluster], ids_by_name: dict[str, int], problems: list[ProblemNotice]) -> None:
    ''' Overrides base elements with the derived cluster values for derived clusters. '''

    def combine_attributes(base: dict[uint, XmlAttribute], derived: dict[uint, XmlAttribute], cluster_id: uint, problems: list[ProblemNotice]) -> dict[uint, XmlAttribute]:
        ret = deepcopy(base)
        extras = {k: v for k, v in derived.items() if k not in base.keys()}
        overrides = {k: v for k, v in derived.items() if k in base.keys()}
        ret.update(extras)
        for id, override in overrides.items():
            if override.conformance:
                ret[id].conformance = override.conformance
            if override.read_access:
                ret[id].read_access = override.read_access
            if override.write_access:
                ret[id].write_access = override.write_access
            if ret[id].read_access is None and ret[id].write_access is None:
                location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=id)
                problems.append(ProblemNotice(test_name='Spec XML parsing', location=location,
                                              severity=ProblemSeverity.WARNING, problem='Unable to find access element'))
            if ret[id].read_access is None:
                ret[id].read_access == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
            if ret[id].write_access is None:
                ret[id].write_access = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue
        return ret

    # We have the information now about which clusters are derived, so we need to fix them up. Apply first the base cluster,
    # then add the specific cluster overtop
    for id, c in xml_clusters.items():
        if c.derived:
            base_name = c.derived
            if base_name in ids_by_name:
                base = xml_clusters[ids_by_name[c.derived]]
            else:
                base = pure_base_clusters[base_name]

            feature_map = deepcopy(base.feature_map)
            feature_map.update(c.feature_map)
            attribute_map = deepcopy(base.attribute_map)
            attribute_map.update(c.attribute_map)
            command_map = deepcopy(base.command_map)
            command_map.update(c.command_map)
            features = deepcopy(base.features)
            features.update(c.features)
            attributes = combine_attributes(base.attributes, c.attributes, id, problems)
            accepted_commands = deepcopy(base.accepted_commands)
            accepted_commands.update(c.accepted_commands)
            generated_commands = deepcopy(base.generated_commands)
            generated_commands.update(c.generated_commands)
            events = deepcopy(base.events)
            events.update(c.events)
            unknown_commands = deepcopy(base.unknown_commands)
            for cmd in c.unknown_commands:
                if cmd.id in accepted_commands.keys() and cmd.name == accepted_commands[cmd.id].name:
                    accepted_commands[cmd.id].conformance = cmd.conformance
                elif cmd.id in generated_commands.keys() and cmd.name == generated_commands[cmd.id].name:
                    generated_commands[cmd.id].conformance = cmd.conformance
                else:
                    unknown_commands.append(cmd)
            provisional = c.is_provisional or base.is_provisional

            new = XmlCluster(revision=c.revision, derived=c.derived, name=c.name,
                             feature_map=feature_map, attribute_map=attribute_map, command_map=command_map,
                             features=features, attributes=attributes, accepted_commands=accepted_commands,
                             generated_commands=generated_commands, unknown_commands=unknown_commands, events=events, pics=c.pics,
                             is_provisional=provisional)
            xml_clusters[id] = new


def parse_single_device_type(root: ElementTree.Element) -> tuple[list[ProblemNotice], dict[int, XmlDeviceType]]:
    problems: list[ProblemNotice] = []
    device_types: dict[int, XmlDeviceType] = {}
    device = root.iter('deviceType')
    for d in device:
        name = d.attrib['name']
        location = DeviceTypePathLocation(device_type_id=0)

        str_id = d.attrib['id']
        if not str_id:
            if name == "Base Device Type":
                # Base is special device type, we're going to call it -1 so we can combine and remove it later.
                str_id = '-1'
            else:
                problems.append(ProblemNotice("Parse Device Type XML", location=location,
                                severity=ProblemSeverity.WARNING, problem=f"Device type {name} does not have an ID listed"))
                break
        try:
            id = int(str_id, 0)
            revision = int(d.attrib['revision'], 0)
        except ValueError:
            problems.append(ProblemNotice("Parse Device Type XML", location=location,
                            severity=ProblemSeverity.WARNING,
                            problem=f"Device type {name} does not a valid ID or revision. ID: {str_id} revision: {d.get('revision', 'UNKNOWN')}"))
            break
        if id in DEVICE_TYPE_NAME_FIXES:
            name = DEVICE_TYPE_NAME_FIXES[id]
        try:
            classification = next(d.iter('classification'))
            scope = classification.attrib['scope']
            device_class = classification.attrib['class']
        except (KeyError, StopIteration):
            # this is fine for base device type
            if id == -1:
                classification = 'BASE'
                scope = 'BASE'
                device_class = 'BASE'
            else:
                location = DeviceTypePathLocation(device_type_id=id)
                problems.append(ProblemNotice("Parse Device Type XML", location=location,
                                severity=ProblemSeverity.WARNING, problem="Unable to find classification data for device type"))
                break
        device_types[id] = XmlDeviceType(name=name, revision=revision, server_clusters={}, client_clusters={},
                                         classification_class=device_class, classification_scope=scope)
        clusters = d.iter('cluster')
        for c in clusters:
            try:
                cid = int(c.attrib['id'], 0)
                conformance_xml, tmp_problem = get_conformance(c, cid)
                if tmp_problem:
                    problems.append(tmp_problem)
                conformance = parse_device_type_callable_from_xml(conformance_xml)
                side_dict = {'server': ClusterSide.SERVER, 'client': ClusterSide.CLIENT}
                side = side_dict[c.attrib['side']]
                name = c.attrib['name']
                if cid in CLUSTER_NAME_FIXES:
                    name = CLUSTER_NAME_FIXES[cid]
                cluster = XmlDeviceTypeClusterRequirements(name=name, side=side, conformance=conformance)
                if side == ClusterSide.SERVER:
                    device_types[id].server_clusters[cid] = cluster
                else:
                    device_types[id].client_clusters[cid] = cluster
            except ConformanceException:
                location = DeviceTypePathLocation(device_type_id=id, cluster_id=cid)
                problems.append(ProblemNotice("Parse Device Type XML", location=location,
                                severity=ProblemSeverity.WARNING, problem="Unable to parse conformance for cluster"))
            # TODO: Check for features, attributes and commands as element requirements
            # NOTE: Spec currently does a bad job of matching these exactly to the names and codes
            # so this will need a bit of fancy handling here to get this right.
    return device_types, problems


def build_xml_device_types(data_model_directory: typing.Union[PrebuiltDataModelDirectory, str] = PrebuiltDataModelDirectory.k1_4) -> tuple[dict[int, XmlDeviceType], list[ProblemNotice]]:
    dir = _get_data_model_directory(data_model_directory, DataModelLevel.kDeviceType)
    device_types: dict[int, XmlDeviceType] = {}
    problems = []
    for xml in glob.glob(f"{dir}/*.xml"):
        logging.info(f'Parsing file {xml}')
        tree = ElementTree.parse(f'{xml}')
        root = tree.getroot()
        tmp_device_types, tmp_problems = parse_single_device_type(root)
        problems = problems + tmp_problems
        device_types.update(tmp_device_types)

    if -1 not in device_types.keys():
        raise ConformanceException("Base device type not found in device type xml data")

    # Add in the base device type information and remove the base device type from the device_types
    for d in device_types.values():
        d.server_clusters.update(device_types[-1].server_clusters)
    device_types.pop(-1)

    return device_types, problems
