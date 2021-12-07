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
from typing import Tuple, Type, Union, List, Any, Callable
from ctypes import CFUNCTYPE, c_char_p, c_size_t, c_void_p, c_uint64, c_uint32,  c_uint16, c_uint8, py_object, c_uint64

from .ClusterObjects import Cluster, ClusterAttributeDescriptor, ClusterEventDescriptor
import chip.exceptions
import chip.interaction_model
import chip.tlv
from enum import Enum, unique
import inspect
import sys
import logging
import threading
import builtins


@unique
class EventTimestampType(Enum):
    SYSTEM = 0
    EPOCH = 1


@unique
class EventPriority(Enum):
    DEBUG = 1
    INFO = 2
    CRITICAL = 3


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

    def __hash__(self):
        return str(self).__hash__()


@dataclass
class EventPath:
    EndpointId: int = None
    ClusterId: int = None
    EventId: int = None

    def __init__(self, EndpointId: int = None, Cluster=None, Event=None, ClusterId=None, EventId=None):
        self.EndpointId = EndpointId
        if Cluster is not None:
            # Wildcard read for a specific cluster
            if (Event is not None) or (ClusterId is not None) or (EventId is not None):
                raise Warning(
                    "Event, ClusterId and AttributeId is ignored when Cluster is specified")
            self.ClusterId = Cluster.id
            return
        if Event is not None:
            if (ClusterId is not None) or (EventId is not None):
                raise Warning(
                    "ClusterId and EventId is ignored when Event is specified")
            self.ClusterId = Event.cluster_id
            self.EventId = Event.event_id
            return
        self.ClusterId = ClusterId
        self.EventId = EventId

    def __str__(self) -> str:
        return f"{self.EndpointId}/{self.ClusterId}/{self.EventId}"

    def __hash__(self):
        return str(self).__hash__()


@dataclass
class AttributePathWithListIndex(AttributePath):
    ListIndex: int = None


@dataclass
class EventHeader:
    EndpointId: int = None
    Event: ClusterEventDescriptor = None
    EventNumber: int = None
    Priority: EventPriority = None
    Timestamp: int = None
    TimestampType: EventTimestampType = None

    def __init__(self, EndpointId: int = None, Event=None, EventNumber=None, Priority=None, Timestamp=None, TimestampType=None):
        self.EndpointId = EndpointId
        self.Event = Event
        self.EventNumber = EventNumber
        self.Priority = Priority
        self.Timestamp = Timestamp
        self.Timestamp = TimestampType

    def __str__(self) -> str:
        return f"{self.EndpointId}/{self.Event.cluster_id}/{self.Event.event_id}/{self.EventNumber}/{self.Priority}/{self.Timestamp}/{self.TimestampType}"


@dataclass
class AttributeStatus:
    Path: AttributePath
    Status: Union[chip.interaction_model.Status, int]


@dataclass
class EventStatus:
    Header: EventHeader
    Status: chip.interaction_model.Status


AttributeWriteResult = AttributeStatus


@dataclass
class AttributeDescriptorWithEndpoint:
    EndpointId: int
    Attribute: ClusterAttributeDescriptor


@dataclass
class EventDescriptorWithEndpoint:
    EndpointId: int
    Event: ClusterEventDescriptor


@dataclass
class AttributeWriteRequest(AttributeDescriptorWithEndpoint):
    Data: Any


AttributeReadRequest = AttributeDescriptorWithEndpoint
EventReadRequest = EventDescriptorWithEndpoint


@dataclass
class AttributeReadResult(AttributeStatus):
    Data: Any = None


@dataclass
class ValueDecodeFailure:
    ''' Encapsulates a failure to decode a TLV value into a cluster object.
        Some exceptions have custom fields, so run str(ReasonException) to get more info.
    '''

    TLVValue: Any = None
    Reason: Exception = None


@dataclass
class EventReadResult(EventStatus):
    Data: Any = None


_AttributeIndex = {}
_EventIndex = {}


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


def _on_update_noop(path: AttributePath, value: Any):
    '''
    Default OnUpdate callback, simplily does nothing.
    '''
    pass


@dataclass
class SubscriptionParameters:
    MinReportIntervalFloorSeconds: int
    MaxReportIntervalCeilingSeconds: int


class SubscriptionTransaction:
    def __init__(self, transaction: 'AsyncReadTransaction', subscriptionId, devCtrl):
        self._on_update = _on_update_noop
        self._read_transaction = transaction
        self._subscriptionId = subscriptionId
        self._devCtrl = devCtrl

    def GetAttributeValue(self, path: Tuple[int, Type[ClusterAttributeDescriptor]]):
        '''
        Gets the attribute from cache, returns the value and the timestamp when it was updated last time.
        '''
        return self._read_transaction.GetAttributeValue(AttributePath(path[0], Attribute=path[1]))

    def GetAllAttributeValues(self):
        return self._read_transaction.GetAllAttributeValues()

    def GetAllEventValues(self):
        return self._read_transaction.GetAllEventValues()

    def SetAttributeUpdateCallback(self, callback: Callable[[AttributePath, Any], None]):
        '''
        Sets the callback function for the attribute value change event, accepts a Callable accpets an attribute path and its updated value.
        '''
        if callback is None:
            self._on_update = _on_update_noop
        else:
            self._on_update = callback

    @property
    def OnUpdate(self) -> Callable[[AttributePath, Any], None]:
        return self._on_update

    def Shutdown(self):
        self._devCtrl.ZCLShutdownSubscription(self._subscriptionId)

    def __repr__(self):
        return f'<Subscription (Id={self._subscriptionId})>'


def _BuildEventIndex():
    ''' Build internal event index for locating the corresponding cluster object by path in the future.
    We do this because this operation will take a long time when there are lots of events, it takes about 300ms for a single query.
    This is acceptable during init, but unacceptable when the server returns lots of events at the same time.
    '''
    for clusterName, obj in inspect.getmembers(sys.modules['chip.clusters.Objects']):
        if ('chip.clusters.Objects' in str(obj)) and inspect.isclass(obj):
            for objName, subclass in inspect.getmembers(obj):
                if inspect.isclass(subclass) and (('Events' == objName)):
                    for eventName, event in inspect.getmembers(subclass):
                        if inspect.isclass(event):
                            base_classes = inspect.getmro(event)

                            # Only match on classes that extend the ClusterEventescriptor class
                            matched = [
                                value for value in base_classes if 'ClusterEventDescriptor' in str(value)]
                            if (matched == []):
                                continue
                            logging.error(
                                f"event iterate {event.cluster_id} and {event.event_id}")
                            _EventIndex[str(EventPath(ClusterId=event.cluster_id, EventId=event.event_id))] = eval(
                                'chip.clusters.Objects.' + clusterName + '.Events.' + eventName)


class AsyncReadTransaction:
    def __init__(self, future: Future, eventLoop, devCtrl):
        self._event_loop = eventLoop
        self._future = future
        self._subscription_handler = None
        self._res = {'Attributes': {}, 'Events': []}
        self._devCtrl = devCtrl
        # For subscriptions, the data comes from CHIP Thread, whild the value will be accessed from Python's thread, so a lock is required here.
        self._resLock = threading.Lock()

    def GetAttributeValue(self, path: AttributePath):
        with self._resLock:
            return self._res['Attributes'].get(path)

    def GetAllAttributeValues(self):
        return self._res['Attributes']

    def GetAllEventValues(self):
        return self._res['Events']

    def _handleAttributeData(self, path: AttributePathWithListIndex, status: int, data: bytes):
        try:
            imStatus = status
            try:
                imStatus = chip.interaction_model.Status(status)
            except:
                pass

            if (imStatus != chip.interaction_model.Status.Success):
                logging.error(
                    f"For path: Endpoint = {path.EndpointId}/Cluster = {path.ClusterId}/Attribute = {path.AttributeId}, got IM Error: {str(imStatus)}")
                attributeValue = None
            else:
                attributeType = _AttributeIndex.get(str(AttributePath(
                    ClusterId=path.ClusterId, AttributeId=path.AttributeId)), None)
                attributeValue = None
                tlvData = chip.tlv.TLVReader(data).get().get("Any", {})
                if attributeType is None:
                    attributeValue = ValueDecodeFailure(
                        tlvData, LookupError("attribute schema not found"))
                else:
                    try:
                        attributeValue = attributeType(
                            attributeType.FromTLV(data))
                    except Exception as ex:
                        logging.error(
                            f"Error convering TLV to Cluster Object for path: Endpoint = {path.EndpointId}/Cluster = {path.ClusterId}/Attribute = {path.AttributeId}")
                        logging.error(
                            f"Failed Cluster Object: {str(attributeType)}")
                        logging.error(ex)
                        attributeValue = ValueDecodeFailure(
                            tlvData, ex)

                        # If we're in debug mode, raise the exception so that we can better debug what's happening.
                        if (builtins.enableDebugMode):
                            raise

            with self._resLock:
                self._res['Attributes'][path] = AttributeReadResult(
                    Path=path, Status=imStatus, Data=attributeValue)
                if self._subscription_handler is not None:
                    self._subscription_handler.OnUpdate(
                        path, attributeType(attributeValue))
        except Exception as ex:
            logging.exception(ex)

    def handleAttributeData(self, path: AttributePath, status: int, data: bytes):
        if self._subscription_handler is not None:
            self._handleAttributeData(path, status, data)
        else:
            self._event_loop.call_soon_threadsafe(
                self._handleAttributeData, path, status, data)

    def _handleEventData(self, header: EventHeader, path: EventPath, data: bytes):
        try:
            eventType = _EventIndex.get(str(path), None)
            eventValue = None
            tlvData = chip.tlv.TLVReader(data).get().get("Any", {})
            if eventType is None:
                eventValue = ValueDecodeFailure(
                    tlvData, LookupError("event schema not found"))
            else:
                eventValue = tlvData
                header.Event = eventType

            with self._resLock:
                self._res['Events'].append(EventReadResult(
                    Header=header, Status=chip.interaction_model.Status.Success, Data=eventValue))
        except Exception as ex:
            logging.exception(ex)

    def handleEventData(self, header: EventHeader, path: EventPath, data: bytes):
        if self._subscription_handler is not None:
            self._handleEventData(header, path, data)
        else:
            self._event_loop.call_soon_threadsafe(
                self._handleEventData, header, path, data)

    def _handleError(self, chipError: int):
        self._future.set_exception(
            chip.exceptions.ChipStackError(chipError))

    def handleError(self, chipError: int):
        self._event_loop.call_soon_threadsafe(
            self._handleError, chipError
        )

    def _handleSubscriptionEstablished(self, subscriptionId):
        if not self._future.done():
            self._subscription_handler = SubscriptionTransaction(
                self, subscriptionId, self._devCtrl)
            self._future.set_result(self._subscription_handler)

    def handleSubscriptionEstablished(self, subscriptionId):
        self._event_loop.call_soon_threadsafe(
            self._handleSubscriptionEstablished, subscriptionId)

    def _handleDone(self):
        if not self._future.done():
            self._future.set_result(self._res)

    def handleDone(self):
        self._event_loop.call_soon_threadsafe(self._handleDone)


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
_OnSubscriptionEstablishedCallbackFunct = CFUNCTYPE(None, py_object, c_uint64)
_OnReadEventDataCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint32, c_uint8, c_uint64, c_uint8, c_void_p, c_size_t)
_OnReadErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32)
_OnReadDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnReadAttributeDataCallbackFunct
def _OnReadAttributeDataCallback(closure, endpoint: int, cluster: int, attribute: int, status, data, len):
    dataBytes = ctypes.string_at(data, len)
    closure.handleAttributeData(AttributePath(
        EndpointId=endpoint, ClusterId=cluster, AttributeId=attribute), status, dataBytes[:])


@_OnReadEventDataCallbackFunct
def _OnReadEventDataCallback(closure, endpoint: int, cluster: int, event: int, number: int, priority: int, timestamp: int, timestampType: int, data, len):
    dataBytes = ctypes.string_at(data, len)
    path = EventPath(ClusterId=cluster, EventId=event)
    closure.handleEventData(EventHeader(
        EndpointId=endpoint, EventNumber=number, Priority=EventPriority(priority), Timestamp=timestamp, TimestampType=EventTimestampType(timestampType)), path, dataBytes[:])


@_OnSubscriptionEstablishedCallbackFunct
def _OnSubscriptionEstablishedCallback(closure, subscriptionId):
    closure.handleSubscriptionEstablished(subscriptionId)


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


def ReadAttributes(future: Future, eventLoop, device, devCtrl, attributes: List[AttributePath], subscriptionParameters: SubscriptionParameters = None) -> int:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncReadTransaction(future, eventLoop, devCtrl)

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
    minInterval = 0
    maxInterval = 0
    if subscriptionParameters is not None:
        minInterval = subscriptionParameters.MinReportIntervalFloorSeconds
        maxInterval = subscriptionParameters.MaxReportIntervalCeilingSeconds
    res = handle.pychip_ReadClient_ReadAttributes(
        ctypes.py_object(transaction), device,
        ctypes.c_bool(subscriptionParameters is not None),
        ctypes.c_uint32(minInterval), ctypes.c_uint32(maxInterval),
        ctypes.c_size_t(len(attributes)), *readargs)
    if res != 0:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def ReadEvents(future: Future, eventLoop, device, devCtrl, events: List[EventPath], subscriptionParameters: SubscriptionParameters = None) -> int:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncReadTransaction(future, eventLoop, devCtrl)

    readargs = []
    for attr in events:
        path = chip.interaction_model.EventPathIBstruct.parse(
            b'\xff' * chip.interaction_model.EventPathIBstruct.sizeof())
        if attr.EndpointId is not None:
            path.EndpointId = attr.EndpointId
        if attr.ClusterId is not None:
            path.ClusterId = attr.ClusterId
        if attr.EventId is not None:
            path.EventId = attr.EventId
        path = chip.interaction_model.EventPathIBstruct.build(path)
        readargs.append(ctypes.c_char_p(path))

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    minInterval = 0
    maxInterval = 0
    if subscriptionParameters is not None:
        minInterval = subscriptionParameters.MinReportIntervalFloorSeconds
        maxInterval = subscriptionParameters.MaxReportIntervalCeilingSeconds
    res = handle.pychip_ReadClient_ReadEvents(
        ctypes.py_object(transaction), device,
        ctypes.c_bool(subscriptionParameters is not None),
        ctypes.c_uint32(minInterval), ctypes.c_uint32(maxInterval),
        ctypes.c_size_t(len(events)), *readargs)
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
                   _OnReadAttributeDataCallbackFunct, _OnReadEventDataCallbackFunct, _OnSubscriptionEstablishedCallbackFunct, _OnReadErrorCallbackFunct, _OnReadDoneCallbackFunct])

    handle.pychip_WriteClient_InitCallbacks(
        _OnWriteResponseCallback, _OnWriteErrorCallback, _OnWriteDoneCallback)
    handle.pychip_ReadClient_InitCallbacks(
        _OnReadAttributeDataCallback, _OnReadEventDataCallback, _OnSubscriptionEstablishedCallback, _OnReadErrorCallback, _OnReadDoneCallback)

    _BuildAttributeIndex()
    _BuildEventIndex()
