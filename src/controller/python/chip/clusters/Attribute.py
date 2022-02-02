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

# Needed to use types in type hints before they are fully defined.
from __future__ import annotations

from asyncio.futures import Future
import ctypes
from dataclasses import dataclass, field
from typing import Tuple, Type, Union, List, Any, Callable, Dict, Set
from ctypes import CFUNCTYPE, c_char_p, c_size_t, c_void_p, c_uint64, c_uint32,  c_uint16, c_uint8, py_object, c_uint64
import construct
from rich.pretty import pprint

from .ClusterObjects import Cluster, ClusterAttributeDescriptor, ClusterEvent
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
class TypedAttributePath:
    ''' Encapsulates an attribute path that has strongly typed references to cluster and attribute
        cluster object types. These types serve as keys into the attribute cache.
    '''
    ClusterType: Cluster = None
    AttributeType: ClusterAttributeDescriptor = None
    AttributeName: str = None
    Path: AttributePath = None

    def __init__(self, ClusterType: Cluster = None, AttributeType: ClusterAttributeDescriptor = None,
                 Path: AttributePath = None):
        ''' Only one of either ClusterType and AttributeType OR Path may be provided.
        '''

        #
        # First, let's populate ClusterType and AttributeType. If it's already provided,
        # we can continue onwards to deriving the label. Otherwise, we'll need to
        # walk the attribute index to find the right type information.
        #
        if (ClusterType is not None and AttributeType is not None):
            self.ClusterType = ClusterType
            self.AttributeType = AttributeType
        else:
            if (Path is None):
                raise ValueError("Path should have a valid value")

            for cluster, attribute in _AttributeIndex:
                attributeType = _AttributeIndex[(cluster, attribute)][0]
                clusterType = _AttributeIndex[(cluster, attribute)][1]

                if (clusterType.id == Path.ClusterId and attributeType.attribute_id == Path.AttributeId):
                    self.ClusterType = clusterType
                    self.AttributeType = attributeType
                    break

            if (self.ClusterType is None or self.AttributeType is None):
                raise Exception("Schema not found")

        # Next, let's figure out the label.
        for field in self.ClusterType.descriptor.Fields:
            if (field.Tag != self.AttributeType.attribute_id):
                continue

            self.AttributeName = field.Label

        if (self.AttributeName is None):
            raise Exception("Schema not found")

        self.Path = Path
        self.ClusterId = self.ClusterType.id
        self.AttributeId = self.AttributeType.attribute_id


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
    Event: ClusterEvent = None
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
    Event: ClusterEvent


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
_ClusterIndex = {}


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

                            _AttributeIndex[(attribute.cluster_id, attribute.attribute_id)] = (eval(
                                'chip.clusters.Objects.' + clusterName + '.Attributes.' + attributeName), obj)


def _BuildClusterIndex():
    ''' Build internal cluster index for locating the corresponding cluster object by path in the future.
    '''
    for clusterName, obj in inspect.getmembers(sys.modules['chip.clusters.Objects']):
        if ('chip.clusters.Objects' in str(obj)) and inspect.isclass(obj):
            _ClusterIndex[obj.id] = obj


@dataclass
class SubscriptionParameters:
    MinReportIntervalFloorSeconds: int
    MaxReportIntervalCeilingSeconds: int


@dataclass
class AttributeCache:
    ''' A cache that stores data & errors returned in read/subscribe reports, but organizes it topologically
        in a collection of nested dictionaries. The organization follows the natural data model composition of
        the device: endpoint, then cluster, then attribute.

        TLV data (or ValueDecodeFailure in the case of IM status codes) are stored for each attribute in
        attributeTLVCache[endpoint][cluster][attribute].

        Upon completion of data population, it can be retrieved in a more friendly cluster object format,
        with two options available. In both options, data is in the dictionary is key'ed not by the raw numeric
        cluster and attribute IDs, but instead by the cluster object descriptor types for each of those generated
        cluster objects.

            E.g Clusters.TestCluster is the literal key for indexing the test cluster.
                Clusters.TestCluster.Attributes.Int16u is the listeral key for indexing an attribute in the test cluster.

        This strongly typed keys permit a more natural and safer form of indexing.
    '''
    returnClusterObject: bool = False
    attributeTLVCache: Dict[int, Dict[int, Dict[int, bytes]]] = field(
        default_factory=lambda: {})
    attributeCache: Dict[int, List[Cluster]] = field(
        default_factory=lambda: {})

    def UpdateTLV(self, path: AttributePath, dataVersion: int, data: Union[bytes, ValueDecodeFailure]):
        ''' Store data in TLV since that makes it easiest to eventually convert to either the
            cluster or attribute view representations (see below in UpdateCachedData).
        '''
        if (path.EndpointId not in self.attributeTLVCache):
            self.attributeTLVCache[path.EndpointId] = {}

        endpointCache = self.attributeTLVCache[path.EndpointId]
        if (path.ClusterId not in endpointCache):
            endpointCache[path.ClusterId] = {}

        clusterCache = endpointCache[path.ClusterId]
        if (path.AttributeId not in clusterCache):
            clusterCache[path.AttributeId] = None

        clusterCache[path.AttributeId] = data

    def UpdateCachedData(self):
        ''' This converts the raw TLV data into a cluster object format.

            Two formats are available:
                1. Attribute-View (returnClusterObject=False): Dict[EndpointId, Dict[ClusterObjectType, Dict[AttributeObjectType, AttributeValue]]]
                2. Cluster-View (returnClusterObject=True): Dict[EndpointId, Dict[ClusterObjectType, ClusterValue]]

            In the attribute-view, only attributes that match the original path criteria are present in the dictionary. The attribute values can
            either be the actual data for the attribute, or a ValueDecodeFailure in the case of non-success IM status codes, or other errors encountered during decode.

            In the cluster-view, a cluster object that corresponds to all attributes on a given cluster instance is returned,
            regardless of the subset of attributes read. For attributes not returned in the report, defaults are used. If a cluster cannot be decoded,
            instead of a cluster object value, a ValueDecodeFailure shall be present.
        '''

        tlvCache = self.attributeTLVCache
        attributeCache = self.attributeCache

        for endpoint in tlvCache:
            if (endpoint not in attributeCache):
                attributeCache[endpoint] = {}

            endpointCache = attributeCache[endpoint]

            for cluster in tlvCache[endpoint]:
                clusterType = _ClusterIndex[cluster]
                if (clusterType is None):
                    raise Exception("Cannot find cluster in cluster index")

                if (clusterType not in endpointCache):
                    endpointCache[clusterType] = {}

                clusterCache = endpointCache[clusterType]

                if (self.returnClusterObject):
                    try:
                        # Since the TLV data is already organized by attribute tags, we can trivially convert to a cluster object representation.
                        endpointCache[clusterType] = clusterType.FromDict(
                            data=clusterType.descriptor.TagDictToLabelDict([], tlvCache[endpoint][cluster]))
                    except Exception as ex:
                        logging.error(
                            f"Error converting TLV to Cluster Object for path: Endpoint = {endpoint}, cluster = {str(clusterType)}")
                        logging.error(f"|-- Exception: {repr(ex)}")
                        decodedValue = ValueDecodeFailure(
                            tlvCache[endpoint][cluster], ex)
                        endpointCache[clusterType] = decodedValue
                else:
                    for attribute in tlvCache[endpoint][cluster]:
                        value = tlvCache[endpoint][cluster][attribute]

                        attributeType = _AttributeIndex[(
                            cluster, attribute)][0]
                        if (attributeType is None):
                            raise Exception(
                                "Cannot find attribute in attribute index")

                        if (attributeType not in clusterCache):
                            clusterCache[attributeType] = {}

                        if (type(value) is ValueDecodeFailure):
                            logging.error(
                                f"For path: Endpoint = {endpoint}, Attribute = {str(attributeType)}, got IM Error: {str(value.Reason)}")
                            clusterCache[attributeType] = value
                        else:
                            try:
                                decodedValue = attributeType.FromTagDictOrRawValue(
                                    tlvCache[endpoint][cluster][attribute])
                            except Exception as ex:
                                logging.error(
                                    f"Error converting TLV to Cluster Object for path: Endpoint = {endpoint}, Attribute = {str(attributeType)}")
                                logging.error(f"|-- Exception: {repr(ex)}")
                                decodedValue = ValueDecodeFailure(value, ex)

                            clusterCache[attributeType] = decodedValue


class SubscriptionTransaction:
    def __init__(self, transaction: 'AsyncReadTransaction', subscriptionId, devCtrl):
        self._onAttributeChangeCb = DefaultAttributeChangeCallback
        self._onEventChangeCb = DefaultEventChangeCallback
        self._readTransaction = transaction
        self._subscriptionId = subscriptionId
        self._devCtrl = devCtrl
        self._isDone = False

    def GetAttributes(self):
        ''' Returns the attribute value cache tracking the latest state on the publisher.
        '''
        return self._readTransaction._cache.attributeCache

    def GetAttribute(self, path: TypedAttributePath) -> Any:
        ''' Returns a specific attribute given a TypedAttributePath.
        '''
        data = self._readTransaction._cache.attributeCache

        if (self._readTransaction._cache.returnClusterObject):
            return eval(f'data[path.Path.EndpointId][path.ClusterType].{path.AttributeName}')
        else:
            return data[path.Path.EndpointId][path.ClusterType][path.AttributeType]

    def GetEvents(self):
        return self._readTransaction.GetAllEventValues()

    def SetAttributeUpdateCallback(self, callback: Callable[[TypedAttributePath, SubscriptionTransaction], None]):
        '''
        Sets the callback function for the attribute value change event, accepts a Callable accepts an attribute path and the cached data.
        '''
        if callback is not None:
            self._onAttributeChangeCb = callback

    def SetEventUpdateCallback(self, callback: Callable[[EventReadResult, SubscriptionTransaction], None]):
        if callback is not None:
            self._onEventChangeCb = callback

    @property
    def OnAttributeChangeCb(self) -> Callable[[TypedAttributePath, SubscriptionTransaction], None]:
        return self._onAttributeChangeCb

    @property
    def OnEventChangeCb(self) -> Callable[[EventReadResult, SubscriptionTransaction], None]:
        return self._onEventChangeCb

    def Shutdown(self):
        if (self._isDone):
            print("Subscription was already terminated previously!")
            return

        handle = chip.native.GetLibraryHandle()
        handle.pychip_ReadClient_Abort(
            self._readTransaction._pReadClient, self._readTransaction._pReadCallback)
        self._isDone = True

    def __del__(self):
        self.Shutdown()

    def __repr__(self):
        return f'<Subscription (Id={self._subscriptionId})>'


def DefaultAttributeChangeCallback(path: TypedAttributePath, transaction: SubscriptionTransaction):
    data = transaction.GetAttribute(path)
    value = {
        'Endpoint': path.Path.EndpointId,
        'Attribute': path.AttributeType,
        'Value': data
    }

    print("Attribute Changed:")
    pprint(value, expand_all=True)


def DefaultEventChangeCallback(data: EventReadResult, transaction: SubscriptionTransaction):
    print("Received Event:")
    pprint(data, expand_all=True)


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
                                value for value in base_classes if 'ClusterEvent' in str(value)]
                            if (matched == []):
                                continue

                            _EventIndex[str(EventPath(ClusterId=event.cluster_id, EventId=event.event_id))] = eval(
                                'chip.clusters.Objects.' + clusterName + '.Events.' + eventName)


class TransactionType(Enum):
    READ_EVENTS = 1
    READ_ATTRIBUTES = 2


class AsyncReadTransaction:
    def __init__(self, future: Future, eventLoop, devCtrl, transactionType: TransactionType, returnClusterObject: bool):
        self._event_loop = eventLoop
        self._future = future
        self._subscription_handler = None
        self._events = []
        self._devCtrl = devCtrl
        self._transactionType = transactionType
        self._cache = AttributeCache(returnClusterObject=returnClusterObject)
        self._changedPathSet = set()
        self._pReadClient = None
        self._pReadCallback = None

    def SetClientObjPointers(self, pReadClient, pReadCallback):
        self._pReadClient = pReadClient
        self._pReadCallback = pReadCallback

    def GetAllEventValues(self):
        return self._events

    def _handleAttributeData(self, path: AttributePathWithListIndex, dataVersion: int, status: int, data: bytes):
        try:
            imStatus = status
            try:
                imStatus = chip.interaction_model.Status(status)
            except:
                pass

            if (imStatus != chip.interaction_model.Status.Success):
                attributeValue = ValueDecodeFailure(
                    None, chip.interaction_model.InteractionModelError(imStatus))
            else:
                tlvData = chip.tlv.TLVReader(data).get().get("Any", {})
                attributeValue = tlvData

            self._cache.UpdateTLV(path, dataVersion, attributeValue)
            self._changedPathSet.add(path)

        except Exception as ex:
            logging.exception(ex)

    def handleAttributeData(self, path: AttributePath, dataVersion: int, status: int, data: bytes):
        self._handleAttributeData(path, dataVersion, status, data)

    def _handleEventData(self, header: EventHeader, path: EventPath, data: bytes):
        try:
            eventType = _EventIndex.get(str(path), None)
            eventValue = None
            tlvData = chip.tlv.TLVReader(data).get().get("Any", {})
            if eventType is None:
                eventValue = ValueDecodeFailure(
                    tlvData, LookupError("event schema not found"))
            else:
                try:
                    eventValue = eventType.FromTLV(data)
                except Exception as ex:
                    logging.error(
                        f"Error convering TLV to Cluster Object for path: Endpoint = {path.EndpointId}/Cluster = {path.ClusterId}/Event = {path.EventId}")
                    logging.error(
                        f"Failed Cluster Object: {str(eventType)}")
                    logging.error(ex)
                    eventValue = ValueDecodeFailure(
                        tlvData, ex)

                    # If we're in debug mode, raise the exception so that we can better debug what's happening.
                    if (builtins.enableDebugMode):
                        raise

            eventResult = EventReadResult(
                Header=header, Data=eventValue, Status=chip.interaction_model.Status.Success)
            self._events.append(eventResult)

            if (self._subscription_handler is not None):
                self._subscription_handler.OnEventChangeCb(
                    eventResult, self._subscription_handler)

        except Exception as ex:
            logging.exception(ex)

    def handleEventData(self, header: EventHeader, path: EventPath, data: bytes):
        self._handleEventData(header, path, data)

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

    def _handleReportBegin(self):
        pass

    def _handleReportEnd(self):
        self._cache.UpdateCachedData()

        if (self._subscription_handler is not None):
            for change in self._changedPathSet:
                self._subscription_handler.OnAttributeChangeCb(
                    TypedAttributePath(Path=change), self._subscription_handler)

            # Clear it out once we've notified of all changes in this transaction.
        self._changedPathSet = set()

    def _handleDone(self):
        if not self._future.done():
            if (self._transactionType == TransactionType.READ_EVENTS):
                self._future.set_result(self._events)
            else:
                self._future.set_result(self._cache.attributeCache)

    def handleDone(self):
        self._event_loop.call_soon_threadsafe(self._handleDone)

    def handleReportBegin(self):
        pass

    def handleReportEnd(self):
        # self._event_loop.call_soon_threadsafe(self._handleReportEnd)
        self._handleReportEnd()


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
    None, py_object, c_uint32, c_uint16, c_uint32, c_uint32, c_uint32, c_void_p, c_size_t)
_OnSubscriptionEstablishedCallbackFunct = CFUNCTYPE(None, py_object, c_uint64)
_OnReadEventDataCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint32, c_uint8, c_uint64, c_uint8, c_void_p, c_size_t)
_OnReadErrorCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32)
_OnReadDoneCallbackFunct = CFUNCTYPE(
    None, py_object)
_OnReportBeginCallbackFunct = CFUNCTYPE(
    None, py_object)
_OnReportEndCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnReadAttributeDataCallbackFunct
def _OnReadAttributeDataCallback(closure, dataVersion: int, endpoint: int, cluster: int, attribute: int, status, data, len):
    dataBytes = ctypes.string_at(data, len)
    closure.handleAttributeData(AttributePath(
        EndpointId=endpoint, ClusterId=cluster, AttributeId=attribute), dataVersion, status, dataBytes[:])


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


@_OnReportBeginCallbackFunct
def _OnReportBeginCallback(closure):
    closure.handleReportBegin()


@_OnReportEndCallbackFunct
def _OnReportEndCallback(closure):
    closure.handleReportEnd()


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


def WriteAttributes(future: Future, eventLoop, device, attributes: List[AttributeWriteRequest], timedRequestTimeoutMs: int = None) -> int:
    handle = chip.native.GetLibraryHandle()

    writeargs = []
    for attr in attributes:
        if attr.Attribute.must_use_timed_write and timedRequestTimeoutMs is None or timedRequestTimeoutMs == 0:
            raise ValueError(
                f"Attribute {attr.__class__} must use timed write, please specify a valid timedRequestTimeoutMs value.")
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

    transaction = AsyncWriteTransaction(future, eventLoop)
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    res = handle.pychip_WriteClient_WriteAttributes(
        ctypes.py_object(transaction), device, ctypes.c_uint16(0 if timedRequestTimeoutMs is None else timedRequestTimeoutMs), ctypes.c_size_t(len(attributes)), *writeargs)
    if res != 0:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


# This struct matches the PyReadAttributeParams in attribute.cpp, for passing various params together.
_ReadParams = construct.Struct(
    "MinInterval" / construct.Int32ul,
    "MaxInterval" / construct.Int32ul,
    "IsSubscription" / construct.Flag,
    "IsFabricFiltered" / construct.Flag,
)


def ReadAttributes(future: Future, eventLoop, device, devCtrl, attributes: List[AttributePath], returnClusterObject: bool = True, subscriptionParameters: SubscriptionParameters = None, fabricFiltered: bool = True) -> int:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncReadTransaction(
        future, eventLoop, devCtrl, TransactionType.READ_ATTRIBUTES, returnClusterObject)

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

    readClientObj = ctypes.POINTER(c_void_p)()
    readCallbackObj = ctypes.POINTER(c_void_p)()

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    params = _ReadParams.parse(b'\x00' * _ReadParams.sizeof())
    if subscriptionParameters is not None:
        params.MinInterval = subscriptionParameters.MinReportIntervalFloorSeconds
        params.MaxInterval = subscriptionParameters.MaxReportIntervalCeilingSeconds
        params.IsSubscription = True
    params.IsFabricFiltered = fabricFiltered
    params = _ReadParams.build(params)

    res = handle.pychip_ReadClient_ReadAttributes(
        ctypes.py_object(transaction),
        ctypes.byref(readClientObj),
        ctypes.byref(readCallbackObj),
        device,
        ctypes.c_char_p(params),
        ctypes.c_size_t(len(attributes)), *readargs)

    transaction.SetClientObjPointers(readClientObj, readCallbackObj)

    if res != 0:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def ReadEvents(future: Future, eventLoop, device, devCtrl, events: List[EventPath], subscriptionParameters: SubscriptionParameters = None) -> int:
    handle = chip.native.GetLibraryHandle()
    transaction = AsyncReadTransaction(
        future, eventLoop, devCtrl, TransactionType.READ_EVENTS, False)

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

    readClientObj = ctypes.POINTER(c_void_p)()
    readCallbackObj = ctypes.POINTER(c_void_p)()

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))

    params = _ReadParams.parse(b'\x00' * _ReadParams.sizeof())

    if subscriptionParameters is not None:
        params.MinInterval = subscriptionParameters.MinReportIntervalFloorSeconds
        params.MaxInterval = subscriptionParameters.MaxReportIntervalCeilingSeconds
        params.IsSubscription = True
    params = _ReadParams.build(params)

    res = handle.pychip_ReadClient_ReadEvents(
        ctypes.py_object(transaction),
        ctypes.byref(readClientObj),
        ctypes.byref(readCallbackObj),
        device,
        ctypes.c_char_p(params),
        ctypes.c_size_t(len(events)), *readargs)

    transaction.SetClientObjPointers(readClientObj, readCallbackObj)

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
                   _OnReadAttributeDataCallbackFunct, _OnReadEventDataCallbackFunct, _OnSubscriptionEstablishedCallbackFunct, _OnReadErrorCallbackFunct, _OnReadDoneCallbackFunct,
                   _OnReportBeginCallbackFunct, _OnReportEndCallbackFunct])

    handle.pychip_WriteClient_InitCallbacks(
        _OnWriteResponseCallback, _OnWriteErrorCallback, _OnWriteDoneCallback)
    handle.pychip_ReadClient_InitCallbacks(
        _OnReadAttributeDataCallback, _OnReadEventDataCallback, _OnSubscriptionEstablishedCallback, _OnReadErrorCallback, _OnReadDoneCallback,
        _OnReportBeginCallback, _OnReportEndCallback)

    _BuildAttributeIndex()
    _BuildClusterIndex()
    _BuildEventIndex()
