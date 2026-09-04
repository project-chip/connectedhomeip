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

"""Capture inbound Interaction Model response messages on ExchangeManager."""

import ctypes
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from matter.ChipDeviceCtrl import ChipDeviceController

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments, PyChipError


class PyImCaptureSnapshot(ctypes.Structure):
    _fields_ = [
        ("invokeResponseCount", ctypes.c_uint32),
        ("statusResponseCount", ctypes.c_uint32),
        ("writeResponseCount", ctypes.c_uint32),
        ("totalImResponseCount", ctypes.c_uint32),
    ]


def _GetLibraryHandle() -> ctypes.CDLL:
    handle = GetLibraryHandle(HandleFlags(0))
    if not handle.pychip_im_capture_set_observer.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_im_capture_set_observer', PyChipError, [ctypes.c_void_p])
        setter.Set('pychip_im_capture_reset', PyChipError, [])
        setter.Set('pychip_im_capture_get_snapshot', PyChipError, [ctypes.POINTER(PyImCaptureSnapshot)])
    return handle


def SetObserver(controller: "ChipDeviceController") -> None:
    """Set the IM capture observer on the given controller's ExchangeManager."""
    handle = _GetLibraryHandle()
    handle.pychip_im_capture_set_observer(controller.devCtrl).raise_on_error()


def Reset() -> None:
    """Clear all captured message counts. Call before triggering an action whose
    unexpected response counts you intend to verify."""
    handle = _GetLibraryHandle()
    handle.pychip_im_capture_reset().raise_on_error()


def GetSnapshot() -> PyImCaptureSnapshot:
    """Read the current Interaction Model capture state in one call."""
    handle = _GetLibraryHandle()
    snapshot = PyImCaptureSnapshot()
    handle.pychip_im_capture_get_snapshot(ctypes.pointer(snapshot)).raise_on_error()
    return snapshot
