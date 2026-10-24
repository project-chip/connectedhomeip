#!/usr/bin/env python3

# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import unittest
from importlib import import_module
from pathlib import Path

EXAMPLES_DIR = Path(__file__).resolve().parents[2] / "examples"


class TestMatterIdlPlugin(unittest.TestCase):
    def test_to_protobuf_type(self):
        sys.path.append(str(EXAMPLES_DIR))
        toProtobufType = import_module("matter_idl_plugin").toProtobufType

        self.assertEqual(toProtobufType("boolean"), "bool")
        self.assertEqual(toProtobufType("single"), "float")
        self.assertEqual(toProtobufType("double"), "double")
        self.assertEqual(toProtobufType("char_string"), "string")
        self.assertEqual(toProtobufType("long_octet_string"), "bytes")
        self.assertEqual(toProtobufType("int16u"), "uint32")
        self.assertEqual(toProtobufType("epoch_s"), "uint32")
        self.assertEqual(toProtobufType("int40u"), "uint64")
        self.assertEqual(toProtobufType("int16s"), "int32")
        self.assertEqual(toProtobufType("temperature"), "int32")
        self.assertEqual(toProtobufType("int40s"), "int64")
        self.assertEqual(toProtobufType("MyEnum"), "MyEnum")
        self.assertEqual(toProtobufType("CustomStruct"), "CustomStruct")


if __name__ == "__main__":
    unittest.main()
