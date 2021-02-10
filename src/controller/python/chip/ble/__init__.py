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

from chip import ChipDeviceCtrl
import sys
import platform

class Adapter:
    """Generic exposed adapter information."""

    def __init__(self, name, macAddress):
        self.name = name
        self.macAddress = macAddress

    def __str__(self):
        return 'chip.ble.Adapter(%s, %s)' % (self.name, self.macAddress)

if platform.system() == 'Darwin':
    from chip.ChipCoreBluetoothMgr import CoreBluetoothManager as BleManager

    def ConvertNativeAdapter(nativeAdapter):
        raise NotImplementedError('Not implemented for Darwin')

elif sys.platform.startswith('linux'):
    from chip.ChipBluezMgr import BluezManager as BleManager

    def ConvertNativeAdapter(nativeAdapter):
        return Adapter(name = str(nativeAdapter.path), macAddress = str(nativeAdapter.Address))

class LazyHandles(object):
    """Contains a handle to an underlying BLE manager."""

    def __init__(self):
        self._deviceController = None
        self._bleManager = None

    @property
    def deviceController(self):
        if self._deviceController is None:
            self._deviceController = ChipDeviceCtrl.ChipDeviceController()
        return self._deviceController

    @property
    def bleManager(self):
        if self._bleManager is None:
            self._bleManager = BleManager(self.deviceController)
        return self._bleManager

    def GetAdapters(self):
        """Return available BLE adapters on this platform."""
        return [ConvertNativeAdapter(a) for a in self.bleManager.get_adapters()]


lazyHandles = LazyHandles()

def GetBleAdapters():
    return lazyHandles.GetAdapters()

__all__ = [
    "GetBleAdapters",
]
