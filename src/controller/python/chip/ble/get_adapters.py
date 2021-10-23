from typing import List
from dataclasses import dataclass
from chip.ble.library_handle import _GetBleLibraryHandle


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
