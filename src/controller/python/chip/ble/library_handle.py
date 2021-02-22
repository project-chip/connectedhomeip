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

import chip.native
import ctypes
from ctypes import c_bool, c_void_p, c_char_p, c_uint, py_object
from chip.ble.types import DeviceScannedCallback_t, ScanDoneCallback_t


def _GetBleLibraryHandle() -> ctypes.CDLL:
  """ Get the native library handle with BLE method initialization.

    Retreives the CHIP native library handle and attaches signatures to
    native methods.
    """

  handle = chip.native.GetLibraryHandle()

  # Uses one of the type decorators as an indicator for everything being
  # initialized. Native methods default to c_int return types
  if handle.pychip_ble_adapter_list_new.restype != c_void_p:
    setter = chip.native.NativeLibraryHandleMethodArguments(handle)

    setter.Set('pychip_ble_adapter_list_new', c_void_p, [])
    setter.Set('pychip_ble_adapter_list_next', c_bool, [c_void_p])
    setter.Set('pychip_ble_adapter_list_get_index', c_uint, [c_void_p])
    setter.Set('pychip_ble_adapter_list_get_address', c_char_p, [c_void_p])
    setter.Set('pychip_ble_adapter_list_get_alias', c_char_p, [c_void_p])
    setter.Set('pychip_ble_adapter_list_get_name', c_char_p, [c_void_p])
    setter.Set('pychip_ble_adapter_list_is_powered', c_bool, [c_void_p])
    setter.Set('pychip_ble_adapter_list_delete', None, [c_void_p])
    setter.Set('pychip_ble_adapter_list_get_raw_adapter', c_void_p, [c_void_p])

    setter.Set('pychip_ble_start_scanning', c_void_p, [
        py_object, c_void_p, c_uint, DeviceScannedCallback_t, ScanDoneCallback_t
    ])

  return handle
