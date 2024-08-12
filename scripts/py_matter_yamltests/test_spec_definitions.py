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

import io
import unittest

from matter_yamltests.definitions import Attribute, Bitmap, Command, Enum, Event, ParseSource, SpecDefinitions, Struct

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

source_response_with_nullable = '''<?xml version="1.0"?>
  <configurator>
    <cluster>
      <name>Test</name>
      <code>0x1234</code>

      <command source="server" code="0x0" name="TestCommandResponse">
          <arg name="arg1" type="int8u"/>
          <arg name="arg2" type="int8u" isNullable="true"/>
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

      <event code="0x1" name="TestEventFabricScoped" priority="info" side="server" isFabricSensitive="true"></event>

    </cluster>
  </configurator>
'''

source_bitmap = '''<?xml version="1.0"?>
  <configurator>
    <bitmap name="TestGlobalBitmap" type="bitmap8">
      <field name="a" mask="0x1"/>
    </bitmap>

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
    <enum name="TestGlobalEnum" type="enum8">
      <item name="a" value="0x00"/>
    </enum>

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
    <struct name="TestGlobalStruct">
        <item name="a" type="boolean"/>
    </struct>

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

    def test_response_name_with_nullable(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_response_with_nullable), name='source_response_with_nullable')])
        cluster_name = 'Test'
        response_name = 'TestCommandResponse'

        self.assertEqual(definitions.get_cluster_name(0x1234), cluster_name)
        self.assertEqual(definitions.get_response_name(
            0x1234, 0x0), response_name)

        response = definitions.get_response_by_name(
            cluster_name, response_name)
        for field in response.fields:
            if field.name == 'arg1':
                self.assertFalse(definitions.is_nullable(field))
            else:
                self.assertTrue(definitions.is_nullable(field))

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
        self.assertIsNone(definitions.get_event_name(0x1234, 0x2))
        self.assertEqual(definitions.get_event_name(0x1234, 0x0), 'TestEvent')
        self.assertEqual(definitions.get_event_name(
            0x1234, 0x1), 'TestEventFabricScoped')

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
        self.assertIsNone(
            definitions.get_command_by_name('Test', 'testcommand'))

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
        self.assertIsNone(definitions.get_response_by_name(
            'Test', 'testcommandresponse'))

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
        self.assertIsNone(definitions.get_attribute_by_name(
            'Test', 'testattribute'))
        self.assertIsNone(definitions.get_attribute_by_name(
            'Test', 'testglobalattribute'))

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
        self.assertIsNone(definitions.get_event_by_name('Test', 'testevent'))

    def test_get_bitmap_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_bitmap), name='source_bitmap')])
        self.assertIsNone(definitions.get_bitmap_by_name(
            'WrongName', 'TestBitmap'))
        self.assertIsNone(definitions.get_bitmap_by_name(
            'TestWrong', 'TestBitmap'))
        self.assertIsNone(definitions.get_bitmap_by_name(
            'Test', 'TestWrongBitmap'))
        self.assertIsInstance(definitions.get_bitmap_by_name(
            'Test', 'TestBitmap'), Bitmap)
        self.assertIsInstance(definitions.get_bitmap_by_name(
            'Test', 'TestGlobalBitmap'), Bitmap)
        self.assertIsInstance(definitions.get_bitmap_by_name(
            'TestWrong', 'TestGlobalBitmap'), Bitmap)
        self.assertIsNone(definitions.get_bitmap_by_name('test', 'TestBitmap'))
        self.assertIsNone(definitions.get_bitmap_by_name('Test', 'testbitmap'))

    def test_get_enum_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_enum), name='source_enum')])
        self.assertIsNone(definitions.get_enum_by_name(
            'WrongName', 'TestEnum'))
        self.assertIsNone(definitions.get_enum_by_name(
            'TestWrong', 'TestEnum'))
        self.assertIsNone(definitions.get_enum_by_name(
            'Test', 'TestWrongEnum'))
        self.assertIsInstance(
            definitions.get_enum_by_name('Test', 'TestEnum'), Enum)
        self.assertIsInstance(
            definitions.get_enum_by_name('Test', 'TestGlobalEnum'), Enum)
        self.assertIsInstance(
            definitions.get_enum_by_name('TestWrong', 'TestGlobalEnum'), Enum)
        self.assertIsNone(definitions.get_enum_by_name('test', 'TestEnum'))
        self.assertIsNone(definitions.get_enum_by_name('Test', 'testenum'))

    def test_get_struct_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_struct), name='source_struct')])
        self.assertIsNone(definitions.get_struct_by_name(
            'WrongName', 'TestStruct'))
        self.assertIsNone(definitions.get_struct_by_name(
            'TestWrong', 'TestStruct'))
        self.assertIsNone(definitions.get_struct_by_name(
            'Test', 'TestWrongStruct'))
        self.assertIsInstance(definitions.get_struct_by_name(
            'Test', 'TestStruct'), Struct)
        self.assertIsInstance(definitions.get_struct_by_name(
            'Test', 'TestGlobalStruct'), Struct)
        self.assertIsInstance(definitions.get_struct_by_name(
            'TestWrong', 'TestGlobalStruct'), Struct)
        self.assertIsNone(definitions.get_struct_by_name('test', 'TestStruct'))
        self.assertIsNone(definitions.get_struct_by_name('Test', 'teststruct'))

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
        self.assertIsInstance(
            definitions.get_type_by_name('Test', 'TestEvent'), Event)

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_bitmap), name='source_bitmap')])
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestBitmap'), Bitmap)
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestGlobalBitmap'), Bitmap)

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_enum), name='source_enum')])
        self.assertIsInstance(
            definitions.get_type_by_name('Test', 'TestEnum'), Enum)
        self.assertIsInstance(
            definitions.get_type_by_name('Test', 'TestGlobalEnum'), Enum)

        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_struct), name='source_struct')])
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestStruct'), Struct)
        self.assertIsInstance(definitions.get_type_by_name(
            'Test', 'TestGlobalStruct'), Struct)

    def test_struct_is_fabric_scoped(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_struct), name='source_struct')])

        struct = definitions.get_struct_by_name('Test', 'TestStruct')
        self.assertFalse(definitions.is_fabric_scoped(struct))

        struct = definitions.get_struct_by_name(
            'Test', 'TestStructFabricScoped')
        self.assertTrue(definitions.is_fabric_scoped(struct))

    def test_event_is_fabric_scoped(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_event), name='source_event')])

        event = definitions.get_event_by_name('Test', 'TestEvent')
        self.assertFalse(definitions.is_fabric_scoped(event))

        event = definitions.get_event_by_name(
            'Test', 'TestEventFabricScoped')
        self.assertTrue(definitions.is_fabric_scoped(event))

    def test_get_cluster_id_by_name(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_cluster), name='source_cluster')])

        cluster_id = definitions.get_cluster_id_by_name('Test')
        self.assertEqual(cluster_id, 0x1234)

        cluster_id = definitions.get_cluster_id_by_name('test')
        self.assertIsNone(cluster_id)

    def test_get_command_names(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_command), name='source_command')])

        commands = definitions.get_command_names('Test')
        self.assertEqual(commands, ['TestCommand'])

        commands = definitions.get_command_names('test')
        self.assertEqual(commands, [])

    def test_get_attribute_names(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_attribute), name='source_attribute')])

        attributes = definitions.get_attribute_names('Test')
        self.assertEqual(attributes, ['TestAttribute', 'TestGlobalAttribute'])

        attributes = definitions.get_attribute_names('test')
        self.assertEqual(attributes, [])

    def test_get_event_names(self):
        definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(source_event), name='source_event')])

        events = definitions.get_event_names('Test')
        self.assertEqual(events, ['TestEvent', 'TestEventFabricScoped'])

        events = definitions.get_event_names('test')
        self.assertEqual(events, [])


if __name__ == '__main__':
    unittest.main()
