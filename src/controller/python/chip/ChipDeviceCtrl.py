#
#    Copyright (c) 2020-2021 Project CHIP Authors
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
from .clusters.CHIPClusters import *
from .interaction_model import delegate as im
from .exceptions import *
import enum


__all__ = ["ChipDeviceController"]

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, c_uint32)
_DeviceAddressUpdateDelegate_OnUpdateComplete = CFUNCTYPE(
    None, c_uint64, c_uint32)

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
    def __init__(self, startNetworkThread=True, controllerNodeId=0, bluetoothAdapter=None):
        self.state = DCState.NOT_INITIALIZED
        self.devCtrl = None
        if bluetoothAdapter is None:
            bluetoothAdapter = 0
        self._ChipStack = ChipStack(bluetoothAdapter=bluetoothAdapter)
        self._dmLib = None

        self._InitLib()

        devCtrl = c_void_p(None)
        res = self._dmLib.pychip_DeviceController_NewDeviceController(
            pointer(devCtrl), controllerNodeId)
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        self.devCtrl = devCtrl
        self._ChipStack.devCtrl = devCtrl

        self._Cluster = ChipClusters(self._ChipStack)
        self._Cluster.InitLib(self._dmLib)

        def HandleKeyExchangeComplete(err):
            if err != 0:
                print("Failed to establish secure session to device: {}".format(err))
                self._ChipStack.callbackRes = self._ChipStack.ErrorToException(
                    err)
            else:
                print("Secure Session to Device Established")
                self._ChipStack.callbackRes = True
            self.state = DCState.IDLE
            self._ChipStack.completeEvent.set()

        def HandleAddressUpdateComplete(nodeid, err):
            if err != 0:
                print("Failed to update node address: {}".format(err))
            else:
                print("Node address has been updated")
            self.state = DCState.IDLE
            self._ChipStack.callbackRes = err
            self._ChipStack.completeEvent.set()

        im.InitIMDelegate()

        self.cbHandleKeyExchangeCompleteFunct = _DevicePairingDelegate_OnPairingCompleteFunct(
            HandleKeyExchangeComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(
            self.devCtrl, self.cbHandleKeyExchangeCompleteFunct)

        self.cbOnAddressUpdateComplete = _DeviceAddressUpdateDelegate_OnUpdateComplete(
            HandleAddressUpdateComplete)
        self._dmLib.pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete(
            self.cbOnAddressUpdateComplete)

        self.state = DCState.IDLE

    def __del__(self):
        if self.devCtrl != None:
            self._dmLib.pychip_DeviceController_DeleteDeviceController(
                self.devCtrl)
            self.devCtrl = None

    def IsConnected(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_IsConnected(
                self.devCtrl)
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

    def ConnectBLE(self, discriminator, setupPinCode, nodeid):
        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectBLE(
                self.devCtrl, discriminator, setupPinCode, nodeid)
        )

    def CloseBLEConnection(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceCommissioner_CloseBleConnection(self.devCtrl)
        )

    def ConnectIP(self, ipaddr, setupPinCode, nodeid):
        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectIP(
                self.devCtrl, ipaddr, setupPinCode, nodeid)
        )

    def ResolveNode(self, fabricid, nodeid):
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_Resolver_ResolveNode(fabricid, nodeid)
        )

    def GetAddressAndPort(self, nodeid):
        address = create_string_buffer(64)
        port = c_uint16(0)

        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetAddressAndPort(
                self.devCtrl, nodeid, address, 64, pointer(port))
        )

        return (address.value.decode(), port.value) if error == 0 else None

    def ZCLSend(self, cluster, command, nodeid, endpoint, groupid, args, blocking=False):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.devCtrl, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        commandSenderHandle = self._dmLib.pychip_GetCommandSenderHandle(device)
        im.ClearCommandStatus(commandSenderHandle)
        self._Cluster.SendCommand(
            device, cluster, command, endpoint, groupid, args, commandSenderHandle != 0)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(commandSenderHandle, 1)
        return (0, None)

    def ZCLReadAttribute(self, cluster, attribute, nodeid, endpoint, groupid, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.devCtrl, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        commandSenderHandle = self._dmLib.pychip_GetCommandSenderHandle(device)
        im.ClearCommandStatus(commandSenderHandle)
        res = self._Cluster.ReadAttribute(
            device, cluster, attribute, endpoint, groupid, commandSenderHandle != 0)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(commandSenderHandle, 1)

    def ZCLConfigureAttribute(self, cluster, attribute, nodeid, endpoint, minInterval, maxInterval, change, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.devCtrl, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        commandSenderHandle = self._dmLib.pychip_GetCommandSenderHandle(device)
        im.ClearCommandStatus(commandSenderHandle)
        res = self._Cluster.ConfigureAttribute(
            device, cluster, attribute, endpoint, minInterval, maxInterval, change, commandSenderHandle != 0)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(commandSenderHandle, 1)

    def ZCLCommandList(self):
        return self._Cluster.ListClusterCommands()

    def ZCLAttributeList(self):
        return self._Cluster.ListClusterAttributes()

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
        ret = self._dmLib.pychip_ScriptDevicePairingDelegate_SetWifiCredential(
            self.devCtrl, ssid.encode("utf-8") + b'\0', password.encode("utf-8") + b'\0')
        if ret != 0:
            raise self._ChipStack.ErrorToException(res)

    def SetThreadCredential(self, channel, panid, masterKey):
        ret = self._dmLib.pychip_ScriptDevicePairingDelegate_SetThreadCredential(
            self.devCtrl, channel, panid, masterKey.encode("utf-8") + b'\0')
        if ret != 0:
            raise self._ChipStack.ErrorToException(ret)

    # ----- Private Members -----
    def _InitLib(self):
        if self._dmLib is None:
            self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

            self._dmLib.pychip_DeviceController_NewDeviceController.argtypes = [
                POINTER(c_void_p), c_uint64]
            self._dmLib.pychip_DeviceController_NewDeviceController.restype = c_uint32

            self._dmLib.pychip_DeviceController_DeleteDeviceController.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_DeleteDeviceController.restype = c_uint32

            self._dmLib.pychip_DeviceController_ConnectBLE.argtypes = [
                c_void_p, c_uint16, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectBLE.restype = c_uint32

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectIP.restype = c_uint32

            self._dmLib.pychip_DeviceController_GetAddressAndPort.argtypes = [
                c_void_p, c_uint64, c_char_p, c_uint64, POINTER(c_uint16)]
            self._dmLib.pychip_DeviceController_GetAddressAndPort.restype = c_uint32

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetWifiCredential.argtypes = [
                c_void_p, c_char_p, c_char_p]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetWifiCredential.restype = c_uint32

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnPairingCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.restype = c_uint32

            self._dmLib.pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete.argtypes = [
                _DeviceAddressUpdateDelegate_OnUpdateComplete]
            self._dmLib.pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete.restype = None

            self._dmLib.pychip_Resolver_ResolveNode.argtypes = [
                c_uint64, c_uint64]
            self._dmLib.pychip_Resolver_ResolveNode.restype = c_uint32

            self._dmLib.pychip_GetDeviceByNodeId.argtypes = [
                c_void_p, c_uint64, POINTER(c_void_p)]
            self._dmLib.pychip_GetDeviceByNodeId.restype = c_uint32

            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.argtypes = [c_void_p]
            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.restype = c_uint32

            self._dmLib.pychip_GetCommandSenderHandle.argtypes = [c_void_p]
            self._dmLib.pychip_GetCommandSenderHandle.restype = c_uint64

