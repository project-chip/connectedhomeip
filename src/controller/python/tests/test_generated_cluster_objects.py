import itertools
import unittest

from rich.pretty import pprint

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.tlv import TLVReader, TLVWriter

'''
This file contains tests for validating the generated cluster objects by running encoding and decoding
through them symmetrically to exercise both pathways.
'''

enable_debug = False


def _channel_recording_cases():
    external_ids = [Clusters.Channel.Structs.AdditionalInfoStruct(name="provider", value="program")]
    for command_type in (Clusters.Channel.Commands.RecordProgram, Clusters.Channel.Commands.CancelRecordProgram):
        for ids, data in itertools.product((None, [], external_ids), (None, b"", b"app-data")):
            # Matter 1.6.1 sections 6.6.7.7 and 6.6.7.8 make tags 2 and 3 independently optional.
            fields = {0: "program", 1: False}
            if ids is not None:
                fields[2] = [{0: entry.name, 1: entry.value} for entry in ids]
            if data is not None:
                fields[3] = data
            yield command_type, ids, data, fields


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

    def test_channel_recording_encode_optional_fields(self) -> None:
        for command_type, ids, data, fields in _channel_recording_cases():
            with self.subTest(command=command_type.__name__, externalIDList=ids, data=data):
                command = command_type(programIdentifier="program", shouldRecordSeries=False, externalIDList=ids, data=data)
                self.assertEqual(TLVReader(command.ToTLV()).get()["Any"], fields)

    def test_channel_recording_decode_optional_fields(self) -> None:
        for command_type, ids, data, fields in _channel_recording_cases():
            with self.subTest(command=command_type.__name__, externalIDList=ids, data=data):
                writer = TLVWriter()
                writer.put(None, fields)
                command = command_type.FromTLV(bytes(writer.encoding))
                self.assertEqual(command.programIdentifier, "program")
                self.assertIs(command.shouldRecordSeries, False)
                self.assertEqual(command.externalIDList, ids)
                self.assertEqual(command.data, data)

    def test_channel_recording_optional_fields_default_to_absent(self) -> None:
        for command_type in (Clusters.Channel.Commands.RecordProgram, Clusters.Channel.Commands.CancelRecordProgram):
            with self.subTest(command=command_type.__name__):
                command = command_type(programIdentifier="program", shouldRecordSeries=False)
                self.assertIsNone(command.externalIDList)
                self.assertIsNone(command.data)
                self.assertEqual(TLVReader(command.ToTLV()).get()["Any"], {0: "program", 1: False})

    def test_channel_recording_optional_fields_are_not_nullable(self) -> None:
        for command_type in (Clusters.Channel.Commands.RecordProgram, Clusters.Channel.Commands.CancelRecordProgram):
            for field in ("externalIDList", "data"):
                with self.subTest(command=command_type.__name__, field=field):
                    command = command_type(programIdentifier="program", shouldRecordSeries=False, externalIDList=[], data=b"")
                    setattr(command, field, NullValue)
                    # Optional fields may be absent, but neither field permits a TLV Null value.
                    with self.assertRaisesRegex(ValueError, rf"{field} was not nullable, but got a null"):
                        command.ToTLV()


if __name__ == '__main__':
    unittest.main()
