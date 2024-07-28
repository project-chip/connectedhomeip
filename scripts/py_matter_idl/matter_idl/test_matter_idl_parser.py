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

from difflib import unified_diff

try:
    from matter_idl.matter_idl_parser import CreateParser
except ModuleNotFoundError:
    import os
    import sys
    sys.path.append(os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))))

    from matter_idl.matter_idl_parser import CreateParser

import unittest
from typing import Optional

from matter_idl.generators import GeneratorStorage
from matter_idl.generators.idl import IdlGenerator
from matter_idl.matter_idl_types import (AccessPrivilege, ApiMaturity, Attribute, AttributeInstantiation, AttributeQuality,
                                         AttributeStorage, Bitmap, Cluster, Command, CommandInstantiation, CommandQuality,
                                         ConstantEntry, DataType, DeviceType, Endpoint, Enum, Event, EventPriority, EventQuality,
                                         Field, FieldQuality, Idl, ParseMetaData, ServerClusterInstantiation, Struct, StructTag)


class GeneratorContentStorage(GeneratorStorage):
    def __init__(self):
        super().__init__()
        self.content: Optional[str] = None

    def get_existing_data(self, relative_path: str):
        # Force re-generation each time
        return None

    def write_new_data(self, relative_path: str, content: str):
        if self.content:
            raise Exception(
                "Unexpected extra data: single file generation expected")
        self.content = content


def RenderAsIdlTxt(idl: Idl) -> str:
    storage = GeneratorContentStorage()
    IdlGenerator(storage=storage, idl=idl).render(dry_run=False)
    return storage.content or ""


def parseText(txt, skip_meta=True, merge_globals=True):
    return CreateParser(skip_meta=skip_meta, merge_globals=merge_globals).parse(txt)


class TestParser(unittest.TestCase):

    def assertIdlEqual(self, a: Idl, b: Idl):
        if a == b:
            # seems the same. This will just pass
            self.assertEqual(a, b)
            return

        # Not the same. Try to make a human readable diff:
        a_txt = RenderAsIdlTxt(a)
        b_txt = RenderAsIdlTxt(b)

        delta = unified_diff(a_txt.splitlines(keepends=True),
                             b_txt.splitlines(keepends=True),
                             fromfile='actual.matter',
                             tofile='expected.matter',
                             )
        self.assertEqual(a, b, '\n' + ''.join(delta))
        self.fail("IDLs are not equal (above delta should have failed)")

    def test_skips_comments(self):
        actual = parseText("""
            // this is a single line comment
            // repeated

            /* This is a C++ comment
               and also whitespace should be ignored
             */
        """)
        expected = Idl()

        self.assertIdlEqual(actual, expected)

    def test_cluster_attribute(self):
        actual = parseText("""
            server cluster MyCluster = 0x321 {
                readonly attribute int8u roAttr = 1;
                attribute int32u rwAttr[] = 123;
                readonly nosubscribe attribute int8s nosub[] = 0xaa;
                readonly attribute nullable int8s isNullable = 0xab;
            }
        """)

        expected = Idl(clusters=[
            Cluster(name="MyCluster",
                    code=0x321,
                    attributes=[
                        Attribute(qualities=AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="int8u"), code=1, name="roAttr")),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=123, name="rwAttr", is_list=True)),
                        Attribute(qualities=AttributeQuality.NOSUBSCRIBE | AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=0xAA, name="nosub", is_list=True)),
                        Attribute(qualities=AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=0xAB, name="isNullable", qualities=FieldQuality.NULLABLE)),
                    ]
                    )])
        self.assertIdlEqual(actual, expected)

    def test_doc_comments(self):
        actual = parseText("""
            /** Documentation for MyCluster */
            server cluster MyCluster = 0x321 {
            }

            /** Documentation for MyCluster #2 */
            client cluster MyCluster2 = 0x322 {
                /* NOT a doc comment */
                command WithoutArg(): DefaultSuccess = 123;

                /** Some command doc comment */
                command InOutStuff(InParam): OutParam = 222;
            }
        """, skip_meta=False)

        # meta_data may not match but is required for doc comments. Clean it up

        # Metadata parsing varies line/column, so only check doc comments
        self.assertIdlEqual(
            actual.clusters[0].description, "Documentation for MyCluster")
        self.assertIdlEqual(
            actual.clusters[1].description, "Documentation for MyCluster #2")
        self.assertIsNone(actual.clusters[1].commands[0].description)
        self.assertIdlEqual(
            actual.clusters[1].commands[1].description, "Some command doc comment")

    def test_sized_attribute(self):
        actual = parseText("""
            server cluster MyCluster = 1 {
                attribute char_string<11> attr1 = 1;
                attribute octet_string<33> attr2[] = 2;
            }
        """)

        expected = Idl(clusters=[
            Cluster(name="MyCluster",
                    code=1,
                    attributes=[
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="char_string", max_length=11), code=1, name="attr1")),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="octet_string", max_length=33), code=2, name="attr2", is_list=True)),
                    ]
                    )])
        self.assertIdlEqual(actual, expected)

    def test_timed_attributes(self):
        actual = parseText("""
            server cluster MyCluster = 1 {
                attribute int32u attr1 = 1;
                timedwrite attribute int32u attr2 = 2;
                attribute int32u attr3 = 3;
                timedwrite attribute octet_string<44> attr4[] = 4;
            }
        """)

        expected = Idl(clusters=[
            Cluster(name="MyCluster",
                    code=1,
                    attributes=[
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=1, name="attr1")),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE | AttributeQuality.TIMED_WRITE, definition=Field(
                            data_type=DataType(name="int32u"), code=2, name="attr2")),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=3, name="attr3")),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE | AttributeQuality.TIMED_WRITE, definition=Field(
                            data_type=DataType(name="octet_string", max_length=44), code=4, name="attr4", is_list=True)),
                    ]
                    )])
        self.assertIdlEqual(actual, expected)

    def test_attribute_access(self):
        actual = parseText("""
            server cluster MyCluster = 1 {
                attribute                                      int8s attr1 = 1;
                attribute access()                             int8s attr2 = 2;
                attribute access(read: manage)                 int8s attr3 = 3;
                attribute access(write: administer)            int8s attr4 = 4;
                attribute access(read: operate, write: manage) int8s attr5 = 5;
            }
        """)

        expected = Idl(clusters=[
            Cluster(name="MyCluster",
                    code=1,
                    attributes=[
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=1, name="attr1"),
                            readacl=AccessPrivilege.VIEW,
                            writeacl=AccessPrivilege.OPERATE
                        ),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=2, name="attr2"),
                            readacl=AccessPrivilege.VIEW,
                            writeacl=AccessPrivilege.OPERATE
                        ),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=3, name="attr3"),
                            readacl=AccessPrivilege.MANAGE
                        ),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=4, name="attr4"),
                            writeacl=AccessPrivilege.ADMINISTER
                        ),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int8s"), code=5, name="attr5"),
                            readacl=AccessPrivilege.OPERATE,
                            writeacl=AccessPrivilege.MANAGE
                        ),
                    ]
                    )])
        self.assertIdlEqual(actual, expected)

    def test_cluster_commands(self):
        actual = parseText("""
            server cluster WithCommands = 1 {
                struct FreeStruct {}
                request struct InParam {}
                response struct OutParam = 223 {}

                command WithoutArg(): DefaultSuccess = 123;
                command InOutStuff(InParam): OutParam = 222;
                timed command TimedCommand(InParam): DefaultSuccess = 0xab;
                fabric command FabricScopedCommand(InParam): DefaultSuccess = 0xac;
                fabric Timed command FabricScopedTimedCommand(InParam): DefaultSuccess = 0xad;
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="WithCommands",
                    code=1,
                    structs=[
                        Struct(name="FreeStruct", fields=[]),
                        Struct(name="InParam", fields=[],
                               tag=StructTag.REQUEST),
                        Struct(name="OutParam", fields=[],
                               tag=StructTag.RESPONSE, code=223),
                    ],
                    commands=[
                        Command(name="WithoutArg", code=123,
                                input_param=None, output_param="DefaultSuccess"),
                        Command(name="InOutStuff", code=222,
                                input_param="InParam", output_param="OutParam"),
                        Command(name="TimedCommand", code=0xab,
                                input_param="InParam", output_param="DefaultSuccess",
                                qualities=CommandQuality.TIMED_INVOKE),
                        Command(name="FabricScopedCommand", code=0xac,
                                input_param="InParam", output_param="DefaultSuccess",
                                qualities=CommandQuality.FABRIC_SCOPED),
                        Command(name="FabricScopedTimedCommand", code=0xad,
                                input_param="InParam", output_param="DefaultSuccess",
                                qualities=CommandQuality.TIMED_INVOKE | CommandQuality.FABRIC_SCOPED),
                    ],
                    )])
        self.assertIdlEqual(actual, expected)

    def test_cluster_command_access(self):
        actual = parseText("""
            server cluster WithCommands = 1 {
                request struct InParam {}
                response struct OutParam = 4 {}

                command WithoutArg(): DefaultSuccess = 1;
                timed command access(invoke: manage) TimedCommand(InParam): OutParam = 2;
                command access(invoke: administer) OutOnly(): OutParam = 3;
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="WithCommands",
                    code=1,
                    structs=[
                        Struct(name="InParam", fields=[],
                               tag=StructTag.REQUEST),
                        Struct(name="OutParam", fields=[],
                               tag=StructTag.RESPONSE, code=4),
                    ],
                    commands=[
                        Command(name="WithoutArg", code=1,
                                invokeacl=AccessPrivilege.OPERATE,
                                input_param=None, output_param="DefaultSuccess"),
                        Command(name="TimedCommand", code=2,
                                input_param="InParam", output_param="OutParam",
                                invokeacl=AccessPrivilege.MANAGE,
                                qualities=CommandQuality.TIMED_INVOKE),
                        Command(name="OutOnly", code=3,
                                input_param=None, output_param="OutParam",
                                invokeacl=AccessPrivilege.ADMINISTER,
                                ),
                    ],
                    )])
        self.assertIdlEqual(actual, expected)

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
            Cluster(name="WithEnums",
                    code=0xab,
                    enums=[
                        Enum(name="TestEnum", base_type="ENUM16",
                             entries=[
                                 ConstantEntry(name="A", code=0x123),
                                 ConstantEntry(name="B", code=0x234),
                             ])],
                    )])
        self.assertIdlEqual(actual, expected)

    def test_event_field_api_maturity(self):
        actual = parseText("""
            server cluster MaturityTest = 1 {
                critical event TestEvent = 123 {
                  nullable int16u someStableMember = 0;
                  provisional nullable int16u someProvisionalMember = 1;
                  internal nullable int16u someInternalMember = 2;
                }
           }
        """)
        expected = Idl(clusters=[
            Cluster(name="MaturityTest",
                    code=1,
                    events=[
                        Event(priority=EventPriority.CRITICAL, name="TestEvent", code=123, fields=[
                            Field(name="someStableMember", code=0, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE),
                            Field(name="someProvisionalMember", code=1, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE, api_maturity=ApiMaturity.PROVISIONAL),
                            Field(name="someInternalMember", code=2, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE, api_maturity=ApiMaturity.INTERNAL),

                        ]),
                    ],
                    )])
        self.assertIdlEqual(actual, expected)

    def test_enum_constant_maturity(self):
        actual = parseText("""
            client cluster WithEnums = 0xab {
                enum TestEnum : ENUM16 {
                    kStable = 0x123;
                    provisional kProvisional = 0x234;
                    internal kInternal = 0x345;
                }
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="WithEnums",
                    code=0xab,
                    enums=[
                        Enum(name="TestEnum", base_type="ENUM16",
                             entries=[
                                 ConstantEntry(name="kStable", code=0x123),
                                 ConstantEntry(
                                     name="kProvisional", code=0x234, api_maturity=ApiMaturity.PROVISIONAL),
                                 ConstantEntry(
                                     name="kInternal", code=0x345, api_maturity=ApiMaturity.INTERNAL),
                             ])],
                    )])
        self.assertIdlEqual(actual, expected)

    def test_bitmap_constant_maturity(self):
        actual = parseText("""
            client cluster Test = 0xab {
                bitmap TestBitmap : BITMAP32 {
                    kStable = 0x1;
                    internal kInternal = 0x2;
                    provisional kProvisional = 0x4;
                }
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="Test",
                    code=0xab,
                    bitmaps=[
                        Bitmap(name="TestBitmap", base_type="BITMAP32",
                               entries=[
                                   ConstantEntry(name="kStable", code=0x1),
                                   ConstantEntry(
                                       name="kInternal", code=0x2, api_maturity=ApiMaturity.INTERNAL),
                                   ConstantEntry(
                                       name="kProvisional", code=0x4, api_maturity=ApiMaturity.PROVISIONAL),
                               ])],
                    )])
        self.assertIdlEqual(actual, expected)

    def test_struct_field_api_maturity(self):
        actual = parseText("""
            server cluster MaturityTest = 1 {
                struct TestStruct {
                  nullable int16u someStableMember = 0;
                  provisional nullable int16u someProvisionalMember = 1;
                  internal nullable int16u someInternalMember = 2;
                }
           }
        """)
        expected = Idl(clusters=[
            Cluster(name="MaturityTest",
                    code=1,
                    structs=[
                        Struct(name="TestStruct", fields=[
                            Field(name="someStableMember", code=0, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE),
                            Field(name="someProvisionalMember", code=1, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE, api_maturity=ApiMaturity.PROVISIONAL),
                            Field(name="someInternalMember", code=2, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE, api_maturity=ApiMaturity.INTERNAL),

                        ]),
                    ],
                    )])
        self.assertIdlEqual(actual, expected)

    def test_cluster_entry_maturity(self):
        actual = parseText("""
            client cluster Test = 0xab {
                enum StableEnum : ENUM16 {}
                provisional enum ProvisionalEnum : ENUM16 {}
                internal enum InternalEnum : ENUM16 {}
                deprecated enum DeprecatedEnum : ENUM16 {}

                bitmap StableBitmap : BITMAP32 {}
                provisional bitmap ProvisionalBitmap : BITMAP32 {}
                internal bitmap InternalBitmap : BITMAP32 {}

                struct StableStruct {}
                provisional struct ProvisionalStruct {}
                internal struct InternalStruct {}

                info event StableEvent = 1 {}
                provisional info event ProvisionalEvent = 2 {}
                internal info event InternalEvent = 3 {}

                request struct StableCommandRequest {}
                response struct StableCommandResponse = 200 {}

                provisional request struct ProvisionalCommandRequest {}
                provisional response struct ProvisionalCommandResponse = 201 {}

                internal request struct InternalCommandRequest {}
                internal response struct InternalCommandResponse = 202 {}

                command StableCommand(StableCommandRequest): StableCommandResponse = 100;
                provisional command ProvisionalCommand(ProvisionalCommandRequest): ProvisionalCommandResponse = 101;
                internal command InternalCommand(InternalCommandRequest): InternalCommandResponse = 102;

                readonly attribute int8u roStable = 1;
                attribute int32u rwStable[] = 2;
                provisional readonly attribute int8u roProvisional = 11;
                provisional attribute int32u rwProvisional[] = 12;
                internal readonly attribute int8u roInternal = 21;
                internal attribute int32u rwInternal[] = 22;
                stable attribute int32u rwForcedStable[] = 31;
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="Test",
                    code=0xab,
                    enums=[
                        Enum(name="StableEnum", base_type="ENUM16", entries=[]),
                        Enum(name="ProvisionalEnum", base_type="ENUM16",
                             entries=[], api_maturity=ApiMaturity.PROVISIONAL),
                        Enum(name="InternalEnum", base_type="ENUM16",
                             entries=[], api_maturity=ApiMaturity.INTERNAL),
                        Enum(name="DeprecatedEnum", base_type="ENUM16",
                             entries=[], api_maturity=ApiMaturity.DEPRECATED),
                    ],
                    bitmaps=[
                        Bitmap(name="StableBitmap",
                               base_type="BITMAP32", entries=[]),
                        Bitmap(name="ProvisionalBitmap", base_type="BITMAP32",
                               entries=[], api_maturity=ApiMaturity.PROVISIONAL),
                        Bitmap(name="InternalBitmap", base_type="BITMAP32",
                               entries=[], api_maturity=ApiMaturity.INTERNAL),
                    ],
                    structs=[
                        Struct(name="StableStruct", fields=[]),
                        Struct(name="ProvisionalStruct", fields=[],
                               api_maturity=ApiMaturity.PROVISIONAL),
                        Struct(name="InternalStruct", fields=[],
                               api_maturity=ApiMaturity.INTERNAL),

                        Struct(name="StableCommandRequest",
                               fields=[], tag=StructTag.REQUEST),
                        Struct(name="StableCommandResponse", fields=[],
                               tag=StructTag.RESPONSE, code=200),
                        Struct(name="ProvisionalCommandRequest", fields=[
                        ], tag=StructTag.REQUEST, api_maturity=ApiMaturity.PROVISIONAL),
                        Struct(name="ProvisionalCommandResponse", fields=[
                        ], tag=StructTag.RESPONSE, code=201, api_maturity=ApiMaturity.PROVISIONAL),
                        Struct(name="InternalCommandRequest", fields=[
                        ], tag=StructTag.REQUEST, api_maturity=ApiMaturity.INTERNAL),
                        Struct(name="InternalCommandResponse", fields=[
                        ], tag=StructTag.RESPONSE, code=202, api_maturity=ApiMaturity.INTERNAL),
                    ],
                    events=[
                        Event(priority=EventPriority.INFO,
                              name="StableEvent", code=1, fields=[]),
                        Event(priority=EventPriority.INFO, name="ProvisionalEvent",
                              code=2, fields=[], api_maturity=ApiMaturity.PROVISIONAL),
                        Event(priority=EventPriority.INFO, name="InternalEvent",
                              code=3, fields=[], api_maturity=ApiMaturity.INTERNAL),
                    ],
                    commands=[
                        Command(name="StableCommand", code=100, input_param="StableCommandRequest",
                                output_param="StableCommandResponse"),
                        Command(name="ProvisionalCommand", code=101, input_param="ProvisionalCommandRequest",
                                output_param="ProvisionalCommandResponse", api_maturity=ApiMaturity.PROVISIONAL),
                        Command(name="InternalCommand", code=102, input_param="InternalCommandRequest",
                                output_param="InternalCommandResponse", api_maturity=ApiMaturity.INTERNAL),
                    ],
                    attributes=[
                        Attribute(qualities=AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="int8u"), code=1, name="roStable")),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=2, name="rwStable", is_list=True)),
                        Attribute(qualities=AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="int8u"), code=11, name="roProvisional"), api_maturity=ApiMaturity.PROVISIONAL),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=12, name="rwProvisional", is_list=True), api_maturity=ApiMaturity.PROVISIONAL),
                        Attribute(qualities=AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="int8u"), code=21, name="roInternal"), api_maturity=ApiMaturity.INTERNAL),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=22, name="rwInternal", is_list=True), api_maturity=ApiMaturity.INTERNAL),
                        Attribute(qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE, definition=Field(
                            data_type=DataType(name="int32u"), code=31, name="rwForcedStable", is_list=True), api_maturity=ApiMaturity.STABLE),
                    ]
                    )])
        self.assertIdlEqual(actual, expected)

    def test_cluster_bitmap(self):
        actual = parseText("""
            client cluster Test = 0xab {
                bitmap TestBitmap : BITMAP32 {
                    kFirst = 0x1;
                    kSecond = 0x2;
                }
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="Test",
                    code=0xab,
                    bitmaps=[
                        Bitmap(name="TestBitmap", base_type="BITMAP32",
                               entries=[
                                   ConstantEntry(name="kFirst", code=0x1),
                                   ConstantEntry(name="kSecond", code=0x2),
                               ])],
                    )])
        self.assertIdlEqual(actual, expected)

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
            Cluster(name="EventTester",
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
        self.assertIdlEqual(actual, expected)

    def test_cluster_event_acl(self):
        actual = parseText("""
            client cluster EventTester = 0x123 {
               info event Hello = 1 {}
               debug event access(read: manage) GoodBye = 2 {}
               debug event access(read: administer) AdminEvent = 3 {}
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="EventTester",
                    code=0x123,
                    events=[
                        Event(priority=EventPriority.INFO, readacl=AccessPrivilege.VIEW,
                              name="Hello", code=1, fields=[]),
                        Event(priority=EventPriority.DEBUG, readacl=AccessPrivilege.MANAGE,
                              name="GoodBye", code=2, fields=[]),
                        Event(priority=EventPriority.DEBUG, readacl=AccessPrivilege.ADMINISTER,
                              name="AdminEvent", code=3, fields=[]),
                    ])])
        self.assertIdlEqual(actual, expected)

    def test_fabric_sensitive_event(self):
        actual = parseText("""
            client cluster EventTester = 0x123 {
               fabric_sensitive info event Hello = 1 {}
               fabric_sensitive debug event access(read: manage) GoodBye = 2 {}
               fabric_sensitive debug event access(read: administer) AdminEvent = 3 {}
            }
        """)
        expected = Idl(clusters=[
            Cluster(name="EventTester",
                    code=0x123,
                    events=[
                        Event(priority=EventPriority.INFO, readacl=AccessPrivilege.VIEW,
                              name="Hello", code=1, fields=[], qualities=EventQuality.FABRIC_SENSITIVE),
                        Event(priority=EventPriority.DEBUG, readacl=AccessPrivilege.MANAGE,
                              name="GoodBye", code=2, fields=[], qualities=EventQuality.FABRIC_SENSITIVE),
                        Event(priority=EventPriority.DEBUG, readacl=AccessPrivilege.ADMINISTER,
                              name="AdminEvent", code=3, fields=[], qualities=EventQuality.FABRIC_SENSITIVE),
                    ])])
        self.assertIdlEqual(actual, expected)

    def test_parsing_metadata_for_cluster(self):
        actual = CreateParser(skip_meta=False).parse("""
server cluster A = 1 { /* Test comment */ }

// some empty lines and then indented
   client cluster B = 2 { }
        """)

        expected = Idl(clusters=[
            Cluster(parse_meta=ParseMetaData(line=2, column=1, start_pos=1),
                    name="A", code=1),
            Cluster(parse_meta=ParseMetaData(line=5, column=4, start_pos=87),
                    name="B", code=2),
        ])
        self.assertIdlEqual(actual, expected)

    def test_multiple_clusters(self):
        actual = parseText("""
            server cluster A = 1 { /* Test comment */ }
            client cluster B = 2 { }
            client cluster C = 3 { }
        """)

        expected = Idl(clusters=[
            Cluster(name="A", code=1),
            Cluster(name="B", code=2),
            Cluster(name="C", code=3),
        ])
        self.assertIdlEqual(actual, expected)

    def test_endpoints(self):
        actual = parseText("""
            endpoint 12 {
                device type foo = 123, version 1;
                device type bar = 0xFF, version 2;

                server cluster Foo { }
                server cluster Bar { }
                binding cluster Bar;
                binding cluster Test;
            }
        """)

        expected = Idl(endpoints=[Endpoint(number=12,
                                           device_types=[
                                               DeviceType(
                                                   name="foo", code=123, version=1),
                                               DeviceType(
                                                   name="bar", code=0xFF, version=2),
                                           ],
                                           server_clusters=[
                                               ServerClusterInstantiation(
                                                   name="Foo"),
                                               ServerClusterInstantiation(
                                                   name="Bar"),
                                           ],
                                           client_bindings=["Bar", "Test"],)
                                  ])
        self.assertIdlEqual(actual, expected)

    def test_cluster_instantiation(self):
        actual = parseText("""
            endpoint 3 {
                server cluster Example {
                    ram attribute inRamZero;
                    ram attribute inRamWithDefault default=123;
                    persist attribute inNVMNoDef;
                    persist attribute inNVMStr default="abc";
                    persist attribute inNVMWithDefault default = -33;
                    callback attribute hasCallbackBool default = true;
                }
            }
        """)

        expected = Idl(endpoints=[Endpoint(number=3,
                                           server_clusters=[
                                               ServerClusterInstantiation(name="Example", attributes=[
                                                   AttributeInstantiation(
                                                       name='inRamZero', storage=AttributeStorage.RAM),
                                                   AttributeInstantiation(name='inRamWithDefault',
                                                                          storage=AttributeStorage.RAM, default=123),
                                                   AttributeInstantiation(
                                                       name='inNVMNoDef', storage=AttributeStorage.PERSIST),
                                                   AttributeInstantiation(
                                                       name='inNVMStr', storage=AttributeStorage.PERSIST, default="abc"),
                                                   AttributeInstantiation(name='inNVMWithDefault',
                                                                          storage=AttributeStorage.PERSIST, default=-33),
                                                   AttributeInstantiation(name='hasCallbackBool',
                                                                          storage=AttributeStorage.CALLBACK, default=True),
                                               ]),
                                           ],
                                           client_bindings=[],)
                                  ])
        self.assertIdlEqual(actual, expected)

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
        self.assertIdlEqual(actual, expected)

    def test_cluster_api_maturity(self):
        actual = parseText("""
            provisional server cluster A = 1 { /* Test comment */ }
            internal client cluster B = 2 { }
            client cluster C = 3 { }
        """)

        expected = Idl(clusters=[
            Cluster(name="A", code=1, api_maturity=ApiMaturity.PROVISIONAL),
            Cluster(name="B", code=2, api_maturity=ApiMaturity.INTERNAL),
            Cluster(name="C", code=3),
        ])
        self.assertIdlEqual(actual, expected)

    def test_just_globals(self):
        actual = parseText("""
            enum TestEnum : ENUM16 { A = 0x123; B = 0x234; }
            bitmap TestBitmap : BITMAP32 {
                kStable = 0x1;
                internal kInternal = 0x2;
                provisional kProvisional = 0x4;
            }
            struct TestStruct {
               nullable int16u someStableMember = 0;
               provisional nullable int16u someProvisionalMember = 1;
               internal nullable int16u someInternalMember = 2;
            }
        """)

        expected = Idl(
            global_enums=[
                Enum(name="TestEnum", base_type="ENUM16",
                     entries=[
                         ConstantEntry(name="A", code=0x123),
                         ConstantEntry(name="B", code=0x234),
                     ],
                     is_global=True,
                     )],
            global_bitmaps=[
                Bitmap(name="TestBitmap", base_type="BITMAP32",
                       entries=[
                           ConstantEntry(name="kStable", code=0x1),
                           ConstantEntry(
                               name="kInternal", code=0x2, api_maturity=ApiMaturity.INTERNAL),
                           ConstantEntry(
                               name="kProvisional", code=0x4, api_maturity=ApiMaturity.PROVISIONAL),
                       ],
                       is_global=True,
                       )],
            global_structs=[
                Struct(name="TestStruct", fields=[
                            Field(name="someStableMember", code=0, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE),
                            Field(name="someProvisionalMember", code=1, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE, api_maturity=ApiMaturity.PROVISIONAL),
                            Field(name="someInternalMember", code=2, data_type=DataType(
                                name="int16u"), qualities=FieldQuality.NULLABLE, api_maturity=ApiMaturity.INTERNAL),

                ],
                    is_global=True,
                )],
        )
        self.assertIdlEqual(actual, expected)

    def test_cluster_reference_globals(self):
        actual = parseText("""
            enum TestEnum : ENUM16 {}
            bitmap TestBitmap : BITMAP32 {}
            struct TestStruct {}

            server cluster Foo = 1 {
                info event BitmapEvent = 0 {
                    TestBitmap someBitmap = 0;
                }
                struct MyStruct {
                    nullable TestStruct subStruct = 0;
                }
                readonly attribute TestEnum enumAttribute = 1;
            }
        """)

        global_enum = Enum(name="TestEnum", base_type="ENUM16", entries=[], is_global=True)
        global_bitmap = Bitmap(name="TestBitmap", base_type="BITMAP32", entries=[], is_global=True)
        global_struct = Struct(name="TestStruct", fields=[], is_global=True)
        expected = Idl(
            global_enums=[global_enum],
            global_bitmaps=[global_bitmap],
            global_structs=[global_struct],
            clusters=[
                Cluster(
                    name="Foo",
                    code=1,
                    enums=[global_enum],
                    bitmaps=[global_bitmap],
                    events=[
                        Event(priority=EventPriority.INFO,
                              name="BitmapEvent", code=0, fields=[
                                  Field(data_type=DataType(name="TestBitmap"),
                                        code=0, name="someBitmap"),
                              ]),
                    ],
                    structs=[
                        Struct(name="MyStruct", fields=[
                            Field(name="subStruct", code=0, data_type=DataType(name="TestStruct"), qualities=FieldQuality.NULLABLE), ],
                        ),
                        global_struct,
                    ],
                    attributes=[
                        Attribute(qualities=AttributeQuality.READABLE, definition=Field(
                            data_type=DataType(name="TestEnum"), code=1, name="enumAttribute")),
                    ],
                )
            ],
        )
        self.assertIdlEqual(actual, expected)

    def test_cluster_reference_globals_recursive(self):
        actual = parseText("""
            enum TestEnum : ENUM16 {}
            bitmap TestBitmap : BITMAP32 {}

            struct TestStruct1 {
                TestEnum enumField = 0;
            }

            struct TestStruct2 {
                TestStruct1 substruct = 0;
            }

            struct TestStruct3 {
                TestStruct2 substruct = 0;
                TestBitmap bmp = 1;
            }

            server cluster Foo = 1 {
                attribute TestStruct3 structAttr = 1;
            }
        """)

        global_enum = Enum(name="TestEnum", base_type="ENUM16", entries=[], is_global=True)
        global_bitmap = Bitmap(name="TestBitmap", base_type="BITMAP32", entries=[], is_global=True)
        global_struct1 = Struct(name="TestStruct1", fields=[
            Field(name="enumField", code=0, data_type=DataType(name="TestEnum")),

        ], is_global=True)
        global_struct2 = Struct(name="TestStruct2", fields=[
            Field(name="substruct", code=0, data_type=DataType(name="TestStruct1")),

        ], is_global=True)
        global_struct3 = Struct(name="TestStruct3", fields=[
            Field(name="substruct", code=0, data_type=DataType(name="TestStruct2")),
            Field(name="bmp", code=1, data_type=DataType(name="TestBitmap")),
        ], is_global=True)
        expected = Idl(
            global_enums=[global_enum],
            global_bitmaps=[global_bitmap],
            global_structs=[global_struct1, global_struct2, global_struct3],
            clusters=[
                Cluster(
                    name="Foo",
                    code=1,
                    enums=[global_enum],
                    bitmaps=[global_bitmap],
                    structs=[
                        global_struct3,
                        global_struct2,
                        global_struct1,
                    ],
                    attributes=[
                        Attribute(
                            qualities=AttributeQuality.READABLE | AttributeQuality.WRITABLE,
                            definition=Field(data_type=DataType(name="TestStruct3"), code=1, name="structAttr")),
                    ],
                )
            ],
        )
        self.assertIdlEqual(actual, expected)

    def test_emits_events(self):
        actual = parseText("""
            endpoint 1 {
                server cluster Example {}
            }
            endpoint 2 {
              server cluster Example {
                emits event FooBar;
                emits event SomeNewEvent;
              }
              server cluster AnotherExample {
                emits event StartUp;
                emits event ShutDown;
              }
            }
        """)

        expected = Idl(endpoints=[
            Endpoint(number=1, server_clusters=[
                     ServerClusterInstantiation(name="Example")]),
            Endpoint(number=2, server_clusters=[
                ServerClusterInstantiation(name="Example", events_emitted={
                                           "FooBar", "SomeNewEvent"}),
                ServerClusterInstantiation(name="AnotherExample", events_emitted={
                    "StartUp", "ShutDown"}),
            ])
        ])

        self.assertIdlEqual(actual, expected)

    def test_revision(self):
        actual = parseText("""
            server cluster A = 1 { } // revision 1 implied
            client cluster B = 2 { revision 1; }
            client cluster C = 3 { revision 2; }
            client cluster D = 4 { revision 123; }
        """)

        expected = Idl(clusters=[
            Cluster(name="A", code=1, revision=1),
            Cluster(name="B", code=2, revision=1),
            Cluster(name="C", code=3, revision=2),
            Cluster(name="D", code=4, revision=123),
        ])
        self.assertIdlEqual(actual, expected)

    def test_handle_commands(self):
        actual = parseText("""
            endpoint 1 {
                server cluster Example {}
            }
            endpoint 2 {
              server cluster Example {
                handle command TestCommand;
                handle command AnotherOne;
              }
              server cluster AnotherExample {
                handle command Xyz;
              }
            }
        """)

        expected = Idl(endpoints=[
            Endpoint(number=1, server_clusters=[
                     ServerClusterInstantiation(name="Example")]),
            Endpoint(number=2, server_clusters=[
                ServerClusterInstantiation(name="Example", commands=[
                    CommandInstantiation(name="TestCommand"),
                    CommandInstantiation(name="AnotherOne"),
                ]),
                ServerClusterInstantiation(name="AnotherExample", commands=[
                    CommandInstantiation(name="Xyz"),
                ]),
            ])
        ])

        self.assertIdlEqual(actual, expected)


if __name__ == '__main__':
    unittest.main()
