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

from construct import Struct, Int32ul, Int16ul, Int8ul
from ctypes import CFUNCTYPE, c_void_p, c_size_t, c_uint32, c_uint64, c_uint8, c_uint16, c_ssize_t
import ctypes
import chip.native
import threading
import chip.tlv
import chip.exceptions
import typing
from dataclasses import dataclass

# The type should match CommandStatus in interaction_model/Delegate.h
# CommandStatus should not contain padding
IMCommandStatus = Struct(
    "ProtocolId" /  Int32ul,
    "ProtocolCode" / Int16ul,
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "CommandId" / Int32ul,
    "CommandIndex" / Int8ul,
)

# AttributePath should not contain padding
AttributePathStruct = Struct(
    "EndpointId" / Int16ul,
    "ClusterId" / Int32ul,
    "AttributeId" / Int32ul,
)


@dataclass
class AttributePath:
    nodeId: int
    endpointId: int
    clusterId: int
    attributeId: int

@dataclass
class AttributeReadResult:
    path: AttributePath
    status: int
    value: 'typing.Any'

# typedef void (*PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct)(uint64_t commandSenderPtr,
#                                                                                         void * commandStatusBuf);
# typedef void (*PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct)(uint64_t commandSenderPtr, uint8_t commandIndex);
# typedef void (*PythonInteractionModelDelegate_OnCommandResponseFunct)(uint64_t commandSenderPtr, uint32_t error);
# typedef void (*PythonInteractionModelDelegate_OnReportDataFunct)(chip::NodeId nodeId, uint64_t readClientAppIdentifier,
#                                                                  void * attributePathBuf, size_t attributePathBufLen,
#                                                                  uint8_t * readTlvData, size_t readTlvDataLen, uint16_t statusCode);
_OnCommandResponseStatusCodeReceivedFunct = CFUNCTYPE(None, c_uint64, c_void_p, c_uint32)
_OnCommandResponseProtocolErrorFunct = CFUNCTYPE(None, c_uint64, c_uint8)
_OnCommandResponseFunct = CFUNCTYPE(None, c_uint64, c_uint32)
_OnReportDataFunct = CFUNCTYPE(None, c_uint64, c_ssize_t, c_void_p, c_uint32, c_void_p, c_uint32, c_uint16)

_commandStatusDict = dict()
_commandIndexStatusDict = dict()
_commandStatusLock = threading.RLock()
_commandStatusCV = threading.Condition(_commandStatusLock)

_attributeDict = dict()
_attributeDictLock = threading.RLock()

# A placeholder commandHandle, will be removed once we decouple CommandSender with CHIPClusters
PLACEHOLDER_COMMAND_HANDLE = 1
DEFAULT_ATTRIBUTEREAD_APPID = 0

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
        print("SetCommandIndexStatus commandHandle={} commandIndex={}".format(commandHandle, commandIndex))
        indexDict = _commandIndexStatusDict.get(commandHandle, {})
        indexDict[commandIndex] = status
        _commandIndexStatusDict[commandHandle] = indexDict

@_OnCommandResponseStatusCodeReceivedFunct
def _OnCommandResponseStatusCodeReceived(commandHandle: int, IMCommandStatusBuf, IMCommandStatusBufLen):
    status = IMCommandStatus.parse(ctypes.string_at(IMCommandStatusBuf, IMCommandStatusBufLen))
    _SetCommandIndexStatus(PLACEHOLDER_COMMAND_HANDLE, status["CommandIndex"], status)

@_OnCommandResponseProtocolErrorFunct
def _OnCommandResponseProtocolError(commandHandle: int, errorcode: int):
    pass

@_OnCommandResponseFunct
def _OnCommandResponse(commandHandle: int, errorcode: int):
    _SetCommandStatus(PLACEHOLDER_COMMAND_HANDLE, errorcode)


@_OnReportDataFunct
def _OnReportData(nodeId: int, appId: int, attrPathBuf, attrPathBufLen: int, tlvDataBuf, tlvDataBufLen: int, statusCode: int):
    attrPath = AttributePathStruct.parse(
        ctypes.string_at(attrPathBuf, attrPathBufLen))
    tlvData = None
    path = AttributePath(nodeId, attrPath["EndpointId"],
                         attrPath["ClusterId"], attrPath["AttributeId"])
    if tlvDataBufLen > 0:
        tlvBuf = ctypes.string_at(tlvDataBuf, tlvDataBufLen)
        # We converts the data to AnonymousTag, and it becomes Any in decoded values.
        tlvData = chip.tlv.TLVReader(tlvBuf).get().get('Any')

    if appId < 256:
        # For all attribute read requests using CHIPCluster API, appId is filled by CHIPDevice, and should be smaller than 256 (UINT8_MAX).
        appId = DEFAULT_ATTRIBUTEREAD_APPID

    with _attributeDictLock:
        _attributeDict[appId] = AttributeReadResult(
            path, statusCode, tlvData)

def InitIMDelegate():
    handle = chip.native.GetLibraryHandle()
    if not handle.pychip_InteractionModelDelegate_SetCommandResponseStatusCallback.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)
        setter.Set("pychip_InteractionModelDelegate_SetCommandResponseStatusCallback", None, [_OnCommandResponseStatusCodeReceivedFunct])
        setter.Set("pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback", None, [_OnCommandResponseProtocolErrorFunct])
        setter.Set("pychip_InteractionModelDelegate_SetCommandResponseErrorCallback", None, [_OnCommandResponseFunct])
        setter.Set("pychip_InteractionModel_GetCommandSenderHandle", c_uint32, [ctypes.POINTER(c_uint64)])
        setter.Set("pychip_InteractionModelDelegate_SetOnReportDataCallback", None, [_OnReportDataFunct])

        handle.pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(_OnCommandResponseStatusCodeReceived)
        handle.pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(_OnCommandResponseProtocolError)
        handle.pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(_OnCommandResponse)
        handle.pychip_InteractionModelDelegate_SetOnReportDataCallback(_OnReportData)

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

def GetCommandSenderHandle()->int:
    handle = chip.native.GetLibraryHandle()
    resPointer = c_uint64()
    res = handle.pychip_InteractionModel_GetCommandSenderHandle(ctypes.pointer(resPointer))
    if res != 0:
        raise chip.exceptions.ChipStackError(res)
    ClearCommandStatus(resPointer.value)
    return resPointer.value

def GetAttributeReadResponse(appId: int) -> AttributeReadResult:
    with _attributeDictLock:
        return _attributeDict.get(appId, None)
