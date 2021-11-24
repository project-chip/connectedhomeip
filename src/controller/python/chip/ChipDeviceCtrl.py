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
import asyncio
from ctypes import *
from .ChipStack import *
from .interaction_model import InteractionModelError, delegate as im
from .exceptions import *
from .clusters import Command as ClusterCommand
from .clusters import Attribute as ClusterAttribute
from .clusters import ClusterObjects as ClusterObjects
from .clusters import Objects as GeneratedObjects
from .clusters.CHIPClusters import *
import enum
import threading
import typing


__all__ = ["ChipDeviceController"]

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, c_uint32)
_DevicePairingDelegate_OnCommissioningCompleteFunct = CFUNCTYPE(
    None, c_uint64, c_uint32)
_DeviceAddressUpdateDelegate_OnUpdateComplete = CFUNCTYPE(
    None, c_uint64, c_uint32)
# void (*)(Device *, CHIP_ERROR).
#
# CHIP_ERROR is actually signed, so using c_uint32 is weird, but everything
# else seems to do it.
_DeviceAvailableFunct = CFUNCTYPE(None, c_void_p, c_uint32)


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
                # Failed update address, don't wait for HandleCommissioningComplete
                self.state = DCState.IDLE
                self._ChipStack.callbackRes = err
                self._ChipStack.completeEvent.set()
            else:
                print("Node address has been updated")
                # Wait for HandleCommissioningComplete before setting
                # self._ChipStack.callbackRes; we're not done until that happens.

        def HandleCommissioningComplete(nodeid, err):
            if err != 0:
                print("Failed to commission: {}".format(err))
            else:
                print("Commissioning complete")
            self.state = DCState.IDLE
            self._ChipStack.callbackRes = err
            self._ChipStack.completeEvent.set()
            self._ChipStack.commissioningCompleteEvent.set()
            self._ChipStack.commissioningEventRes = err

        im.InitIMDelegate()
        ClusterCommand.Init(self)
        ClusterAttribute.Init()

        self.cbHandleKeyExchangeCompleteFunct = _DevicePairingDelegate_OnPairingCompleteFunct(
            HandleKeyExchangeComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(
            self.devCtrl, self.cbHandleKeyExchangeCompleteFunct)

        self.cbHandleCommissioningCompleteFunct = _DevicePairingDelegate_OnCommissioningCompleteFunct(
            HandleCommissioningComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
            self.devCtrl, self.cbHandleCommissioningCompleteFunct)

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
            lambda: self._dmLib.pychip_DeviceCommissioner_CloseBleConnection(
                self.devCtrl)
        )

    def CloseSession(self, nodeid):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_CloseSession(
                self.devCtrl, nodeid)
        )

    def EstablishPASESessionIP(self, ipaddr, setupPinCode, nodeid):
        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_EstablishPASESessionIP(
                self.devCtrl, ipaddr, setupPinCode, nodeid)
        )

    def Commission(self, nodeid):
        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_Commission(
                self.devCtrl, nodeid)
        )
        # Wait up to 5 additional seconds for the commissioning complete event
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            self._ChipStack.commissioningCompleteEvent.wait(5.0)
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

    def ConnectIP(self, ipaddr, setupPinCode, nodeid):
        # IP connection will run through full commissioning, so we need to wait
        # for the commissioning complete event, not just any callback.
        self.state = DCState.RENDEZVOUS_ONGOING
        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectIP(
                self.devCtrl, ipaddr, setupPinCode, nodeid)
        )
        # Wait up to 5 additional seconds for the commissioning complete event
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            self._ChipStack.commissioningCompleteEvent.wait(5.0)
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

    def ResolveNode(self, nodeid):
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_UpdateDevice(
                self.devCtrl, nodeid)
        )

    def GetAddressAndPort(self, nodeid):
        address = create_string_buffer(64)
        port = c_uint16(0)

        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetAddressAndPort(
                self.devCtrl, nodeid, address, 64, pointer(port))
        )

        return (address.value.decode(), port.value) if error == 0 else None

    def DiscoverCommissionableNodesLongDiscriminator(self, long_discriminator):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(
                self.devCtrl, long_discriminator)
        )

    def DiscoverCommissionableNodesShortDiscriminator(self, short_discriminator):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(
                self.devCtrl, short_discriminator)
        )

    def DiscoverCommissionableNodesVendor(self, vendor):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor(
                self.devCtrl, vendor)
        )

    def DiscoverCommissionableNodesDeviceType(self, device_type):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType(
                self.devCtrl, device_type)
        )

    def DiscoverCommissionableNodesCommissioningEnabled(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(
                self.devCtrl)
        )

    def PrintDiscoveredDevices(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_PrintDiscoveredDevices(
                self.devCtrl)
        )

    def ParseQRCode(self, qrCode, output):
        print(output)
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_ParseQRCode(
                qrCode, output)
        )

    def GetIPForDiscoveredDevice(self, idx, addrStr, length):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice(
                self.devCtrl, idx, addrStr, length)
        )

    def DiscoverAllCommissioning(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes(
                self.devCtrl)
        )

    def OpenCommissioningWindow(self, nodeid, timeout, iteration, discriminator, option):
        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_OpenCommissioningWindow(
                self.devCtrl, nodeid, timeout, iteration, discriminator, option)
        )

        if res != 0:
            raise self._ChipStack.ErrorToException(res)

    def GetCompressedFabricId(self):
        fabricid = c_uint64(0)

        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetCompressedFabricId(
                self.devCtrl, pointer(fabricid))
        )

        if res == 0:
            return fabricid.value
        else:
            raise self._ChipStack.ErrorToException(res)

    def GetFabricId(self):
        fabricid = c_uint64(0)

        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetFabricId(
                self.devCtrl, pointer(fabricid))
        )

        if res == 0:
            return fabricid.value
        else:
            raise self._ChipStack.ErrorToException(res)

    def GetClusterHandler(self):
        return self._Cluster

    def GetConnectedDeviceSync(self, nodeid):
        returnDevice = c_void_p(None)
        deviceAvailableCV = threading.Condition()

        def DeviceAvailableCallback(device, err):
            nonlocal returnDevice
            nonlocal deviceAvailableCV
            with deviceAvailableCV:
                returnDevice = c_void_p(device)
                deviceAvailableCV.notify_all()
            if err != 0:
                print("Failed in getting the connected device: {}".format(err))
                raise self._ChipStack.ErrorToException(err)

        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetConnectedDeviceByNodeId(
            self.devCtrl, nodeid, _DeviceAvailableFunct(DeviceAvailableCallback)))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        # The callback might have been received synchronously (during self._ChipStack.Call()).
        # Check if the device is already set before waiting for the callback.
        if returnDevice.value == None:
            with deviceAvailableCV:
                deviceAvailableCV.wait()

        if returnDevice.value == None:
            raise self._ChipStack.ErrorToException(CHIP_ERROR_INTERNAL)
        return returnDevice

    async def SendCommand(self, nodeid: int, endpoint: int, payload: ClusterObjects.ClusterCommand, responseType=None):
        '''
        Send a cluster-object encapsulated command to a node and get returned a future that can be awaited upon to receive the response.
        If a valid responseType is passed in, that will be used to deserialize the object. If not, the type will be automatically deduced
        from the metadata received over the wire.
        '''

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid)
        res = self._ChipStack.Call(
            lambda: ClusterCommand.SendCommand(
                future, eventLoop, responseType, device, ClusterCommand.CommandPath(
                    EndpointId=endpoint,
                    ClusterId=payload.cluster_id,
                    CommandId=payload.command_id,
                ), payload)
        )
        if res != 0:
            future.set_exception(self._ChipStack.ErrorToException(res))
        return await future

    async def WriteAttribute(self, nodeid: int, attributes):
        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid)
        res = self._ChipStack.Call(
            lambda: ClusterAttribute.WriteAttributes(
                future, eventLoop, device, attributes)
        )
        if res != 0:
            raise self._ChipStack.ErrorToException(res)
        return await future

    async def ReadAttribute(self, nodeid: int, attributes: typing.List[typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[int],  # Endpoint
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster]],
        # Wildcard endpoint, Cluster + Attribute present
        typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Wildcard attribute id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster]],
        # Concrete path
        typing.Tuple[int, typing.Type[ClusterObjects.ClusterAttributeDescriptor]]
    ]]):
        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid)
        attrs = []
        for v in attributes:
            endpoint = None
            cluster = None
            attribute = None
            if v == () or v == ('*'):
                # Wildcard
                pass
            elif len(v) == 1:
                if v[0] is int:
                    endpoint = v[0]
                elif issubclass(v[0], ClusterObjects.Cluster):
                    cluster = v[0]
                elif issubclass(v[0], ClusterObjects.ClusterAttributeDescriptor):
                    attribute = v[0]
                else:
                    raise ValueError("Unsupported Attribute Path")
            elif len(v) == 2:
                # endpoint + (cluster) attribute / endpoint + cluster
                endpoint = v[0]
                if issubclass(v[1], ClusterObjects.Cluster):
                    cluster = v[1]
                elif issubclass(v[1], ClusterAttribute.ClusterAttributeDescriptor):
                    attribute = v[1]
                else:
                    raise ValueError("Unsupported Attribute Path")
            attrs.append(ClusterAttribute.AttributePath(
                EndpointId=endpoint, Cluster=cluster, Attribute=attribute))
        res = self._ChipStack.Call(
            lambda: ClusterAttribute.ReadAttributes(future, eventLoop, device, attrs))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)
        return await future

    def ZCLSend(self, cluster, command, nodeid, endpoint, groupid, args, blocking=False):
        req = None
        try:
            req = eval(
                f"GeneratedObjects.{cluster}.Commands.{command}")(**args)
        except:
            raise UnknownCommand(cluster, command)
        try:
            res = asyncio.run(self.SendCommand(nodeid, endpoint, req))
            print(f"CommandResponse {res}")
            return (0, res)
        except InteractionModelError as ex:
            return (int(ex.state), None)

    def ZCLReadAttribute(self, cluster, attribute, nodeid, endpoint, groupid, blocking=True):
        device = self.GetConnectedDeviceSync(nodeid)

        # We are not using IM for Attributes.
        self._Cluster.ReadAttribute(
            device, cluster, attribute, endpoint, groupid, False)
        if blocking:
            return im.GetAttributeReadResponse(im.DEFAULT_ATTRIBUTEREAD_APPID)

    def ZCLWriteAttribute(self, cluster, attribute, nodeid, endpoint, groupid, value, blocking=True):
        req = None
        try:
            req = ClusterAttribute.AttributeWriteRequest(EndpointId=endpoint, Attribute=eval(
                f"GeneratedObjects.{cluster}.Attributes.{attribute}"), Data=value)
        except:
            raise UnknownAttribute(cluster, attribute)

        return asyncio.run(self.WriteAttribute(nodeid, [req]))

    def ZCLSubscribeAttribute(self, cluster, attribute, nodeid, endpoint, minInterval, maxInterval, blocking=True):
        device = self.GetConnectedDeviceSync(nodeid)

        commandSenderHandle = self._dmLib.pychip_GetCommandSenderHandle(device)
        im.ClearCommandStatus(commandSenderHandle)
        self._Cluster.SubscribeAttribute(
            device, cluster, attribute, endpoint, minInterval, maxInterval, commandSenderHandle != 0)
        if blocking:
            # We only send 1 command by this function, so index is always 0
            return im.WaitCommandIndexStatus(commandSenderHandle, 1)

    def ZCLShutdownSubscription(self, subscriptionId: int):
        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_InteractionModel_ShutdownSubscription(subscriptionId))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

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

            self._dmLib.pychip_DeviceController_Commission.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_Commission.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled.restype = c_uint32

            self._dmLib.pychip_DeviceController_EstablishPASESessionIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_EstablishPASESessionIP.restype = c_uint32

            self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_PrintDiscoveredDevices.argtypes = [
                c_void_p]

            self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice.argtypes = [
                c_void_p, c_int, c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice.restype = c_bool

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectIP.restype = c_uint32

            self._dmLib.pychip_DeviceController_CloseSession.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_CloseSession.restype = c_uint32

            self._dmLib.pychip_DeviceController_GetAddressAndPort.argtypes = [
                c_void_p, c_uint64, c_char_p, c_uint64, POINTER(c_uint16)]
            self._dmLib.pychip_DeviceController_GetAddressAndPort.restype = c_uint32

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnPairingCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.restype = c_uint32

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnCommissioningCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.restype = c_uint32

            self._dmLib.pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete.argtypes = [
                _DeviceAddressUpdateDelegate_OnUpdateComplete]
            self._dmLib.pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete.restype = None

            self._dmLib.pychip_DeviceController_UpdateDevice.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_UpdateDevice.restype = c_uint32

            self._dmLib.pychip_GetConnectedDeviceByNodeId.argtypes = [
                c_void_p, c_uint64, _DeviceAvailableFunct]
            self._dmLib.pychip_GetConnectedDeviceByNodeId.restype = c_uint32

            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.restype = c_uint32

            self._dmLib.pychip_GetCommandSenderHandle.argtypes = [c_void_p]
            self._dmLib.pychip_GetCommandSenderHandle.restype = c_uint64

            self._dmLib.pychip_DeviceController_GetCompressedFabricId.argtypes = [
                c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetCompressedFabricId.restype = c_uint32

            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.argtypes = [
                c_void_p, c_uint64, c_uint16, c_uint16, c_uint16, c_uint8]
            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.restype = c_uint32

            self._dmLib.pychip_InteractionModel_ShutdownSubscription.argtypes = [
                c_uint64]
            self._dmLib.pychip_InteractionModel_ShutdownSubscription.restype = c_uint32
