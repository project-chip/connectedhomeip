#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

# TODO: Implement the decode method

CODES = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
         'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
         'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
         'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.']
RADIX = len(CODES)
BASE38_CHARS_NEEDED_IN_CHUNK = [2, 4, 5]
MAX_BYTES_IN_CHUNK = 3


def encode(bytes):
    total_bytes = len(bytes)
    qrcode = ''

    for i in range(0, total_bytes, MAX_BYTES_IN_CHUNK):
        if (i + MAX_BYTES_IN_CHUNK) > total_bytes:
            bytes_in_chunk = total_bytes - i
        else:
            bytes_in_chunk = MAX_BYTES_IN_CHUNK

        value = 0
        for j in range(i, i + bytes_in_chunk):
            value = value + (bytes[j] << (8 * (j - i)))

        base38_chars_needed = BASE38_CHARS_NEEDED_IN_CHUNK[bytes_in_chunk - 1]
        while base38_chars_needed > 0:
            qrcode += CODES[int(value % RADIX)]
            value = int(value / RADIX)
            base38_chars_needed -= 1

    return qrcode
