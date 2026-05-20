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
import glob
import json
import logging
import os
import typing
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field

import matter.clusters as Clusters
from matter.clusters.Attribute import AsyncReadTransaction
from matter.testing.conformance import ConformanceAssessmentData, ConformanceException
from matter.testing.global_attribute_ids import (AttributeIdType, GlobalAttributeIds, attribute_id_type, is_standard_cluster_id,
                                                 is_standard_command_id)
from matter.testing.problem_notices import ClusterPathLocation, ProblemNotice, ProblemSeverity
from matter.testing.spec_parsing import XmlCluster
from matter.tlv import uint

LOGGER = logging.getLogger(__name__)

# Aggregator device type ID. Endpoints with this device type identify the
# device as a Bridge for the MCORE.BRIDGE PICS.
_AGGREGATOR_DEVICE_TYPE_ID = 0x000E

# Root node device type ID. EP0 carries this on any commissionable Matter
# device. Used to derive MCORE.ROLE.COMMISSIONEE.
_ROOT_NODE_DEVICE_TYPE_ID = 0x0016

# Network Commissioning cluster ID + WiFi feature bit. Used to gate the
# SupportedWiFiBands read that drives the band PICS.
_NETWORK_COMMISSIONING_CLUSTER_ID = 0x0031
_NETCOMM_FEATURE_BIT_WIFI = 0


def event_pics_str(pics_base: str, eid: int) -> str:
    return f'{pics_base}.S.E{eid:02x}'


def attribute_pics_str(pics_base: str, aid: int) -> str:
    return f'{pics_base}.S.A{aid:04x}'


def accepted_cmd_pics_str(pics_base: str, cid: int) -> str:
    return f'{pics_base}.S.C{cid:02x}.Rsp'


def generated_cmd_pics_str(pics_base: str, cid: int) -> str:
    return f'{pics_base}.S.C{cid:02x}.Tx'


def feature_pics_str(pics_base: str, bit: int) -> str:
    return f'{pics_base}.S.F{bit:02x}'


def server_pics_str(pics_base: str) -> str:
    return f'{pics_base}.S'


def client_pics_str(pics_base: str) -> str:
    return f'{pics_base}.C'


def parse_pics(lines: typing.List[str]) -> dict[str, bool]:
    pics = {}
    for raw in lines:
        line, _, _ = raw.partition("#")
        line = line.strip()

        if not line:
            continue

        key, _, val = line.partition("=")
        val = val.strip()
        if val not in ["1", "0"]:
            raise ValueError('PICS {} must have a value of 0 or 1'.format(key))

        pics[key.strip()] = (val == "1")
    return pics


def parse_pics_xml(contents: str) -> dict[str, bool]:
    pics: dict[str, bool] = {}
    mytree = ET.fromstring(contents)
    for pi in mytree.iter('picsItem'):
        name_elem = pi.find('itemNumber')
        support_elem = pi.find('support')

        # Raise an error if either element is None
        if name_elem is None:
            raise ValueError(f"PICS XML item missing 'itemNumber' element: {ET.tostring(pi, encoding='unicode')}")
        if support_elem is None:
            raise ValueError(f"PICS XML item missing 'support' element: {ET.tostring(pi, encoding='unicode')}")

        # Raise an error if either text is None
        name = name_elem.text
        support = support_elem.text
        if name is None:
            raise ValueError(f"PICS XML item 'itemNumber' element missing text: {ET.tostring(pi, encoding='unicode')}")
        if support is None:
            raise ValueError(f"PICS XML item 'support' element missing text: {ET.tostring(pi, encoding='unicode')}")

        pics[name] = int(json.loads(support.lower())) == 1
    return pics


def read_pics_from_file(path: str) -> dict[str, bool]:
    """ Reads a dictionary of PICS from a file (ci format) or directory (xml format). """
    if os.path.isdir(os.path.abspath(path)):
        pics_dict = {}
        for filename in glob.glob(f'{path}/*.xml'):
            with open(filename, 'r') as f:
                contents = f.read()
                pics_dict.update(parse_pics_xml(contents))
        return pics_dict

    with open(path, 'r') as f:
        lines = f.readlines()
        return parse_pics(lines)


@dataclass
class BasePicsFacts:
    """
    Device facts that map to Base/MCORE PICS codes.

    Populated by derive_base_pics_facts_from_device_wildcard. Two consumers
    today: PICSGenerator writes these to Base.xml, and TC_IDM_10_4 asserts
    them against the supplied PICS file.

    Wi-Fi bands come from NetworkCommissioning's SupportedWiFiBands. Wi-Fi /
    Thread / Ethernet top-level transport bits and MCORE.COM.WIRELESS are
    intentionally not derived here while QA stress-test feedback on those is
    outstanding (Cecille, May 2026). PICSGenerator continues to derive them
    locally until the test-plans cleanup PRs land.
    """
    is_commissionee: bool = False
    is_server: bool = False
    is_bridge: bool = False
    is_ota_requestor: bool = False
    is_ota_provider: bool = False
    has_groups_on_multiple_endpoints: bool = False
    supports_wifi_2g4: bool = False
    supports_wifi_5g: bool = False
    # endpoint_id -> cluster_id -> set of event ids the spec marks MANDATORY
    # for this device's feature set, attribute list, command list, and
    # cluster revision. Populated by running each XmlEvent's parsed
    # conformance against ConformanceAssessmentData built from the wildcard.
    mandatory_events_by_cluster: dict[int, dict[int, set[int]]] = field(default_factory=dict)


# Complete set of Base/MCORE PICS codes this helper knows how to derive. The
# Base/MCORE TC_IDM_10_4 step uses this set for the "device says no, so PICS
# file must also say no" half of the consistency check.
BASE_PICS_CODES_DERIVED: frozenset[str] = frozenset({
    "MCORE.ROLE.COMMISSIONEE",
    "MCORE.IDM.S",
    "MCORE.BRIDGE",
    "MCORE.OTA.Requestor",
    "MCORE.OTA.Provider",
    "MCORE.G.MULTIENDPOINT",
    "MCORE.COM.WIFI_2P4GHZ",
    "MCORE.COM.WIFI_5GHZ",
})


def base_pics_facts_to_pics_codes(facts: BasePicsFacts) -> set[str]:
    """
    Translate a BasePicsFacts to the set of MCORE PICS codes that should be
    marked true.

    Only codes in BASE_PICS_CODES_DERIVED can appear in the result. Per-event
    PICS (cluster.S.E<id>) are NOT included here; those have a different
    callable (event_pics_str) and a separate iteration path on the consumer.
    """
    codes: set[str] = set()
    if facts.is_commissionee:
        codes.add("MCORE.ROLE.COMMISSIONEE")
    if facts.is_server:
        codes.add("MCORE.IDM.S")
    if facts.is_bridge:
        codes.add("MCORE.BRIDGE")
    if facts.is_ota_requestor:
        codes.add("MCORE.OTA.Requestor")
    if facts.is_ota_provider:
        codes.add("MCORE.OTA.Provider")
    if facts.has_groups_on_multiple_endpoints:
        codes.add("MCORE.G.MULTIENDPOINT")
    if facts.supports_wifi_2g4:
        codes.add("MCORE.COM.WIFI_2P4GHZ")
    if facts.supports_wifi_5g:
        codes.add("MCORE.COM.WIFI_5GHZ")
    return codes


def derive_base_pics_facts_from_device_wildcard(
    wildcard: AsyncReadTransaction.ReadResponse,
    xml_clusters: dict[uint, XmlCluster],
) -> tuple[BasePicsFacts, list[ProblemNotice]]:
    """
    Derive device-fact-based Base/MCORE PICS from a wildcard read.

    Mirrors the in-scope logic in PICSGenerator.DeviceMapping. Does not touch
    MCORE.COM.WIFI / THR / ETH / WIRELESS while GRL stress-test feedback on
    those is outstanding.
    """
    facts = BasePicsFacts()
    problems: list[ProblemNotice] = []

    # Commissionee: root node device type on EP0. Matches the rule already
    # used by generate_device_element_pics_from_device_wildcard below.
    ep0_device_type_list = wildcard.attributes.get(0, {}).get(
        Clusters.Descriptor, {}).get(Clusters.Descriptor.Attributes.DeviceTypeList, [])
    if any(d.deviceType == _ROOT_NODE_DEVICE_TYPE_ID for d in ep0_device_type_list):
        facts.is_commissionee = True

    # Bridge: aggregator device type on any endpoint.
    for endpoint_id, endpoint_attributes in wildcard.attributes.items():
        device_type_list = endpoint_attributes.get(
            Clusters.Descriptor, {}).get(Clusters.Descriptor.Attributes.DeviceTypeList, [])
        if any(d.deviceType == _AGGREGATOR_DEVICE_TYPE_ID for d in device_type_list):
            facts.is_bridge = True
            break

    groups_endpoint_count = 0

    for endpoint_id, endpoint in wildcard.tlvAttributes.items():
        endpoint_has_server = False
        for cluster_id, cluster_attrs in endpoint.items():
            if not is_standard_cluster_id(cluster_id):
                continue
            endpoint_has_server = True

            # OTA Requestor/Provider and Groups membership keyed off cluster
            # presence in the wildcard. The wildcard already implies the
            # cluster is on the ServerList, so no separate Descriptor read.
            if cluster_id == Clusters.OtaSoftwareUpdateRequestor.id:
                facts.is_ota_requestor = True
            if cluster_id == Clusters.OtaSoftwareUpdateProvider.id:
                facts.is_ota_provider = True
            if cluster_id == Clusters.Groups.id:
                groups_endpoint_count += 1

            # Wi-Fi bands. Read SupportedWiFiBands from the
            # NetworkCommissioning cluster when WiFi is in the feature map.
            if cluster_id == _NETWORK_COMMISSIONING_CLUSTER_ID:
                feature_map = cluster_attrs.get(GlobalAttributeIds.FEATURE_MAP_ID, 0)
                if feature_map & (1 << _NETCOMM_FEATURE_BIT_WIFI):
                    wifi_bands = wildcard.attributes.get(endpoint_id, {}).get(
                        Clusters.NetworkCommissioning, {}).get(
                            Clusters.NetworkCommissioning.Attributes.SupportedWiFiBands, [])
                    WiFiBandEnum = Clusters.NetworkCommissioning.Enums.WiFiBandEnum
                    if WiFiBandEnum.k2g4 in wifi_bands:
                        facts.supports_wifi_2g4 = True
                    if WiFiBandEnum.k5g in wifi_bands:
                        facts.supports_wifi_5g = True

            # Mandatory events: build a ConformanceAssessmentData for this
            # cluster instance and ask each XmlEvent's conformance whether
            # it's mandatory. EventList (0xFFFA) is provisional and not read
            # back by the SDK, so spec conformance is the only signal.
            if cluster_id in xml_clusters and xml_clusters[cluster_id].events:
                attribute_list = list(cluster_attrs.get(GlobalAttributeIds.ATTRIBUTE_LIST_ID, []))
                accepted_commands = list(cluster_attrs.get(GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID, []))
                generated_commands = list(cluster_attrs.get(GlobalAttributeIds.GENERATED_COMMAND_LIST_ID, []))
                assessment_data = ConformanceAssessmentData(
                    feature_map=uint(cluster_attrs.get(GlobalAttributeIds.FEATURE_MAP_ID, 0)),
                    attribute_list=attribute_list,
                    all_command_list=accepted_commands + generated_commands,
                    cluster_revision=uint(cluster_attrs.get(GlobalAttributeIds.CLUSTER_REVISION_ID, 1)),
                )
                for event_id, xml_event in xml_clusters[cluster_id].events.items():
                    try:
                        decision = xml_event.conformance(assessment_data)
                    except ConformanceException as e:
                        LOGGER.debug(
                            "Conformance evaluation failed for endpoint %s cluster 0x%04x event 0x%02x: %s",
                            endpoint_id, cluster_id, event_id, e)
                        continue
                    if decision.is_mandatory():
                        facts.mandatory_events_by_cluster.setdefault(
                            endpoint_id, {}).setdefault(cluster_id, set()).add(event_id)

        if endpoint_has_server:
            facts.is_server = True

    if groups_endpoint_count >= 2:
        facts.has_groups_on_multiple_endpoints = True

    return facts, problems


def generate_device_element_pics_from_device_wildcard(wildcard: AsyncReadTransaction.ReadResponse, xml_clusters: dict[uint, XmlCluster]) -> tuple[dict[int, list[str]], list[ProblemNotice]]:
    ''' Returns a list of device element PICS and problems from each device wildcard.
    '''
    # Endpoint to list of device element PICS
    device_pics: dict[int, list[str]] = {}
    problems = []
    for endpoint_id, endpoint in wildcard.tlvAttributes.items():
        endpoint_has_server = False
        device_pics[endpoint_id] = []
        for cluster_id, cluster in endpoint.items():
            if not is_standard_cluster_id(cluster_id):
                continue
            if cluster_id not in xml_clusters:
                # This is covered by another test - we don't want to block every test, so just warn here
                location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id)
                problems.append(ProblemNotice(test_name="General error", location=location,
                                severity=ProblemSeverity.WARNING, problem="Unknown standard cluster on device"))
                continue
            cluster_pics = xml_clusters[cluster_id].pics
            device_pics[endpoint_id].append(server_pics_str(cluster_pics))
            endpoint_has_server = True
            for attribute_id in cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]:
                if attribute_id_type(attribute_id) != AttributeIdType.kStandardNonGlobal:
                    continue
                device_pics[endpoint_id].append(attribute_pics_str(cluster_pics, attribute_id))
            feature_map = cluster[GlobalAttributeIds.FEATURE_MAP_ID]
            for i in range(0, 16):
                bit = 1 << i
                if feature_map & bit:
                    device_pics[endpoint_id].append(feature_pics_str(cluster_pics, i))
            for cmd_id in cluster[GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID]:
                if not is_standard_command_id(cmd_id):
                    continue
                device_pics[endpoint_id].append(accepted_cmd_pics_str(cluster_pics, cmd_id))
            for cmd_id in cluster.get(GlobalAttributeIds.GENERATED_COMMAND_LIST_ID, []):
                if not is_standard_command_id(cmd_id):
                    continue
                device_pics[endpoint_id].append(generated_cmd_pics_str(cluster_pics, cmd_id))
        if endpoint_has_server:
            device_pics[endpoint_id].append('IDM.S')
    ep0_device_type_list = wildcard.attributes.get(0, {}).get(
        Clusters.Descriptor, {}).get(Clusters.Descriptor.Attributes.DeviceTypeList, [])
    if any(d.deviceType == 0x16 for d in ep0_device_type_list):
        device_pics.setdefault(0, []).append('MCORE.ROLE.COMMISSIONEE')

    return device_pics, problems
