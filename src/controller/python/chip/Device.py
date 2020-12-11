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
#      Python interface for Device
#

from abc import ABC, abstractmethod
from ctypes import *
from enum import Enum

class DeviceStatusDelegate(ABC):
    OnStatusChangeFunction = CFUNCTYPE(None)

    def __init__(self, dmLib):
        self.__dmLib = dmLib

        def HandleStatusChange():
            self.OnStatusChange()

        self.__delegate = c_void_p(None)
        # hold the reference to the callback to prevent it from being disposed.
        self.handleOnStatusChange = DeviceStatusDelegate.OnStatusChangeFunction(HandleStatusChange)
        self.__dmLib.nl_Chip_DeviceStatusDelegate_Create(pointer(self.__delegate), self.handleOnStatusChange);
        pass

    def __del__(self):
        self.__dmLib.nl_Chip_DeviceStatusDelegate_Destory(self.__delegate);
        pass

    def GetCtypesDelegateObject(self):
        return self.__delegate

    @abstractmethod
    def OnStatusChange(self):
        pass


class Device(DeviceStatusDelegate):
    class State(Enum):
        NotConnected = 1
        PaseConnecting = 2
        PaseConnected = 3

    def __init__(self, chipStack, devCtrlNewApi, localNodeId):
        self._ChipStack = chipStack
        self._loadLib()
        super().__init__(self.__dmLib)

        self.state = self.State.NotConnected

        device = c_void_p(None)
        res = self.__dmLib.nl_Chip_Controller_NewDevice(devCtrlNewApi, localNodeId, pointer(device))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)
        self.device = device

        self.SetStatusDelegate(self)

    def SetStatusDelegate(self, delegate):
        self.__dmLib.nl_Chip_Device_SetDelegate(self.device, delegate.GetCtypesDelegateObject());

    def ConnectIP(self, ipaddr, setupPinCode):
        self.state =self.State.PaseConnecting
        return self._ChipStack.CallAsync(
            lambda: self.__dmLib.nl_Chip_Device_EstablishPaseSession(self.device, ipaddr, setupPinCode)
        )

    def OnStatusChange(self):
        self.state = self.State.PaseConnected
        self._ChipStack.callbackRes = True
        self._ChipStack.completeEvent.set()

    def _loadLib(self):
        if not hasattr(self, '__dmLib'):
            self.__dmLib = CDLL(self._ChipStack.LocateChipDLL())
