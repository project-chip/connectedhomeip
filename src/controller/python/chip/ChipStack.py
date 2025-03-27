#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2020 Google LLC.
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
#      Python interface for Chip Stack
#

"""Chip Stack interface
"""

from __future__ import absolute_import, print_function

import asyncio
import builtins
from ctypes import CFUNCTYPE, Structure, c_bool, c_char_p, c_uint16, c_uint32, c_void_p, py_object, pythonapi
from threading import Condition, Lock
from typing import Any, Optional

import chip.native
from chip.native import PyChipError

from .bdx import Bdx
from .clusters import Attribute as ClusterAttribute
from .clusters import Command as ClusterCommand
from .exceptions import ChipStackError, ChipStackException, DeviceError
from .interaction_model import delegate as im
from .storage import PersistentStorage

__all__ = [
    "DeviceStatusStruct",
    "ChipStackException",
    "DeviceError",
    "ChipStackError",
    "ChipStack",
]

ChipStackDLLBaseName = "_ChipDeviceCtrl.so"


def _singleton(cls):
    instance = [None]

    def wrapper(*args, **kwargs):
        if instance[0] is None:
            instance[0] = cls(*args, **kwargs)
        return instance[0]

    return wrapper


class DeviceStatusStruct(Structure):
    _fields_ = [
        ("ProfileId", c_uint32),
        ("StatusCode", c_uint16),
        ("SysErrorCode", c_uint32),
    ]


class AsyncCallableHandle:
    def __init__(self, callback):
        self._callback = callback
        self._res = None
        self._exc = None
        self._finish = False
        self._cv_lock = Lock()
        self._cv = Condition(self._cv_lock)

    def __call__(self):
        with self._cv_lock:
            try:
                self._res = self._callback()
            except Exception as ex:
                self._exc = ex
            self._finish = True
            self._cv.notify_all()
        pythonapi.Py_DecRef(py_object(self))

    def Wait(self, timeoutMs: Optional[int] = None):
        timeout = None
        if timeoutMs is not None:
            timeout = float(timeoutMs) / 1000

        with self._cv:
            while self._finish is False:
                res = self._cv.wait(timeout)
                if res is False:
                    raise TimeoutError("Timed out waiting for task to finish executing on the Matter thread")

            if self._exc is not None:
                raise self._exc
            return self._res


class AsyncioCallableHandle:
    """Class which handles Matter SDK Calls asyncio friendly"""

    def __init__(self, callback):
        self._callback = callback
        self._loop = asyncio.get_event_loop()
        self._future = self._loop.create_future()
        self._result = None
        self._exception = None

    @property
    def future(self):
        return self._future

    def _done(self):
        if self._exception:
            self._future.set_exception(self._exception)
        else:
            self._future.set_result(self._result)

    def __call__(self):
        try:
            self._result = self._callback()
        except Exception as ex:
            self._exception = ex
        self._loop.call_soon_threadsafe(self._done)
        pythonapi.Py_DecRef(py_object(self))


_ChipThreadTaskRunnerFunct = CFUNCTYPE(None, py_object)


@_singleton
class ChipStack(object):
    def __init__(self, persistentStoragePath: str, enableServerInteractions=True):
        builtins.enableDebugMode = False

        self._ChipStackLib: Any = None
        self._chipDLLPath = None
        self.devMgr = None
        self._enableServerInteractions = enableServerInteractions
        self._subscriptions = {}

        # Locate and load the chip shared library.
        # This also implicitly does a minimal stack initialization (i.e call MemoryInit).
        self._loadLib()

        @_ChipThreadTaskRunnerFunct
        def HandleChipThreadRun(callback):
            callback()

        self.cbHandleChipThreadRun = HandleChipThreadRun

        # Storage has to be initialized BEFORE initializing the stack, since the latter
        # requires a PersistentStorageDelegate to be provided to DeviceControllerFactory.
        self._persistentStorage = PersistentStorage(persistentStoragePath)

        # Initialize the chip stack.
        res = self._ChipStackLib.pychip_DeviceController_StackInit(
            self._persistentStorage.GetSdkStorageObject(), enableServerInteractions)
        res.raise_on_error()

        im.InitIMDelegate()
        ClusterAttribute.Init()
        ClusterCommand.Init()
        Bdx.Init()

        builtins.chipStack = self

    def GetStorageManager(self):
        return self._persistentStorage

    @property
    def enableServerInteractions(self):
        return self._enableServerInteractions

    def RegisterSubscription(self, subscription: ClusterAttribute.SubscriptionTransaction):
        self._subscriptions[id(subscription)] = subscription

    def UnregisterSubscription(self, subscription: ClusterAttribute.SubscriptionTransaction):
        del self._subscriptions[id(subscription)]

    def Shutdown(self):

        # Shutdown all subscriptions before shutting down the stack. Please note it is not
        # possible to directly iterate over the dictionary values, because when the subscription
        # is shut down, it will remove itself from the dictionary - causing the iterator to be
        # invalidated. Hence, we need to create a local copy of the values before iterating.
        for subscription in tuple(self._subscriptions.values()):
            subscription.Shutdown()

        # Terminate Matter thread and shutdown the stack.
        self._ChipStackLib.pychip_DeviceController_StackShutdown()

        # We only shutdown the persistent storage layer AFTER we've shut down the stack,
        # since there is a possibility of interactions with the storage layer during shutdown.
        self._persistentStorage.Shutdown()
        self._persistentStorage = None

        # Stack init happens in native, but shutdown happens here unfortunately.
        # #20437 tracks consolidating these.
        self._ChipStackLib.pychip_CommonStackShutdown()
        self.completeEvent = None
        self._ChipStackLib = None
        self._chipDLLPath = None
        self.devMgr = None

        delattr(builtins, "chipStack")

    def Call(self, callFunct, timeoutMs: Optional[int] = None):
        '''Run a Python function on CHIP stack, and wait for the response.
        This function is a wrapper of PostTaskOnChipThread, which includes some handling of application specific logics.
        Calling this function on CHIP on CHIP mainloop thread will cause deadlock.
        '''
        return self.PostTaskOnChipThread(callFunct).Wait(timeoutMs)

    async def CallAsyncWithResult(self, callFunct, timeoutMs: Optional[int] = None):
        '''Run a Python function on CHIP stack, and wait for the response.
        This function will post a task on CHIP mainloop and waits for the call response in a asyncio friendly manner.
        '''
        callObj = AsyncioCallableHandle(callFunct)
        pythonapi.Py_IncRef(py_object(callObj))

        res = self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread(
            self.cbHandleChipThreadRun, py_object(callObj))

        if not res.is_success:
            pythonapi.Py_DecRef(py_object(callObj))
            raise res.to_exception()

        return await asyncio.wait_for(callObj.future, timeoutMs / 1000 if timeoutMs else None)

    async def CallAsync(self, callFunct, timeoutMs: Optional[int] = None) -> None:
        '''Run a Python function on CHIP stack, and wait for the response.'''
        res: PyChipError = await self.CallAsyncWithResult(callFunct, timeoutMs)
        res.raise_on_error()

    def PostTaskOnChipThread(self, callFunct) -> AsyncCallableHandle:
        '''Run a Python function on CHIP stack, and wait for the response.
        This function will post a task on CHIP mainloop, and return an object with Wait() method for getting the result.
        Calling Wait inside main loop will cause deadlock.
        '''
        callObj = AsyncCallableHandle(callFunct)
        pythonapi.Py_IncRef(py_object(callObj))
        res = self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread(
            self.cbHandleChipThreadRun, py_object(callObj))
        if not res.is_success:
            pythonapi.Py_DecRef(py_object(callObj))
            raise res.to_exception()
        return callObj

    def LocateChipDLL(self):
        self._loadLib()
        return self._chipDLLPath

    def _loadLib(self):
        if self._ChipStackLib is None:
            self._ChipStackLib = chip.native.GetLibraryHandle()
            self._chipDLLPath = chip.native.FindNativeLibraryPath(chip.native.Library.CONTROLLER)

            self._ChipStackLib.pychip_DeviceController_StackInit.argtypes = [c_void_p, c_bool]
            self._ChipStackLib.pychip_DeviceController_StackInit.restype = PyChipError
            self._ChipStackLib.pychip_DeviceController_StackShutdown.argtypes = []
            self._ChipStackLib.pychip_DeviceController_StackShutdown.restype = PyChipError
            self._ChipStackLib.pychip_Stack_StatusReportToString.argtypes = [
                c_uint32,
                c_uint16,
            ]
            self._ChipStackLib.pychip_Stack_StatusReportToString.restype = c_char_p

            self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread.argtypes = [
                _ChipThreadTaskRunnerFunct, py_object]
            self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread.restype = PyChipError
