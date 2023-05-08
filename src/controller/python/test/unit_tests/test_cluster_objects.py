import typing
import unittest
from dataclasses import dataclass

import chip.ChipUtility
from chip.clusters import ClusterObjects
from chip.tlv import TLVReader, TLVWriter, uint

'''
This file contains tests for checking if the cluster object can generate correct TLV data.

For readability, the TLV related tests are placed in test_tlv.py, and we will test the following case:
1. Generate a struct, encode it, and decode it using TLVReader
2. Generate a dict, encode it using TLVWriter, and decode it using the cluster objects

If TLVReader / TLVWriter emitted wrong TLV data, related test cases should be added to test_tlv.py.
To avoid failures caused by schema change, this file should only contain manually crafted structs.
'''


def _encode_and_then_decode_to_native(data: ClusterObjects.ClusterObject):
    tlv = data.ToTLV()
    return TLVReader(tlv).get()['Any']


def _encode_attribute_and_then_decode_to_native(data, type: ClusterObjects.ClusterAttributeDescriptor):
    return TLVReader(type.ToTLV(None, data)).get()['Any']


def _encode_from_native_and_then_decode(data,
                                        cls: typing.Union[ClusterObjects.ClusterObject,
                                                          ClusterObjects.ClusterAttributeDescriptor]):
    tlv = TLVWriter()
    tlv.put(None, data)
    return cls.FromTLV(bytes(tlv.encoding))


class TestClusterObjects(unittest.TestCase):
    @dataclass
    class C(ClusterObjects.ClusterObject):
        @chip.ChipUtility.classproperty
        def descriptor(cls) -> ClusterObjects.ClusterObjectDescriptor:
            return ClusterObjects.ClusterObjectDescriptor(
                Fields=[
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="X", Tag=0, Type=uint),
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="Y", Tag=1, Type=int),
                ])
        X: 'uint' = None
        Y: 'int' = None

    def test_basic_encode(self):
        data = TestClusterObjects.C(X=5, Y=23)
        res = _encode_and_then_decode_to_native(data)
        self.assertEqual(res, {0: 5, 1: 23})

    def test_basic_decode(self):
        res = _encode_from_native_and_then_decode(
            {0: uint(5), 1: 23}, TestClusterObjects.C)
        self.assertEqual(res, TestClusterObjects.C(X=5, Y=23))

    @dataclass
    class StructWithArray(ClusterObjects.ClusterObject):
        @chip.ChipUtility.classproperty
        def descriptor(cls) -> ClusterObjects.ClusterObjectDescriptor:
            return ClusterObjects.ClusterObjectDescriptor(
                Fields=[
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="X", Tag=0, Type=typing.List[uint]),
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="Y", Tag=1, Type=int),
                ])

        X: typing.List['uint'] = None
        Y: int = None

    def test_struct_w_array_encode(self):
        data = TestClusterObjects.StructWithArray(X=[5, 6], Y=23)
        res = _encode_and_then_decode_to_native(data)
        self.assertEqual(res, {0: [5, 6], 1: 23})

    def test_struct_w_array_decode(self):
        res = _encode_from_native_and_then_decode(
            {0: [uint(5), uint(6)], 1: 23}, TestClusterObjects.StructWithArray)
        self.assertEqual(
            res, TestClusterObjects.StructWithArray(X=[5, 6], Y=23))

    @dataclass
    class StructWithEmbeddedStructAndString(ClusterObjects.ClusterObject):
        @chip.ChipUtility.classproperty
        def descriptor(cls) -> ClusterObjects.ClusterObjectDescriptor:
            return ClusterObjects.ClusterObjectDescriptor(
                Fields=[
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="X", Tag=0, Type=str),
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="Y", Tag=1, Type=TestClusterObjects.C),
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="Z", Tag=2, Type=bytes),
                ])

        X: 'str' = None
        Y: 'TestClusterObjects.C' = None
        Z: 'bytes' = None

    def test_struct_w_string_and_string_encode(self):
        data = TestClusterObjects.StructWithEmbeddedStructAndString(
            X='test-str', Y=TestClusterObjects.C(X=12, Y=34), Z=b'byte-string')
        res = _encode_and_then_decode_to_native(data)
        self.assertEqual(
            res, {0: 'test-str', 1: {0: 12, 1: 34}, 2: b'byte-string'})

    def test_struct_w_string_and_string_decode(self):
        res = _encode_from_native_and_then_decode(
            {0: 'test-str', 1: {0: uint(12), 1: 34}, 2: b'byte-string'},
            TestClusterObjects.StructWithEmbeddedStructAndString)
        self.assertEqual(
            res, TestClusterObjects.StructWithEmbeddedStructAndString(
                X='test-str', Y=TestClusterObjects.C(X=12, Y=34), Z=b'byte-string'))

    @dataclass
    class StructWithArrayOfStructWithArray(ClusterObjects.ClusterObject):
        @chip.ChipUtility.classproperty
        def descriptor(cls) -> ClusterObjects.ClusterObjectDescriptor:
            return ClusterObjects.ClusterObjectDescriptor(
                Fields=[
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="X", Tag=0, Type=typing.List[str]),
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="Y", Tag=1, Type=typing.List[TestClusterObjects.C]),
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="Z", Tag=2, Type=typing.List[TestClusterObjects.StructWithArray]),
                    # Recursive!
                    ClusterObjects.ClusterObjectFieldDescriptor(
                        Label="W", Tag=3, Type=typing.List[TestClusterObjects.StructWithArrayOfStructWithArray]),
                ])

        X: typing.List['str'] = None
        Y: typing.List['TestClusterObjects.C'] = None
        Z: typing.List['TestClusterObjects.StructWithArray'] = None
        W: typing.List['TestClusterObjects.StructWithArrayOfStructWithArray'] = None

    def test_struct_with_array_of_struct_with_array_encode(self):
        C = TestClusterObjects.C
        SWA = TestClusterObjects.StructWithArray
        SWAOSWA = TestClusterObjects.StructWithArrayOfStructWithArray
        data = TestClusterObjects.StructWithArrayOfStructWithArray(
            X=['test-str1', 'test-str2', 'test-str3'],
            Y=[C(X=12, Y=34), C(X=56, Y=78), C(X=23, Y=33)],
            Z=[SWA(X=[12, 34], Y=5678), SWA(X=[1, 3], Y=57),
               SWA(X=[2, 4], Y=68), SWA(X=[233, 333], Y=333)],
            W=[SWAOSWA(X=['test-str4'], Y=[C(X=55, Y=66)], Z=[SWA(X=[123], Y=456)], W=[])])
        res = _encode_and_then_decode_to_native(data)
        self.assertEqual(
            res,
            {0: ['test-str1', 'test-str2', 'test-str3'],
             1: [{0: 12, 1: 34}, {0: 56, 1: 78}, {0: 23, 1: 33}],
             2: [{0: [12, 34], 1: 5678},
                 {0: [1, 3], 1: 57},
                 {0: [2, 4], 1: 68},
                 {0: [233, 333], 1: 333}],
             3: [{0: ['test-str4'], 1: [{0: 55, 1: 66}], 2: [{0: [123], 1: 456}], 3: []}]})

    def test_struct_with_array_of_struct_with_array_decode(self):
        res = _encode_from_native_and_then_decode(
            {0: ['test-str1', 'test-str2', 'test-str3'],
             1: [{0: uint(12), 1: 34}, {0: uint(56), 1: 78}, {0: uint(23), 1: 33}],
             2: [{0: [uint(12), uint(34)], 1: 5678},
                 {0: [uint(1), uint(3)], 1: 57},
                 {0: [uint(2), uint(4)], 1: 68},
                 {0: [uint(233), uint(333)], 1: 333}],
             3: [{0: ['test-str4'], 1: [{0: uint(55), 1: 66}], 2: [{0: [uint(123)], 1: 456}], 3: []}]},
            TestClusterObjects.StructWithArrayOfStructWithArray)

        C = TestClusterObjects.C
        SWA = TestClusterObjects.StructWithArray
        SWAOSWA = TestClusterObjects.StructWithArrayOfStructWithArray

        data = TestClusterObjects.StructWithArrayOfStructWithArray(
            X=['test-str1', 'test-str2', 'test-str3'],
            Y=[C(X=12, Y=34), C(X=56, Y=78), C(X=23, Y=33)],
            Z=[SWA(X=[12, 34], Y=5678), SWA(X=[1, 3], Y=57),
               SWA(X=[2, 4], Y=68), SWA(X=[233, 333], Y=333)],
            W=[SWAOSWA(X=['test-str4'], Y=[C(X=55, Y=66)], Z=[SWA(X=[123], Y=456)], W=[])])

        self.assertEqual(res, data)


class TestAttributeDescriptor(unittest.TestCase):
    class IntAttribute(ClusterObjects.ClusterAttributeDescriptor):
        @chip.ChipUtility.classproperty
        def attribute_type(cls) -> ClusterObjects.ClusterObjectFieldDescriptor:
            return ClusterObjects.ClusterObjectFieldDescriptor(Type=int)

    def test_basic_encode(self):
        res = _encode_attribute_and_then_decode_to_native(
            42, TestAttributeDescriptor.IntAttribute)
        self.assertEqual(res, 42)

    def test_basic_decode(self):
        res = _encode_from_native_and_then_decode(
            42, TestAttributeDescriptor.IntAttribute)
        self.assertEqual(res, 42)

    class StructAttribute(ClusterObjects.ClusterAttributeDescriptor):
        @chip.ChipUtility.classproperty
        def attribute_type(cls) -> ClusterObjects.ClusterObjectFieldDescriptor:
            return ClusterObjects.ClusterObjectFieldDescriptor(Type=TestClusterObjects.C)

    def test_struct_encode(self):
        res = _encode_attribute_and_then_decode_to_native(
            TestClusterObjects.C(X=42, Y=24), TestAttributeDescriptor.StructAttribute)
        self.assertEqual(res, {0: 42, 1: 24})

    def test_struct_decode(self):
        res = _encode_from_native_and_then_decode(
            {0: uint(42), 1: 24}, TestClusterObjects.C)
        self.assertEqual(res, TestClusterObjects.C(X=42, Y=24))

    class ArrayAttribute(ClusterObjects.ClusterAttributeDescriptor):
        @chip.ChipUtility.classproperty
        def attribute_type(cls) -> ClusterObjects.ClusterObjectFieldDescriptor:
            return ClusterObjects.ClusterObjectFieldDescriptor(Type=typing.List[int])

    def test_array_encode(self):
        res = _encode_attribute_and_then_decode_to_native(
            [1, 2, 3, 4, 5], TestAttributeDescriptor.ArrayAttribute)
        self.assertEqual(res, [1, 2, 3, 4, 5])

    def test_array_decode(self):
        res = _encode_from_native_and_then_decode(
            [1, 2, 3, 4, 5], TestAttributeDescriptor.ArrayAttribute)
        self.assertEqual(res, [1, 2, 3, 4, 5])


if __name__ == '__main__':
    unittest.main()
