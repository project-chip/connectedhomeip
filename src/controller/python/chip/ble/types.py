#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from ctypes import CFUNCTYPE, c_char_p, c_uint16, py_object

DeviceScannedCallback = CFUNCTYPE(None, py_object, c_char_p, c_uint16,
                                  c_uint16, c_uint16)

ScanDoneCallback = CFUNCTYPE(None, py_object)

ScanErrorCallback = CFUNCTYPE(None, py_object, c_uint16)
