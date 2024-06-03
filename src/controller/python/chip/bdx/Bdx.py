#
#    Copyright (c) 2024 Project CHIP Authors
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

import ctypes
from ctypes import CFUNCTYPE, POINTER, c_bool, c_char_p, c_size_t, c_uint8, c_uint16, c_uint32, c_void_p, cast, py_object


_OnTransferObtainedCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError, c_void_p, c_uint8, c_uint16, c_uint64, c_uint64, c_uint8_p, c_uint16, c_uint8_p, c_size_t)
_OnDataReceivedCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint8, PyChipError)
_OnTransferCompletedCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError)


@_OnTransferObtainedCallbackFunct
def _OnTransferObtainedCallback(closure, result: PyChipError, bdxTransfer, transferControlFlags: int, maxBlockSize: int,
                                startOffset: int, length: int, fileDesignator, fileDesignatorLength: int, metadata,
                                metadataLength: int):
    # TODO: Call the closure with the rest of the parameters.
    pass


@_OnDataReceivedCallbackFunct
def _OnDataReceivedCallback(closure, dataBuffer, bufferLength: int):
    # TODO: Call the closure with the data.
    pass


@_OnTransferCompletedCallbackFunct
def _OnTransferCompletedCallback(closure, result: PyChipError):
    # TODO: Call the closure.
    pass


def Init():
    handle = chip.native.GetLibraryHandle()
    # Uses one of the type decorators as an indicator for everything being initialized.
    if not handle.pychip_Bdx_ExpectBdxTransfer.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_Bdx_ExpectBdxTransfer',
                   PyChipError, [])
        setter.Set('pychip_Bdx_StopExpectingBdxTransfer',
                   PyChipError, [])
        setter.Set('pychip_Bdx_AcceptSendTransfer',
                   PyChipError, [c_void_p, py_object, py_object])
        setter.Set('pychip_Bdx_AcceptReceiveTransfer',
                   PyChipError, [c_void_p, c_uint8_p, c_size_t])
        setter.Set('pychip_Bdx_RejectTransfer',
                   PyChipError, [c_void_p])
        setter.Set('pychip_Bdx_InitCallbacks', None, [
                   _OnTransferObtainedCallbackFunct, _OnDataReceivedCallbackFunct, _OnTransferCompletedCallbackFunct])

    handle.pychip_Bdx_InitCallbacks(
        _OnTransferObtainedCallback, _OnDataReceivedCallback, _OnTransferCompletedCallback)
