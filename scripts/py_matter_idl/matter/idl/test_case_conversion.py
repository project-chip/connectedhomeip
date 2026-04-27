#!/usr/bin/env python3
# Copyright (c) 2023 Project CHIP Authors
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

import unittest

import matter.idl.generators.filters as case_convert

'''
This file contains tests for checking five of the case conversion functions, notably: snake_case, CONSTANT_CASE, spinal-case, PascalCase and camelCase.


'''
inputs = [
    "FooBarBaz",
    "FOOBarBaz",
    "FOOBarBAZ",
    "fooBARBaz",
    "fooBarBAZ",
    "foo BarBaz",
    " FooBarBaz ",
    "foo_bar_baz",
    "FOO-bar-baz",
    "FOO_BAR_BAZ",
    "__FooBarBaz__",
    "_fooBar_Baz_",
    "foo_Bar Baz"
]


class TestSnakeCase(unittest.TestCase):
    def test_snake_case(self):
        expected = "foo_bar_baz"
        for input in inputs:
            converted = case_convert.to_snake_case(input)
            self.assertEqual(converted, expected, "they are not equal")

    def test_constant_case(self):
        expected = "FOO_BAR_BAZ"
        for input in inputs:
            converted = case_convert.to_constant_case(input)
            self.assertEqual(converted, expected, "they are not equal")

    def test_spinal_case(self):
        expected = "foo-bar-baz"
        for input in inputs:
            converted = case_convert.to_spinal_case(input)
            self.assertEqual(converted, expected, "they are not equal")

    def test_pascal_case(self):
        expected = "FooBarBaz"
        for input in inputs:
            converted = case_convert.to_pascal_case(input)
            self.assertEqual(converted, expected, "they are not equal")

    def test_camel_case(self):
        expected = "fooBarBaz"
        for input in inputs:
            converted = case_convert.to_camel_case(input)
            self.assertEqual(converted, expected, "they are not equal")


if __name__ == '__main__':
    unittest.main()
