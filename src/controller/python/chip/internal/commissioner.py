#
#    Copyright (c) 2021 Project CHIP Authors
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
from chip.configuration import GetLocalNodeId
from chip.native import NativeLibraryHandleMethodArguments, GetLibraryHandle
from ctypes import c_uint64, c_uint32, c_uint16, c_char_p
from typing import Optional
from chip.internal.types import NetworkCredentialsRequested, OperationalCredentialsRequested, PairingComplete
import ctypes

# Not using c_void_p directly is IMPORTANT. Python auto-casts c_void_p
# to intergers and this can cause 32/64 bit issues.
class Commisioner_p(ctypes.c_void_p):
    pass

@NetworkCredentialsRequested
def OnNetworkCredentialsRequested():
    GetCommisioner()._OnNetworkCredentialsRequested()

@OperationalCredentialsRequested
def OnOperationalCredentialsRequested(csr, csr_length):
    GetCommisioner()._OnOperationalCredentialsRequested(ctypes.string_at(csr, csr_length))

@PairingComplete
def OnPairingComplete(err: int):
    GetCommisioner()._OnPairingComplete(err)


# Commissioner states regarding pairing
COMMISIONER_INITIALIZED = 0
COMMISIONER_PAIRING = 1
COMMISIONER_NEEDS_NETCREDS = 2
COMMISIONER_NEEDS_OPCREDS = 3


class Commissioner:
    """Commissioner wraps the DeviceCommisioner native class.
    

    The commissioner is a DeviceController that supports pairing. Since the device
    controller supports multiple devices, this class is expected to be used 
    as a singleton
    
    """

    def __init__(self, handle: ctypes.CDLL, native: Commisioner_p):
        self._handle = handle
        self._native = native
        self.pairing_state = COMMISIONER_INITIALIZED
        self.on_network_credentials_requested = None
        self.on_operational_credentials_requested = None
        self.on_pairing_complete = None

    
    def BlePair(self, remoteDeviceId: int, pinCode: int, discriminator: int):
        result = self._handle.pychip_internal_Commisioner_BleConnectForPairing(self._native, remoteDeviceId, pinCode, discriminator)
        if result != 0: 
            raise Exception("Failed to pair. CHIP Error code %d" % result)

        self.pairing_state = COMMISIONER_PAIRING
    
    def PairSendWifiCredentials(self, ssid, password):
        """Send wifi credentials to the actively connected device."""

        if self.pairing_state != COMMISIONER_NEEDS_NETCREDS:
            raise Exception("Not in a state requiring network credentials")

        self._handle.pychip_internal_PairingDelegate_SetNetworkCredentials(c_char_p(ssid.encode('utf8')), c_char_p(password.encode('utf8')))


    def Unpair(self, remoteDeviceId: int):
        result = self._handle.pychip_internal_Commisioner_Unpair(self._native, remoteDeviceId)
        if result != 0: 
            raise Exception("Failed to unpair. CHIP Error code %d" % result)

    
    def _OnNetworkCredentialsRequested(self):
        self.pairing_state = COMMISIONER_NEEDS_NETCREDS
        if self.on_network_credentials_requested:
          self.on_network_credentials_requested()

    def _OnOperationalCredentialsRequested(self, csr: bytes):
        self.pairing_state = COMMISIONER_NEEDS_OPCREDS
        if self.on_operational_credentials_requested:
          self.on_operational_credentials_requested(csr)

    def _OnPairingComplete(self, err: int):
        self.pairing_state = COMMISIONER_INITIALIZED
        if self.on_pairing_complete:
          self.on_pairing_complete(err)


def _SetNativeCallSignatues(handle: ctypes.CDLL):
    """Sets up the FFI types for the cdll handle."""
    setter = NativeLibraryHandleMethodArguments(handle)

    setter.Set('pychip_internal_Commisioner_New', Commisioner_p, [c_uint64])
    setter.Set('pychip_internal_Commisioner_Unpair', c_uint32, [Commisioner_p, c_uint64])
    setter.Set('pychip_internal_Commisioner_BleConnectForPairing', c_uint32, [Commisioner_p, c_uint64, c_uint32, c_uint16])

    setter.Set('pychip_internal_PairingDelegate_SetNetworkCredentialsRequestedCallback', None, [NetworkCredentialsRequested])
    setter.Set('pychip_internal_PairingDelegate_SetOperationalCredentialsRequestedCallback', None, [OperationalCredentialsRequested])
    setter.Set('pychip_internal_PairingDelegate_SetPairingCompleteCallback', None, [PairingComplete])
    setter.Set('pychip_internal_PairingDelegate_SetNetworkCredentials', None, [c_char_p, c_char_p])


commissionerSingleton: Optional[Commissioner] = None

def GetCommissioner() -> Commissioner:
    """Gets a reference to the global commissioner singleton.

    Uses the configuration GetLocalNodeId().
    """

    global commissionerSingleton

    if commissionerSingleton is None:
        handle = GetLibraryHandle()
        _SetNativeCallSignatues(handle)

        native = handle.pychip_internal_Commisioner_New(GetLocalNodeId())
        if not native:
            raise Exception('Failed to create commissioner object.') 

        handle.pychip_internal_PairingDelegate_SetNetworkCredentialsRequestedCallback(OnNetworkCredentialsRequested)
        handle.pychip_internal_PairingDelegate_SetOperationalCredentialsRequestedCallback(OnOperationalCredentialsRequested)
        handle.pychip_internal_PairingDelegate_SetPairingCompleteCallback(OnPairingComplete)

        commissionerSingleton = Commissioner(handle, native)


    return commissionerSingleton
