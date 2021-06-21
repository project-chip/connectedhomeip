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
import sys

from clusters.CHIPClusters import ChipClusters
from pybindings.PyChip import ChipExceptions
from pybindings.PyChip.chip import PeerId
from pybindings.PyChip.chip import Logging
from pybindings.PyChip.chip import Controller
from pybindings.PyChip.chip import Inet
from pybindings.PyChip.chip import Mdns
from pybindings.PyChip.chip import Platform
from pybindings.PyChip.chip import Transport
from pybindings.PyChip.chip import RendezvousParameters
from pybindings.PyChip.chip import DeviceLayer
from pybindings.PyChip.chip import PersistentStorageDelegate
from pybindings.PyChip.chip import QRCodeSetupPayloadParser, ManualSetupPayloadParser, SetupPayload

import enum
from threading import Lock, Event

def controllerLogDetail(msg:str):
    Logging.Log(Logging.kLogModule_Controller, Logging.kLogCategory_Detail, msg)

def controllerLogProgress(msg:str):
    Logging.Log(Logging.kLogModule_Controller, Logging.kLogCategory_Progress, msg)

IS_LINUX = sys.platform == "linux" or sys.platform == "linux2"

__all__ = ["ChipDeviceController"]

GLOBAL_EVENT = Event()

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, c_uint32)
_DeviceAddressUpdateDelegate_OnUpdateComplete = CFUNCTYPE(
    None, c_uint64, c_uint32)

# This is a fix for WEAV-429. Jay Logue recommends revisiting this at a later
# date to allow for truely multiple instances so this is temporary.

class PyPersistentStorageDelegate(PersistentStorageDelegate):
    data = {}
    def SyncGetKeyValue(self, key,  buffer, size):
        controllerLogDetail('SyncGetKeyValue key: {}'.format(key))
        controllerLogDetail('Stored Value: {}'.format(self.data.get(key, 4016)))
        return self.data.get(key, 4016)

    def SyncSetKeyValue(self, key, value, size):
        controllerLogDetail('SyncSetKeyValue Key: {}'.format(key))
        controllerLogDetail('SyncSetKeyValue Value: {}'.format(value))
        self.data[key] = value
        return 0

    def SyncDeleteKeyValue(self, key):
        controllerLogDetail('SyncDeleteKeyValue: {}'.format(key))
        del self.data[key]
        return 0

class PyDeviceAddressUpdateDelegate(Controller.DeviceAddressUpdateDelegate):
    def OnAddressUpdateComplete(nodeId, error):
        controllerLogDetail('Node ID: {}'.format(nodeId))
        controllerLogDetail('Error: {}'.format(error))

class PyDevicePairingDelegate(Controller.DevicePairingDelegate):
    def OnPairingComplete(self, error):
        GLOBAL_EVENT.set()
        controllerLogProgress("OnPairingComplete Error: {}".format(error))

    def OnStatusUpdate(self, status):
        controllerLogProgress("OnStatusUpdate Status: {}".format(status))

    def OnPairingDeleted(self, error):
        controllerLogProgress("OnPairingDeleted Error: {}".format(error))



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
    def __init__(self, controller_node_id: int=0, bluetoothAdapter: int=0):
        self.commissioner_init_params = None
        self.dev_com = None
        self.rendezvous_parms = RendezvousParameters()

        self.controller_node_id = controller_node_id
        ChipExceptions.CHIPErrorToException(Platform.MemoryInit())
        self.platform_manager = DeviceLayer.PlatformMgr()

        # This has to go before InitChipStack or else there will be GLib critical errors.
        if IS_LINUX:
            DeviceLayer.Internal.BLEMgrImpl().ConfigureBle(bluetoothAdapter, True)  # Central is set to True
        
        controllerLogProgress("Initializing Chip Stack")
        chip_stack_init_result = self.platform_manager.InitChipStack()
        controllerLogDetail("Chip Stack Init Result: {}".format(chip_stack_init_result))
        ChipExceptions.CHIPErrorToException(chip_stack_init_result)
        self.__init_delegates()

        self._Cluster = ChipClusters()
        self.__init_dev_com()

    def __del__(self):
        if self.dev_com != None:
            self.dev_com.Shutdown()
            self.dev_com = None

    def __init_delegates(self):
        # Need to assign to class variables, because they need to be persistent.
        self.storage_delegate = PyPersistentStorageDelegate()
        self.device_pairing_delegate = PyDevicePairingDelegate()
        self.device_address_delegate = PyDeviceAddressUpdateDelegate()
        self.interaction_delegate = Controller.DeviceControllerInteractionModelDelegate()
        self.commissioner_init_params = Controller.CommissionerInitParams()

    def __init_dev_com(self):
        crendential_issuer_delegate = Controller.ExampleOperationalCredentialsIssuer()
        credential_init_error = crendential_issuer_delegate.Initialize(self.storage_delegate)
        controllerLogDetail("Credential Init Error : {}".format(credential_init_error))
        ChipExceptions.CHIPErrorToException(credential_init_error)

        self.commissioner_init_params.storageDelegate = self.storage_delegate
        self.commissioner_init_params.mDeviceAddressUpdateDelegate = self.device_address_delegate
        self.commissioner_init_params.pairingDelegate = self.device_pairing_delegate
        self.commissioner_init_params.operationalCredentialsDelegate = crendential_issuer_delegate
        self.commissioner_init_params.imDelegate = self.interaction_delegate

        self.dev_com = Controller.DeviceCommissioner()
        init_error = self.dev_com.Init(self.controller_node_id, self.commissioner_init_params)
        controllerLogDetail("Device Commissioner Init Error : {}".format(init_error))
        ChipExceptions.CHIPErrorToException(init_error)
        ChipExceptions.CHIPErrorToException(self.dev_com.ServiceEvents())

    def IsConnected(self):
        # TODO: Did not see implementation. Need to ass one
        return None

    def ConnectBle(self, bleConnection):
        # TODO: Did not see implementation. Need to ass one
        return None

    def ConnectBLE(self, discriminator, setupPinCode, nodeid):
        GLOBAL_EVENT.clear()
        result = self.dev_com.PairDevice(nodeid,
            RendezvousParameters().SetDiscriminator(discriminator).SetPeerAddress(Transport.PeerAddress.BLE()).SetSetupPINCode(setupPinCode))
        GLOBAL_EVENT.wait(timeout=65)
        return result

    def CloseBLEConnection(self):
        controllerLogDetail("Closing BLE connection.")
        return self.dev_com.CloseBleConnection()

    def ConnectIP(self, ipaddr, setupPinCode, nodeid):
        GLOBAL_EVENT.clear()
        peer_address = Transport.PeerAddress()
        ip_address = Inet.IPAddress()
        Inet.IPAddress.FromString(ipaddr, ip_address)
        rendezvous_params = RendezvousParameters().SetSetupPINCode(setupPinCode)
        peer_address.SetTransportType(Transport.Type.kTcp).SetIPAddress(ip_address)
        rendezvous_params.SetPeerAddress(peer_address).SetDiscriminator(0)
        result = self.dev_com.PairDevice(nodeid, rendezvous_params)
        GLOBAL_EVENT.wait(timeout=65)
        return result

    def ResolveNode(self, fabricid, nodeid):
        return Mdns.Resolver.Instance().ResolveNodeId(PeerId().SetNodeId(nodeid).SetFabricId(fabricid), Inet.kIPAddressType_Any)

    def GetAddressAndPort(self, nodeid):
        address = create_string_buffer(64)
        port = c_uint16(0)

        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetAddressAndPort(
                self.dev_com, nodeid, address, 64, pointer(port))
        )

        return (address.value.decode(), port.value) if error == 0 else None
    def DiscoverCommissioningLongDiscriminator(self, long_discriminator):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissioningLongDiscriminator(self.dev_com, long_discriminator)
        )

    def PrintDiscoveredDevices(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_PrintDiscoveredDevices(self.dev_com)
        )
    def PrintSetupPayload(setup_payload):
        controllerLogDetail("Version: {}".format(setup_payload.version))
        controllerLogDetail("Vendor ID: {}".format(setup_payload.vendorID))
        controllerLogDetail("Product ID: {}".format(setup_payload.productID))
        controllerLogDetail("Commissioning Flow: {}".format(setup_payload.commissioningFlow))
        controllerLogDetail("Rendezvous Information Flag: {}".format(setup_payload.rendezvousInformation.Raw()))
        controllerLogDetail("Discriminator: {}".format(setup_payload.discriminator))
        controllerLogDetail("Setup Pin Code: {}".format(setup_payload.setUpPINCode))
        controllerLogDetail("All Vendor Options: {}".format(setup_payload.getAllOptionalVendorData()))

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
            lambda: self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice(self.dev_com, idx, addrStr, length)
        )

    def DiscoverAllCommissioning(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverAllCommissioning(self.dev_com)
        )

    def GetFabricId(self):
        fabricid = c_uint64(0)

        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetFabricId(
                self.dev_com, pointer(fabricid))
        )

        if error == 0:
            return fabricid.value
        else:
            return 0

    def IMSend(self, cluster, command, nodeid, endpoint, groupid, args, blocking=False):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.dev_com, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)
        im.ClearCommandStatus(im.PLACEHOLDER_COMMAND_HANDLE)
        self._Cluster.SendCommand(
            device, cluster, command, endpoint, groupid, args, True)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(im.PLACEHOLDER_COMMAND_HANDLE, 1)
        return (0, None)

    def IMReadAttribute(self, cluster, attribute, nodeid, endpoint, groupid, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.dev_com, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        # We are not using IM for Attributes.
        res = self._Cluster.ReadAttribute(
            device, cluster, attribute, endpoint, groupid, False)

    def IMWriteAttribute(self, cluster, attribute, nodeid, endpoint, groupid, value, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.dev_com, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        # We are not using IM for Attributes.
        res = self._Cluster.WriteAttribute(
            device, cluster, attribute, endpoint, groupid, value, False)

    def IMConfigureAttribute(self, cluster, attribute, nodeid, endpoint, minInterval, maxInterval, change, blocking=True):
        device = c_void_p(None)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceByNodeId(
            self.dev_com, nodeid, pointer(device)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        commandSenderHandle = self._dmLib.pychip_GetCommandSenderHandle(device)
        im.ClearCommandStatus(commandSenderHandle)
        res = self._Cluster.ConfigureAttribute(
            device, cluster, attribute, endpoint, minInterval, maxInterval, change, commandSenderHandle != 0)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(commandSenderHandle, 1)

    def IMCommandList(self):
        return self._Cluster.ListClusterCommands()

    def IMAttributeList(self):
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
