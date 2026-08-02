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

template <typename... Delegates>
class ThermostatClusterWithFeatures : public ThermostatCluster
{
public:
    static constexpr bool kHasCoreDelegate = (std::is_same_v<Thermostat::Delegate, Delegates> || ...);
    static constexpr bool kHasPresets      = (std::is_same_v<ThermostatPresets::Delegate, Delegates> || ...);
    static constexpr bool kHasSuggestions  = (std::is_same_v<ThermostatSuggestions::Delegate, Delegates> || ...);
    static constexpr bool kHasOccupancy    = (std::is_same_v<ThermostatOccupancy::Delegate, Delegates> || ...);

    ThermostatClusterWithFeatures(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features,
                                  const OptionalAttributes & optionalAttributes) :
        ThermostatCluster(aEndpointId, features, optionalAttributes)
    {
        if constexpr (kHasPresets)
        {
            mPresets.SetCluster(*this);
        }
        if constexpr (kHasSuggestions)
        {
            mSuggestions.SetCluster(*this);
            if constexpr (kHasPresets)
            {
                mSuggestions.SetPresets(&mPresets);
            }
        }
        if constexpr (kHasOccupancy)
        {
            mOccupancy.SetCluster(*this);
        }
    }

    template <typename DelegateT>
    void SetDelegate(DelegateT * delegate)
    {
        if constexpr (std::is_base_of_v<Thermostat::Delegate, DelegateT>)
        {
            mDelegate = delegate;
        }
        if constexpr (kHasPresets)
        {
            if (auto * presets = dynamic_cast<ThermostatPresets::Delegate *>(delegate))
            {
                mPresets.SetDelegate(presets);
            }
        }
        if constexpr (kHasSuggestions)
        {
            if (auto * suggestions = dynamic_cast<ThermostatSuggestions::Delegate *>(delegate))
            {
                mSuggestions.SetDelegate(suggestions);
            }
        }
        if constexpr (kHasOccupancy)
        {
            if (auto * occupancy = dynamic_cast<ThermostatOccupancy::Delegate *>(delegate))
            {
                mOccupancy.SetDelegate(occupancy);
            }
        }
    }

    bool IsOccupied() const override
    {
        if constexpr (kHasOccupancy)
        {
            return mOccupancy.IsOccupied();
        }
        return ThermostatCluster::IsOccupied();
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
                mPresets.SetActivePreset(std::nullopt);
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
    [[no_unique_address]] std::conditional_t<kHasPresets, ThermostatPresets, std::monostate> mPresets;
    [[no_unique_address]] std::conditional_t<kHasSuggestions, ThermostatSuggestions, std::monostate> mSuggestions;
    [[no_unique_address]] std::conditional_t<kHasOccupancy, ThermostatOccupancy, std::monostate> mOccupancy;
};

using DefaultThermostatCluster = ThermostatClusterWithFeatures<Thermostat::Delegate, ThermostatPresets::Delegate,
                                                               ThermostatSuggestions::Delegate, ThermostatOccupancy::Delegate>;

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
