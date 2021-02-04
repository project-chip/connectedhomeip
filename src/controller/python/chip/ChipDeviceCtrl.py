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

"""Chip Device Controller interface
"""

from __future__ import absolute_import
from __future__ import print_function
import time
from threading import Thread
from ctypes import *
from .ChipStack import *
from .ChipCluster import *
import enum


__all__ = ["ChipDeviceController"]

# typedef void (*OnConnectFunct)(Chip::DeviceController::hipDeviceController * dc,
#                                const chip::Transport::PeerConnectionState * state, void * appReqState);
# typedef void (*OnErrorFunct)(Chip::DeviceController::ChipDeviceController * dc, void * appReqState, CHIP_ERROR err,
#                              const Inet::IPPacketInfo * pi);
# typedef void (*OnMessageFunct)(Chip::DeviceController::ChipDeviceController * dc, void * appReqState, PacketBuffer * buffer);

_OnConnectFunct = CFUNCTYPE(None, c_void_p, c_void_p, c_void_p)
_OnRendezvousErrorFunct = CFUNCTYPE(None, c_void_p, c_void_p, c_uint32, c_void_p)
_OnMessageFunct = CFUNCTYPE(None, c_void_p, c_void_p, c_void_p)

# This is a fix for WEAV-429. Jay Logue recommends revisiting this at a later
# date to allow for truely multiple instances so this is temporary.
def _singleton(cls):
    instance = [None]

    def wrapper(*args, **kwargs):
        if instance[0] is None:
            instance[0] = cls(*args, **kwargs)
        return instance[0]

    return wrapper


class DCState(enum.IntEnum):
    NOT_INITIALIZED = 0
    IDLE = 1
    BLE_READY = 2
    RENDEZVOUS_ONGOING = 3
    RENDEZVOUS_CONNECTED = 4

@_singleton
class ChipDeviceController(object):
    def __init__(self, startNetworkThread=True):
        self.state = DCState.NOT_INITIALIZED
        self.devCtrl = None
        self._ChipStack = ChipStack()
        self._dmLib = None

        self._InitLib()

        devCtrl = c_void_p(None)
        res = self._dmLib.pychip_DeviceController_NewDeviceController(pointer(devCtrl))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        pairingDelegate = c_void_p(None)
        res = self._dmLib.pychip_ScriptDevicePairingDelegate_NewPairingDelegate(pointer(pairingDelegate))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        res = self._dmLib.pychip_DeviceController_SetDevicePairingDelegate(devCtrl, pairingDelegate)
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        self.devCtrl = devCtrl
        self.pairingDelegate = pairingDelegate
        self._ChipStack.devCtrl = devCtrl

        self._Cluster = ChipCluster(self._ChipStack)
        self._Cluster.InitLib(self._dmLib)

        def DeviceCtrlHandleMessage(appReqState, buffer):
            pass

        self.cbHandleMessage = _OnMessageFunct(DeviceCtrlHandleMessage)

        def HandleRendezvousError(appState, reqState, err, devStatusPtr):
            if self.state == DCState.RENDEZVOUS_ONGOING:
                print("Failed to connect to device: {}".format(err))
                self._ChipStack.callbackRes = True
                self._ChipStack.completeEvent.set()
            elif self.state == DCState.RENDEZVOUS_CONNECTED:
                print("Disconnected from device")

        self.cbHandleRendezvousError = _OnRendezvousErrorFunct(HandleRendezvousError)

        self.state = DCState.IDLE

    def __del__(self):
        if self.devCtrl != None:
            self._dmLib.pychip_DeviceController_DeleteDeviceController(self.devCtrl)
            self.devCtrl = None

    def IsConnected(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_IsConnected(self.devCtrl)
        )

    def ConnectBle(self, bleConnection):
        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ValidateBTP(
                self.devCtrl,
                bleConnection,
                self._ChipStack.cbHandleComplete,
                self._ChipStack.cbHandleError,
            )
        )

    def ConnectBLE(self, discriminator, setupPinCode):
        def HandleComplete(dc, connState, appState):
            print("Rendezvous Initialized")
            self.state = DCState.RENDEZVOUS_CONNECTED
            self._ChipStack.callbackRes = True
            self._ChipStack.completeEvent.set()
        onConnectFunct = _OnConnectFunct(HandleComplete)

        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectBLE(
                self.devCtrl, discriminator, setupPinCode, onConnectFunct, self.cbHandleMessage, self.cbHandleRendezvousError)
        )

    def ConnectIP(self, ipaddr, setupPinCode):
        def HandleComplete(dc, connState, appState):
            print("Rendezvous Initialized")
            self.state = DCState.RENDEZVOUS_CONNECTED
            self._ChipStack.callbackRes = True
            self._ChipStack.completeEvent.set()
        onConnectFunct = _OnConnectFunct(HandleComplete)

        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectIP(
                self.devCtrl, ipaddr, setupPinCode, onConnectFunct, self.cbHandleMessage, self.cbHandleRendezvousError)
        )

    def ZCLSend(self, cluster, command, nodeid, endpoint, groupid, args):
        device = c_void_p(None)
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_GetDeviceByNodeId(self.devCtrl, nodeid, pointer(device))
        )
        self._Cluster.SendCommand(device, cluster, command, endpoint, groupid, args)

    def ZCLList(self):
        return self._Cluster.ListClusters()

    def Close(self):
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_Close(self.devCtrl)
        )

    def SetLogFilter(self, category):
        if category < 0 or category > pow(2, 8):
            raise ValueError("category must be an unsigned 8-bit integer")

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetLogFilter(category)
        )

    def GetLogFilter(self):
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetLogFilter()
        )

    def SetBlockingCB(self, blockingCB):
        self._ChipStack.blockingCB = blockingCB

    def SetWifiCredential(self, ssid, password):
        ret = self._dmLib.pychip_ScriptDevicePairingDelegate_SetWifiCredential(self.pairingDelegate, ssid.encode("utf-8"), password.encode("utf-8"))
        if ret != 0:
            raise self._ChipStack.ErrorToException(res)

    # ----- Private Members -----
    def _InitLib(self):
        if self._dmLib is None:
            self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

            self._dmLib.pychip_DeviceController_NewDeviceController.argtypes = [
                POINTER(c_void_p)
            ]
            self._dmLib.pychip_DeviceController_NewDeviceController.restype = c_uint32

            self._dmLib.pychip_DeviceController_DeleteDeviceController.argtypes = [
                c_void_p
            ]
            self._dmLib.pychip_DeviceController_DeleteDeviceController.restype = (
                c_uint32
            )

            self._dmLib.pychip_DeviceController_Close.argtypes = [c_void_p]
            self._dmLib.pychip_DeviceController_Close.restype = None

            self._dmLib.pychip_DeviceController_ConnectBLE.argtypes = [
                c_void_p, c_uint16, c_uint32, _OnConnectFunct, _OnMessageFunct, _OnRendezvousErrorFunct]
            self._dmLib.pychip_DeviceController_ConnectBLE.restype = c_uint32

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, _OnConnectFunct, _OnMessageFunct, _OnRendezvousErrorFunct]
            self._dmLib.pychip_DeviceController_ConnectIP.restype = c_uint32

            self._dmLib.pychip_ScriptDevicePairingDelegate_NewPairingDelegate.argtypes = [POINTER(c_void_p)]
            self._dmLib.pychip_ScriptDevicePairingDelegate_NewPairingDelegate.restype = c_uint32

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetWifiCredential.argtypes = [c_void_p, c_char_p, c_char_p]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetWifiCredential.restype = c_uint32

            self._dmLib.pychip_DeviceController_SetDevicePairingDelegate.argtypes = [c_void_p, c_void_p]
            self._dmLib.pychip_DeviceController_SetDevicePairingDelegate.restype = c_uint32

            self._dmLib.pychip_GetDeviceByNodeId.argtypes = [c_void_p, c_uint64, POINTER(c_void_p)]
            self._dmLib.pychip_GetDeviceByNodeId.restype = c_uint32
