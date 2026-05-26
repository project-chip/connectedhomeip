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
import zipfile

import matter.clusters as Clusters
from matter.clusters.Attribute import AsyncReadTransaction
from matter.testing.global_attribute_ids import (AttributeIdType, GlobalAttributeIds, attribute_id_type, is_standard_cluster_id,
                                                 is_standard_command_id)
from matter.testing.problem_notices import ClusterPathLocation, ProblemNotice, ProblemSeverity
from matter.testing.spec_parsing import XmlCluster
from matter.tlv import uint


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


def _sorted_endpoint_subdirs(dir_path: str) -> list[str]:
    """Immediate subdirectories that may each hold PICS XML for one endpoint."""
    entries: list[str] = []
    for name in os.listdir(dir_path):
        if name.startswith('.') or name == '__MACOSX':
            continue
        full = os.path.join(dir_path, name)
        if os.path.isdir(full):
            entries.append(full)

    def sort_key(p: str) -> tuple[int, typing.Union[int, str]]:
        base = os.path.basename(p)
        if base.isdigit():
            return (0, int(base))
        return (1, base.lower())

    return sorted(entries, key=sort_key)


def _iter_pics_xml_under_endpoint_dirs(root_dir: str) -> typing.Iterator[str]:
    for sub in _sorted_endpoint_subdirs(root_dir):
        for dirpath, _, filenames in os.walk(sub):
            for fn in sorted(filenames):
                if fn.lower().endswith('.xml'):
                    yield os.path.join(dirpath, fn)


def _read_pics_from_zip(zip_path: str) -> dict[str, bool]:
    pics_dict: dict[str, bool] = {}
    with zipfile.ZipFile(zip_path) as zf:
        for name in sorted(zf.namelist()):
            normalized = name.replace('\\', '/')
            if normalized.startswith('__MACOSX/') or normalized == '__MACOSX':
                continue
            if normalized.endswith('/'):
                continue
            if not name.lower().endswith('.xml'):
                continue
            with zf.open(name) as member:
                data = member.read().decode('utf-8')
                pics_dict.update(parse_pics_xml(data))
    return pics_dict


def _read_pics_from_directory(dir_path: str) -> dict[str, bool]:
    pics_dict: dict[str, bool] = {}
    top_level = sorted(glob.glob(os.path.join(dir_path, '*.xml')))
    for filename in top_level:
        with open(filename, encoding='utf-8') as f:
            pics_dict.update(parse_pics_xml(f.read()))
    for xml_path in _iter_pics_xml_under_endpoint_dirs(dir_path):
        with open(xml_path, encoding='utf-8') as f:
            pics_dict.update(parse_pics_xml(f.read()))
    return pics_dict


def read_pics_from_file(path: str) -> dict[str, bool]:
    """Load PICS definitions from a path.

    Supported inputs:

    * A text file in CI format (``KEY=0`` or ``KEY=1`` per line), as used by
      ``ci-pics-values``.
    * A directory of PICS XML files for a single endpoint (``*.xml`` in the
      directory).
    * A directory whose immediate subdirectories each hold XML files for a
      distinct device endpoint (numeric names such as ``0``, ``1`` are sorted in
      numeric order; other names sort case-insensitively). XML files may appear
      at any depth under each subdirectory.
    * A ``.zip`` archive with the same layout as that directory (any ``*.xml``
      members are merged; ``__MACOSX`` is ignored).

    When the same PICS item appears in multiple XML sources, the last read
    definition wins.
    """
    expanded = os.path.expanduser(path)
    ap = os.path.abspath(expanded)

    if os.path.isdir(ap):
        return _read_pics_from_directory(ap)

    if os.path.isfile(ap) and zipfile.is_zipfile(ap):
        return _read_pics_from_zip(ap)

    with open(ap, encoding='utf-8') as f:
        lines = f.readlines()
    return parse_pics(lines)


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
