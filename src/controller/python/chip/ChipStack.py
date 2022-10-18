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

from __future__ import absolute_import
from __future__ import print_function
import sys
import os
import time
import glob
import platform
import logging
from threading import Lock, Event, Condition
from ctypes import *
from .ChipUtility import ChipUtility
from .storage import *
from .exceptions import *
import builtins

from .interaction_model import InteractionModelError, delegate as im
from .clusters import Command as ClusterCommand
from .clusters import Attribute as ClusterAttribute
from .clusters import ClusterObjects as ClusterObjects
from .clusters import Objects as GeneratedObjects
from .clusters.CHIPClusters import *

import chip.native
from chip.native import PyChipError

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


class LogCategory(object):
    """Debug logging categories used by chip."""

    # NOTE: These values must correspond to those used in the chip C++ code.
    Disabled = 0
    Error = 1
    Progress = 2
    Detail = 3
    Retain = 4

    @staticmethod
    def categoryToLogLevel(cat):
        if cat == LogCategory.Error:
            return logging.ERROR
        elif cat == LogCategory.Progress:
            return logging.INFO
        elif cat == LogCategory.Detail:
            return logging.DEBUG
        elif cat == LogCategory.Retain:
            return logging.CRITICAL
        else:
            return logging.NOTSET


class ChipLogFormatter(logging.Formatter):
    """A custom logging.Formatter for logging chip library messages."""

    def __init__(
        self,
        datefmt=None,
        logModulePrefix=False,
        logLevel=False,
        logTimestamp=False,
        logMSecs=True,
    ):
        fmt = "%(message)s"
        if logModulePrefix:
            fmt = "CHIP:%(chip-module)s: " + fmt
        if logLevel:
            fmt = "%(levelname)s:" + fmt
        if datefmt is not None or logTimestamp:
            fmt = "%(asctime)s " + fmt
        super(ChipLogFormatter, self).__init__(fmt=fmt, datefmt=datefmt)
        self.logMSecs = logMSecs

    def formatTime(self, record, datefmt=None):
        if datefmt is None:
            timestampStr = time.strftime("%Y-%m-%d %H:%M:%S%z")
        if self.logMSecs:
            timestampUS = record.__dict__.get("timestamp-usec", 0)
            timestampStr = "%s.%03ld" % (timestampStr, timestampUS / 1000)
        return timestampStr


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


_CompleteFunct = CFUNCTYPE(None, c_void_p, c_void_p)
_ErrorFunct = CFUNCTYPE(None, c_void_p, c_void_p,
                        c_ulong, POINTER(DeviceStatusStruct))
_LogMessageFunct = CFUNCTYPE(
    None, c_int64, c_int64, c_char_p, c_uint8, c_char_p)
_ChipThreadTaskRunnerFunct = CFUNCTYPE(None, py_object)


@_singleton
class ChipStack(object):
    def __init__(self, persistentStoragePath: str, installDefaultLogHandler=True, bluetoothAdapter=None, enableServerInteractions=True):
        builtins.enableDebugMode = False

        self.networkLock = Lock()
        self.completeEvent = Event()
        self.commissioningCompleteEvent = Event()
        self._ChipStackLib = None
        self._chipDLLPath = None
        self.devMgr = None
        self.callbackRes = None
        self.commissioningEventRes = None
        self._activeLogFunct = None
        self.addModulePrefixToLogMessage = True
        self._enableServerInteractions = enableServerInteractions

        #
        # Locate and load the chip shared library.
        # This also implictly does a minimal stack initialization (i.e call MemoryInit).
        #
        self._loadLib()

        # Arrange to log output from the chip library to a python logger object with the
        # name 'chip.ChipStack'.  If desired, applications can override this behavior by
        # setting self.logger to a different python logger object, or by calling setLogFunct()
        # with their own logging function.
        self.logger = logging.getLogger(__name__)
        self.setLogFunct(self.defaultLogFunct)

        # Determine if there are already handlers installed for the logger.  Python 3.5+
        # has a method for this; on older versions the check has to be done manually.
        if hasattr(self.logger, "hasHandlers"):
            hasHandlers = self.logger.hasHandlers()
        else:
            hasHandlers = False
            logger = self.logger
            while logger is not None:
                if len(logger.handlers) > 0:
                    hasHandlers = True
                    break
                if not logger.propagate:
                    break
                logger = logger.parent

        # If a logging handler has not already been initialized for 'chip.ChipStack',
        # or any one of its parent loggers, automatically configure a handler to log to
        # stdout.  This maintains compatibility with a number of applications which expect
        # chip log output to go to stdout by default.
        #
        # This behavior can be overridden in a variety of ways:
        #     - Initialize a different log handler before ChipStack is initialized.
        #     - Pass installDefaultLogHandler=False when initializing ChipStack.
        #     - Replace the StreamHandler on self.logger with a different handler object.
        #     - Set a different Formatter object on the existing StreamHandler object.
        #     - Reconfigure the existing ChipLogFormatter object.
        #     - Configure chip to call an application-specific logging function by
        #       calling self.setLogFunct().
        #     - Call self.setLogFunct(None), which will configure the chip library
        #       to log directly to stdout, bypassing python altogether.
        #
        if installDefaultLogHandler and not hasHandlers:
            logHandler = logging.StreamHandler(stream=sys.stdout)
            logHandler.setFormatter(ChipLogFormatter())
            self.logger.addHandler(logHandler)
            self.logger.setLevel(logging.DEBUG)

        def HandleComplete(appState, reqState):
            self.callbackRes = True
            self.completeEvent.set()

        def HandleError(appState, reqState, err, devStatusPtr):
            self.callbackRes = self.ErrorToException(err, devStatusPtr)
            self.completeEvent.set()

        @_ChipThreadTaskRunnerFunct
        def HandleChipThreadRun(callback):
            callback()

        self.cbHandleChipThreadRun = HandleChipThreadRun
        self.cbHandleComplete = _CompleteFunct(HandleComplete)
        self.cbHandleError = _ErrorFunct(HandleError)
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

    @property
    def defaultLogFunct(self):
        """Returns a python callable which, when called, logs a message to the python logger object
        currently associated with the ChipStack object.
        The returned function is suitable for passing to the setLogFunct() method."""

        def logFunct(timestamp, timestampUSec, moduleName, logCat, message):
            moduleName = ChipUtility.CStringToString(moduleName)
            message = ChipUtility.CStringToString(message)
            if self.addModulePrefixToLogMessage:
                message = "CHIP:%s: %s" % (moduleName, message)
            logLevel = LogCategory.categoryToLogLevel(logCat)
            msgAttrs = {
                "chip-module": moduleName,
                "timestamp": timestamp,
                "timestamp-usec": timestampUSec,
            }
            self.logger.log(logLevel, message, extra=msgAttrs)

        return logFunct

    def setLogFunct(self, logFunct):
        """Set the function used by the chip library to log messages.
        The supplied object must be a python callable that accepts the following
        arguments:
           timestamp (integer)
           timestampUS (integer)
           module name (encoded UTF-8 string)
           log category (integer)
           message (encoded UTF-8 string)
        Specifying None configures the chip library to log directly to stdout."""
        if logFunct is None:
            logFunct = 0
        if not isinstance(logFunct, _LogMessageFunct):
            logFunct = _LogMessageFunct(logFunct)
        with self.networkLock:
            # NOTE: ChipStack must hold a reference to the CFUNCTYPE object while it is
            # set. Otherwise it may get garbage collected, and logging calls from the
            # chip library will fail.
            self._activeLogFunct = logFunct
            self._ChipStackLib.pychip_Stack_SetLogFunct(logFunct)

    def Shutdown(self):
        self.Call(lambda: self._ChipStackLib.pychip_DeviceController_StackShutdown()).raise_on_error()

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
        # throw error if op in progress
        self.callbackRes = None
        self.completeEvent.clear()
        with self.networkLock:
            res = self.PostTaskOnChipThread(callFunct).Wait(timeoutMs)
        self.completeEvent.set()
        if res == 0 and self.callbackRes != None:
            return self.callbackRes
        return res

    def CallAsync(self, callFunct):
        '''Run a Python function on CHIP stack, and wait for the application specific response.
        This function is a wrapper of PostTaskOnChipThread, which includes some handling of application specific logics.
        Calling this function on CHIP on CHIP mainloop thread will cause deadlock.
        '''
        # throw error if op in progress
        self.callbackRes = None
        self.completeEvent.clear()
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
            if sysErrorCode != None:
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
            self._chipDLLPath = chip.native.FindNativeLibraryPath()

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
            self._ChipStackLib.pychip_Stack_SetLogFunct.argtypes = [
                _LogMessageFunct]
            self._ChipStackLib.pychip_Stack_SetLogFunct.restype = PyChipError

            self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread.argtypes = [
                _ChipThreadTaskRunnerFunct, py_object]
            self._ChipStackLib.pychip_DeviceController_PostTaskOnChipThread.restype = PyChipError
