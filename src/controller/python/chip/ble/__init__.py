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
"""BLE-related functionality within CHIP"""

import sys
import platform
import chip.native
import ctypes
from ctypes import c_bool, c_void_p, c_char_p, c_uint
from dataclasses import dataclass


@dataclass
class AdapterInfo:
  index: int
  address: str
  name: str
  alias: str
  powered_on: bool


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

  return handle


def GetAdapters():
  handle = _GetBleLibraryHandle()

  result = []
  nativeList = handle.pychip_ble_adapter_list_new()
  if nativeList == 0:
    raise Exception('Failed to get BLE adapter list')

  try:
    while handle.pychip_ble_adapter_list_next(nativeList):
      result.append(
          AdapterInfo(
              index=handle.pychip_ble_adapter_list_get_index(nativeList),
              address=handle.pychip_ble_adapter_list_get_address(nativeList).decode('utf8'),
              name=handle.pychip_ble_adapter_list_get_name(nativeList).decode('utf8'),
              alias=handle.pychip_ble_adapter_list_get_alias(nativeList).decode('utf8'),
              powered_on=handle.pychip_ble_adapter_list_is_powered(nativeList),
          ))

  finally:
    handle.pychip_ble_adapter_list_delete(nativeList)

  return result


__all__ = [
    'GetBleAdapters',
]
