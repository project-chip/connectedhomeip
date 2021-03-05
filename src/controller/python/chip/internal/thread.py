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

from construct import BitStruct, Byte, Bytes, Enum, Flag, Int16ul, Int32ul, Int64ul, PaddedString, Padding, Struct

ThreadDeviceNetworkInfo = Struct(
    "NetworkId" / Int32ul,
    "FieldPresent" / BitStruct (
        "NetworkId" / Flag,
        "ThreadExtendedPANId" / Flag,
        "ThreadMeshPrefix" / Flag,
        "ThreadPSKc" / Flag,
        "Padding" / Padding(4),
    ),
    "WiFiSSID" / PaddedString(33, 'utf8'),
    "WiFikey" / Bytes(64),
    "WiFiKeyLen" / Byte,
    "WiFiAuthSecurityType" / Enum(Byte, 
       # NotAvailable is the same as None/NotSpecified in DeviceNetworkInfo.h
       # Used this name because 'None' is reserved in python
       NotAvailable        = 1, 
       WEP                 = 2,
       WPAPersonal         = 3,
       WPA2Personal        = 4,
       WPA2MixedPersonal   = 5,
       WPAEnterprise       = 6,
       WPA2Enterprise      = 7,
       WPA2MixedEnterprise = 8,
       WPA3Personal        = 9,
       WPA3MixedPersonal   = 10,
       WPA3Enterprise      = 11,
       WPA3MixedEnterprise = 12,
    ),
    "ThreadNetworkName" / PaddedString(17, 'utf8'),
    "ThreadExtendedPANId" / Bytes(8),
    "ThreadMeshPrefix" / Bytes(8),
    "ThreadMasterKey" / Bytes(16),
    "ThreadPSKc" / Bytes(16),
    "_ThreadPANIdPadding" / Padding(1), # Aligns ThreadPANId
    "ThreadPANId" / Int16ul,
    "ThreadChannel" / Byte,
    "_ThreadDatasetTimestampPadding" / Padding(3), # Aligns ThreadDAtasetTimestamp
    "ThreadDatasetTimestamp" / Int64ul,
)