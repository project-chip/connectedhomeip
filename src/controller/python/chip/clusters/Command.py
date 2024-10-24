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

import builtins
import ctypes
import inspect
import logging
import sys
from asyncio.futures import Future
from ctypes import CFUNCTYPE, POINTER, c_bool, c_char_p, c_size_t, c_uint8, c_uint16, c_uint32, c_void_p, cast, py_object
from dataclasses import dataclass
from typing import List, Optional, Type, Union

import chip.exceptions
import chip.interaction_model
from chip.interaction_model import PyInvokeRequestData, TestOnlyPyBatchCommandsOverrides, TestOnlyPyOnDoneInfo
from chip.native import PyChipError

from .ClusterObjects import ClusterCommand

logger = logging.getLogger('chip.cluster.Command')
logger.setLevel(logging.ERROR)


@dataclass
class CommandPath:
    EndpointId: int
    ClusterId: int
    CommandId: int


@dataclass
class InvokeRequestInfo:
    EndpointId: int
    Command: ClusterCommand
    ResponseType: Optional[Type] = None


@dataclass
class Status:
    IMStatus: int
    ClusterStatus: int


@dataclass
class TestOnlyBatchCommandResponse:
    Responses: object
    ResponseMessageCount: int


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
                                    if (field['cluster_id'].default == path.ClusterId) and (field['command_id'].default ==
                                                                                            path.CommandId) and (field['is_client'].default == isClientSideCommand):
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

    def handleResponse(self, path: CommandPath, index: int, status: Status, response: bytes):
        # For AsyncCommandTransaction we only expect to ever get one response so we don't bother
        # checking `index`. We just share a callback API with batch commands. If we ever get a
        # second call to `handleResponse` we will see a different error on trying to set future
        # that has already been set.
        self._event_loop.call_soon_threadsafe(
            self._handleResponse, path, status, response)

    def _handleError(self, imError: Status, chipError: PyChipError, exception: Exception):
        if exception:
            self._future.set_exception(exception)
        elif chipError != 0:
            self._future.set_exception(chipError.to_exception())
        else:
            try:
                # If you got an exception from this call other than AttributeError please
                # add it to the except block below. We changed Exception->AttributeError as
                # that is what we thought we are trying to catch here.
                self._future.set_exception(
                    chip.interaction_model.InteractionModelError(chip.interaction_model.Status(imError.IMStatus), imError.ClusterStatus))
            except AttributeError:
                logger.exception("Failed to map interaction model status received: %s. Remapping to Failure." % imError)
                self._future.set_exception(chip.interaction_model.InteractionModelError(
                    chip.interaction_model.Status.Failure, imError.ClusterStatus))

    def handleError(self, status: Status, chipError: PyChipError):
        self._event_loop.call_soon_threadsafe(
            self._handleError, status, chipError, None
        )

    def handleDone(self):
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))


class AsyncBatchCommandsTransaction:
    def __init__(self, future: Future, eventLoop, expectTypes: List[Type]):
        self._event_loop = eventLoop
        self._future = future
        self._expect_types = expectTypes
        default_im_failure = chip.interaction_model.InteractionModelError(
            chip.interaction_model.Status.NoCommandResponse)
        self._responses = [default_im_failure] * len(expectTypes)

    def _handleResponse(self, path: CommandPath, index: int, status: Status, response: bytes):
        if index > len(self._responses):
            self._handleError(status, 0, IndexError(f"CommandSenderCallback has given us an unexpected index value {index}"))
            return

        if status.IMStatus != chip.interaction_model.Status.Success:
            try:
                self._responses[index] = chip.interaction_model.InteractionModelError(
                    chip.interaction_model.Status(status.IMStatus), status.ClusterStatus)
            except AttributeError as ex:
                self._handleError(status, 0, ex)
        elif (len(response) == 0):
            self._responses[index] = None
        else:
            # If a type hasn't been assigned, let's auto-deduce it.
            if (self._expect_types[index] is None):
                self._expect_types[index] = FindCommandClusterObject(False, path)

            if self._expect_types[index]:
                try:
                    # If you got an exception from this call other than AttributeError please
                    # add it to the except block below. We changed Exception->AttributeError as
                    # that is what we thought we are trying to catch here.
                    self._responses[index] = self._expect_types[index].FromTLV(response)
                except AttributeError as ex:
                    self._handleError(status, 0, ex)
            else:
                self._responses[index] = None

    def handleResponse(self, path: CommandPath, index: int, status: Status, response: bytes):
        self._event_loop.call_soon_threadsafe(
            self._handleResponse, path, index, status, response)

    def _handleError(self, imError: Status, chipError: PyChipError, exception: Exception):
        if self._future.done():
            logger.exception(f"Recieved another error. Only expecting one error. imError:{imError}, chipError {chipError}")
            return
        if exception:
            self._future.set_exception(exception)
        elif chipError != 0:
            self._future.set_exception(chipError.to_exception())
        else:
            try:
                # If you got an exception from this call other than AttributeError please
                # add it to the except block below. We changed Exception->AttributeError as
                # that is what we thought we are trying to catch here.
                self._future.set_exception(
                    chip.interaction_model.InteractionModelError(chip.interaction_model.Status(imError.IMStatus), imError.ClusterStatus))
            except AttributeError:
                logger.exception("Failed to map interaction model status received: %s. Remapping to Failure." % imError)
                self._future.set_exception(chip.interaction_model.InteractionModelError(
                    chip.interaction_model.Status.Failure, imError.ClusterStatus))

    def handleError(self, status: Status, chipError: PyChipError):
        self._event_loop.call_soon_threadsafe(
            self._handleError, status, chipError, None
        )

    def _handleDone(self):
        # Future might already be set with exception from `handleError`
        if not self._future.done():
            self._future.set_result(self._responses)
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

    def handleDone(self):
        self._event_loop.call_soon_threadsafe(
            self._handleDone
        )


class TestOnlyAsyncBatchCommandsTransaction(AsyncBatchCommandsTransaction):
    def __init__(self, future: Future, eventLoop, expectTypes: List[Type]):
        self._responseMessageCount = 0
        super().__init__(future, eventLoop, expectTypes)

    def _handleDone(self):
        # Future might already be set with exception from `handleError`
        if not self._future.done():
            self._future.set_result(TestOnlyBatchCommandResponse(self._responses, self._responseMessageCount))
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

    def testOnlyDoneInfo(self, testOnlyDoneInfo: TestOnlyPyOnDoneInfo):
        self._responseMessageCount = testOnlyDoneInfo.responseMessageCount


_OnCommandSenderResponseCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_size_t, c_uint16, c_uint8, c_void_p, c_uint32)
_OnCommandSenderErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint8, PyChipError)
_OnCommandSenderDoneCallbackFunct = CFUNCTYPE(
    None, py_object)
_TestOnlyOnCommandSenderDoneCallbackFunct = CFUNCTYPE(
    None, py_object, TestOnlyPyOnDoneInfo)


@_OnCommandSenderResponseCallbackFunct
def _OnCommandSenderResponseCallback(closure, endpoint: int, cluster: int, command: int, index: int,
                                     imStatus: int, clusterStatus: int, payload, size):
    data = ctypes.string_at(payload, size)
    closure.handleResponse(CommandPath(endpoint, cluster, command), index, Status(
        imStatus, clusterStatus), data[:])


@_OnCommandSenderErrorCallbackFunct
def _OnCommandSenderErrorCallback(closure, imStatus: int, clusterStatus: int, chiperror: PyChipError):
    closure.handleError(Status(imStatus, clusterStatus), chiperror)


@_OnCommandSenderDoneCallbackFunct
def _OnCommandSenderDoneCallback(closure):
    closure.handleDone()


@_TestOnlyOnCommandSenderDoneCallbackFunct
def _TestOnlyOnCommandSenderDoneCallback(closure, testOnlyDoneInfo: TestOnlyPyOnDoneInfo):
    closure.testOnlyDoneInfo(testOnlyDoneInfo)
    closure.handleDone()


def TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(future: Future, eventLoop, responseType, device, commandPath, payload):
    ''' ONLY TO BE USED FOR TEST: Sends the payload with a TimedRequest flag but no TimedInvoke transaction
    '''
    if (responseType is not None) and (not issubclass(responseType, ClusterCommand)):
        raise ValueError("responseType must be a ClusterCommand or None")

    handle = chip.native.GetLibraryHandle()
    transaction = AsyncCommandTransaction(future, eventLoop, responseType)

    payloadTLV = payload.ToTLV()
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    return builtins.chipStack.Call(
        lambda: handle.pychip_CommandSender_TestOnlySendCommandTimedRequestNoTimedInvoke(
            ctypes.py_object(transaction), device,
            commandPath.EndpointId, commandPath.ClusterId, commandPath.CommandId, payloadTLV, len(payloadTLV),
            ctypes.c_uint16(0),  # interactionTimeoutMs
            ctypes.c_uint16(0),  # busyWaitMs
            ctypes.c_bool(False)  # suppressResponse
        ))


async def SendCommand(future: Future, eventLoop, responseType: Type, device, commandPath: CommandPath, payload: ClusterCommand,
                      timedRequestTimeoutMs: Union[None, int] = None, interactionTimeoutMs: Union[None, int] = None,
                      busyWaitMs: Union[None, int] = None, suppressResponse: Union[None, bool] = None) -> PyChipError:
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
        raise chip.interaction_model.InteractionModelError(chip.interaction_model.Status.NeedsTimedInteraction)

    handle = chip.native.GetLibraryHandle()
    transaction = AsyncCommandTransaction(future, eventLoop, responseType)

    payloadTLV = payload.ToTLV()
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_CommandSender_SendCommand(
            ctypes.py_object(transaction), device,
            c_uint16(0 if timedRequestTimeoutMs is None else timedRequestTimeoutMs), commandPath.EndpointId,
            commandPath.ClusterId, commandPath.CommandId, payloadTLV, len(payloadTLV),
            ctypes.c_uint16(0 if interactionTimeoutMs is None else interactionTimeoutMs),
            ctypes.c_uint16(0 if busyWaitMs is None else busyWaitMs),
            ctypes.c_bool(False if suppressResponse is None else suppressResponse)
        ))


def _BuildPyInvokeRequestData(commands: List[InvokeRequestInfo], timedRequestTimeoutMs: Optional[int], responseTypes, suppressTimedRequestMessage: bool = False) -> List[PyInvokeRequestData]:
    numberOfCommands = len(commands)
    pyBatchCommandsDataArrayType = PyInvokeRequestData * numberOfCommands
    pyBatchCommandsData = pyBatchCommandsDataArrayType()
    for idx, command in enumerate(commands):
        clusterCommand = command.Command
        responseType = command.ResponseType
        if (responseType is not None) and (not issubclass(responseType, ClusterCommand)):
            raise ValueError("responseType must be a ClusterCommand or None")
        if clusterCommand.must_use_timed_invoke and timedRequestTimeoutMs is None or timedRequestTimeoutMs == 0:
            if not suppressTimedRequestMessage:
                raise chip.interaction_model.InteractionModelError(chip.interaction_model.Status.NeedsTimedInteraction)

        payloadTLV = clusterCommand.ToTLV()

        pyBatchCommandsData[idx].commandPath.endpointId = c_uint16(command.EndpointId)
        pyBatchCommandsData[idx].commandPath.clusterId = c_uint32(clusterCommand.cluster_id)
        pyBatchCommandsData[idx].commandPath.commandId = c_uint32(clusterCommand.command_id)
        pyBatchCommandsData[idx].tlvData = cast(c_char_p(bytes(payloadTLV)), c_void_p)
        pyBatchCommandsData[idx].tlvLength = c_size_t(len(payloadTLV))

        responseTypes.append(responseType)

    return pyBatchCommandsData


async def SendBatchCommands(future: Future, eventLoop, device, commands: List[InvokeRequestInfo],
                            timedRequestTimeoutMs: Optional[int] = None, interactionTimeoutMs: Optional[int] = None,
                            busyWaitMs: Optional[int] = None, suppressResponse: Optional[bool] = None) -> PyChipError:
    ''' Initiates an InvokeInteraction with the batch commands provided.

    Arguments:
        - timedRequestTimeoutMs: If a valid value is provided, a timed interaction will be initiated.
        - interactionTimeoutMs: If a valid value is provided, the interaction will terminate with a
          CHIP_ERROR_TIMEOUT if a response is not received within the specified timeout. If not provided,
          a suitable value will be automatically computed based on transport characteristics and
          receiver responsiveness.

    Returns:
        - PyChipError: Indicates the outcome of initiating the InvokeRequest. Upon success the caller
          is expected to await on `future` to get result of the InvokeInteraction.

    Results passed via the provided future:
        - Successful InvokeInteraction with path-specific responses (including path-specific errors):
            - A list of responses is returned in the same order as the `commands` argument.
            - Possible response elements:
                - `None`: Successful command execution without additional cluster data.
                - Encapsulated cluster-object: Successful command with response data.
                - interaction_model.Status.*: Command failure with IM Status.
                - interaction_model.Status.NoCommandResponse: No response from the server for
                  a specific command.
        - Non-path-specific error: An `InteractionModelError` exception is raised through the future.
    '''
    handle = chip.native.GetLibraryHandle()

    responseTypes: List[Type] = []
    pyBatchCommandsData = _BuildPyInvokeRequestData(commands, timedRequestTimeoutMs, responseTypes)

    transaction = AsyncBatchCommandsTransaction(future, eventLoop, responseTypes)
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))

    return await builtins.chipStack.CallAsyncWithResult(
        lambda: handle.pychip_CommandSender_SendBatchCommands(
            py_object(transaction), device,
            c_uint16(0 if timedRequestTimeoutMs is None else timedRequestTimeoutMs),
            c_uint16(0 if interactionTimeoutMs is None else interactionTimeoutMs),
            c_uint16(0 if busyWaitMs is None else busyWaitMs),
            c_bool(False if suppressResponse is None else suppressResponse),
            pyBatchCommandsData, c_size_t(len(pyBatchCommandsData)))
    )


def TestOnlySendBatchCommands(future: Future, eventLoop, device, commands: List[InvokeRequestInfo],
                              timedRequestTimeoutMs: Optional[int] = None, interactionTimeoutMs: Optional[int] = None, busyWaitMs: Optional[int] = None,
                              suppressResponse: Optional[bool] = None, remoteMaxPathsPerInvoke: Optional[int] = None,
                              suppressTimedRequestMessage: bool = False, commandRefsOverride: Optional[List[int]] = None) -> PyChipError:
    ''' ONLY TO BE USED FOR TEST: Send batch commands using various overrides.
    '''
    if suppressTimedRequestMessage and timedRequestTimeoutMs is not None:
        raise ValueError("timedRequestTimeoutMs has non-None value while suppressTimedRequestMessage")

    overrideCommandRefs = None
    if commandRefsOverride is not None:
        if len(commandRefsOverride) != len(commands):
            raise ValueError("Mismatch in the number of elements provided in commandRefsOverride")
        overrideCommandRefsType = c_uint16 * len(commandRefsOverride)
        overrideCommandRefs = overrideCommandRefsType()

    handle = chip.native.GetLibraryHandle()

    responseTypes: List[Type] = []
    pyBatchCommandsData = _BuildPyInvokeRequestData(commands, timedRequestTimeoutMs,
                                                    responseTypes, suppressTimedRequestMessage=suppressTimedRequestMessage)

    transaction = TestOnlyAsyncBatchCommandsTransaction(future, eventLoop, responseTypes)
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))

    testOnlyOverrides = TestOnlyPyBatchCommandsOverrides()
    testOnlyOverrides.suppressTimedRequestMessage = suppressTimedRequestMessage
    testOnlyOverrides.overrideRemoteMaxPathsPerInvoke = 0 if remoteMaxPathsPerInvoke is None else c_uint16(remoteMaxPathsPerInvoke)
    testOnlyOverrides.overrideCommandRefsList = overrideCommandRefs
    testOnlyOverrides.overrideCommandRefsListLength = 0 if overrideCommandRefs is None else c_size_t(len(overrideCommandRefs))

    return builtins.chipStack.Call(
        lambda: handle.pychip_CommandSender_TestOnlySendBatchCommands(
            py_object(transaction), device,
            c_uint16(0 if timedRequestTimeoutMs is None else timedRequestTimeoutMs),
            c_uint16(0 if interactionTimeoutMs is None else interactionTimeoutMs),
            c_uint16(0 if busyWaitMs is None else busyWaitMs),
            c_bool(False if suppressResponse is None else suppressResponse),
            testOnlyOverrides,
            pyBatchCommandsData, c_size_t(len(pyBatchCommandsData)))
    )


def SendGroupCommand(groupId: int, devCtrl: c_void_p, payload: ClusterCommand, busyWaitMs: Union[None, int] = None) -> PyChipError:
    ''' Send a cluster-object encapsulated group command to a device and does the following:
            - None (on a successful response containing no data)
            - Raises an exception if any errors are encountered.
    '''
    handle = chip.native.GetLibraryHandle()

    payloadTLV = payload.ToTLV()
    return builtins.chipStack.Call(
        lambda: handle.pychip_CommandSender_SendGroupCommand(
            c_uint16(groupId), devCtrl,
            payload.cluster_id, payload.command_id, payloadTLV, len(payloadTLV),
            ctypes.c_uint16(0 if busyWaitMs is None else busyWaitMs),
        ))


def Init():
    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_CommandSender_SendCommand.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_CommandSender_SendCommand',
                   PyChipError, [py_object, c_void_p, c_uint16, c_uint16, c_uint32, c_uint32, c_char_p, c_size_t, c_uint16, c_uint16, c_bool])
        setter.Set('pychip_CommandSender_SendBatchCommands',
                   PyChipError, [py_object, c_void_p, c_uint16, c_uint16, c_uint16, c_bool, POINTER(PyInvokeRequestData), c_size_t])
        setter.Set('pychip_CommandSender_TestOnlySendBatchCommands',
                   PyChipError, [py_object, c_void_p, c_uint16, c_uint16, c_uint16, c_bool, TestOnlyPyBatchCommandsOverrides, POINTER(PyInvokeRequestData), c_size_t])
        setter.Set('pychip_CommandSender_TestOnlySendCommandTimedRequestNoTimedInvoke',
                   PyChipError, [py_object, c_void_p, c_uint16, c_uint32, c_uint32, c_char_p, c_size_t, c_uint16, c_uint16, c_bool])
        setter.Set('pychip_CommandSender_SendGroupCommand',
                   PyChipError, [c_uint16, c_void_p, c_uint32, c_uint32, c_char_p, c_size_t, c_uint16])
        setter.Set('pychip_CommandSender_InitCallbacks', None, [
                   _OnCommandSenderResponseCallbackFunct, _OnCommandSenderErrorCallbackFunct, _OnCommandSenderDoneCallbackFunct, _TestOnlyOnCommandSenderDoneCallbackFunct])

    handle.pychip_CommandSender_InitCallbacks(
        _OnCommandSenderResponseCallback, _OnCommandSenderErrorCallback, _OnCommandSenderDoneCallback, _TestOnlyOnCommandSenderDoneCallback)
