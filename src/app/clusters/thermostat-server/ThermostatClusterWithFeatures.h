/**
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include "ThermostatCluster.h"
#include "ThermostatClusterOccupancy.h"
#include "ThermostatClusterPresets.h"
#include "ThermostatClusterSuggestions.h"
#include <type_traits>
#include <variant>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

namespace {

template <typename TargetDelegate, typename First, typename... Rest>
constexpr decltype(auto) FindDelegate(First && first, Rest &&... rest)
{
    if constexpr (std::is_base_of_v<TargetDelegate, std::decay_t<First>>)
    {
        return std::forward<First>(first);
    }
    else if constexpr (sizeof...(Rest) > 0)
    {
        return FindDelegate<TargetDelegate>(std::forward<Rest>(rest)...);
    }
    else
    {
        static_assert(std::is_base_of_v<TargetDelegate, std::decay_t<First>>,
                      "Exhausted list of delegates without finding target delegate");
    }
}

template <bool Enabled, typename FeatureType, typename... DelegateArgs, typename... ExtraArgs>
static auto MakeFeature(const std::tuple<DelegateArgs &...> & delegates, ExtraArgs &&... extraArgs)
{
    if constexpr (Enabled)
    {
        return std::apply(
            [&](auto &... dels) {
                return FeatureType(std::forward<ExtraArgs>(extraArgs)..., FindDelegate<typename FeatureType::Delegate>(dels...));
            },
            delegates);
    }
    else
    {
        return std::monostate{};
    }
}

template <typename Target, typename... Args>
inline constexpr bool kArgsHasDelegate = (std::is_base_of_v<Target, std::decay_t<Args>> || ...);

} // namespace

template <typename... Delegates>
class ThermostatClusterWithFeatures : public ThermostatCluster
{
public:
    static constexpr bool kHasPresets     = (std::is_base_of_v<ThermostatPresets::Delegate, Delegates> || ...);
    static constexpr bool kHasSuggestions = (std::is_base_of_v<ThermostatSuggestions::Delegate, Delegates> || ...);
    static constexpr bool kHasOccupancy   = (std::is_base_of_v<ThermostatOccupancy::Delegate, Delegates> || ...);

    static_assert(!kHasSuggestions || kHasPresets, "Suggestions feature requires Presets feature");

    ThermostatClusterWithFeatures(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features, const Config & config,
                                  Delegates &... delegates) :
        ThermostatCluster(aEndpointId, features, config, FindDelegate<Thermostat::Delegate>(delegates...)),
        mPresets(MakeFeature<kHasPresets, ThermostatPresets>(std::forward_as_tuple(delegates...), *this)),
        mSuggestions(MakeFeature<kHasSuggestions, ThermostatSuggestions>(std::forward_as_tuple(delegates...), *this, mPresets)),
        mOccupancy(MakeFeature<kHasOccupancy, ThermostatOccupancy>(std::forward_as_tuple(delegates...)))
    {
        static_assert(kArgsHasDelegate<Thermostat::Delegate, Delegates...>,
                      "Missing Thermostat::Delegate in constructor arguments");
    }

    bool IsOccupied() const override
    {
        if constexpr (kHasOccupancy)
        {
            return mOccupancy.IsOccupied();
        }
        return ThermostatCluster::IsOccupied();
    }

    template <bool Cond = kHasOccupancy, typename std::enable_if_t<Cond, int> = 0>
    Protocols::InteractionModel::Status SetOccupied(BitMask<OccupancyBitmap> occupied)
    {
        return mOccupancy.SetOccupied(occupied);
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if constexpr (kHasOccupancy)
        {
            if (auto status = mOccupancy.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasSuggestions)
        {
            if (auto status = mSuggestions.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        return ThermostatCluster::ReadAttribute(request, encoder);
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        auto status = ThermostatCluster::WriteAttribute(request, decoder);
        if constexpr (kHasPresets)
        {
            if (status.IsSuccess() && IsActiveSetpoint(request.path.mAttributeId))
            {
                ChipLogProgress(Zcl, "Setting active preset to null");
                mPresets.SetActivePreset(DataModel::NullNullable);
            }
        }
        return status;
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.InvokeCommand(request, input_arguments, handler))
            {
                return status;
            }
        }
        if constexpr (kHasSuggestions)
        {
            bool handled = false;
            if (auto status = mSuggestions.InvokeCommand(request, input_arguments, handler, handled))
            {
                return status;
            }
            if (handled)
            {
                return std::nullopt;
            }
        }
        return ThermostatCluster::InvokeCommand(request, input_arguments, handler);
    }

    Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteBegin(attributeId))
            {
                return *status;
            }
        }
        return ThermostatCluster::OnAtomicWriteBegin(attributeId);
    }

    Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWritePrecommit(attributeId))
            {
                return *status;
            }
        }
        return ThermostatCluster::OnAtomicWritePrecommit(attributeId);
    }

    Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteCommit(attributeId))
            {
                return *status;
            }
        }
        return ThermostatCluster::OnAtomicWriteCommit(attributeId);
    }

    Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteRollback(attributeId))
            {
                return *status;
            }
        }
        return ThermostatCluster::OnAtomicWriteRollback(attributeId);
    }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto timeout = mPresets.GetMaxAtomicWriteTimeout(attributeId))
            {
                return timeout;
            }
        }
        return ThermostatCluster::GetMaxAtomicWriteTimeout(attributeId);
    }

private:
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasPresets, ThermostatPresets, std::monostate> mPresets;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasSuggestions, ThermostatSuggestions, std::monostate> mSuggestions;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasOccupancy, ThermostatOccupancy, std::monostate> mOccupancy;
};

template <typename... DelegateArgs>
ThermostatClusterWithFeatures(EndpointId, BitFlags<Thermostat::Feature>, const ThermostatCluster::Config &,
                              DelegateArgs &...) -> ThermostatClusterWithFeatures<std::decay_t<DelegateArgs>...>;

using DefaultThermostatCluster =
    ThermostatClusterWithFeatures<ThermostatPresets::Delegate, ThermostatSuggestions::Delegate, ThermostatOccupancy::Delegate>;

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
