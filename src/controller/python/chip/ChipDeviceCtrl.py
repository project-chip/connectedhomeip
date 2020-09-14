#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2019-2020 Google, LLC.
#    Copyright (c) 2013-2018 Nest Labs, Inc.
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
#      Python interface for Chip Device Manager
#

"""Chip Device Manager interface
"""

from __future__ import absolute_import
from __future__ import print_function
import functools
import sys
import os
import re
import copy
import binascii
import datetime
import time
import glob
import platform
import ast
from threading import Thread, Lock, Event
from ctypes import *
import six
from six.moves import range
from .ChipUtility import ChipUtility
from .ChipStack import *


__all__ = [ 'ChipDeviceController' ]

TargetDeviceMode_Any                        = 0x00000000    # Locate all devices regardless of mode.
TargetDeviceMode_UserSelectedMode           = 0x00000001    # Locate all devices in 'user-selected' mode -- i.e. where the device has
                                                            #   has been directly identified by a user, e.g. by pressing a button.

TargetVendorId_Any                          = 0xFFFF
TargetProductId_Any                         = 0xFFFF
TargetDeviceId_Any                          = 0xFFFFFFFFFFFFFFFF

_CompleteFunct                              = CFUNCTYPE(None, c_void_p, c_void_p)
_ErrorFunct                                 = CFUNCTYPE(None, c_void_p, c_void_p, c_ulong, POINTER(DeviceStatusStruct))
_GetBleEventFunct                           = CFUNCTYPE(c_void_p)
_WriteBleCharacteristicFunct                = CFUNCTYPE(c_bool, c_void_p, c_void_p, c_void_p, c_void_p, c_uint16)
_SubscribeBleCharacteristicFunct            = CFUNCTYPE(c_bool, c_void_p, c_void_p, c_void_p, c_bool)
_CloseBleFunct                              = CFUNCTYPE(c_bool, c_void_p)

# This is a fix for WEAV-429. Jay Logue recommends revisiting this at a later
# date to allow for truely multiple instances so this is temporary.
def _singleton(cls):
    instance = [None]

    def wrapper(*args, **kwargs):
        if instance[0] is None:
            instance[0] = cls(*args, **kwargs)
        return instance[0]

    return wrapper

@_singleton
class ChipDeviceController(object):
    def __init__(self, startNetworkThread=True):
        self.devCtrl = None
        self.networkThread = None
        self.networkThreadRunable = False
        self._ChipStack = ChipStack()
        self._dmLib = None

        self._InitLib()

        devCtrl = c_void_p(None)
        res = self._dmLib.nl_Chip_DeviceController_NewDeviceController(pointer(devCtrl))
        if (res != 0):
            raise self._ChipStack.ErrorToException(res)

        self.devCtrl = devCtrl
        self._ChipStack.devCtrl = devCtrl

        self.blockingCB = None # set by other modules(BLE) that require service by thread while thread blocks.
        self.cbHandleBleEvent = None # set by other modules (BLE) that provide event callback to Chip.
        self.cbHandleBleWriteChar = None
        self.cbHandleBleSubscribeChar = None
        self.cbHandleBleClose = None

        if (startNetworkThread):
            self.StartNetworkThread()

    def __del__(self):
        if (self.devCtrl != None):
            self._dmLib.nl_Chip_DeviceController_DeleteDeviceManager(self.devCtrl)
            self.devCtrl = None

    def DriveBleIO(self):
        # perform asynchronous write to pipe in IO thread's select() to wake for BLE input
        res = self._dmLib.nl_Chip_DeviceController_WakeForBleIO()
        if (res != 0):
            raise self._ChipStack.ErrorToException(res)

    def SetBleEventCB(self, bleEventCB):
        if (self.devCtrl != None):
            self.cbHandleBleEvent = _GetBleEventFunct(bleEventCB)
            self._dmLib.nl_Chip_DeviceController_SetBleEventCB(self.cbHandleBleEvent)

    def SetBleWriteCharCB(self, bleWriteCharCB):
        if (self.devCtrl != None):
            self.cbHandleBleWriteChar = _WriteBleCharacteristicFunct(bleWriteCharCB)
            self._dmLib.nl_Chip_DeviceController_SetBleWriteCharacteristic(self.cbHandleBleWriteChar)

    def SetBleSubscribeCharCB(self, bleSubscribeCharCB):
        if (self.devCtrl != None):
            self.cbHandleBleSubscribeChar = _SubscribeBleCharacteristicFunct(bleSubscribeCharCB)
            self._dmLib.nl_Chip_DeviceController_SetBleSubscribeCharacteristic(self.cbHandleBleSubscribeChar)

    def SetBleCloseCB(self, bleCloseCB):
        if (self.devCtrl != None):
            self.cbHandleBleClose = _CloseBleFunct(bleCloseCB)
            self._dmLib.nl_Chip_DeviceController_SetBleClose(self.cbHandleBleClose)

    def SetBleCloseCB(self, bleCloseCB):
        if (self.devCtrl != None):
            self.cbHandleBleClose = _CloseBleFunct(bleCloseCB)
            self._dmLib.nl_Chip_DeviceController_SetBleClose(self.cbHandleBleClose)

    def StartNetworkThread(self):
        if (self.networkThread != None):
            return

        def RunNetworkThread():
            while (self.networkThreadRunable):
                self._ChipStack.networkLock.acquire()
                self._dmLib.nl_Chip_DeviceController_DriveIO(50)
                self._ChipStack.networkLock.release()
                time.sleep(0.005)

        self.networkThread = Thread(target=RunNetworkThread, name="ChipNetworkThread")
        self.networkThread.daemon = True
        self.networkThreadRunable = True
        self.networkThread.start()

    def IsConnected(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.nl_Chip_DeviceController_IsConnected(self.devCtrl)
        )


    def ConnectBle(self, bleConnection):
        self._ChipStack.CallAsync(
            lambda: self._dmLib.nl_Chip_DeviceController_ValidateBTP(self.devCtrl, bleConnection, self._ChipStack.cbHandleComplete, self._ChipStack.cbHandleError)
        )

    def Close(self):
        self._ChipStack.Call(
            lambda: self._dmLib.nl_Chip_DeviceController_Close(self.devCtrl)
        )

    def SetLogFilter(self, category):
        if category < 0 or category > pow(2, 8):
            raise ValueError("category must be an unsigned 8-bit integer")

        self._ChipStack.Call(
            lambda: self._dmLib.nl_Chip_DeviceController_SetLogFilter(category)
        )

    def GetLogFilter(self):
        self._ChipStack.Call(
            lambda: self._dmLib.nl_Chip_DeviceController_GetLogFilter()
        )

    def SetBlockingCB(self, blockingCB):
        self._ChipStack.blockingCB = blockingCB

    # ----- Private Members -----
    def _InitLib(self):
        if (self._dmLib == None):
            self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

            self._dmLib.nl_Chip_DeviceController_NewDeviceController.argtypes = [ POINTER(c_void_p) ]
            self._dmLib.nl_Chip_DeviceController_NewDeviceController.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_DeleteDeviceController.argtypes = [ c_void_p ]
            self._dmLib.nl_Chip_DeviceController_DeleteDeviceController.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_Close.argtypes = [ c_void_p ]
            self._dmLib.nl_Chip_DeviceController_Close.restype = None

            self._dmLib.nl_Chip_DeviceController_DriveIO.argtypes = [ c_uint32 ]
            self._dmLib.nl_Chip_DeviceController_DriveIO.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_WakeForBleIO.argtypes = [ ]
            self._dmLib.nl_Chip_DeviceController_WakeForBleIO.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_SetBleEventCB.argtypes = [ _GetBleEventFunct ]
            self._dmLib.nl_Chip_DeviceController_SetBleEventCB.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_SetBleWriteCharacteristic.argtypes = [ _WriteBleCharacteristicFunct ]
            self._dmLib.nl_Chip_DeviceController_SetBleWriteCharacteristic.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_SetBleSubscribeCharacteristic.argtypes = [ _SubscribeBleCharacteristicFunct ]
            self._dmLib.nl_Chip_DeviceController_SetBleSubscribeCharacteristic.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_SetBleClose.argtypes = [ _CloseBleFunct ]
            self._dmLib.nl_Chip_DeviceController_SetBleClose.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_IsConnected.argtypes = [ c_void_p ]
            self._dmLib.nl_Chip_DeviceController_IsConnected.restype = c_bool

            self._dmLib.nl_Chip_DeviceController_ValidateBTP.argtypes = [ c_void_p, c_void_p, _CompleteFunct, _ErrorFunct ]
            self._dmLib.nl_Chip_DeviceController_ValidateBTP.restype = c_uint32

            self._dmLib.nl_Chip_DeviceController_GetLogFilter.argtypes = [ ]
            self._dmLib.nl_Chip_DeviceController_GetLogFilter.restype = c_uint8

            self._dmLib.nl_Chip_DeviceController_SetLogFilter.argtypes = [ c_uint8 ]
            self._dmLib.nl_Chip_DeviceController_SetLogFilter.restype = None

