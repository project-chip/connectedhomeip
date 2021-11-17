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

#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <app/AttributePathParams.h>
#include <app/ReadClient.h>
#include <vector>
#include <map>
#include <lib/support/Variant.h>

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
 */
class AttributeCache : public ReadClient::Callback
{
public:
    class Callback : public ReadClient::Callback {
        /*
         * Called anytime an attribute value has changed in the cache
         */
        virtual void OnAttributeChanged(ConcreteAttributePath &path) {};

        /*
         * Called anytime any attribute in a cluster has changed in the cache
         */
        virtual void OnClusterChanged(EndpointId endpointId, ClusterId clusterId) {};

        /*
         * Called anytime an endpoint was added to the cache
         */
        virtual void OnEndpointAdded(EndpointId endpointId) {};
    };

    AttributeCache(Callback & callback) : mCallback(callback) {}

    /*
     * Retrieve the value of an attribute from the cache (if present) given a concrete path and decode
     * is using DataModel::Decode into the in-out argument 'value'.
     *
     * The template parameter AttributeObjectTypeT is generally expected to be a
     * ClusterName::Attributes::AttributeName::DecodableType, but any
     * object that can be decoded using the DataModel::Decode machinery will work.
     *
     * If it isn't present, CHIP_ERROR_INVALID_ARGUMENT is returned.
     *
     */
    template <typename AttributeObjectTypeT>
    CHIP_ERROR Get(ConcreteAttributePath &path, AttributeObjectTypeT &value);

    /*
     * Retrieve the value of an entire cluster instance from the cache (if present) given a path
     * and decode it using DataModel::Decode into the in-out argument 'value'.
     *
     * The template parameter ClusterObjectT is generally expected to be a
     * ClusterName::DecodableType, but any
     * object that can be decoded using the DataModel::Decode machinery will work.
     *
     * If it isn't present, CHIP_ERROR_INVALID_ARGUMENT is returned.
     *
     */
    template <typename ClusterObjectTypeT>
    CHIP_ERROR Get(EndpointId endpointId, ClusterId clusterId, ClusterObjectTypeT &value);

    /*
     * Retrieve the value of an attribute by updating a in-out TLVReader to be positioned
     * right at the attribute value.
     *
     * If it isn't present, CHIP_ERROR_INVALID_ARGUMENT is returned.
     *
     */
    CHIP_ERROR Get(ConcreteAttributePath &path, TLV::TLVReader &reader);

    size_t GetNumEndpoints();
    size_t GetNumClusters(EndpointId endpointId);
    size_t GetNumAttributes(EndpointId endpointId, ClusterId clusterId);

    /*
     * Execute an iterator function that is called for every attribute
     * in a given endpoint and cluster and is passed in a concrete attribute path
     * to every attribute that matches in the cache.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(ConcreteAttributePath &path);
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachAttribute(EndpointId endpointId, ClusterId clusterId);

    /*
     * Execute an iterator function that is called for every attribute
     * for a given cluster and is passed a concrete attribute path
     * to every attribute that matches in the cache.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(ConcreteAttributePath &path);
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachAttribute(ClusterId clusterId);

    /*
     * Execute an iterator function that is called for every cluster
     * in a given endpoint is passed in a ClusterId for every cluster that
     * matches.
     *
     * The iterator is expected to have this signature:
     *      CHIP_ERROR IteratorFunc(ClusterId clusterId);
     *
     */
    template <typename IteratorFunc>
    CHIP_ERROR ForEachCluster(EndpointId endpointId);

private:
    using AttributeState = Variant<System::PacketBufferHandle, StatusIB>;
    using ClusterState = std::map<AttributeId, AttributeState>;
    using EndpointState = std::map<ClusterId, ClusterState>;

    //
    // ReadClient::Callback
    //
    void OnReportBegin(const ReadClient *apReadClient) override;
    void OnReportEnd(const ReadClient *apReadClient) override;
    void OnAttributeData(const ReadClient * apReadClient, const ConcreteAttributePath & aPath, TLV::TLVReader * apData,
                         const StatusIB & aStatus) override;
    void OnError(const ReadClient * apReadClient, CHIP_ERROR aError) override { return mCallback.OnError(apReadClient, aError); }
    void OnEventData(const ReadClient * apReadClient, TLV::TLVReader & aEventReports) override
    {
        return mCallback.OnEventData(apReadClient, aEventReports);
    }

    void OnDone(ReadClient * apReadClient) override { return mCallback.OnDone(apReadClient); }
    void OnSubscriptionEstablished(const ReadClient * apReadClient) override { mCallback.OnSubscriptionEstablished(apReadClient); }

    Callback & mCallback;

    std::map<EndpointId, EndpointState> mCache;
};

}; // namespace app
}; // namespace chip
