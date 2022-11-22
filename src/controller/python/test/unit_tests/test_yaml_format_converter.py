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

from chip.yaml.format_converter import convert_yaml_octet_string_to_bytes
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


def main():
    unittest.main()


if __name__ == "__main__":
    main()
