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

from matter_idl.matter_idl_types import (AccessPrivilege, Attribute, AttributeQuality, Bitmap, Cluster, ClusterSide, Command,
                                         ConstantEntry, DataType, Enum, Event, EventPriority, EventQuality, Field, FieldQuality,
                                         Idl, Struct, StructQuality, StructTag)

def GlobalAttributes() -> List[Attribute]:
    return [
        Attribute(definition=Field(data_type=DataType(name="attrib_id"), code=65531, name="attributeList", is_list=True)),
        Attribute(definition=Field(data_type=DataType(name="event_id"), code=65530, name="eventList", is_list=True)),
        Attribute(definition=Field(data_type=DataType(name="command_id"), code=65529, name="acceptedCommandList", is_list=True)),
        Attribute(definition=Field(data_type=DataType(name="command_id"), code=65528, name="generatedCommandList", is_list=True)),
        Attribute(definition=Field(data_type=DataType(name="bitmap32"), code=65532, name="featureMap", is_list=False)),
        Attribute(definition=Field(data_type=DataType(name="int16u"), code=65533, name="clusterRevision", is_list=False)),
    ]


def XmlToIdl(what: Union[str, List[str]]) -> Idl:
    if not isinstance(what, list):
        what = [what]

    sources = []
    for idx, txt in enumerate(what):
        sources.append(ParseSource(source=io.StringIO(
            txt), name=("Input %d" % (idx + 1))))

    return ParseXmls(sources, include_meta_data=False)


class TestXmlParser(unittest.TestCase):

    def testBasicInput(self):
        idl = XmlToIdl('<cluster id="123" name="Test" revision="1"/>')

        self.assertEqual(idl, Idl(
            clusters=[Cluster(side=ClusterSide.CLIENT, name="Test", code=123, attributes=GlobalAttributes())]
        ))


if __name__ == '__main__':
    unittest.main()
