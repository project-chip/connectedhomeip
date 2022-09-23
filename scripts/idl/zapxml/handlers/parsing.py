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

from idl.matter_idl_types import *

def ParseInt(value: str) -> int:
    if value.startswith('0x'):
        return int(value[2:], 16)
    else:
        return int(value)


def AttrsToAccessPrivilege(attrs) -> AccessPrivilege:
    # XML seems to use both role and privilege to mean the same thing
    # they are used interchangeably
    if 'role' in attrs:
        role = attrs['role']
    else:
        role = attrs['privilege']

    if role.lower() == 'view':
        return AccessPrivilege.VIEW
    elif role.lower() == 'operate':
        return AccessPrivilege.OPERATE
    elif role.lower() == 'manage':
        return AccessPrivilege.MANAGE
    elif role.lower() == 'administer':
        return AccessPrivilege.ADMINISTER
    else:
        raise Exception('Unknown ACL role: %r' % role)


def AttrsToAttribute(attrs) -> Attribute:
    if attrs['type'].lower() == 'array':
        data_type = DataType(name=attrs['entryType'])
    else:
        data_type = DataType(name=attrs['type'])

    if 'length' in attrs:
        data_type.max_length = ParseInt(attrs['length'])

    field = Field(
        data_type=data_type,
        code=ParseInt(attrs['code']),
        name=None,
        is_list=(attrs['type'].lower() == 'array')
    )

    attribute = Attribute(definition=field)

    if attrs.get('optional', "false").lower() == 'true':
        attribute.definition.attributes.add(FieldAttribute.OPTIONAL)

    if attrs.get('isNullable', "false").lower() == 'true':
        attribute.definition.attributes.add(FieldAttribute.NULLABLE)

    if attrs.get('readable', "true").lower() == 'true':
        attribute.tags.add(AttributeTag.READABLE)

    if attrs.get('writable', "false").lower() == 'true':
        attribute.tags.add(AttributeTag.WRITABLE)

    # TODO: XML does not seem to contain information about
    #   - NOSUBSCRIBE

    # TODO: do we care about default value at all?
    #       General storage of default only applies to instantiation

    return attribute

