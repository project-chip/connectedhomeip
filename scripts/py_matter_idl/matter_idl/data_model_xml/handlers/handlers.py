# Copyright (c) 2022 Project CHIP Authors
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

import logging
from typing import Any, Optional

from matter_idl.matter_idl_types import (Attribute, Bitmap, Cluster, ClusterSide, Command, CommandQuality, ConstantEntry, DataType,
                                         Enum, Event, EventPriority, EventQuality, Field, FieldQuality, Idl, Struct, StructQuality,
                                         StructTag)

from .base import BaseHandler, HandledDepth
from .context import Context, IdlPostProcessor
from .parsing import NormalizeName, ParseInt

LOGGER = logging.getLogger('data-model-xml-parser')


class FeaturesHandler(BaseHandler):

    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster
        self._bitmap = Bitmap(name="Feature", base_type="bitmap32", entries=[])

    def EndProcessing(self):
        if self._bitmap.entries:
            self._cluster.bitmaps.append(self._bitmap)

    def GetNextProcessor(self, name: str, attrs):
        if name in {"section", "optionalConform"}:
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "feature":
            assert ("bit" in attrs)
            assert ("name" in attrs)

            self._bitmap.entries.append(ConstantEntry(
                name="k" + NormalizeName(attrs["name"]),
                code=1 << ParseInt(attrs["bit"])
            ))

            # assume everything handled. Sub-item is only section
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)


class BitmapHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

        # TODO: base type is GUESSED here because xml does not contain it
        self._bitmap = Bitmap(name=NormalizeName(
            attrs["name"]), base_type="bitmap32", entries=[])

    def EndProcessing(self):
        if self._bitmap.entries:
            self._cluster.bitmaps.append(self._bitmap)

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "bitfield":
            assert ("name" in attrs)
            assert ("bit" in attrs)
            self._bitmap.entries.append(
                ConstantEntry(
                    name="k" + NormalizeName(attrs["name"]), code=1 << ParseInt(attrs["bit"]))
            )
            # Assume fully handled. We do not parse "mandatoryConform and such"
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)

class StructHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

        self._struct = Struct(name=NormalizeName(attrs["name"]), fields=[])

    def EndProcessing(self):
        self._cluster.structs.append(self._struct)

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "field":
            assert ("name" in attrs)
            assert ("id" in attrs)
            assert ("type" in attrs)

            # TODO:
            #   - is_list would be useful here
            #   - qualities (NULLABLE, OPTIONAL, FABRIC_SENSITIVE)
            #   - data type: min/max length and min/max value
            self._struct.fields.append(Field(
                name=NormalizeName(attrs["name"]),
                code=ParseInt(attrs["id"]),
                data_type=DataType(name=attrs["type"])
            ))


            # Assume fully handled.
            #
            # TODO: "access" is unclear here. mandatoryConform and section is probably
            #       not needed in general
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)


class EnumHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

        # TODO: base type is GUESSED here because xml does not contain it
        self._enum = Enum(name=NormalizeName(
            attrs["name"]), base_type="enum32", entries=[])

    def EndProcessing(self):
        if self._enum.entries:
            self._cluster.enums.append(self._enum)

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "item":
            assert ("name" in attrs)
            assert ("value" in attrs)
            self._enum.entries.append(
                ConstantEntry(
                    name="k" + NormalizeName(attrs["name"]), code=ParseInt(attrs["value"]))
            )
            # Assume fully handled
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)


class DataTypesHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "number":
            # Seems like a documentation of a number format
            #
            # TODO: actually ensure this has no meaning
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "enum":
            return EnumHandler(self.context, self._cluster, attrs)
        elif name == "bitmap":
            return BitmapHandler(self.context, self._cluster, attrs)
        elif name == "struct":
            return StructHandler(self.context, self._cluster, attrs)
        else:
            return BaseHandler(self.context)


class ClusterHandler(BaseHandler):
    """ Handling /cluster elements."""

    def __init__(self, context: Context, idl: Idl, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._idl = idl

        assert ("name" in attrs)
        assert ("id" in attrs)

        self._cluster = Cluster(
            side=ClusterSide.CLIENT,
            name=NormalizeName(attrs["name"]),
            code=ParseInt(attrs["id"]),
            parse_meta=context.GetCurrentLocationMeta()
        )

        # Global things MUST be available everywhere
        to_add = [
            # type, code, name, is_list
            ('attrib_id', 65531, 'attributeList', True),
            ('event_id', 65530, 'eventList', True),
            ('command_id', 65529, 'acceptedCommandList', True),
            ('command_id', 65528, 'generatedCommandList', True),
            ('bitmap32', 65532, 'featureMap', False),
            ('int16u', 65533, 'clusterRevision', False),
        ]
        for data_type, code, name, is_list in to_add:
            self._cluster.attributes.append(Attribute(definition=Field(
                data_type=DataType(name=data_type), code=code, name=name, is_list=is_list)))

    def EndProcessing(self):
        self._idl.clusters.append(self._cluster)

    def GetNextProcessor(self, name: str, attrs):
        # TODO:
        #   - dataTypes (enum, bitmap, list???, struct)
        #   - attributes
        #   - commands
        #   - events
        #   - ??? classification
        #   - ??? identifiers
        if name == "revisionHistory":
            # Revision history COULD be used to find the latest revision of a cluster
            # however current IDL files do NOT have a revision info field
            #
            # NOTE: we COULD set this as a `default` for attribute clusterRevision, however this will likely
            #       not match with what matter IDL would parse into.
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "classification":
            # Not an obvious u
            # derived could have meaning
            #
            # TODO IFF hierarchy == derived, we should use baseCluster
            #
            # Other elements like role, picsCode, scope and primaryTransaction seem
            # to not be used
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "features":
            return FeaturesHandler(self.context, self._cluster)
        elif name == "dataTypes":
            return DataTypesHandler(self.context, self._cluster)
        else:
            return BaseHandler(self.context)
