#!/usr/bin/env python3
#
#    Copyright (c) 2022-2024 Project CHIP Authors
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


class Base38:
    """
    A class for encoding and decoding data using a custom Base38 encoding scheme.

    This class provides methods to encode bytes into a Base38 string representation
    and decode Base38 strings back into bytes. The encoding uses 38 characters:
    digits 0-9, uppercase letters A-Z, and the symbols '-' and '.'.

    The encoding process splits the input bytes into chunks of up to 3 bytes,
    which are then encoded into 2, 4, or 5 Base38 characters depending on the
    chunk size.

    Attributes:
        CODES (list): The 38 characters used in the encoding.
        RADIX (int): The base of the encoding (38).
        BASE38_CHARS_NEEDED_IN_CHUNK (list): Number of Base38 chars needed for each chunk size.
        MAX_BYTES_IN_CHUNK (int): Maximum number of bytes processed in each chunk (3).
        MAX_ENCODED_BYTES_IN_CHUNK (int): Maximum number of Base38 chars in an encoded chunk (5).
    """

    CODES = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
             'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
             'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
             'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.']
    RADIX = len(CODES)
    BASE38_CHARS_NEEDED_IN_CHUNK = [2, 4, 5]
    MAX_BYTES_IN_CHUNK = 3
    MAX_ENCODED_BYTES_IN_CHUNK = 5

    @staticmethod
    def encode(bytes_to_encode):
        """
        Encode a byte sequence into a Base38 string.

        Args:
            bytes_to_encode (bytes): The byte sequence to encode.

        Returns:
            str: The Base38 encoded string.
        """
        total_bytes = len(bytes_to_encode)
        qrcode = ''

        for i in range(0, total_bytes, Base38.MAX_BYTES_IN_CHUNK):
            if (i + Base38.MAX_BYTES_IN_CHUNK) > total_bytes:
                bytes_in_chunk = total_bytes - i
            else:
                bytes_in_chunk = Base38.MAX_BYTES_IN_CHUNK

            value = 0
            for j in range(i, i + bytes_in_chunk):
                value = value + (bytes_to_encode[j] << (8 * (j - i)))

            base38_chars_needed = Base38.BASE38_CHARS_NEEDED_IN_CHUNK[bytes_in_chunk - 1]
            while base38_chars_needed > 0:
                qrcode += Base38.CODES[int(value % Base38.RADIX)]
                value = int(value / Base38.RADIX)
                base38_chars_needed -= 1

        return qrcode

    @staticmethod
    def decode(qrcode):
        """
        Decode a Base38 string back into bytes.

        Args:
            qrcode (str): The Base38 encoded string to decode.

        Returns:
            bytearray: The decoded byte sequence.
        """
        total_chars = len(qrcode)
        decoded_bytes = bytearray()

        for i in range(0, total_chars, Base38.MAX_ENCODED_BYTES_IN_CHUNK):
            if (i + Base38.MAX_ENCODED_BYTES_IN_CHUNK) > total_chars:
                chars_in_chunk = total_chars - i
            else:
                chars_in_chunk = Base38.MAX_ENCODED_BYTES_IN_CHUNK

            value = 0
            for j in range(i + chars_in_chunk - 1, i - 1, -1):
                value = value * Base38.RADIX + Base38.CODES.index(qrcode[j])

            bytes_in_chunk = Base38.BASE38_CHARS_NEEDED_IN_CHUNK.index(chars_in_chunk) + 1
            for k in range(0, bytes_in_chunk):
                decoded_bytes.append(value & 0xFF)
                value = value >> 8

        return decoded_bytes
