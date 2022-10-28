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
from chip.discovery.types import DiscoverSuccessCallback_t, DiscoverFailureCallback_t
from chip.native import PyChipError


def _GetDiscoveryLibraryHandle() -> ctypes.CDLL:
    """ Get the native library handle with discovery methods initialized.

      Retreives the CHIP native library handle and attaches signatures to
      native methods.
      """

    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_discovery_resolve.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_discovery_resolve', PyChipError,
                   [ctypes.c_uint64, ctypes.c_uint64])
        setter.Set('pychip_discovery_set_callbacks', None, [
                   DiscoverSuccessCallback_t, DiscoverFailureCallback_t])

    return handle
