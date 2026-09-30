/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <array>
#include <tuple>
#include <utility>

#include <device/types/root-node/RootNode.h>
#include <device/types/root-node/RootNodeFeatures.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {

/// Root Node endpoint with a compile-time-selectable set of optional features.
///
/// Each `Feature` type must satisfy the policy contract described in
/// `RootNodeFeatures.h`. The set of features is fixed at instantiation, and
/// the extra device types / client clusters they contribute are aggregated at
/// compile time so no runtime allocation is needed.
///
/// Feature registration runs in declaration order after the base clusters have
/// been registered but before the endpoint is added to the provider, so
/// AddCluster never targets an already-registered endpoint. Unregistration
/// runs in reverse.
///
/// Example:
/// @code
///   using WifiRootNode = RootNodeWith<WifiFeature>;
///   using WifiOtaRootNode = RootNodeWith<WifiFeature, OtaFeature>;
/// @endcode
template <typename... Features>
class RootNodeWith : public RootNode
{
public:
    RootNodeWith(const Context & context, const typename Features::Context &... featureContexts) :
        RootNode(context, Span<const DataModel::DeviceTypeEntry>(kDeviceTypesStorage.data(), kDeviceTypesStorage.size())),
        mFeatures(Features(featureContexts)...)
    {}
    ~RootNodeWith() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override
    {
        DeviceRegistrationTransaction transaction(*this, provider);
        ReturnErrorOnFailure(RootNode::RegisterRootClusters(endpoint, provider, composition));
        ReturnErrorOnFailure(RegisterFeaturesImpl(endpoint, provider, std::index_sequence_for<Features...>{}));
        ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
        transaction.Commit();
        return CHIP_NO_ERROR;
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        RootNode::Unregister(provider);
        UnregisterFeaturesImpl(provider, std::index_sequence_for<Features...>{});
    }

    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override
    {
        if constexpr (kClientClustersStorage.size() == 0)
        {
            return CHIP_NO_ERROR;
        }
        else
        {
            return out.ReferenceExisting(Span<const ClusterId>(kClientClustersStorage.data(), kClientClustersStorage.size()));
        }
    }

private:
    /// Composite device-type list: the root-node entry followed by each
    /// feature's extra device types, in declaration order.
    static constexpr auto kDeviceTypesStorage = aggregate::ConcatArrays(
        std::array<DataModel::DeviceTypeEntry, 1>{ Device::Type::kRootNode }, Features::kExtraDeviceTypes...);

    /// Composite client-cluster list contributed by features.
    static constexpr auto kClientClustersStorage =
        aggregate::ConcatArrays(std::array<ClusterId, 0>{}, Features::kExtraClientClusters...);

    template <std::size_t... I>
    CHIP_ERROR RegisterFeaturesImpl(EndpointId endpoint, CodeDrivenDataModelProvider & provider, std::index_sequence<I...>)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        auto attempt   = [&](auto & feature) {
            if (err == CHIP_NO_ERROR)
            {
                err = feature.RegisterFeatureClusters(endpoint, provider, mContext, GeneralCommissioning());
            }
        };
        (attempt(std::get<I>(mFeatures)), ...);
        return err;
    }

    template <std::size_t... I>
    void UnregisterFeaturesImpl(CodeDrivenDataModelProvider & provider, std::index_sequence<I...>)
    {
        (std::get<sizeof...(Features) - 1 - I>(mFeatures).UnregisterFeatureClusters(provider), ...);
    }

    std::tuple<Features...> mFeatures;
};

} // namespace app
} // namespace chip
