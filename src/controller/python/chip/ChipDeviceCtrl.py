#
#    Copyright (c) 2020-2022 Project CHIP Authors
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
from dataclasses import dataclass

from .ChipStack import *
from .interaction_model import InteractionModelError, delegate as im
from .exceptions import *
from .clusters import Command as ClusterCommand
from .clusters import Attribute as ClusterAttribute
from .clusters import ClusterObjects as ClusterObjects
from .clusters import Objects as GeneratedObjects
from .clusters.CHIPClusters import *
from . import clusters as Clusters
import enum
import threading
import typing
import builtins
import ipdb
import ctypes
import copy

__all__ = ["ChipDeviceController"]

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, c_uint32)
_DevicePairingDelegate_OnCommissioningCompleteFunct = CFUNCTYPE(
    None, c_uint64, c_uint32)
_DevicePairingDelegate_OnCommissioningStatusUpdateFunct = CFUNCTYPE(
    None, c_uint64, c_uint8, c_uint32)
# void (*)(Device *, CHIP_ERROR).
#
# CHIP_ERROR is actually signed, so using c_uint32 is weird, but everything
# else seems to do it.
_DeviceAvailableFunct = CFUNCTYPE(None, c_void_p, c_uint32)

_IssueNOCChainCallbackPythonCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32, c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_size_t, c_uint64)


@dataclass
class NOCChain:
    nocBytes: bytes
    icacBytes: bytes
    rcacBytes: bytes
    ipkBytes: bytes
    adminSubject: int


@_IssueNOCChainCallbackPythonCallbackFunct
def _IssueNOCChainCallbackPythonCallback(devCtrl, status: int, noc: c_void_p, nocLen: int, icac: c_void_p, icacLen: int, rcac: c_void_p, rcacLen: int, ipk: c_void_p, ipkLen: int, adminSubject: int):
    nocChain = NOCChain(None, None, None, None, 0)
    if status == 0:
        nocBytes = None
        if nocLen > 0:
            nocBytes = string_at(noc, nocLen)[:]
        icacBytes = None
        if icacLen > 0:
            icacBytes = string_at(icac, icacLen)[:]
        rcacBytes = None
        if rcacLen > 0:
            rcacBytes = string_at(rcac, rcacLen)[:]
        ipkBytes = None
        if ipkLen > 0:
            ipkBytes = string_at(ipk, ipkLen)[:]
        nocChain = NOCChain(nocBytes, icacBytes, rcacBytes, ipkBytes, adminSubject)
    devCtrl.NOCChainCallback(nocChain)

# This is a fix for WEAV-429. Jay Logue recommends revisiting this at a later
# date to allow for truly multiple instances so this is temporary.


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
    COMMISSIONING = 5


class ChipDeviceController():
    activeList = set()

    def __init__(self, opCredsContext: ctypes.c_void_p, fabricId: int, fabricIndex: int, nodeId: int, adminVendorId: int, paaTrustStorePath: str = "", useTestCommissioner: bool = False):
        self.state = DCState.NOT_INITIALIZED
        self.devCtrl = None
        self._ChipStack = builtins.chipStack
        self._dmLib = None

        self._InitLib()

        self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback(_IssueNOCChainCallbackPythonCallback)

        devCtrl = c_void_p(None)

        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_OpCreds_AllocateController(ctypes.c_void_p(
                opCredsContext), pointer(devCtrl), fabricIndex, fabricId, nodeId, adminVendorId, ctypes.c_char_p(None if len(paaTrustStorePath) == 0 else str.encode(paaTrustStorePath)), useTestCommissioner)
        )

        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        self.devCtrl = devCtrl

        self._Cluster = ChipClusters(builtins.chipStack)
        self._Cluster.InitLib(self._dmLib)

        def HandleKeyExchangeComplete(err):
            if err != 0:
                print("Failed to establish secure session to device: {}".format(err))
                self._ChipStack.callbackRes = self._ChipStack.ErrorToException(
                    err)
            else:
                print("Established CASE with Device")
            if self.state != DCState.COMMISSIONING:
                # During Commissioning, HandleKeyExchangeComplete will also be called,
                # in this case the async operation should be marked as finished by
                # HandleCommissioningComplete instead this function.
                self.state = DCState.IDLE
                self._ChipStack.completeEvent.set()

        def HandleCommissioningComplete(nodeid, err):
            if err != 0:
                print("Failed to commission: {}".format(err))
            else:
                print("Commissioning complete")
            self.state = DCState.IDLE
            self._ChipStack.callbackRes = err
            self._ChipStack.commissioningEventRes = err
            self._ChipStack.commissioningCompleteEvent.set()
            self._ChipStack.completeEvent.set()

        self.cbHandleKeyExchangeCompleteFunct = _DevicePairingDelegate_OnPairingCompleteFunct(
            HandleKeyExchangeComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(
            self.devCtrl, self.cbHandleKeyExchangeCompleteFunct)

        self.cbHandleCommissioningCompleteFunct = _DevicePairingDelegate_OnCommissioningCompleteFunct(
            HandleCommissioningComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
            self.devCtrl, self.cbHandleCommissioningCompleteFunct)

        self.state = DCState.IDLE
        self.isActive = True

        ChipDeviceController.activeList.add(self)

    def Shutdown(self):
        ''' Shuts down this controller and reclaims any used resources, including the bound
            C++ constructor instance in the SDK.
        '''
        if (self.isActive):
            if self.devCtrl != None:
                self._ChipStack.Call(
                    lambda: self._dmLib.pychip_DeviceController_DeleteDeviceController(
                        self.devCtrl)
                )
                self.devCtrl = None

            ChipDeviceController.activeList.remove(self)
            self.isActive = False

    def ShutdownAll():
        ''' Shut down all active controllers and reclaim any used resources.
        '''
        #
        # We want a shallow copy here since it would other create new instances
        # of the controllers in the list.
        #
        # We need a copy since we're going to walk through the list and shutdown
        # each controller, which in turn, will remove themselves from the active list.
        #
        # We cannot do that while iterating through the original list.
        #
        activeList = copy.copy(ChipDeviceController.activeList)

        for controller in activeList:
            controller.Shutdown()

        ChipDeviceController.activeList.clear()

    def CheckIsActive(self):
        if (not self.isActive):
            raise RuntimeError(
                "DeviceCtrl instance was already shutdown previously!")

    def __del__(self):
        self.Shutdown()

    def IsConnected(self):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_IsConnected(
                self.devCtrl)
        )

    def ConnectBle(self, bleConnection):
        self.CheckIsActive()

        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ValidateBTP(
                self.devCtrl,
                bleConnection,
                self._ChipStack.cbHandleComplete,
                self._ChipStack.cbHandleError,
            )
        )

    def ConnectBLE(self, discriminator, setupPinCode, nodeid):
        self.CheckIsActive()

        self._ChipStack.commissioningCompleteEvent.clear()

        self.state = DCState.COMMISSIONING
        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectBLE(
                self.devCtrl, discriminator, setupPinCode, nodeid)
        )
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

    def CloseBLEConnection(self):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceCommissioner_CloseBleConnection(
                self.devCtrl)
        )

    def CloseSession(self, nodeid):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_CloseSession(
                self.devCtrl, nodeid)
        )

    def EstablishPASESessionIP(self, ipaddr: str, setupPinCode: int, nodeid: int):
        self.CheckIsActive()

        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_EstablishPASESessionIP(
                self.devCtrl, ipaddr.encode("utf-8"), setupPinCode, nodeid)
        )

    def Commission(self, nodeid):
        self.CheckIsActive()
        self._ChipStack.commissioningCompleteEvent.clear()
        self.state = DCState.COMMISSIONING

        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_Commission(
                self.devCtrl, nodeid)
        )
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

    def GetTestCommissionerUsed(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_TestCommissionerUsed()
        )

    def ResetTestCommissioner(self):
        self._dmLib.pychip_ResetCommissioningTests()

    def SetTestCommissionerSimulateFailureOnStage(self, stage: int):
        return self._dmLib.pychip_SetTestCommissionerSimulateFailureOnStage(
            stage)

    def SetTestCommissionerSimulateFailureOnReport(self, stage: int):
        return self._dmLib.pychip_SetTestCommissionerSimulateFailureOnReport(
            stage)

    def CheckTestCommissionerCallbacks(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_TestCommissioningCallbacks()
        )

    def CheckTestCommissionerPaseConnection(self, nodeid):
        return self._dmLib.pychip_TestPaseConnection(nodeid)

    def CommissionWithCode(self, setupPayload: str, nodeid: int):
        self.CheckIsActive()

        setupPayload = setupPayload.encode() + b'\0'

        # IP connection will run through full commissioning, so we need to wait
        # for the commissioning complete event, not just any callback.
        self.state = DCState.COMMISSIONING

        self._ChipStack.commissioningCompleteEvent.clear()

        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectWithCode(
                self.devCtrl, setupPayload, nodeid)
        )
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

    def CommissionIP(self, ipaddr: str, setupPinCode: int, nodeid: int):
        self.CheckIsActive()

        # IP connection will run through full commissioning, so we need to wait
        # for the commissioning complete event, not just any callback.
        self.state = DCState.COMMISSIONING

        self._ChipStack.commissioningCompleteEvent.clear()

        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectIP(
                self.devCtrl, ipaddr.encode("utf-8"), setupPinCode, nodeid)
        )
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

    def NOCChainCallback(self, nocChain):
        self._ChipStack.callbackRes = nocChain
        self._ChipStack.completeEvent.set()
        return

    def CommissionThread(self, discriminator, setupPinCode, nodeId, threadOperationalDataset: bytes):
        ''' Commissions a Thread device over BLE
        '''
        self.SetThreadOperationalDataset(threadOperationalDataset)
        return self.ConnectBLE(discriminator, setupPinCode, nodeId)

    def CommissionWiFi(self, discriminator, setupPinCode, nodeId, ssid: str, credentials: str):
        ''' Commissions a WiFi device over BLE
        '''
        self.SetWiFiCredentials(ssid, credentials)
        return self.ConnectBLE(discriminator, setupPinCode, nodeId)

    def SetWiFiCredentials(self, ssid: str, credentials: str):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetWiFiCredentials(
                ssid.encode("utf-8"), credentials.encode("utf-8"))
        )

    def SetThreadOperationalDataset(self, threadOperationalDataset):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetThreadOperationalDataset(
                threadOperationalDataset, len(threadOperationalDataset))
        )

    def ResolveNode(self, nodeid):
        self.CheckIsActive()

        self.GetConnectedDeviceSync(nodeid, allowPASE=False)

    def GetAddressAndPort(self, nodeid):
        self.CheckIsActive()

        address = create_string_buffer(64)
        port = c_uint16(0)

        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetAddressAndPort(
                self.devCtrl, nodeid, address, 64, pointer(port))
        )

        return (address.value.decode(), port.value) if error == 0 else None

    def DiscoverCommissionableNodesLongDiscriminator(self, long_discriminator):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(
                self.devCtrl, long_discriminator)
        )

    def DiscoverCommissionableNodesShortDiscriminator(self, short_discriminator):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(
                self.devCtrl, short_discriminator)
        )

    def DiscoverCommissionableNodesVendor(self, vendor):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor(
                self.devCtrl, vendor)
        )

    def DiscoverCommissionableNodesDeviceType(self, device_type):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType(
                self.devCtrl, device_type)
        )

    def DiscoverCommissionableNodesCommissioningEnabled(self):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(
                self.devCtrl)
        )

    def PrintDiscoveredDevices(self):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_PrintDiscoveredDevices(
                self.devCtrl)
        )

    def ParseQRCode(self, qrCode, output):
        self.CheckIsActive()

        print(output)
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_ParseQRCode(
                qrCode, output)
        )

    def GetIPForDiscoveredDevice(self, idx, addrStr, length):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice(
                self.devCtrl, idx, addrStr, length)
        )

    def DiscoverAllCommissioning(self):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes(
                self.devCtrl)
        )

    def OpenCommissioningWindow(self, nodeid, timeout, iteration, discriminator, option):
        self.CheckIsActive()

        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_OpenCommissioningWindow(
                self.devCtrl, nodeid, timeout, iteration, discriminator, option)
        )

        if res != 0:
            raise self._ChipStack.ErrorToException(res)

    def GetCompressedFabricId(self):
        self.CheckIsActive()

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
        self.CheckIsActive()

        fabricid = c_uint64(0)

        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetFabricId(
                self.devCtrl, pointer(fabricid))
        )

        if res == 0:
            return fabricid.value
        else:
            raise self._ChipStack.ErrorToException(res)

    def GetNodeId(self):
        self.CheckIsActive()

        nodeid = c_uint64(0)

        res = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetNodeId(
                self.devCtrl, pointer(nodeid))
        )

        if res == 0:
            return nodeid.value
        else:
            raise self._ChipStack.ErrorToException(res)

    def GetClusterHandler(self):
        self.CheckIsActive()

        return self._Cluster

    def GetConnectedDeviceSync(self, nodeid, allowPASE=True):
        self.CheckIsActive()

        returnDevice = c_void_p(None)
        returnErr = None
        deviceAvailableCV = threading.Condition()

        @_DeviceAvailableFunct
        def DeviceAvailableCallback(device, err):
            nonlocal returnDevice
            nonlocal returnErr
            nonlocal deviceAvailableCV
            with deviceAvailableCV:
                returnDevice = c_void_p(device)
                returnErr = err
                deviceAvailableCV.notify_all()

        if allowPASE:
            res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceBeingCommissioned(
                self.devCtrl, nodeid, byref(returnDevice)))
            if res == 0:
                print('Using PASE connection')
                return returnDevice

        res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetConnectedDeviceByNodeId(
            self.devCtrl, nodeid, DeviceAvailableCallback))
        if res != 0:
            raise self._ChipStack.ErrorToException(res)

        # The callback might have been received synchronously (during self._ChipStack.Call()).
        # Check if the device is already set before waiting for the callback.
        if returnDevice.value is None:
            with deviceAvailableCV:
                deviceAvailableCV.wait()

        if returnDevice.value is None:
            raise self._ChipStack.ErrorToException(returnErr)
        return returnDevice

    async def SendCommand(self, nodeid: int, endpoint: int, payload: ClusterObjects.ClusterCommand, responseType=None, timedRequestTimeoutMs: int = None):
        '''
        Send a cluster-object encapsulated command to a node and get returned a future that can be awaited upon to receive the response.
        If a valid responseType is passed in, that will be used to deserialize the object. If not, the type will be automatically deduced
        from the metadata received over the wire.

        timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid)
        res = ClusterCommand.SendCommand(
            future, eventLoop, responseType, device, ClusterCommand.CommandPath(
                EndpointId=endpoint,
                ClusterId=payload.cluster_id,
                CommandId=payload.command_id,
            ), payload, timedRequestTimeoutMs=timedRequestTimeoutMs)
        if res != 0:
            future.set_exception(self._ChipStack.ErrorToException(res))
        return await future

    async def WriteAttribute(self, nodeid: int, attributes: typing.List[typing.Tuple[int, ClusterObjects.ClusterAttributeDescriptor, int]], timedRequestTimeoutMs: int = None):
        '''
        Write a list of attributes on a target node.

        nodeId: Target's Node ID
        timedWriteTimeoutMs: Timeout for a timed write request. Omit or set to 'None' to indicate a non-timed request.
        attributes: A list of tuples of type (endpoint, cluster-object):

        E.g
            (1, Clusters.TestCluster.Attributes.XYZAttribute('hello')) -- Write 'hello' to the XYZ attribute on the test cluster to endpoint 1
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid)

        attrs = []
        for v in attributes:
            if len(v) == 2:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    v[0], v[1], 0, 0, v[1].value))
            else:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    v[0], v[1], v[2], 1, v[1].value))

        res = ClusterAttribute.WriteAttributes(
            future, eventLoop, device, attrs, timedRequestTimeoutMs=timedRequestTimeoutMs)
        if res != 0:
            raise self._ChipStack.ErrorToException(res)
        return await future

    def _parseAttributePathTuple(self, pathTuple: typing.Union[
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
    ]):
        endpoint = None
        cluster = None
        attribute = None

        if pathTuple == ('*') or pathTuple == ():
            # Wildcard
            pass
        elif type(pathTuple) is not tuple:
            if type(pathTuple) is int:
                endpoint = pathTuple
            elif issubclass(pathTuple, ClusterObjects.Cluster):
                cluster = pathTuple
            elif issubclass(pathTuple, ClusterObjects.ClusterAttributeDescriptor):
                attribute = pathTuple
            else:
                raise ValueError("Unsupported Attribute Path")
        else:
            # endpoint + (cluster) attribute / endpoint + cluster
            endpoint = pathTuple[0]
            if issubclass(pathTuple[1], ClusterObjects.Cluster):
                cluster = pathTuple[1]
            elif issubclass(pathTuple[1], ClusterAttribute.ClusterAttributeDescriptor):
                attribute = pathTuple[1]
            else:
                raise ValueError("Unsupported Attribute Path")
        return ClusterAttribute.AttributePath(
            EndpointId=endpoint, Cluster=cluster, Attribute=attribute)

    def _parseDataVersionFilterTuple(self, pathTuple: typing.List[typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int]]):
        endpoint = None
        cluster = None

        # endpoint + (cluster) attribute / endpoint + cluster
        endpoint = pathTuple[0]
        if issubclass(pathTuple[1], ClusterObjects.Cluster):
            cluster = pathTuple[1]
        else:
            raise ValueError("Unsupported Cluster Path")
        dataVersion = pathTuple[2]
        return ClusterAttribute.DataVersionFilter(
            EndpointId=endpoint, Cluster=cluster, DataVersion=dataVersion)

    def _parseEventPathTuple(self, pathTuple: typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[str, int],  # all wildcard with urgency set
        typing.Tuple[int, int],  # Endpoint,
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster], int],
        # Wildcard endpoint, Cluster + Event present
        typing.Tuple[typing.Type[ClusterObjects.ClusterEvent], int],
        # Wildcard event id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int],
        # Concrete path
        typing.Tuple[int,
                     typing.Type[ClusterObjects.ClusterEvent], int]
    ]):
        endpoint = None
        cluster = None
        event = None
        urgent = False
        if pathTuple in [('*'), ()]:
            # Wildcard
            pass
        elif type(pathTuple) is not tuple:
            print(type(pathTuple))
            if type(pathTuple) is int:
                endpoint = pathTuple
            elif issubclass(pathTuple, ClusterObjects.Cluster):
                cluster = pathTuple
            elif issubclass(pathTuple, ClusterObjects.ClusterEvent):
                event = pathTuple
            else:
                raise ValueError("Unsupported Event Path")
        else:
            if pathTuple[0] == '*':
                urgent = pathTuple[-1]
                pass
            else:
                # endpoint + (cluster) event / endpoint + cluster
                endpoint = pathTuple[0]
                if issubclass(pathTuple[1], ClusterObjects.Cluster):
                    cluster = pathTuple[1]
                elif issubclass(pathTuple[1], ClusterAttribute.ClusterEvent):
                    event = pathTuple[1]
                else:
                    raise ValueError("Unsupported Attribute Path")
                urgent = pathTuple[-1]
        return ClusterAttribute.EventPath(
            EndpointId=endpoint, Cluster=cluster, Event=event, Urgent=urgent)

    async def Read(self, nodeid: int, attributes: typing.List[typing.Union[
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
    ]] = None, dataVersionFilters: typing.List[typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int]] = None, events: typing.List[typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[str, int],  # all wildcard with urgency set
        typing.Tuple[int, int],  # Endpoint,
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster], int],
        # Wildcard endpoint, Cluster + Event present
        typing.Tuple[typing.Type[ClusterObjects.ClusterEvent], int],
        # Wildcard event id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int],
        # Concrete path
        typing.Tuple[int,
                     typing.Type[ClusterObjects.ClusterEvent], int]
    ]] = None,
            returnClusterObject: bool = False, reportInterval: typing.Tuple[int, int] = None, fabricFiltered: bool = True, keepSubscriptions: bool = False):
        '''
        Read a list of attributes and/or events from a target node

        nodeId: Target's Node ID
        attributes: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.AttributeA):   Endpoint = specific,    Cluster = specific,   Attribute = specific
            (endpoint, Clusters.ClusterA):              Endpoint = specific,    Cluster = specific,   Attribute = *
            (Clusters.ClusterA.AttributeA):             Endpoint = *,           Cluster = specific,   Attribute = specific
            endpoint:                                   Endpoint = specific,    Cluster = *,          Attribute = *
            Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Attribute = *
            '*' or ():                                  Endpoint = *,           Cluster = *,          Attribute = *

            The cluster and attributes specified above are to be selected from the generated cluster objects.

            e.g.
                ReadAttribute(1, [ 1 ] ) -- case 4 above.
                ReadAttribute(1, [ Clusters.Basic ] ) -- case 5 above.
                ReadAttribute(1, [ (1, Clusters.Basic.Attributes.Location ] ) -- case 1 above.

        dataVersionFilters: A list of tuples of (endpoint, cluster, data version).

        events: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,    Cluster = specific,   Event = specific, Urgent = True/False
            (endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,    Cluster = specific,   Event = *, Urgent = True/False
            (Clusters.ClusterA.EventA, urgent):                 Endpoint = *,           Cluster = specific,   Event = specific, Urgent = True/False
            endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
            Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Event = *, Urgent = True/False
            '*' or ():                                  Endpoint = *,           Cluster = *,          Event = *, Urgent = True/False

        returnClusterObject: This returns the data as consolidated cluster objects, with all attributes for a cluster inside
                             a single cluster-wide cluster object.

        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid)
        attributePaths = [self._parseAttributePathTuple(
            v) for v in attributes] if attributes else None
        clusterDataVersionFilters = [self._parseDataVersionFilterTuple(
            v) for v in dataVersionFilters] if dataVersionFilters else None
        eventPaths = [self._parseEventPathTuple(
            v) for v in events] if events else None

        res = ClusterAttribute.Read(future=future, eventLoop=eventLoop, device=device, devCtrl=self, attributes=attributePaths, dataVersionFilters=clusterDataVersionFilters, events=eventPaths, returnClusterObject=returnClusterObject,
                                    subscriptionParameters=ClusterAttribute.SubscriptionParameters(reportInterval[0], reportInterval[1]) if reportInterval else None, fabricFiltered=fabricFiltered, keepSubscriptions=keepSubscriptions)
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
    ]], dataVersionFilters: typing.List[typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int]] = None, returnClusterObject: bool = False, reportInterval: typing.Tuple[int, int] = None, fabricFiltered: bool = True, keepSubscriptions: bool = False):
        '''
        Read a list of attributes from a target node, this is a wrapper of DeviceController.Read()

        nodeId: Target's Node ID
        attributes: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.AttributeA):   Endpoint = specific,    Cluster = specific,   Attribute = specific
            (endpoint, Clusters.ClusterA):              Endpoint = specific,    Cluster = specific,   Attribute = *
            (Clusters.ClusterA.AttributeA):             Endpoint = *,           Cluster = specific,   Attribute = specific
            endpoint:                                   Endpoint = specific,    Cluster = *,          Attribute = *
            Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Attribute = *
            '*' or ():                                  Endpoint = *,           Cluster = *,          Attribute = *

            The cluster and attributes specified above are to be selected from the generated cluster objects.

            e.g.
                ReadAttribute(1, [ 1 ] ) -- case 4 above.
                ReadAttribute(1, [ Clusters.Basic ] ) -- case 5 above.
                ReadAttribute(1, [ (1, Clusters.Basic.Attributes.Location ] ) -- case 1 above.

        returnClusterObject: This returns the data as consolidated cluster objects, with all attributes for a cluster inside
                             a single cluster-wide cluster object.

        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        '''
        res = await self.Read(nodeid, attributes=attributes, dataVersionFilters=dataVersionFilters, returnClusterObject=returnClusterObject, reportInterval=reportInterval, fabricFiltered=fabricFiltered, keepSubscriptions=keepSubscriptions)
        if isinstance(res, ClusterAttribute.SubscriptionTransaction):
            return res
        else:
            return res.attributes

    async def ReadEvent(self, nodeid: int, events: typing.List[typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[str, int],  # all wildcard with urgency set
        typing.Tuple[int, int],  # Endpoint,
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster], int],
        # Wildcard endpoint, Cluster + Event present
        typing.Tuple[typing.Type[ClusterObjects.ClusterEvent], int],
        # Wildcard event id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int],
        # Concrete path
        typing.Tuple[int, typing.Type[ClusterObjects.ClusterEvent], int]
    ]], reportInterval: typing.Tuple[int, int] = None, keepSubscriptions: bool = False):
        '''
        Read a list of events from a target node, this is a wrapper of DeviceController.Read()

        nodeId: Target's Node ID
        events: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,    Cluster = specific,   Event = specific, Urgent = True/False
            (endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,    Cluster = specific,   Event = *, Urgent = True/False
            (Clusters.ClusterA.EventA, urgent):                 Endpoint = *,           Cluster = specific,   Event = specific, Urgent = True/False
            endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
            Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Event = *, Urgent = True/False
            '*' or ():                                  Endpoint = *,           Cluster = *,          Event = *, Urgent = True/False

        The cluster and events specified above are to be selected from the generated cluster objects.

        e.g.
            ReadEvent(1, [ 1 ] ) -- case 4 above.
            ReadEvent(1, [ Clusters.Basic ] ) -- case 5 above.
            ReadEvent(1, [ (1, Clusters.Basic.Events.Location ] ) -- case 1 above.

        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        '''
        res = await self.Read(nodeid=nodeid, events=events, reportInterval=reportInterval, keepSubscriptions=keepSubscriptions)
        if isinstance(res, ClusterAttribute.SubscriptionTransaction):
            return res
        else:
            return res.events

    def ZCLSend(self, cluster, command, nodeid, endpoint, groupid, args, blocking=False):
        self.CheckIsActive()

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
        self.CheckIsActive()

        req = None
        clusterType = eval(f"GeneratedObjects.{cluster}")

        try:
            attributeType = eval(
                f"GeneratedObjects.{cluster}.Attributes.{attribute}")
        except:
            raise UnknownAttribute(cluster, attribute)

        result = asyncio.run(self.ReadAttribute(
            nodeid, [(endpoint, attributeType)]))
        path = ClusterAttribute.AttributePath(
            EndpointId=endpoint, Attribute=attributeType)
        return im.AttributeReadResult(path=im.AttributePath(nodeId=nodeid, endpointId=path.EndpointId, clusterId=path.ClusterId, attributeId=path.AttributeId), status=0, value=result[endpoint][clusterType][attributeType])

    def ZCLWriteAttribute(self, cluster: str, attribute: str, nodeid, endpoint, groupid, value, dataVersion=0, blocking=True):
        req = None
        try:
            req = eval(
                f"GeneratedObjects.{cluster}.Attributes.{attribute}")(value)
        except:
            raise UnknownAttribute(cluster, attribute)

        return asyncio.run(self.WriteAttribute(nodeid, [(endpoint, req, dataVersion)]))

    def ZCLSubscribeAttribute(self, cluster, attribute, nodeid, endpoint, minInterval, maxInterval, blocking=True):
        self.CheckIsActive()

        req = None
        try:
            req = eval(f"GeneratedObjects.{cluster}.Attributes.{attribute}")
        except:
            raise UnknownAttribute(cluster, attribute)
        return asyncio.run(self.ReadAttribute(nodeid, [(endpoint, req)], None, False, reportInterval=(minInterval, maxInterval)))

    def ZCLCommandList(self):
        self.CheckIsActive()
        return self._Cluster.ListClusterCommands()

    def ZCLAttributeList(self):
        self.CheckIsActive()

        return self._Cluster.ListClusterAttributes()

    def SetLogFilter(self, category):
        self.CheckIsActive()

        if category < 0 or category > pow(2, 8):
            raise ValueError("category must be an unsigned 8-bit integer")

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetLogFilter(category)
        )

    def GetLogFilter(self):
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetLogFilter()
        )

    def SetBlockingCB(self, blockingCB):
        self.CheckIsActive()

        self._ChipStack.blockingCB = blockingCB

    def IssueNOCChain(self, csr: Clusters.OperationalCredentials.Commands.CSRResponse, nodeId: int):
        """Issue an NOC chain using the associated OperationalCredentialsDelegate.
        The NOC chain will be provided in TLV cert format."""
        self.CheckIsActive()

        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_IssueNOCChain(
                self.devCtrl, py_object(self), csr.NOCSRElements, len(csr.NOCSRElements), nodeId)
        )

    # ----- Private Members -----
    def _InitLib(self):
        if self._dmLib is None:
            self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

            self._dmLib.pychip_DeviceController_DeleteDeviceController.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_DeleteDeviceController.restype = c_uint32

            self._dmLib.pychip_DeviceController_ConnectBLE.argtypes = [
                c_void_p, c_uint16, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectBLE.restype = c_uint32

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]

            self._dmLib.pychip_DeviceController_SetThreadOperationalDataset.argtypes = [
                c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_SetThreadOperationalDataset.restype = c_uint32

            self._dmLib.pychip_DeviceController_SetWiFiCredentials.argtypes = [
                c_char_p, c_char_p]
            self._dmLib.pychip_DeviceController_SetWiFiCredentials.restype = c_uint32

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

            self._dmLib.pychip_DeviceController_ConnectWithCode.argtypes = [
                c_void_p, c_char_p, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectWithCode.restype = c_uint32

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

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnCommissioningStatusUpdateFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.restype = c_uint32

            self._dmLib.pychip_GetConnectedDeviceByNodeId.argtypes = [
                c_void_p, c_uint64, _DeviceAvailableFunct]
            self._dmLib.pychip_GetConnectedDeviceByNodeId.restype = c_uint32

            self._dmLib.pychip_GetDeviceBeingCommissioned.argtypes = [
                c_void_p, c_uint64, c_void_p]
            self._dmLib.pychip_GetDeviceBeingCommissioned.restype = c_uint32

            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.restype = c_uint32

            self._dmLib.pychip_GetCommandSenderHandle.argtypes = [c_void_p]
            self._dmLib.pychip_GetCommandSenderHandle.restype = c_uint64

            self._dmLib.pychip_DeviceController_GetCompressedFabricId.argtypes = [
                c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetCompressedFabricId.restype = c_uint32

            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.argtypes = [
                c_void_p, c_uint64, c_uint16, c_uint32, c_uint16, c_uint8]
            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.restype = c_uint32
            self._dmLib.pychip_TestCommissionerUsed.argtypes = []
            self._dmLib.pychip_TestCommissionerUsed.restype = c_bool

            self._dmLib.pychip_TestCommissioningCallbacks.argtypes = []
            self._dmLib.pychip_ResetCommissioningTests.argtypes = []
            self._dmLib.pychip_TestPaseConnection.argtypes = [c_uint64]

            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnStage.argtypes = [
                c_uint8]
            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnStage.restype = c_bool
            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnReport.argtypes = [
                c_uint8]
            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnReport.restype = c_bool

            self._dmLib.pychip_DeviceController_IssueNOCChain.argtypes = [
                c_void_p, py_object, c_char_p, c_size_t, c_uint64
            ]
            self._dmLib.pychip_DeviceController_IssueNOCChain.restype = c_uint32

            self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback.argtypes = [
                _IssueNOCChainCallbackPythonCallbackFunct]
            self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback.restype = None
