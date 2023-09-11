#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Contains type definition to help with thread credentials.
# Generally thread credentials are assumed to be binary objects, however for
# testing purposes, we expose the internal structure here.

from construct import Byte, Bytes, Int16ul, Int64ul, PaddedString, Struct

ThreadNetworkInfo = Struct(
    "ActiveTimestamp" / Int64ul,
    "MasterKey" / Bytes(16),
    "PSKc" / Bytes(16),
    "ExtendedPanId" / Bytes(8),
    "MeshPrefix" / Bytes(8),
    "PanId" / Int16ul,
    "NetworkName" / PaddedString(17, 'utf8'),
    "Channel" / Byte,
)
