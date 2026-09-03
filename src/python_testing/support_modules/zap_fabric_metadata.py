#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Fabric markers read from the ZAP source XMLs in
``src/app/zap-templates/zcl/data-model/chip``.

These XMLs are what the C++ code generator consumes, so they decide what the
device actually does at runtime: ``isFabricScoped`` on a struct emits the
``FabricIndex`` field and the ``EncodeForRead`` entry point, and
``isFabricSensitive`` on a struct item emits the ``includeSensitive`` gate that
omits that field from a cross-fabric read.

The spec data model XMLs parsed by ``matter.testing.spec_parsing`` say what the
device is *supposed* to do. Comparing the two is what
``test_testing/TestFabricMetadataConsistency.py`` does; this module supplies the
ZAP half. It is deliberately a direct ElementTree walk rather than a route
through matter_idl, whose ``AttributeQuality`` has no fabric member at all.
"""

import logging
import xml.etree.ElementTree as ElementTree
from dataclasses import dataclass, field
from pathlib import Path

log = logging.getLogger(__name__)

# Repository-relative location of the ZAP source XMLs, resolved from this file so
# the module works regardless of the caller's working directory.
ZAP_DATA_MODEL_DIRECTORY = Path(__file__).resolve().parents[2] / 'app' / 'zap-templates' / 'zcl' / 'data-model' / 'chip'


def _is_true(element: ElementTree.Element, attribute_name: str) -> bool:
    """Whether the XML attribute is present and set to "true"."""
    return element.attrib.get(attribute_name) == 'true'


def _parse_id(raw: str | None) -> int | None:
    """Parse a ZAP id, which may be decimal or 0x-prefixed. None when unparseable.

    ZAP XMLs are hand-edited, so a malformed id is treated as an absent element
    rather than raising: the audit that consumes this reports what it could not
    check instead of failing to run at all.
    """
    if raw is None:
        return None
    try:
        return int(raw, 0)
    except ValueError:
        return None


@dataclass(frozen=True)
class ZapStruct:
    """Fabric markers on one ZAP ``<struct>``."""
    name: str
    # isFabricScoped on the struct: codegen adds the FabricIndex field and the
    # fabric-aware EncodeForRead/DoEncode pair.
    fabric_scoped: bool
    # Field ids whose <item> carries isFabricSensitive, i.e. the fields codegen
    # omits when the entry belongs to another fabric.
    sensitive_field_ids: frozenset[int]
    # Every field id on the struct, so a caller can ask "which fields are *not*
    # masked" without re-reading the XML.
    field_ids: frozenset[int]


@dataclass(frozen=True)
class ZapCluster:
    """Fabric markers on one ZAP ``<cluster>``."""
    name: str
    cluster_id: int
    # isFabricSensitive on an <attribute>. ZAP has no isFabricScoped on
    # attributes at all; scoping is inferred through the entry struct.
    fabric_sensitive_attribute_ids: frozenset[int]
    # isFabricScoped on a <command source="client">. Response commands are
    # excluded: they are not invoked, so the quality does not apply to them.
    fabric_scoped_command_ids: frozenset[int]
    # isFabricSensitive on an <event>.
    fabric_sensitive_event_ids: frozenset[int]
    # Attribute id to the name in its entryType, for list attributes only. Used
    # to reach the entry struct of an attribute the spec marks fabric sensitive.
    entry_type_by_attribute_id: dict[int, str] = field(default_factory=dict)


@dataclass(frozen=True)
class ZapFabricMetadata:
    """Everything the audit needs from the ZAP XMLs, parsed once."""
    clusters: dict[int, ZapCluster]
    # Keyed by (cluster_id, struct name); cluster_id is None for a global struct
    # such as WebRTCSessionStruct in global-structs.xml. Struct names are not
    # unique across clusters, so the key has to carry the cluster.
    structs: dict[tuple[int | None, str], ZapStruct]

    def struct(self, cluster_id: int | None, name: str) -> ZapStruct | None:
        """Look up a struct by name, falling back to the global structs.

        A cluster referencing a struct it does not define is referencing a global
        one, so the cluster-scoped lookup is tried first and the global second.
        """
        cluster_scoped = self.structs.get((cluster_id, name))
        if cluster_scoped is not None:
            return cluster_scoped
        return self.structs.get((None, name))


def _parse_struct(element: ElementTree.Element) -> tuple[int | None, ZapStruct] | None:
    """Parse one <struct>, returning its (cluster_id, ZapStruct) or None if unusable."""
    name = element.attrib.get('name')
    if name is None:
        return None

    # A struct is associated with a cluster by a <cluster code="0x..."/> child.
    # Structs with no such child are global. A struct shared by several clusters
    # carries one child per cluster; the first is enough to key it, because the
    # markers themselves are properties of the struct, not of the association.
    cluster_reference = element.find('cluster')
    cluster_id = None
    if cluster_reference is not None:
        cluster_id = _parse_id(cluster_reference.attrib.get('code'))

    sensitive_field_ids = set()
    field_ids = set()
    for item in element.findall('item'):
        field_id = _parse_id(item.attrib.get('fieldId'))
        if field_id is None:
            continue
        field_ids.add(field_id)
        if _is_true(item, 'isFabricSensitive'):
            sensitive_field_ids.add(field_id)

    return cluster_id, ZapStruct(name=name,
                                 fabric_scoped=_is_true(element, 'isFabricScoped'),
                                 sensitive_field_ids=frozenset(sensitive_field_ids),
                                 field_ids=frozenset(field_ids))


def _parse_cluster(element: ElementTree.Element) -> ZapCluster | None:
    """Parse one top-level <cluster>, returning None when it carries no usable id."""
    cluster_id = _parse_id(element.findtext('code'))
    if cluster_id is None:
        return None

    fabric_sensitive_attribute_ids = set()
    entry_type_by_attribute_id = {}
    for attribute in element.findall('attribute'):
        attribute_id = _parse_id(attribute.attrib.get('code'))
        if attribute_id is None:
            continue
        if _is_true(attribute, 'isFabricSensitive'):
            fabric_sensitive_attribute_ids.add(attribute_id)
        entry_type = attribute.attrib.get('entryType')
        if entry_type is not None:
            entry_type_by_attribute_id[attribute_id] = entry_type

    fabric_scoped_command_ids = set()
    for command in element.findall('command'):
        # Only client-to-server commands are invoked, so only they can be
        # fabric scoped; a response command carrying the marker would be a
        # separate kind of mistake and is not this audit's subject.
        if command.attrib.get('source') != 'client':
            continue
        command_id = _parse_id(command.attrib.get('code'))
        if command_id is not None and _is_true(command, 'isFabricScoped'):
            fabric_scoped_command_ids.add(command_id)

    fabric_sensitive_event_ids = set()
    for event in element.findall('event'):
        event_id = _parse_id(event.attrib.get('code'))
        if event_id is not None and _is_true(event, 'isFabricSensitive'):
            fabric_sensitive_event_ids.add(event_id)

    return ZapCluster(name=(element.findtext('name') or '').strip(),
                      cluster_id=cluster_id,
                      fabric_sensitive_attribute_ids=frozenset(fabric_sensitive_attribute_ids),
                      fabric_scoped_command_ids=frozenset(fabric_scoped_command_ids),
                      fabric_sensitive_event_ids=frozenset(fabric_sensitive_event_ids),
                      entry_type_by_attribute_id=entry_type_by_attribute_id)


def build_zap_fabric_metadata(directory: Path = ZAP_DATA_MODEL_DIRECTORY) -> ZapFabricMetadata:
    """Parse every ZAP XML in the directory and collect its fabric markers.

    A file that fails to parse is logged and skipped rather than raising: the
    audit is more useful reporting on the files it could read than refusing to
    run because one unrelated XML is malformed.
    """
    clusters: dict[int, ZapCluster] = {}
    structs: dict[tuple[int | None, str], ZapStruct] = {}

    for path in sorted(directory.glob('*.xml')):
        try:
            root = ElementTree.parse(path).getroot()
        except ElementTree.ParseError as e:
            log.warning("Skipping %s: %s", path.name, e)
            continue

        for element in root.findall('struct'):
            parsed = _parse_struct(element)
            if parsed is None:
                continue
            cluster_id, struct = parsed
            structs[(cluster_id, struct.name)] = struct

        # findall rather than iter: <cluster code="0x..."/> also appears inside
        # <struct> as an association reference, and iter would match those too.
        for element in root.findall('cluster'):
            cluster = _parse_cluster(element)
            if cluster is not None:
                clusters[cluster.cluster_id] = cluster

    return ZapFabricMetadata(clusters=clusters, structs=structs)
