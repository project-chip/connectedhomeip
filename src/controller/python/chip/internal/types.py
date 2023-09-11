#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from ctypes import CFUNCTYPE, c_size_t, c_uint32, c_void_p

# General callback of 'network credentials requested. No python-data
# is available as the underlying callback is used internally
NetworkCredentialsRequested = CFUNCTYPE(None)

# Operational credential request
# Can access the csr data using ctypes.string_at
OperationalCredentialsRequested = CFUNCTYPE(None, c_void_p, c_size_t)

# Notification that pairing has been coompleted
PairingComplete = CFUNCTYPE(None, c_uint32)
