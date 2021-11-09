#
#    Copyright (c) 2021 Project CHIP Authors
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
from typing import Type, Union, List, Any
from ctypes import CFUNCTYPE, c_char_p, c_size_t, c_void_p, c_uint32,  c_uint16, py_object

from .ClusterObjects import ClusterAttributeDescriptor
import chip.exceptions
import chip.interaction_model


@dataclass
class AttributePath:
    EndpointId: int
    ClusterId: int
    AttributeId: int


@dataclass
class AttributeStatus:
    Path: AttributePath
    Status: Union[chip.interaction_model.Status, int]


AttributeWriteResult = AttributeStatus


@dataclass
class AttributeDescriptorWithEndpoint:
    EndpointId: int
    Attribute: ClusterAttributeDescriptor


@dataclass
class AttributeWriteRequest(AttributeDescriptorWithEndpoint):
    Data: Any


AttributeReadRequest = AttributeDescriptorWithEndpoint


@dataclass
class AttributeReadResult(AttributeStatus):
    Data: Any = None


class AsyncWriteTransaction:
    def __init__(self, future: Future, eventLoop):
        self._event_loop = eventLoop
        self._future = future
        self._res = []

    def _handleResponse(self, path: AttributePath, status: int):
        try:
            imStatus = chip.interaction_model.Status(status)
            self._res.append(AttributeWriteResult(Path=path, Status=imStatus))
        except:
            self._res.append(AttributeWriteResult(Path=path, Status=status))

    def handleResponse(self, path: AttributePath, status: int):
        self._event_loop.call_soon_threadsafe(
            self._handleResponse, path, status)

    def _handleError(self, chipError: int):
        self._future.set_exception(
            chip.exceptions.ChipStackError(chipError))

    def handleError(self, chipError: int):
        self._event_loop.call_soon_threadsafe(
            self._handleError, chipError
        )

    def _handleDone(self):
        if not self._future.done():
            self._future.set_result(self._res)

    def handleDone(self):
        self._event_loop.call_soon_threadsafe(self._handleDone)


_OnWriteResponseCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint16)
_OnWriteErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32)
_OnWriteDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnWriteResponseCallbackFunct
def _OnWriteResponseCallback(closure, endpoint: int, cluster: int, attribute: int, status):
    closure.handleResponse(AttributePath(endpoint, cluster, attribute), status)


@_OnWriteErrorCallbackFunct
def _OnWriteErrorCallback(closure, chiperror: int):
    closure.handleError(chiperror)


@_OnWriteDoneCallbackFunct
def _OnWriteDoneCallback(closure):
    closure.handleDone()
    ctypes.pythonapi.Py_DecRef(ctypes.py_object(closure))


def WriteAttributes(future: Future, eventLoop, device, attributes: List[AttributeWriteRequest]) -> int:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncWriteTransaction(future, eventLoop)

    writeargs = []
    for attr in attributes:
        path = chip.interaction_model.AttributePathIBstruct.parse(
            b'\x00' * chip.interaction_model.AttributePathIBstruct.sizeof())
        path.EndpointId = attr.EndpointId
        path.ClusterId = attr.Attribute.cluster_id
        path.AttributeId = attr.Attribute.attribute_id
        path = chip.interaction_model.AttributePathIBstruct.build(path)
        tlv = attr.Attribute.ToTLV(None, attr.Data)
        writeargs.append(ctypes.c_char_p(path))
        writeargs.append(ctypes.c_char_p(bytes(tlv)))
        writeargs.append(ctypes.c_int(len(tlv)))

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    res = handle.pychip_WriteClient_WriteAttributes(
        ctypes.py_object(transaction), device, ctypes.c_size_t(len(attributes)), *writeargs)
    if res != 0:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def Init():
    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_WriteClient_InitCallbacks.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        handle.pychip_WriteClient_WriteAttributes.restype = c_uint32
        setter.Set('pychip_WriteClient_InitCallbacks', None, [
                   _OnWriteResponseCallbackFunct, _OnWriteErrorCallbackFunct, _OnWriteDoneCallbackFunct])

    handle.pychip_WriteClient_InitCallbacks(
        _OnWriteResponseCallback, _OnWriteErrorCallback, _OnWriteDoneCallback)
