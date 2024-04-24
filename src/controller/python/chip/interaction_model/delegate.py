'''
   Copyright (c) 2021 Project CHIP Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
'''

import ctypes
import threading
import typing
from ctypes import CFUNCTYPE, POINTER, c_uint8, c_uint32, c_uint64, c_void_p
from dataclasses import dataclass

import chip.exceptions
import chip.native
import chip.tlv
from construct import Int8ul, Int16ul, Int32ul, Int64ul, Struct

# The type should match CommandStatus in interaction_model/Delegate.h
# CommandStatus should not contain padding
IMCommandStatus = Struct(
    "Status" / Int8ul,
    "ClusterStatus" / Int8ul,
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "CommandId" / Int32ul,
    "CommandIndex" / Int8ul,
)

# The type should match WriteStatus in interaction_model/Delegate.h
IMWriteStatus = Struct(
    "NodeId" / Int64ul,
    "AppIdentifier" / Int64ul,
    "Status" / Int8ul,
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "AttributeId" / Int32ul,
)

# AttributePath should not contain padding
AttributePathIBstruct = Struct(
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "AttributeId" / Int32ul,
    "DataVersion" / Int32ul,
    "HasDataVersion" / Int8ul,
)

# EventPath should not contain padding
EventPathIBstruct = Struct(
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "EventId" / Int32ul,
    "Urgent" / Int8ul,
)

DataVersionFilterIBstruct = Struct(
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "DataVersion" / Int32ul,
)

SessionParametersStruct = Struct(
    "SessionIdleInterval" / Int32ul,
    "SessionActiveInterval" / Int32ul,
    "SessionActiveThreshold" / Int16ul,
    "DataModelRevision" / Int16ul,
    "InteractionModelRevision" / Int16ul,
    "SpecificationVersion" / Int32ul,
    "MaxPathsPerInvoke" / Int16ul,
)


@dataclass
class AttributePath:
    nodeId: int
    endpointId: int
    clusterId: int
    attributeId: int


@dataclass
class EventPath:
    nodeId: int
    endpointId: int
    clusterId: int
    eventId: int
    urgent: int


@dataclass
class AttributeReadResult:
    path: AttributePath
    status: int
    value: 'typing.Any'
    dataVersion: int


@dataclass
class EventReadResult:
    path: EventPath
    value: 'typing.Any'


@dataclass
class AttributeWriteResult:
    path: AttributePath
    status: int


@dataclass
class SessionParameters:
    sessionIdleInterval: typing.Optional[int]
    sessionActiveInterval: typing.Optional[int]
    sessionActiveThreshold: typing.Optional[int]
    dataModelRevision: typing.Optional[int]
    interactionModelRevision: typing.Optional[int]
    specficiationVersion: typing.Optional[int]
    maxPathsPerInvoke: int


class PyCommandPath(ctypes.Structure):
    ''' InvokeRequest Path struct that has c++ counterpart for CFFI.

    We are using the following struct for passing the information of InvokeRequest between Python and C++:

    ```c
    struct PyCommandPath
    {
        chip::EndpointId endpointId;
        chip::ClusterId clusterId;
        chip::CommandId commandId;
    };
    ```
    '''
    _fields_ = [('endpointId', ctypes.c_uint16), ('clusterId', ctypes.c_uint32), ('commandId', ctypes.c_uint32)]


class PyInvokeRequestData(ctypes.Structure):
    ''' InvokeRequest struct that has c++ counterpart for CFFI.

    We are using the following struct for passing the information of InvokeRequest between Python and C++:

    ```c
    struct PyInvokeRequestData
    {
        PyCommandPath commandPath;
        void * tlvData;
        size_t tlvLength;
    };
    ```
    '''
    _fields_ = [('commandPath', PyCommandPath), ('tlvData', ctypes.c_void_p), ('tlvLength', ctypes.c_size_t)]


class PyAttributePath(ctypes.Structure):
    ''' Attributed Path struct that has c++ counterpart for CFFI.

    We are using the following struct for passing the information of WriteAttributes between Python and C++:

    ```c
    struct PyAttributePath
    {
        chip::EndpointId endpointId;
        chip::ClusterId clusterId;
        chip::AttributeId attributeId;
        chip::DataVersion dataVersion;
        uint8_t hasDataVersion;
    };
    ```
    '''
    _fields_ = [('endpointId', ctypes.c_uint16), ('clusterId', ctypes.c_uint32), ('attributeId',
                                                                                  ctypes.c_uint32), ('dataVersion', ctypes.c_uint32), ('hasDataVersion', ctypes.c_uint8)]


class PyWriteAttributeData(ctypes.Structure):
    ''' WriteAttribute struct that has c++ counterpart for CFFI.

    We are using the following struct for passing the information of WriteAttributes between Python and C++:

    ```c
    struct PyWriteAttributeData
    {
        PyAttributePath attributePath;
        void * tlvData;
        size_t tlvLength;
    };
    ```
    '''
    _fields_ = [('attributePath', PyAttributePath), ('tlvData', ctypes.c_void_p), ('tlvLength', ctypes.c_size_t)]


class TestOnlyPyBatchCommandsOverrides(ctypes.Structure):
    ''' TestOnly struct for overriding aspects of batch command to send invalid commands.

    We are using the following struct for passing the information of TestOnlyPyBatchCommandsOverrides between Python and C++:

    ```c
    struct TestOnlyPyBatchCommandsOverrides
    {
        uint16_t overrideRemoteMaxPathsPerInvoke;
        bool suppressTimedRequestMessage;
        uint16_t * overrideCommandRefsList;
        size_t overrideCommandRefsListLength;
    };
    ```
    '''
    _fields_ = [('overrideRemoteMaxPathsPerInvoke', ctypes.c_uint16), ('suppressTimedRequestMessage', ctypes.c_bool),
                ('overrideCommandRefsList', POINTER(ctypes.c_uint16)), ('overrideCommandRefsListLength', ctypes.c_size_t)]


class TestOnlyPyOnDoneInfo(ctypes.Structure):
    ''' TestOnly struct for overriding aspects of batch command to send invalid commands.

    We are using the following struct for passing the information of TestOnlyPyBatchCommandsOverrides between Python and C++:

    ```c
    struct TestOnlyPyOnDoneInfo
    {
        size_t responseMessageCount;
    };
    ```
    '''
    _fields_ = [('responseMessageCount', ctypes.c_size_t)]


# typedef void (*PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct)(uint64_t commandSenderPtr,
#                                                                                         void * commandStatusBuf);
# typedef void (*PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct)(uint64_t commandSenderPtr,
#                                                                                    uint8_t commandIndex);
# typedef void (*PythonInteractionModelDelegate_OnCommandResponseFunct)(uint64_t commandSenderPtr, uint32_t error);
_OnCommandResponseStatusCodeReceivedFunct = CFUNCTYPE(
    None, c_uint64, c_void_p, c_uint32)
_OnCommandResponseProtocolErrorFunct = CFUNCTYPE(None, c_uint64, c_uint8)
_OnCommandResponseFunct = CFUNCTYPE(None, c_uint64, c_uint32)
_OnWriteResponseStatusFunct = CFUNCTYPE(None, c_void_p, c_uint32)

_commandStatusDict = dict()
_commandIndexStatusDict = dict()
_commandStatusLock = threading.RLock()
_commandStatusCV = threading.Condition(_commandStatusLock)

_attributeDict = dict()
_attributeDictLock = threading.RLock()

_writeStatusDict = dict()
_writeStatusDictLock = threading.RLock()

# A placeholder commandHandle, will be removed once we decouple CommandSender with CHIPClusters
PLACEHOLDER_COMMAND_HANDLE = 1
DEFAULT_ATTRIBUTEREAD_APPID = 0
DEFAULT_ATTRIBUTEWRITE_APPID = 0


def _GetCommandStatus(commandHandle: int):
    with _commandStatusLock:
        return _commandStatusDict.get(commandHandle, None)


def _GetCommandIndexStatus(commandHandle: int, commandIndex: int):
    with _commandStatusLock:
        indexDict = _commandIndexStatusDict.get(commandHandle, {})
        return indexDict.get(commandIndex, None)


def _SetCommandStatus(commandHandle: int, val):
    with _commandStatusLock:
        _commandStatusDict[commandHandle] = val
        _commandStatusCV.notify_all()


def _SetCommandIndexStatus(commandHandle: int, commandIndex: int, status):
    with _commandStatusLock:
        indexDict = _commandIndexStatusDict.get(commandHandle, {})
        indexDict[commandIndex] = status
        _commandIndexStatusDict[commandHandle] = indexDict


@ _OnCommandResponseStatusCodeReceivedFunct
def _OnCommandResponseStatusCodeReceived(commandHandle: int, IMCommandStatusBuf, IMCommandStatusBufLen):
    status = IMCommandStatus.parse(ctypes.string_at(
        IMCommandStatusBuf, IMCommandStatusBufLen))
    _SetCommandIndexStatus(PLACEHOLDER_COMMAND_HANDLE,
                           status["CommandIndex"], status)


@ _OnCommandResponseProtocolErrorFunct
def _OnCommandResponseProtocolError(commandHandle: int, errorcode: int):
    pass


@ _OnCommandResponseFunct
def _OnCommandResponse(commandHandle: int, errorcode: int):
    _SetCommandStatus(PLACEHOLDER_COMMAND_HANDLE, errorcode)


@_OnWriteResponseStatusFunct
def _OnWriteResponseStatus(IMAttributeWriteResult, IMAttributeWriteResultLen):
    status = IMWriteStatus.parse(ctypes.string_at(
        IMAttributeWriteResult, IMAttributeWriteResultLen))

    appId = status["AppIdentifier"]
    if appId < 256:
        # For all attribute write requests using CHIPCluster API, appId is filled by CHIPDevice,
        # and should be smaller than 256 (UINT8_MAX).
        appId = DEFAULT_ATTRIBUTEWRITE_APPID

    with _writeStatusDictLock:
        _writeStatusDict[appId] = AttributeWriteResult(AttributePath(
            status["NodeId"], status["EndpointId"], status["ClusterId"], status["AttributeId"]), status["Status"])


def InitIMDelegate():
    handle = chip.native.GetLibraryHandle()
    if not handle.pychip_InteractionModelDelegate_SetCommandResponseStatusCallback.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)
        setter.Set("pychip_InteractionModelDelegate_SetCommandResponseStatusCallback", None, [
                   _OnCommandResponseStatusCodeReceivedFunct])
        setter.Set("pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback", None, [
                   _OnCommandResponseProtocolErrorFunct])
        setter.Set("pychip_InteractionModelDelegate_SetCommandResponseErrorCallback", None, [
                   _OnCommandResponseFunct])
        setter.Set("pychip_InteractionModel_GetCommandSenderHandle",
                   c_uint32, [ctypes.POINTER(c_uint64)])
        setter.Set("pychip_InteractionModelDelegate_SetOnWriteResponseStatusCallback", None, [
                   _OnWriteResponseStatusFunct])

        handle.pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
            _OnCommandResponseStatusCodeReceived)
        handle.pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
            _OnCommandResponseProtocolError)
        handle.pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(
            _OnCommandResponse)
        handle.pychip_InteractionModelDelegate_SetOnWriteResponseStatusCallback(
            _OnWriteResponseStatus)


def ClearCommandStatus(commandHandle: int):
    """
    Clear internal state and prepare for next command, should be called before sending commands.
    """
    with _commandStatusLock:
        _SetCommandStatus(commandHandle, None)
        _commandIndexStatusDict[commandHandle] = {}


def WaitCommandStatus(commandHandle: int):
    """
    Wait for response from device, returns error code.
    ClearCommandStatus should be called before sending command or it will
    return result from last command sent.
    """
    # commandHandle is null, means we are not using IM
    # Note: This should be an error after we fully switched to IM.
    if commandHandle == 0:
        return None
    with _commandStatusCV:
        ret = _GetCommandStatus(commandHandle)
        while ret is None:
            _commandStatusCV.wait()
            ret = _GetCommandStatus(commandHandle)
        return ret


def WaitCommandIndexStatus(commandHandle: int, commandIndex: int):
    """
    Wait for response of particular command from device, returns error code and struct of response info.
    When device returns a command instead of command status, the response info is None.
    ClearCommandStatus should be called before sending command or it will
    return result from last command sent.
    """
    # commandHandle is null, means we are not using IM
    # Note: This should be an error after we fully switched to IM.
    if commandHandle == 0:
        return (0, None)
    err = WaitCommandStatus(commandHandle)
    return (err, _GetCommandIndexStatus(commandHandle, commandIndex))


def GetCommandSenderHandle() -> int:
    handle = chip.native.GetLibraryHandle()
    resPointer = c_uint64()
    res = handle.pychip_InteractionModel_GetCommandSenderHandle(
        ctypes.pointer(resPointer))
    if res != 0:
        raise chip.exceptions.ChipStackError(res)
    ClearCommandStatus(resPointer.value)
    return resPointer.value


def GetAttributeReadResponse(appId: int) -> AttributeReadResult:
    with _attributeDictLock:
        return _attributeDict.get(appId, None)


def GetAttributeWriteResponse(appId: int) -> AttributeWriteResult:
    with _writeStatusDictLock:
        return _writeStatusDict.get(appId, None)
