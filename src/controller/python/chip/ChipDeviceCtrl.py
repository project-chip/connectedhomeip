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
from .FabricAdmin import FabricAdmin
from . import discovery
from .native import PyChipError
import enum
import threading
import typing
import builtins
import ctypes
import copy
import json
import time
import dacite

__all__ = ["ChipDeviceController"]

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, PyChipError)
_DevicePairingDelegate_OnCommissioningCompleteFunct = CFUNCTYPE(
    None, c_uint64, PyChipError)
_DevicePairingDelegate_OnCommissioningStatusUpdateFunct = CFUNCTYPE(
    None, c_uint64, c_uint8, PyChipError)
# void (*)(Device *, CHIP_ERROR).
#
# CHIP_ERROR is actually signed, so using c_uint32 is weird, but everything
# else seems to do it.
_DeviceAvailableFunct = CFUNCTYPE(None, c_void_p, PyChipError)

_IssueNOCChainCallbackPythonCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError, c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_size_t, c_uint64)

_ChipDeviceController_IterateDiscoveredCommissionableNodesFunct = CFUNCTYPE(None, c_char_p, c_size_t)


@dataclass
class NOCChain:
    nocBytes: bytes
    icacBytes: bytes
    rcacBytes: bytes
    ipkBytes: bytes
    adminSubject: int


@_IssueNOCChainCallbackPythonCallbackFunct
def _IssueNOCChainCallbackPythonCallback(devCtrl, status: PyChipError, noc: c_void_p, nocLen: int, icac: c_void_p, icacLen: int, rcac: c_void_p, rcacLen: int, ipk: c_void_p, ipkLen: int, adminSubject: int):
    nocChain = NOCChain(None, None, None, None, 0)
    if status.is_success:
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


class CommissionableNode(discovery.CommissionableNode):
    def SetDeviceController(self, devCtrl: 'ChipDeviceController'):
        self._devCtrl = devCtrl

    def Commission(self, nodeId: int, setupPinCode: int):
        ''' Commission the device using the device controller discovered this device.

        nodeId: The nodeId commissioned to the device
        setupPinCode: The setup pin code of the device
        '''
        self._devCtrl.CommissionOnNetwork(
            nodeId, setupPinCode, filterType=discovery.FilterType.INSTANCE_NAME, filter=self.instanceName)

    def __rich_repr__(self):
        yield "(To Be Commissioned By)", self._devCtrl.name

        for k in self.__dataclass_fields__.keys():
            if k in self.__dict__:
                yield k, self.__dict__[k]


class DeviceProxyWrapper():
    ''' Encapsulates a pointer to OperationalDeviceProxy on the c++ side that needs to be
        freed when DeviceProxyWrapper goes out of scope. There is a potential issue where
        if this is copied around that a double free will occure, but how this is used today
        that is not an issue that needs to be accounted for and it will become very apparent
        if that happens.
    '''

    def __init__(self, deviceProxy: ctypes.c_void_p, dmLib=None):
        self._deviceProxy = deviceProxy
        self._dmLib = dmLib

    def __del__(self):
        if (self._dmLib is not None and hasattr(builtins, 'chipStack') and builtins.chipStack is not None):
            # This destructor is called from any threading context, including on the Matter threading context.
            # So, we cannot call chipStack.Call or chipStack.CallAsync which waits for the posted work to
            # actually be executed. Instead, we just post/schedule the work and move on.
            builtins.chipStack.PostTaskOnChipThread(lambda: self._dmLib.pychip_FreeOperationalDeviceProxy(self._deviceProxy))

    @property
    def deviceProxy(self) -> ctypes.c_void_p:
        return self._deviceProxy

    @property
    def localSessionId(self) -> int:
        self._dmLib.pychip_GetLocalSessionId.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_uint16)]
        self._dmLib.pychip_GetLocalSessionId.restype = PyChipError

        localSessionId = ctypes.c_uint16(0)

        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_GetLocalSessionId(self._deviceProxy, pointer(localSessionId))
        ).raise_on_error()

        return localSessionId.value

    @property
    def numTotalSessions(self) -> int:
        self._dmLib.pychip_GetNumSessionsToPeer.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_uint32)]
        self._dmLib.pychip_GetNumSessionsToPeer.restype = PyChipError

        numSessions = ctypes.c_uint32(0)

        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_GetNumSessionsToPeer(self._deviceProxy, pointer(numSessions))
        ).raise_on_error()

        return numSessions.value


DiscoveryFilterType = discovery.FilterType


class ChipDeviceController():
    activeList = set()

    def __init__(self, opCredsContext: ctypes.c_void_p, fabricId: int, nodeId: int, adminVendorId: int, catTags: typing.List[int] = [], paaTrustStorePath: str = "", useTestCommissioner: bool = False, fabricAdmin: FabricAdmin = None, name: str = None):
        self.state = DCState.NOT_INITIALIZED
        self.devCtrl = None
        self._ChipStack = builtins.chipStack
        self._dmLib = None

        self._InitLib()

        self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback(_IssueNOCChainCallbackPythonCallback)

        devCtrl = c_void_p(None)

        c_catTags = (c_uint32 * len(catTags))()

        for i, item in enumerate(catTags):
            c_catTags[i] = item

        self._dmLib.pychip_OpCreds_AllocateController.argtypes = [c_void_p, POINTER(
            c_void_p), c_uint64, c_uint64, c_uint16, c_char_p, c_bool, c_bool, POINTER(c_uint32), c_uint32]
        self._dmLib.pychip_OpCreds_AllocateController.restype = PyChipError

        # TODO(erjiaqing@): Figure out how to control enableServerInteractions for a single device controller (node)
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_OpCreds_AllocateController(c_void_p(
                opCredsContext), pointer(devCtrl), fabricId, nodeId, adminVendorId, c_char_p(None if len(paaTrustStorePath) == 0 else str.encode(paaTrustStorePath)), useTestCommissioner, self._ChipStack.enableServerInteractions, c_catTags, len(catTags))
        ).raise_on_error()

        self.devCtrl = devCtrl
        self._fabricAdmin = fabricAdmin
        self._fabricId = fabricId
        self._nodeId = nodeId
        self._caIndex = fabricAdmin.caIndex

        if name is None:
            self._name = "caIndex(%x)/fabricId(0x%016X)/nodeId(0x%016X)" % (fabricAdmin.caIndex, fabricId, nodeId)
        else:
            self._name = name

        self._Cluster = ChipClusters(builtins.chipStack)
        self._Cluster.InitLib(self._dmLib)

        def HandleCommissioningComplete(nodeid, err):
            if err.is_success:
                print("Commissioning complete")
            else:
                print("Failed to commission: {}".format(err))

            self.state = DCState.IDLE
            self._ChipStack.callbackRes = err
            self._ChipStack.commissioningEventRes = err
            self._ChipStack.commissioningCompleteEvent.set()
            self._ChipStack.completeEvent.set()

        def HandlePASEEstablishmentComplete(err: PyChipError):
            if err.is_success:
                print("Failed to establish secure session to device: {}".format(err))
                self._ChipStack.callbackRes = err.to_exception()
            else:
                print("Established secure session with Device")

            if self.state != DCState.COMMISSIONING:
                # During Commissioning, HandlePASEEstablishmentComplete will also be called,
                # in this case the async operation should be marked as finished by
                # HandleCommissioningComplete instead this function.
                self.state = DCState.IDLE
                self._ChipStack.completeEvent.set()
            else:
                # When commissioning, getting an error during key exhange
                # needs to unblock the entire commissioning flow.
                if not err.is_success:
                    HandleCommissioningComplete(0, err)

        self.cbHandlePASEEstablishmentCompleteFunct = _DevicePairingDelegate_OnPairingCompleteFunct(
            HandlePASEEstablishmentComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(
            self.devCtrl, self.cbHandlePASEEstablishmentCompleteFunct)

        self.cbHandleCommissioningCompleteFunct = _DevicePairingDelegate_OnCommissioningCompleteFunct(
            HandleCommissioningComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
            self.devCtrl, self.cbHandleCommissioningCompleteFunct)

        self.state = DCState.IDLE
        self._isActive = True

        # Validate FabricID/NodeID followed from NOC Chain
        self._fabricId = self.GetFabricIdInternal()
        assert self._fabricId == fabricId
        self._nodeId = self.GetNodeIdInternal()
        assert self._nodeId == nodeId

        ChipDeviceController.activeList.add(self)

    @property
    def fabricAdmin(self) -> FabricAdmin:
        return self._fabricAdmin

    @property
    def nodeId(self) -> int:
        return self._nodeId

    @property
    def fabricId(self) -> int:
        return self._fabricId

    @property
    def caIndex(self) -> int:
        return self._caIndex

    @property
    def name(self) -> str:
        return self._name

    @name.setter
    def name(self, new_name: str):
        self._name = new_name

    @property
    def isActive(self) -> bool:
        return self._isActive

    def Shutdown(self):
        ''' Shuts down this controller and reclaims any used resources, including the bound
            C++ constructor instance in the SDK.
        '''
        if (self._isActive):
            if self.devCtrl != None:
                self._ChipStack.Call(
                    lambda: self._dmLib.pychip_DeviceController_DeleteDeviceController(
                        self.devCtrl)
                ).raise_on_error()
                self.devCtrl = None

            ChipDeviceController.activeList.remove(self)
            self._isActive = False

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
        if (not self._isActive):
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

    def ConnectBLE(self, discriminator, setupPinCode, nodeid):
        self.CheckIsActive()

        self._ChipStack.commissioningCompleteEvent.clear()

        self.state = DCState.COMMISSIONING
        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_ConnectBLE(
                self.devCtrl, discriminator, setupPinCode, nodeid)
        ).raise_on_error()
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes.is_success

    def CloseBLEConnection(self):
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceCommissioner_CloseBleConnection(
                self.devCtrl)
        ).raise_on_error()

    def ExpireSessions(self, nodeid):
        """Close all sessions with `nodeid` (if any existed) so that sessions get re-established.

        This is needed to properly handle operations that invalidate a node's state, such as
        UpdateNOC.

        WARNING: ONLY CALL THIS IF YOU UNDERSTAND THE SIDE-EFFECTS
        """
        self.CheckIsActive()

        self._ChipStack.Call(lambda: self._dmLib.pychip_ExpireSessions(self.devCtrl, nodeid)).raise_on_error()

    # TODO: This needs to be called MarkSessionDefunct
    def CloseSession(self, nodeid):
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_CloseSession(
                self.devCtrl, nodeid)
        ).raise_on_error()

    def EstablishPASESessionBLE(self, setupPinCode: int, discriminator: int, nodeid: int):
        self.CheckIsActive()

        self.state = DCState.RENDEZVOUS_ONGOING
        return self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_EstablishPASESessionBLE(
                self.devCtrl, setupPinCode, discriminator, nodeid)
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

    def CommissionOnNetwork(self, nodeId: int, setupPinCode: int, filterType: DiscoveryFilterType = DiscoveryFilterType.NONE, filter: typing.Any = None):
        '''
        Does the routine for OnNetworkCommissioning, with a filter for mDNS discovery.
        Supported filters are:

            DiscoveryFilterType.NONE
            DiscoveryFilterType.SHORT_DISCRIMINATOR
            DiscoveryFilterType.LONG_DISCRIMINATOR
            DiscoveryFilterType.VENDOR_ID
            DiscoveryFilterType.DEVICE_TYPE
            DiscoveryFilterType.COMMISSIONING_MODE
            DiscoveryFilterType.INSTANCE_NAME
            DiscoveryFilterType.COMMISSIONER
            DiscoveryFilterType.COMPRESSED_FABRIC_ID

        The filter can be an integer, a string or None depending on the actual type of selected filter.
        '''
        self.CheckIsActive()

        # IP connection will run through full commissioning, so we need to wait
        # for the commissioning complete event, not just any callback.
        self.state = DCState.COMMISSIONING

        # Convert numerical filters to string for passing down to binding.
        if isinstance(filter, int):
            filter = str(filter)

        self._ChipStack.commissioningCompleteEvent.clear()

        self._ChipStack.CallAsync(
            lambda: self._dmLib.pychip_DeviceController_OnNetworkCommission(
                self.devCtrl, nodeId, setupPinCode, int(filterType), str(filter).encode("utf-8") + b"\x00" if filter is not None else None)
        )
        if not self._ChipStack.commissioningCompleteEvent.isSet():
            # Error 50 is a timeout
            return False
        return self._ChipStack.commissioningEventRes == 0

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
        """ DEPRECATED, DO NOT USE! Use `CommissionOnNetwork` or `CommissionWithCode` """
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

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetWiFiCredentials(
                ssid.encode("utf-8"), credentials.encode("utf-8"))
        ).raise_on_error()

    def SetThreadOperationalDataset(self, threadOperationalDataset):
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetThreadOperationalDataset(
                threadOperationalDataset, len(threadOperationalDataset))
        ).raise_on_error()

    def ResolveNode(self, nodeid):
        self.CheckIsActive()

        self.GetConnectedDeviceSync(nodeid, allowPASE=False)

    def GetAddressAndPort(self, nodeid):
        self.CheckIsActive()

        address = create_string_buffer(64)
        port = c_uint16(0)

        # Intentially return None instead of raising exceptions on error
        error = self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetAddressAndPort(
                self.devCtrl, nodeid, address, 64, pointer(port))
        )

        return (address.value.decode(), port.value) if error == 0 else None

    def DiscoverCommissionableNodes(self, filterType: discovery.FilterType = discovery.FilterType.NONE, filter: typing.Any = None, stopOnFirst: bool = False, timeoutSecond: int = 5) -> typing.Union[None, CommissionableNode, typing.List[CommissionableNode]]:
        ''' Discover commissionable nodes via DNS-SD with specified filters.
            Supported filters are:

                discovery.FilterType.NONE
                discovery.FilterType.SHORT_DISCRIMINATOR
                discovery.FilterType.LONG_DISCRIMINATOR
                discovery.FilterType.VENDOR_ID
                discovery.FilterType.DEVICE_TYPE
                discovery.FilterType.COMMISSIONING_MODE
                discovery.FilterType.INSTANCE_NAME
                discovery.FilterType.COMMISSIONER
                discovery.FilterType.COMPRESSED_FABRIC_ID

            This function will always return a list of CommissionableDevice. When stopOnFirst is set, this function will return when at least one device is discovered or on timeout.
        '''
        self.CheckIsActive()

        if isinstance(filter, int):
            filter = str(filter)

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodes(
                self.devCtrl, int(filterType), str(filter).encode("utf-8") + b"\x00")).raise_on_error()

        if timeoutSecond != 0:
            if stopOnFirst:
                target = time.time() + timeoutSecond
                while time.time() < target:
                    if self._ChipStack.Call(lambda: self._dmLib.pychip_DeviceController_HasDiscoveredCommissionableNode(self.devCtrl)):
                        break
                    time.sleep(0.1)
            else:
                time.sleep(timeoutSecond)

        return self.GetDiscoveredDevices()

    def DiscoverCommissionableNodesLongDiscriminator(self, long_discriminator):
        ''' Deprecated, use DiscoverCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(
                self.devCtrl, long_discriminator)
        ).raise_on_error()

    def DiscoverCommissionableNodesShortDiscriminator(self, short_discriminator):
        ''' Deprecated, use DiscoverCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(
                self.devCtrl, short_discriminator)
        ).raise_on_error()

    def DiscoverCommissionableNodesVendor(self, vendor):
        ''' Deprecated, use DiscoverCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor(
                self.devCtrl, vendor)
        ).raise_on_error()

    def DiscoverCommissionableNodesDeviceType(self, device_type):
        ''' Deprecated, use DiscoverCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType(
                self.devCtrl, device_type)
        ).raise_on_error()

    def DiscoverCommissionableNodesCommissioningEnabled(self):
        ''' Deprecated, use DiscoverCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(
                self.devCtrl)
        ).raise_on_error()

    def PrintDiscoveredDevices(self):
        ''' Deprecated, use GetCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_PrintDiscoveredDevices(
                self.devCtrl)
        )

    def GetDiscoveredDevices(self):
        def GetDevices(devCtrl):
            devices = []

            @_ChipDeviceController_IterateDiscoveredCommissionableNodesFunct
            def HandleDevice(deviceJson, deviceJsonLen):
                jsonStr = ctypes.string_at(deviceJson, deviceJsonLen).decode("utf-8")
                device = dacite.from_dict(data_class=CommissionableNode, data=json.loads(jsonStr))
                device.SetDeviceController(devCtrl)
                devices.append(device)

            self._dmLib.pychip_DeviceController_IterateDiscoveredCommissionableNodes(devCtrl.devCtrl, HandleDevice)
            return devices

        return self._ChipStack.Call(lambda: GetDevices(self))

    def GetIPForDiscoveredDevice(self, idx, addrStr, length):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice(
                self.devCtrl, idx, addrStr, length)
        )

    def DiscoverAllCommissioning(self):
        ''' Deprecated, use DiscoverCommissionableNodes
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes(
                self.devCtrl)
        ).raise_on_error()

    def OpenCommissioningWindow(self, nodeid, timeout, iteration, discriminator, option):
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_OpenCommissioningWindow(
                self.devCtrl, nodeid, timeout, iteration, discriminator, option)
        ).raise_on_error()

    def GetCompressedFabricId(self):
        self.CheckIsActive()

        fabricid = c_uint64(0)

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetCompressedFabricId(
                self.devCtrl, pointer(fabricid))
        ).raise_on_error()

        return fabricid.value

    def GetFabricIdInternal(self):
        """Get the fabric ID from the object. Only used to validate cached value from property."""
        self.CheckIsActive()

        fabricid = c_uint64(0)

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetFabricId(
                self.devCtrl, pointer(fabricid))
        ).raise_on_error()

        return fabricid.value

    def GetNodeIdInternal(self) -> int:
        """Get the node ID from the object. Only used to validate cached value from property."""
        self.CheckIsActive()

        nodeid = c_uint64(0)

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetNodeId(
                self.devCtrl, pointer(nodeid))
        ).raise_on_error()

        return nodeid.value

    def GetClusterHandler(self):
        self.CheckIsActive()

        return self._Cluster

    def GetConnectedDeviceSync(self, nodeid, allowPASE=True, timeoutMs: int = None):
        ''' Returns DeviceProxyWrapper upon success.'''
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
                self.devCtrl, nodeid, byref(returnDevice)), timeoutMs)
            if res.is_success:
                print('Using PASE connection')
                return DeviceProxyWrapper(returnDevice)

        self._ChipStack.Call(lambda: self._dmLib.pychip_GetConnectedDeviceByNodeId(
            self.devCtrl, nodeid, DeviceAvailableCallback), timeoutMs).raise_on_error()

        # The callback might have been received synchronously (during self._ChipStack.Call()).
        # Check if the device is already set before waiting for the callback.
        if returnDevice.value is None:
            with deviceAvailableCV:
                timeout = None
                if (timeoutMs):
                    timeout = float(timeoutMs) / 1000

                ret = deviceAvailableCV.wait(timeout)
                if ret is False:
                    raise TimeoutError("Timed out waiting for DNS-SD resolution")

        if returnDevice.value is None:
            returnErr.raise_on_error()

        return DeviceProxyWrapper(returnDevice, self._dmLib)

    def ComputeRoundTripTimeout(self, nodeid, upperLayerProcessingTimeoutMs: int = 0):
        ''' Returns a computed timeout value based on the round-trip time it takes for the peer at the other end of the session to
            receive a message, process it and send it back. This is computed based on the session type, the type of transport, sleepy
            characteristics of the target and a caller-provided value for the time it takes to process a message at the upper layer on
            the target For group sessions.

            This will result in a session being established if one wasn't already.
        '''
        device = self.GetConnectedDeviceSync(nodeid)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_DeviceProxy_ComputeRoundTripTimeout(
            device.deviceProxy, upperLayerProcessingTimeoutMs))
        return res

    async def SendCommand(self, nodeid: int, endpoint: int, payload: ClusterObjects.ClusterCommand, responseType=None, timedRequestTimeoutMs: int = None, interactionTimeoutMs: int = None):
        '''
        Send a cluster-object encapsulated command to a node and get returned a future that can be awaited upon to receive the response.
        If a valid responseType is passed in, that will be used to deserialize the object. If not, the type will be automatically deduced
        from the metadata received over the wire.

        timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
        interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the right
                              timeout value based on transport characteristics as well as the responsiveness of the target.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid, timeoutMs=interactionTimeoutMs)
        ClusterCommand.SendCommand(
            future, eventLoop, responseType, device.deviceProxy, ClusterCommand.CommandPath(
                EndpointId=endpoint,
                ClusterId=payload.cluster_id,
                CommandId=payload.command_id,
            ), payload, timedRequestTimeoutMs=timedRequestTimeoutMs, interactionTimeoutMs=interactionTimeoutMs).raise_on_error()
        return await future

    async def WriteAttribute(self, nodeid: int, attributes: typing.List[typing.Tuple[int, ClusterObjects.ClusterAttributeDescriptor, int]], timedRequestTimeoutMs: int = None, interactionTimeoutMs: int = None):
        '''
        Write a list of attributes on a target node.

        nodeId: Target's Node ID
        timedWriteTimeoutMs: Timeout for a timed write request. Omit or set to 'None' to indicate a non-timed request.
        attributes: A list of tuples of type (endpoint, cluster-object):
        interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the right
                              timeout value based on transport characteristics as well as the responsiveness of the target.

        E.g
            (1, Clusters.UnitTesting.Attributes.XYZAttribute('hello')) -- Write 'hello' to the XYZ attribute on the test cluster to endpoint 1
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = self.GetConnectedDeviceSync(nodeid, timeoutMs=interactionTimeoutMs)

        attrs = []
        for v in attributes:
            if len(v) == 2:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    v[0], v[1], 0, 0, v[1].value))
            else:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    v[0], v[1], v[2], 1, v[1].value))

        ClusterAttribute.WriteAttributes(
            future, eventLoop, device.deviceProxy, attrs, timedRequestTimeoutMs=timedRequestTimeoutMs, interactionTimeoutMs=interactionTimeoutMs).raise_on_error()
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
            eventNumberFilter: typing.Optional[int] = None, returnClusterObject: bool = False, reportInterval: typing.Tuple[int, int] = None, fabricFiltered: bool = True, keepSubscriptions: bool = False):
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
                ReadAttribute(1, [ Clusters.BasicInformation ] ) -- case 5 above.
                ReadAttribute(1, [ (1, Clusters.BasicInformation.Attributes.Location ] ) -- case 1 above.

        dataVersionFilters: A list of tuples of (endpoint, cluster, data version).

        events: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,    Cluster = specific,   Event = specific, Urgent = True/False
            (endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,    Cluster = specific,   Event = *, Urgent = True/False
            (Clusters.ClusterA.EventA, urgent):                 Endpoint = *,           Cluster = specific,   Event = specific, Urgent = True/False
            endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
            Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Event = *, Urgent = True/False
            '*' or ():                                  Endpoint = *,           Cluster = *,          Event = *, Urgent = True/False

        eventNumberFilter: Optional minimum event number filter.

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

        ClusterAttribute.Read(future=future, eventLoop=eventLoop, device=device.deviceProxy, devCtrl=self, attributes=attributePaths, dataVersionFilters=clusterDataVersionFilters, events=eventPaths, eventNumberFilter=eventNumberFilter, returnClusterObject=returnClusterObject,
                              subscriptionParameters=ClusterAttribute.SubscriptionParameters(reportInterval[0], reportInterval[1]) if reportInterval else None, fabricFiltered=fabricFiltered, keepSubscriptions=keepSubscriptions).raise_on_error()
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
                ReadAttribute(1, [ Clusters.BasicInformation ] ) -- case 5 above.
                ReadAttribute(1, [ (1, Clusters.BasicInformation.Attributes.Location ] ) -- case 1 above.

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
    ]], eventNumberFilter: typing.Optional[int] = None, reportInterval: typing.Tuple[int, int] = None, keepSubscriptions: bool = False):
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
            ReadEvent(1, [ Clusters.BasicInformation ] ) -- case 5 above.
            ReadEvent(1, [ (1, Clusters.BasicInformation.Events.Location ] ) -- case 1 above.

        eventNumberFilter: Optional minimum event number filter.
        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        '''
        res = await self.Read(nodeid=nodeid, events=events, eventNumberFilter=eventNumberFilter, reportInterval=reportInterval, keepSubscriptions=keepSubscriptions)
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
            return (int(ex.status), None)

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
            self._dmLib.pychip_DeviceController_DeleteDeviceController.restype = PyChipError

            self._dmLib.pychip_DeviceController_ConnectBLE.argtypes = [
                c_void_p, c_uint16, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectBLE.restype = PyChipError

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]

            self._dmLib.pychip_DeviceController_SetThreadOperationalDataset.argtypes = [
                c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_SetThreadOperationalDataset.restype = PyChipError

            self._dmLib.pychip_DeviceController_SetWiFiCredentials.argtypes = [
                c_char_p, c_char_p]
            self._dmLib.pychip_DeviceController_SetWiFiCredentials.restype = PyChipError

            self._dmLib.pychip_DeviceController_Commission.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_Commission.restype = PyChipError

            self._dmLib.pychip_DeviceController_OnNetworkCommission.argtypes = [c_void_p, c_uint64, c_uint32, c_uint8, c_char_p]
            self._dmLib.pychip_DeviceController_OnNetworkCommission.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodes.argtypes = [
                c_void_p, c_uint8, c_char_p]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodes.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesVendor.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType.argtypes = [
                c_void_p, c_uint16]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesDeviceType.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled.restype = PyChipError

            self._dmLib.pychip_DeviceController_EstablishPASESessionIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_EstablishPASESessionIP.restype = PyChipError

            self._dmLib.pychip_DeviceController_EstablishPASESessionBLE.argtypes = [
                c_void_p, c_uint32, c_uint16, c_uint64]
            self._dmLib.pychip_DeviceController_EstablishPASESessionBLE.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_DiscoverAllCommissionableNodes.restype = PyChipError
            self._dmLib.pychip_DeviceController_PrintDiscoveredDevices.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_PrintDiscoveredDevices.argtypes = [
                c_void_p, _ChipDeviceController_IterateDiscoveredCommissionableNodesFunct]
            self._dmLib.pychip_DeviceController_HasDiscoveredCommissionableNode.argtypes = [c_void_p]
            self._dmLib.pychip_DeviceController_HasDiscoveredCommissionableNode.restype = c_bool

            self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice.argtypes = [
                c_void_p, c_int, c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice.restype = c_bool

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectIP.restype = PyChipError

            self._dmLib.pychip_DeviceController_ConnectWithCode.argtypes = [
                c_void_p, c_char_p, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectWithCode.restype = PyChipError

            self._dmLib.pychip_DeviceController_CloseSession.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_CloseSession.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetAddressAndPort.argtypes = [
                c_void_p, c_uint64, c_char_p, c_uint64, POINTER(c_uint16)]
            self._dmLib.pychip_DeviceController_GetAddressAndPort.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnPairingCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnCommissioningCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnCommissioningStatusUpdateFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.restype = PyChipError

            self._dmLib.pychip_GetConnectedDeviceByNodeId.argtypes = [
                c_void_p, c_uint64, _DeviceAvailableFunct]
            self._dmLib.pychip_GetConnectedDeviceByNodeId.restype = PyChipError

            self._dmLib.pychip_FreeOperationalDeviceProxy.argtypes = [
                c_void_p]
            self._dmLib.pychip_FreeOperationalDeviceProxy.restype = PyChipError

            self._dmLib.pychip_GetDeviceBeingCommissioned.argtypes = [
                c_void_p, c_uint64, c_void_p]
            self._dmLib.pychip_GetDeviceBeingCommissioned.restype = PyChipError

            self._dmLib.pychip_ExpireSessions.argtypes = [c_void_p, c_uint64]
            self._dmLib.pychip_ExpireSessions.restype = PyChipError

            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceCommissioner_CloseBleConnection.restype = PyChipError

            self._dmLib.pychip_GetCommandSenderHandle.argtypes = [c_void_p]
            self._dmLib.pychip_GetCommandSenderHandle.restype = c_uint64

            self._dmLib.pychip_DeviceController_GetCompressedFabricId.argtypes = [
                c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetCompressedFabricId.restype = PyChipError

            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.argtypes = [
                c_void_p, c_uint64, c_uint16, c_uint32, c_uint16, c_uint8]
            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.restype = PyChipError
            self._dmLib.pychip_TestCommissionerUsed.argtypes = []
            self._dmLib.pychip_TestCommissionerUsed.restype = c_bool

            self._dmLib.pychip_TestCommissioningCallbacks.argtypes = []
            self._dmLib.pychip_TestCommissioningCallbacks.restype = c_bool

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
            self._dmLib.pychip_DeviceController_IssueNOCChain.restype = PyChipError

            self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback.argtypes = [
                _IssueNOCChainCallbackPythonCallbackFunct]
            self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback.restype = None

            self._dmLib.pychip_DeviceController_GetNodeId.argtypes = [c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetNodeId.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetFabricId.argtypes = [c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetFabricId.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetLogFilter = [None]
            self._dmLib.pychip_DeviceController_GetLogFilter = c_uint8
