#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Capture inbound CASE handshake messages (Sigma2, Sigma2_Resume, StatusReport).
Call Reset() before triggering the handshake; check `.present`
on each slot to see whether the message was observed."""

import ctypes

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments, PyChipError


# Mirror of the C struct PychipCaseCapturedHeaders defined in CASECapture.h.
class PyCaseCapturedHeaders(ctypes.Structure):
    _fields_ = [
        ("sessionType", ctypes.c_uint8),
        ("sFlag", ctypes.c_uint8),
        ("dsiz", ctypes.c_uint8),
        ("protocolId", ctypes.c_uint16),
        ("opcode", ctypes.c_uint8),
        ("isInitiator", ctypes.c_uint8),
        ("present", ctypes.c_uint8),
    ]


# Mirror of the C struct PychipCaseCaptureSnapshot defined in CASECapture.h.
class PyCaseCaptureSnapshot(ctypes.Structure):
    _fields_ = [
        ("sigma2", PyCaseCapturedHeaders),
        ("sigma2Resume", PyCaseCapturedHeaders),
        ("statusReport", PyCaseCapturedHeaders),
        ("statusReportParsed", ctypes.c_uint8),
        ("statusReportGeneralCode", ctypes.c_uint16),
        ("statusReportProtocolId", ctypes.c_uint32),
        ("statusReportProtocolCode", ctypes.c_uint16),
    ]


def _GetLibraryHandle() -> ctypes.CDLL:
    handle = GetLibraryHandle(HandleFlags(0))
    if not handle.pychip_case_capture_set_observer.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_case_capture_set_observer', PyChipError, [ctypes.c_void_p])
        setter.Set('pychip_case_capture_reset', PyChipError, [])
        setter.Set('pychip_case_capture_get_snapshot', PyChipError, [ctypes.POINTER(PyCaseCaptureSnapshot)])
    return handle


def SetObserver(controller) -> None:
    """Set the CASE capture observer on the given controller's ExchangeManager."""
    handle = _GetLibraryHandle()
    handle.pychip_case_capture_set_observer(controller.devCtrl).raise_on_error()


def Reset() -> None:
    """Clear all captured slots. Call before triggering a CASE handshake whose
    values you intend to read."""
    handle = _GetLibraryHandle()
    handle.pychip_case_capture_reset().raise_on_error()


def GetSnapshot() -> PyCaseCaptureSnapshot:
    """Read the entire current capture state in one call."""
    handle = _GetLibraryHandle()
    snapshot = PyCaseCaptureSnapshot()
    handle.pychip_case_capture_get_snapshot(ctypes.pointer(snapshot)).raise_on_error()
    return snapshot
