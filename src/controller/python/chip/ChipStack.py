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
import os
from ctypes import CFUNCTYPE, Structure, c_bool, c_char_p, c_uint16, c_uint32, c_void_p, py_object, pythonapi
from threading import Condition, Event, Lock

import chip.native
from chip.native import PyChipError

from .ChipUtility import ChipUtility
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

    def Wait(self, timeoutMs: int = None):
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

        # TODO: Probably no longer necessary, see https://github.com/project-chip/connectedhomeip/issues/33321.
        self.networkLock = Lock()
        self.completeEvent = Event()
        self.commissioningCompleteEvent = Event()
        self._ChipStackLib = None
        self._chipDLLPath = None
        self.devMgr = None
        self.callbackRes = None
        self.commissioningEventRes = None
        self.openCommissioningWindowPincode = {}
        self._enableServerInteractions = enableServerInteractions

        #
        # Locate and load the chip shared library.
        # This also implictly does a minimal stack initialization (i.e call MemoryInit).
        #
        self._loadLib()

        @_ChipThreadTaskRunnerFunct
        def HandleChipThreadRun(callback):
            callback()

        self.cbHandleChipThreadRun = HandleChipThreadRun
        # set by other modules(BLE) that require service by thread while thread blocks.
        self.blockingCB = None

        #
        # Storage has to be initialized BEFORE initializing the stack, since the latter
        # requires a PersistentStorageDelegate to be provided to DeviceControllerFactory.
        #
        self._persistentStorage = PersistentStorage(persistentStoragePath)

        # Initialize the chip stack.
        res = self._ChipStackLib.pychip_DeviceController_StackInit(
            self._persistentStorage.GetSdkStorageObject(), enableServerInteractions)
        res.raise_on_error()

        im.InitIMDelegate()
        ClusterAttribute.Init()
        ClusterCommand.Init()

        builtins.chipStack = self

    def GetStorageManager(self):
        return self._persistentStorage

    @property
    def enableServerInteractions(self):
        return self._enableServerInteractions

    def Shutdown(self):
        #
        # Terminate Matter thread and shutdown the stack.
        #
        self._ChipStackLib.pychip_DeviceController_StackShutdown()

        #
        # We only shutdown the persistent storage layer AFTER we've shut down the stack,
        # since there is a possibility of interactions with the storage layer during shutdown.
        #
        self._persistentStorage.Shutdown()
        self._persistentStorage = None

        #
        # Stack init happens in native, but shutdown happens here unfortunately.
        # #20437 tracks consolidating these.
        #
        self._ChipStackLib.pychip_CommonStackShutdown()
        self.networkLock = None
        self.completeEvent = None
        self._ChipStackLib = None
        self._chipDLLPath = None
        self.devMgr = None
        self.callbackRes = None

        delattr(builtins, "chipStack")

    def Call(self, callFunct, timeoutMs: int = None):
        '''Run a Python function on CHIP stack, and wait for the response.
        This function is a wrapper of PostTaskOnChipThread, which includes some handling of application specific logics.
        Calling this function on CHIP on CHIP mainloop thread will cause deadlock.
        '''
        # TODO: Lock probably no longer necessary, see https://github.com/project-chip/connectedhomeip/issues/33321.
        with self.networkLock:
            res = self.PostTaskOnChipThread(callFunct).Wait(timeoutMs)
        return res

    async def CallAsync(self, callFunct, timeoutMs: int = None):
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

    def CallAsyncWithCompleteCallback(self, callFunct):
        '''Run a Python function on CHIP stack, and wait for the application specific response.
        This function is a wrapper of PostTaskOnChipThread, which includes some handling of application specific logics.
        Calling this function on CHIP on CHIP mainloop thread will cause deadlock.
        Make sure to register the necessary callbacks which release the function by setting the completeEvent.
        '''
        # throw error if op in progress
        self.callbackRes = None
        self.completeEvent.clear()
        # TODO: Lock probably no longer necessary, see https://github.com/project-chip/connectedhomeip/issues/33321.
        with self.networkLock:
            res = self.PostTaskOnChipThread(callFunct).Wait()

        if not res.is_success:
            self.completeEvent.set()
            raise res.to_exception()
        while not self.completeEvent.isSet():
            if self.blockingCB:
                self.blockingCB()

            self.completeEvent.wait(0.05)
        if isinstance(self.callbackRes, ChipStackException):
            raise self.callbackRes
        return self.callbackRes

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

    def ErrorToException(self, err, devStatusPtr=None):
        if err == 0x2C and devStatusPtr:
            devStatus = devStatusPtr.contents
            msg = ChipUtility.CStringToString(
                (
                    self._ChipStackLib.pychip_Stack_StatusReportToString(
                        devStatus.ProfileId, devStatus.StatusCode
                    )
                )
            )
            sysErrorCode = (
                devStatus.SysErrorCode if (
                    devStatus.SysErrorCode != 0) else None
            )
            if sysErrorCode is not None:
                msg = msg + " (system err %d)" % (sysErrorCode)
            return DeviceError(
                devStatus.ProfileId, devStatus.StatusCode, sysErrorCode, msg
            )
        else:
            return ChipStackError(
                err,
                ChipUtility.CStringToString(
                    (self._ChipStackLib.pychip_Stack_ErrorToString(err))
                ),
            )

    def LocateChipDLL(self):
        self._loadLib()
        return self._chipDLLPath

    # ----- Private Members -----
    def _AllDirsToRoot(self, dir):
        dir = os.path.abspath(dir)
        while True:
            yield dir
            parent = os.path.dirname(dir)
            if parent == "" or parent == dir:
                break
            dir = parent

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
            self._ChipStackLib.pychip_Stack_ErrorToString.argtypes = [c_uint32]
            self._ChipStackLib.pychip_Stack_ErrorToString.restype = c_char_p

            self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread.argtypes = [
                _ChipThreadTaskRunnerFunct, py_object]
            self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread.restype = PyChipError
