/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#if 0
// Helper functions to convert AdjustmentCauseEnum to ForecastUpdateReasonEnum and PowerAdjustReasonEnum
ForecastUpdateReasonEnum AdjustmentCauseToForecastUpdateReason(AdjustmentCauseEnum cause);
PowerAdjustReasonEnum AdjustmentCauseToPowerAdjustReason(AdjustmentCauseEnum cause);
} // namespace CommissioningProxy
#endif

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

private:
    DataModel::ActionReturnStatus HandleProxyScanRequest(const DataModel::InvokeRequest & request,
                                                           TLV::TLVReader & input_arguments, CommandHandler * handler);
#if 0
    // DataModel::ActionReturnStatus CheckOptOutAllowsRequest(CommissioningProxy::AdjustmentCauseEnum adjustmentCause);
    DataModel::ActionReturnStatus HandlePowerAdjustRequest(const DataModel::InvokeRequest & request,
                                                           TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleCancelPowerAdjustRequest(const DataModel::InvokeRequest & request,
                                                                 TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleStartTimeAdjustRequest(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandlePauseRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler);
    DataModel::ActionReturnStatus HandleResumeRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                      CommandHandler * handler);
    DataModel::ActionReturnStatus HandleModifyForecastRequest(const DataModel::InvokeRequest & request,
                                                              TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleRequestConstraintBasedForecast(const DataModel::InvokeRequest & request,
                                                                       TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleCancelRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                      CommandHandler * handler);
#endif
    CommissioningProxy::Delegate & mDelegate;
    const BitFlags<CommissioningProxy::Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
