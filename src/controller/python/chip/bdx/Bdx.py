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

import builtins
import ctypes
from asyncio.futures import Future
from ctypes import CFUNCTYPE, POINTER, c_char_p, c_size_t, c_uint8, c_uint16, c_uint64, c_void_p, py_object

import chip
from chip.native import PyChipError

from . import BdxTransfer


c_uint8_p = POINTER(c_uint8)


_OnTransferObtainedCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError, c_void_p, c_uint8, c_uint16, c_uint64, c_uint64, c_uint8_p, c_uint16, c_uint8_p, c_size_t)
_OnDataReceivedCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint8, PyChipError)
_OnTransferCompletedCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError)


@_OnTransferObtainedCallbackFunct
def _OnTransferObtainedCallback(future: Future, result: PyChipError, bdxTransfer, transferControlFlags: int, maxBlockSize: int,
                                startOffset: int, length: int, fileDesignator, fileDesignatorLength: int, metadata,
                                metadataLength: int):
    if result is CHIP_NO_ERROR:
        fileDesignatorData = ctypes.string_at(fileDesignator, fileDesignatorLength)
        metadataData = ctypes.string_at(metadata, metadataLength)

        initMessage = BdxTransfer.InitMessage()
        initMessage.TransferControlFlags = transferControlFlags
        initMessage.MaxBlockSize = maxBlockSize
        initMessage.StartOffset = startOffset
        initMessage.Length = length
        initMessage.FileDesignator = fileDesignatorData[:]
        initMessage.Metadata = metadataData[:]

        future.handleTransfer(bdxTransfer, initMessage)
    else:
        future.handleError(result)


@_OnDataReceivedCallbackFunct
def _OnDataReceivedCallback(context, dataBuffer, bufferLength: int):
    data = ctypes.string_at(dataBuffer, bufferLength)
    context(data[:])


@_OnTransferCompletedCallbackFunct
def _OnTransferCompletedCallback(future: Future, result: PyChipError):
    if result is CHIP_NO_ERROR:
        future.set_result(result)
    else:
        future.set_exception(result.to_exception())


class AsyncTransferObtainedTransaction:
    def __init__(self, future, data=None):
        self._future = future
        self._data = data

    def handleTransfer(self, bdxTransfer, initMessage: BdxTransfer.InitMessage):
        transfer = BdxTransfer(bdx_transfer=bdxTransfer, init_message=initMessage, data=self._data)
        self._future.set_result(transfer)

    def handleError(self, result: PyChipError):
        self._future.set_exception(result.to_exception())


async def PrepareToReceiveBdxData(future: Future):
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncTransferObtainedTransaction(future=future)

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_Bdx_ExpectBdxTransfer(ctypes.py_object(transaction))
    )


async def PrepareToSendBdxData(future: Future, data: bytes):
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncTransferObtainedTransaction(future=future, data=data)

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_Bdx_ExpectBdxTransfer(ctypes.py_object(transaction))
    )


async def AcceptSendTransfer(transfer: c_void_p, dataReceivedClosure, transferComplete: Future):
    handle = chip.native.GetLibraryHandle()
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(dataReceivedClosure))
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transferComplete))
    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_Bdx_AcceptSendTransfer(transfer, dataReceivedClosure, transferComplete)
    )


async def AcceptReceiveTransfer(transfer: c_void_p, data: bytes, transferComplete: Future):
    handle = chip.native.GetLibraryHandle()
    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_Bdx_AcceptReceiveTransfer(transfer, ctypes.c_char_p(data), len(data), transferComplete)
    )


async def RejectTransfer(transfer: c_void_p):
    handle = chip.native.GetLibraryHandle()
    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_Bdx_RejectTransfer(transfer)
    )


def Init():
    handle = chip.native.GetLibraryHandle()
    # Uses one of the type decorators as an indicator for everything being initialized.
    if not handle.pychip_Bdx_ExpectBdxTransfer.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_Bdx_ExpectBdxTransfer',
                   PyChipError, [py_object])
        setter.Set('pychip_Bdx_StopExpectingBdxTransfer',
                   PyChipError, [py_object])
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
