import ctypes
from enum import IntEnum
from typing import Dict, List, Optional, Tuple

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments


# Only ChipFaults (defined in src/lib/support/CHIPFaultInjection.h) are implemented, Implement others as needed
class FaultType(IntEnum):
    Unspecified = 0,
    SystemFault = 1,
    InetFault = 2,
    ChipFault = 3,


def _GetNlFaultInjectionLibraryHandle() -> ctypes.CDLL:
    """Get the native library handle with fault injection methods initialized."""
    handle = GetLibraryHandle(HandleFlags(0))

    if not handle.pychip_faultinjection_get_num_faults.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_faultinjection_get_fault_names', ctypes.POINTER(ctypes.c_char_p), None)
        setter.Set('pychip_faultinjection_get_num_faults', ctypes.c_uint32, None)

        setter.Set('pychip_faultinjection_fail_at_fault', ctypes.c_uint32, [
                   ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_bool])

        setter.Set('pychip_faultinjection_get_fault_counter', ctypes.c_uint32, [
                   ctypes.c_uint32])

        setter.Set('pychip_faultinjection_reset_fault_counters', None, None)

    return handle


def _generate_fault_enum_from_c(enum_name: str = "CHIPFaultId") -> IntEnum:
    """Generate a Python IntEnum from the sFaultNames[] array defined in src/lib/support/CHIPFaultInjection.cpp """

    handle = _GetNlFaultInjectionLibraryHandle()

    count = handle.pychip_faultinjection_get_num_faults()
    array_ptr = handle.pychip_faultinjection_get_fault_names()

    names = []
    for i in range(count):
        c_str = array_ptr[i]
        if c_str is None:
            raise ValueError(
                f"sFaultNames[{i}] is NULL —> check that all fault ids defined in enum 'Id' in src/lib/support/CHIPFaultInjection.h" +
                "are present in sFaultNames[] array defined in src/lib/support/CHIPFaultInjection.cpp ")
        names.append(c_str.decode())

    enum_dict = {name: i for i, name in enumerate(names)}

    return IntEnum(enum_name, enum_dict)


# This Enum is dynamically generated, it will have the same names as the sFaultNames[] array defined in src/lib/support/CHIPFaultInjection.cpp
CHIPFaultId = _generate_fault_enum_from_c()


def FailAtFault(faultID, numCallsToSkip: int, numCallsToFail: int, takeMutex: bool = True):
    """ Configure a fault to be triggered
    This should only be used to inject faults locally into the Client being run by the script. Otherwise to inject Faults over the data model, use the FailAtFault variant in clusters/Objects.py"""
    handle = _GetNlFaultInjectionLibraryHandle()

    nlfaultinjectionReturnCode = handle.pychip_faultinjection_fail_at_fault(
        faultID, numCallsToSkip, numCallsToFail, takeMutex)

    if nlfaultinjectionReturnCode != 0:
        raise Exception(f"Fault injection failed with return code: {nlfaultinjectionReturnCode}")


def GetFaultCounter(faultID):
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
