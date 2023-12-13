#!/usr/bin/env python
# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os
from dataclasses import dataclass
from typing import Generator, List, Optional

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Cluster, Field, Idl, StructTag


@dataclass
class TableEntry:
    code: str                   # Encoding like ContextTag() or AnonymousTag() or similar
    name: str                   # human friendly name
    reference: Optional[str]    # reference to full name
    real_type: str              # real type
    item_type: str = 'kDefault'  # type flag for decoding


@dataclass
class Table:
    # Usable variable fully qualified name (like <Cluster>_<name>)
    full_name: str
    entries: List[TableEntry]


class ClusterTablesGenerator:
    """Handles conversion from a cluster to tables."""

    def __init__(self, cluster: Cluster):
        self.cluster = cluster
        self.known_types = set()  # all types where we create reference_to
        self.list_types = set()  # all types that require a list entry
        self.item_type_map = {
            "protocol_cluster_id": "kProtocolClusterId",
            "protocol_attribute_id": "kProtocolAttributeId",
            "protocol_command_id": "kProtocolCommandId",
            "protocol_event_id": "kProtocolEventId",

            "cluster_attribute_payload": "kProtocolPayloadAttribute",
            "cluster_command_payload": "kProtocolPayloadCommand",
            "cluster_event_payload": "kProtocolPayloadEvent",

            "protocol_binary_data": "kProtocolBinaryData",
        }

        for e in self.cluster.enums:
            self.item_type_map[e.name] = "kEnum"

        for b in self.cluster.bitmaps:
            self.item_type_map[b.name] = "kBitmap"

    def FieldEntry(self, field: Field, tag_type: str = 'ContextTag', type_override: Optional[str] = None) -> TableEntry:
        data_type_name = type_override or field.data_type.name
        type_reference = "%s_%s" % (self.cluster.name, data_type_name)

        if type_reference not in self.known_types:
            type_reference = None

        item_type = self.item_type_map.get(data_type_name, 'kDefault')

        real_type = "%s::%s" % (self.cluster.name, data_type_name)
        if field.is_list:
            real_type = real_type + "[]"
            item_type = "kList"

            if type_reference:
                self.list_types.add(type_reference)
                type_reference = type_reference + "_list_"
            else:
                type_reference = "primitive_type_list_"

        return TableEntry(
            code=f'{tag_type}({field.code})',
            name=field.name,
            reference=type_reference,
            real_type=real_type,
            item_type=item_type,
        )

    def ComputeKnownTypes(self):
        self.known_types.clear()

        for s in self.cluster.structs:
            self.known_types.add("%s_%s" % (self.cluster.name, s.name))

        # Events are structures
        for e in self.cluster.events:
            if e.fields:
                self.known_types.add("%s_%s" % (self.cluster.name, e.name))

        for e in self.cluster.enums:
            self.known_types.add("%s_%s" % (self.cluster.name, e.name))

        for b in self.cluster.bitmaps:
            self.known_types.add("%s_%s" % (self.cluster.name, b.name))

    def CommandEntries(self) -> Generator[TableEntry, None, None]:
        # yield entries for every command input
        for c in self.cluster.commands:
            if c.input_param:
                yield TableEntry(
                    name=c.name,
                    code=f'CommandTag({c.code})',
                    reference="%s_%s" % (
                        self.cluster.name, c.input_param),
                    real_type="%s::%s::%s" % (
                        self.cluster.name, c.name, c.input_param)
                )
            else:
                yield TableEntry(
                    name=c.name,
                    code=f'CommandTag({c.code})',
                    reference=None,
                    real_type="%s::%s::()" % (
                        self.cluster.name, c.name)
                )

        # yield entries for every command output. We use "respons struct"
        # for this to figure out where to tag IDs from.
        for c in self.cluster.structs:
            if c.tag != StructTag.RESPONSE:
                continue
            yield TableEntry(
                name=c.name,
                code=f'CommandTag({c.code})',
                reference="%s_%s" % (
                    self.cluster.name, c.name),
                real_type="%s::%s" % (self.cluster.name, c.name),
            )

    def GenerateTables(self) -> Generator[Table, None, None]:
        self.ComputeKnownTypes()

        cluster_feature_map = None
        for b in self.cluster.bitmaps:
            # Older matter files use `ClusterNameFeature` as naming, newer code was
            # updated to just `Feature`. For now support both.
            if b.name in {'Feature', f'{self.cluster.name}Feature'} and b.base_type.lower() == 'bitmap32':
                cluster_feature_map = b.name

        # Clusters have attributes. They are direct descendants for
        # attributes
        cluster_entries = []
        cluster_entries.extend([
            self.FieldEntry(a.definition, tag_type='AttributeTag',
                            type_override=(cluster_feature_map if a.definition.code == 0xFFFC else None))
            for a in self.cluster.attributes
        ])

        cluster_entries.extend([
            # events always reference an existing struct
            TableEntry(
                code=f'EventTag({e.code})',
                name=e.name,
                reference="%s_%s" % (self.cluster.name, e.name),
                real_type='%s::%s' % (self.cluster.name, e.name)
            )
            for e in self.cluster.events if e.fields
        ])
        cluster_entries.extend(
            [entry for entry in self.CommandEntries()]
        )

        yield Table(
            full_name=self.cluster.name,
            entries=cluster_entries,
        )

        for s in self.cluster.structs:
            yield Table(
                full_name="%s_%s" % (self.cluster.name, s.name),
                entries=[self.FieldEntry(field) for field in s.fields]
            )

        for e in self.cluster.events:
            if e.fields:
                yield Table(
                    full_name="%s_%s" % (self.cluster.name, e.name),
                    entries=[self.FieldEntry(field) for field in e.fields]
                )

        # some items have lists, create an intermediate item for those
        for name in self.list_types:
            yield Table(
                full_name="%s_list_" % name,
                entries=[
                    TableEntry(
                        code="AnonymousTag()",
                        name="Anonymous<>",
                        reference=name,
                        real_type="%s[]" % name,
                    )
                ]
            )

        for e in self.cluster.enums:
            yield Table(
                full_name="%s_%s" % (self.cluster.name, e.name),
                entries=[
                    TableEntry(
                        code="ConstantValueTag(0x%X)" % entry.code,
                        name=entry.name,
                        reference=None,
                        real_type="%s::%s::%s" % (
                            self.cluster.name, e.name, entry.name)
                    )
                    for entry in e.entries
                ]
            )

        for e in self.cluster.bitmaps:
            yield Table(
                full_name="%s_%s" % (self.cluster.name, e.name),
                entries=[
                    TableEntry(
                        code="ConstantValueTag(0x%X)" % entry.code,
                        name=entry.name,
                        reference=None,
                        real_type="%s::%s::%s" % (
                            self.cluster.name, e.name, entry.name)
                    )
                    for entry in e.entries
                ]
            )


def CreateTables(idl: Idl) -> List[Table]:
    result = []
    for cluster in idl.clusters:
        result.extend(
            [table for table in ClusterTablesGenerator(cluster).GenerateTables()])

    return result


def IndexInTable(name: Optional[str], table: List[Table]) -> str:
    """Find the index of the given name in the table.

    The index is 1-based (to allow for a first entry containing a
    starting point for the app)
    """
    if not name:
        return "kInvalidNodeIndex"

    if name == "primitive_type_list_":
        return "1"

    for idx, t in enumerate(table):
        if t.full_name == name:
            # Index skipping hard-coded items
            return "%d" % (idx + 2)

    raise Exception("Name %r not found in table" % name)


class TLVMetaDataGenerator(CodeGenerator):
    """
    Generation of cpp code containing TLV metadata information.

    Epecting extra option for constant naming

    Example execution via codegen.py:

      ./scripts/codegen.py                        \
          --output-dir out/metaexample            \
          --generator cpp-tlvmeta                 \
          --option table_name:protocols_meta      \
          src/lib/format/protocol_messages.matter
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, table_name: str = "clusters_meta", **kargs):
        super().__init__(storage, idl, fs_loader_searchpath=os.path.dirname(__file__))
        self.table_name = table_name
        self.jinja_env.filters['indexInTable'] = IndexInTable

    def internal_render_all(self):
        """
        Renders the cpp and header files required for applications
        """

        tables = CreateTables(self.idl)

        self.internal_render_one_output(
            template_path="TLVMetaData_cpp.jinja",
            output_file_name=f"tlv/meta/{self.table_name}.cpp",
            vars={
                'clusters': self.idl.clusters,
                'table_name': self.table_name,
                'sub_tables': tables,
            }
        )

        self.internal_render_one_output(
            template_path="TLVMetaData_h.jinja",
            output_file_name=f"tlv/meta/{self.table_name}.h",
            vars={
                'clusters': self.idl.clusters,
                'table_name': self.table_name,
                'sub_tables': tables,
            }
        )
