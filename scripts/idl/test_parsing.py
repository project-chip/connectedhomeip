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

from parser import CreateParser
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
            }
        """)

        expected = Idl(structs=[
            Struct(name='Something',
                   members=[
                        StructureMember(
                            data_type="CHAR_STRING", code=1, name="astring", ),
                        StructureMember(data_type="CLUSTER_ID", code=2, name="idlist", is_list=True, attributes=set(
                            [MemberAttribute.OPTIONAL])),
                        StructureMember(data_type="int", code=0x123, name="valueThatIsNullable", attributes=set(
                            [MemberAttribute.NULLABLE])),
                   ])]
        )
        self.assertEqual(actual, expected)

    def test_cluster_attribute(self):
        actual = parseText("""
            server cluster MyCluster = 0x321 {
                attribute(readonly) int8u roAttr = 1;
                attribute(writable) int32u rwAttr[] = 123;
            }
        """)

        expected = Idl(clusters=[
            Cluster(side=ClusterSide.SERVER,
                    name="MyCluster",
                    code=0x321,
                    attributes=[
                        Attribute(access=AttributeAccess.READONLY, definition=StructureMember(
                            data_type="int8u", code=1, name="roAttr")),
                        Attribute(access=AttributeAccess.READWRITE, definition=StructureMember(
                            data_type="int32u", code=123, name="rwAttr", is_list=True)),
                    ]
                    )])
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


if __name__ == '__main__':
    unittest.main()
