# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import enum

from idl.generators.java import JavaGenerator
from idl.generators.bridge import BridgeGenerator
from idl.generators.cpp.application import CppApplicationGenerator


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
