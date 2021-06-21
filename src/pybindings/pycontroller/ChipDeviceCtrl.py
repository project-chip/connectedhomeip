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
from pybindings.PyChip import chip
from pybindings.PyChip.chip import Controller
from PyCHIPController import CHIPController 
from PyCHIPController.CHIPController import SetupPayload
from PyCHIPController.CHIPController import QRCodeSetupPayloadParser, ManualSetupPayloadParser
import enum
from threading import Lock, Event


__all__ = ["ChipDeviceController"]

GLOBAL_EVENT = Event()

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, c_uint32)
_DeviceAddressUpdateDelegate_OnUpdateComplete = CFUNCTYPE(
    None, c_uint64, c_uint32)

# This is a fix for WEAV-429. Jay Logue recommends revisiting this at a later
# date to allow for truely multiple instances so this is temporary.

class PyDeviceAddressUpdateDelegate(CHIPController.DeviceAddressUpdateDelegate):
    def OnAddressUpdateComplete(nodeId, error):
        print('Node ID: {}'.format(nodeId))
        print('Error: {}'.format(error))

class PyDevicePairingDelegate(Controller.DevicePairingDelegate):
    def OnPairingComplete(self, error):
        GLOBAL_EVENT.set()
        print("OnPairingComplete Error: {}".format(error))

    def OnStatusUpdate(self, status):
        print("OnStatusUpdate Status: {}".format(status))

    def OnPairingDeleted(self, error):
        print("OnPairingDeleted Error: {}".format(error))



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
        # self._ChipStack = ChipStack(bluetoothAdapter=bluetoothAdapter)
        # self._dmLib = None


        # devCtrl = c_void_p(None)
        # res = self._dmLib.pychip_DeviceController_NewDeviceController(
        #     pointer(devCtrl), controllerNodeId)
        # if res != 0:
        #     raise self._ChipStack.ErrorToException(res)

        # self.devCtrl = devCtrl
        # self._ChipStack.devCtrl = devCtrl

        # self._Cluster = ChipClusters(self._ChipStack)

        self.devCtrl = CHIPController.CHIPDeviceControllerPyBind()
        device_address_delegate = PyDeviceAddressUpdateDelegate()
        device_pairing_delegate = PyDevicePairingDelegate()
        self.devCtrl.SetDeviceAddressUpdateDelegate(device_address_delegate)
        self.devCtrl.SetDevicePairingDelegate(device_pairing_delegate)
        self.devCtrl.Init()

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

        self.state = DCState.IDLE

    def __del__(self):
        if self.devCtrl != None:
            self.devCtrl.Shutdown()
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
        GLOBAL_EVENT.clear()
        self.devCtrl.PairBLE(discriminator, setupPinCode, nodeid)
        GLOBAL_EVENT.wait(timeout=30)


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
    def DiscoverCommissioningLongDiscriminator(self, long_discriminator):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissioningLongDiscriminator(self.devCtrl, long_discriminator)
        )

    def PrintDiscoveredDevices(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_PrintDiscoveredDevices(self.devCtrl)
        )
    def PrintSetupPayload(setup_payload):
        print("Version: {}".format(setup_payload.version))
        print("Vendor ID: {}".format(setup_payload.vendorID))
        print("Product ID: {}".format(setup_payload.productID))
        print("Commissioning Flow: {}".format(setup_payload.commissioningFlow))
        print("Rendezvous Information Flag: {}".format(setup_payload.rendezvousInformation.Raw()))
        print("Discriminator: {}".format(setup_payload.discriminator))
        print("Setup Pin Code: {}".format(setup_payload.setUpPINCode))
        print("All Vendor Options: {}".format(setup_payload.getAllOptionalVendorData()))

    def ParseQRCode(self, qrCode):
        setup_payload = SetupPayload()
        qr_code_parser = QRCodeSetupPayloadParser(qrCode)
        qr_code_parser.populatePayload(setup_payload)

        return setup_payload

    def ParseManualSetupCode(self, code):
        setup_payload = SetupPayload()
        qr_code_parser = ManualSetupPayloadParser(code)
        qr_code_parser.populatePayload(setup_payload)
        return setup_payload

    def GetIPForDiscoveredDevice(self, idx, addrStr, length):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice(self.devCtrl, idx, addrStr, length)
        )

    def DiscoverAllCommissioning(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverAllCommissioning(self.devCtrl)
        )

    def GetFabricId(self):
        fabricid = c_uint64(0)

        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetFabricId(
                self.devCtrl, pointer(fabricid))
        )

        if error == 0:
            return fabricid.value
        else:
            return 0

    def ZCLSend(self, cluster, command, nodeid, endpoint, groupid, args, blocking=False):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.devCtrl, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)
        im.ClearCommandStatus(im.PLACEHOLDER_COMMAND_HANDLE)
        self._Cluster.SendCommand(
            device, cluster, command, endpoint, groupid, args, True)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(im.PLACEHOLDER_COMMAND_HANDLE, 1)
        return (0, None)

    def ZCLReadAttribute(self, cluster, attribute, nodeid, endpoint, groupid, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.devCtrl, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        # We are not using IM for Attributes.
        res = self._Cluster.ReadAttribute(
            device, cluster, attribute, endpoint, groupid, False)

    def ZCLWriteAttribute(self, cluster, attribute, nodeid, endpoint, groupid, value, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.devCtrl, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        # We are not using IM for Attributes.
        res = self._Cluster.WriteAttribute(
            device, cluster, attribute, endpoint, groupid, value, False)

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
