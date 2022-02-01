'''
   Copyright (c) 2021 Project CHIP Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
'''

from abc import abstractmethod
from construct import Struct, Int64ul, Int32ul, Int16ul, Int8ul
from ctypes import CFUNCTYPE, c_void_p, c_uint32, c_uint64, c_uint8, c_uint16, c_ssize_t
import ctypes
import chip.native
import threading
import chip.tlv
import chip.exceptions
import typing
from dataclasses import dataclass

# The type should match CommandStatus in interaction_model/Delegate.h
# CommandStatus should not contain padding
IMCommandStatus = Struct(
    "Status" / Int8ul,
    "ClusterStatus" / Int8ul,
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "CommandId" / Int32ul,
    "CommandIndex" / Int8ul,
)

# The type should match WriteStatus in interaction_model/Delegate.h
IMWriteStatus = Struct(
    "NodeId" / Int64ul,
    "AppIdentifier" / Int64ul,
    "Status" / Int8ul,
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "AttributeId" / Int32ul,
)

# AttributePath should not contain padding
AttributePathIBstruct = Struct(
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "AttributeId" / Int32ul,
)

# EventPath should not contain padding
EventPathIBstruct = Struct(
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "EventId" / Int32ul,
)


@dataclass
class AttributePath:
    nodeId: int
    endpointId: int
    clusterId: int
    attributeId: int


@dataclass
class EventPath:
    nodeId: int
    endpointId: int
    clusterId: int
    eventId: int

@dataclass
class AttributeReadResult:
    path: AttributePath
    status: int
    value: 'typing.Any'


@dataclass
class EventReadResult:
    path: EventPath
    value: 'typing.Any'


@dataclass
class AttributeWriteResult:
    path: AttributePath
    status: int
