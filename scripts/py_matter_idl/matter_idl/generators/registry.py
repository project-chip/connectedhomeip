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

import enum
import importlib

from matter_idl.generators.cpp.application import CppApplicationGenerator
from matter_idl.generators.cpp.tlvmeta import TLVMetaDataGenerator
from matter_idl.generators.idl import IdlGenerator
from matter_idl.generators.java import JavaClassGenerator, JavaJNIGenerator
from matter_idl.generators.kotlin import KotlinClassGenerator
from matter_idl.generators.markdown import SummaryMarkdownGenerator


class CodeGenerator(enum.Enum):
    """
    Represents every generator type supported by codegen and maps
    the simple enum value (user friendly and can be a command line input)
    into underlying generators.
    """
    JAVA_JNI = enum.auto()
    JAVA_CLASS = enum.auto()
    KOTLIN_CLASS = enum.auto()
    CPP_APPLICATION = enum.auto()
    CPP_TLVMETA = enum.auto()
    IDL = enum.auto()
    SUMMARY_MARKDOWN = enum.auto()
    CUSTOM = enum.auto()

    def Create(self, *args, **kargs):
        if self == CodeGenerator.JAVA_JNI:
            return JavaJNIGenerator(*args, **kargs)
        elif self == CodeGenerator.JAVA_CLASS:
            return JavaClassGenerator(*args, **kargs)
        elif self == CodeGenerator.KOTLIN_CLASS:
            return KotlinClassGenerator(*args, **kargs)
        elif self == CodeGenerator.CPP_APPLICATION:
            return CppApplicationGenerator(*args, **kargs)
        elif self == CodeGenerator.CPP_TLVMETA:
            return TLVMetaDataGenerator(*args, **kargs)
        elif self == CodeGenerator.IDL:
            return IdlGenerator(*args, **kargs)
        elif self == CodeGenerator.SUMMARY_MARKDOWN:
            return SummaryMarkdownGenerator(*args, **kargs)
        elif self == CodeGenerator.CUSTOM:
            # Use a package naming convention to find the custom generator:
            # ./matter_idl_plugin/__init__.py defines a subclass of CodeGenerator named CustomGenerator.
            # The plugin is expected to be in the path provided via the `--plugin <path>` cli argument.
            # Replaces `from plugin_module import CustomGenerator``
            plugin_module = importlib.import_module(kargs['plugin_module'])
            CustomGenerator = plugin_module.CustomGenerator
            return CustomGenerator(*args, **kargs)
        else:
            raise NameError("Unknown code generator type")

    @staticmethod
    def FromString(name):
        global GENERATORS

        if name.lower() in GENERATORS:
            return GENERATORS[name.lower()]
        else:
            raise NameError("Unknown code generator type '%s'" % name)


# Contains all known code generators along with a string
# to uniquely identify them when running command line tools or
# executing tests
GENERATORS = {
    'java-jni': CodeGenerator.JAVA_JNI,
    'java-class': CodeGenerator.JAVA_CLASS,
    'kotlin-class': CodeGenerator.KOTLIN_CLASS,
    'cpp-app': CodeGenerator.CPP_APPLICATION,
    'cpp-tlvmeta': CodeGenerator.CPP_TLVMETA,
    'idl': CodeGenerator.IDL,
    'summary-markdown': CodeGenerator.SUMMARY_MARKDOWN,
    'custom': CodeGenerator.CUSTOM,
}
