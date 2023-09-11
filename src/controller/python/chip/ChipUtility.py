#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
# SPDX-FileCopyrightText: 2020 Google LLC.
#
# SPDX-License-Identifier: Apache-2.0
#

#
#    @file
#      This file is utility for Chip
#

from __future__ import absolute_import, print_function

import binascii
from ctypes import c_byte, c_void_p, cast, memmove


class ChipUtility(object):
    @staticmethod
    def Hexlify(val):
        return binascii.hexlify(val).decode()

    @staticmethod
    def VoidPtrToByteArray(ptr, len):
        if ptr:
            v = bytearray(len)
            memmove((c_byte * len).from_buffer(v), ptr, len)
            return v
        return None

    @staticmethod
    def ByteArrayToVoidPtr(array):
        if array is not None:
            if not (isinstance(array, bytes) or isinstance(array, bytearray)):
                raise TypeError("Array must be an str or a bytearray")
            return cast((c_byte * len(array)).from_buffer_copy(array), c_void_p)
        return c_void_p(0)

    @staticmethod
    def IsByteArrayAllZeros(array):
        for i in array:
            if i != 0:
                return False
        return True

    @staticmethod
    def ByteArrayToHex(array):
        return ChipUtility.Hexlify(bytes(array))

    @staticmethod
    def CStringToString(s):
        return None if s is None else s.decode()

    @staticmethod
    def StringToCString(s):
        return None if s is None else s.encode()

# To support Python 3.8 and older versions, which does not support @classmethod + @property


class classproperty(property):
    def __get__(self, cls, owner):
        return classmethod(self.fget).__get__(None, owner)()
