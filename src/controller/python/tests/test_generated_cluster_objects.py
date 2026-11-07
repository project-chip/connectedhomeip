import unittest

from rich.pretty import pprint

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.tlv import TLVReader

'''
This file contains tests for validating the generated cluster objects by running encoding and decoding
through them symmetrically to exercise both pathways.
'''

enable_debug = False


class TestGeneratedClusterObjects(unittest.TestCase):
    def CheckData(self, expected):
        tlv = expected.ToTLV()
        actual = expected.FromTLV(tlv)

        if (enable_debug):
            print("Expected Data:")
            pprint(expected, expand_all=True)

            print("Actual Data:")
            pprint(actual, expand_all=True)

        self.assertEqual(actual, expected)

    def test_simple_struct(self):
        data = Clusters.UnitTesting.Structs.SimpleStruct()
        data.a = 23
        data.b = True
        data.c = Clusters.UnitTesting.Enums.SimpleEnum.kValueA
        data.d = b'1234'
        data.e = 'hello'
        data.f = 1
        data.g = 0
        data.h = 0

        self.CheckData(data)

    def test_content_app_message_without_encoding_hint(self) -> None:
        # Matter 1.6.1 section 6.12.5.1 requires Data; EncodingHint is optional.
        for data in ('', 'payload'):
            with self.subTest(data=data):
                request = Clusters.ContentAppObserver.Commands.ContentAppMessage(data=data)
                self.assertEqual(TLVReader(request.ToTLV()).get()['Any'], {0: data})
                self.CheckData(request)

    def test_content_app_message_with_encoding_hint(self) -> None:
        for hint in ('', 'text/plain'):
            with self.subTest(encoding_hint=hint):
                request = Clusters.ContentAppObserver.Commands.ContentAppMessage(data='payload', encodingHint=hint)
                self.assertEqual(TLVReader(request.ToTLV()).get()['Any'], {0: 'payload', 1: hint})
                self.CheckData(request)

    def test_content_app_message_missing_data(self) -> None:
        request = Clusters.ContentAppObserver.Commands.ContentAppMessage(data=None, encodingHint='text/plain')
        # None would omit Data, which is mandatory in Matter 1.6.1 section 6.12.5.1.
        with self.assertRaises(ValueError):
            request.ToTLV()

    def test_double_nested_struct_list(self):
        simpleStruct = Clusters.UnitTesting.Structs.SimpleStruct()
        simpleStruct.a = 23
        simpleStruct.b = True
        simpleStruct.c = Clusters.UnitTesting.Enums.SimpleEnum.kValueA
        simpleStruct.d = b'1234'
        simpleStruct.e = 'hello'
        simpleStruct.f = 1
        simpleStruct.g = 0
        simpleStruct.h = 0

        data = Clusters.UnitTesting.Structs.NestedStructList()
        data.a = 23
        data.b = True
        data.c = simpleStruct
        data.d = []
        data.d.append(simpleStruct)
        data.d.append(simpleStruct)

        data.e = [1, 2, 3, 4]
        data.f = [b'1', b'2', b'3']
        data.g = [2, 3, 4, 5]

        self.CheckData(data)

    def test_nullable_optional_struct(self):
        data = Clusters.UnitTesting.Structs.NullablesAndOptionalsStruct()

        data.nullableInt = 2
        data.optionalInt = 3
        data.nullableOptionalInt = 4
        data.nullableString = 'hello1'
        data.optionalString = 'hello2'
        data.nullableOptionalString = 'hello3'
        data.nullableStruct = Clusters.UnitTesting.Structs.SimpleStruct(
            23, True, Clusters.UnitTesting.Enums.SimpleEnum.kValueA, b'1234', 'hello', 1, 0, 0)
        data.optionalStruct = Clusters.UnitTesting.Structs.SimpleStruct(
            24, True, Clusters.UnitTesting.Enums.SimpleEnum.kValueA, b'1234', 'hello', 1, 0, 0)
        data.nullableOptionalStruct = Clusters.UnitTesting.Structs.SimpleStruct(
            25, True, Clusters.UnitTesting.Enums.SimpleEnum.kValueA, b'1234', 'hello', 1, 0, 0)

        data.nullableList = [Clusters.UnitTesting.Enums.SimpleEnum.kValueA]
        data.optionalList = [Clusters.UnitTesting.Enums.SimpleEnum.kValueA]
        data.nullableOptionalList = [
            Clusters.UnitTesting.Enums.SimpleEnum.kValueA]

        self.CheckData(data)

        data.nullableInt = NullValue
        data.nullableOptionalInt = NullValue
        data.nullableOptionalString = NullValue
        data.nullableStruct = NullValue
        data.nullableOptionalStruct = NullValue
        data.nullableList = NullValue
        data.nullableOptionalList = NullValue

        self.CheckData(data)

        data.nullableOptionalInt = None
        data.nullableOptionalString = None
        data.nullableOptionalStruct = None
        data.nullableOptionalList = None

        self.CheckData(data)


if __name__ == '__main__':
    unittest.main()
