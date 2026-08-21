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
import re
import tempfile
import xml.etree.ElementTree as ET
import zipfile
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

_ENDPOINT_DIR_PATTERN = re.compile(r'^(?:endpoint|ep)?[\s_-]*(\d+)$', re.IGNORECASE)


# Sanity limits for zip extraction. PICS bundles in the wild are on the
# order of hundreds of KiB across ~100 XML files, so these bounds are
# very generous while still capping runaway or malformed archives (one
# with millions of members, or one whose metadata claims tens of GiB).
# These check *reported* uncompressed sizes; a maliciously crafted zip
# whose metadata lies about entry size can still exceed the limit at
# extract time. That's out of scope for a test-harness input reader:
# the caller controls the file and we're guarding against accidents,
# not adversarial input.
_MAX_ZIP_MEMBERS = 1000
_MAX_ZIP_TOTAL_BYTES = 50 * 1024 * 1024  # 50 MiB

# Archive-metadata names that OS "compress folder" utilities scatter
# alongside the real payload. Filtered out when detecting a wrapping
# directory so a macOS-Finder zip (which adds __MACOSX/ next to the
# compressed folder) is still recognised as single-wrapper.
_ARCHIVE_METADATA_ENTRIES = frozenset({
    '__MACOSX', '.DS_Store', 'Thumbs.db', 'desktop.ini',
})


def _find_endpoint_subdir(root_dir: str, endpoint: int) -> str | None:
    """
    Find the subdirectory under root_dir whose name resolves to `endpoint`.
    Tolerates common conventions: endpoint0, Endpoint_0, EP0, ep 0, 0, etc.
    Case-insensitive. Returns None if no match.
    """
    for name in os.listdir(root_dir):
        full = os.path.join(root_dir, name)
        if not os.path.isdir(full):
            continue
        match = _ENDPOINT_DIR_PATTERN.match(name)
        if match and int(match.group(1)) == endpoint:
            return full
    return None


def _safe_extract_zip(zip_path: str, dest: str) -> None:
    """
    Extract `zip_path` into `dest`, refusing:
      * entries whose resolved path escapes `dest` (absolute paths or `..`
        traversal — the "zip slip" pattern),
      * archives with more than `_MAX_ZIP_MEMBERS` entries, and
      * archives whose total reported uncompressed size exceeds
        `_MAX_ZIP_TOTAL_BYTES` (guards against zip-bomb-style disk use).

    Portable pre-check rather than relying on the version-dependent
    `extractall(filter=...)` behavior added in 3.12.
    """
    dest_abs = os.path.abspath(dest)
    with zipfile.ZipFile(zip_path) as zf:
        infos = zf.infolist()
        if len(infos) > _MAX_ZIP_MEMBERS:
            raise ValueError(
                f"Zip has {len(infos)} entries; exceeds limit of {_MAX_ZIP_MEMBERS}")
        total_bytes = 0
        for info in infos:
            total_bytes += info.file_size
            if total_bytes > _MAX_ZIP_TOTAL_BYTES:
                raise ValueError(
                    f"Zip total uncompressed size exceeds limit of "
                    f"{_MAX_ZIP_TOTAL_BYTES} bytes")
            target = os.path.abspath(os.path.join(dest_abs, info.filename))
            # dest_abs itself is fine (empty member); everything else must
            # sit strictly under it.
            if target != dest_abs and not target.startswith(dest_abs + os.sep):
                raise ValueError(
                    f"Refusing to extract zip entry outside destination: "
                    f"{info.filename!r}")
        zf.extractall(dest_abs)


def _pics_root_within(extract_dir: str) -> str:
    """
    Locate the effective PICS root inside an extracted archive.

    OS "compress folder" utilities (macOS Finder, Windows Explorer, `zip -r`
    when called on a directory) produce archives with everything nested
    under a single top-level directory. Detect that case and descend one
    level so users don't have to re-zip. Otherwise use `extract_dir` as-is.

    Known archive-metadata entries (`__MACOSX/`, `.DS_Store`, ...) are
    ignored when counting; without this a macOS-Finder zip would look like
    a multi-entry archive (payload + `__MACOSX/`) and never unwrap. A lone
    directory whose name matches the endpoint pattern is left alone: it IS
    the per-endpoint subdir the reader is looking for, not a wrapper, and
    drilling in would demote its per-endpoint PICS to device-wide PICS.
    """
    if glob.glob(os.path.join(extract_dir, '*.xml')):
        return extract_dir
    entries = [e for e in os.listdir(extract_dir)
               if e not in _ARCHIVE_METADATA_ENTRIES]
    if len(entries) == 1:
        only = os.path.join(extract_dir, entries[0])
        if os.path.isdir(only) and not _ENDPOINT_DIR_PATTERN.match(entries[0]):
            return only
    return extract_dir


def _read_pics_from_directory(path: str, endpoint: int | None) -> dict[str, bool]:
    """
    Load PICS XML files from a directory tree. See `read_pics_from_file`
    for the loading rules; this is the shared implementation used both for
    directory paths and for the temporary directory an archive is
    extracted into.
    """
    pics_dict: dict[str, bool] = {}
    for filename in glob.glob(f'{path}/*.xml'):
        with open(filename) as f:
            pics_dict.update(parse_pics_xml(f.read()))
    if endpoint is not None:
        ep_dir = _find_endpoint_subdir(path, endpoint)
        if ep_dir is not None:
            for filename in glob.glob(f'{ep_dir}/*.xml'):
                with open(filename) as f:
                    pics_dict.update(parse_pics_xml(f.read()))
    return pics_dict


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


def parse_pics(lines: list[str]) -> dict[str, bool]:
    pics = {}
    for raw in lines:
        line, _, _ = raw.partition("#")
        line = line.strip()

        if not line:
            continue

        key, _, val = line.partition("=")
        val = val.strip()
        if val not in ["1", "0"]:
            raise ValueError(f'PICS {key} must have a value of 0 or 1')

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


def read_pics_from_file(path: str, endpoint: int | None = None) -> dict[str, bool]:
    """
    Reads PICS from one of three source types, auto-detected from `path`:
      * a CI-format text file (one KEY=0|1 per line),
      * a directory of PICS XML files, or
      * a zip archive of PICS XML files.

    For directory and zip inputs, top-level *.xml files are always loaded
    (device-wide codes like MCORE.*). If `endpoint` is supplied, the matching
    per-endpoint subdirectory's *.xml files are loaded too. Common naming
    conventions are accepted: `endpoint0`, `Endpoint_0`, `EP0`, `ep 0`, `0`,
    etc. (case-insensitive). Other endpoint subdirs are skipped so
    per-endpoint test checks don't see foreign clusters.

    Zip archives may store the XML files at the archive root or nested
    inside a single enclosing directory (as OS "compress folder" tools
    produce); both layouts are handled. Archive entries whose paths would
    resolve outside the extraction directory are refused.
    """
    abs_path = os.path.abspath(path)

    # Directory check comes first: a directory whose name happens to end in
    # `.zip` is still a directory, not an archive.
    if os.path.isdir(abs_path):
        return _read_pics_from_directory(abs_path, endpoint)

    # Zip archives: extract into a temp dir and reuse the directory reader.
    # Extraction (instead of streaming from the zip) keeps the "single
    # enclosing folder" and endpoint-subdir logic in one place.
    if os.path.isfile(abs_path) and zipfile.is_zipfile(abs_path):
        with tempfile.TemporaryDirectory() as tmp_dir:
            _safe_extract_zip(abs_path, tmp_dir)
            return _read_pics_from_directory(_pics_root_within(tmp_dir), endpoint)

    with open(path) as f:
        return parse_pics(f.readlines())


@dataclass
class BasePicsFacts:
    """
    Device facts that map to Base/MCORE PICS codes.

    Populated by derive_base_pics_facts_from_device_wildcard. Two consumers
    today: PICSGenerator writes these to Base.xml, and TC_IDM_10_4 asserts
    them against the supplied PICS file.

    The MCORE.COM.* transport-related PICS (WIFI / THR / ETH / WIRELESS and
    the WIFI_2P4GHZ / WIFI_5GHZ band marks) are intentionally not derived
    here: the band PICS indicate Public Action Frame support on the
    corresponding band, which is not protocol-observable from a wildcard
    read. PICSGenerator continues to derive transport bits locally until
    the test-plans cleanup PRs land.
    """
    is_commissionee: bool = False
    is_server: bool = False
    is_bridge: bool = False
    is_ota_requestor: bool = False
    is_ota_provider: bool = False
    has_groups_on_multiple_endpoints: bool = False
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