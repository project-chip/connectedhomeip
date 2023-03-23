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
from ctypes import CFUNCTYPE, c_char_p, c_size_t, c_void_p, c_uint32, c_uint16, c_uint8, py_object

from construct.core import ValidationError

from .ClusterObjects import ClusterCommand
import chip.exceptions
import chip.interaction_model
from chip.native import PyChipError

import inspect
import sys
import builtins
import logging


logger = logging.getLogger('chip.cluster.Command')
logger.setLevel(logging.ERROR)


@dataclass
class CommandPath:
    EndpointId: int
    ClusterId: int
    CommandId: int


@dataclass
class Status:
    IMStatus: int
    ClusterStatus: int


def FindCommandClusterObject(isClientSideCommand: bool, path: CommandPath):
    ''' Locates the right generated cluster object given a set of parameters.

        isClientSideCommand: True if it is a client-to-server command, else False.
        path: A CommandPath that describes the endpoint, cluster and ID of the command.

        Returns the type of the cluster object if one is found. Otherwise, returns None.
    '''
    for clusterName, obj in inspect.getmembers(sys.modules['chip.clusters.Objects']):
        if ('chip.clusters.Objects' in str(obj)) and inspect.isclass(obj):
            for objName, subclass in inspect.getmembers(obj):
                if inspect.isclass(subclass) and (('Commands') in str(subclass)):
                    for commandName, command in inspect.getmembers(subclass):
                        if inspect.isclass(command):
                            for name, field in inspect.getmembers(command):
                                if ('__dataclass_fields__' in name):
                                    if (field['cluster_id'].default == path.ClusterId) and (field['command_id'].default == path.CommandId) and (field['is_client'].default == isClientSideCommand):
                                        return eval('chip.clusters.Objects.' + clusterName + '.Commands.' + commandName)
    return None


class AsyncCommandTransaction:
    def __init__(self, future: Future, eventLoop, expectType: Type):
        self._event_loop = eventLoop
        self._future = future
        self._expect_type = expectType

    def _handleResponse(self, path: CommandPath, status: Status, response: bytes):
        if (len(response) == 0):
            self._future.set_result(None)
        else:
            # If a type hasn't been assigned, let's auto-deduce it.
            if (self._expect_type is None):
                self._expect_type = FindCommandClusterObject(False, path)

            if self._expect_type:
                try:
                    self._future.set_result(
                        self._expect_type.FromTLV(response))
                except Exception as ex:
                    self._handleError(
                        status, 0, ex)
            else:
                self._future.set_result(None)

    def handleResponse(self, path: CommandPath, status: Status, response: bytes):
        self._event_loop.call_soon_threadsafe(
            self._handleResponse, path, status, response)

    def _handleError(self, imError: Status, chipError: PyChipError, exception: Exception):
        if exception:
            self._future.set_exception(exception)
        elif chipError != 0:
            self._future.set_exception(chipError.to_exception())
        else:
            try:
                self._future.set_exception(
                    chip.interaction_model.InteractionModelError(chip.interaction_model.Status(imError.IMStatus)))
            except Exception as e2:
                logger.exception("Failed to map interaction model status received: %s. Remapping to Failure." % imError)
                self._future.set_exception(chip.interaction_model.InteractionModelError(
                    chip.interaction_model.Status.Failure))

    def handleError(self, status: Status, chipError: PyChipError):
        self._event_loop.call_soon_threadsafe(
            self._handleError, status, chipError, None
        )


_OnCommandSenderResponseCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint16, c_uint8, c_void_p, c_uint32)
_OnCommandSenderErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint8, PyChipError)
_OnCommandSenderDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnCommandSenderResponseCallbackFunct
def _OnCommandSenderResponseCallback(closure, endpoint: int, cluster: int, command: int, imStatus: int, clusterStatus: int, payload, size):
    data = ctypes.string_at(payload, size)
    closure.handleResponse(CommandPath(endpoint, cluster, command), Status(
        imStatus, clusterStatus), data[:])


@_OnCommandSenderErrorCallbackFunct
def _OnCommandSenderErrorCallback(closure, imStatus: int, clusterStatus: int, chiperror: PyChipError):
    closure.handleError(Status(imStatus, clusterStatus), chiperror)


@_OnCommandSenderDoneCallbackFunct
def _OnCommandSenderDoneCallback(closure):
    ctypes.pythonapi.Py_DecRef(ctypes.py_object(closure))


def SendCommand(future: Future, eventLoop, responseType: Type, device, commandPath: CommandPath, payload: ClusterCommand, timedRequestTimeoutMs: int = None, interactionTimeoutMs: int = None) -> PyChipError:
    ''' Send a cluster-object encapsulated command to a device and does the following:
            - On receipt of a successful data response, returns the cluster-object equivalent through the provided future.
            - None (on a successful response containing no data)
            - Raises an exception if any errors are encountered.

        If no response type is provided above, the type will be automatically deduced.

        If a valid timedRequestTimeoutMs is provided, a timed interaction will be initiated instead.
        If a valid interactionTimeoutMs is provided, the interaction will terminate with a CHIP_ERROR_TIMEOUT if a response
        has not been received within that timeout. If it isn't provided, a sensible value will be automatically computed that
        accounts for the underlying characteristics of both the transport and the responsiveness of the receiver.
    '''
    if (responseType is not None) and (not issubclass(responseType, ClusterCommand)):
        raise ValueError("responseType must be a ClusterCommand or None")
    if payload.must_use_timed_invoke and timedRequestTimeoutMs is None or timedRequestTimeoutMs == 0:
        raise ValueError(
            f"Command {payload.__class__} must use timed invoke, please specify a valid timedRequestTimeoutMs value")

    handle = chip.native.GetLibraryHandle()
    transaction = AsyncCommandTransaction(future, eventLoop, responseType)

    payloadTLV = payload.ToTLV()
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    return builtins.chipStack.Call(
        lambda: handle.pychip_CommandSender_SendCommand(ctypes.py_object(
            transaction), device, c_uint16(0 if timedRequestTimeoutMs is None else timedRequestTimeoutMs), commandPath.EndpointId, commandPath.ClusterId, commandPath.CommandId, payloadTLV, len(payloadTLV), ctypes.c_uint16(0 if interactionTimeoutMs is None else interactionTimeoutMs)))


def Init():
    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_CommandSender_SendCommand.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_CommandSender_SendCommand',
                   PyChipError, [py_object, c_void_p, c_uint16, c_uint32, c_uint32, c_char_p, c_size_t, c_uint16])
        setter.Set('pychip_CommandSender_InitCallbacks', None, [
                   _OnCommandSenderResponseCallbackFunct, _OnCommandSenderErrorCallbackFunct, _OnCommandSenderDoneCallbackFunct])

    handle.pychip_CommandSender_InitCallbacks(
        _OnCommandSenderResponseCallback, _OnCommandSenderErrorCallback, _OnCommandSenderDoneCallback)
