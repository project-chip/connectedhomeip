
import unittest

import matter_idl.generators.filters as case_convert

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
