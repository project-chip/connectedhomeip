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

from idl.matter_idl_types import *
from typing import Optional, Union, List

from .context import Context, IdlPostProcessor
from .parsing import ParseInt, AttrsToAccessPrivilege, AttrsToAttribute
from .base import HandledDepth, BaseHandler


class ClusterNameHandler(BaseHandler):
    """Handles /configurator/cluster/name elements."""

    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def HandleContent(self, content):
        self._cluster.name = content.replace(' ', '')


class AttributeDescriptionHandler(BaseHandler):
    """Handles /configurator/cluster/attribute/description elements."""

    def __init__(self, context: Context, attribute: Attribute):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._attribute = attribute

    def HandleContent(self, content: str):
        self._attribute.definition.name = content.replace(' ', '')


class ClusterCodeHandler(BaseHandler):
    """Handles /configurator/cluster/code elements."""

    def __init__(self, context: Context, cluster: Cluster):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._cluster = cluster

    def HandleContent(self, content: str):
        self._cluster.code = ParseInt(content)


class EventHandler(BaseHandler):
    """Handles /configurator/cluster/event elements."""

    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context)
        self._cluster = cluster

        if attrs['priority'] == 'debug':
            priority = EventPriority.DEBUG
        elif attrs['priority'] == 'info':
            priority = EventPriority.INFO
        elif attrs['priority'] == 'critical':
            priority = EventPriority.CRITICAL
        else:
            raise Exception("Unknown event priority: %s" % attrs['priority'])

        self._event = Event(
            priority=priority,
            code=ParseInt(attrs['code']),
            name=attrs['name'],
            fields=[],
        )

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'field':
            data_type = DataType(name=attrs['type'])
            if 'length' in attrs:
                data_type.max_length = ParseInt(attrs['length'])

            field = Field(
                data_type=data_type,
                code=ParseInt(attrs['id']),
                name=attrs['name'],
                is_list=(attrs.get('array', 'false').lower() == 'true'),
            )

            if attrs.get('optional', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.OPTIONAL)

            if attrs.get('isNullable', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.NULLABLE)

            self._event.fields.append(field)
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'access':
            self._event.readacl = AttrsToAccessPrivilege(attrs)
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)

    def EndProcessing(self):
        self._cluster.events.append(self._event)


class AttributeHandler(BaseHandler):
    """Handles /configurator/cluster/attribute elements."""

    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context)
        self._cluster = cluster
        self._attribute = AttrsToAttribute(attrs)

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'access':
            if 'modifier' in attrs:
                if attrs['modifier'] != 'fabric-scoped':
                    raise Exception("UNKNOWN MODIFIER: %s" % attrs['modifier'])
                self._attribute.tags.add(AttributeTag.FABRIC_SCOPED)
            else:
                role = AttrsToAccessPrivilege(attrs)

                if attrs['op'] == 'read':
                    self._attribute.readacl = role
                elif attrs['op'] == 'write':
                    self._attribute.writeacl = role
                else:
                    logging.error("Unknown access: %r" % attrs['op'])
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return AttributeDescriptionHandler(self.context, self._attribute)
        else:
            return BaseHandler(self.context)

    def HandleContent(self, content: str):
        # Content generally is the name EXCEPT if access controls
        # exist, in which case `description` contains the name
        content = content.strip()
        if content and not self._attribute.definition.name:
            self._attribute.definition.name = content

    def EndProcessing(self):
        if self._attribute.definition.name is None:
            raise Exception("Name for attribute was not parsed.")

        self._cluster.attributes.append(self._attribute)


class StructHandler(BaseHandler, IdlPostProcessor):
    """ Handling /configurator/struct elements."""

    def __init__(self, context: Context, attrs):
        super().__init__(context)

        # if set, struct belongs to a specific cluster
        self._cluster_codes = set()
        self._struct = Struct(name=attrs['name'], fields=[])
        self._field_index = 0
        # The following are not set:
        #    - tag not set because not a request/response
        #    - code not set because not a response

        self._is_fabric_scoped = (attrs.get('isFabricScoped', "false").lower() == 'true')

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'item':
            data_type = DataType(
                name=attrs['type']
            )

            # TODO(#22938): IDL should keep track and use the
            #               isFabricSensitive attribute here

            if 'fieldId' in attrs:
                self._field_index = ParseInt(attrs['fieldId'])
            else:
                # NOTE: code does NOT exist, so the number is incremental here
                #       this seems a defficiency in XML format.
                self._field_index += 1

            if 'length' in attrs:
                data_type.max_length = ParseInt(attrs['length'])

            field = Field(
                data_type=data_type,
                code=self._field_index,
                name=attrs['name'],
                is_list=(attrs.get('array', 'false').lower() == 'true'),
            )

            if attrs.get('optional', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.OPTIONAL)

            if attrs.get('isNullable', "false").lower() == 'true':
                field.attributes.add(FieldAttribute.NULLABLE)

            self._struct.fields.append(field)
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'cluster':
            self._cluster_codes.add(ParseInt(attrs['code']))
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return BaseHandler(self.context)

    def FinalizeProcessing(self, idl: Idl):
        # We have two choices of adding a struct:
        #   - inside a cluster if a code exists
        #   - inside top level if no codes were associated
        if self._cluster_codes:
            for code in self._cluster_codes:
                found = False
                for c in idl.clusters:
                    if c.code == code:
                        c.structs.append(self._struct)
                        found = True

                if not found:
                    logging.error('Enum %s could not find cluster (code %d/0x%X)' %
                                  (self._struct.name, code, code))
        else:
            idl.structs.append(self._struct)

        # Secondary processing, for fabric scoped:
        #      while ZAP XML maintains these at a struct level. Update attributes
        if self._is_fabric_scoped:
            for cluster in idl.clusters:
                for attribute in cluster.attributes:
                    if self._struct.name == attribute.definition.data_type.name:
                        attribute.tags.add(AttributeTag.FABRIC_SCOPED)

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)


class EnumHandler(BaseHandler, IdlPostProcessor):
    """ Handling /configurator/enum elements."""

    def __init__(self, context: Context, attrs):
        super().__init__(context)
        self._cluster_code = None  # if set, enum belongs to a specific cluster
        self._enum = Enum(name=attrs['name'], base_type=attrs['type'], entries=[])

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'item':
            self._enum.entries.append(ConstantEntry(
                name=attrs['name'],
                code=ParseInt(attrs['value'])
            ))
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'cluster':
            if self._cluster_code is not None:
                raise Exception('Multiple cluster codes for enum %s' % self._enum.name)
            self._cluster_code = ParseInt(attrs['code'])
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return BaseHandler(self.context)

    def FinalizeProcessing(self, idl: Idl):
        # We have two choices of adding an enum:
        #   - inside a cluster if a code exists
        #   - inside top level if a code does not exist

        if self._cluster_code is None:
            idl.enums.append(self._enum)
        else:
            found = False
            for c in idl.clusters:
                if c.code == self._cluster_code:
                    c.enums.append(self._enum)
                    found = True

            if not found:
                logging.error('Enum %s could not find its cluster (code %d/0x%X)' %
                              (self._enum.name, self._cluster_code, self._cluster_code))

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)


class BitmapHandler(BaseHandler):
    """ Handling /configurator/bitmap elements."""

    def __init__(self, context: Context, attrs):
        super().__init__(context)
        self._cluster_codes = set()
        self._bitmap = Bitmap(name=attrs['name'], base_type=attrs['type'], entries=[])

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'cluster':
            # Multiple clusters may be associated, like IasZoneStatus
            self._cluster_codes.add(ParseInt(attrs['code']))
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'field':
            self._bitmap.entries.append(ConstantEntry(
                name=attrs['name'],
                code=ParseInt(attrs['mask'])
            ))
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)

    def FinalizeProcessing(self, idl: Idl):
        # We have two choices of adding an enum:
        #   - inside a cluster if a code exists
        #   - inside top level if a code does not exist
        if not self._cluster_codes:
            # Log only instead of critical, as not our XML is well formed.
            # For example at the time of writing this, SwitchFeature in switch-cluster.xml
            # did not have a code associated with it.
            logging.error("Bitmap %r has no cluster codes" % self._bitmap)
            return

        for code in self._cluster_codes:
            found = False
            for c in idl.clusters:
                if c.code == code:
                    c.bitmaps.append(self._bitmap)
                    found = True
            if not found:
                logging.error('Bitmap %s could not find its cluster (code %d/0x%X)' %
                              (self._bitmap.name, code, code))

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)


class CommandHandler(BaseHandler):
    """Handles /configurator/cluster/command elements."""

    def __init__(self, context: Context, cluster: Cluster, attrs):
        super().__init__(context)
        self._cluster = cluster
        self._command = None
        self._struct = Struct(name=attrs['name'], fields=[])
        self._field_index = 0  # commands DO NOT support field index it seems

        if attrs['source'].lower() == 'client':
            self._struct.tag = StructTag.REQUEST

            name = attrs['name']

            if name.endswith('Request'):
                request_name = name
                command_name = name[:-7]
            else:
                request_name = name+'Request'
                command_name = name

            self._struct.name = request_name

            if 'response' in attrs:
                response_name = attrs['response']
            else:
                response_name = 'DefaultResponse'

            self._command = Command(
                name=name,
                code=ParseInt(attrs['code']),
                input_param=request_name,
                output_param=response_name,
            )

            if attrs.get('isFabricScoped', 'false') == 'true':
                self._command.attributes.add(CommandAttribute.FABRIC_SCOPED)

            if attrs.get('mustUseTimedInvoke', 'false') == 'true':
                self._command.attributes.add(CommandAttribute.TIMED_INVOKE)

        else:
            self._struct.tag = StructTag.RESPONSE
            self._struct.code = ParseInt(attrs['code'])

    def GetArgumentField(self, attrs):
        data_type = DataType(name=attrs['type'])

        if 'length' in attrs:
            data_type.max_length = ParseInt(attrs['length'])

        self._field_index += 1

        field = Field(
            data_type=data_type,
            code=self._field_index,
            name=attrs['name'],
            is_list=(attrs.get('array', 'false') == 'true')
        )

        if attrs.get('optional', "false").lower() == 'true':
            field.attributes.add(FieldAttribute.OPTIONAL)

        if attrs.get('isNullable', "false").lower() == 'true':
            field.attributes.add(FieldAttribute.NULLABLE)

        return field

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'access':
            if attrs['op'] != 'invoke':
                raise Exception('Unknown access for %r' % self._struct)

            if self._command:
                self._command.invokeacl = AttrsToAccessPrivilege(attrs)
            else:
                logging.warning("Ignored access role for reply %r" % self._struct)
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'arg':
            self._struct.fields.append(self.GetArgumentField(attrs))
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        elif name.lower() == 'description':
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)

    def EndProcessing(self):

        if self._struct.fields:
            self._cluster.structs.append(self._struct)
        else:
            # no input
            self._command.input_param = None

        if self._command:
            self._cluster.commands.append(self._command)


class ClusterGlobalAttributeHandler(BaseHandler):
    """Handles /configurator/cluster/globalAttribute elements."""

    def __init__(self, context: Context, cluster: Cluster, code: int):
        super().__init__(context)
        self._cluster = cluster
        self._code = code

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'featurebit':
            # It is uncler what featurebits mean. likely a bitmap should be created
            # here, however only one such example exists currently: door-lock-cluster.xml
            logging.info('Ignoring featurebit tag for global attribute 0x%X (%d)' % (self._code, self._code))
            return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)
        else:
            return BaseHandler(self.context)

    def EndProcessing(self):
        self._cluster.attributes.append(self.context.GetGlobalAttribute(self._code))


class ClusterHandler(BaseHandler):
    """Handles /configurator/cluster elements."""

    def __init__(self, context: Context, idl: Idl):
        super().__init__(context)
        self._cluster = Cluster(
            side=ClusterSide.CLIENT,
            name=None,
            code=None,
            parse_meta=context.GetCurrentLocationMeta()
        )
        self._idl = idl

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'code':
            return ClusterCodeHandler(self.context, self._cluster)
        elif name.lower() == 'name':
            return ClusterNameHandler(self.context, self._cluster)
        elif name.lower() == 'attribute':
            return AttributeHandler(self.context, self._cluster, attrs)
        elif name.lower() == 'event':
            return EventHandler(self.context, self._cluster, attrs)
        elif name.lower() == 'globalattribute':
            # We ignore 'side' and 'value' since they do not seem useful
            return ClusterGlobalAttributeHandler(self.context, self._cluster, ParseInt(attrs['code']))
        elif name.lower() == 'command':
            return CommandHandler(self.context, self._cluster, attrs)
        elif name.lower() in ['define', 'description', 'domain', 'tag', 'client', 'server']:
            # NOTE: we COULD use client and server to create separate definitions
            #       of each, but the usefulness of this is unclear as the definitions are
            #       likely identical and matter has no concept of differences between the two
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)

    def EndProcessing(self):
        if self._cluster.name is None:
            raise Exception("Missing cluster name")
        elif self._cluster.code is None:
            raise Exception("Missing cluster code")

        self._idl.clusters.append(self._cluster)

# Cluster extensions have extra bits for existing clusters. Can only be loaded
# IF the underlying cluster exits


class ClusterExtensionHandler(ClusterHandler, IdlPostProcessor):
    """Handling /configurator/clusterExtension elements."""

    def __init__(self, context: Context, code: int):
        # NOTE: IDL is set to NONE so that ClusterHandler cannot
        #       inadvertently change it (it will be invalid anyway)
        super().__init__(context, None)
        self._cluster_code = code

    def EndProcessing(self):
        self.context.AddIdlPostProcessor(self)

    def FinalizeProcessing(self, idl: Idl):
        found = False
        for c in idl.clusters:
            if c.code == self._cluster_code:
                found = True

                # Append everything that can be appended
                c.enums.extend(self._cluster.enums)
                c.bitmaps.extend(self._cluster.bitmaps)
                c.events.extend(self._cluster.events)
                c.attributes.extend(self._cluster.attributes)
                c.structs.extend(self._cluster.structs)
                c.commands.extend(self._cluster.commands)

        if not found:
            logging.error('Could not extend cluster 0x%X (%d): cluster not found' %
                          (self._cluster_code, self._cluster_code))


class GlobalAttributeHandler(BaseHandler):
    """Handling configurator/global/globalAttribute elements."""

    def __init__(self, context: Context, attribute: Attribute):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._attribute = attribute

    def HandleContent(self, content: str):
        # Content generally is the name EXCEPT if access controls
        # exist, in which case `description` contains the name
        #
        # Global attributes do not currently have access controls, so this
        # case is not handled here
        content = content.strip()
        if content and not self._attribute.definition.name:
            self._attribute.definition.name = content

    def EndProcessing(self):
        if self._attribute.definition.name is None:
            raise Exception("Name for attribute was not parsed.")

        self.context.AddGlobalAttribute(self._attribute)


class GlobalHandler(BaseHandler):
    """Handling configurator/global elements."""

    def __init__(self, context: Context):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'attribute':
            if attrs['side'].lower() == 'client':
                # We expect to also have 'server' equivalent, so ignore client
                # side attributes
                logging.debug('Ignoring global client-side attribute %s' % (attrs['code']))
                return BaseHandler(self.context, handled=HandledDepth.SINGLE_TAG)

            return GlobalAttributeHandler(self.context, AttrsToAttribute(attrs))
        else:
            return BaseHandler(self.context)


class ConfiguratorHandler(BaseHandler):
    """ Handling /configurator elements."""

    def __init__(self, context: Context, idl: Idl):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._idl = idl

    def GetNextProcessor(self, name: str, attrs):
        if name.lower() == 'cluster':
            return ClusterHandler(self.context, self._idl)
        elif name.lower() == 'enum':
            return EnumHandler(self.context, attrs)
        elif name.lower() == 'struct':
            return StructHandler(self.context, attrs)
        elif name.lower() == 'bitmap':
            return BitmapHandler(self.context, attrs)
        elif name.lower() == 'domain':
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name.lower() == 'clusterextension':
            return ClusterExtensionHandler(self.context, ParseInt(attrs['code']))
        elif name.lower() == 'accesscontrol':
            # These contain operation/role/modifier and generally only contain a
            # description. These do not seem as useful to parse.
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name.lower() == 'atomic':
            # A list of types in 'chip-types'
            # Generally does not seem useful - matches a type id to a description, size and some discrete/analog flags
            #
            # Could be eventually used as a preload of types into base types, however matter idl
            # generator logic has hardcoded sizing as well.
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name.lower() == 'devicetype':
            # A list of device types in 'matter-devices.xml'
            # Useful for conformance tests, but does not seem usable for serialization logic
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        elif name.lower() == 'global':
            return GlobalHandler(self.context)
        else:
            return BaseHandler(self.context)
