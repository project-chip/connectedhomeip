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
#include <app/data-model/Decode.h>
#include <lib/support/Variant.h>
#include <list>
#include <map>
#include <set>
#include <vector>

namespace chip {
namespace app {

/*
 * This implements an attribute cache designed to aggregate attribute data received by a client
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
 * of the received data which is organized by endpoint, cluster and attribute ID. These permit greater
 * flexibility when dealing with interactions that use wildcards heavily.
 *
 * The data is stored internally in the cache as TLV. This permits re-use of the existing cluster objects
 * to de-serialize the state on-demand.
 *
 * The cache serves as a callback adapter as well in that it 'forwards' the ReadClient::Callback calls transparently
 * through to a registered callback. In addition, it provides its own enhancements to the base ReadClient::Callback
 * to make it easier to know what has changed in the cache.
 *
 * **NOTE** This already includes the BufferedReadCallback, so there is no need to add that to the ReadClient callback chain.
 *
 */
class AttributeCache : protected ReadClient::Callback
{
public:
    class Callback : public ReadClient::Callback
    {
    public:
        /*
         * Called anytime an attribute value has changed in the cache
         */
        virtual void OnAttributeChanged(AttributeCache * cache, const ConcreteAttributePath & path){};

        /*
         * Called anytime any attribute in a cluster has changed in the cache
         */
        virtual void OnClusterChanged(AttributeCache * cache, EndpointId endpointId, ClusterId clusterId){};

        /*
         * Called anytime an endpoint was added to the cache
         */
        virtual void OnEndpointAdded(AttributeCache * cache, EndpointId endpointId){};
    };

    AttributeCache(Callback & callback) : mCallback(callback), mBufferedReader(*this) {}

    /*
     * When registering as a callback to the ReadClient, the AttributeCache cannot not be passed as a callback
     * directly. Instead, utilize this method below to correctly set up the callback chain such that
     * the buffered reader is the first callback in the chain before calling into cache subsequently.
     */
    ReadClient::Callback & GetBufferedCallback() { return mBufferedReader; }

    /*
     * Retrieve the value of an attribute from the cache (if present) given a concrete path and decode
     * is using DataModel::Decode into the in-out argument 'value'.
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
    CHIP_ERROR Get(const ConcreteAttributePath & path, typename AttributeObjectTypeT::DecodableType & value)
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
    CHIP_ERROR GetStatus(const ConcreteAttributePath & path, StatusIB & status);

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
    CHIP_ERROR Get(EndpointId endpointId, ClusterId clusterId, ClusterObjectTypeT & value, std::list<AttributeStatus> & statusList)
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
    CHIP_ERROR Get(const ConcreteAttributePath & path, TLV::TLVReader & reader);

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
    CHIP_ERROR ForEachAttribute(EndpointId endpointId, ClusterId clusterId, IteratorFunc func)
    {
        CHIP_ERROR err;

        auto clusterState = GetClusterState(endpointId, clusterId, err);
        ReturnErrorOnFailure(err);

        for (auto & attributeIter : *clusterState)
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
    CHIP_ERROR ForEachAttribute(ClusterId clusterId, IteratorFunc func)
    {
        for (auto & endpointIter : mCache)
        {
            for (auto & clusterIter : endpointIter.second)
            {
                if (clusterIter.first == clusterId)
                {
                    for (auto & attributeIter : clusterIter.second)
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
    CHIP_ERROR ForEachCluster(EndpointId endpointId, IteratorFunc func)
    {
        auto endpointIter = mCache.find(endpointId);
        if (endpointIter->first == endpointId)
        {
            for (auto & clusterIter : endpointIter->second)
            {
                ReturnErrorOnFailure(func(clusterIter.first));
            }
        }
    }

private:
    using AttributeState = Variant<System::PacketBufferHandle, StatusIB>;
    using ClusterState   = std::map<AttributeId, AttributeState>;
    using EndpointState  = std::map<ClusterId, ClusterState>;
    using NodeState      = std::map<EndpointId, EndpointState>;

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
    EndpointState * GetEndpointState(EndpointId endpointId, CHIP_ERROR & err);
    ClusterState * GetClusterState(EndpointId endpointId, ClusterId clusterId, CHIP_ERROR & err);
    AttributeState * GetAttributeState(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, CHIP_ERROR & err);

    /*
     * Updates the state of an attribute in the cache given a reader. If the reader is null, the state is updated
     * with the provided status.
     */
    CHIP_ERROR UpdateCache(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus);

private:
    //
    // ReadClient::Callback
    //
    void OnReportBegin() override;
    void OnReportEnd() override;
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnError(CHIP_ERROR aError) override { return mCallback.OnError(aError); }

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override
    {
        return mCallback.OnEventData(aEventHeader, apData, apStatus);
    }

    void OnDone() override { return mCallback.OnDone(); }
    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override { mCallback.OnSubscriptionEstablished(aSubscriptionId); }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        return mCallback.OnDeallocatePaths(std::move(aReadPrepareParams));
    }

private:
    Callback & mCallback;
    NodeState mCache;
    std::set<ConcreteAttributePath> mChangedAttributeSet;
    std::vector<EndpointId> mAddedEndpoints;
    BufferedReadCallback mBufferedReader;
};

}; // namespace app
}; // namespace chip
