#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# TODO Once yamltest is a proper self contained module we can move this file
# to a more appropriate spot. For now, having this file to do some quick checks
# is arguably better then no checks at all.

import io
import tempfile
import unittest

from matter_yamltests.definitions import *
from matter_yamltests.parser import TestParser

simple_test_description = '''<?xml version="1.0"?>
  <configurator>
    <struct name="TestStruct">
        <cluster code="0x1234"/>
        <item name="a" type="boolean"/>
    </struct>

    <cluster>
      <name>Test</name>
      <code>0x1234</code>
    </cluster>

  </configurator>
'''

simple_test_yaml = '''
name: Test Cluster Tests

config:
    nodeId: 0x12344321
    cluster: "Test"
    endpoint: 1

tests:
    - label: "Send Test Command"
      command: "test"

    - label: "Send Test Not Handled Command"
      command: "testNotHandled"
      response:
          error: INVALID_COMMAND

    - label: "Send Test Specific Command"
      command: "testSpecific"
      response:
          values:
              - name: "returnValue"
                value: 7
'''


class TestYamlParser(unittest.TestCase):
    def setUp(self):
        self._definitions = SpecDefinitions(
            [ParseSource(source=io.StringIO(simple_test_description), name='simple_test_description')])
        self._temp_file = tempfile.NamedTemporaryFile(suffix='.yaml')
        with open(self._temp_file.name, 'w') as f:
            f.writelines(simple_test_yaml)
        pics_file = None
        self._yaml_parser = TestParser(
            self._temp_file.name, pics_file, self._definitions)

    def test_able_to_iterate_over_all_parsed_tests(self):
        # self._yaml_parser.tests implements `__next__`, which does value substitution. We are
        # simply ensure there is no exceptions raise.
        count = 0
        for idx, test_step in enumerate(self._yaml_parser.tests):
            count += 1
            pass
        self.assertEqual(count, 3)


def main():
    unittest.main()


if __name__ == '__main__':
    main()
