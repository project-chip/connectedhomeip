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
import os
import typing
import xml.etree.ElementTree as ET

import matter.clusters as Clusters
from matter.clusters.Attribute import AsyncReadTransaction
from matter.testing.global_attribute_ids import (AttributeIdType, GlobalAttributeIds, attribute_id_type, is_standard_cluster_id,
                                                 is_standard_command_id)
from matter.testing.problem_notices import ClusterPathLocation, ProblemNotice, ProblemSeverity
from matter.testing.spec_parsing import XmlCluster
from matter.tlv import uint


def attribute_pics_str(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.A{id:04x}'


def accepted_cmd_pics_str(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Rsp'


def generated_cmd_pics_str(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Tx'


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
