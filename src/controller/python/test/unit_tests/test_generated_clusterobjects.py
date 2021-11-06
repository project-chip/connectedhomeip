import unittest
import chip.clusters as Clusters
from rich.pretty import pprint
from chip.clusters.Types import Nullable, NullValue

'''
This file contains tests for validating the generated cluster objects by running encoding and decoding
through them symmetrically to exercise both pathways.
'''


class TestGeneratedClusterObjects(unittest.TestCase):
    def CheckData(self, expected):
        print("\nExpected Data:")
        pprint(expected, expand_all=True)

        tlv = expected.ToTLV()
        actual = expected.FromTLV(tlv)

        print("Actual Data:")
        pprint(actual, expand_all=True)
        self.assertEqual(actual, expected)

    def test_simple_struct(self):
        data = Clusters.TestCluster.Structs.SimpleStruct()
        data.a = 23
        data.b = True
        data.c = Clusters.TestCluster.Enums.SimpleEnum.kValueA
        data.d = b'1234'
        data.e = 'hello'
        data.f = 1

        self.CheckData(data)

    def test_double_nested_struct_list(self):
        simpleStruct = Clusters.TestCluster.Structs.SimpleStruct()
        simpleStruct.a = 23
        simpleStruct.b = True
        simpleStruct.c = Clusters.TestCluster.Enums.SimpleEnum.kValueA
        simpleStruct.d = b'1234'
        simpleStruct.e = 'hello'
        simpleStruct.f = 1

        data = Clusters.TestCluster.Structs.NestedStructList()
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
        data = Clusters.TestCluster.Structs.NullablesAndOptionalsStruct()

        data.nullableInt = 2
        data.optionalInt = 3
        data.nullableOptionalInt = 4
        data.nullableString = 'hello1'
        data.optionalString = 'hello2'
        data.nullableOptionalString = 'hello3'
        data.nullableStruct = Clusters.TestCluster.Structs.SimpleStruct(
            23, True, Clusters.TestCluster.Enums.SimpleEnum.kValueA, b'1234', 'hello', 1)
        data.optionalStruct = Clusters.TestCluster.Structs.SimpleStruct(
            24, True, Clusters.TestCluster.Enums.SimpleEnum.kValueA, b'1234', 'hello', 1)
        data.nullableOptionalStruct = Clusters.TestCluster.Structs.SimpleStruct(
            25, True, Clusters.TestCluster.Enums.SimpleEnum.kValueA, b'1234', 'hello', 1)

        data.nullableList = [Clusters.TestCluster.Enums.SimpleEnum.kValueA]
        data.optionalList = [Clusters.TestCluster.Enums.SimpleEnum.kValueA]
        data.nullableOptionalList = [
            Clusters.TestCluster.Enums.SimpleEnum.kValueA]

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
