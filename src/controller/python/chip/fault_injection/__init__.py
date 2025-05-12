#
#    Copyright (c) 2025 Project CHIP Authors
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

import ctypes
from enum import Enum, auto
from typing import Optional

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments, PyChipError


def _GetNlFaultInjectionLibraryHandle() -> ctypes.CDLL:
    """ Get the native library handle with tracing methods initialized.

      Retrieves the CHIP native library handle and attaches signatures to
      native methods.
      """

    # Getting a handle without requiring init, as nlfaultinjection methods
    # do not require chip stack startup
    handle = GetLibraryHandle(HandleFlags(0))

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_tracing_start_json_file.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_faultinjection_fail_at_fault', ctypes.c_uint32, [
                   ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_bool])

    return handle


class FaultIDs(Enum):
    JSON = auto()
    PERFETTO = auto()


def FailAtFault(faultID, numCallsToSkip, numCallsToFail, takeMutex):
    """
    Initiate tracing to the specified destination.

    Note that only one active trace can exist of a given type (i.e. cannot trace both
    to files and logs/system).
    """
    handle = _GetNlFaultInjectionLibraryHandle()

    nlfaultinjectionReturnCode = 0
    nlfaultinjectionReturnCode = handle.pychip_faultinjection_fail_at_fault(faultID, numCallsToSkip, numCallsToFail, takeMutex)

    if nlfaultinjectionReturnCode != 0:
        raise Exception
