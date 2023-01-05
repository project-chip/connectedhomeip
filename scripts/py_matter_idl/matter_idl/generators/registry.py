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

from matter_idl.generators.bridge import BridgeGenerator
from matter_idl.generators.cpp.application import CppApplicationGenerator
from matter_idl.generators.java import JavaGenerator


class CodeGenerator(enum.Enum):
    """
    Represents every generator type supported by codegen and maps
    the simple enum value (user friendly and can be a command line input)
    into underlying generators.
    """
    JAVA = enum.auto()
    BRIDGE = enum.auto()
    CPP_APPLICATION = enum.auto()

    def Create(self, *args, **kargs):
        if self == CodeGenerator.JAVA:
            return JavaGenerator(*args, **kargs)
        elif self == CodeGenerator.BRIDGE:
            return BridgeGenerator(*args, **kargs)
        elif self == CodeGenerator.CPP_APPLICATION:
            return CppApplicationGenerator(*args, **kargs)
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
    'java': CodeGenerator.JAVA,
    'bridge': CodeGenerator.BRIDGE,
    'cpp-app': CodeGenerator.CPP_APPLICATION,
}
