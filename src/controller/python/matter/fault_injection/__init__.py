import ctypes
from enum import IntEnum

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments


# Only ChipFaults (defined in src/lib/support/CHIPFaultInjection.h) are supported in this module, Implement others as needed
class FaultType(IntEnum):
    Unspecified = 0,
    SystemFault = 1,
    InetFault = 2,
    ChipFault = 3,


# IMPORTANT: CHIPFaultId enum must be kept in sync with the 'Id' enum in src/lib/support/CHIPFaultInjection.h
# If you change values in the C/C++ header, update them here as well.
# BEGIN-IF-CHANGE-ALSO-CHANGE(/src/lib/support/CHIPFaultInjection.h)
class CHIPFaultId(IntEnum):
    """Fault IDs for CHIP fault injection, matching the `Id` enum in src/lib/support/CHIPFaultInjection.h"""
    AllocExchangeContext = 0
    DropIncomingUDPMsg = 1
    DropOutgoingUDPMsg = 2
    AllocBinding = 3
    SendAlarm = 4
    HandleAlarm = 5
    FuzzExchangeHeaderTx = 6
    RMPDoubleTx = 7
    RMPSendError = 8
    BDXBadBlockCounter = 9
    BDXAllocTransfer = 10
    SecMgrBusy = 11
    IMInvoke_SeparateResponses = 12
    IMInvoke_SeparateResponsesInvertResponseOrder = 13
    IMInvoke_SkipSecondResponse = 14
    ModifyWebRTCAnswerSessionId = 15
    ModifyWebRTCOfferSessionId = 16
    CASEServerBusy = 17
    CASESkipInitiatorResumeMIC = 18
    CASESkipResumptionID = 19
    CASECorruptInitiatorResumeMIC = 20
    CASECorruptDestinationID = 21
    CASECorruptTBEData3Encrypted = 22
    CASECorruptSigma3NOC = 23
    CASECorruptSigma3ICAC = 24
    CASECorruptSigma3Signature = 25
    CASECorruptSigma3InitiatorEphPubKey = 26
    CASECorruptSigma3ResponderEphPubKey = 27
    CASECorruptTBEData2Encrypted = 28
    CASECorruptSigma2NOC = 29
    CASECorruptSigma2ICAC = 30
    CASECorruptSigma2Signature = 31


# END-IF-CHANGE-ALSO-CHANGE(/src/lib/support/CHIPFaultInjection.h)
# IMPORTANT: CHIPFaultId enum above must be kept in sync with the 'Id' enum in src/lib/support/CHIPFaultInjection.h
# If you change values in the C/C++ header, update them here as well.


def _GetNlFaultInjectionLibraryHandle() -> ctypes.CDLL:
    """Get the native library handle with fault injection methods initialized."""
    handle = GetLibraryHandle(HandleFlags(0))

    if not handle.pychip_faultinjection_fail_at_fault.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_faultinjection_get_num_faults', ctypes.c_uint32, None)

        setter.Set('pychip_faultinjection_fail_at_fault', ctypes.c_uint32, [
                   ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_bool])

        setter.Set('pychip_faultinjection_get_fault_counter', ctypes.c_uint32, [
                   ctypes.c_uint32])

        setter.Set('pychip_faultinjection_reset_fault_counters', None, None)

        num_faults = handle.pychip_faultinjection_get_num_faults()
        assert len(CHIPFaultId) == num_faults, "The Number of Faults in the CHIPFaultId Enum doesn't match that in the C++ CHIPFaultInjection Header"

    return handle


def FailAtFault(faultID: CHIPFaultId, numCallsToSkip: int, numCallsToFail: int, takeMutex: bool = True):
    """ Configure a fault to be triggered
    This should only be used to inject faults locally into the Client being run by the script. Otherwise to inject Faults over the data model, use the FailAtFault variant in clusters/Objects.py"""
    handle = _GetNlFaultInjectionLibraryHandle()

    nlfaultinjectionReturnCode = handle.pychip_faultinjection_fail_at_fault(
        faultID, numCallsToSkip, numCallsToFail, takeMutex)

    if nlfaultinjectionReturnCode != 0:
        raise Exception(f"Fault injection failed with return code: {nlfaultinjectionReturnCode}")


def GetFaultCounter(faultID: CHIPFaultId):
    """ Returns the number of times a specific fault was checked during execution.

    This is useful for verifying that the code path containing the fault injection was actually executed,
    Note: The count includes all checks, even if the fault was not triggered.
      """

    handle = _GetNlFaultInjectionLibraryHandle()
    return handle.pychip_faultinjection_get_fault_counter(faultID)


def ResetFaultCounters():
    """ Resets the counter that checks the number of times a specific fault was checked """

    handle = _GetNlFaultInjectionLibraryHandle()
    handle.pychip_faultinjection_reset_fault_counters()
