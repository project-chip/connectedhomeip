#
#    Copyright (c) 2021 Project CHIP Authors
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


from chip.tlv import TLVWriter, TLVReader
from chip.tlv import uint as tlvUint

import unittest


class TestTLVWriter(unittest.TestCase):
    def _getEncoded(self, val, tag=None):
        writer = TLVWriter()
        writer.put(tag, val)
        return writer.encoding

    def test_int(self):
        encodedVal = self._getEncoded(0x00deadbeefca00fe)
        self.assertEqual(encodedVal,
                         bytearray([0b00000011,
                                    0xfe, 0x00, 0xca, 0xef, 0xbe, 0xad, 0xde, 0x00]))
        encodedVal = self._getEncoded(0x7cadbeef)
        self.assertEqual(encodedVal,
                         bytearray([0b00000010, 0xef, 0xbe, 0xad, 0x7c]))
        encodedVal = self._getEncoded(0x7cad)
        self.assertEqual(encodedVal,
                         bytearray([0b00000001, 0xad, 0x7c]))
        encodedVal = self._getEncoded(0x7c)
        self.assertEqual(encodedVal,
                         bytearray([0b00000000, 0x7c]))
        # Negative numbers
        encodedVal = self._getEncoded(-(0x5555555555555555))
        self.assertEqual(encodedVal,
                         bytearray([0b00000011,
                                    0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa]))
        encodedVal = self._getEncoded(-(0x55555555))
        self.assertEqual(encodedVal,
                         bytearray([0b00000010, 0xab, 0xaa, 0xaa, 0xaa]))
        encodedVal = self._getEncoded(-(0x5555))
        self.assertEqual(encodedVal,
                         bytearray([0b00000001, 0xab, 0xaa]))
        encodedVal = self._getEncoded(-(0x55))
        self.assertEqual(encodedVal,
                         bytearray([0b00000000, 0xab]))
        # The following numbers are positive values but exceeds the upper bounds of the type they seems to be.
        encodedVal = self._getEncoded(0xdeadbeef)
        self.assertEqual(encodedVal,
                         bytearray([0b00000011, 0xef, 0xbe, 0xad, 0xde, 0x00, 0x00, 0x00, 0x00]))
        encodedVal = self._getEncoded(0xdead)
        self.assertEqual(encodedVal,
                         bytearray([0b00000010, 0xad, 0xde, 0x00, 0x00]))
        encodedVal = self._getEncoded(0xad)
        self.assertEqual(encodedVal,
                         bytearray([0b00000001, 0xad, 0x00]))
        # Similar, these negative numbers also exceedes the width of the type they seems to be.
        encodedVal = self._getEncoded(-(0xaaaaaaaa))
        self.assertEqual(encodedVal,
                         bytearray([0b00000011, 0x56, 0x55, 0x55, 0x55, 0xff, 0xff, 0xff, 0xff]))
        encodedVal = self._getEncoded(-(0xaaaa))
        self.assertEqual(encodedVal,
                         bytearray([0b00000010, 0x56, 0x55, 0xff, 0xff]))
        encodedVal = self._getEncoded(-(0xaa))
        self.assertEqual(encodedVal,
                         bytearray([0b00000001, 0x56, 0xff]))
        try:
            encodedVal = self._getEncoded(0xf000f000f000f000)
            self.fail("Signed number exceeds INT64_MAX but no exception received")
        except Exception:
            pass

        try:
            encodedVal = self._getEncoded(-(0xf000f000f000f000))
            self.fail("Signed number exceeds INT64_MIN but no exception received")
        except Exception:
            pass

    def test_uint(self):
        encodedVal = self._getEncoded(tlvUint(0xdeadbeefca0000fe))
        self.assertEqual(encodedVal,
                         bytearray([0b00000111,
                                    0xfe, 0x00, 0x00, 0xca, 0xef, 0xbe, 0xad, 0xde]))
        encodedVal = self._getEncoded(tlvUint(0xdeadbeef))
        self.assertEqual(encodedVal,
                         bytearray([0b00000110, 0xef, 0xbe, 0xad, 0xde]))
        encodedVal = self._getEncoded(tlvUint(0xdead))
        self.assertEqual(encodedVal,
                         bytearray([0b00000101, 0xad, 0xde]))
        encodedVal = self._getEncoded(tlvUint(0xde))
        self.assertEqual(encodedVal,
                         bytearray([0b00000100, 0xde]))
        try:
            encodedVal = self._getEncoded(tlvUint(-1))
            self.fail("Negative unsigned int but no exception raised.")
        except Exception:
            pass
        try:
            encodedVal = self._getEncoded(tlvUint(0x10000000000000000))
            self.fail("Overflowed uint but no exception raised.")
        except Exception:
            pass


class TestTLVReader(unittest.TestCase):
    def _read_case(self, input, answer):
        decoded = TLVReader(bytearray(input)).get()["Any"]
        self.assertEqual(type(decoded), type(answer))
        self.assertEqual(decoded, answer)

    def test_int(self):
        self._read_case([0b00000011,
                         0xfe, 0x00, 0xca, 0xef, 0xbe, 0xad, 0xde, 0x00], 0x00deadbeefca00fe)
        self._read_case([0b00000010, 0xef, 0xbe, 0xad, 0x7c], 0x7cadbeef)
        self._read_case([0b00000001, 0xad, 0x7c], 0x7cad)
        self._read_case([0b00000000, 0x7c], 0x7c)

        self._read_case([0b00000011,
                         0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa], -(0x5555555555555555))
        self._read_case([0b00000010, 0xab, 0xaa, 0xaa, 0xaa], -(0x55555555))
        self._read_case([0b00000001, 0xab, 0xaa], -(0x5555))
        self._read_case([0b00000000, 0xab], -(0x55))

    def test_uint(self):
        self._read_case([0b00000111,
                         0xfe, 0x00, 0xca, 0xef, 0xbe, 0xad, 0xde, 0x00], tlvUint(0x00deadbeefca00fe))
        self._read_case([0b00000110, 0xef, 0xbe, 0xad, 0x7c],
                        tlvUint(0x7cadbeef))
        self._read_case([0b00000101, 0xad, 0x7c], tlvUint(0x7cad))
        self._read_case([0b00000100, 0x7c], tlvUint(0x7c))

        self._read_case([0b00000111,
                         0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa], tlvUint(0xaaaaaaaaaaaaaaab))
        self._read_case([0b00000110, 0xab, 0xaa, 0xaa, 0xaa],
                        tlvUint(0xaaaaaaab))
        self._read_case([0b00000101, 0xab, 0xaa], tlvUint(0xaaab))
        self._read_case([0b00000100, 0xab], tlvUint(0xab))


if __name__ == '__main__':
    unittest.main()
