#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import ctypes

import chip.native
from chip.discovery.types import DiscoverFailureCallback_t, DiscoverSuccessCallback_t
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
