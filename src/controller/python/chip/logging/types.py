#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from ctypes import CFUNCTYPE, c_char_p, c_uint8

# Log callback: void(category, module, message)
LogRedirectCallback_t = CFUNCTYPE(None, c_uint8, c_char_p, c_char_p)
