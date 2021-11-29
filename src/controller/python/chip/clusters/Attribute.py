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
import chip.tlv

import inspect
import sys
import logging


@dataclass
class AttributePath:
    EndpointId: int = None
    ClusterId: int = None
    AttributeId: int = None

    def __init__(self, EndpointId: int = None, Cluster=None, Attribute=None, ClusterId=None, AttributeId=None):
        self.EndpointId = EndpointId
        if Cluster is not None:
            # Wildcard read for a specific cluster
            if (Attribute is not None) or (ClusterId is not None) or (AttributeId is not None):
                raise Warning(
                    "Attribute, ClusterId and AttributeId is ignored when Cluster is specified")
            self.ClusterId = Cluster.id
            return
        if Attribute is not None:
            if (ClusterId is not None) or (AttributeId is not None):
                raise Warning(
                    "ClusterId and AttributeId is ignored when Attribute is specified")
            self.ClusterId = Attribute.cluster_id
            self.AttributeId = Attribute.attribute_id
            return
        self.ClusterId = ClusterId
        self.AttributeId = AttributeId

    def __str__(self) -> str:
        return f"{self.EndpointId}/{self.ClusterId}/{self.AttributeId}"


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


_AttributeIndex = {}


def _BuildAttributeIndex():
    ''' Build internal attribute index for locating the corresponding cluster object by path in the future.
    We do this because this operation will take a long time when there are lots of attributes, it takes about 300ms for a single query.
    This is acceptable during init, but unacceptable when the server returns lots of attributes at the same time.
    '''
    for clusterName, obj in inspect.getmembers(sys.modules['chip.clusters.Objects']):
        if ('chip.clusters.Objects' in str(obj)) and inspect.isclass(obj):
            for objName, subclass in inspect.getmembers(obj):
                if inspect.isclass(subclass) and (('Attributes') in str(subclass)):
                    for attributeName, attribute in inspect.getmembers(subclass):
                        if inspect.isclass(attribute):
                            base_classes = inspect.getmro(attribute)

                            # Only match on classes that extend the ClusterAttributeDescriptor class
                            matched = [
                                value for value in base_classes if 'ClusterAttributeDescriptor' in str(value)]
                            if (matched == []):
                                continue

                            _AttributeIndex[str(AttributePath(ClusterId=attribute.cluster_id, AttributeId=attribute.attribute_id))] = eval(
                                'chip.clusters.Objects.' + clusterName + '.Attributes.' + attributeName)


class AsyncReadTransaction:
    def __init__(self, future: Future, eventLoop):
        self._event_loop = eventLoop
        self._future = future
        self._res = []

    def _handleAttributeData(self, path: AttributePath, status: int, data: bytes):
        try:
            imStatus = status
            try:
                imStatus = chip.interaction_model.Status(status)
            except:
                pass
            attributeType = _AttributeIndex.get(str(AttributePath(
                ClusterId=path.ClusterId, AttributeId=path.AttributeId)), None)
            attributeValue = None
            if attributeType is None:
                attributeValue = chip.tlv.TLVReader(data).get().get("Any", {})
            else:
                try:
                    attributeValue = attributeType.FromTLV(data)
                except:
                    logging.error(
                        f"Error convering TLV to Cluster Object for path: Endpoint = {path.EndpointId}/Cluster = {path.ClusterId}/Attribute = {path.AttributeId}")
                    logging.error(
                        f"Failed Cluster Object: {str(attributeType)}")
                    raise

            self._res.append(AttributeReadResult(
                Path=path, Status=imStatus, Data=attributeType(attributeValue)))
        except Exception as ex:
            logging.exception(ex)

    def handleAttributeData(self, path: AttributePath, status: int, data: bytes):
        self._event_loop.call_soon_threadsafe(
            self._handleAttributeData, path, status, data)

    def _handleError(self, chipError: int):
        self._future.set_exception(
            chip.exceptions.ChipStackError(chipError))

    def handleError(self, chipError: int):
        self._event_loop.call_soon_threadsafe(
            self._handleError, chipError
        )

    def _handleDone(self, asd):
        if not self._future.done():
            self._future.set_result(self._res)

    def handleDone(self):
        self._event_loop.call_soon_threadsafe(self._handleDone, "asdasa")


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


_OnReadAttributeDataCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint32, c_void_p, c_size_t)
_OnReadErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32)
_OnReadDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnReadAttributeDataCallbackFunct
def _OnReadAttributeDataCallback(closure, endpoint: int, cluster: int, attribute: int, status, data, len):
    dataBytes = ctypes.string_at(data, len)
    closure.handleAttributeData(AttributePath(
        EndpointId=endpoint, ClusterId=cluster, AttributeId=attribute), status, dataBytes[:])


@_OnReadErrorCallbackFunct
def _OnReadErrorCallback(closure, chiperror: int):
    closure.handleError(chiperror)


@_OnReadDoneCallbackFunct
def _OnReadDoneCallback(closure):
    closure.handleDone()
    ctypes.pythonapi.Py_DecRef(ctypes.py_object(closure))


_OnWriteResponseCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint16)
_OnWriteErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32)
_OnWriteDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnWriteResponseCallbackFunct
def _OnWriteResponseCallback(closure, endpoint: int, cluster: int, attribute: int, status):
    closure.handleResponse(AttributePath(
        EndpointId=endpoint, ClusterId=cluster, AttributeId=attribute), status)


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


def ReadAttributes(future: Future, eventLoop, device, attributes: List[AttributePath]) -> int:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncReadTransaction(future, eventLoop)

    readargs = []
    for attr in attributes:
        path = chip.interaction_model.AttributePathIBstruct.parse(
            b'\xff' * chip.interaction_model.AttributePathIBstruct.sizeof())
        if attr.EndpointId is not None:
            path.EndpointId = attr.EndpointId
        if attr.ClusterId is not None:
            path.ClusterId = attr.ClusterId
        if attr.AttributeId is not None:
            path.AttributeId = attr.AttributeId
        path = chip.interaction_model.AttributePathIBstruct.build(path)
        readargs.append(ctypes.c_char_p(path))

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    res = handle.pychip_ReadClient_ReadAttributes(
        ctypes.py_object(transaction), device, ctypes.c_size_t(len(attributes)), *readargs)
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
        handle.pychip_ReadClient_ReadAttributes.restype = c_uint32
        setter.Set('pychip_ReadClient_InitCallbacks', None, [
                   _OnReadAttributeDataCallbackFunct, _OnReadErrorCallbackFunct, _OnReadDoneCallbackFunct])

    handle.pychip_WriteClient_InitCallbacks(
        _OnWriteResponseCallback, _OnWriteErrorCallback, _OnWriteDoneCallback)
    handle.pychip_ReadClient_InitCallbacks(
        _OnReadAttributeDataCallback, _OnReadErrorCallback, _OnReadDoneCallback)

    _BuildAttributeIndex()
