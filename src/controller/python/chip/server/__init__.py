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

import ctypes

from chip import native
from chip.native import PostAttributeChangeCallback, c_PostAttributeChangeCallback

__all__ = [
    "GetLibraryHandle",
    "PostAttributeChangeCallback",
]


def GetLibraryHandle(cb: c_PostAttributeChangeCallback) -> ctypes.CDLL:
    """Get a memoized handle to the chip native code dll.

    Args:
      cb: A callback decorated by PostAttributeChangeCallback decorator.
    """

    handle = native._GetLibraryHandle(native.Library.SERVER, False)
    if not handle.initialized:
        handle.dll.pychip_server_native_init().raise_on_error()
        handle.dll.pychip_server_set_callbacks(cb)
        handle.initialized = True

    return handle.dll
