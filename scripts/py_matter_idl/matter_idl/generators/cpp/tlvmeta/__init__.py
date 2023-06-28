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
from typing import List, Optional, Generator
from dataclasses import dataclass

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Cluster, ClusterSide, Idl, Field

@dataclass
class TableEntry:
    code: int                # Unique code for the entry
    name: str                # human friendly name
    reference: Optional[str] # reference to full name


@dataclass
class Table:
    full_name: str            # Usable variable fully qualified name (like <Cluster>_<name>)
    entries: List[TableEntry]


class ClusterTablesGenerator:
    """Handles conversion from a cluster to tables."""
    def __init__(self, cluster: Cluster):
        self.cluster = cluster
        self.known_types = set()

    def FieldEntry(self, field: Field) -> TableEntry:
        type_reference = "%s_%s" % (self.cluster.name, field.data_type.name)
        if type_reference not in self.known_types:
            type_reference = None

        # FIXME: implement
        return TableEntry(
            code=field.code,
            name=field.name,
            reference=type_reference
        )

    def ComputeKnownTypes(self):
        self.known_types.clear()

        for s in self.cluster.structs:
            self.known_types.add("%s_%s" % (self.cluster.name, s.name))

    def GenerateTables(self) -> Generator[Table, None, None]:
        self.ComputeKnownTypes()

        for s in self.cluster.structs:
            yield Table(
                full_name="%s_%s" % (self.cluster.name, s.name),
                entries = [self.FieldEntry(field) for field in s.fields]
            )


        # TODO: attributes (also structs)
        # TODO: commands (also structs)
        # TODO: events (also structs)
        #
        # TODO: enums, bitmaps, lists



def ClusterToTables(cluster: Cluster) -> List[Table]:

    cluster_table = Table(full_name=cluster.name, entries=[
        TableEntry(code=1, name="attributes", reference="%s_attributes"),
        TableEntry(code=2, name="commands", reference="%s_commands"),
        TableEntry(code=3, name="events", reference="%s_events"),
    ])
    result = [cluster_table]
    result.extend([ table for table in ClusterTablesGenerator(cluster).GenerateTables()])

    return result

def CreateTables(idl: Idl) -> List[Table]:
    result = []
    for cluster in idl.clusters:
        result.extend(ClusterToTables(cluster))

    # TODO: sort by usage

    return result


def IndexInTable(name: Optional[str], table: List[Table]) -> str:
    """Find the index of the given name in the table.

    The index is 1-based (to allow for a first entry containing a
    starting point for the app)
    """
    if not name:
        return "kInvalidNodeIndex"

    for idx, t in enumerate(table):
        if t.full_name == name:
            return idx + 1

    # raise Exception("Name %r not found in table" % name)
    return "kFIXME: %s" % name


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

    def __init__(self, storage: GeneratorStorage, idl: Idl, table_name:str = "clusters_meta", **kargs):
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
            output_file_name="tlv/ClusterMetaData.cpp",
            vars={
                'clusters': [c for c in self.idl.clusters if c.side == ClusterSide.CLIENT],
                'table_name': self.table_name,
                'sub_tables': tables,

            }
        )

        self.internal_render_one_output(
            template_path="TLVMetaData_h.jinja",
            output_file_name="tlv/ClusterMetaData.h",
            vars={
                'clusters': [c for c in self.idl.clusters if c.side == ClusterSide.CLIENT],
                'table_name': self.table_name,
                'sub_tables': tables,
            }
        )
