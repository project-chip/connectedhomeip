#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import ctypes

import chip.native
from chip.logging.types import LogRedirectCallback_t


def _GetLoggingLibraryHandle() -> ctypes.CDLL:
    """ Get the native library handle with logging method initialization.

      Retreives the CHIP native library handle and attaches signatures to
      native methods.
      """

    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_logging_set_callback.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_logging_set_callback',
                   ctypes.c_void_p, [LogRedirectCallback_t])

    return handle
