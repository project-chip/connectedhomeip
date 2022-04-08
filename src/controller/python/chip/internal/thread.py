#
#    Copyright (c) 2021 Project CHIP Authors
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
