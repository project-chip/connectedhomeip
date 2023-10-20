#!/usr/bin/env python3
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

import io
import unittest
from typing import List, Union

try:
    from matter_idl.data_model_xml import ParseSource, ParseXmls
except ImportError:
    import os
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.data_model_xml import ParseSource, ParseXmls

from matter_idl_parser import CreateParser
from matter_idl.matter_idl_types import (AccessPrivilege, Attribute, AttributeQuality, Bitmap, Cluster, ClusterSide, Command,
                                         ConstantEntry, DataType, Enum, Event, EventPriority, EventQuality, Field, FieldQuality,
                                         Idl, Struct, StructQuality, StructTag)

def XmlToIdl(what: Union[str, List[str]]) -> Idl:
    if not isinstance(what, list):
        what = [what]

    sources = []
    for idx, txt in enumerate(what):
        sources.append(ParseSource(source=io.StringIO(
            txt), name=("Input %d" % (idx + 1))))

    return ParseXmls(sources, include_meta_data=False)

def IdlTextToIdl(what: str) -> Idl:
    return CreateParser(skip_meta=True).parse(what)


class TestXmlParser(unittest.TestCase):

    def testBasicInput(self):
        xml_idl = XmlToIdl('<cluster id="123" name="Test" revision="1"/>')
        expected_idl = IdlTextToIdl('''
        client cluster Test = 123 {
           readonly attribute attrib_id attributeList[] = 65531;
           readonly attribute event_id eventList[] = 65530;
           readonly attribute command_id acceptedCommandList[] = 65529;
           readonly attribute command_id generatedCommandList[] = 65528;
           readonly attribute bitmap32 featureMap = 65532;
           readonly attribute int16u clusterRevision = 65533;
        }
        ''')

        self.assertEqual(xml_idl, expected_idl)


if __name__ == '__main__':
    unittest.main()
