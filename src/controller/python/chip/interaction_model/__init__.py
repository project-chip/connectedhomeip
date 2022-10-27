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

#
#    @file
#      Provides Python APIs for CHIP.
#

"""Provides Python APIs for CHIP."""
import enum
from .delegate import AttributePath, AttributePathIBstruct, EventPath, EventPathIBstruct, DataVersionFilterIBstruct

from chip.exceptions import ChipStackException

__all__ = ["Status", "InteractionModelError"]


class Status(enum.IntEnum):
    Success = 0x0
    Failure = 0x01
    InvalidSubscription = 0x7d
    UnsupportedAccess = 0x7e
    UnsupportedEndpoint = 0x7f
    InvalidAction = 0x80
    UnsupportedCommand = 0x81
    Deprecated82 = 0x82
    Deprecated83 = 0x83
    Deprecated84 = 0x84
    InvalidCommand = 0x85
    UnsupportedAttribute = 0x86
    ConstraintError = 0x87
    UnsupportedWrite = 0x88
    ResourceExhausted = 0x89
    Deprecated8a = 0x8a
    NotFound = 0x8b
    UnreportableAttribute = 0x8c
    InvalidDataType = 0x8d
    Deprecated8e = 0x8e
    UnsupportedRead = 0x8f
    Deprecated90 = 0x90
    Deprecated91 = 0x91
    DataVersionMismatch = 0x92
    Deprecated93 = 0x93
    Timeout = 0x94
    Reserved95 = 0x95
    Reserved96 = 0x96
    Reserved97 = 0x97
    Reserved98 = 0x98
    Reserved99 = 0x99
    Reserved9a = 0x9a
    Busy = 0x9c
    Deprecatedc0 = 0xc0
    Deprecatedc1 = 0xc1
    Deprecatedc2 = 0xc2
    UnsupportedCluster = 0xc3
    Deprecatedc4 = 0xc4
    NoUpstreamSubscription = 0xc5
    NeedsTimedInteraction = 0xc6
    UnsupportedEvent = 0xc7
    PathsExhausted = 0xc8
    TimedRequestMismatch = 0xc9
    FailsafeRequired = 0xca


class InteractionModelError(ChipStackException):
    def __init__(self, status: Status):
        self._status = status

    def __str__(self):
        return f"InteractionModelError: {self._status.name} (0x{self._status.value:x})"

    @property
    def status(self) -> Status:
        return self._status
