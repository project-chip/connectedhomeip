#
#    Copyright (c) 2026 Project CHIP Authors
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

import unittest

from matter.storage import _escapeIniKey, _unescapeIniKey


class TestIniKeyEscaping(unittest.TestCase):
    def test_escape_equals(self):
        # Session resumption keys are base64 encoded and may end with '=' padding.
        self.assertEqual(_escapeIniKey('g/s/abc=='), 'g/s/abc\\x3d\\x3d')

    def test_escape_special_chars(self):
        self.assertEqual(_escapeIniKey('a b\\c'), 'a\\x20b\\x5cc')
        self.assertEqual(_escapeIniKey('a\tb\nc'), 'a\\x09b\\x0ac')
        self.assertEqual(_escapeIniKey('a\x7fb'), 'a\\x7fb')

    def test_escape_non_ascii(self):
        # Non-ASCII characters are escaped per UTF-8 byte, matching chip-tool.
        self.assertEqual(_escapeIniKey('café'), 'caf\\xc3\\xa9')
        self.assertEqual(_unescapeIniKey('caf\\xc3\\xa9'), 'café')

    def test_no_escape_needed(self):
        for key in ['f/1/n', 'g/fidx', 'ExampleOpCredsCAKey0', 'g/s/XAZyZmxGULaZfTMiAvOFbA+/']:
            self.assertEqual(_escapeIniKey(key), key)
            self.assertEqual(_unescapeIniKey(key), key)

    def test_unescape_chip_tool_key(self):
        # Key as written by chip-tool (src/lib/support/IniEscaping.cpp).
        self.assertEqual(_unescapeIniKey('g/s/XAZyZmxGULaZfTMiAvOFbA\\x3d\\x3d'), 'g/s/XAZyZmxGULaZfTMiAvOFbA==')

    def test_round_trip(self):
        for key in ['g/s/abc==', 'a b\\c', 'a\tb\nc', '\\x3d', 'f/1/n', '', 'café', '日本', '\U0001f600']:
            self.assertEqual(_unescapeIniKey(_escapeIniKey(key)), key)


if __name__ == '__main__':
    unittest.main()
