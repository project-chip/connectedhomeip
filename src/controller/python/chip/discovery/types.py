#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from ctypes import CFUNCTYPE, c_char_p, c_uint16, c_uint32, c_uint64

from chip.native import PyChipError

DiscoverSuccessCallback_t = CFUNCTYPE(
    None,  # void return
    c_uint64,  # fabric id
    c_uint64,  # node id
    c_uint32,  # interface id
    c_char_p,  # IP address
    c_uint16  # port
)

DiscoverFailureCallback_t = CFUNCTYPE(
    None,  # void return
    c_uint64,  # fabric id
    c_uint64,  # node id
    PyChipError,  # CHIP_ERROR error code
)
