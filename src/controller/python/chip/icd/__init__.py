#
#    Copyright (c) 2024 Project CHIP Authors
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

from ctypes import CFUNCTYPE, c_uint8, c_uint64
from dataclasses import dataclass
from threading import Condition, Event, Lock
from typing import Callable

from chip import native

_OnCheckInCompleteFunct = CFUNCTYPE(None, c_uint8, c_uint64)

_OnCheckInCompleteWaitListLock = Lock()
_OnCheckInCompleteWaitList = dict()


@dataclass
class OnCheckInCompleteParams:
    fabricIndex: int
    nodeId: int


@_OnCheckInCompleteFunct
def _OnCheckInComplete(fabricIndex: int, nodeId: int):
    callbacks = list()
    with _OnCheckInCompleteWaitListLock:
        callbacks = list(_OnCheckInCompleteWaitList.get((fabricIndex, nodeId), set()))

    for callback in callbacks:
        callback(OnCheckInCompleteParams(fabricIndex, nodeId))


_initialized = False


def _ensureInit():
    global _initialized

    if _initialized:
        return

    libraryHandle = native.GetLibraryHandle()
    libraryHandle.pychip_CheckInDelegate_SetOnCheckInCompleteCallback.restype = None
    libraryHandle.pychip_CheckInDelegate_SetOnCheckInCompleteCallback.argtypes = [_OnCheckInCompleteFunct]

    libraryHandle.pychip_CheckInDelegate_SetOnCheckInCompleteCallback(_OnCheckInComplete)

    _initialized = True


def AddOnActiveCallback(fabricIndex: int, nodeId: int, callback: Callable[None, [OnCheckInCompleteParams]]):
    with _OnCheckInCompleteWaitListLock:
        waitList = _OnCheckInCompleteWaitList.get((fabricIndex, nodeId), set())
        waitList.add(callback)
        _OnCheckInCompleteWaitList[(fabricIndex, nodeId)] = waitList


def RemoveOnActiveCallback(fabricIndex: int, nodeId: int, callback: Callable[None, [OnCheckInCompleteParams]]):
    with _OnCheckInCompleteWaitListLock:
        _OnCheckInCompleteWaitList.get((fabricIndex, nodeId), set()).remove(callback)
