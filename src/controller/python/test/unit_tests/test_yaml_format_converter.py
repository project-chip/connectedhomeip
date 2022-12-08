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

from chip.yaml.format_converter import convert_yaml_octet_string_to_bytes, substitute_in_config_variables
from binascii import unhexlify
import unittest


class TestOctetStringYamlDecode(unittest.TestCase):
    def test_common_cases(self):
        self.assertEqual(convert_yaml_octet_string_to_bytes("hex:aa55"), unhexlify("aa55"))
        self.assertEqual(convert_yaml_octet_string_to_bytes("hex:"), unhexlify(""))
        self.assertEqual(convert_yaml_octet_string_to_bytes("hex:AA55"), unhexlify("aa55"))

        self.assertEqual(convert_yaml_octet_string_to_bytes("0\xaa\x55"), unhexlify("30aa55"))
        self.assertEqual(convert_yaml_octet_string_to_bytes("0\xAA\x55"), unhexlify("30aa55"))
        self.assertEqual(convert_yaml_octet_string_to_bytes("0\xAa\x55"), unhexlify("30aa55"))

        self.assertEqual(convert_yaml_octet_string_to_bytes("0hex:"), b"0hex:")
        self.assertEqual(convert_yaml_octet_string_to_bytes("0hex:A"), b"0hex:A")
        self.assertEqual(convert_yaml_octet_string_to_bytes("0hex:AA55"), b"0hex:AA55")

        self.assertEqual(convert_yaml_octet_string_to_bytes("AA55"), b"AA55")
        self.assertEqual(convert_yaml_octet_string_to_bytes("AA\n\r\t55"), unhexlify("41410a0d093535"))
        # TODO(#23669): After utf8 is properly supported expected result is unhexlify("c3a9c3a90a0a")
        self.assertEqual(convert_yaml_octet_string_to_bytes("\xC3\xA9é\n\n"), unhexlify("c3a9e90a0a"))

        # Partial hex nibble
        with self.assertRaises(ValueError):
            convert_yaml_octet_string_to_bytes("hex:aa5")


class TestSubstitueInConfigVariables(unittest.TestCase):

    def setUp(self):
        self.common_config = {
            'arg1': {
                'defaultValue': 1
            },
            'arg2': {
                'defaultValue': 2
            },
            'no_explicit_default': 3
        }

    def test_basic_substitution(self):
        self.assertEqual(substitute_in_config_variables('arg1', self.common_config), 1)
        self.assertEqual(substitute_in_config_variables('arg2', self.common_config), 2)
        self.assertEqual(substitute_in_config_variables('arg3', self.common_config), 'arg3')
        self.assertEqual(substitute_in_config_variables('no_explicit_default', self.common_config), 3)

    def test_basis_dict_substitution(self):
        basic_dict = {
            'arg1': 'arg1',
            'arg2': 'arg2',
            'arg3': 'arg3',
            'no_explicit_default': 'no_explicit_default',
        }
        expected_dict = {
            'arg1': 1,
            'arg2': 2,
            'arg3': 'arg3',
            'no_explicit_default': 3,
        }
        self.assertEqual(substitute_in_config_variables(basic_dict, self.common_config), expected_dict)

    def test_basis_list_substitution(self):
        basic_list = ['arg1', 'arg2', 'arg3', 'no_explicit_default']
        expected_list = [1, 2, 'arg3', 3]
        self.assertEqual(substitute_in_config_variables(basic_list, self.common_config), expected_list)

    def test_complex_nested_type(self):
        complex_nested_type = {
            'arg1': ['arg1', 'arg2', 'arg3', 'no_explicit_default'],
            'arg2': 'arg22',
            'arg3': {
                'no_explicit_default': 'no_explicit_default',
                'arg2': 'arg2',
                'another_dict': {
                    'arg1': ['arg1', 'arg1', 'arg1', 'no_explicit_default'],
                },
                'another_list': ['arg1', 'arg2', 'arg3', 'no_explicit_default']
            },
            'no_explicit_default': 'no_explicit_default',
        }
        expected_result = {
            'arg1': [1, 2, 'arg3', 3],
            'arg2': 'arg22',
            'arg3': {
                'no_explicit_default': 3,
                'arg2': 2,
                'another_dict': {
                    'arg1': [1, 1, 1, 3],
                },
                'another_list': [1, 2, 'arg3', 3]
            },
            'no_explicit_default': 3,
        }
        self.assertEqual(substitute_in_config_variables(complex_nested_type, self.common_config), expected_result)


def main():
    unittest.main()


if __name__ == "__main__":
    main()
