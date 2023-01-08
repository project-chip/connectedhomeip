#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2022 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

from matter_yamltests.definitions import *

import unittest
import io

source_cluster = '''<?xml version="1.0"?>
  <configurator>
    <cluster>
      <name>Test</name>
      <code>0x1234</code>
    </cluster>
  </configurator>
'''

source_command = '''<?xml version="1.0"?>
  <configurator>
    <cluster>
      <name>Test</name>
      <code>0x1234</code>

      <command source="client" code="0x0" name="TestCommand"></command>

    </cluster>
  </configurator>
'''

source_response = '''<?xml version="1.0"?>
  <configurator>
    <cluster>
      <name>Test</name>
      <code>0x1234</code>

      <command source="server" code="0x0" name="TestCommandResponse">
          <arg name="arg1" type="int8u"/>
      </command>

    </cluster>
  </configurator>
'''

source_attribute = '''<?xml version="1.0"?>
  <configurator>
    <global>
      <attribute side="server" code="0xFFFD" type="boolean">TestGlobalAttribute</attribute>
    </global>

    <cluster>
      <name>Test</name>
      <code>0x1234</code>

      <globalAttribute side="server" code="0xFFFD" value="true"/>
      <attribute code="0x0" type="boolean">TestAttribute</attribute>

    </cluster>
  </configurator>
'''

source_event = '''<?xml version="1.0"?>
  <configurator>
    <cluster>
      <name>Test</name>
      <code>0x1234</code>

      <event code="0x0" name="TestEvent" priority="info" side="server"></event>

    </cluster>
  </configurator>
'''

source_bitmap = '''<?xml version="1.0"?>
  <configurator>
    <bitmap name="TestBitmap" type="bitmap8">
      <cluster code="0x1234"/>
      <field name="a" mask="0x1"/>
    </bitmap>

    <bitmap name="TestWrongBitmap" type="bitmap8">
      <cluster code="0x4321"/>
      <field name="a" mask="0x1"/>
    </bitmap>

    <cluster>
      <name>Test</name>
      <code>0x1234</code>
    </cluster>

    <cluster>
      <name>TestWrong</name>
      <code>0x4321</code>
    </cluster>
  </configurator>
'''

source_enum = '''<?xml version="1.0"?>
  <configurator>
    <enum name="TestEnum" type="enum8">
      <cluster code="0x1234"/>
      <item name="a" value="0x00"/>
    </enum>

    <enum name="TestWrongEnum" type="enum8">
      <cluster code="0x4321"/>
      <item name="a" value="0x00"/>
    </enum>

    <cluster>
      <name>Test</name>
      <code>0x1234</code>
    </cluster>

    <cluster>
      <name>TestWrong</name>
      <code>0x4321</code>
    </cluster>
  </configurator>
'''

source_struct = '''<?xml version="1.0"?>
  <configurator>
    <struct name="TestStruct">
        <cluster code="0x1234"/>
        <item name="a" type="boolean"/>
    </struct>

    <struct name="TestStructFabricScoped" isFabricScoped="true">
        <cluster code="0x1234"/>
        <item name="a" type="boolean"/>
    </struct>

    <struct name="TestWrongStruct">
        <cluster code="0x4321"/>
        <item name="a" type="boolean"/>
    </struct>

    <cluster>
      <name>Test</name>
      <code>0x1234</code>
    </cluster>

    <cluster>
      <name>TestWrong</name>
      <code>0x4321</code>
    </cluster>
  </configurator>
'''


class TestSpecDefinitions(unittest.TestCase):
    def test_cluster_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_cluster), name='source_cluster')])
        self.assertIsNone(definitions.get_cluster_name(0x4321))
        self.assertEqual(definitions.get_cluster_name(0x1234), 'Test')

    def test_command_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_command), name='source_command')])
        self.assertIsNone(definitions.get_command_name(0x4321, 0x0))
        self.assertIsNone(definitions.get_command_name(0x1234, 0x1))
        self.assertEqual(definitions.get_command_name(
            0x1234, 0x0), 'TestCommand')

    def test_response_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_response), name='source_response')])
        self.assertIsNone(definitions.get_response_name(0x4321, 0x0))
        self.assertIsNone(definitions.get_response_name(0x1234, 0x1))
        self.assertEqual(definitions.get_response_name(
            0x1234, 0x0), 'TestCommandResponse')

    def test_attribute_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_attribute), name='source_attribute')])
        self.assertIsNone(definitions.get_attribute_name(0x4321, 0x0))
        self.assertIsNone(definitions.get_attribute_name(0x4321, 0xFFFD))
        self.assertIsNone(definitions.get_attribute_name(0x1234, 0x1))
        self.assertEqual(definitions.get_attribute_name(
            0x1234, 0x0), 'TestAttribute')
        self.assertEqual(definitions.get_attribute_name(
            0x1234, 0xFFFD), 'TestGlobalAttribute')

    def test_event_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_event), name='source_event')])
        self.assertIsNone(definitions.get_event_name(0x4321, 0x0))
        self.assertIsNone(definitions.get_event_name(0x1234, 0x1))
        self.assertEqual(definitions.get_event_name(0x1234, 0x0), 'TestEvent')

    def test_get_command_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_command), name='source_command')])
        self.assertIsNone(definitions.get_command_by_name(
            'WrongName', 'TestCommand'))
        self.assertIsNone(definitions.get_command_by_name(
            'Test', 'TestWrongCommand'))
        self.assertIsNone(
            definitions.get_response_by_name('Test', 'TestCommand'))
        self.assertIsInstance(definitions.get_command_by_name(
            'Test', 'TestCommand'), Command)
        self.assertIsNone(
            definitions.get_command_by_name('test', 'TestCommand'))
        self.assertIsInstance(definitions.get_command_by_name(
            'Test', 'testcommand'), Command)

    def test_get_response_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_response), name='source_response')])
        self.assertIsNone(definitions.get_response_by_name(
            'WrongName', 'TestCommandResponse'))
        self.assertIsNone(definitions.get_response_by_name(
            'Test', 'TestWrongCommandResponse'))
        self.assertIsNone(definitions.get_command_by_name(
            'Test', 'TestCommandResponse'))
        self.assertIsInstance(definitions.get_response_by_name(
            'Test', 'TestCommandResponse'), Struct)
        self.assertIsNone(definitions.get_response_by_name(
            'test', 'TestCommandResponse'))
        self.assertIsInstance(definitions.get_response_by_name(
            'Test', 'testcommandresponse'), Struct)

    def test_get_attribute_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_attribute), name='source_attribute')])
        self.assertIsNone(definitions.get_attribute_by_name(
            'WrongName', 'TestAttribute'))
        self.assertIsNone(definitions.get_attribute_by_name(
            'WrongName', 'TestGlobalAttribute'))
        self.assertIsNone(definitions.get_attribute_by_name(
            'Test', 'TestWrongAttribute'))
        self.assertIsInstance(definitions.get_attribute_by_name(
            'Test', 'TestAttribute'), Attribute)
        self.assertIsInstance(definitions.get_attribute_by_name(
            'Test', 'TestGlobalAttribute'), Attribute)
        self.assertIsNone(definitions.get_attribute_by_name(
            'test', 'TestAttribute'))
        self.assertIsNone(definitions.get_attribute_by_name(
            'test', 'TestGlobalAttribute'))
        self.assertIsInstance(definitions.get_attribute_by_name(
            'Test', 'testattribute'), Attribute)
        self.assertIsInstance(definitions.get_attribute_by_name(
            'Test', 'testglobalattribute'), Attribute)

    def test_get_event_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_event), name='source_event')])
        self.assertIsNone(definitions.get_event_by_name(
            'WrongName', 'TestEvent'))
        self.assertIsNone(definitions.get_event_by_name(
            'Test', 'TestWrongEvent'))
        self.assertIsInstance(
            definitions.get_event_by_name('Test', 'TestEvent'), Event)
        self.assertIsNone(definitions.get_event_by_name('test', 'TestEvent'))
        self.assertIsInstance(
            definitions.get_event_by_name('Test', 'testevent'), Event)

    def test_get_bitmap_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_bitmap), name='source_bitmap')])
        self.assertIsNone(definitions.get_bitmap_by_name(
            'WrongName', 'TestBitmap'))
        self.assertIsNone(definitions.get_bitmap_by_name(
            'Test', 'TestWrongBitmap'))
        self.assertIsInstance(definitions.get_bitmap_by_name(
            'Test', 'TestBitmap'), Bitmap)
        self.assertIsNone(definitions.get_bitmap_by_name('test', 'TestBitmap'))
        self.assertIsInstance(definitions.get_bitmap_by_name(
            'Test', 'testbitmap'), Bitmap)

    def test_get_enum_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_enum), name='source_enum')])
        self.assertIsNone(definitions.get_enum_by_name(
            'WrongName', 'TestEnum'))
        self.assertIsNone(definitions.get_enum_by_name(
            'Test', 'TestWrongEnum'))
        self.assertIsInstance(
            definitions.get_enum_by_name('Test', 'TestEnum'), Enum)
        self.assertIsNone(definitions.get_enum_by_name('test', 'TestEnum'))
        self.assertIsInstance(
            definitions.get_enum_by_name('Test', 'testenum'), Enum)

    def test_get_struct_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_struct), name='source_struct')])
        self.assertIsNone(definitions.get_struct_by_name(
            'WrongName', 'TestStruct'))
        self.assertIsNone(definitions.get_struct_by_name(
            'Test', 'TestWrongStruct'))
        self.assertIsInstance(definitions.get_struct_by_name(
            'Test', 'TestStruct'), Struct)
        self.assertIsNone(definitions.get_struct_by_name('test', 'TestStruct'))
        self.assertIsInstance(definitions.get_struct_by_name(
            'Test', 'teststruct'), Struct)

    def test_get_type_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_command), name='source_command')])
        self.assertIsNone(definitions.get_type_by_name('Test', 'TestCommand'))

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_response), name='source_response')])
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestCommandResponse'), Struct)

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_attribute), name='source_attribute')])
        self.assertIsNone(definitions.get_type_by_name(
            'Test', 'TestAttribute'))

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_event), name='source_event')])
        self.assertIsNone(definitions.get_type_by_name('Test', 'TestEvent'))

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_bitmap), name='source_bitmap')])
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestBitmap'), Bitmap)

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_enum), name='source_enum')])
        self.assertIsInstance(
            definitions.get_type_by_name('Test', 'TestEnum'), Enum)

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_struct), name='source_struct')])
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestStruct'), Struct)

    def test_is_fabric_scoped(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_struct), name='source_struct')])

        struct = definitions.get_struct_by_name('Test', 'TestStruct')
        self.assertFalse(definitions.is_fabric_scoped(struct))

        struct = definitions.get_struct_by_name(
            'Test', 'TestStructFabricScoped')
        self.assertTrue(definitions.is_fabric_scoped(struct))


if __name__ == '__main__':
    unittest.main()
