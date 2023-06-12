/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "lib/core/CHIPError.h"
#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <app/AttributePathParams.h>
#include <app/BufferedReadCallback.h>
#include <app/ReadClient.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <lib/support/Variant.h>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <vector>

namespace chip {
namespace app {
/*
 * This implements a cluster state cache designed to aggregate both attribute and event data received by a client
 * from either read or subscribe interactions and keep it resident and available for clients to
 * query at any time while the cache is active.
 *
 * The cache can be used with either read/subscribe, with the consumer connecting it up appropriately
 * to the right ReadClient instance.
 *
 * The cache provides an up-to-date and consistent view of the state of a target node, with the scope of the
 * state being determined by the associated ReadClient's path set.
 *
 * The cache provides a number of getters and helper functions to iterate over the topology
 * of the received data which is organized by endpoint, cluster and attribute ID (for attributes). These permit greater
 * flexibility when dealing with interactions that use wildcards heavily.
 *
 * For events, functions that permit iteration over the cached events sorted by event number are provided.
 *
 * The data is stored internally in the cache as TLV. This permits re-use of the existing cluster objects
 * to de-serialize the state on-demand.
 *
 * The cache serves as a callback adapter as well in that it 'forwards' the ReadClient::Callback calls transparently
 * through to a registered callback. In addition, it provides its own enhancements to the base ReadClient::Callback
 * to make it easier to know what has changed in the cache.
 *
 * **NOTE**
 * 1. This already includes the BufferedReadCallback, so there is no need to add that to the ReadClient callback chain.
 * 2. The same cache cannot be used by multiple subscribe/read interactions at the same time.
 *
 */
class ClusterStateCache : protected ReadClient::Callback
{
public:
    class Callback : public ReadClient::Callback
    {
    public:
        /*
         * Called anytime an attribute value has changed in the cache
         */
        virtual void OnAttributeChanged(ClusterStateCache * cache, const ConcreteAttributePath & path){};

        /*
         * Called anytime any attribute in a cluster has changed in the cache
         */
        virtual void OnClusterChanged(ClusterStateCache * cache, EndpointId endpointId, ClusterId clusterId){};

        /*
         * Called anytime an endpoint was added to the cache
         */
        virtual void OnEndpointAdded(ClusterStateCache * cache, EndpointId endpointId){};
    };

    /**
     *
     * @param [in] callback the derived callback which inherit from ReadClient::Callback
     * @param [in] highestReceivedEventNumber optional highest received event number, if cache receive the events with the number
     *             less than or equal to this value, skip those events
     * @param [in] cacheData boolean to decide whether this cache would store attribute/event data/status,
     *             the default is true.
     */
    ClusterStateCache(Callback & callback, Optional<EventNumber> highestReceivedEventNumber = Optional<EventNumber>::Missing(),
                      bool cacheData = true) :
        mCallback(callback),
        mBufferedReader(*this), mCacheData(cacheData)
    {
        mHighestReceivedEventNumber = highestReceivedEventNumber;
    }

    void SetHighestReceivedEventNumber(EventNumber highestReceivedEventNumber)
    {
        mHighestReceivedEventNumber.SetValue(highestReceivedEventNumber);
    }

    /*
     * When registering as a callback to the ReadClient, the ClusterStateCache cannot not be passed as a callback
     * directly. Instead, utilize this method below to correctly set up the callback chain such that
     * the buffered reader is the first callback in the chain before calling into cache subsequently.
     */
    ReadClient::Callback & GetBufferedCallback() { return mBufferedReader; }

    /*
     * Retrieve the value of an attribute from the cache (if present) given a concrete path by decoding
     * it using DataModel::Decode into the in-out argument 'value'.
     *
     * For some types of attributes, the value for the attribute is directly backed by the underlying TLV buffer
     * and has pointers into that buffer. (e.g octet strings, char strings and lists).  This buffer only remains
     * valid until the cached value for that path is updated, so it must not be held
     * across any async call boundaries.
     *
     * The template parameter AttributeObjectTypeT is generally expected to be a
     * ClusterName::Attributes::AttributeName::DecodableType, but any
     * object that can be decoded using the DataModel::Decode machinery will work.
     *
     * Notable return values:
     *      - If the provided attribute object's Cluster and Attribute IDs don't match that of the provided path,
     *        a CHIP_ERROR_SCHEMA_MISMATCH shall be returned.
     *
     *      - If neither data or status for the specified path don't exist in the cache, CHIP_ERROR_KEY_NOT_FOUND
     *        shall be returned.
     *
     *      - If a StatusIB is present in the cache instead of data, a CHIP_ERROR_IM_STATUS_CODE_RECEIVED error
     *        shall be returned from this call instead. The actual StatusIB can be retrieved using the GetStatus() API below.
     *
     */
    template <typename AttributeObjectTypeT>
    CHIP_ERROR Get(const ConcreteAttributePath & path, typename AttributeObjectTypeT::DecodableType & value) const
    {
        TLV::TLVReader reader;

        if (path.mClusterId != AttributeObjectTypeT::GetClusterId() || path.mAttributeId != AttributeObjectTypeT::GetAttributeId())
        {
            return CHIP_ERROR_SCHEMA_MISMATCH;
        }

        ReturnErrorOnFailure(Get(path, reader));
        return DataModel::Decode(reader, value);
    }

    /*
     * Retrieve the StatusIB for a given attribute if one exists currently in the cache.
     *
     * Notable return values:
     *      - If neither data or status for the specified path don't exist in the cache, CHIP_ERROR_KEY_NOT_FOUND
     *        shall be returned.
     *
     *      - If data exists in the cache instead of status, CHIP_ERROR_INVALID_ARGUMENT shall be returned.
     *
     */
    CHIP_ERROR GetStatus(const ConcreteAttributePath & path, StatusIB & status) const;

    /*
     * Encapsulates a StatusIB and a ConcreteAttributePath pair.
     */
    struct AttributeStatus
    {
        AttributeStatus(const ConcreteAttributePath & path, StatusIB & status) : mPath(path), mStatus(status) {}
        ConcreteAttributePath mPath;
        StatusIB mStatus;
    };

    /*
     * Retrieve the value of an entire cluster instance from the cache (if present) given a path
     * and decode it using DataModel::Decode into the in-out argument 'value'. If any StatusIBs
     * are present in the cache instead of data, they will be provided in the statusList argument.
     *
     * For some types of attributes, the value for the attribute is directly backed by the underlying TLV buffer
     * and has pointers into that buffer. (e.g octet strings, char strings and lists).  This buffer only remains
     * valid until the cached value for that path is updated, so it must not be held
     * across any async call boundaries.
     *
     * The template parameter ClusterObjectT is generally expected to be a
     * ClusterName::Attributes::DecodableType, but any
     * object that can be decoded using the DataModel::Decode machinery will work.
     *
     * Notable return values:
     *      - If neither data or status for the specified path exist in the cache, CHIP_ERROR_KEY_NOT_FOUND
     *        shall be returned.
     *
     */
    template <typename ClusterObjectTypeT>
    CHIP_ERROR Get(EndpointId endpointId, ClusterId clusterId, ClusterObjectTypeT & value,
                   std::list<AttributeStatus> & statusList) const
    {
        statusList.clear();

        return ForEachAttribute(endpointId, clusterId, [&value, this, &statusList](const ConcreteAttributePath & path) {
            TLV::TLVReader reader;
            CHIP_ERROR err;

            err = Get(path, reader);
            if (err == CHIP_ERROR_IM_STATUS_CODE_RECEIVED)
            {
                StatusIB status;
                ReturnErrorOnFailure(GetStatus(path, status));
                statusList.push_back(AttributeStatus(path, status));
                err = CHIP_NO_ERROR;
            }
            else if (err == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(DataModel::Decode(reader, path, value));
            }
            else
            {
                return err;
            }

            return CHIP_NO_ERROR;
        });
    }

    /*
     * Retrieve the value of an attribute by updating a in-out TLVReader to be positioned
     * right at the attribute value.
     *
     * The underlying TLV buffer only remains valid until the cached value for that path is updated, so it must
     * not be held across any async call boundaries.
     *
     * Notable return values:
     *      - If neither data nor status for the specified path exist in the cache, CHIP_ERROR_KEY_NOT_FOUND
     *        shall be returned.
     *
     *      - If a StatusIB is present in the cache instead of data, a CHIP_ERROR_IM_STATUS_CODE_RECEIVED error
     *        shall be returned from this call instead. The actual StatusIB can be retrieved using the GetStatus() API above.
     *
     */
    CHIP_ERROR Get(const ConcreteAttributePath & path, TLV::TLVReader & reader) const;

    /*
     * Retrieve the data version for the given cluster.  If there is no data for the specified path in the cache,
     * CHIP_ERROR_KEY_NOT_FOUND shall be returned.  Otherwise aVersion will be set to the
     * current data version for the cluster (which may have no value if we don't have a known data version
     * for it, for example because none of our paths were wildcards that covered the whole cluster).
     */
    CHIP_ERROR GetVersion(const ConcreteClusterPath & path, Optional<DataVersion> & aVersion) const;

    /*
     * Get highest received event number.
     */
    virtual CHIP_ERROR GetHighestReceivedEventNumber(Optional<EventNumber> & aEventNumber) final
    {
        aEventNumber = mHighestReceivedEventNumber;
        return CHIP_NO_ERROR;
    }

    /*
     * Retrieve the value of an event from the cache given an EventNumber by decoding
     * it using DataModel::Decode into the in-out argument 'value'.
     *
     * This should be used in conjunction with the ForEachEvent() iterator function to
     * retrieve the EventHeader (and corresponding metadata information for the event) along with its EventNumber.
     *
     * For some types of events, the values for the fields in the event are directly backed by the underlying TLV buffer
     * and have pointers into that buffer. (e.g octet strings, char strings and lists). Unlike its attribute counterpart,
     * these pointers are stable and will not change until a call to `ClearEventCache` happens.
     *
     * The template parameter EventObjectTypeT is generally expected to be a
     * ClusterName::Events::EventName::DecodableType, but any
     * object that can be decoded using the DataModel::Decode machinery will work.
     *
     * Notable return values:
     *      - If the provided event object's Cluster and Event IDs don't match those of the event in the cache,
     *        a CHIP_ERROR_SCHEMA_MISMATCH shall be returned.
     *
     *      - If event doesn't exist in the cache, CHIP_ERROR_KEY_NOT_FOUND
     *        shall be returned.
     */

    template <typename EventObjectTypeT>
    CHIP_ERROR Get(EventNumber eventNumber, EventObjectTypeT & value) const
    {
        TLV::TLVReader reader;
        CHIP_ERROR err;

        auto * eventData = GetEventData(eventNumber, err);
        ReturnErrorOnFailure(err);

        if (eventData->first.mPath.mClusterId != value.GetClusterId() || eventData->first.mPath.mEventId != value.GetEventId())
        {
            return CHIP_ERROR_SCHEMA_MISMATCH;
        }

        ReturnErrorOnFailure(Get(eventNumber, reader));
        return DataModel::Decode(reader, value);
    }

    /*
     * Retrieve the data of an event by updating a in-out TLVReader to be positioned
     * right at the structure that encapsulates the event payload.
     *
     * Notable return values:
     *      - If no event with a matching eventNumber exists in the cache, CHIP_ERROR_KEY_NOT_FOUND
     *        shall be returned.
     *
     */
    CHIP_ERROR Get(EventNumber eventNumber, TLV::TLVReader & reader) const;

    /*
     * Retrieve the StatusIB for a specific event from the event status cache (if one exists).
     * Otherwise, a CHIP_ERROR_KEY_NOT_FOUND error will be returned.
     *
     * This is to be used with the `ForEachEventStatus` iterator function.
     *
     * NOTE: Receipt of a StatusIB does not affect any pre-existing or future event data entries in the cache (and vice-versa).
     *
     */
    CHIP_ERROR GetStatus(const ConcreteEventPath & path, StatusIB & status) const;

    /*
     * Execute an iterator function that is called for every attribute
     * in a given endpoint and cluster. The function when invoked is provided a concrete attribute path
     * to every attribute that matches in the cache.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(const ConcreteAttributePath &path);
     *
     * Notable return values:
     *      - If a cluster instance corresponding to endpointId and clusterId doesn't exist in the cache,
     *        CHIP_ERROR_KEY_NOT_FOUND shall be returned.
     *
     *      - If func returns an error, that will result in termination of any further iteration over attributes
     *        and that error shall be returned back up to the original call to this function.
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachAttribute(EndpointId endpointId, ClusterId clusterId, IteratorFunc func) const
    {
        CHIP_ERROR err;

        auto clusterState = GetClusterState(endpointId, clusterId, err);
        ReturnErrorOnFailure(err);

        for (auto & attributeIter : clusterState->mAttributes)
        {
            const ConcreteAttributePath path(endpointId, clusterId, attributeIter.first);
            ReturnErrorOnFailure(func(path));
        }

        return CHIP_NO_ERROR;
    }

    /*
     * Execute an iterator function that is called for every attribute
     * for a given cluster across all endpoints in the cache. The function is passed a
     * concrete attribute path to every attribute that matches in the cache.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(const ConcreteAttributePath &path);
     *
     * Notable return values:
     *      - If func returns an error, that will result in termination of any further iteration over attributes
     *        and that error shall be returned back up to the original call to this function.
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachAttribute(ClusterId clusterId, IteratorFunc func) const
    {
        for (auto & endpointIter : mCache)
        {
            for (auto & clusterIter : endpointIter.second)
            {
                if (clusterIter.first == clusterId)
                {
                    for (auto & attributeIter : clusterIter.second.mAttributes)
                    {
                        const ConcreteAttributePath path(endpointIter.first, clusterId, attributeIter.first);
                        ReturnErrorOnFailure(func(path));
                    }
                }
            }
        }
        return CHIP_NO_ERROR;
    }

    /*
     * Execute an iterator function that is called for every cluster
     * in a given endpoint and passed a ClusterId for every cluster that
     * matches.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(ClusterId clusterId);
     *
     * Notable return values:
     *      - If func returns an error, that will result in termination of any further iteration over attributes
     *        and that error shall be returned back up to the original call to this function.
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachCluster(EndpointId endpointId, IteratorFunc func) const
    {
        auto endpointIter = mCache.find(endpointId);
        if (endpointIter->first == endpointId)
        {
            for (auto & clusterIter : endpointIter->second)
            {
                ReturnErrorOnFailure(func(clusterIter.first));
            }
        }
        return CHIP_NO_ERROR;
    }

    /*
     * Execute an iterator function that is called for every event in the event data cache that satisfies the following
     * conditions:
     *      - It matches the provided path filter
     *      - Its event number is greater than or equal to the provided minimum event number filter.
     *
     * Each filter argument can be omitted from the match criteria above by passing in an empty EventPathParams() and/or
     * a minimum event filter of 0.
     *
     * This iterator is called in increasing order from the event with the lowest event number to the highest.
     *
     * The function is passed a const reference to the EventHeader associated with that event.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(const EventHeader & eventHeader);
     *
     * Notable return values:
     *      - If func returns an error, that will result in termination of any further iteration over events
     *        and that error shall be returned back up to the original call to this function.
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachEventData(IteratorFunc func, EventPathParams pathFilter = EventPathParams(),
                                EventNumber minEventNumberFilter = 0) const
    {
        for (const auto & item : mEventDataCache)
        {
            if (pathFilter.IsEventPathSupersetOf(item.first.mPath) && item.first.mEventNumber >= minEventNumberFilter)
            {
                ReturnErrorOnFailure(func(item.first));
            }
        }

        return CHIP_NO_ERROR;
    }

    /*
     * Execute an iterator function that is called for every StatusIB in the event status cache.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(const ConcreteEventPath & eventPath, const StatusIB & statusIB);
     *
     * Notable return values:
     *      - If func returns an error, that will result in termination of any further iteration over events
     *        and that error shall be returned back up to the original call to this function.
     *
     * NOTE: Receipt of a StatusIB does not affect any pre-existing event data entries in the cache (and vice-versa).
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachEventStatus(IteratorFunc func) const
    {
        for (const auto & item : mEventStatusCache)
        {
            ReturnErrorOnFailure(func(item.first, item.second));
        }
    }

    /*
     * Clear out the event data and status caches.
     *
     * By default, this will not clear out any internally tracked event counters, specifically:
     *   - the highest event number seen so far. This is used in reads/subscribe requests to express to the receiving
     *    server to not send events that the client has already seen so far.
     *
     * That can be over-ridden by passing in 'true' to `resetTrackedEventCounters`.
     *
     */
    void ClearEventCache(bool resetTrackedEventCounters = false)
    {
        mEventDataCache.clear();
        if (resetTrackedEventCounters)
        {
            mHighestReceivedEventNumber.ClearValue();
        }

        mEventStatusCache.clear();
    }

    /*
     *  Get the last concrete report data path, if path is not concrete cluster path, return CHIP_ERROR_NOT_FOUND
     *
     */
    CHIP_ERROR GetLastReportDataPath(ConcreteClusterPath & aPath);

private:
    // An attribute state can be one of three things:
    // * If we got a path-specific error for the attribute, the corresponding
    //   status.
    // * If we got data for the attribute and we are storing data ourselves, the
    //   data.
    // * If we got data for the attribute and we are not storing data
    //   oureselves, the size of the data, so we can still prioritize sending
    //   DataVersions correctly.
    using AttributeData  = Platform::ScopedMemoryBufferWithSize<uint8_t>;
    using AttributeState = Variant<StatusIB, AttributeData, size_t>;
    // mPendingDataVersion represents a tentative data version for a cluster that we have gotten some reports for.
    //
    // mCurrentDataVersion represents a known data version for a cluster.  In order for this to have a
    // value the cluster must be included in a path in mRequestPathSet that has a wildcard attribute
    // and we must not be in the middle of receiving reports for that cluster.
    struct ClusterState
    {
        std::map<AttributeId, AttributeState> mAttributes;
        Optional<DataVersion> mPendingDataVersion;
        Optional<DataVersion> mCommittedDataVersion;
    };
    using EndpointState = std::map<ClusterId, ClusterState>;
    using NodeState     = std::map<EndpointId, EndpointState>;

    struct Comparator
    {
        bool operator()(const AttributePathParams & x, const AttributePathParams & y) const
        {
            return x.mEndpointId < y.mEndpointId || x.mClusterId < y.mClusterId;
        }
    };

    using EventData = std::pair<EventHeader, System::PacketBufferHandle>;

    //
    // This is a custom comparator for use with the std::set<EventData> below. Uniqueness
    // is determined solely by the event number associated with each event.
    //
    struct EventDataCompare
    {
        bool operator()(const EventData & lhs, const EventData & rhs) const
        {
            return (lhs.first.mEventNumber < rhs.first.mEventNumber);
        }
    };

    /*
     * These functions provide a way to index into the cached state with different sub-sets of a path, returning
     * appropriate slices of the data as requested.
     *
     * In all variants, the respective slice is returned if a valid path is provided. 'err' is updated to reflect
     * the status of the operation.
     *
     * Notable status values:
     *      - If a cluster instance corresponding to endpointId and clusterId doesn't exist in the cache,
     *        CHIP_ERROR_KEY_NOT_FOUND shall be returned.
     *
     */
    const EndpointState * GetEndpointState(EndpointId endpointId, CHIP_ERROR & err) const;
    const ClusterState * GetClusterState(EndpointId endpointId, ClusterId clusterId, CHIP_ERROR & err) const;
    const AttributeState * GetAttributeState(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                             CHIP_ERROR & err) const;

    const EventData * GetEventData(EventNumber number, CHIP_ERROR & err) const;

    /*
     * Updates the state of an attribute in the cache given a reader. If the reader is null, the state is updated
     * with the provided status.
     */
    CHIP_ERROR UpdateCache(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus);

    /*
     * If apData is not null, updates the cached event set with the specified event header + payload.
     * If apData is null and apStatus is not null, the StatusIB is stored in the event status cache.
     *
     * Storage of either of these do not affect pre-existing data for the other events in the cache.
     *
     */
    CHIP_ERROR UpdateEventCache(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus);

    //
    // ReadClient::Callback
    //
    void OnReportBegin() override;
    void OnReportEnd() override;
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnError(CHIP_ERROR aError) override { return mCallback.OnError(aError); }

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnDone(ReadClient * apReadClient) override
    {
        mRequestPathSet.clear();
        return mCallback.OnDone(apReadClient);
    }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override
    {
        mCallback.OnSubscriptionEstablished(aSubscriptionId);
    }

    CHIP_ERROR OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override
    {
        return mCallback.OnResubscriptionNeeded(apReadClient, aTerminationCause);
    }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        mCallback.OnDeallocatePaths(std::move(aReadPrepareParams));
    }

    virtual CHIP_ERROR OnUpdateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                     const Span<AttributePathParams> & aAttributePaths,
                                                     bool & aEncodedDataVersionList) override;

    void OnUnsolicitedMessageFromPublisher(ReadClient * apReadClient) override
    {
        return mCallback.OnUnsolicitedMessageFromPublisher(apReadClient);
    }

    // Commit the pending cluster data version, if there is one.
    void CommitPendingDataVersion();

    // Get our list of data version filters, sorted from larges to smallest by the total size of the TLV
    // payload for the filter's cluster.  Applying filters in this order should maximize space savings
    // on the wire if not all filters can be applied.
    void GetSortedFilters(std::vector<std::pair<DataVersionFilter, size_t>> & aVector) const;

    CHIP_ERROR GetElementTLVSize(TLV::TLVReader * apData, size_t & aSize);

    Callback & mCallback;
    NodeState mCache;
    std::set<ConcreteAttributePath> mChangedAttributeSet;
    std::set<AttributePathParams, Comparator> mRequestPathSet; // wildcard attribute request path only
    std::vector<EndpointId> mAddedEndpoints;

    std::set<EventData, EventDataCompare> mEventDataCache;
    Optional<EventNumber> mHighestReceivedEventNumber;
    std::map<ConcreteEventPath, StatusIB> mEventStatusCache;
    BufferedReadCallback mBufferedReader;
    ConcreteClusterPath mLastReportDataPath = ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId);
    const bool mCacheData                   = true;
};

}; // namespace app
}; // namespace chip
