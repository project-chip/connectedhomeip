#
#    Copyright (c) 2020 Project CHIP Authors
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

from asyncio.futures import Future
import ctypes
from dataclasses import dataclass
from typing import Type
from ctypes import CFUNCTYPE, c_char_p, c_size_t, c_void_p, c_uint32,  c_uint16, py_object


from .ClusterObjects import ClusterCommand
import chip
from chip import exceptions as ChipExceptions
from chip.interaction_model import exceptions as IMExceptions


@dataclass
class CommandPath:
    EndpointId: int
    ClusterId: int
    CommandId: int


class AsyncCommandTransaction:
    def __init__(self, future: Future, eventLoop, expectType: Type):
        self._event_loop = eventLoop
        self._future = future
        self._expect_type = expectType

    def _handleResponse(self, response: bytes):
        if self._expect_type:
            try:
                self._future.set_result(self._expect_type.FromTLV(response))
            except Exception as ex:
                self._handleError(
                    IMExceptions.InteractionModelState.Failure, 0, ex)
        else:
            self._future.set_result(None)

    def handleResponse(self, path: CommandPath, response: bytes):
        self._event_loop.call_soon_threadsafe(
            self._handleResponse, response)

    def _handleError(self, imError: int, chipError: int, exception: Exception):
        if exception:
            self._future.set_exception(exception)
        elif chipError != 0 and chipError != 0xCA:
            # 0xCA is CHIP_IM_STATUS_CODE_RECEIVED
            self._future.set_exception(
                ChipExceptions.ChipStackError(chipError))
        else:
            try:
                self._future.set_exception(
                    IMExceptions.InteractionModelError(IMExceptions.InteractionModelState(imError)))
            except:
                self._future.set_exception(IMExceptions.InteractionModelError(
                    IMExceptions.InteractionModelState.Failure))
        pass

    def handleError(self, imError: int, chipError: int):
        self._event_loop.call_soon_threadsafe(
            self._handleError, imError, chipError, None
        )


_OnCommandSenderResponseCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_void_p, c_uint32)
_OnCommandSenderErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32)
_OnCommandSenderDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnCommandSenderResponseCallbackFunct
def _OnCommandSenderResponseCallback(closure, endpoint: int, cluster: int, command: int, payload, size):
    data = ctypes.string_at(payload, size)
    closure.handleResponse(CommandPath(endpoint, cluster, command), data[:])


@_OnCommandSenderErrorCallbackFunct
def _OnCommandSenderErrorCallback(closure, imerror: int, chiperror: int):
    closure.handleError(imerror, chiperror)


@_OnCommandSenderDoneCallbackFunct
def _OnCommandSenderDoneCallback(closure):
    ctypes.pythonapi.Py_DecRef(ctypes.py_object(closure))


def SendCommand(future: Future, eventLoop, responseType: Type, device, commandPath: CommandPath, payload: ClusterCommand) -> int:
    if (responseType is not None) and (not issubclass(responseType, ClusterCommand)):
        raise ValueError("responseType must be a ClusterCommand or None")

    handle = chip.native.GetLibraryHandle()
    transaction = AsyncCommandTransaction(future, eventLoop, responseType)

    payloadTLV = payload.ToTLV()
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    return handle.pychip_CommandSender_SendCommand(ctypes.py_object(
        transaction), device, commandPath.EndpointId, commandPath.ClusterId, commandPath.CommandId, payloadTLV, len(payloadTLV))


_deviceController = None


def SetDeviceController(deviceCtrl):
    global _deviceController
    _deviceController = deviceCtrl


def GetDeviceController():
    global _deviceController
    return _deviceController


def Init(devCtrl):
    SetDeviceController(devCtrl)
    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_CommandSender_SendCommand.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_CommandSender_SendCommand',
                   c_uint32, [py_object, c_void_p, c_uint16, c_uint32, c_uint32, c_char_p, c_size_t])
        setter.Set('pychip_CommandSender_InitCallbacks', None, [
                   _OnCommandSenderResponseCallbackFunct, _OnCommandSenderErrorCallbackFunct, _OnCommandSenderDoneCallbackFunct])

    handle.pychip_CommandSender_InitCallbacks(
        _OnCommandSenderResponseCallback, _OnCommandSenderErrorCallback, _OnCommandSenderDoneCallback)
