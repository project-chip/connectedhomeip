#!/usr/bin/env python3
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
import unittest
import xml.etree.ElementTree as ET

try:
    from matter_idl.generators.type_definitions import ParseDataType
except ImportError:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.generators.type_definitions import ParseDataType

from matter_idl.generators.type_definitions import BasicInteger, TypeLookupContext
from matter_idl.matter_idl_types import DataType, Idl


class TestSupportedTypes(unittest.TestCase):

    def __init__(self, *args, **kargs):
        super().__init__(*args, **kargs)
        self.maxDiff = None

    def testAllTypesSupported(self):
        # ALL types defined in chip-types.xml should be understandable
        # by the generator type parsing
        path = "src/app/zap-templates/zcl/data-model/chip/chip-types.xml"
        path = os.path.join(os.path.dirname(__file__), "../../..", path)
        dom = ET.parse(path).getroot()

        # Format we expect:
        #   - configurator/atomic/type
        self.assertEqual(dom.tag, "configurator")
        types = dom.findall("./atomic/type")

        # Arbitrary non-empty assumption to make sure we
        # test something and did not mess up our XPath query
        self.assertTrue(len(types) > 10)

        empty_lookup = TypeLookupContext(idl=Idl(), cluster=None)

        for t in types:
            # every type has the following intersting attributes:
            #  - name (to be validated)
            #  - size (in bytes, but may not be power of two)
            #  - one of discrete/analog/composite

            if "composite" in t.attrib and t.attrib["composite"] == "true":
                # struct, array, octet_string and such
                continue

            data_type = DataType(name=t.attrib["name"])

            # filter some know things
            if data_type.name in {
                    "no_data",  # intentionally skipped
                    # handled as a non-integer type
                    "boolean", "single", "double",
                    # handled as specific bitmaps
                    "bitmap8", "bitmap16", "bitmap32", "bitmap64",
                    # handled as specific enums. Note that spec defines enumerations only for 8 and 16
                    "enum8", "enum16",

                    # TODO: these may be bugs to fix
                    "unknown"
            }:
                continue

            parsed = ParseDataType(data_type, empty_lookup)

            self.assertTrue(parsed is not None)  # this should always pass.
            fail_message = f"{data_type.name} was parsed as {parsed}"

            self.assertIs(type(parsed), BasicInteger, fail_message)

            # check that types match
            if "signed" in t.attrib and t.attrib["signed"] == "true":
                # Oddly enough, we have no sign info for int8s and int8u
                # Only temperature really has a signed component
                self.assertTrue(parsed.is_signed, fail_message)

            if "size" in t.attrib:
                self.assertEqual(parsed.byte_count, int(
                    t.attrib["size"]), fail_message)


if __name__ == '__main__':
    unittest.main()
