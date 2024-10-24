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
    from matter_idl.zapxml import ParseSource, ParseXmls
except ImportError:
    import os
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.zapxml import ParseSource, ParseXmls

from matter_idl.matter_idl_types import (AccessPrivilege, Attribute, AttributeQuality, Bitmap, Cluster, Command, ConstantEntry,
                                         DataType, Enum, Event, EventPriority, EventQuality, Field, FieldQuality, Idl, Struct,
                                         StructQuality, StructTag)


def XmlToIdl(what: Union[str, List[str]]) -> Idl:
    if not isinstance(what, list):
        what = [what]

    sources = []
    for idx, txt in enumerate(what):
        sources.append(ParseSource(source=io.StringIO(
            txt), name=("Input %d" % (idx + 1))))

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
                <name>Test</name>
                <code>0x1234</code>
                <description>Test</description>

                <attribute side="server" code="10" type="CHAR_STRING" minLength="2" length="10" isNullable="true" \
                    reportable="true" writable="false">SomeCharStringAttribute</attribute>

                <attribute side="server" code="11" type="INT32U" min="0" max="2" isNullable="true" \
                    reportable="true" writable="false">SomeIntAttribute</attribute>

                <attribute side="server" code="22" define="SOME_DEFINE" type="INT8U" min="0" max="10" \
                    reportable="true" default="0" writable="true" optional="true">
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

                <command source="server" code="0x44" name="GetSomeDataResponse" disableDefaultResponse="true" optional="true">
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
                                 name='Test',
                                 code=0x1234,
                                 description="Test",
                                 attributes=[
                                     Attribute(definition=Field(
                                         data_type=DataType(
                                             name='CHAR_STRING', min_length=2, max_length=10),
                                         code=10,
                                         name='SomeCharStringAttribute',
                                         qualities=FieldQuality.NULLABLE),
                                         qualities=AttributeQuality.READABLE,
                                         readacl=AccessPrivilege.VIEW, writeacl=AccessPrivilege.OPERATE),

                                     Attribute(definition=Field(
                                         data_type=DataType(
                                             name='INT32U', min_value=0, max_value=2),
                                         code=11,
                                         name='SomeIntAttribute',
                                         qualities=FieldQuality.NULLABLE),
                                         qualities=AttributeQuality.READABLE,
                                         readacl=AccessPrivilege.VIEW, writeacl=AccessPrivilege.OPERATE),

                                     Attribute(definition=Field(
                                         data_type=DataType(
                                             name='INT8U', min_value=0, max_value=10),
                                         code=22, name='AttributeWithAccess',
                                         qualities=FieldQuality.OPTIONAL),
                                         qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE,
                                         readacl=AccessPrivilege.OPERATE,
                                         writeacl=AccessPrivilege.MANAGE)
                                 ],
                                 structs=[
                                     Struct(name='GetSomeDataRequest',
                                            fields=[
                                                Field(data_type=DataType(
                                                    name='INT8U'), code=0, name='firstInput'),
                                                Field(data_type=DataType(
                                                    name='INT16U'), code=1, name='secondInput')
                                            ],
                                            tag=StructTag.REQUEST),
                                     Struct(name='GetSomeDataResponse',
                                            fields=[
                                                Field(data_type=DataType(name='INT8U'), code=0,
                                                      name='dataPoint1'),
                                                Field(data_type=DataType(name='INT8U'), code=1, name='dataPoint2',
                                                      qualities=FieldQuality.OPTIONAL)
                                            ],
                                            tag=StructTag.RESPONSE, code=0x44)
                                 ],
                                 commands=[
                                     Command(name='GetSomeData', code=33,
                                             input_param='GetSomeDataRequest', output_param='GetSomeDataResponse',
                                             description='This is just a test: client to server',
                                             invokeacl=AccessPrivilege.ADMINISTER)
                                 ])
                         ]))

    def testBitmap(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <cluster><name>Test1</name><code>0x0001</code></cluster>
              <cluster><name>Test2</name><code>0x0002</code></cluster>

              <bitmap name="MyBitmap" type="BITMAP32">
                 <cluster code="1"/>
                 <cluster code="0x02"/>
                 <field name="BitmapMask1" mask="0x1"/>
                 <field name="BitmapMask2" mask="0x2"/>
                 <field name="BitmapMask3" mask="0x4"/>
              </bitmap>
            </configurator>
        ''')
        bitmap = Bitmap(
            name='MyBitmap',
            base_type='BITMAP32',
            entries=[
                ConstantEntry(name='BitmapMask1', code=1),
                ConstantEntry(name='BitmapMask2', code=2),
                ConstantEntry(name='BitmapMask3', code=4)
            ])

        self.assertEqual(idl,
                         Idl(clusters=[
                             Cluster(name='Test1', code=1, bitmaps=[bitmap]),
                             Cluster(name='Test2', code=2, bitmaps=[bitmap]),
                         ]))

    def testFabricScopedAndSensitive(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <cluster>
                <name>Test</name>
                <code>0x0001</code>

                <event side="server" code="0x1234" name="FabricEvent" priority="info" isFabricSensitive="true" optional="false">
                  <description>This is a test event</description>
                  <field id="1" name="AdminNodeID" type="node_id" isNullable="true"/>
                  <access op="read" privilege="administer"/>
                </event>

              </cluster>

              <struct name="FabricStruct" isFabricScoped="true">
                <cluster code="1"/>
                <item fieldId="1" name="Field1" type="int32u" isFabricSensitive="true"/>
                <item fieldId="3" name="Field3" type="int32u" isFabricSensitive="true"/>
                <item fieldId="10" name="Field10" type="int32u" />
              </struct>


            </configurator>
        ''')
        self.assertEqual(idl,
                         Idl(clusters=[Cluster(name='Test',
                                               code=1,
                                               events=[Event(priority=EventPriority.INFO,
                                                             name='FabricEvent',
                                                             code=0x1234,
                                                             description="This is a test event",
                                                             fields=[Field(data_type=DataType(name='node_id'),
                                                                           code=1,
                                                                           name='AdminNodeID',
                                                                           qualities=FieldQuality.NULLABLE)],
                                                             readacl=AccessPrivilege.ADMINISTER,
                                                             qualities=EventQuality.FABRIC_SENSITIVE)],
                             structs=[Struct(name='FabricStruct',
                                      fields=[Field(data_type=DataType(name='int32u'),
                                                    code=1,
                                                    name='Field1',
                                                    qualities=FieldQuality.FABRIC_SENSITIVE),
                                              Field(data_type=DataType(name='int32u'),
                                                    code=3,
                                                    name='Field3',
                                                    qualities=FieldQuality.FABRIC_SENSITIVE),
                                              Field(data_type=DataType(name='int32u',
                                                                       min_length=None,
                                                                       max_length=None,
                                                                       min_value=None,
                                                                       max_value=None),
                                                    code=10,
                                                    name='Field10')],
                                      qualities=StructQuality.FABRIC_SCOPED)],
                         )]))

    def testGlobalEnum(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <enum name="One" type="ENUM8">
                <item value="3" name="Three" />
              </enum>

              <enum name="Two" type="ENUM8">
                <item value="100" name="Big" />
                <item value="2000" name="Bigger" />
              </enum>
            </configurator>
        ''')
        e1 = Enum(
            name='One',
            base_type="ENUM8",
            entries=[
                ConstantEntry(name="Three", code=3),
            ]
        )
        e2 = Enum(
            name='Two',
            base_type="ENUM8",
            entries=[
                ConstantEntry(name="Big", code=100),
                ConstantEntry(name="Bigger", code=2000),
            ]
        )
        self.assertEqual(idl, Idl(global_enums=[e1, e2]))

    def testEnum(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <cluster><name>Test1</name><code>10</code></cluster>
              <cluster><name>Test2</name><code>20</code></cluster>

              <enum name="OneCluster" type="ENUM8">
                <cluster code="10" />
                <item value="3" name="Three" />
              </enum>

              <enum name="TwoClusters" type="ENUM8">
                <cluster code="10" />
                <cluster code="20" />
                <item value="100" name="Big" />
                <item value="2000" name="Bigger" />
              </enum>
            </configurator>
        ''')
        e2 = Enum(
            name='OneCluster',
            base_type="ENUM8",
            entries=[
                ConstantEntry(name="Three", code=3),
            ]
        )
        e3 = Enum(
            name='TwoClusters',
            base_type="ENUM8",
            entries=[
                ConstantEntry(name="Big", code=100),
                ConstantEntry(name="Bigger", code=2000),
            ]
        )
        self.assertEqual(idl,
                         Idl(clusters=[
                             Cluster(name='Test1', code=10, enums=[e2, e3]),
                             Cluster(name='Test2', code=20, enums=[e3])],
                             ))

    def testFeatures(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <cluster>
                  <name>TestFeatures</name>
                  <code>20</code>

                  <features>
                    <feature bit="0" code="DEPONOFF" name="OnOff" summary="Test">
                      <optionalConform/>
                    </feature>
                    <feature bit="1" code="TEST" name="TestFeature" summary="Test2">
                      <optionalConform/>
                    </feature>
                    <feature bit="2" code="XYZ" name="AnotherTest" summary="Test2">
                      <optionalConform/>
                    </feature>
                  </features>
              </cluster>
            </configurator>
        ''')
        bitmap = Bitmap(
            name='Feature',
            base_type='bitmap32',
            entries=[
                ConstantEntry(name='OnOff', code=1),
                ConstantEntry(name='TestFeature', code=2),
                ConstantEntry(name='AnotherTest', code=4),
            ])
        self.assertEqual(idl,
                         Idl(clusters=[
                             Cluster(name='TestFeatures', code=20, bitmaps=[bitmap])
                         ])),

    def testGlobalStruct(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <struct name="SomeStruct" isFabricScoped="true">
                <item name="FirstMember" type="int16u" />
                <item name="SecondMember" type="int32u" />
              </struct>

            </configurator>
        ''')
        struct = Struct(
            name='SomeStruct',
            qualities=StructQuality.FABRIC_SCOPED,
            fields=[
                Field(data_type=DataType(name='int16u'),
                      code=0, name='FirstMember'),
                Field(data_type=DataType(name='int32u'),
                      code=1, name='SecondMember')
            ]
        )
        self.assertEqual(idl, Idl(global_structs=[struct]))

    def testStruct(self):
        idl = XmlToIdl('''<?xml version="1.0"?>
            <configurator>
              <cluster><name>Test1</name><code>0x000A</code></cluster>
              <cluster>
                  <name>Test2</name>
                  <code>20</code>

                  <attribute side="server" code="123" type="SomeStruct" isNullable="true" writable="false">
                     FabricAttribute
                  </attribute>
              </cluster>

              <struct name="SomeStruct" isFabricScoped="true">
                <cluster code="10" />
                <cluster code="0x0014" />
                <item name="FirstMember" type="int16u" />
                <item name="SecondMember" type="int32u" />
              </struct>

            </configurator>
        ''')
        struct = Struct(
            name='SomeStruct',
            qualities=StructQuality.FABRIC_SCOPED,
            fields=[
                Field(data_type=DataType(name='int16u'),
                      code=0, name='FirstMember'),
                Field(data_type=DataType(name='int32u'),
                      code=1, name='SecondMember')
            ]
        )
        self.assertEqual(idl,
                         Idl(clusters=[
                             Cluster(name='Test1', code=10, structs=[struct]),
                             Cluster(name='Test2', code=20,
                                     structs=[struct],
                                     attributes=[
                                         Attribute(
                                             definition=Field(
                                                 data_type=DataType(
                                                     name='SomeStruct'),
                                                 code=123,
                                                 name='FabricAttribute',
                                                 qualities=FieldQuality.NULLABLE
                                             ),
                                             qualities=AttributeQuality.READABLE,
                                             readacl=AccessPrivilege.VIEW,
                                             writeacl=AccessPrivilege.OPERATE)]), ]))

    def testSkipsNotProcessedFields(self):
        # Zap has extra fields that are generally not processed
        # This includes such fields and ansures we do not consider them
        idl = XmlToIdl('''<?xml version="1.0"?>
<!--
Some copyright here... testing that we skip over comments
-->
<configurator>
  <domain name="CHIP"/>
  <cluster>
    <name>Window Covering</name>
    <domain>Closures</domain>
    <code>0x0102</code>
    <define>WINDOW_COVERING_CLUSTER</define>
    <description>Provides an interface for controlling and adjusting automatic window coverings. </description>

    <!-- Abbreviations used in descriptions -->
    <tag name="LF" description="Lift Control"/>
    <tag name="TL" description="Tilt Control"/>
    <tag name="PA_LF" description="Position Aware Lift"/>
    <tag name="ABS" description="Absolute Positioning"/>
    <tag name="PA_TL" description="Position Aware Tilt"/>

    <client tick="false" init="false">true</client>
    <server tick="false" init="false">true</server>

    <!-- Window Covering Information Attribute Set -->
    <!-- Conformance feature M -->
    <attribute side="server" writable="false" code="0x0000" define="WC_TYPE"
               type="Type"     min="0x00"   max="0x09"   default="0x00"   optional="false">Type</attribute>
  </cluster>
</configurator>
        ''')
        self.assertEqual(idl,
                         Idl(clusters=[
                             Cluster(name='WindowCovering', code=0x102,
                                     description='Provides an interface for controlling and adjusting automatic window coverings. ',
                                     structs=[],
                                     attributes=[
                                         Attribute(
                                             definition=Field(
                                                 data_type=DataType(
                                                     name='Type', min_value=0, max_value=9),
                                                 code=0,
                                                 name='Type',
                                             ),
                                             qualities=AttributeQuality.READABLE,
                                             readacl=AccessPrivilege.VIEW,
                                             writeacl=AccessPrivilege.OPERATE)]), ]))


if __name__ == '__main__':
    unittest.main()
