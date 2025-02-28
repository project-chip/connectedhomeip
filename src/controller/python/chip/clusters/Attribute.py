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

import builtins
import ctypes
import inspect
import logging
import sys
from asyncio.futures import Future
from ctypes import CFUNCTYPE, POINTER, c_size_t, c_uint8, c_uint16, c_uint32, c_uint64, c_void_p, cast, py_object
from dataclasses import dataclass, field
from enum import Enum, unique
from typing import Any, Callable, Dict, List, Optional, Set, Tuple, Union

import chip
import chip.exceptions
import chip.interaction_model
import chip.tlv
import construct  # type: ignore
from chip.interaction_model import PyWriteAttributeData
from chip.native import ErrorSDKPart, PyChipError
from rich.pretty import pprint  # type: ignore

from .ClusterObjects import Cluster, ClusterAttributeDescriptor, ClusterEvent

LOGGER = logging.getLogger(__name__)


@unique
class EventTimestampType(Enum):
    SYSTEM = 0
    EPOCH = 1


@unique
class EventPriority(Enum):
    DEBUG = 0
    INFO = 1
    CRITICAL = 2


@dataclass(frozen=True)
class AttributePath:
    EndpointId: Optional[int] = None
    ClusterId: Optional[int] = None
    AttributeId: Optional[int] = None

    @staticmethod
    def from_cluster(EndpointId: int, Cluster: Cluster) -> AttributePath:
        if Cluster is None:
            raise ValueError("Cluster cannot be None")
        return AttributePath(EndpointId=EndpointId, ClusterId=Cluster.id)

    @staticmethod
    def from_attribute(EndpointId: int, Attribute: ClusterAttributeDescriptor) -> AttributePath:
        if Attribute is None:
            raise ValueError("Attribute cannot be None")
        return AttributePath(EndpointId=EndpointId, ClusterId=Attribute.cluster_id, AttributeId=Attribute.attribute_id)

    def __str__(self) -> str:
        return f"{self.EndpointId}/{self.ClusterId}/{self.AttributeId}"


@dataclass(frozen=True)
class DataVersionFilter:
    EndpointId: Optional[int] = None
    ClusterId: Optional[int] = None
    DataVersion: Optional[int] = None

    @staticmethod
    def from_cluster(EndpointId: int, Cluster: Cluster, DataVersion: int) -> DataVersionFilter:
        if Cluster is None:
            raise ValueError("Cluster cannot be None")
        return DataVersionFilter(EndpointId=EndpointId, ClusterId=Cluster.id, DataVersion=DataVersion)

    def __str__(self) -> str:
        return f"{self.EndpointId}/{self.ClusterId}/{self.DataVersion}"


@dataclass
class TypedAttributePath:
    ''' Encapsulates an attribute path that has strongly typed references to cluster and attribute
        cluster object types. These types serve as keys into the attribute cache.
    '''
    ClusterType: Optional[Cluster] = None
    AttributeType: Optional[ClusterAttributeDescriptor] = None
    AttributeName: Optional[str] = None
    Path: Optional[AttributePath] = None
    ClusterId: Optional[int] = None
    AttributeId: Optional[int] = None

    def __post_init__(self):
        '''Only one of either ClusterType and AttributeType OR Path may be provided.'''

        if (self.ClusterType is not None and self.AttributeType is not None) and self.Path is not None:
            raise ValueError(
                "Only one of either ClusterType and AttributeType OR Path may be provided.")
        if (self.ClusterType is None or self.AttributeType is None) and self.Path is None:
            raise ValueError(
                "Either ClusterType and AttributeType OR Path must be provided.")

        # if ClusterType and AttributeType were provided we can continue onwards to deriving the label.
        # Otherwise, we'll need to walk the attribute index to find the right type information.

        # If Path is provided, derive ClusterType and AttributeType from it
        if self.Path is not None:
            for cluster, attribute in _AttributeIndex:
                attributeType = _AttributeIndex[(cluster, attribute)][0]
                clusterType = _AttributeIndex[(cluster, attribute)][1]

                if clusterType.id == self.Path.ClusterId and attributeType.attribute_id == self.Path.AttributeId:
                    self.ClusterType = clusterType
                    self.AttributeType = attributeType
                    break

            if self.ClusterType is None or self.AttributeType is None:
                raise KeyError(f"No Schema found for Attribute {self.Path}")

        # Next, let's figure out the label.
        for c_field in self.ClusterType.descriptor.Fields:
            if c_field.Tag != self.AttributeType.attribute_id:
                continue

            self.AttributeName = c_field.Label

        if self.AttributeName is None:
            raise KeyError(f"Unable to resolve name for Attribute {self.Path}")

        self.ClusterId = self.ClusterType.id
        self.AttributeId = self.AttributeType.attribute_id


@dataclass(frozen=True)
class EventPath:
    EndpointId: Optional[int] = None
    ClusterId: Optional[int] = None
    EventId: Optional[int] = None
    Urgent: Optional[int] = None

    @staticmethod
    def from_cluster(EndpointId: int, Cluster: Cluster, EventId: Optional[int] = None, Urgent: Optional[int] = None) -> "EventPath":
        if Cluster is None:
            raise ValueError("Cluster cannot be None")
        return EventPath(EndpointId=EndpointId, ClusterId=Cluster.id, EventId=EventId, Urgent=Urgent)

    @staticmethod
    def from_event(EndpointId: int, Event: ClusterEvent, Urgent: Optional[int] = None) -> "EventPath":
        if Event is None:
            raise ValueError("Event cannot be None")
        return EventPath(EndpointId=EndpointId, ClusterId=Event.cluster_id, EventId=Event.event_id, Urgent=Urgent)

    def __str__(self) -> str:
        return f"{self.EndpointId}/{self.ClusterId}/{self.EventId}/{self.Urgent}"


@dataclass
class EventHeader:
    EndpointId: Optional[int] = None
    ClusterId: Optional[int] = None
    EventId: Optional[int] = None
    EventNumber: Optional[int] = None
    Priority: Optional[EventPriority] = None
    Timestamp: Optional[int] = None
    TimestampType: Optional[EventTimestampType] = None

    def __str__(self) -> str:
        return (f"{self.EndpointId}/{self.ClusterId}/{self.EventId}/"
                f"{self.EventNumber}/{self.Priority}/{self.Timestamp}/{self.TimestampType}")


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
    DataVersion: int
    HasDataVersion: int


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
    DataVersion: int = 0


@dataclass
class ValueDecodeFailure:
    ''' Encapsulates a failure to decode a TLV value into a cluster object.
        Some exceptions have custom fields, so run str(ReasonException) to get more info.
    '''

    TLVValue: Any = None
    Reason: Optional[Exception] = None


@dataclass
class EventReadResult(EventStatus):
    Data: Any = None


_AttributeIndex = {}
_EventIndex = {}
_ClusterIndex = {}


def _BuildAttributeIndex():
    ''' Build internal attribute index for locating the corresponding cluster object by path in the future.
        We do this because this operation will take a long time when there are lots of attributes,
        it takes about 300ms for a single query.
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
        if ('chip.clusters.Objects' in str(obj)) and inspect.isclass(obj) and issubclass(obj, Cluster):
            _ClusterIndex[obj.id] = obj


@dataclass
class SubscriptionParameters:
    MinReportIntervalFloorSeconds: int
    MaxReportIntervalCeilingSeconds: int


class DataVersion:
    '''
    A helper class as a key for getting cluster data version when reading attributes without returnClusterObject.
    '''
    pass


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

            E.g Clusters.UnitTesting.is the literal key for indexing the test cluster.
                Clusters.UnitTesting.Attributes.Int16u is the listeral key for indexing an attribute in the test cluster.

        This strongly typed keys permit a more natural and safer form of indexing.
    '''
    returnClusterObject: bool = False
    attributeTLVCache: Dict[int, Dict[int, Dict[int, bytes]]] = field(
        default_factory=lambda: {})
    versionList: Dict[int, Dict[int, Dict[int, int]]] = field(
        default_factory=lambda: {})

    _attributeCacheUpdateNeeded: set[AttributePath] = field(
        default_factory=lambda: set())
    _attributeCache: Dict[int, List[Cluster]] = field(
        default_factory=lambda: {})

    def UpdateTLV(self, path: AttributePath, dataVersion: int,  data: Union[bytes, ValueDecodeFailure]):
        ''' Store data in TLV since that makes it easiest to eventually convert to either the
            cluster or attribute view representations (see below in GetUpdatedAttributeCache()).
        '''
        if (path.EndpointId not in self.attributeTLVCache):
            self.attributeTLVCache[path.EndpointId] = {}

        if (path.EndpointId not in self.versionList):
            self.versionList[path.EndpointId] = {}

        endpointCache = self.attributeTLVCache[path.EndpointId]
        endpointVersion = self.versionList[path.EndpointId]
        if (path.ClusterId not in endpointCache):
            endpointCache[path.ClusterId] = {}

        # All attributes from the same cluster instance should have the same dataVersion,
        # so we can set the dataVersion of the cluster to the dataVersion with a random attribute.
        endpointVersion[path.ClusterId] = dataVersion

        clusterCache = endpointCache[path.ClusterId]
        if (path.AttributeId not in clusterCache):
            clusterCache[path.AttributeId] = None

        clusterCache[path.AttributeId] = data

        # For this path the attribute cache still requires an update.
        self._attributeCacheUpdateNeeded.add(path)

    def GetUpdatedAttributeCache(self) -> Dict[int, List[Cluster]]:
        ''' This converts the raw TLV data into a cluster object format.

            Two formats are available:
                1. Attribute-View (returnClusterObject=False): Dict[EndpointId,
                                                                    Dict[ClusterObjectType,
                                                                    Dict[AttributeObjectType, Dict[AttributeValue, DataVersion]]]]
                2. Cluster-View (returnClusterObject=True): Dict[EndpointId, Dict[ClusterObjectType, ClusterValue]]

            In the attribute-view, only attributes that match the original path criteria are present in the dictionary.
            The attribute values can either be the actual data for the attribute, or a ValueDecodeFailure in the case of
            non-success IM status codes, or other errors encountered during decode.

            In the cluster-view, a cluster object that corresponds to all attributes on a given cluster instance is returned,
            regardless of the subset of attributes read. For attributes not returned in the report,
            defaults are used. If a cluster cannot be decoded,
            instead of a cluster object value, a ValueDecodeFailure shall be present.
        '''

        def handle_cluster_view(endpointId, clusterId, clusterType):
            try:
                decodedData = clusterType.FromDict(
                    data=clusterType.descriptor.TagDictToLabelDict([], self.attributeTLVCache[endpointId][clusterId]))
                decodedData.SetDataVersion(
                    self.versionList.get(endpointId, {}).get(clusterId))
                return decodedData
            except Exception as ex:
                return ValueDecodeFailure(self.attributeTLVCache[endpointId][clusterId], ex)

        def handle_attribute_view(endpointId, clusterId, attributeId, attributeType):
            value = self.attributeTLVCache[endpointId][clusterId][attributeId]
            if isinstance(value, ValueDecodeFailure):
                return value
            try:
                return attributeType.FromTagDictOrRawValue(value)
            except Exception as ex:
                return ValueDecodeFailure(value, ex)

        for attributePath in self._attributeCacheUpdateNeeded:
            endpointId, clusterId, attributeId = attributePath.EndpointId, attributePath.ClusterId, attributePath.AttributeId

            if endpointId not in self._attributeCache:
                self._attributeCache[endpointId] = {}
            endpointCache = self._attributeCache[endpointId]

            if clusterId not in _ClusterIndex:
                #
                # #22599 tracks dealing with unknown clusters more
                # gracefully so that clients can still access this data.
                #
                continue

            clusterType = _ClusterIndex[clusterId]

            if self.returnClusterObject:
                endpointCache[clusterType] = handle_cluster_view(
                    endpointId, clusterId, clusterType)
            else:
                if clusterType not in endpointCache:
                    endpointCache[clusterType] = {}
                clusterCache = endpointCache[clusterType]
                clusterCache[DataVersion] = self.versionList.get(
                    endpointId, {}).get(clusterId)

                if (clusterId, attributeId) not in _AttributeIndex:
                    #
                    # #22599 tracks dealing with unknown clusters more
                    # gracefully so that clients can still access this data.
                    #
                    continue

                attributeType = _AttributeIndex[(clusterId, attributeId)][0]
                clusterCache[attributeType] = handle_attribute_view(
                    endpointId, clusterId, attributeId, attributeType)
        self._attributeCacheUpdateNeeded.clear()
        return self._attributeCache


class SubscriptionTransaction:
    def __init__(self, transaction: AsyncReadTransaction, subscriptionId, devCtrl):
        self._onResubscriptionAttemptedCb: Callable[[SubscriptionTransaction,
                                                     int, int], None] = DefaultResubscriptionAttemptedCallback
        self._onAttributeChangeCb: Callable[[
            TypedAttributePath, SubscriptionTransaction], None] = DefaultAttributeChangeCallback
        self._onEventChangeCb: Callable[[
            EventReadResult, SubscriptionTransaction], None] = DefaultEventChangeCallback
        self._onErrorCb: Callable[[
            int, SubscriptionTransaction], None] = DefaultErrorCallback
        self._readTransaction = transaction
        self._subscriptionId = subscriptionId
        self._devCtrl = devCtrl
        self._isDone = False
        self._onResubscriptionSucceededCb: Optional[Callable[[
            SubscriptionTransaction], None]] = None
        self._onResubscriptionSucceededCb_isAsync = False
        self._onResubscriptionAttemptedCb_isAsync = False
        builtins.chipStack.RegisterSubscription(self)

    def GetAttributes(self):
        ''' Returns the attribute value cache tracking the latest state on the publisher.
        '''
        return self._readTransaction._cache.GetUpdatedAttributeCache()

    def GetAttribute(self, path: TypedAttributePath) -> Any:
        ''' Returns a specific attribute given a TypedAttributePath.
        '''
        data = self._readTransaction._cache.GetUpdatedAttributeCache()

        if (self._readTransaction._cache.returnClusterObject):
            return eval(f'data[path.Path.EndpointId][path.ClusterType].{path.AttributeName}')
        else:
            return data[path.Path.EndpointId][path.ClusterType][path.AttributeType]

    def GetEvents(self):
        return self._readTransaction.GetAllEventValues()

    def OverrideLivenessTimeoutMs(self, timeoutMs: int):
        handle = chip.native.GetLibraryHandle()
        builtins.chipStack.Call(
            lambda: handle.pychip_ReadClient_OverrideLivenessTimeout(
                self._readTransaction._pReadClient, timeoutMs)
        )

    async def TriggerResubscribeIfScheduled(self, reason: str):
        handle = chip.native.GetLibraryHandle()
        await builtins.chipStack.CallAsyncWithResult(
            lambda: handle.pychip_ReadClient_TriggerResubscribeIfScheduled(
                self._readTransaction._pReadClient, reason.encode("utf-8"))
        )

    def GetReportingIntervalsSeconds(self) -> Tuple[int, int]:
        '''
        Retrieve the reporting intervals associated with an active subscription.
        This should only be called if we're of subscription interaction type and after a subscription has been established.
        '''
        handle = chip.native.GetLibraryHandle()
        handle.pychip_ReadClient_GetReportingIntervals.argtypes = [
            ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint16), ctypes.POINTER(ctypes.c_uint16)]
        handle.pychip_ReadClient_GetReportingIntervals.restype = PyChipError

        minIntervalSec = ctypes.c_uint16(0)
        maxIntervalSec = ctypes.c_uint16(0)

        builtins.chipStack.Call(
            lambda: handle.pychip_ReadClient_GetReportingIntervals(
                self._readTransaction._pReadClient, ctypes.pointer(minIntervalSec), ctypes.pointer(maxIntervalSec))
        ).raise_on_error()

        return minIntervalSec.value, maxIntervalSec.value

    def GetSubscriptionTimeoutMs(self) -> int:
        '''
        Returns the timeout(milliseconds) after which we consider the subscription to have
        dropped, if we have received no messages within that amount of time.
        Returns 0 milliseconds if a subscription has not yet been established (and
        hence the MaxInterval is not yet known), or if the subscription session
        is gone and hence the relevant MRP parameters can no longer be determined.
        '''
        timeoutMs = ctypes.c_uint32(0)
        handle = chip.native.GetLibraryHandle()
        builtins.chipStack.Call(
            lambda: handle.pychip_ReadClient_GetSubscriptionTimeoutMs(
                self._readTransaction._pReadClient, ctypes.pointer(timeoutMs))
        )
        return timeoutMs.value

    def SetResubscriptionAttemptedCallback(self, callback: Callable[[SubscriptionTransaction, int, int], None], isAsync=False):
        '''
        Sets the callback function that gets invoked anytime a re-subscription is attempted. The callback is expected
        to have the following signature:
            def Callback(transaction: SubscriptionTransaction, errorEncountered: int, nextResubscribeIntervalMsec: int)

        If the callback is an awaitable co-routine, isAsync should be set to True.
        '''
        if callback is not None:
            self._onResubscriptionAttemptedCb = callback
            self._onResubscriptionAttemptedCb_isAsync = isAsync

    def SetResubscriptionSucceededCallback(self, callback: Callable[[SubscriptionTransaction], None], isAsync=False):
        '''
        Sets the callback function that gets invoked when a re-subscription attempt succeeds. The callback
        is expected to have the following signature:
            def Callback(transaction: SubscriptionTransaction)

        If the callback is an awaitable co-routine, isAsync should be set to True.
        '''
        if callback is not None:
            self._onResubscriptionSucceededCb = callback
            self._onResubscriptionSucceededCb_isAsync = isAsync

    def SetAttributeUpdateCallback(self, callback: Callable[[TypedAttributePath, SubscriptionTransaction], None]):
        '''
        Sets the callback function for the attribute value change event,
        accepts a Callable accepts an attribute path and the cached data.
        '''
        if callback is not None:
            self._onAttributeChangeCb = callback

    def SetEventUpdateCallback(self, callback: Callable[[EventReadResult, SubscriptionTransaction], None]):
        if callback is not None:
            self._onEventChangeCb = callback

    def SetErrorCallback(self, callback: Callable[[int, SubscriptionTransaction], None]):
        '''
        Sets the callback function in case a subscription error occurred,
        accepts a Callable accepts an error code and the cached data.
        '''
        if callback is not None:
            self._onErrorCb = callback

    @property
    def OnAttributeChangeCb(self) -> Callable[[TypedAttributePath, SubscriptionTransaction], None]:
        return self._onAttributeChangeCb

    @property
    def OnEventChangeCb(self) -> Callable[[EventReadResult, SubscriptionTransaction], None]:
        return self._onEventChangeCb

    @property
    def OnErrorCb(self) -> Callable[[int, SubscriptionTransaction], None]:
        return self._onErrorCb

    @property
    def subscriptionId(self) -> int:
        return self._subscriptionId

    def Shutdown(self):
        if self._isDone:
            LOGGER.warning(
                "Subscription 0x%08x was already terminated previously!", self.subscriptionId)
            return

        handle = chip.native.GetLibraryHandle()
        builtins.chipStack.UnregisterSubscription(self)
        builtins.chipStack.Call(
            lambda: handle.pychip_ReadClient_ShutdownSubscription(
                self._readTransaction._pReadClient))
        self._isDone = True

    def __repr__(self):
        return f'<Subscription (Id={self._subscriptionId})>'


def DefaultResubscriptionAttemptedCallback(transaction: SubscriptionTransaction,
                                           terminationError, nextResubscribeIntervalMsec):
    print(
        f"Previous subscription failed with Error: {terminationError} - re-subscribing in {nextResubscribeIntervalMsec}ms...")


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


def DefaultErrorCallback(chipError: int, transaction: SubscriptionTransaction):
    print(f"Error during Subscription: Chip Stack Error {chipError}")


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

                            # Only match on classes that extend the ClusterEvent class
                            matched = [
                                value for value in base_classes if 'ClusterEvent' in str(value)]
                            if (matched == []):
                                continue

                            _EventIndex[str(EventPath(ClusterId=event.cluster_id, EventId=event.event_id))] = eval(
                                'chip.clusters.Objects.' + clusterName + '.Events.' + eventName)


class AsyncReadTransaction:
    @dataclass
    class ReadResponse:
        attributes: dict[Any, Any]
        events: list[ClusterEvent]
        tlvAttributes: dict[int, Any]

    def __init__(self, future: Future, eventLoop, devCtrl, returnClusterObject: bool):
        self._event_loop = eventLoop
        self._future = future
        self._subscription_handler = None
        self._events: List[EventReadResult] = []
        self._devCtrl = devCtrl
        self._cache = AttributeCache(returnClusterObject=returnClusterObject)
        self._changedPathSet: Set[AttributePath] = set()
        self._pReadClient = None
        self._resultError: Optional[PyChipError] = None

    def SetClientObjPointers(self, pReadClient):
        self._pReadClient = pReadClient

    def GetAllEventValues(self):
        return self._events

    def GetReadResponse(self) -> AsyncReadTransaction.ReadResponse:
        """Prepares and returns the ReadResponse object."""
        return self.ReadResponse(
            attributes=self._cache.GetUpdatedAttributeCache(),
            events=self._events,
            tlvAttributes=self._cache.attributeTLVCache
        )

    def GetSubscriptionHandler(self) -> SubscriptionTransaction | None:
        """Returns subscription transaction."""
        return self._subscription_handler

    def handleAttributeData(self, path: AttributePath, dataVersion: int, status: int, data: bytes):
        try:
            imStatus = chip.interaction_model.Status(status)

            if (imStatus != chip.interaction_model.Status.Success):
                attributeValue = ValueDecodeFailure(
                    None, chip.interaction_model.InteractionModelError(imStatus))
            else:
                tlvData = chip.tlv.TLVReader(data).get().get("Any", {})
                attributeValue = tlvData

            self._cache.UpdateTLV(path, dataVersion, attributeValue)
            self._changedPathSet.add(path)

        except Exception as ex:
            LOGGER.exception(ex)

    def handleEventData(self, header: EventHeader, path: EventPath, data: bytes, status: int):
        try:
            eventType = _EventIndex.get(str(path), None)
            eventValue = None

            if data:
                # data will be an empty buffer when we received an EventStatusIB instead of an EventDataIB.
                tlvData = chip.tlv.TLVReader(data).get().get("Any", {})

                if eventType is None:
                    eventValue = ValueDecodeFailure(
                        tlvData, LookupError("event schema not found"))
                else:
                    try:
                        eventValue = eventType.FromTLV(data)
                    except Exception as ex:
                        LOGGER.error(
                            f"Error convering TLV to Cluster Object for path: Endpoint = {path.EndpointId}/"
                            f"Cluster = {path.ClusterId}/Event = {path.EventId}")
                        LOGGER.error(
                            f"Failed Cluster Object: {str(eventType)}")
                        LOGGER.error(ex)
                        eventValue = ValueDecodeFailure(
                            tlvData, ex)

                        # If we're in debug mode, raise the exception so that we can better debug what's happening.
                        if (builtins.enableDebugMode):
                            raise

            eventResult = EventReadResult(
                Header=header, Data=eventValue, Status=chip.interaction_model.Status(status))
            self._events.append(eventResult)

            if (self._subscription_handler is not None):
                self._subscription_handler.OnEventChangeCb(
                    eventResult, self._subscription_handler)

        except Exception as ex:
            LOGGER.exception(ex)

    def handleError(self, chipError: PyChipError):
        if self._subscription_handler:
            self._subscription_handler.OnErrorCb(
                chipError.code, self._subscription_handler)
        self._resultError = chipError

    def _handleSubscriptionEstablished(self, subscriptionId):
        if not self._future.done():
            self._subscription_handler = SubscriptionTransaction(
                self, subscriptionId, self._devCtrl)
            self._future.set_result(self)
        else:
            self._subscription_handler._subscriptionId = subscriptionId
            if self._subscription_handler._onResubscriptionSucceededCb is not None:
                if (self._subscription_handler._onResubscriptionSucceededCb_isAsync):
                    self._event_loop.create_task(
                        self._subscription_handler._onResubscriptionSucceededCb(self._subscription_handler))
                else:
                    self._subscription_handler._onResubscriptionSucceededCb(
                        self._subscription_handler)

    def handleSubscriptionEstablished(self, subscriptionId):
        self._event_loop.call_soon_threadsafe(
            self._handleSubscriptionEstablished, subscriptionId)

    def handleResubscriptionAttempted(self, terminationCause: PyChipError, nextResubscribeIntervalMsec: int):
        if not self._subscription_handler:
            return
        if self._subscription_handler._onResubscriptionAttemptedCb_isAsync:
            self._event_loop.create_task(self._subscription_handler._onResubscriptionAttemptedCb(
                self._subscription_handler, terminationCause.code, nextResubscribeIntervalMsec))
        else:
            self._event_loop.call_soon_threadsafe(
                self._subscription_handler._onResubscriptionAttemptedCb,
                self._subscription_handler, terminationCause.code, nextResubscribeIntervalMsec)

    def _handleReportBegin(self):
        pass

    def _handleReportEnd(self):
        if self._subscription_handler is not None:
            for change in self._changedPathSet:
                try:
                    attribute_path = TypedAttributePath(Path=change)
                except (KeyError, ValueError) as err:
                    # path could not be resolved into a TypedAttributePath
                    LOGGER.exception(err)
                    continue
                self._subscription_handler.OnAttributeChangeCb(
                    attribute_path, self._subscription_handler)

            # Clear it out once we've notified of all changes in this transaction.
        self._changedPathSet = set()

    def _handleDone(self):
        #
        # We only set the exception/result on the future in this _handleDone call (if it hasn't
        # already been set yet, which can be in the case of subscriptions) since doing so earlier
        # would result in the callers awaiting the result to
        # move on, possibly invalidating the provided _event_loop.
        #
        if not self._future.done():
            if self._resultError is not None:
                self._future.set_exception(self._resultError.to_exception())
            else:
                self._future.set_result(self)

        #
        # Decrement the ref on ourselves to match the increment that happened at allocation.
        # This happens synchronously as part of handling done to ensure the object remains valid
        # right till the very end.
        #
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

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
        self._resultData: List[AttributeWriteResult] = []
        self._resultError: Optional[PyChipError] = None

    def handleResponse(self, path: AttributePath, status: int):
        try:
            imStatus = chip.interaction_model.Status(status)
            self._resultData.append(
                AttributeWriteResult(Path=path, Status=imStatus))
        except ValueError as ex:
            LOGGER.exception(ex)

    def handleError(self, chipError: PyChipError):
        self._resultError = chipError

    def _handleDone(self):
        #
        # We only set the exception/result on the future in this _handleDone call,
        # since doing so earlier would result in the callers awaiting the result to
        # move on, possibly invalidating the provided _event_loop.
        #
        if self._resultError is not None:
            if self._resultError.sdk_part is ErrorSDKPart.IM_GLOBAL_STATUS:
                im_status = chip.interaction_model.Status(
                    self._resultError.sdk_code)
                self._future.set_exception(
                    chip.interaction_model.InteractionModelError(im_status))
            else:
                self._future.set_exception(self._resultError.to_exception())
        else:
            self._future.set_result(self._resultData)

        #
        # Decrement the ref on ourselves to match the increment that happened at allocation.
        # This happens synchronously as part of handling done to ensure the object remains valid
        # right till the very end.
        #
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

    def handleDone(self):
        self._event_loop.call_soon_threadsafe(self._handleDone)


_OnReadAttributeDataCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint32, c_uint16, c_uint32, c_uint32, c_uint8, c_void_p, c_size_t)
_OnSubscriptionEstablishedCallbackFunct = CFUNCTYPE(None, py_object, c_uint32)
_OnResubscriptionAttemptedCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError, c_uint32)
_OnReadEventDataCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint64, c_uint8, c_uint64, c_uint8, c_void_p, c_size_t, c_uint8)
_OnReadErrorCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError)
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
def _OnReadEventDataCallback(closure, endpoint: int, cluster: int, event: c_uint64,
                             number: int, priority: int, timestamp: int, timestampType: int, data, len, status):
    dataBytes = ctypes.string_at(data, len)
    path = EventPath(ClusterId=cluster, EventId=event)

    # EventHeader is valid only when successful
    eventHeader = None
    if status == chip.interaction_model.Status.Success.value:
        eventHeader = EventHeader(
            EndpointId=endpoint, ClusterId=cluster, EventId=event, EventNumber=number, Priority=EventPriority(priority), Timestamp=timestamp, TimestampType=EventTimestampType(timestampType))
    closure.handleEventData(eventHeader, path, dataBytes[:], status)


@_OnSubscriptionEstablishedCallbackFunct
def _OnSubscriptionEstablishedCallback(closure, subscriptionId):
    closure.handleSubscriptionEstablished(subscriptionId)


@_OnResubscriptionAttemptedCallbackFunct
def _OnResubscriptionAttemptedCallback(closure, terminationCause: PyChipError, nextResubscribeIntervalMsec: int):
    closure.handleResubscriptionAttempted(
        terminationCause, nextResubscribeIntervalMsec)


@_OnReadErrorCallbackFunct
def _OnReadErrorCallback(closure, chiperror: PyChipError):
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


_OnWriteResponseCallbackFunct = CFUNCTYPE(
    None, py_object, c_uint16, c_uint32, c_uint32, c_uint16)
_OnWriteErrorCallbackFunct = CFUNCTYPE(
    None, py_object, PyChipError)
_OnWriteDoneCallbackFunct = CFUNCTYPE(
    None, py_object)


@_OnWriteResponseCallbackFunct
def _OnWriteResponseCallback(closure, endpoint: int, cluster: int, attribute: int, status):
    closure.handleResponse(AttributePath(
        EndpointId=endpoint, ClusterId=cluster, AttributeId=attribute), status)


@_OnWriteErrorCallbackFunct
def _OnWriteErrorCallback(closure, chiperror: PyChipError):
    closure.handleError(chiperror)


@_OnWriteDoneCallbackFunct
def _OnWriteDoneCallback(closure):
    closure.handleDone()


def WriteAttributes(future: Future, eventLoop, device,
                    attributes: List[AttributeWriteRequest], timedRequestTimeoutMs: Union[None, int] = None,
                    interactionTimeoutMs: Union[None, int] = None, busyWaitMs: Union[None, int] = None) -> PyChipError:
    handle = chip.native.GetLibraryHandle()

    numberOfAttributes = len(attributes)
    pyWriteAttributesArrayType = PyWriteAttributeData * numberOfAttributes
    pyWriteAttributes = pyWriteAttributesArrayType()
    for idx, attr in enumerate(attributes):
        if attr.Attribute.must_use_timed_write and timedRequestTimeoutMs is None or timedRequestTimeoutMs == 0:
            raise chip.interaction_model.InteractionModelError(
                chip.interaction_model.Status.NeedsTimedInteraction)

        tlv = attr.Attribute.ToTLV(None, attr.Data)

        pyWriteAttributes[idx].attributePath.endpointId = c_uint16(
            attr.EndpointId)
        pyWriteAttributes[idx].attributePath.clusterId = c_uint32(
            attr.Attribute.cluster_id)
        pyWriteAttributes[idx].attributePath.attributeId = c_uint32(
            attr.Attribute.attribute_id)
        pyWriteAttributes[idx].attributePath.dataVersion = c_uint32(
            attr.DataVersion)
        pyWriteAttributes[idx].attributePath.hasDataVersion = c_uint8(
            attr.HasDataVersion)
        pyWriteAttributes[idx].tlvData = cast(
            ctypes.c_char_p(bytes(tlv)), c_void_p)
        pyWriteAttributes[idx].tlvLength = c_size_t(len(tlv))

    transaction = AsyncWriteTransaction(future, eventLoop)
    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    res = builtins.chipStack.Call(
        lambda: handle.pychip_WriteClient_WriteAttributes(
            ctypes.py_object(transaction), device,
            ctypes.c_size_t(
                0 if timedRequestTimeoutMs is None else timedRequestTimeoutMs),
            ctypes.c_size_t(
                0 if interactionTimeoutMs is None else interactionTimeoutMs),
            ctypes.c_size_t(0 if busyWaitMs is None else busyWaitMs),
            pyWriteAttributes, ctypes.c_size_t(numberOfAttributes))
    )
    if not res.is_success:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def WriteGroupAttributes(groupId: int, devCtrl: c_void_p, attributes: List[AttributeWriteRequest], busyWaitMs: Union[None, int] = None) -> PyChipError:
    handle = chip.native.GetLibraryHandle()

    numberOfAttributes = len(attributes)
    pyWriteAttributesArrayType = PyWriteAttributeData * numberOfAttributes
    pyWriteAttributes = pyWriteAttributesArrayType()
    for idx, attr in enumerate(attributes):

        tlv = attr.Attribute.ToTLV(None, attr.Data)

        pyWriteAttributes[idx].attributePath.endpointId = c_uint16(
            attr.EndpointId)
        pyWriteAttributes[idx].attributePath.clusterId = c_uint32(
            attr.Attribute.cluster_id)
        pyWriteAttributes[idx].attributePath.attributeId = c_uint32(
            attr.Attribute.attribute_id)
        pyWriteAttributes[idx].attributePath.dataVersion = c_uint32(
            attr.DataVersion)
        pyWriteAttributes[idx].attributePath.hasDataVersion = c_uint8(
            attr.HasDataVersion)
        pyWriteAttributes[idx].tlvData = cast(
            ctypes.c_char_p(bytes(tlv)), c_void_p)
        pyWriteAttributes[idx].tlvLength = c_size_t(len(tlv))

    return builtins.chipStack.Call(
        lambda: handle.pychip_WriteClient_WriteGroupAttributes(
            ctypes.c_size_t(groupId), devCtrl,
            ctypes.c_size_t(0 if busyWaitMs is None else busyWaitMs),
            pyWriteAttributes, ctypes.c_size_t(numberOfAttributes))
    )


# This struct matches the PyReadAttributeParams in attribute.cpp, for passing various params together.
_ReadParams = construct.Struct(
    "MinInterval" / construct.Int16ul,
    "MaxInterval" / construct.Int16ul,
    "IsSubscription" / construct.Flag,
    "IsFabricFiltered" / construct.Flag,
    "KeepSubscriptions" / construct.Flag,
    "AutoResubscribe" / construct.Flag,
)


def Read(transaction: AsyncReadTransaction, device,
         attributes: Optional[List[AttributePath]] = None, dataVersionFilters: Optional[List[DataVersionFilter]] = None,
         events: Optional[List[EventPath]] = None, eventNumberFilter: Optional[int] = None,
         subscriptionParameters: Optional[SubscriptionParameters] = None,
         fabricFiltered: bool = True, keepSubscriptions: bool = False, autoResubscribe: bool = True) -> PyChipError:
    if (not attributes) and dataVersionFilters:
        raise ValueError(
            "Must provide valid attribute list when data version filters is not null")

    handle = chip.native.GetLibraryHandle()

    attributePathsForCffi = None
    if attributes is not None:
        numberOfAttributePaths = len(attributes)
        attributePathsForCffiArrayType = c_void_p * numberOfAttributePaths
        attributePathsForCffi = attributePathsForCffiArrayType()
        for idx, attr in enumerate(attributes):
            path = chip.interaction_model.AttributePathIBstruct.parse(
                b'\xff' * chip.interaction_model.AttributePathIBstruct.sizeof())
            if attr.EndpointId is not None:
                path.EndpointId = attr.EndpointId
            if attr.ClusterId is not None:
                path.ClusterId = attr.ClusterId
            if attr.AttributeId is not None:
                path.AttributeId = attr.AttributeId
            path = chip.interaction_model.AttributePathIBstruct.build(path)
            attributePathsForCffi[idx] = cast(ctypes.c_char_p(path), c_void_p)

    dataVersionFiltersForCffi = None
    if dataVersionFilters is not None:
        numberOfDataVersionFilters = len(dataVersionFilters)
        dataVersionFiltersForCffiArrayType = c_void_p * numberOfDataVersionFilters
        dataVersionFiltersForCffi = dataVersionFiltersForCffiArrayType()
        for idx, f in enumerate(dataVersionFilters):
            filter = chip.interaction_model.DataVersionFilterIBstruct.parse(
                b'\xff' * chip.interaction_model.DataVersionFilterIBstruct.sizeof())
            if f.EndpointId is not None:
                filter.EndpointId = f.EndpointId
            else:
                raise ValueError(
                    "DataVersionFilter must provide EndpointId.")
            if f.ClusterId is not None:
                filter.ClusterId = f.ClusterId
            else:
                raise ValueError(
                    "DataVersionFilter must provide ClusterId.")
            if f.DataVersion is not None:
                filter.DataVersion = f.DataVersion
            else:
                raise ValueError(
                    "DataVersionFilter must provide DataVersion.")
            filter = chip.interaction_model.DataVersionFilterIBstruct.build(
                filter)
            dataVersionFiltersForCffi[idx] = cast(
                ctypes.c_char_p(filter), c_void_p)

    eventPathsForCffi = None
    if events is not None:
        numberOfEvents = len(events)
        eventPathsForCffiArrayType = c_void_p * numberOfEvents
        eventPathsForCffi = eventPathsForCffiArrayType()
        for idx, event in enumerate(events):
            path = chip.interaction_model.EventPathIBstruct.parse(
                b'\xff' * chip.interaction_model.EventPathIBstruct.sizeof())
            if event.EndpointId is not None:
                path.EndpointId = event.EndpointId
            if event.ClusterId is not None:
                path.ClusterId = event.ClusterId
            if event.EventId is not None:
                path.EventId = event.EventId
            if event.Urgent is not None and subscriptionParameters is not None:
                path.Urgent = event.Urgent
            else:
                path.Urgent = 0
            path = chip.interaction_model.EventPathIBstruct.build(path)
            eventPathsForCffi[idx] = cast(ctypes.c_char_p(path), c_void_p)

    readClientObj = ctypes.POINTER(c_void_p)()

    ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
    params = _ReadParams.parse(b'\x00' * _ReadParams.sizeof())
    if subscriptionParameters is not None:
        params.MinInterval = subscriptionParameters.MinReportIntervalFloorSeconds
        params.MaxInterval = subscriptionParameters.MaxReportIntervalCeilingSeconds
        params.AutoResubscribe = autoResubscribe
        params.IsSubscription = True
        params.KeepSubscriptions = keepSubscriptions
    params.IsFabricFiltered = fabricFiltered
    params = _ReadParams.build(params)
    eventNumberFilterPtr = ctypes.POINTER(ctypes.c_ulonglong)()
    if eventNumberFilter is not None:
        eventNumberFilterPtr = ctypes.POINTER(ctypes.c_ulonglong)(
            ctypes.c_ulonglong(eventNumberFilter))

    res = builtins.chipStack.Call(
        lambda: handle.pychip_ReadClient_Read(
            ctypes.py_object(transaction),
            ctypes.byref(readClientObj),
            device,
            ctypes.c_char_p(params),
            attributePathsForCffi,
            ctypes.c_size_t(0 if attributes is None else len(attributes)),
            dataVersionFiltersForCffi,
            ctypes.c_size_t(
                0 if dataVersionFilters is None else len(dataVersionFilters)),
            eventPathsForCffi,
            ctypes.c_size_t(0 if events is None else len(events)),
            eventNumberFilterPtr))

    transaction.SetClientObjPointers(readClientObj)

    if not res.is_success:
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
    return res


def Init():
    handle = chip.native.GetLibraryHandle()

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_WriteClient_InitCallbacks.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        handle.pychip_WriteClient_WriteAttributes.restype = PyChipError
        handle.pychip_WriteClient_WriteGroupAttributes.restype = PyChipError

        # Both WriteAttributes and WriteGroupAttributes are variadic functions. As per ctype documentation
        # https://docs.python.org/3/library/ctypes.html#calling-varadic-functions, it is critical that we
        # specify the argtypes attribute for the regular, non-variadic, function arguments for this to work
        # on ARM64 for Apple Platforms.
        # TODO We could move away from a variadic function to one where we provide a vector of the
        # attribute information we want written using a vector. This possibility was not implemented at the
        # time where simply specified the argtypes, because of time constraints. This solution was quicker
        # to fix the crash on ARM64 Apple platforms without a refactor.
        handle.pychip_WriteClient_WriteAttributes.argtypes = [py_object, c_void_p,
                                                              c_size_t, c_size_t, c_size_t, POINTER(PyWriteAttributeData), c_size_t]
        handle.pychip_WriteClient_WriteGroupAttributes.argtypes = [
            c_size_t, c_void_p, c_size_t, POINTER(PyWriteAttributeData), c_size_t]

        setter.Set('pychip_WriteClient_InitCallbacks', None, [
                   _OnWriteResponseCallbackFunct, _OnWriteErrorCallbackFunct, _OnWriteDoneCallbackFunct])
        handle.pychip_ReadClient_Read.restype = PyChipError
        setter.Set('pychip_ReadClient_InitCallbacks', None, [
                   _OnReadAttributeDataCallbackFunct, _OnReadEventDataCallbackFunct,
                   _OnSubscriptionEstablishedCallbackFunct, _OnResubscriptionAttemptedCallbackFunct,
                   _OnReadErrorCallbackFunct, _OnReadDoneCallbackFunct,
                   _OnReportBeginCallbackFunct, _OnReportEndCallbackFunct])

    handle.pychip_WriteClient_InitCallbacks(
        _OnWriteResponseCallback, _OnWriteErrorCallback, _OnWriteDoneCallback)
    handle.pychip_ReadClient_InitCallbacks(
        _OnReadAttributeDataCallback, _OnReadEventDataCallback,
        _OnSubscriptionEstablishedCallback, _OnResubscriptionAttemptedCallback, _OnReadErrorCallback, _OnReadDoneCallback,
        _OnReportBeginCallback, _OnReportEndCallback)

    _BuildAttributeIndex()
    _BuildClusterIndex()
    _BuildEventIndex()
