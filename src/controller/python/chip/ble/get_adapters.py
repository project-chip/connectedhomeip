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

from dataclasses import dataclass
from typing import List

from .library_handle import _GetBleLibraryHandle


@dataclass
class AdapterInfo:
    index: int
    address: str
    name: str
    alias: str
    powered_on: bool


def GetAdapters() -> List[AdapterInfo]:
    """Get a list of BLE adapters available on the system. """
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
                    address=handle.pychip_ble_adapter_list_get_address(
                        nativeList).decode('utf8'),
                    name=handle.pychip_ble_adapter_list_get_name(nativeList).decode(
                        'utf8'),
                    alias=handle.pychip_ble_adapter_list_get_alias(nativeList).decode(
                        'utf8'),
                    powered_on=handle.pychip_ble_adapter_list_is_powered(
                        nativeList),
                ))

    finally:
        handle.pychip_ble_adapter_list_delete(nativeList)

    return result
