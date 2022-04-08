#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2020 Google LLC.
#    All rights reserved.
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

#
#    @file
#      This file is utility for Chip
#

from __future__ import absolute_import
from __future__ import print_function
import binascii
from ctypes import *


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
        if array != None:
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
