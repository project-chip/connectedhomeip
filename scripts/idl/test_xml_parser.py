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

import unittest
import io

from typing import Optional, Union, List

try:
    from idl.matter_idl_types import *
    from idl.zapxml import ParseSource, ParseXmls
except:
    import os
    import sys

    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

    from idl.matter_idl_types import *
    from idl.zapxml import ParseSource, ParseXmls


def XmlToIdl(what: Union[str, List[str]]) -> Idl:
    if not isinstance(what, list):
        what = [what]

    sources = []
    for idx, txt in enumerate(what):
        sources.append(ParseSource(source=io.StringIO(txt), name=("Input %d" % (idx + 1))))

    return ParseXmls(sources, include_meta_data=False)


class TestXmlParser(unittest.TestCase):

    def testEmptyInput(self):
        idl = XmlToIdl('<configurator/>')
        self.assertEqual(idl, Idl())

    def testCluster(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <!-- Comments should be supported -->
            <configurator>
              <cluster>
                <name>Test Cluster</name>
                <code>0x1234</code>

                <attribute side="server" code="11" type="INT32U" min="0" max="2" isNullable="true" reportable="true" writable="false">SomeIntAttribute</attribute>

                <attribute side="server" code="22" define="SOME_DEFINE" type="INT8U" min="0" max="10" reportable="true" default="0" writable="true" optional="true">
                    <description>AttributeWithAccess</description>
                    <access op="read" role="operate" />
                    <access op="write" role="manage" />
                </attribute>

                <command source="client" code="33" name="GetSomeData" response="GetSomeDataResponse" optional="true">
                    <description>This is just a test: client to server</description>
                    <access op="invoke" role="administer" />
                    <arg name="firstInput" type="INT8U" />
                    <arg name="secondInput" type="INT16U" />
                </command>

                <command source="server" code="44" name="GetSomeDataResponse" disableDefaultResponse="true" optional="true">
                    <description>Reply from server</description>
                    <arg name="dataPoint1" type="INT8U" />
                    <arg name="dataPoint2" type="INT8U" optional="true" />
                </command>
              </cluster>
            </configurator>
        ''')
        self.assertEqual(idl,
                         Idl(clusters=[
                             Cluster(
                                 side=ClusterSide.CLIENT,
                                 name='TestCluster',
                                 code=0x1234,
                                 attributes=[
                                     Attribute(definition=Field(data_type=DataType(name='INT32U'), code=11, name='SomeIntAttribute',
                                                                attributes={FieldAttribute.NULLABLE}), tags={AttributeTag.READABLE},
                                               readacl=AccessPrivilege.VIEW, writeacl=AccessPrivilege.OPERATE),
                                     Attribute(definition=Field(data_type=DataType(name='INT8U'), code=22, name='AttributeWithAccess',
                                                                attributes={FieldAttribute.OPTIONAL}),
                                               tags={AttributeTag.READABLE, AttributeTag.WRITABLE}, readacl=AccessPrivilege.OPERATE,
                                               writeacl=AccessPrivilege.MANAGE)
                                 ],
                                 structs=[
                                     Struct(name='GetSomeDataRequest',
                                            fields=[
                                                Field(data_type=DataType(name='INT8U'), code=1, name='firstInput'),
                                                Field(data_type=DataType(name='INT16U'), code=2, name='secondInput')
                                            ],
                                            tag=StructTag.REQUEST),
                                     Struct(name='GetSomeDataResponse',
                                            fields=[
                                                Field(data_type=DataType(name='INT8U'), code=1,
                                                      name='dataPoint1'),
                                                Field(data_type=DataType(name='INT8U'), code=2, name='dataPoint2',
                                                      attributes={FieldAttribute.OPTIONAL})
                                            ],
                                            tag=StructTag.RESPONSE, code=44)
                                 ],
                                 commands=[
                                     Command(name='GetSomeData', code=33, input_param='GetSomeDataRequest', output_param='GetSomeDataResponse',
                                             invokeacl=AccessPrivilege.ADMINISTER)
                                 ])
                         ]))


if __name__ == '__main__':
    unittest.main()
