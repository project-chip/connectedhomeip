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

import unittest
from unittest.mock import mock_open, patch

from matter_yamltests.pics_checker import InvalidPICSConfigurationError, InvalidPICSConfigurationValueError, PICSChecker

empty_config = ''

simple_config = '''
A.A=0
A.B=1
A.C=0
'''

simple_config_with_whitespaces_and_control_characters = '''
 A.A=0 \n
\tA.B = 1
'''

simple_config_with_comments = '''
# This is a comment
A.A=0
# This is an other comment
A.B=1
A.C=1 # This is a comment
'''

simple_config_with_invalid_entry_1 = '''
A.A=FOO
'''

simple_config_with_invalid_entry_2 = '''
A.B=2
'''

simple_config_with_invalid_entry_3 = '''
A.C=
'''

simple_config_with_invalid_entry_4 = '''
A.E==
'''

simple_config_with_invalid_entry_5 = '''
A.D
'''

simple_config_with_invalid_entry_6 = '''
A.D=1=
'''

simple_real_config = '''
# Color Control Cluster
CC.S.F00=0
CC.S.F01=1
CC.S.F02=1
CC.S.F03=1
CC.S.F04=1
'''


class TestPICSChecker(unittest.TestCase):
    def test_no_file(self):
        pics_checker = PICSChecker(None)
        self.assertIsInstance(pics_checker, PICSChecker)
        self.assertFalse(pics_checker.check('A.A'))

    @patch('builtins.open', mock_open(read_data=empty_config))
    def test_empty_config(self):
        pics_checker = PICSChecker('')
        self.assertIsInstance(pics_checker, PICSChecker)

    @patch('builtins.open', mock_open(read_data=simple_config))
    def test_simple_config(self):
        pics_checker = PICSChecker('')

        self.assertFalse(pics_checker.check('A.A'))
        self.assertFalse(pics_checker.check('A.DoesNotExist'))

        self.assertTrue(pics_checker.check('A.B'))
        self.assertTrue(pics_checker.check('A.b'))
        self.assertTrue(pics_checker.check('a.b'))
        self.assertTrue(pics_checker.check(' A.B'))
        self.assertTrue(pics_checker.check('A.B '))

    @patch('builtins.open', mock_open(read_data=simple_config))
    def test_logic_negation(self):
        pics_checker = PICSChecker('')
        self.assertFalse(pics_checker.check('A.C'))
        self.assertTrue(pics_checker.check('!A.C'))
        self.assertFalse(pics_checker.check('!!A.C'))
        self.assertTrue(pics_checker.check('!!!A.C'))

    @patch('builtins.open', mock_open(read_data=simple_config))
    def test_logical_and(self):
        pics_checker = PICSChecker('')
        self.assertFalse(pics_checker.check('A.C && A.B'))
        self.assertFalse(pics_checker.check('A.C && A.B'))
        self.assertTrue(pics_checker.check('!A.A && A.B'))

    @patch('builtins.open', mock_open(read_data=simple_config))
    def test_logical_or(self):
        pics_checker = PICSChecker('')
        self.assertFalse(pics_checker.check('A.A || A.C'))
        self.assertTrue(pics_checker.check('A.B || A.C'))
        self.assertTrue(pics_checker.check('!A.A || A.C'))
        self.assertTrue(pics_checker.check('A.A || A.B || A.C'))

    @patch('builtins.open', mock_open(read_data=simple_config))
    def test_logical_parenthesis(self):
        pics_checker = PICSChecker('')
        self.assertFalse(pics_checker.check('(A.A)'))
        self.assertTrue(pics_checker.check('(A.B)'))
        self.assertTrue(pics_checker.check('!(A.A)'))
        self.assertTrue(pics_checker.check('(!(A.A))'))
        self.assertFalse(pics_checker.check('(A.A && A.B)'))
        self.assertFalse(pics_checker.check('((A.A) && (A.B))'))
        self.assertTrue(pics_checker.check('(!A.A && A.B)'))
        self.assertTrue(pics_checker.check('(!(A.A) && (A.B))'))
        self.assertTrue(pics_checker.check('(A.A || A.B)'))
        self.assertFalse(pics_checker.check('A.C && (A.A || A.B)'))
        self.assertTrue(pics_checker.check('A.B || (A.A || A.C)'))
        self.assertTrue(pics_checker.check('!A.C && (A.A || A.B)'))
        self.assertTrue(pics_checker.check('!A.C || (A.A && A.B)'))

    @patch('builtins.open', mock_open(read_data=simple_config_with_whitespaces_and_control_characters))
    def test_simple_config_with_whitespaces_and_control_characters(self):
        pics_checker = PICSChecker('')
        self.assertFalse(pics_checker.check('A.A'))
        self.assertTrue(pics_checker.check('A.B'))

    @patch('builtins.open', mock_open(read_data=simple_config_with_comments))
    def test_simple_config_with_comments(self):
        pics_checker = PICSChecker('')
        self.assertFalse(pics_checker.check('A.A'))
        self.assertTrue(pics_checker.check('A.B'))
        self.assertTrue(pics_checker.check('A.C'))

    def test_simple_config_with_invalid_entry(self):
        with patch("builtins.open", mock_open(read_data=simple_config_with_invalid_entry_1)) as mock_file:
            self.assertRaises(
                InvalidPICSConfigurationValueError, PICSChecker, mock_file)

        with patch("builtins.open", mock_open(read_data=simple_config_with_invalid_entry_2)) as mock_file:
            self.assertRaises(
                InvalidPICSConfigurationValueError, PICSChecker, mock_file)

        with patch("builtins.open", mock_open(read_data=simple_config_with_invalid_entry_3)) as mock_file:
            self.assertRaises(
                InvalidPICSConfigurationValueError, PICSChecker, mock_file)

        with patch("builtins.open", mock_open(read_data=simple_config_with_invalid_entry_4)) as mock_file:
            self.assertRaises(InvalidPICSConfigurationError,
                              PICSChecker, mock_file)

        with patch("builtins.open", mock_open(read_data=simple_config_with_invalid_entry_5)) as mock_file:
            self.assertRaises(InvalidPICSConfigurationError,
                              PICSChecker, mock_file)

        with patch("builtins.open", mock_open(read_data=simple_config_with_invalid_entry_6)) as mock_file:
            self.assertRaises(InvalidPICSConfigurationError,
                              PICSChecker, mock_file)

    @patch('builtins.open', mock_open(read_data=simple_real_config))
    def test_simple_real_config(self):
        pics_checker = PICSChecker('')
        self.assertIsInstance(pics_checker, PICSChecker)
        self.assertFalse(pics_checker.check(
            '( !CC.S.F00 && !CC.S.F01 && !CC.S.F02 && !CC.S.F03 && !CC.S.F04 )'))


if __name__ == '__main__':
    unittest.main()
