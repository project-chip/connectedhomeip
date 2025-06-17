# Copyright (c) 2025 Project CHIP Authors
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
#
import os

from matter.idl.generators import CodeGenerator
from matter.idl.generators.storage import GeneratorStorage
from matter.idl.matter_idl_types import (AccessPrivilege, Attribute, AttributeQuality, Command, CommandQuality, FieldQuality, Idl,
                                         Struct)


def as_privilege(privilege: AccessPrivilege) -> str:
    match privilege:
        case AccessPrivilege.VIEW:
            return "Access::Privilege::kView"
        case AccessPrivilege.OPERATE:
            return "Access::Privilege::kOperate"
        case AccessPrivilege.MANAGE:
            return "Access::Privilege::kManage"
        case AccessPrivilege.ADMINISTER:
            return "Access::Privilege::kAdminister"


def extract_attribute_quality_flags(attribute: Attribute) -> list[str]:
    result = []

    if attribute.qualities & AttributeQuality.TIMED_WRITE:
        result.append('DataModel::AttributeQualityFlags::kTimed')

    if attribute.definition.is_list:
        result.append('DataModel::AttributeQualityFlags::kListAttribute')

    if attribute.definition.qualities & FieldQuality.FABRIC_SENSITIVE:
        result.append('DataModel::AttributeQualityFlags::kFabricSensitive')

    # TODO: kChangesOmitted is not available
    # TODO: kFabricScoped is not available here (it is a struct/field quality...)

    return result


def extract_command_quality_flags(command: Command) -> list[str]:
    result = []

    if command.qualities & CommandQuality.FABRIC_SCOPED:
        result.append('DataModel::CommandQualityFlags::kFabricScoped')

    if command.qualities & CommandQuality.TIMED_INVOKE:
        result.append('DataModel::CommandQualityFlags::kTimed')

    # TODO: kLargeMessage is not available

    return result


def name_for_id_usage(name: str) -> str:
    """
    Generally the same as `upfirst` EXCEPT that it has additional handling for special
    casing where `.matter` naming (which is already convereted by zap) does not match
    what zap used to generate. Since original "names with spaces" is lost in matter
    as names are already converted, this is a special case-based implementation.
    """
    if name == "RFID":
        return "Rfid"

    return name[0].upper() + name[1:]


def global_attribute(attribute: Attribute) -> bool:
    return 0xFFF8 <= attribute.definition.code <= 0xFFFF


def response_struct(s: Struct) -> bool:
    return s.code is not None


class SdkGenerator(CodeGenerator):
    """
    Generation of cpp code for application implementation for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, **kargs):
        """
        Inintialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl, fs_loader_searchpath=os.path.dirname(__file__))
        self.jinja_env.filters['as_privilege'] = as_privilege
        self.jinja_env.filters['extract_attribute_quality_flags'] = extract_attribute_quality_flags
        self.jinja_env.filters['extract_command_quality_flags'] = extract_command_quality_flags
        self.jinja_env.filters['name_for_id_usage'] = name_for_id_usage
        self.jinja_env.tests['global_attribute'] = global_attribute
        self.jinja_env.tests['response_struct'] = response_struct

    def internal_render_all(self):
        """
        Renders the cpp and header files required for sdk cluster definitions
        """

        self.internal_render_one_output(
            template_path="AllItemsBuild.jinja",
            output_file_name="BUILD.gn",
            vars={
                "clusters": self.idl.clusters,
                "input_name": self.idl.parse_file_name,
            },
        )

        for cluster in self.idl.clusters:

            build_targets = {
                "Build.jinja": "BUILD.gn",

                # contains `*Entry` items for attributes and commands
                "Metadata.h.jinja": "Metadata.h",

                # contains id definitions
                "AttributeIds.h.jinja": "AttributeIds.h",
                "ClusterId.h.jinja": "ClusterId.h",
                "CommandIds.h.jinja": "CommandIds.h",
                "EventIds.h.jinja": "EventIds.h",
                "Ids.h.jinja": "Ids.h",
            }

            for template_path, output_file in build_targets.items():
                self.internal_render_one_output(
                    template_path=template_path,
                    output_file_name=f"{cluster.name}/{output_file}",
                    vars={
                        "cluster": cluster,
                        "input_name": self.idl.parse_file_name,
                    },
                )
