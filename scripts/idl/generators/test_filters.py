# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import unittest
import yaml
import logging
from typing import List
from dataclasses import dataclass, field

try:
    from idl.generators.filters import *
except:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..', '..')))
    from idl.generators.filters import *


class TestFilders(unittest.TestCase):
    def test_as_upper_camel_case(self):
        self.assertEqual(asUpperCamelCase('foo'), 'Foo')

        # basic separators and casing
        self.assertEqual(asUpperCamelCase('foo-bar'), 'FooBar')
        self.assertEqual(asUpperCamelCase('fOo-bAr-BaZ'), 'FooBarBaz')
        self.assertEqual(asUpperCamelCase('fOo bAr/BaZ'), 'FooBarBaz')
        self.assertEqual(asUpperCamelCase('fOo_bAr BaZ'), 'FooBarBaz')

        # handling of acronyms
        self.assertEqual(asUpperCamelCase('FOO_bAr BaZ'), 'FOOBarBaz')
        self.assertEqual(asUpperCamelCase('FOO_bAr/BaZ', preserve_acronyms=False), 'FooBarBaz')
        self.assertEqual(asUpperCamelCase('fOo-BAR/BAZ'), 'FooBARBAZ')
        self.assertEqual(asUpperCamelCase('fOo/BAR-BAZ', preserve_acronyms=False), 'FooBarBaz')

    def test_as_lower_camel_case(self):
        self.assertEqual(asLowerCamelCase('foo'), 'foo')
        self.assertEqual(asLowerCamelCase('fOo'), 'foo')

        # basic separators and casing
        self.assertEqual(asLowerCamelCase('foo-bar'), 'fooBar')
        self.assertEqual(asLowerCamelCase('fOo-bAr-BaZ'), 'fooBarBaz')
        self.assertEqual(asLowerCamelCase('fOo bAr/BaZ'), 'fooBarBaz')
        self.assertEqual(asLowerCamelCase('fOo_bAr BaZ'), 'fooBarBaz')

        # handling of acronyms
        self.assertEqual(asLowerCamelCase('FOO_bAr BaZ'), 'FOOBarBaz')
        self.assertEqual(asLowerCamelCase('FOO_bAr/BaZ', preserve_acronyms=False), 'fooBarBaz')
        self.assertEqual(asLowerCamelCase('fOo-BAR/BAZ'), 'fooBARBAZ')
        self.assertEqual(asLowerCamelCase('fOo/BAR-BAZ', preserve_acronyms=False), 'fooBarBaz')
        self.assertEqual(asLowerCamelCase('FOO-BAR/BAZ'), 'FOOBARBAZ')
        self.assertEqual(asLowerCamelCase('FOO/BAR-BAZ', preserve_acronyms=False), 'fooBarBaz')


if __name__ == '__main__':
    unittest.main()
