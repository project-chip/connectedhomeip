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
from typing import Union

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import (AccessPrivilege, ApiMaturity, Attribute, AttributeQuality, AttributeStorage, Command,
                                         CommandQuality, Event, EventPriority, EventQuality, FieldQuality, Idl, StructQuality,
                                         StructTag)


def human_text_string(value: Union[StructTag, StructQuality, EventPriority, EventQuality, AccessPrivilege, AttributeQuality, CommandQuality, ApiMaturity, AttributeStorage]) -> str:
    if type(value) is StructTag:
        if value == StructTag.REQUEST:
            return "request"
        if value == StructTag.RESPONSE:
            return "response"
    elif type(value) is FieldQuality:
        # mypy seems confused if using `FieldQuality.OPTIONAL in value`
        # directly, so do a useless cast here
        quality: FieldQuality = value

        result = ""
        if FieldQuality.OPTIONAL in quality:
            result += "optional "
        if FieldQuality.NULLABLE in quality:
            result += "nullable "
        if FieldQuality.FABRIC_SENSITIVE in quality:
            result += "fabric_sensitive "
        return result.strip()
    elif type(value) is StructQuality:
        result = ""
        if value == StructQuality.FABRIC_SCOPED:
            result += "fabric_scoped "
        return result.strip()
    elif type(value) is EventPriority:
        if value == EventPriority.DEBUG:
            return "debug"
        if value == EventPriority.INFO:
            return "info"
        if value == EventPriority.CRITICAL:
            return "critical"
    elif type(value) is EventQuality:
        result = ""
        if EventQuality.FABRIC_SENSITIVE in value:
            result += "fabric_sensitive "
        return result.strip()
    elif type(value) is AccessPrivilege:
        if value == AccessPrivilege.VIEW:
            return "view"
        if value == AccessPrivilege.OPERATE:
            return "operate"
        if value == AccessPrivilege.MANAGE:
            return "manage"
        if value == AccessPrivilege.ADMINISTER:
            return "administer"
    elif type(value) is AttributeQuality:
        result = ""
        if AttributeQuality.TIMED_WRITE in value:
            result += "timedwrite "
        if AttributeQuality.WRITABLE not in value:
            result += "readonly "
        if AttributeQuality.NOSUBSCRIBE in value:
            result += "nosubscribe "
        return result
    elif type(value) is CommandQuality:
        result = ""
        if CommandQuality.FABRIC_SCOPED in value:
            result += "fabric "
        if CommandQuality.TIMED_INVOKE in value:
            result += "timed "
        return result
    elif type(value) is ApiMaturity:
        if value == ApiMaturity.STABLE:
            return ""
        if value == ApiMaturity.PROVISIONAL:
            return "provisional "
        if value == ApiMaturity.INTERNAL:
            return "internal "
        if value == ApiMaturity.DEPRECATED:
            return "deprecated "
    elif type(value) is AttributeStorage:
        if value == AttributeStorage.RAM:
            return "ram"
        if value == AttributeStorage.PERSIST:
            return "persist"
        if value == AttributeStorage.CALLBACK:
            return "callback"

    # wrong value in general
    return "Unknown/unsupported: %r" % value


def event_access_string(e: Event) -> str:
    """Generates the access string required for an event. If string is non-empty it will
       include a trailing space
    """
    result = ""
    if e.readacl != AccessPrivilege.VIEW:
        result += "read: " + human_text_string(e.readacl)

    if not result:
        return ""
    return f"access({result}) "


def command_access_string(c: Command) -> str:
    """Generates the access string required for a command. If string is non-empty it will
       include a trailing space
    """
    result = ""
    if c.invokeacl != AccessPrivilege.OPERATE:
        result += "invoke: " + human_text_string(c.invokeacl)

    if not result:
        return ""
    return f"access({result}) "


def attribute_access_string(a: Attribute) -> str:
    """Generates the access string required for a struct. If string is non-empty it will
       include a trailing space
    """
    result = []

    if a.readacl != AccessPrivilege.VIEW:
        result.append("read: " + human_text_string(a.readacl))

    if a.writeacl != AccessPrivilege.OPERATE:
        result.append("write: " + human_text_string(a.writeacl))

    if not result:
        return ""

    return f"access({', '.join(result)}) "


def render_default(value: Union[str, int, bool]) -> str:
    """
    Renders a idl-style default.

    Generally quotes strings and handles bools
    """
    if type(value) is str:
        # TODO: technically this should support escaping for quotes
        #       however currently we never needed this. Escaping can be
        #       added once we use this info
        return f'"{value}"'
    elif type(value) is bool:
        if value:
            return "true"
        else:
            return "false"
    return str(value)


class IdlGenerator(CodeGenerator):
    """
    Generation .matter idl files for a given IDL
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, **kargs):
        super().__init__(storage, idl, fs_loader_searchpath=os.path.dirname(__file__))

        self.jinja_env.filters['idltxt'] = human_text_string
        self.jinja_env.filters['event_access'] = event_access_string
        self.jinja_env.filters['command_access'] = command_access_string
        self.jinja_env.filters['attribute_access'] = attribute_access_string
        self.jinja_env.filters['render_default'] = render_default

        # Easier whitespace management
        self.jinja_env.trim_blocks = True
        self.jinja_env.lstrip_blocks = True

    def internal_render_all(self):
        """
        Renders the output.
        """

        # Header containing a macro to initialize all cluster plugins
        self.internal_render_one_output(
            template_path="MatterIdl.jinja",
            output_file_name="idl.matter",
            vars={
                'idl': self.idl
            }
        )
