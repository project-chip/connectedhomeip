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

import asyncio
import builtins
import ctypes
from asyncio.futures import Future
from ctypes import CFUNCTYPE, POINTER, c_char_p, c_size_t, c_uint8, c_uint16, c_uint64, c_void_p, py_object

import chip
from chip.native import PyChipError

from . import BdxTransfer

c_uint8_p = POINTER(c_uint8)


_OnTransferObtainedCallbackFunct = CFUNCTYPE(
    None, py_object, c_void_p, c_uint8, c_uint16, c_uint64, c_uint64, c_uint8_p, c_uint16, c_uint8_p, c_size_t)
_OnFailedToObtainTransferCallbackFunct = CFUNCTYPE(None, py_object, PyChipError)
_OnDataReceivedCallbackFunct = CFUNCTYPE(None, py_object, c_uint8_p, c_size_t)
_OnTransferCompletedCallbackFunct = CFUNCTYPE(None, py_object, PyChipError)


class AsyncTransferObtainedTransaction:
    def __init__(self, future, event_loop, data=None):
        self._future = future
        self._data = data
        self._event_loop = event_loop

    def _handleTransfer(self, bdxTransfer, initMessage: BdxTransfer.InitMessage):
        transfer = BdxTransfer.BdxTransfer(bdx_transfer=bdxTransfer, init_message=initMessage, data=self._data)
        self._future.set_result(transfer)

    def handleTransfer(self, bdxTransfer, initMessage: BdxTransfer.InitMessage):
        self._event_loop.call_soon_threadsafe(self._handleTransfer, bdxTransfer, initMessage)

    def _handleError(self, result: PyChipError):
        self._future.set_exception(result.to_exception())

    def handleError(self, result: PyChipError):
        self._event_loop.call_soon_threadsafe(self._handleError, result)


class AsyncTransferCompletedTransaction:
    def __init__(self, future, event_loop):
        self._future = future
        self._event_loop = event_loop

    def _handleResult(self, result: PyChipError):
        if result.is_success:
            self._future.set_result(result)
        else:
            self._future.set_exception(result.to_exception())

    def handleResult(self, result: PyChipError):
        self._event_loop.call_soon_threadsafe(self._handleResult, result)


@_OnTransferObtainedCallbackFunct
def _OnTransferObtainedCallback(transaction: AsyncTransferObtainedTransaction, bdxTransfer, transferControlFlags: int,
                                maxBlockSize: int, startOffset: int, length: int, fileDesignator, fileDesignatorLength: int,
                                metadata, metadataLength: int):
    fileDesignatorData = ctypes.string_at(fileDesignator, fileDesignatorLength)
    metadataData = ctypes.string_at(metadata, metadataLength)

    initMessage = BdxTransfer.InitMessage(
        transferControlFlags,
        maxBlockSize,
        startOffset,
        length,
        fileDesignatorData[:],
        metadataData[:],
    )

    transaction.handleTransfer(bdxTransfer, initMessage)


@_OnFailedToObtainTransferCallbackFunct
def _OnFailedToObtainTransferCallback(transaction: AsyncTransferObtainedTransaction, result: PyChipError):
    transaction.handleError(result)


@_OnDataReceivedCallbackFunct
def _OnDataReceivedCallback(context, dataBuffer: c_uint8_p, bufferLength: int):
    data = ctypes.string_at(dataBuffer, bufferLength)
    context(data)


@_OnTransferCompletedCallbackFunct
def _OnTransferCompletedCallback(transaction: AsyncTransferCompletedTransaction, result: PyChipError):
    transaction.handleResult(result)


def PrepareToReceiveBdxData(future: Future) -> PyChipError:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncTransferObtainedTransaction(future=future, event_loop=asyncio.get_running_loop())

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    res = builtins.chipStack.Call(
        lambda: handle.pychip_Bdx_ExpectBdxTransfer(ctypes.py_object(transaction))
    )
    if not res.is_success:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def PrepareToSendBdxData(future: Future, data: bytes) -> PyChipError:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncTransferObtainedTransaction(future=future, event_loop=asyncio.get_running_loop(), data=data)

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    res = builtins.chipStack.Call(
        lambda: handle.pychip_Bdx_ExpectBdxTransfer(ctypes.py_object(transaction))
    )
    if not res.is_success:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def AcceptSendTransfer(transfer: c_void_p, dataReceivedClosure, transferComplete: Future):
    handle = chip.native.GetLibraryHandle()
    complete_transaction = AsyncTransferCompletedTransaction(future=transferComplete, event_loop=asyncio.get_running_loop())
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(dataReceivedClosure))
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(complete_transaction))
    res = builtins.chipStack.Call(
        lambda: handle.pychip_Bdx_AcceptSendTransfer(transfer, dataReceivedClosure, complete_transaction)
    )
    if not res.is_success:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(dataReceivedClosure))
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(complete_transaction))
    return res


def AcceptReceiveTransfer(transfer: c_void_p, data: bytes, transferComplete: Future):
    handle = chip.native.GetLibraryHandle()
    return builtins.chipStack.Call(
        lambda: handle.pychip_Bdx_AcceptReceiveTransfer(transfer, c_char_p(data), len(data), transferComplete)
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
                   _OnTransferObtainedCallbackFunct, _OnFailedToObtainTransferCallbackFunct, _OnDataReceivedCallbackFunct,
                   _OnTransferCompletedCallbackFunct])

    handle.pychip_Bdx_InitCallbacks(
        _OnTransferObtainedCallback, _OnFailedToObtainTransferCallback, _OnDataReceivedCallback, _OnTransferCompletedCallback)
