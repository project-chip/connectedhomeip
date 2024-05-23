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

import asyncio
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


def RegisterOnActiveCallback(fabricIndex: int, nodeId: int, callback: Callable[None, [OnCheckInCompleteParams]]):
    ''' Registers a callback when the device with given (fabric index, node id) becomes active.

    Does nothing if the callback is already registered.
    '''
    _ensureInit()
    with _OnCheckInCompleteWaitListLock:
        waitList = _OnCheckInCompleteWaitList.get((fabricIndex, nodeId), set())
        waitList.add(callback)
        _OnCheckInCompleteWaitList[(fabricIndex, nodeId)] = waitList


def UnregisterOnActiveCallback(fabricIndex: int, nodeId: int, callback: Callable[None, [OnCheckInCompleteParams]]):
    ''' Unregisters a callback when the device with given (fabric index, node id) becomes active.

    Does nothing if the callback has not been registered.
    '''
    with _OnCheckInCompleteWaitListLock:
        _OnCheckInCompleteWaitList.get((fabricIndex, nodeId), set()).remove(callback)


def WaitForCheckIn(fabricIndex: int, nodeId: int):
    ''' Waits for a device becomes active.

    Returns:
        - A future, completes when the device becomes active.
    '''
    eventLoop = asyncio.get_running_loop()
    future = eventLoop.create_future()

    def OnCheckInCallback(nodeid):
        eventLoop.call_soon_threadsafe(lambda: future.done() or future.set_result(None))
        RemoveOnActiveCallback(fabricIndex, nodeId, OnCheckInCallback)
    AddOnActiveCallback(fabricIndex, nodeId, OnCheckInCallback)
    return future
