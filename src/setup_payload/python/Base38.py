#!/usr/bin/env python3
#
#    Copyright (c) 2022 Project CHIP Authors
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

# TODO: Implement the Decode method

kCodes = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
          'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
          'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
          'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.']
kRadix = len(kCodes)
kBase38CharactersNeededInNBytesChunk = [2, 4, 5]
kMaxBytesInChunk = 3


def Encode(bytes):
    totalBytes = len(bytes)
    qrcode = ''

    for i in range(0, totalBytes, kMaxBytesInChunk):
        if (i + kMaxBytesInChunk) > totalBytes:
            bytesInChunk = totalBytes - i
        else:
            bytesInChunk = kMaxBytesInChunk

        value = 0
        for j in range(i, i + bytesInChunk):
            value = value + (bytes[j] << (8 * (j - i)))

        base38CharNeeded = kBase38CharactersNeededInNBytesChunk[bytesInChunk - 1]
        while base38CharNeeded > 0:
            qrcode += kCodes[int(value % kRadix)]
            value = int(value / kRadix)
            base38CharNeeded -= 1

    return qrcode
