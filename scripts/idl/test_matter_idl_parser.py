#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
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

try:
    from .matter_idl_parser import CreateParser
    from .matter_idl_types import *
except:
    import os
    import sys
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))

    from matter_idl_parser import CreateParser
    from matter_idl_types import *

import unittest


def parseText(txt):
    return CreateParser().parse(txt)


class TestParser(unittest.TestCase):

    def test_skips_comments(self):
        actual = parseText("""
            // this is a single line comment
            // repeated

            /* This is a C++ comment
               and also whitespace should be ignored
             */
        """)
        expected = Idl()

        self.assertEqual(actual, expected)

    def test_global_enum(self):
        actual = parseText("""
            enum GlobalEnum : ENUM8 {
               kValue1 = 1;
               kOther = 0x12; /* hex numbers tested sporadically */
            }
        """)

        expected = Idl(enums=[
            Enum(name='GlobalEnum', base_type='ENUM8',
                 entries=[
                     EnumEntry(name="kValue1", code=1),
                     EnumEntry(name="kOther", code=0x12),
                 ])]
        )
        self.assertEqual(actual, expected)

    def test_global_struct(self):
        actual = parseText("""
            struct Something {
                CHAR_STRING astring = 1;
                optional CLUSTER_ID idlist[] = 2;
                nullable int valueThatIsNullable = 0x123;
                char_string<123> sized_string = 222;
            }
        """)

        expected = Idl(structs=[
            Struct(name='Something',
                   fields=[
                        Field(
                            data_type=DataType(name="CHAR_STRING"), code=1, name="astring", ),
                        Field(data_type=DataType(name="CLUSTER_ID"), code=2, name="idlist", is_list=True, attributes=set(
                            [FieldAttribute.OPTIONAL])),
                        Field(data_type=DataType(name="int"), code=0x123, name="valueThatIsNullable", attributes=set(
                            [FieldAttribute.NULLABLE])),
                        Field(data_type=DataType(name="char_string", max_length=123),
                              code=222, name="sized_string", attributes=set()),
                   ])]
        )
        self.assertEqual(actual, expected)

    def test_cluster_attribute(self):
        actual = parseText("""
            server cluster MyCluster = 0x321 {
                readonly attribute int8u roAttr = 1;
                attribute int32u rwAttr[] = 123;
                global attribute int32u grwAttr[] = 124;
                readonly global attribute int32u groAttr[] = 125;
                readonly nosubscribe attribute int8s nosub[] = 0xaa;
                readonly attribute nullable int8s isNullable = 0xab;
            }
        """)

        expected = Idl(clusters=[
            Cluster(side=ClusterSide.SERVER,
                    name="MyCluster",
                    code=0x321,
                    attributes=[
                        Attribute(tags=set([AttributeTag.READABLE]), definition=Field(
                            data_type=DataType(name="int8u"), code=1, name="roAttr")),
                        Attribute(tags=set([AttributeTag.READABLE, AttributeTag.WRITABLE]), definition=Field(
                            data_type=DataType(name="int32u"), code=123, name="rwAttr", is_list=True)),
                        Attribute(tags=set([AttributeTag.GLOBAL, AttributeTag.READABLE, AttributeTag.WRITABLE]), definition=Field(
                            data_type=DataType(name="int32u"), code=124, name="grwAttr", is_list=True)),
                        Attribute(tags=set([AttributeTag.GLOBAL, AttributeTag.READABLE]), definition=Field(
                            data_type=DataType(name="int32u"), code=125, name="groAttr", is_list=True)),
                        Attribute(tags=set([AttributeTag.NOSUBSCRIBE, AttributeTag.READABLE]), definition=Field(
                            data_type=DataType(name="int8s"), code=0xAA, name="nosub", is_list=True)),
                        Attribute(tags=set([AttributeTag.READABLE]), definition=Field(
                            data_type=DataType(name="int8s"), code=0xAB, name="isNullable", attributes=set([FieldAttribute.NULLABLE]))),
                    ]
                    )])
        self.assertEqual(actual, expected)

    def test_sized_attribute(self):
        actual = parseText("""
            server cluster MyCluster = 1 {
                attribute char_string<11> attr1 = 1;
                attribute octet_string<33> attr2[] = 2;
            }
        """)

        expected = Idl(clusters=[
            Cluster(side=ClusterSide.SERVER,
                    name="MyCluster",
                    code=1,
                    attributes=[
                        Attribute(tags=set([AttributeTag.READABLE, AttributeTag.WRITABLE]), definition=Field(
                            data_type=DataType(name="char_string", max_length=11), code=1, name="attr1")),
                        Attribute(tags=set([AttributeTag.READABLE, AttributeTag.WRITABLE]), definition=Field(
                            data_type=DataType(name="octet_string", max_length=33), code=2, name="attr2", is_list=True)),
                    ]
                    )])
        self.assertEqual(actual, expected)

    def test_cluster_commands(self):
        actual = parseText("""
            server cluster WithCommands = 1 {
                struct FreeStruct {}
                request struct InParam {}
                response struct OutParam {}

                command WithoutArg(): DefaultSuccess = 123;
                command InOutStuff(InParam): OutParam = 222;
            }
        """)
        expected = Idl(clusters=[
            Cluster(side=ClusterSide.SERVER,
                    name="WithCommands",
                    code=1,
                    structs=[
                        Struct(name="FreeStruct", fields=[]),
                        Struct(name="InParam", fields=[],
                               tag=StructTag.REQUEST),
                        Struct(name="OutParam", fields=[],
                               tag=StructTag.RESPONSE),
                    ],
                    commands=[
                        Command(name="WithoutArg", code=123,
                                input_param=None, output_param="DefaultSuccess"),
                        Command(name="InOutStuff", code=222,
                                input_param="InParam", output_param="OutParam"),
                    ],
                    )])
        self.assertEqual(actual, expected)

    def test_cluster_enum(self):
        actual = parseText("""
            client cluster WithEnums = 0xab {
                enum TestEnum : ENUM16 {
                    A = 0x123;
                    B = 0x234;
                }
            }
        """)
        expected = Idl(clusters=[
            Cluster(side=ClusterSide.CLIENT,
                    name="WithEnums",
                    code=0xab,
                    enums=[
                        Enum(name="TestEnum", base_type="ENUM16",
                             entries=[
                                 EnumEntry(name="A", code=0x123),
                                 EnumEntry(name="B", code=0x234),
                             ])],
                    )])
        self.assertEqual(actual, expected)

    def test_cluster_events(self):
        actual = parseText("""
            client cluster EventTester = 0x123 {
               critical event StartUp = 0 {
                 INT32U softwareVersion = 0;
               }
               info event Hello = 1 {}
               debug event GoodBye = 2 {}
            }
        """)
        expected = Idl(clusters=[
            Cluster(side=ClusterSide.CLIENT,
                    name="EventTester",
                    code=0x123,
                    events=[
                        Event(priority=EventPriority.CRITICAL, name="StartUp", code=0, fields=[
                            Field(data_type=DataType(name="INT32U"),
                                  code=0, name="softwareVersion"),
                        ]),
                        Event(priority=EventPriority.INFO,
                              name="Hello", code=1, fields=[]),
                        Event(priority=EventPriority.DEBUG,
                              name="GoodBye", code=2, fields=[]),
                    ])])
        self.assertEqual(actual, expected)

    def test_multiple_clusters(self):
        actual = parseText("""
            server cluster A = 1 {}
            client cluster B = 2 {}
            client cluster C = 3 {}
        """)

        expected = Idl(clusters=[
            Cluster(side=ClusterSide.SERVER, name="A", code=1),
            Cluster(side=ClusterSide.CLIENT, name="B", code=2),
            Cluster(side=ClusterSide.CLIENT, name="C", code=3),
        ])
        self.assertEqual(actual, expected)

    def test_endpoints(self):
        actual = parseText("""
            endpoint 12 {
                server cluster Foo;
                server cluster Bar;
                binding cluster Bar;
                binding cluster Test;
            }
        """)

        expected = Idl(endpoints=[Endpoint(number=12,
                                           server_clusters=["Foo", "Bar"],
                                           client_bindings=["Bar", "Test"],)
                                  ])
        self.assertEqual(actual, expected)

    def test_multi_endpoints(self):
        actual = parseText("""
            endpoint 1 {}
            endpoint 2 {}
            endpoint 0xa {}
            endpoint 100 {}
        """)

        expected = Idl(endpoints=[
            Endpoint(number=1),
            Endpoint(number=2),
            Endpoint(number=10),
            Endpoint(number=100),
        ])
        self.assertEqual(actual, expected)


if __name__ == '__main__':
    unittest.main()
