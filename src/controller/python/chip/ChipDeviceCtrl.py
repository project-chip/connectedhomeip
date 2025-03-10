#
#    Copyright (c) 2020-2025 Project CHIP Authors
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

# Needed to use types in type hints before they are fully defined.
from __future__ import absolute_import, annotations, print_function

import asyncio
import builtins
import concurrent.futures
import copy
import ctypes
import enum
import json
import logging
import secrets
import threading
import typing
from ctypes import (CDLL, CFUNCTYPE, POINTER, Structure, byref, c_bool, c_char, c_char_p, c_int, c_int32, c_size_t, c_uint8,
                    c_uint16, c_uint32, c_uint64, c_void_p, cast, create_string_buffer, pointer, py_object, resize, string_at)
from dataclasses import dataclass

import dacite  # type: ignore

from . import FabricAdmin
from . import clusters as Clusters
from . import discovery
from .bdx import Bdx
from .clusters import Attribute as ClusterAttribute
from .clusters import ClusterObjects as ClusterObjects
from .clusters import Command as ClusterCommand
from .clusters.CHIPClusters import ChipClusters
from .crypto import p256keypair
from .interaction_model import SessionParameters, SessionParametersStruct
from .native import PyChipError

__all__ = ["ChipDeviceController", "CommissioningParameters"]

# Defined in $CHIP_ROOT/src/lib/core/CHIPError.h
CHIP_ERROR_TIMEOUT: int = 50

_RCACCallbackType = CFUNCTYPE(None, POINTER(c_uint8), c_size_t)

LOGGER = logging.getLogger(__name__)

_DevicePairingDelegate_OnPairingCompleteFunct = CFUNCTYPE(None, PyChipError)
_DeviceUnpairingCompleteFunct = CFUNCTYPE(None, c_uint64, PyChipError)
_DevicePairingDelegate_OnCommissioningCompleteFunct = CFUNCTYPE(
    None, c_uint64, PyChipError)
_DevicePairingDelegate_OnOpenWindowCompleteFunct = CFUNCTYPE(
    None, c_uint64, c_uint32, c_char_p, c_char_p, PyChipError)
_DevicePairingDelegate_OnCommissioningStatusUpdateFunct = CFUNCTYPE(
    None, c_uint64, c_uint8, PyChipError)
_DevicePairingDelegate_OnFabricCheckFunct = CFUNCTYPE(
    None, c_uint64)
# void (*)(Device *, CHIP_ERROR).
#
# CHIP_ERROR is actually signed, so using c_uint32 is weird, but everything
# else seems to do it.
_DeviceAvailableCallbackFunct = CFUNCTYPE(None, py_object, c_void_p, PyChipError)

_IssueNOCChainCallbackPythonCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError, c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_size_t, c_uint64)

_ChipDeviceController_IterateDiscoveredCommissionableNodesFunct = CFUNCTYPE(None, c_char_p, c_size_t)

# Defines for the transport payload types to use to select the suitable
# underlying transport of the session.
# class TransportPayloadCapability(ctypes.c_int):


class TransportPayloadCapability(ctypes.c_int):
    MRP_PAYLOAD = 0
    LARGE_PAYLOAD = 1
    MRP_OR_TCP_PAYLOAD = 2


@dataclass
class CommissioningParameters:
    setupPinCode: int
    setupManualCode: str
    setupQRCode: str


@dataclass
class NOCChain:
    nocBytes: typing.Optional[bytes]
    icacBytes: typing.Optional[bytes]
    rcacBytes: typing.Optional[bytes]
    ipkBytes: typing.Optional[bytes]
    adminSubject: int


@dataclass
class ICDRegistrationParameters:
    symmetricKey: typing.Optional[bytes]
    checkInNodeId: typing.Optional[int]
    monitoredSubject: typing.Optional[int]
    stayActiveMs: typing.Optional[int]
    clientType: typing.Optional[Clusters.IcdManagement.Enums.ClientTypeEnum]

    class CStruct(Structure):
        _fields_ = [('symmetricKey', c_char_p), ('symmetricKeyLength', c_size_t), ('checkInNodeId',
                                                                                   c_uint64), ('monitoredSubject', c_uint64), ('stayActiveMsec', c_uint32), ('clientType', c_uint8)]

    def to_c(self):
        return ICDRegistrationParameters.CStruct(self.symmetricKey, len(self.symmetricKey), self.checkInNodeId, self.monitoredSubject, self.stayActiveMs, self.clientType.value)


@_DeviceAvailableCallbackFunct
def _DeviceAvailableCallback(closure, device, err):
    closure.deviceAvailable(device, err)


@_IssueNOCChainCallbackPythonCallbackFunct
def _IssueNOCChainCallbackPythonCallback(devCtrl, status: PyChipError, noc: c_void_p, nocLen: int, icac: c_void_p,
                                         icacLen: int, rcac: c_void_p, rcacLen: int, ipk: c_void_p, ipkLen: int, adminSubject: int):
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


# Methods for ICD
class ScopedNodeId(Structure):
    _fields_ = [("nodeId", c_uint64), ("fabricIndex", c_uint8)]

    def __hash__(self):
        return self.nodeId << 8 | self.fabricIndex

    def __str__(self):
        return f"({self.fabricIndex}:{self.nodeId:16x})"

    def __eq__(self, other):
        return self.nodeId == other.nodeId and self.fabricIndex == other.fabricIndex


_OnCheckInCompleteFunct = CFUNCTYPE(None, ScopedNodeId)

_OnCheckInCompleteWaitListLock = threading.Lock()
_OnCheckInCompleteWaitList: typing.Dict[ScopedNodeId, set] = dict()


@_OnCheckInCompleteFunct
def _OnCheckInComplete(scopedNodeId: ScopedNodeId):
    callbacks = []
    with _OnCheckInCompleteWaitListLock:
        callbacks = list(_OnCheckInCompleteWaitList.get(scopedNodeId, set()))

    for callback in callbacks:
        callback(scopedNodeId)


def RegisterOnActiveCallback(scopedNodeId: ScopedNodeId, callback: typing.Callable[[ScopedNodeId], None]):
    ''' Registers a callback when the device with given (fabric index, node id) becomes active.

    Does nothing if the callback is already registered.
    '''
    with _OnCheckInCompleteWaitListLock:
        waitList = _OnCheckInCompleteWaitList.get(scopedNodeId, set())
        waitList.add(callback)
        _OnCheckInCompleteWaitList[scopedNodeId] = waitList


def UnregisterOnActiveCallback(scopedNodeId: ScopedNodeId, callback: typing.Callable[[ScopedNodeId], None]):
    ''' Unregisters a callback when the device with given (fabric index, node id) becomes active.

    Does nothing if the callback has not been registered.
    '''
    with _OnCheckInCompleteWaitListLock:
        _OnCheckInCompleteWaitList.get(scopedNodeId, set()).remove(callback)


async def WaitForCheckIn(scopedNodeId: ScopedNodeId, timeoutSeconds: float):
    ''' Waits for a device becomes active.

    Returns:
        - A future, completes when the device becomes active.
    '''
    eventLoop = asyncio.get_running_loop()
    future = eventLoop.create_future()

    def OnCheckInCallback(nodeid):
        eventLoop.call_soon_threadsafe(lambda: future.done() or future.set_result(None))

    RegisterOnActiveCallback(scopedNodeId, OnCheckInCallback)

    try:
        await asyncio.wait_for(future, timeout=timeoutSeconds)
    finally:
        UnregisterOnActiveCallback(scopedNodeId, OnCheckInCallback)

# This is a fix for WEAV-429. Jay Logue recommends revisiting this at a later
# date to allow for truly multiple instances so this is temporary.


def _singleton(cls):
    instance = [None]

    def wrapper(*args, **kwargs):
        if instance[0] is None:
            instance[0] = cls(*args, **kwargs)
        return instance[0]

    return wrapper


class CallbackContext:
    """A context manager for handling callbacks that are expected to be called exactly once.

    The context manager makes sure that no concurrent operations which use the same callback
    handlers are executed.
    """

    def __init__(self, lock: asyncio.Lock) -> None:
        self._lock = lock
        self._future = None

    async def __aenter__(self):
        await self._lock.acquire()
        self._future = concurrent.futures.Future()
        return self

    @property
    def future(self) -> typing.Optional[concurrent.futures.Future]:
        return self._future

    async def __aexit__(self, exc_type, exc_value, traceback):
        if not self._future.done():
            # In case the initial call (which sets up for the callback) fails,
            # the future will never be used actually. So just cancel it here
            # for completeness, in case somebody is expecting it to be completed.
            self._future.cancel()
        self._future = None
        self._lock.release()


class CommissioningContext(CallbackContext):
    """A context manager for handling commissioning callbacks that are expected to be called exactly once.

    This context also resets commissioning related device controller state.
    """

    def __init__(self, devCtrl: ChipDeviceControllerBase, lock: asyncio.Lock) -> None:
        super().__init__(lock)
        self._devCtrl = devCtrl

    async def __aenter__(self):
        await super().__aenter__()
        self._devCtrl._fabricCheckNodeId = -1
        return self

    async def __aexit__(self, exc_type, exc_value, traceback):
        await super().__aexit__(exc_type, exc_value, traceback)


class CommissionableNode(discovery.CommissionableNode):
    def SetDeviceController(self, devCtrl: 'ChipDeviceController'):
        self._devCtrl = devCtrl

    def Commission(self, nodeId: int, setupPinCode: int) -> int:
        ''' Commission the device using the device controller discovered this device.

        nodeId: The nodeId commissioned to the device
        setupPinCode: The setup pin code of the device

        Returns:
            - Effective Node ID of the device (as defined by the assigned NOC)
        '''
        return self._devCtrl.CommissionOnNetwork(
            nodeId, setupPinCode, filterType=discovery.FilterType.INSTANCE_NAME, filter=self.instanceName)

    def __rich_repr__(self):
        yield "(To Be Commissioned By)", self._devCtrl.name

        for k in self.__dataclass_fields__.keys():
            if k in self.__dict__:
                yield k, self.__dict__[k]


class DeviceProxyWrapper():
    ''' Encapsulates a pointer to OperationalDeviceProxy on the c++ side that needs to be
        freed when DeviceProxyWrapper goes out of scope. There is a potential issue where
        if this is copied around that a double free will occur, but how this is used today
        that is not an issue that needs to be accounted for and it will become very apparent
        if that happens.
    '''
    class DeviceProxyType(enum.Enum):
        OPERATIONAL = enum.auto(),
        COMMISSIONEE = enum.auto(),

    def __init__(self, deviceProxy: ctypes.c_void_p, proxyType, dmLib=None):
        self._deviceProxy = deviceProxy
        self._dmLib = dmLib
        self._proxyType = proxyType

    def __del__(self):
        # Commissionee device proxies are owned by the DeviceCommissioner. See #33031
        if (self._proxyType == self.DeviceProxyType.OPERATIONAL and self._dmLib is not None and hasattr(builtins, 'chipStack') and builtins.chipStack is not None):
            # This destructor is called from any threading context, including on the Matter threading context.
            # So, we cannot call chipStack.Call or chipStack.CallAsyncWithCompleteCallback which waits for the posted work to
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

    @property
    def attestationChallenge(self) -> bytes:
        self._dmLib.pychip_GetAttestationChallenge.argtypes = (c_void_p, POINTER(c_uint8), POINTER(c_size_t))
        self._dmLib.pychip_GetAttestationChallenge.restype = PyChipError

        # this buffer is overly large, but we shall resize
        size = 64
        buf = ctypes.c_uint8(size)
        csize = ctypes.c_size_t(size)
        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_GetAttestationChallenge(self._deviceProxy, buf, ctypes.byref(csize))
        ).raise_on_error()

        resize(buf, csize.value)

        return bytes(buf)

    @property
    def sessionAllowsLargePayload(self) -> bool:
        self._dmLib.pychip_SessionAllowsLargePayload.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_bool)]
        self._dmLib.pychip_SessionAllowsLargePayload.restype = PyChipError

        supportsLargePayload = ctypes.c_bool(False)

        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_SessionAllowsLargePayload(self._deviceProxy, pointer(supportsLargePayload))
        ).raise_on_error()

        return supportsLargePayload.value

    @property
    def isSessionOverTCPConnection(self) -> bool:
        self._dmLib.pychip_IsSessionOverTCPConnection.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_bool)]
        self._dmLib.pychip_IsSessionOverTCPConnection.restype = PyChipError

        isSessionOverTCP = ctypes.c_bool(False)

        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_IsSessionOverTCPConnection(self._deviceProxy, pointer(isSessionOverTCP))
        ).raise_on_error()

        return isSessionOverTCP.value

    @property
    def isActiveSession(self) -> bool:
        self._dmLib.pychip_IsActiveSession.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_bool)]
        self._dmLib.pychip_IsActiveSession.restype = PyChipError

        isActiveSession = ctypes.c_bool(False)

        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_IsActiveSession(self._deviceProxy, pointer(isActiveSession))
        ).raise_on_error()

        return isActiveSession.value

    def closeTCPConnectionWithPeer(self):
        self._dmLib.pychip_CloseTCPConnectionWithPeer.argtypes = [ctypes.c_void_p]
        self._dmLib.pychip_CloseTCPConnectionWithPeer.restype = PyChipError

        builtins.chipStack.Call(
            lambda: self._dmLib.pychip_CloseTCPConnectionWithPeer(self._deviceProxy)
        ).raise_on_error()


DiscoveryFilterType = discovery.FilterType
DiscoveryType = discovery.DiscoveryType


class ChipDeviceControllerBase():
    activeList: typing.Set = set()

    def __init__(self, name: str = ''):
        self.devCtrl = None
        self._ChipStack = builtins.chipStack
        self._dmLib: typing.Any = None

        self._InitLib()

        pairingDelegate = c_void_p(None)
        devCtrl = c_void_p(None)

        self.pairingDelegate = pairingDelegate
        self.devCtrl = devCtrl
        self.name = name
        self._fabricCheckNodeId = -1
        self._isActive = False

        self._Cluster = ChipClusters(builtins.chipStack)
        self._Cluster.InitLib(self._dmLib)
        self._commissioning_lock: asyncio.Lock = asyncio.Lock()
        self._commissioning_context: CommissioningContext = CommissioningContext(self, self._commissioning_lock)
        self._open_window_context: CallbackContext = CallbackContext(asyncio.Lock())
        self._unpair_device_context: CallbackContext = CallbackContext(asyncio.Lock())
        self._pase_establishment_context: CallbackContext = CallbackContext(self._commissioning_lock)

    def _set_dev_ctrl(self, devCtrl, pairingDelegate):
        def HandleCommissioningComplete(nodeId: int, err: PyChipError):
            if err.is_success:
                LOGGER.info("Commissioning complete")
            else:
                LOGGER.warning("Failed to commission: {}".format(err))

            self._dmLib.pychip_DeviceController_SetIcdRegistrationParameters(False, None)

            if self._dmLib.pychip_TestCommissionerUsed():
                err = self._dmLib.pychip_GetCompletionError()

            if self._commissioning_context.future is None:
                LOGGER.exception("HandleCommissioningComplete called unexpectedly")
                return

            if err.is_success:
                self._commissioning_context.future.set_result(nodeId)

            else:
                self._commissioning_context.future.set_exception(err.to_exception())

        def HandleFabricCheck(nodeId):
            self._fabricCheckNodeId = nodeId

        def HandleOpenWindowComplete(nodeid: int, setupPinCode: int, setupManualCode: bytes,
                                     setupQRCode: bytes, err: PyChipError) -> None:
            if err.is_success:
                LOGGER.info("Open Commissioning Window complete setting nodeid {} pincode to {}".format(nodeid, setupPinCode))
                commissioningParameters = CommissioningParameters(
                    setupPinCode=setupPinCode, setupManualCode=setupManualCode.decode(), setupQRCode=setupQRCode.decode())
            else:
                LOGGER.warning("Failed to open commissioning window: {}".format(err))

            if self._open_window_context.future is None:
                LOGGER.exception("HandleOpenWindowComplete called unexpectedly")
                return

            if err.is_success:
                self._open_window_context.future.set_result(commissioningParameters)
            else:
                self._open_window_context.future.set_exception(err.to_exception())

        def HandleUnpairDeviceComplete(nodeid: int, err: PyChipError):
            if err.is_success:
                LOGGER.info("Succesfully unpaired device with nodeid {}".format(nodeid))
            else:
                LOGGER.warning("Failed to unpair device: {}".format(err))

            if self._unpair_device_context.future is None:
                LOGGER.exception("HandleUnpairDeviceComplete called unexpectedly")
                return

            if err.is_success:
                self._unpair_device_context.future.set_result(None)
            else:
                self._unpair_device_context.future.set_exception(err.to_exception())

        def HandlePASEEstablishmentComplete(err: PyChipError):
            if not err.is_success:
                LOGGER.warning("Failed to establish secure session to device: {}".format(err))
            else:
                LOGGER.info("Established secure session with Device")

            if self._commissioning_context.future is not None:
                # During Commissioning, HandlePASEEstablishmentComplete will also be called.
                # Only complete the future if PASE session establishment failed.
                if not err.is_success:
                    self._commissioning_context.future.set_exception(err.to_exception())
                return

            if self._pase_establishment_context.future is None:
                LOGGER.exception("HandlePASEEstablishmentComplete called unexpectedly")
                return

            if err.is_success:
                self._pase_establishment_context.future.set_result(None)
            else:
                self._pase_establishment_context.future.set_exception(err.to_exception())

        self.pairingDelegate = pairingDelegate
        self.devCtrl = devCtrl

        self.cbHandlePASEEstablishmentCompleteFunct = _DevicePairingDelegate_OnPairingCompleteFunct(
            HandlePASEEstablishmentComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(
            self.pairingDelegate, self.cbHandlePASEEstablishmentCompleteFunct)

        self.cbHandleCommissioningCompleteFunct = _DevicePairingDelegate_OnCommissioningCompleteFunct(
            HandleCommissioningComplete)

        self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
            self.pairingDelegate, self.cbHandleCommissioningCompleteFunct)

        self.cbHandleFabricCheckFunct = _DevicePairingDelegate_OnFabricCheckFunct(HandleFabricCheck)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetFabricCheckCallback(self.pairingDelegate, self.cbHandleFabricCheckFunct)

        self.cbHandleOpenWindowCompleteFunct = _DevicePairingDelegate_OnOpenWindowCompleteFunct(
            HandleOpenWindowComplete)
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetOpenWindowCompleteCallback(
            self.pairingDelegate, self.cbHandleOpenWindowCompleteFunct)

        self.cbHandleDeviceUnpairCompleteFunct = _DeviceUnpairingCompleteFunct(HandleUnpairDeviceComplete)

        self._isActive = True
        # Validate FabricID/NodeID followed from NOC Chain
        self._fabricId = self.GetFabricIdInternal()
        self._fabricIndex = self.GetFabricIndexInternal()
        self._nodeId = self.GetNodeIdInternal()

    def _finish_init(self):
        self._isActive = True

        ChipDeviceController.activeList.add(self)

    def _enablePairingCompleteCallback(self, value: bool):
        self._dmLib.pychip_ScriptDevicePairingDelegate_SetExpectingPairingComplete(self.pairingDelegate, value)

    @property
    def nodeId(self) -> int:
        return self._nodeId

    @property
    def fabricId(self) -> int:
        return self._fabricId

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
        if not self._isActive:
            return

        if self.devCtrl is not None:
            self._ChipStack.Call(
                lambda: self._dmLib.pychip_DeviceController_DeleteDeviceController(
                    self.devCtrl, self.pairingDelegate)
            ).raise_on_error()
            self.pairingDelegate = None
            self.devCtrl = None

        ChipDeviceController.activeList.remove(self)
        self._isActive = False

    def ShutdownAll(self):
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

    async def ConnectBLE(self, discriminator: int, setupPinCode: int, nodeid: int, isShortDiscriminator: bool = False) -> int:
        """Connect to a BLE device using the given discriminator and setup pin code.

        Returns:
            - Effective Node ID of the device (as defined by the assigned NOC)
        """
        self.CheckIsActive()

        async with self._commissioning_context as ctx:
            self._enablePairingCompleteCallback(True)
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_ConnectBLE(
                    self.devCtrl, discriminator, isShortDiscriminator, setupPinCode, nodeid)
            )

            return await asyncio.futures.wrap_future(ctx.future)

    async def UnpairDevice(self, nodeid: int) -> None:
        self.CheckIsActive()

        async with self._unpair_device_context as ctx:
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_UnpairDevice(
                    self.devCtrl, nodeid, self.cbHandleDeviceUnpairCompleteFunct)
            )

            return await asyncio.futures.wrap_future(ctx.future)

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

    def MarkSessionDefunct(self, nodeid):
        '''
        Marks a previously active session with the specified node as defunct to temporarily prevent it
        from being used with new exchanges to send or receive messages. This should be called when there
        is suspicion of a loss-of-sync with the session state on the associated peer, such as evidence
        of transport failure.

        If messages are received thereafter on this session, the session will be put back into the Active state.

        This function should only be called on an active session.
        This will NOT detach any existing SessionHolders.

        Parameters:
            nodeid (int): The node ID of the device whose session should be marked as defunct.

        Raises:
            RuntimeError: If the controller is not active.
            PyChipError: If the operation fails.
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_MarkSessionDefunct(
                self.devCtrl, nodeid)
        ).raise_on_error()

    def MarkSessionForEviction(self, nodeid):
        '''
        Marks the session with the specified node for eviction. It will first detach all SessionHolders
        attached to this session by calling 'OnSessionReleased' on each of them. This will force them
        to release their reference to the session. If there are no more references left, the session
        will then be de-allocated.

        Once marked for eviction, the session SHALL NOT ever become active again.

        Parameters:
            nodeid (int): The node ID of the device whose session should be marked for eviction.

        Raises:
            RuntimeError: If the controller is not active.
            PyChipError: If the operation fails.
        '''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_MarkSessionForEviction(
                self.devCtrl, nodeid)
        ).raise_on_error()

    async def _establishPASESession(self, callFunct):
        self.CheckIsActive()

        async with self._pase_establishment_context as ctx:
            self._enablePairingCompleteCallback(True)
            await self._ChipStack.CallAsync(callFunct)
            await asyncio.futures.wrap_future(ctx.future)

    async def EstablishPASESessionBLE(self, setupPinCode: int, discriminator: int, nodeid: int) -> None:
        await self._establishPASESession(
            lambda: self._dmLib.pychip_DeviceController_EstablishPASESessionBLE(
                self.devCtrl, setupPinCode, discriminator, nodeid)
        )

    async def EstablishPASESessionIP(self, ipaddr: str, setupPinCode: int, nodeid: int, port: int = 0) -> None:
        await self._establishPASESession(
            lambda: self._dmLib.pychip_DeviceController_EstablishPASESessionIP(
                self.devCtrl, ipaddr.encode("utf-8"), setupPinCode, nodeid, port)
        )

    async def EstablishPASESession(self, setUpCode: str, nodeid: int) -> None:
        await self._establishPASESession(
            lambda: self._dmLib.pychip_DeviceController_EstablishPASESession(
                self.devCtrl, setUpCode.encode("utf-8"), nodeid)
        )

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

    def SetTestCommissionerPrematureCompleteAfter(self, stage: int):
        return self._dmLib.pychip_SetTestCommissionerPrematureCompleteAfter(
            stage)

    def CheckTestCommissionerCallbacks(self):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_TestCommissioningCallbacks()
        )

    def CheckStageSuccessful(self, stage: int):
        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_TestCommissioningStageSuccessful(stage)
        )

    def CheckTestCommissionerPaseConnection(self, nodeid):
        return self._dmLib.pychip_TestPaseConnection(nodeid)

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

    async def DiscoverCommissionableNodes(self, filterType: discovery.FilterType = discovery.FilterType.NONE, filter: typing.Any = None,
                                          stopOnFirst: bool = False, timeoutSecond: int = 5) -> typing.Union[None, CommissionableNode, typing.List[CommissionableNode]]:
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

            This function will always return a list of CommissionableDevice. When stopOnFirst is set,
            this function will return when at least one device is discovered or on timeout.
        '''
        self.CheckIsActive()

        if isinstance(filter, int):
            filter = str(filter)

        # Discovery is also used during commissioning. Make sure this manual discovery
        # and commissioning attempts do not interfere with each other.
        async with self._commissioning_lock:
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_DiscoverCommissionableNodes(
                    self.devCtrl, int(filterType), str(filter).encode("utf-8")))

            async def _wait_discovery():
                while not await self._ChipStack.CallAsyncWithResult(
                        lambda: self._dmLib.pychip_DeviceController_HasDiscoveredCommissionableNode(self.devCtrl)):
                    await asyncio.sleep(0.1)
                return

            try:
                if stopOnFirst:
                    await asyncio.wait_for(_wait_discovery(), timeoutSecond)
                else:
                    await asyncio.sleep(timeoutSecond)
            except TimeoutError:
                # Expected timeout, do nothing
                pass
            finally:
                await self._ChipStack.CallAsync(
                    lambda: self._dmLib.pychip_DeviceController_StopCommissionableDiscovery(self.devCtrl))

            return await self.GetDiscoveredDevices()

    async def GetDiscoveredDevices(self):
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

        return await self._ChipStack.CallAsyncWithResult(lambda: GetDevices(self))

    def GetIPForDiscoveredDevice(self, idx, addrStr, length):
        self.CheckIsActive()

        return self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice(
                self.devCtrl, idx, addrStr, length)
        )

    class CommissioningWindowPasscode(enum.IntEnum):
        kOriginalSetupCode = 0,
        kTokenWithRandomPin = 1,

    async def OpenCommissioningWindow(self, nodeid: int, timeout: int, iteration: int,
                                      discriminator: int, option: CommissioningWindowPasscode) -> CommissioningParameters:
        ''' Opens a commissioning window on the device with the given nodeid.
            nodeid:        Node id of the device
            timeout:       Command timeout
            iteration:     The PAKE iteration count associated with the PAKE Passcode ID and ephemeral
                           PAKE passcode verifier to be used for this commissioning. Valid range: 1000 - 100000
                           Ignored if option == 0
            discriminator: The long discriminator for the DNS-SD advertisement. Valid range: 0-4095
                           Ignored if option == 0
            option:        0 = kOriginalSetupCode
                           1 = kTokenWithRandomPIN

            Returns CommissioningParameters
        '''
        self.CheckIsActive()

        async with self._open_window_context as ctx:
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_OpenCommissioningWindow(
                    self.devCtrl, self.pairingDelegate, nodeid, timeout, iteration, discriminator, option)
            )

            return await asyncio.futures.wrap_future(ctx.future)

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

    def GetFabricIndexInternal(self):
        """Get the fabric index from the object. Only used to validate cached value from property."""
        self.CheckIsActive()

        fabricindex = c_uint8(0)

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_GetFabricIndex(
                self.devCtrl, pointer(fabricindex))
        ).raise_on_error()

        return fabricindex.value

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

    async def FindOrEstablishPASESession(self, setupCode: str, nodeid: int, timeoutMs: typing.Optional[int] = None) -> typing.Optional[DeviceProxyWrapper]:
        ''' Returns CommissioneeDeviceProxy if we can find or establish a PASE connection to the specified device'''
        self.CheckIsActive()
        returnDevice = c_void_p(None)
        res = await self._ChipStack.CallAsyncWithResult(lambda: self._dmLib.pychip_GetDeviceBeingCommissioned(
            self.devCtrl, nodeid, byref(returnDevice)), timeoutMs)
        if res.is_success:
            return DeviceProxyWrapper(returnDevice, DeviceProxyWrapper.DeviceProxyType.COMMISSIONEE, self._dmLib)

        await self.EstablishPASESession(setupCode, nodeid)

        res = await self._ChipStack.CallAsyncWithResult(lambda: self._dmLib.pychip_GetDeviceBeingCommissioned(
            self.devCtrl, nodeid, byref(returnDevice)), timeoutMs)
        if res.is_success:
            return DeviceProxyWrapper(returnDevice, DeviceProxyWrapper.DeviceProxyType.COMMISSIONEE, self._dmLib)

        return None

    def GetConnectedDeviceSync(self, nodeid, allowPASE=True, timeoutMs: typing.Optional[int] = None, payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD):
        ''' Gets an OperationalDeviceProxy or CommissioneeDeviceProxy for the specified Node.

        nodeId: Target's Node ID
        allowPASE: Get a device proxy of a device being commissioned.
        timeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.

        Returns:
            - DeviceProxyWrapper on success
        '''
        self.CheckIsActive()

        returnDevice = c_void_p(None)
        returnErr: typing.Any = None
        deviceAvailableCV = threading.Condition()

        if allowPASE:
            res = self._ChipStack.Call(lambda: self._dmLib.pychip_GetDeviceBeingCommissioned(
                self.devCtrl, nodeid, byref(returnDevice)), timeoutMs)
            if res.is_success:
                LOGGER.info('Using PASE connection')
                return DeviceProxyWrapper(returnDevice, DeviceProxyWrapper.DeviceProxyType.COMMISSIONEE, self._dmLib)

        class DeviceAvailableClosure():
            def deviceAvailable(self, device, err):
                nonlocal returnDevice
                nonlocal returnErr
                nonlocal deviceAvailableCV
                with deviceAvailableCV:
                    returnDevice = c_void_p(device)
                    returnErr = err
                    deviceAvailableCV.notify_all()
                ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

        closure = DeviceAvailableClosure()
        ctypes.pythonapi.Py_IncRef(ctypes.py_object(closure))
        self._ChipStack.Call(lambda: self._dmLib.pychip_GetConnectedDeviceByNodeId(
            self.devCtrl, nodeid, ctypes.py_object(closure), _DeviceAvailableCallback, payloadCapability),
            timeoutMs).raise_on_error()

        # The callback might have been received synchronously (during self._ChipStack.Call()).
        # Check if the device is already set before waiting for the callback.
        if returnDevice.value is None:
            with deviceAvailableCV:
                timeout = None
                if timeoutMs is not None:
                    timeout = float(timeoutMs) / 1000

                ret = deviceAvailableCV.wait(timeout)
                if ret is False:
                    raise TimeoutError("Timed out waiting for DNS-SD resolution")

        if returnDevice.value is None:
            returnErr.raise_on_error()

        return DeviceProxyWrapper(returnDevice, DeviceProxyWrapper.DeviceProxyType.OPERATIONAL, self._dmLib)

    async def WaitForActive(self, nodeid, *, timeoutSeconds=30.0, stayActiveDurationMs=30000):
        ''' Waits a LIT ICD device to become active. Will send a StayActive command to the device on active to allow human operations.

        nodeId: Node ID of the LID ICD
        stayActiveDurationMs: The duration in the StayActive command, in milliseconds

        Returns:
            - StayActiveResponse on success
        '''
        await WaitForCheckIn(ScopedNodeId(nodeid, self._fabricIndex), timeoutSeconds=timeoutSeconds)
        return await self.SendCommand(nodeid, 0, Clusters.IcdManagement.Commands.StayActiveRequest(stayActiveDuration=stayActiveDurationMs))

    async def GetConnectedDevice(self, nodeid, allowPASE: bool = True, timeoutMs: typing.Optional[int] = None,
                                 payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD):
        ''' Gets an OperationalDeviceProxy or CommissioneeDeviceProxy for the specified Node.

        nodeId: Target's Node ID
        allowPASE: Get a device proxy of a device being commissioned.
        timeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.

        Returns:
            - DeviceProxyWrapper on success
        '''
        self.CheckIsActive()

        if allowPASE:
            returnDevice = c_void_p(None)
            res = await self._ChipStack.CallAsyncWithResult(lambda: self._dmLib.pychip_GetDeviceBeingCommissioned(
                self.devCtrl, nodeid, byref(returnDevice)), timeoutMs)
            if res.is_success:
                LOGGER.info('Using PASE connection')
                return DeviceProxyWrapper(returnDevice, DeviceProxyWrapper.DeviceProxyType.COMMISSIONEE, self._dmLib)

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        class DeviceAvailableClosure():
            def __init__(self, loop, future: asyncio.Future):
                self._returnDevice = c_void_p(None)
                self._returnErr = None
                self._event_loop = loop
                self._future = future

            def _deviceAvailable(self):
                if self._future.cancelled():
                    return
                if self._returnDevice.value is not None:
                    self._future.set_result(self._returnDevice)
                else:
                    self._future.set_exception(self._returnErr.to_exception())

            def deviceAvailable(self, device, err):
                self._returnDevice = c_void_p(device)
                self._returnErr = err
                self._event_loop.call_soon_threadsafe(self._deviceAvailable)
                ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

        closure = DeviceAvailableClosure(eventLoop, future)
        ctypes.pythonapi.Py_IncRef(ctypes.py_object(closure))
        await self._ChipStack.CallAsync(lambda: self._dmLib.pychip_GetConnectedDeviceByNodeId(
            self.devCtrl, nodeid, ctypes.py_object(closure), _DeviceAvailableCallback, payloadCapability),
            timeoutMs)

        # The callback might have been received synchronously (during self._ChipStack.CallAsync()).
        # In that case the Future has already been set it will return immediately
        if timeoutMs is not None:
            timeout = float(timeoutMs) / 1000
            await asyncio.wait_for(future, timeout=timeout)
        else:
            await future

        return DeviceProxyWrapper(future.result(), DeviceProxyWrapper.DeviceProxyType.OPERATIONAL, self._dmLib)

    def ComputeRoundTripTimeout(self, nodeid, upperLayerProcessingTimeoutMs: int = 0):
        ''' Returns a computed timeout value based on the round-trip time it takes for the peer at the other end of the session to
            receive a message, process it and send it back. This is computed based on the session type, the type of transport,
            sleepy characteristics of the target and a caller-provided value for the time it takes to process a message
            at the upper layer on the target For group sessions.

            This will result in a session being established if one wasn't already.
        '''
        device = self.GetConnectedDeviceSync(nodeid)
        res = self._ChipStack.Call(lambda: self._dmLib.pychip_DeviceProxy_ComputeRoundTripTimeout(
            device.deviceProxy, upperLayerProcessingTimeoutMs))
        return res

    def GetRemoteSessionParameters(self, nodeid) -> typing.Optional[SessionParameters]:
        ''' Returns the SessionParameters of reported by the remote node associated with `nodeid`.
            If there is some error in getting SessionParameters None is returned.

            This will result in a session being established if one wasn't already established.
        '''

        # First creating the struct to make building the ByteArray to be sent to CFFI easier.
        sessionParametersStruct = SessionParametersStruct.parse(b'\x00' * SessionParametersStruct.sizeof())
        sessionParametersByteArray = SessionParametersStruct.build(sessionParametersStruct)
        device = self.GetConnectedDeviceSync(nodeid)
        self._ChipStack.Call(lambda: self._dmLib.pychip_DeviceProxy_GetRemoteSessionParameters(
            device.deviceProxy, ctypes.c_char_p(sessionParametersByteArray))).raise_on_error()

        sessionParametersStruct = SessionParametersStruct.parse(sessionParametersByteArray)
        return SessionParameters(
            sessionIdleInterval=sessionParametersStruct.SessionIdleInterval if sessionParametersStruct.SessionIdleInterval != 0 else None,
            sessionActiveInterval=sessionParametersStruct.SessionActiveInterval if sessionParametersStruct.SessionActiveInterval != 0 else None,
            sessionActiveThreshold=sessionParametersStruct.SessionActiveThreshold if sessionParametersStruct.SessionActiveThreshold != 0 else None,
            dataModelRevision=sessionParametersStruct.DataModelRevision if sessionParametersStruct.DataModelRevision != 0 else None,
            interactionModelRevision=sessionParametersStruct.InteractionModelRevision if sessionParametersStruct.InteractionModelRevision != 0 else None,
            specficiationVersion=sessionParametersStruct.SpecificationVersion if sessionParametersStruct.SpecificationVersion != 0 else None,
            maxPathsPerInvoke=sessionParametersStruct.MaxPathsPerInvoke)

    async def TestOnlySendBatchCommands(self, nodeid: int, commands: typing.List[ClusterCommand.InvokeRequestInfo],
                                        timedRequestTimeoutMs: typing.Optional[int] = None,
                                        interactionTimeoutMs: typing.Optional[int] = None, busyWaitMs: typing.Optional[int] = None,
                                        suppressResponse: typing.Optional[bool] = None, remoteMaxPathsPerInvoke: typing.Optional[int] = None,
                                        suppressTimedRequestMessage: bool = False, commandRefsOverride: typing.Optional[typing.List[int]] = None):
        '''

        Please see SendBatchCommands for description.
        TestOnly overridable arguments:
            remoteMaxPathsPerInvoke: Overrides the number of batch commands we think can be sent to remote node.
            suppressTimedRequestMessage: When set to true, we suppress sending Timed Request Message.
            commandRefsOverride: List of commandRefs to use for each command with the same index in `commands`.

        Returns:
            - TestOnlyBatchCommandResponse
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = await self.GetConnectedDevice(nodeid, timeoutMs=interactionTimeoutMs)

        ClusterCommand.TestOnlySendBatchCommands(
            future, eventLoop, device.deviceProxy, commands,
            timedRequestTimeoutMs=timedRequestTimeoutMs,
            interactionTimeoutMs=interactionTimeoutMs, busyWaitMs=busyWaitMs, suppressResponse=suppressResponse,
            remoteMaxPathsPerInvoke=remoteMaxPathsPerInvoke, suppressTimedRequestMessage=suppressTimedRequestMessage,
            commandRefsOverride=commandRefsOverride).raise_on_error()
        return await future

    async def TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(self, nodeid: int, endpoint: int,
                                                                   payload: ClusterObjects.ClusterCommand, responseType=None):
        '''

        Please see SendCommand for description.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = await self.GetConnectedDevice(nodeid, timeoutMs=None)
        ClusterCommand.TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(
            future, eventLoop, responseType, device.deviceProxy, ClusterCommand.CommandPath(
                EndpointId=endpoint,
                ClusterId=payload.cluster_id,
                CommandId=payload.command_id,
            ), payload).raise_on_error()
        return await future

    async def SendCommand(self, nodeid: int, endpoint: int, payload: ClusterObjects.ClusterCommand, responseType=None,
                          timedRequestTimeoutMs: typing.Optional[int] = None,
                          interactionTimeoutMs: typing.Optional[int] = None, busyWaitMs: typing.Optional[int] = None,
                          suppressResponse: typing.Optional[bool] = None,
                          payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD):
        '''
        Send a cluster-object encapsulated command to a node and get returned a future that can be awaited upon to receive
        the response. If a valid responseType is passed in, that will be used to de-serialize the object. If not,
        the type will be automatically deduced from the metadata received over the wire.

        timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
        interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the
                              right timeout value based on transport characteristics as well as the responsiveness of the target.

        Returns:
            - command response. The type of the response is defined by the command.
        Raises:
            - InteractionModelError on error
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = await self.GetConnectedDevice(nodeid, timeoutMs=interactionTimeoutMs, payloadCapability=payloadCapability)
        res = await ClusterCommand.SendCommand(
            future, eventLoop, responseType, device.deviceProxy, ClusterCommand.CommandPath(
                EndpointId=endpoint,
                ClusterId=payload.cluster_id,
                CommandId=payload.command_id,
            ), payload, timedRequestTimeoutMs=timedRequestTimeoutMs,
            interactionTimeoutMs=interactionTimeoutMs, busyWaitMs=busyWaitMs, suppressResponse=suppressResponse)
        res.raise_on_error()
        return await future

    async def SendBatchCommands(self, nodeid: int, commands: typing.List[ClusterCommand.InvokeRequestInfo],
                                timedRequestTimeoutMs: typing.Optional[int] = None,
                                interactionTimeoutMs: typing.Optional[int] = None, busyWaitMs: typing.Optional[int] = None,
                                suppressResponse: typing.Optional[bool] = None,
                                payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD):
        '''
        Send a batch of cluster-object encapsulated commands to a node and get returned a future that can be awaited upon to receive
        the responses. If a valid responseType is passed in, that will be used to de-serialize the object. If not,
        the type will be automatically deduced from the metadata received over the wire.

        nodeId: Target's Node ID
        commands: A list of InvokeRequestInfo containing the commands to invoke.
        timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
        interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the
                              right timeout value based on transport characteristics as well as the responsiveness of the target.
        busyWaitMs: How long to wait in ms after sending command to device before performing any other operations.
        suppressResponse: Do not send a response to this action

        Returns:
            - List of command responses in the same order as what was given in `commands`. The type of the response is defined by the command.
                      - A value of `None` indicates success.
                      - If only a single command fails, for example with `UNSUPPORTED_COMMAND`, the corresponding index associated with the command will,
                        contain `interaction_model.Status.UnsupportedCommand`.
                      - If a command is not responded to by server, command will contain `interaction_model.Status.NoCommandResponse`
        Raises:
            - InteractionModelError if error with sending of InvokeRequestMessage fails as a whole.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = await self.GetConnectedDevice(nodeid, timeoutMs=interactionTimeoutMs, payloadCapability=payloadCapability)

        res = await ClusterCommand.SendBatchCommands(
            future, eventLoop, device.deviceProxy, commands,
            timedRequestTimeoutMs=timedRequestTimeoutMs,
            interactionTimeoutMs=interactionTimeoutMs, busyWaitMs=busyWaitMs, suppressResponse=suppressResponse)
        res.raise_on_error()
        return await future

    def SendGroupCommand(self, groupid: int, payload: ClusterObjects.ClusterCommand, busyWaitMs: typing.Optional[int] = None):
        '''
        Send a group cluster-object encapsulated command to a group_id and get returned a future
        that can be awaited upon to get confirmation command was sent.
        Returns:
            - None: responses are not sent to group commands
        Raises:
            - InteractionModelError on error
        '''
        self.CheckIsActive()

        ClusterCommand.SendGroupCommand(
            groupid, self.devCtrl, payload, busyWaitMs=busyWaitMs).raise_on_error()

        # None is the expected return for sending group commands.
        return None

    async def WriteAttribute(self, nodeid: int,
                             attributes: typing.List[typing.Tuple[int, ClusterObjects.ClusterAttributeDescriptor]],
                             timedRequestTimeoutMs: typing.Optional[int] = None,
                             interactionTimeoutMs: typing.Optional[int] = None, busyWaitMs: typing.Optional[int] = None,
                             payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD):
        '''
        Write a list of attributes on a target node.

        nodeId: Target's Node ID
        timedWriteTimeoutMs: Timeout for a timed write request. Omit or set to 'None' to indicate a non-timed request.
        attributes: A list of tuples of type (endpoint, cluster-object):
        interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the
                              right timeout value based on transport characteristics as well as the responsiveness of the target.

        E.g
            (1, Clusters.UnitTesting.Attributes.XYZAttribute('hello')) -- Write 'hello'
            to the XYZ attribute on the test cluster to endpoint 1

        Returns:
            - [AttributeStatus] (list - one for each path)
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = await self.GetConnectedDevice(nodeid, timeoutMs=interactionTimeoutMs, payloadCapability=payloadCapability)

        attrs = []
        for v in attributes:
            if len(v) == 2:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    v[0], v[1], 0, 0, v[1].value))
            else:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    v[0], v[1], v[2], 1, v[1].value))

        ClusterAttribute.WriteAttributes(
            future, eventLoop, device.deviceProxy, attrs, timedRequestTimeoutMs=timedRequestTimeoutMs,
            interactionTimeoutMs=interactionTimeoutMs, busyWaitMs=busyWaitMs).raise_on_error()
        return await future

    def WriteGroupAttribute(
            self, groupid: int, attributes: typing.List[typing.Tuple[ClusterObjects.ClusterAttributeDescriptor, int]], busyWaitMs: typing.Optional[int] = None):
        '''
        Write a list of attributes on a target group.

        groupid: Group ID to send write attribute to.
        attributes: A list of tuples of type (cluster-object, data-version). The data-version can be omitted.

        E.g
            (Clusters.UnitTesting.Attributes.XYZAttribute('hello'), 1) -- Group Write 'hello' with data version 1
        '''
        self.CheckIsActive()

        attrs = []
        invalid_endpoint = 0xFFFF
        for v in attributes:
            if len(v) == 2:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    invalid_endpoint, v[0], v[1], 1, v[0].value))
            else:
                attrs.append(ClusterAttribute.AttributeWriteRequest(
                    invalid_endpoint, v[0], 0, 0, v[0].value))

        ClusterAttribute.WriteGroupAttributes(
            groupid, self.devCtrl, attrs, busyWaitMs=busyWaitMs).raise_on_error()

        # An empty list is the expected return for sending group write attribute.
        return []

    def TestOnlyPrepareToReceiveBdxData(self) -> asyncio.Future:
        '''
        Sets up the system to expect a node to initiate a BDX transfer. The transfer will send data here.

        Returns:
            - a future that will yield a BdxTransfer with the init message from the transfer.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        Bdx.PrepareToReceiveBdxData(future).raise_on_error()
        return future

    def TestOnlyPrepareToSendBdxData(self, data: bytes) -> asyncio.Future:
        '''
        Sets up the system to expect a node to initiate a BDX transfer. The transfer will send data to the node.

        Returns:
            - a future that will yield a BdxTransfer with the init message from the transfer.
        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        Bdx.PrepareToSendBdxData(future, data).raise_on_error()
        return future

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
        typing.Tuple[int, typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Directly specified attribute path
        ClusterAttribute.AttributePath
    ]):
        if isinstance(pathTuple, ClusterAttribute.AttributePath):
            return pathTuple
        if pathTuple == ('*') or pathTuple == ():
            # Wildcard
            return ClusterAttribute.AttributePath()
        elif not isinstance(pathTuple, tuple):
            if isinstance(pathTuple, int):
                return ClusterAttribute.AttributePath(EndpointId=pathTuple)
            elif issubclass(pathTuple, ClusterObjects.Cluster):
                return ClusterAttribute.AttributePath.from_cluster(EndpointId=None, Cluster=pathTuple)
            elif issubclass(pathTuple, ClusterObjects.ClusterAttributeDescriptor):
                return ClusterAttribute.AttributePath.from_attribute(EndpointId=None, Attribute=pathTuple)
            else:
                raise ValueError("Unsupported Attribute Path")
        else:
            # endpoint + (cluster) attribute / endpoint + cluster
            if issubclass(pathTuple[1], ClusterObjects.Cluster):
                return ClusterAttribute.AttributePath.from_cluster(EndpointId=pathTuple[0], Cluster=pathTuple[1])
            elif issubclass(pathTuple[1], ClusterAttribute.ClusterAttributeDescriptor):
                return ClusterAttribute.AttributePath.from_attribute(EndpointId=pathTuple[0], Attribute=pathTuple[1])
            else:
                raise ValueError("Unsupported Attribute Path")

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
        return ClusterAttribute.DataVersionFilter.from_cluster(
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
        if pathTuple in [('*'), ()]:
            # Wildcard
            return ClusterAttribute.EventPath()
        elif not isinstance(pathTuple, tuple):
            if isinstance(pathTuple, int):
                return ClusterAttribute.EventPath(EndpointId=pathTuple)
            elif issubclass(pathTuple, ClusterObjects.Cluster):
                return ClusterAttribute.EventPath.from_cluster(EndpointId=None, Cluster=pathTuple)
            elif issubclass(pathTuple, ClusterObjects.ClusterEvent):
                return ClusterAttribute.EventPath.from_event(EndpointId=None, Event=pathTuple)
            else:
                raise ValueError("Unsupported Event Path")
        else:
            if pathTuple[0] == '*':
                return ClusterAttribute.EventPath(Urgent=pathTuple[-1])
            else:
                urgent = bool(pathTuple[-1]) if len(pathTuple) > 2 else False
                # endpoint + (cluster) event / endpoint + cluster
                if issubclass(pathTuple[1], ClusterObjects.Cluster):
                    return ClusterAttribute.EventPath.from_cluster(EndpointId=pathTuple[0], Cluster=pathTuple[1], Urgent=urgent)
                elif issubclass(pathTuple[1], ClusterAttribute.ClusterEvent):
                    return ClusterAttribute.EventPath.from_event(EndpointId=pathTuple[0], Event=pathTuple[1], Urgent=urgent)
                else:
                    raise ValueError("Unsupported Attribute Path")

    async def Read(
        self,
        nodeid: int,
        attributes: typing.Optional[typing.List[typing.Union[
            None,  # Empty tuple, all wildcard
            typing.Tuple[int],  # Endpoint
            # Wildcard endpoint, Cluster id present
            typing.Tuple[typing.Type[ClusterObjects.Cluster]],
            # Wildcard endpoint, Cluster + Attribute present
            typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
            # Wildcard attribute id
            typing.Tuple[int, typing.Type[ClusterObjects.Cluster]],
            # Concrete path
            typing.Tuple[int, typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
            # Directly specified attribute path
            ClusterAttribute.AttributePath
        ]]] = None,
        dataVersionFilters: typing.Optional[typing.List[typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int]]] = None, events: typing.Optional[typing.List[
            typing.Union[
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
            ]]] = None,
        eventNumberFilter: typing.Optional[int] = None,
        returnClusterObject: bool = False, reportInterval: typing.Optional[typing.Tuple[int, int]] = None,
        fabricFiltered: bool = True, keepSubscriptions: bool = False, autoResubscribe: bool = True,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD
    ):
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

            An AttributePath can also be specified directly by [chip.cluster.Attribute.AttributePath(...)]

        dataVersionFilters: A list of tuples of (endpoint, cluster, data version).

        events: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,
                                                                Cluster = specific,   Event = specific, Urgent = True/False
            (endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,
                                                                Cluster = specific,   Event = *, Urgent = True/False
            (Clusters.ClusterA.EventA, urgent):                 Endpoint = *,
                                                                Cluster = specific,   Event = specific, Urgent = True/False
            endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
            Clusters.ClusterA:                          Endpoint = *,          Cluster = specific,    Event = *, Urgent = True/False
            '*' or ():                                  Endpoint = *,          Cluster = *,          Event = *, Urgent = True/False

        eventNumberFilter: Optional minimum event number filter.

        returnClusterObject: This returns the data as consolidated cluster objects, with all attributes for a cluster inside
                             a single cluster-wide cluster object.

        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        fabricFiltered: If True (default), the read/subscribe is fabric-filtered and will only see things associated with the fabric
            of the reader/subscriber. Relevant for attributes with fabric-scoped data.
        keepSubscriptions: Keep existing subscriptions. If set to False, existing subscriptions with this node will get cancelled
            and a new one gets setup.
        autoResubscribe: Automatically resubscribe to the subscription if subscription is lost. The automatic re-subscription only
            applies if the subscription establishes on first try. If the first subscription establishment attempt fails the function
            returns right away.

        Returns:
            - AsyncReadTransaction.ReadResponse. Please see ReadAttribute and ReadEvent for examples of how to access data.

        Raises:
            - InteractionModelError (chip.interaction_model) on error

        '''
        self.CheckIsActive()

        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()

        device = await self.GetConnectedDevice(nodeid, payloadCapability=payloadCapability)
        attributePaths = [self._parseAttributePathTuple(
            v) for v in attributes] if attributes else None
        clusterDataVersionFilters = [self._parseDataVersionFilterTuple(
            v) for v in dataVersionFilters] if dataVersionFilters else None
        eventPaths = [self._parseEventPathTuple(
            v) for v in events] if events else None

        transaction = ClusterAttribute.AsyncReadTransaction(future, eventLoop, self, returnClusterObject)
        ClusterAttribute.Read(transaction, device=device.deviceProxy,
                              attributes=attributePaths, dataVersionFilters=clusterDataVersionFilters, events=eventPaths,
                              eventNumberFilter=eventNumberFilter,
                              subscriptionParameters=ClusterAttribute.SubscriptionParameters(
                                  reportInterval[0], reportInterval[1]) if reportInterval else None,
                              fabricFiltered=fabricFiltered,
                              keepSubscriptions=keepSubscriptions, autoResubscribe=autoResubscribe).raise_on_error()
        await future

        if result := transaction.GetSubscriptionHandler():
            return result
        return transaction.GetReadResponse()

    async def ReadAttribute(
        self,
        nodeid: int,
        attributes: typing.Optional[typing.List[typing.Union[
            None,  # Empty tuple, all wildcard
            typing.Tuple[int],  # Endpoint
            # Wildcard endpoint, Cluster id present
            typing.Tuple[typing.Type[ClusterObjects.Cluster]],
            # Wildcard endpoint, Cluster + Attribute present
            typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
            # Wildcard attribute id
            typing.Tuple[int, typing.Type[ClusterObjects.Cluster]],
            # Concrete path
            typing.Tuple[int, typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
            # Directly specified attribute path
            ClusterAttribute.AttributePath
        ]]], dataVersionFilters: typing.Optional[typing.List[typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int]]] = None,
        returnClusterObject: bool = False,
        reportInterval: typing.Optional[typing.Tuple[int, int]] = None,
        fabricFiltered: bool = True, keepSubscriptions: bool = False, autoResubscribe: bool = True,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD
    ):
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

            An AttributePath can also be specified directly by [chip.cluster.Attribute.AttributePath(...)]

        returnClusterObject: This returns the data as consolidated cluster objects, with all attributes for a cluster inside
                             a single cluster-wide cluster object.

        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        fabricFiltered: If True (default), the read/subscribe is fabric-filtered and will only see things associated with the fabric
            of the reader/subscriber. Relevant for attributes with fabric-scoped data.
        keepSubscriptions: Keep existing subscriptions. If set to False, existing subscriptions with this node will get cancelled
            and a new one gets setup.
        autoResubscribe: Automatically resubscribe to the subscription if subscription is lost. The automatic re-subscription only
            applies if the subscription establishes on first try. If the first subscription establishment attempt fails the function
            returns right away.

        Returns:
            - subscription request: ClusterAttribute.SubscriptionTransaction
                                    To get notified on attribute change use SetAttributeUpdateCallback on the returned
                                    SubscriptionTransaction. This is used to set a callback function, which is a callable of
                                    type Callable[[TypedAttributePath, SubscriptionTransaction], None]
                                    Get the attribute value from the change path using GetAttribute on the SubscriptionTransaction
                                    You can await changes in the main loop using a trigger mechanism from the callback.
                                    ex. queue.SimpleQueue

            - read request: AsyncReadTransaction.ReadResponse.attributes.
                            This is of type AttributeCache.attributeCache (Attribute.py),
                            which is a dict mapping endpoints to a list of Cluster (ClusterObjects.py) classes
                            (dict[int, List[Cluster]])
                            Access as returned_object[endpoint_id][<Cluster class>][<Attribute class>]
                            Ex. To access the OnTime attribute from the OnOff cluster on endpoint 1
                            returned_object[1][Clusters.OnOff][Clusters.OnOff.Attributes.OnTime]

        Raises:
            - InteractionModelError (chip.interaction_model) on error
        '''
        res = await self.Read(nodeid,
                              attributes=attributes,
                              dataVersionFilters=dataVersionFilters,
                              returnClusterObject=returnClusterObject,
                              reportInterval=reportInterval,
                              fabricFiltered=fabricFiltered,
                              keepSubscriptions=keepSubscriptions,
                              autoResubscribe=autoResubscribe,
                              payloadCapability=payloadCapability)
        if isinstance(res, ClusterAttribute.SubscriptionTransaction):
            return res
        else:
            return res.attributes

    async def ReadEvent(
        self,
        nodeid: int,
        events: typing.List[typing.Union[
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
        ]], eventNumberFilter: typing.Optional[int] = None,
        fabricFiltered: bool = True,
        reportInterval: typing.Optional[typing.Tuple[int, int]] = None,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD
    ):
        '''
        Read a list of events from a target node, this is a wrapper of DeviceController.Read()

        nodeId: Target's Node ID
        events: A list of tuples of varying types depending on the type of read being requested:
            (endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,
                                                                Cluster = specific,   Event = specific, Urgent = True/False
            (endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,
                                                                Cluster = specific,   Event = *, Urgent = True/False
            (Clusters.ClusterA.EventA, urgent):                 Endpoint = *,
                                                                Cluster = specific,   Event = specific, Urgent = True/False
            endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
            Clusters.ClusterA:                          Endpoint = *,          Cluster = specific,    Event = *, Urgent = True/False
            '*' or ():                                  Endpoint = *,          Cluster = *,          Event = *, Urgent = True/False

        The cluster and events specified above are to be selected from the generated cluster objects.

        e.g.
            ReadEvent(1, [ 1 ] ) -- case 4 above.
            ReadEvent(1, [ Clusters.BasicInformation ] ) -- case 5 above.
            ReadEvent(1, [ (1, Clusters.BasicInformation.Events.Location ] ) -- case 1 above.

        eventNumberFilter: Optional minimum event number filter.
        reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
            When not provided, a read request will be sent.
        keepSubscriptions: Keep existing subscriptions. If set to False, existing subscriptions with this node will get cancelled
            and a new one gets setup.
        autoResubscribe: Automatically resubscribe to the subscription if subscription is lost. The automatic re-subscription only
            applies if the subscription establishes on first try. If the first subscription establishment attempt fails the function
            returns right away.

        Returns:
            - subscription request: ClusterAttribute.SubscriptionTransaction
                                    To get notified on event subscriptions, use the SetEventUpdateCallback function on the
                                    returned  SubscriptionTransaction. This is a callable of type
                                    Callable[[EventReadResult, SubscriptionTransaction], None]
                                    You can await events using a trigger mechanism in the callback. ex. queue.SimpleQueue

            - read request: AsyncReadTransaction.ReadResponse.events.
                            This is a List[ClusterEvent].

        Raises:
            - InteractionModelError (chip.interaction_model) on error
        '''
        res = await self.Read(nodeid=nodeid, events=events, eventNumberFilter=eventNumberFilter,
                              fabricFiltered=fabricFiltered, reportInterval=reportInterval, keepSubscriptions=keepSubscriptions,
                              autoResubscribe=autoResubscribe, payloadCapability=payloadCapability)
        if isinstance(res, ClusterAttribute.SubscriptionTransaction):
            return res
        else:
            return res.events

    def SetIpk(self, ipk: bytes):
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetIpk(self.devCtrl, ipk, len(ipk))
        ).raise_on_error()

    def InitGroupTestingData(self):
        """Populates the Device Controller's GroupDataProvider with known test group info and keys."""
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_OpCreds_InitGroupTestingData(
                self.devCtrl)
        ).raise_on_error()

    def CreateManualCode(self, discriminator: int, passcode: int) -> str:
        """ Creates a standard flow manual code from the given discriminator and passcode."""
        # 64 bytes is WAY more than required, but let's be safe
        in_size = 64
        out_size = c_size_t(0)
        buf = create_string_buffer(in_size)
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_CreateManualCode(discriminator, passcode, buf, in_size, pointer(out_size))
        ).raise_on_error()
        if out_size.value == 0 or out_size.value > in_size:
            raise MemoryError("Invalid output size for manual code")
        return buf.value.decode()

    # ----- Private Members -----
    def _InitLib(self):
        if self._dmLib is None:
            self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

            self._dmLib.pychip_DeviceController_DeleteDeviceController.argtypes = [
                c_void_p, c_void_p]
            self._dmLib.pychip_DeviceController_DeleteDeviceController.restype = PyChipError

            self._dmLib.pychip_DeviceController_ConnectBLE.argtypes = [
                c_void_p, c_uint16, c_bool, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectBLE.restype = PyChipError

            self._dmLib.pychip_DeviceController_SetThreadOperationalDataset.argtypes = [
                c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_SetThreadOperationalDataset.restype = PyChipError

            self._dmLib.pychip_DeviceController_SetWiFiCredentials.argtypes = [
                c_char_p, c_char_p]
            self._dmLib.pychip_DeviceController_SetWiFiCredentials.restype = PyChipError

            # Currently only supports 1 list item
            self._dmLib.pychip_DeviceController_SetTimeZone.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetTimeZone.argtypes = [
                c_int32, c_uint64, c_char_p]

            # Currently only supports 1 list item
            self._dmLib.pychip_DeviceController_SetDSTOffset.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetDSTOffset.argtypes = [
                c_int32, c_uint64, c_uint64]

            self._dmLib.pychip_DeviceController_SetDefaultNtp.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetDefaultNtp.argtypes = [c_char_p]

            self._dmLib.pychip_DeviceController_SetTrustedTimeSource.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetTrustedTimeSource.argtypes = [c_uint64, c_uint16]

            self._dmLib.pychip_DeviceController_SetCheckMatchingFabric.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetCheckMatchingFabric.argtypes = [c_bool]

            self._dmLib.pychip_DeviceController_SetIcdRegistrationParameters.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetIcdRegistrationParameters.argtypes = [
                c_bool, c_void_p
            ]

            self._dmLib.pychip_DeviceController_ResetCommissioningParameters.restype = PyChipError
            self._dmLib.pychip_DeviceController_ResetCommissioningParameters.argtypes = []

            self._dmLib.pychip_DeviceController_Commission.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_Commission.restype = PyChipError

            self._dmLib.pychip_DeviceController_OnNetworkCommission.argtypes = [
                c_void_p, c_void_p, c_uint64, c_uint32, c_uint8, c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_OnNetworkCommission.restype = PyChipError

            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodes.argtypes = [
                c_void_p, c_uint8, c_char_p]
            self._dmLib.pychip_DeviceController_DiscoverCommissionableNodes.restype = PyChipError

            self._dmLib.pychip_DeviceController_StopCommissionableDiscovery.argtypes = [
                c_void_p]
            self._dmLib.pychip_DeviceController_StopCommissionableDiscovery.restype = PyChipError

            self._dmLib.pychip_DeviceController_EstablishPASESessionIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64, c_uint16]
            self._dmLib.pychip_DeviceController_EstablishPASESessionIP.restype = PyChipError

            self._dmLib.pychip_DeviceController_EstablishPASESessionBLE.argtypes = [
                c_void_p, c_uint32, c_uint16, c_uint64]
            self._dmLib.pychip_DeviceController_EstablishPASESessionBLE.restype = PyChipError

            self._dmLib.pychip_DeviceController_EstablishPASESession.argtypes = [
                c_void_p, c_char_p, c_uint64]
            self._dmLib.pychip_DeviceController_EstablishPASESession.restype = PyChipError

            self._dmLib.pychip_DeviceController_HasDiscoveredCommissionableNode.argtypes = [c_void_p]
            self._dmLib.pychip_DeviceController_HasDiscoveredCommissionableNode.restype = c_bool

            self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice.argtypes = [
                c_void_p, c_int, c_char_p, c_uint32]
            self._dmLib.pychip_DeviceController_GetIPForDiscoveredDevice.restype = c_bool

            self._dmLib.pychip_DeviceController_ConnectIP.argtypes = [
                c_void_p, c_char_p, c_uint32, c_uint64]
            self._dmLib.pychip_DeviceController_ConnectIP.restype = PyChipError

            self._dmLib.pychip_DeviceController_ConnectWithCode.argtypes = [
                c_void_p, c_char_p, c_uint64, c_uint8]
            self._dmLib.pychip_DeviceController_ConnectWithCode.restype = PyChipError

            self._dmLib.pychip_DeviceController_UnpairDevice.argtypes = [
                c_void_p, c_uint64, _DeviceUnpairingCompleteFunct]
            self._dmLib.pychip_DeviceController_UnpairDevice.restype = PyChipError

            self._dmLib.pychip_DeviceController_MarkSessionDefunct.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_MarkSessionDefunct.restype = PyChipError

            self._dmLib.pychip_DeviceController_MarkSessionForEviction.argtypes = [
                c_void_p, c_uint64]
            self._dmLib.pychip_DeviceController_MarkSessionForEviction.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetAddressAndPort.argtypes = [
                c_void_p, c_uint64, c_char_p, c_uint64, POINTER(c_uint16)]
            self._dmLib.pychip_DeviceController_GetAddressAndPort.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnPairingCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnCommissioningCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetOpenWindowCompleteCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnOpenWindowCompleteFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetOpenWindowCompleteCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnCommissioningStatusUpdateFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetFabricCheckCallback.argtypes = [
                c_void_p, _DevicePairingDelegate_OnFabricCheckFunct]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetFabricCheckCallback.restype = PyChipError

            self._dmLib.pychip_ScriptDevicePairingDelegate_SetExpectingPairingComplete.argtypes = [
                c_void_p, c_bool]
            self._dmLib.pychip_ScriptDevicePairingDelegate_SetExpectingPairingComplete.restype = PyChipError

            self._dmLib.pychip_GetConnectedDeviceByNodeId.argtypes = [
                c_void_p, c_uint64, py_object, _DeviceAvailableCallbackFunct, c_int]
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
                c_void_p, c_void_p, c_uint64, c_uint16, c_uint32, c_uint16, c_uint8]
            self._dmLib.pychip_DeviceController_OpenCommissioningWindow.restype = PyChipError

            self._dmLib.pychip_TestCommissionerUsed.argtypes = []
            self._dmLib.pychip_TestCommissionerUsed.restype = c_bool

            self._dmLib.pychip_TestCommissioningCallbacks.argtypes = []
            self._dmLib.pychip_TestCommissioningCallbacks.restype = c_bool

            self._dmLib.pychip_TestCommissioningStageSuccessful.argtypes = [c_uint8]
            self._dmLib.pychip_TestCommissioningStageSuccessful.restype = c_bool

            self._dmLib.pychip_ResetCommissioningTests.argtypes = []
            self._dmLib.pychip_TestPaseConnection.argtypes = [c_uint64]

            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnStage.argtypes = [
                c_uint8]
            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnStage.restype = c_bool

            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnReport.argtypes = [
                c_uint8]
            self._dmLib.pychip_SetTestCommissionerSimulateFailureOnReport.restype = c_bool

            self._dmLib.pychip_SetTestCommissionerPrematureCompleteAfter.argtypes = [
                c_uint8]
            self._dmLib.pychip_SetTestCommissionerPrematureCompleteAfter.restype = c_bool

            self._dmLib.pychip_GetCompletionError.argtypes = []
            self._dmLib.pychip_GetCompletionError.restype = PyChipError

            self._dmLib.pychip_GetCommissioningRCACData.argtypes = [ctypes.POINTER(
                ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t), ctypes.c_size_t]
            self._dmLib.pychip_GetCommissioningRCACData.restype = None

            self._dmLib.pychip_DeviceController_IssueNOCChain.argtypes = [
                c_void_p, py_object, c_char_p, c_size_t, c_uint64]
            self._dmLib.pychip_DeviceController_IssueNOCChain.restype = PyChipError

            self._dmLib.pychip_OpCreds_InitGroupTestingData.argtypes = [
                c_void_p]
            self._dmLib.pychip_OpCreds_InitGroupTestingData.restype = PyChipError

            self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback.argtypes = [
                _IssueNOCChainCallbackPythonCallbackFunct]
            self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback.restype = None

            self._dmLib.pychip_DeviceController_GetNodeId.argtypes = [c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetNodeId.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetFabricId.argtypes = [c_void_p, POINTER(c_uint64)]
            self._dmLib.pychip_DeviceController_GetFabricId.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetFabricIndex.argtypes = [c_void_p, POINTER(c_uint8)]
            self._dmLib.pychip_DeviceController_GetFabricIndex.restype = PyChipError

            self._dmLib.pychip_DeviceController_GetLogFilter = [None]
            self._dmLib.pychip_DeviceController_GetLogFilter = c_uint8

            self._dmLib.pychip_OpCreds_AllocateController.argtypes = [c_void_p, POINTER(
                c_void_p), POINTER(c_void_p), c_uint64, c_uint64, c_uint16, c_char_p, c_bool, c_bool, POINTER(c_uint32), c_uint32, c_void_p]
            self._dmLib.pychip_OpCreds_AllocateController.restype = PyChipError

            self._dmLib.pychip_OpCreds_AllocateControllerForPythonCommissioningFLow.argtypes = [
                POINTER(c_void_p), POINTER(c_void_p), c_void_p, POINTER(c_char), c_uint32, POINTER(c_char), c_uint32, POINTER(c_char), c_uint32, POINTER(c_char), c_uint32, c_uint16, c_bool]
            self._dmLib.pychip_OpCreds_AllocateControllerForPythonCommissioningFLow.restype = PyChipError

            self._dmLib.pychip_DeviceController_SetIpk.argtypes = [c_void_p, POINTER(c_char), c_size_t]
            self._dmLib.pychip_DeviceController_SetIpk.restype = PyChipError

            self._dmLib.pychip_CheckInDelegate_SetOnCheckInCompleteCallback.restype = None
            self._dmLib.pychip_CheckInDelegate_SetOnCheckInCompleteCallback.argtypes = [_OnCheckInCompleteFunct]

            self._dmLib.pychip_CheckInDelegate_SetOnCheckInCompleteCallback(_OnCheckInComplete)

            self._dmLib.pychip_DeviceProxy_GetRemoteSessionParameters.restype = PyChipError
            self._dmLib.pychip_DeviceProxy_GetRemoteSessionParameters.argtypes = [c_void_p, c_char_p]

            self._dmLib.pychip_CreateManualCode.restype = PyChipError
            self._dmLib.pychip_CreateManualCode.argtypes = [c_uint16, c_uint32, c_char_p, c_size_t, POINTER(c_size_t)]

            self._dmLib.pychip_DeviceController_SetSkipCommissioningComplete.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetSkipCommissioningComplete.argtypes = [c_bool]

            self._dmLib.pychip_DeviceController_SetTermsAcknowledgements.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetTermsAcknowledgements.argtypes = [c_uint16, c_uint16]

            self._dmLib.pychip_DeviceController_SetDACRevocationSetPath.restype = PyChipError
            self._dmLib.pychip_DeviceController_SetDACRevocationSetPath.argtypes = [c_char_p]


class ChipDeviceController(ChipDeviceControllerBase):
    ''' The ChipDeviceCommissioner binding, named as ChipDeviceController

    TODO: This class contains DEPRECATED functions, we should update the test scripts to avoid the usage of those functions.
    '''

    def __init__(self, opCredsContext: ctypes.c_void_p, fabricId: int, nodeId: int, adminVendorId: int, catTags: typing.List[int] = [
    ], paaTrustStorePath: str = "", useTestCommissioner: bool = False, fabricAdmin: typing.Optional[FabricAdmin.FabricAdmin] = None, name: str = '', keypair: typing.Optional[p256keypair.P256Keypair] = None):
        super().__init__(
            name or
            f"caIndex({fabricAdmin.caIndex:x})/fabricId(0x{fabricId:016X})/nodeId(0x{nodeId:016X})"
        )

        self._issue_node_chain_context: CallbackContext = CallbackContext(asyncio.Lock())
        self._dmLib.pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback(_IssueNOCChainCallbackPythonCallback)

        pairingDelegate = c_void_p(None)
        devCtrl = c_void_p(None)

        c_catTags = (c_uint32 * len(catTags))()

        for i, item in enumerate(catTags):
            c_catTags[i] = item

        # TODO(erjiaqing@): Figure out how to control enableServerInteractions for a single device controller (node)
        self._externalKeyPair = keypair
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_OpCreds_AllocateController(cast(
                opCredsContext, c_void_p), pointer(devCtrl), pointer(pairingDelegate), fabricId, nodeId, adminVendorId, c_char_p(None if len(paaTrustStorePath) == 0 else str.encode(paaTrustStorePath)), useTestCommissioner, self._ChipStack.enableServerInteractions, c_catTags, len(catTags), None if keypair is None else keypair.native_object)
        ).raise_on_error()

        self._fabricAdmin = fabricAdmin
        self._fabricId = fabricId
        self._nodeId = nodeId
        self._caIndex = fabricAdmin.caIndex

        self._set_dev_ctrl(devCtrl=devCtrl, pairingDelegate=pairingDelegate)

        self._finish_init()

        assert self._fabricId == fabricId
        assert self._nodeId == nodeId

    @property
    def caIndex(self) -> int:
        return self._caIndex

    @property
    def fabricAdmin(self) -> FabricAdmin.FabricAdmin:
        return self._fabricAdmin

    async def Commission(self, nodeid) -> int:
        '''
        Start the auto-commissioning process on a node after establishing a PASE connection.
        This function is intended to be used in conjunction with `EstablishPASESessionBLE` or
        `EstablishPASESessionIP`. It can be called either before or after the DevicePairingDelegate
        receives the OnPairingComplete call. Commissioners that want to perform simple
        auto-commissioning should use the supplied "CommissionWithCode" function, which will
        establish the PASE connection and commission automatically.

        Raises a ChipStackError on failure.

        Returns:
            - Effective Node ID of the device (as defined by the assigned NOC)
        '''
        self.CheckIsActive()

        async with self._commissioning_context as ctx:
            self._enablePairingCompleteCallback(False)
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_Commission(
                    self.devCtrl, nodeid)
            )

            return await asyncio.futures.wrap_future(ctx.future)

    async def CommissionThread(self, discriminator, setupPinCode, nodeId, threadOperationalDataset: bytes, isShortDiscriminator: bool = False) -> int:
        ''' Commissions a Thread device over BLE
        '''
        self.SetThreadOperationalDataset(threadOperationalDataset)
        return await self.ConnectBLE(discriminator, setupPinCode, nodeId, isShortDiscriminator)

    async def CommissionWiFi(self, discriminator, setupPinCode, nodeId, ssid: str, credentials: str, isShortDiscriminator: bool = False) -> int:
        ''' Commissions a Wi-Fi device over BLE.
        '''
        self.SetWiFiCredentials(ssid, credentials)
        return await self.ConnectBLE(discriminator, setupPinCode, nodeId, isShortDiscriminator)

    def SetWiFiCredentials(self, ssid: str, credentials: str):
        ''' Set the Wi-Fi credentials to set during commissioning.'''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetWiFiCredentials(
                ssid.encode("utf-8"), credentials.encode("utf-8"))
        ).raise_on_error()

    def SetThreadOperationalDataset(self, threadOperationalDataset):
        ''' Set the Thread operational dataset to set during commissioning.'''
        self.CheckIsActive()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetThreadOperationalDataset(
                threadOperationalDataset, len(threadOperationalDataset))
        ).raise_on_error()

    def ResetCommissioningParameters(self):
        ''' Sets the commissioning parameters back to the default values.'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_ResetCommissioningParameters()
        ).raise_on_error()

    def SetTimeZone(self, offset: int, validAt: int, name: str = ""):
        ''' Set the time zone to set during commissioning. Currently only one time zone entry is supported'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetTimeZone(offset, validAt, name.encode("utf-8"))
        ).raise_on_error()

    def SetDSTOffset(self, offset: int, validStarting: int, validUntil: int):
        ''' Set the DST offset to set during commissioning. Currently only one DST entry is supported'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetDSTOffset(offset, validStarting, validUntil)
        ).raise_on_error()

    def SetTCAcknowledgements(self, tcAcceptedVersion: int, tcUserResponse: int):
        ''' Set the TC acknowledgements to set during commissioning'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetTermsAcknowledgements(tcAcceptedVersion, tcUserResponse)
        ).raise_on_error()

    def SetSkipCommissioningComplete(self, skipCommissioningComplete: bool):
        ''' Set whether to skip the commissioning complete callback'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetSkipCommissioningComplete(skipCommissioningComplete)
        ).raise_on_error()

    def SetDefaultNTP(self, defaultNTP: str):
        ''' Set the DefaultNTP to set during commissioning'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetDefaultNtp(defaultNTP.encode("utf-8"))
        ).raise_on_error()

    def SetTrustedTimeSource(self, nodeId: int, endpoint: int):
        ''' Set the trusted time source nodeId to set during commissioning. This must be a node on the commissioner fabric.'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetTrustedTimeSource(nodeId, endpoint)
        ).raise_on_error()

    def SetCheckMatchingFabric(self, check: bool):
        ''' Instructs the auto-commissioner to perform a matching fabric check before commissioning.'''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetCheckMatchingFabric(check)
        ).raise_on_error()

    def GenerateICDRegistrationParameters(self):
        ''' Generates ICD registration parameters for this controller. '''
        return ICDRegistrationParameters(
            secrets.token_bytes(16),
            self._nodeId,
            self._nodeId,
            30,
            Clusters.IcdManagement.Enums.ClientTypeEnum.kPermanent)

    def EnableICDRegistration(self, parameters: ICDRegistrationParameters):
        ''' Enables ICD registration for the following commissioning session.

        Args:
            parameters: A ICDRegistrationParameters for the parameters used for ICD registration, or None for default arguments.
        '''
        if parameters is None:
            raise ValueError("ICD registration parameter required.")
        if len(parameters.symmetricKey) != 16:
            raise ValueError("symmetricKey should be 16 bytes")

        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetIcdRegistrationParameters(
                True, pointer(parameters.to_c()))
        ).raise_on_error()

    def DisableICDRegistration(self):
        ''' Disables ICD registration. '''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetIcdRegistrationParameters(False, None)
        ).raise_on_error()

    def GetFabricCheckResult(self) -> int:
        ''' Returns the fabric check result if SetCheckMatchingFabric was used.'''
        return self._fabricCheckNodeId

    async def CommissionOnNetwork(self, nodeId: int, setupPinCode: int,
                                  filterType: DiscoveryFilterType = DiscoveryFilterType.NONE, filter: typing.Any = None,
                                  discoveryTimeoutMsec: int = 30000) -> int:
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

        Raises a ChipStackError on failure.

        Returns:
            - Effective Node ID of the device (as defined by the assigned NOC)
        '''
        self.CheckIsActive()

        # Convert numerical filters to string for passing down to binding.
        if isinstance(filter, int):
            filter = str(filter)

        async with self._commissioning_context as ctx:
            self._enablePairingCompleteCallback(True)
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_OnNetworkCommission(
                    self.devCtrl, self.pairingDelegate, nodeId, setupPinCode, int(filterType), str(filter).encode("utf-8") if filter is not None else None, discoveryTimeoutMsec)
            )

            return await asyncio.futures.wrap_future(ctx.future)

    def get_rcac(self):
        '''
        Passes captured RCAC data back to Python test modules for validation
        - Setting buffer size to max size mentioned in spec:
        - Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/06c4d55962954546ecf093c221fe1dab57645028/policies/matter_certificate_policy.adoc#615-key-sizes
        '''
        rcac_size = 400
        rcac_buffer = (ctypes.c_uint8 * rcac_size)()  # Allocate buffer

        actual_rcac_size = ctypes.c_size_t()

        # Now calling the C++ function with the buffer size set as an additional parameter
        self._dmLib.pychip_GetCommissioningRCACData(
            ctypes.cast(rcac_buffer, ctypes.POINTER(ctypes.c_uint8)),
            ctypes.byref(actual_rcac_size),
            ctypes.c_size_t(rcac_size)  # Pass the buffer size
        )

        # Check if data is available
        if actual_rcac_size.value > 0:
            # Convert the data to a Python bytes object
            rcac_data = bytearray(rcac_buffer[:actual_rcac_size.value])
            rcac_bytes = bytes(rcac_data)
        else:
            LOGGER.exception("RCAC returned from C++ did not contain any data")
            return None
        return rcac_bytes

    async def CommissionWithCode(self, setupPayload: str, nodeid: int, discoveryType: DiscoveryType = DiscoveryType.DISCOVERY_ALL) -> int:
        ''' Commission with the given nodeid from the setupPayload.
            setupPayload may be a QR or manual code.

            Raises a ChipStackError on failure.

        Returns:
            - Effective Node ID of the device (as defined by the assigned NOC)
        '''
        self.CheckIsActive()

        async with self._commissioning_context as ctx:
            self._enablePairingCompleteCallback(True)
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_ConnectWithCode(
                    self.devCtrl, setupPayload.encode("utf-8"), nodeid, discoveryType.value)
            )

            return await asyncio.futures.wrap_future(ctx.future)

    async def CommissionIP(self, ipaddr: str, setupPinCode: int, nodeid: int) -> int:
        """ DEPRECATED, DO NOT USE! Use `CommissionOnNetwork` or `CommissionWithCode`

        Raises a ChipStackError on failure.

        Returns:
            - Effective Node ID of the device (as defined by the assigned NOC)
        """
        self.CheckIsActive()

        async with self._commissioning_context as ctx:
            self._enablePairingCompleteCallback(True)
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_ConnectIP(
                    self.devCtrl, ipaddr.encode("utf-8"), setupPinCode, nodeid)
            )

            return await asyncio.futures.wrap_future(ctx.future)

    def NOCChainCallback(self, nocChain):
        if self._issue_node_chain_context.future is None:
            LOGGER.exception("NOCChainCallback while not expecting a callback")
            return
        self._issue_node_chain_context.future.set_result(nocChain)
        return

    async def IssueNOCChain(self, csr: Clusters.OperationalCredentials.Commands.CSRResponse, nodeId: int):
        """Issue an NOC chain using the associated OperationalCredentialsDelegate.
        The NOC chain will be provided in TLV cert format."""
        self.CheckIsActive()

        async with self._issue_node_chain_context as ctx:
            await self._ChipStack.CallAsync(
                lambda: self._dmLib.pychip_DeviceController_IssueNOCChain(
                    self.devCtrl, py_object(self), csr.NOCSRElements, len(csr.NOCSRElements), nodeId)
            )

            return await asyncio.futures.wrap_future(ctx.future)

    def SetDACRevocationSetPath(self, dacRevocationSetPath: typing.Optional[str]):
        ''' Set the path to the device attestation revocation set JSON file.

        Args:
            dacRevocationSetPath: Path to the JSON file containing the device attestation revocation set
        '''
        self.CheckIsActive()
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_DeviceController_SetDACRevocationSetPath(
                c_char_p(str.encode(dacRevocationSetPath) if dacRevocationSetPath else None))
        ).raise_on_error()


class BareChipDeviceController(ChipDeviceControllerBase):
    ''' A bare device controller without AutoCommissioner support.
    '''

    def __init__(self, operationalKey: p256keypair.P256Keypair, noc: bytes,
                 icac: typing.Union[bytes, None], rcac: bytes, ipk: typing.Union[bytes, None], adminVendorId: int, name: typing.Optional[str] = None):
        '''Creates a controller without AutoCommissioner.

        The allocated controller uses the noc, icac, rcac and ipk instead of the default,
        random generated certificates / keys. Which is suitable for creating a controller
        for manually signing certificates for testing.

        Args:
            operationalKey: A P256Keypair object for the operational key of the controller.
            noc: The NOC for the controller, in bytes.
            icac: The optional ICAC for the controller.
            rcac: The RCAC for the controller.
            ipk: The optional IPK for the controller, when None is provided, the defaultIpk
                will be used.
            adminVendorId: The adminVendorId of the controller.
            name: The name of the controller, for debugging use only.
        '''
        super().__init__(name or f"ctrl(v/{adminVendorId})")

        pairingDelegate = c_void_p(None)
        devCtrl = c_void_p(None)

        # Device should hold a reference to the key to avoid it being GC-ed.
        self._externalKeyPair = operationalKey
        nativeKey = operationalKey.create_native_object()

        self._ChipStack.Call(
            lambda: self._dmLib.pychip_OpCreds_AllocateControllerForPythonCommissioningFLow(
                cast(devCtrl, c_void_p), cast(pairingDelegate, c_void_p), nativeKey, noc, len(noc), icac, len(icac) if icac else 0, rcac, len(rcac), ipk, len(ipk) if ipk else 0, adminVendorId, self._ChipStack.enableServerInteractions)
        ).raise_on_error()

        self._set_dev_ctrl(devCtrl, pairingDelegate)

        self._finish_init()
