# Copyright (c) 2023 Project CHIP
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

from matter_idl.matter_idl_types import (Attribute, AttributeQuality, Bitmap, Cluster, ClusterSide, CommandQuality, ConstantEntry,
                                         DataType, Enum, Field, FieldQuality, Idl, Struct, StructTag)

from .base import BaseHandler, HandledDepth
from .context import Context
from .parsing import (ApplyConstraint, AttributesToAttribute, AttributesToBitFieldConstantEntry, AttributesToCommand,
                      AttributesToEvent, AttributesToField, NormalizeDataType, NormalizeName, ParseInt, StringToAccessPrivilege)

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
            self._bitmap.entries.append(
                AttributesToBitFieldConstantEntry(attrs))
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
            attrs["name"]), base_type="UNKNOWN", entries=[])

    def EndProcessing(self):
        if not self._bitmap.entries:
            return

        # try to find the best size that fits
        # TODO: this is a pure heuristic. XML containing this would be better.
        acceptable = {8, 16, 32}
        for entry in self._bitmap.entries:
            if entry.code > 0xFF:
                acceptable.remove(8)
            if entry.code > 0xFFFF:
                acceptable.remove(16)

        if 8 in acceptable:
            self._bitmap.base_type = "bitmap8"
        elif 16 in acceptable:
            self._bitmap.base_type = "bitmap16"
        else:
            self._bitmap.base_type = "bitmap32"

        self._cluster.bitmaps.append(self._bitmap)

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "bitfield":
            self._bitmap.entries.append(
                AttributesToBitFieldConstantEntry(attrs))
            # Assume fully handled. We do not parse "mandatoryConform and such"
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)


class MandatoryConfirmFieldHandler(BaseHandler):
    def __init__(self, context: Context, field: Field):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._field = field
        self._hadConditions = False

    def GetNextProcessor(self, name: str, attrs):
        self._hadConditions = True
        return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)

    def EndProcessing(self):
        # A mandatory conformance with conditions means it is
        # optional in some cases
        if self._hadConditions:
            self._field.qualities |= FieldQuality.OPTIONAL


class FieldHandler(BaseHandler):
    def __init__(self, context: Context, field: Field):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._field = field

    def GetNextProcessor(self, name: str, attrs):
        if name == "constraint":
            ApplyConstraint(attrs, self._field)
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name == "mandatoryConform":
            return MandatoryConfirmFieldHandler(self.context, self._field)
        elif name == "optionalConform":
            self._field.qualities |= FieldQuality.OPTIONAL
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "access":
            # per-field access is not something we model
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name == "quality":
            if "nullable" in attrs and attrs["nullable"] != "false":
                self._field.qualities = self._field.qualities | FieldQuality.NULLABLE
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name == "enum":
            LOGGER.warning(
                f"Anonymous enumeration not supported when handling field {self._field.name}")
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "bitmap":
            LOGGER.warning(
                f"Anonymous bitmap not supported when handling field {self._field.name}")
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "entry":
            # Lists have "type=list" and then the type is inside entry

            if self._field.data_type.name != "list":
                LOGGER.warning(
                    f"Entry type provided for non-list element {self._field.name}")

            assert "type" in attrs

            self._field.is_list = True
            self._field.data_type.name = NormalizeDataType(attrs["type"])

            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
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
            field = AttributesToField(attrs)
            self._struct.fields.append(field)
            return FieldHandler(self.context, field)
        else:
            return BaseHandler(self.context)


class EventHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster
        self._event = AttributesToEvent(attrs)

    def EndProcessing(self):
        self._cluster.events.append(self._event)

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "field":
            field = AttributesToField(attrs)
            self._event.fields.append(field)
            return FieldHandler(self.context, field)
        elif name == "mandatoryConform":
            # assume handled (we do not record conformance in IDL)
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "access":
            if "readPrivilege" in attrs:
                self._event.readacl = StringToAccessPrivilege(
                    attrs["readPrivilege"])
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return BaseHandler(self.context)


class EnumHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

        # TODO: base type is GUESSED here because xml does not contain it
        self._enum = Enum(name=NormalizeName(
            attrs["name"]), base_type="UNKNOWN", entries=[])

    def EndProcessing(self):
        if not self._enum.entries:
            return

        # try to find the best enum size that fits out of enum8, enum32 and enum32
        # TODO: this is a pure heuristic. XML containing this would be better.
        acceptable = {8, 16, 32}
        for entry in self._enum.entries:
            if entry.code > 0xFF:
                acceptable.remove(8)
            if entry.code > 0xFFFF:
                acceptable.remove(16)

        if 8 in acceptable:
            self._enum.base_type = "enum8"
        elif 16 in acceptable:
            self._enum.base_type = "enum16"
        else:
            self._enum.base_type = "enum32"

        self._cluster.enums.append(self._enum)

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "item":
            for key in ["name", "value"]:
                if key not in attrs:
                    logging.error("Enumeration %s entry is missing a '%s' entry (at %r)",
                                  self._enum.name, key, self.context.GetCurrentLocationMeta())
                    # bad entry, nothing I can do about it.
                    return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)

            self._enum.entries.append(
                ConstantEntry(
                    name="k" + NormalizeName(attrs["name"]), code=ParseInt(attrs["value"]))
            )
            # Assume fully handled
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)


class EventsHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def GetNextProcessor(self, name: str, attrs):
        if name == "section":
            # Documentation data, skipped
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "event":
            return EventHandler(self.context, self._cluster, attrs)
        else:
            return BaseHandler(self.context)


class AttributeHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster
        self._attribute = AttributesToAttribute(attrs)
        self._deprecated = False

    def EndProcessing(self):
        if self._deprecated:
            # Deprecation skips processing
            return

        self._cluster.attributes.append(self._attribute)

    def GetNextProcessor(self, name: str, attrs):
        if name == "enum":
            LOGGER.warning(
                f"Anonymous enumeration not supported when handling attribute {self._cluster.name}::{self._attribute.definition.name}")
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "bitmap":
            LOGGER.warning(
                f"Anonymous bitmap not supported when handling attribute {self._cluster.name}::{self._attribute.definition.name}")
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "access":
            if "readPrivilege" in attrs:
                self._attribute.readacl = StringToAccessPrivilege(
                    attrs["readPrivilege"])

            if "writePrivilege" in attrs:
                self._attribute.writeacl = StringToAccessPrivilege(
                    attrs["writePrivilege"])

            if "read" in attrs and attrs["read"] != "false":
                self._attribute.qualities = self._attribute.qualities | AttributeQuality.READABLE

            if "write" in attrs and attrs["write"] != "false":
                self._attribute.qualities = self._attribute.qualities | AttributeQuality.WRITABLE

            if "timed" in attrs and attrs["timed"] != "false":
                self._attribute.qualities = self._attribute.qualities | AttributeQuality.TIMED_WRITE
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name == "quality":
            # Out of the many interesting bits, only "nullable" seems relevant for codegen
            if "nullable" in attrs and attrs["nullable"] != "false":
                self._attribute.definition.qualities |= FieldQuality.NULLABLE
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name == "optionalConform":
            self._attribute.definition.qualities |= FieldQuality.OPTIONAL
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "mandatoryConform":
            return MandatoryConfirmFieldHandler(self.context, self._attribute.definition)
        elif name == "deprecateConform":
            self._deprecated = True
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "constraint":
            ApplyConstraint(attrs, self._attribute.definition)
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return BaseHandler(self.context)


class AttributesHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def GetNextProcessor(self, name: str, attrs):
        if name == "attribute":
            return AttributeHandler(self.context, self._cluster, attrs)
        else:
            return BaseHandler(self.context)


class CommandHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

        # Command information layout:
        #   "response":
        #       - is mandatory for "requests" and contains
        #         'Y' for default response and something else
        #         for non-default responses
        #   "direction":
        #       - sometimes missing (seems to just be request to client)
        #       - "commandToClient"
        #       - "responseFromServer"
        #

        # Heuristic logic of direction:
        #   - if we have a response, this must be a request
        #   - if direction is "commandToClient" it should be a request
        #   - if direction is "responseFromServer" it should be a response
        # otherwise guess

        if "response" in attrs:
            is_command = True
        elif ("direction" in attrs) and attrs["direction"] == "commandToClient":
            is_command = True
        elif ("direction" in attrs) and attrs["direction"] == "responseFromServer":
            is_command = False  # response
        else:
            LOGGER.warn("Could not clearly determine command direction: %s" %
                        [item for item in attrs.items()])
            # Do a best-guess. However we should NOT need to guess once
            # we have a good data set
            is_command = not attrs["name"].endswith("Response")

        if is_command:
            self._command = AttributesToCommand(attrs)
            self._struct = Struct(name=NormalizeName(attrs["name"] + "Request"),
                                  fields=[],
                                  tag=StructTag.REQUEST,
                                  )
        else:
            self._command = None
            self._struct = Struct(
                name=NormalizeName(attrs["name"]),
                fields=[],
                code=ParseInt(attrs["id"]),
                tag=StructTag.RESPONSE,
            )

    def EndProcessing(self):
        if self._struct and self._struct.fields:
            # A valid structure exists ...
            self._cluster.structs.append(self._struct)

            if self._command:
                # Input structure is well defined, set it
                self._command.input_param = self._struct.name

        if self._command:
            self._cluster.commands.append(self._command)

    def GetNextProcessor(self, name: str, attrs):
        if name in {"mandatoryConform", "optionalConform", "disallowConform"}:
            # Unclear how commands may be optional or mandatory
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name == "access":
            # <access invokePrivilege="admin" timed="true"/>
            if "invokePrivilege" in attrs:
                if self._command:
                    self._command.invokeacl = StringToAccessPrivilege(
                        attrs["invokePrivilege"])
                else:
                    LOGGER.warn(
                        f"Ignoring invoke privilege for {self._struct.name}")

            if "timed" in attrs and attrs["timed"] != "false":
                self._command.qualities |= CommandQuality.TIMED_INVOKE

            if "fabricScoped" in attrs and attrs["fabricScoped"] != "false":
                self._command.qualities |= CommandQuality.FABRIC_SCOPED

            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name == "field":
            field = AttributesToField(attrs)
            self._struct.fields.append(field)
            return FieldHandler(self.context, field)
        else:
            return BaseHandler(self.context)


class CommandsHandler(BaseHandler):
    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def GetNextProcessor(self, name: str, attrs):
        if name == "command":
            return CommandHandler(self.context, self._cluster, attrs)
        elif name in {"mandatoryConform", "optionalConform"}:
            # Nothing to tag conformance
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

    def EndProcessing(self):
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
                data_type=DataType(name=data_type),
                code=code,
                name=name,
                is_list=is_list,
            ), qualities=AttributeQuality.READABLE))
        self._idl.clusters.append(self._cluster)

    def GetNextProcessor(self, name: str, attrs):
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
            # Not an obvious mapping in the existing data model.
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
        elif name == "events":
            return EventsHandler(self.context, self._cluster)
        elif name == "attributes":
            return AttributesHandler(self.context, self._cluster)
        elif name == "commands":
            return CommandsHandler(self.context, self._cluster)
        else:
            return BaseHandler(self.context)
