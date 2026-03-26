/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/CommissioningProxy/AttributeIds.h>
#include <clusters/CommissioningProxy/ClusterId.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <clusters/CommissioningProxy/Events.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyCluster : public DefaultServerCluster
{
private:
    using OptionalAttributesSet = OptionalAttributeSet<
        CommissioningProxy::Attributes::MaxCachedResults::Id,
        CommissioningProxy::Attributes::NumCachedResults::Id,
        CommissioningProxy::Attributes::CacheTimeout::Id,
        CommissioningProxy::Attributes::CachedResults::Id,
        CommissioningProxy::Attributes::WiFiBand::Id
        >;

public:
    enum State_t
    {
        kState_CPDisconnected = 0,
        kState_CPConnected
    };

    struct Config
    {
        EndpointId endpointId;
        BitMask<CommissioningProxy::Feature> featureFlags;
        CommissioningProxy::Delegate & delegate;

        Config(EndpointId aEndpointId, BitMask<CommissioningProxy::Feature> aFeatures,
               CommissioningProxy::Delegate & aDelegate) :
            endpointId(aEndpointId),
            featureFlags(aFeatures), delegate(aDelegate)
        {}
    };

    // Don't allow the default constructor as this cluster requires a delegate to be set
    CommissioningProxyCluster() = delete;

    CommissioningProxyCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, CommissioningProxy::Id }), mDelegate(config.delegate),
        mFeatureFlags(config.featureFlags), mEnabledOptionalAttributes([&]() {
            OptionalAttributesSet attrs;
            attrs.Set<CommissioningProxy::Attributes::MaxCachedResults::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::NumCachedResults::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::CacheTimeout::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::CachedResults::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::WiFiBand::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kWiFiNetworkInterface));
            return attrs;
        }())
    {
        mDelegate.SetEndpointId(config.endpointId);

        // Initialize local state
        mMainCommissioningProxyState = kState_CPDisconnected;

        // Bind delegate back to this cluster (no RTTI needed)
        mDelegate.SetServer(this);
    }

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<CommissioningProxy::Feature> & Features() const { return mFeatureFlags; }
    CommissioningProxy::Delegate & GetDelegate() { return mDelegate; }

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR SetCPState(State_t state);
    CommissioningProxyCluster::State_t GetCPState(void);

    /**
     * @brief Notify subscribers that CachedResults and NumCachedResults have changed.
     *
     * Called by the delegate whenever the background scan cache is updated.
     */
    void MarkCachedResultsDirty()
    {
        NotifyAttributeChanged(CommissioningProxy::Attributes::CachedResults::Id);
        NotifyAttributeChanged(CommissioningProxy::Attributes::NumCachedResults::Id);
    }

private:
    DataModel::ActionReturnStatus HandleProxyConnectRequest(const DataModel::InvokeRequest & request,
                                                            TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleProxyDisconnectRequest(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleProxyScanRequest(const DataModel::InvokeRequest & request,
                                                         TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleProxyBackGroundScanStartRequest(const DataModel::InvokeRequest & request,
                                                                        TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleProxyBackGroundScanStopRequest(const DataModel::InvokeRequest & request,
                                                                       TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleProxyMessageRequest(const DataModel::InvokeRequest & request,
                                                            TLV::TLVReader & input_arguments, CommandHandler * handler);

    CommissioningProxy::Delegate & mDelegate;
    const BitFlags<CommissioningProxy::Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;
    State_t mMainCommissioningProxyState;

};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
