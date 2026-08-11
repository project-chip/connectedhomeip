/**
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

#include "energy-preference-delegate.h"
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/EnergyPreference/Attributes.h>

namespace chip::app::Clusters {

class EnergyPreferenceCluster : public DefaultServerCluster
{
    // from spec 9.7.5.2 about the `Balance` struct type
    constexpr static size_t kMaxBalanceStructLabelLength = 64;

public:
    EnergyPreferenceCluster(EndpointId endpointId, BitFlags<EnergyPreference::Feature> features) :
        DefaultServerCluster({ endpointId, EnergyPreference::Id }), mFeatures(features)
    {
        VerifyOrDie(mFeatures.HasAny()); // At least one feature must be set.
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetCurrentEnergyBalance(uint8_t currentEnergyBalance)
    {
        return SetCurrentUint8Attribute(
            mCurrentEnergyBalance, currentEnergyBalance, sDelegate->GetNumEnergyBalances(GetEndpointId()),
            EnergyPreference::Attributes::CurrentEnergyBalance::Id, EnergyPreference::Feature::kEnergyBalance,
            &EnergyPreference::Delegate::OnCurrentEnergyBalanceChanged);
    }
    CHIP_ERROR SetCurrentLowPowerModeSensitivity(uint8_t currentLowPowerModeSensitivity)
    {
        return SetCurrentUint8Attribute(mCurrentLowPowerModeSensitivity, currentLowPowerModeSensitivity,
                                        sDelegate->GetNumLowPowerModeSensitivities(GetEndpointId()),
                                        EnergyPreference::Attributes::CurrentLowPowerModeSensitivity::Id,
                                        EnergyPreference::Feature::kLowPowerModeSensitivity,
                                        &EnergyPreference::Delegate::OnCurrentLowPowerModeSensitivityChanged);
    }
    uint8_t GetCurrentEnergyBalance() const { return mCurrentEnergyBalance; }
    uint8_t GetCurrentLowPowerModeSensitivity() const { return mCurrentLowPowerModeSensitivity; }

    static void SetDelegate(EnergyPreference::Delegate * delegate);
    static EnergyPreference::Delegate * GetDelegate();

private:
    EndpointId GetEndpointId() const { return mPath.mEndpointId; }

    using OnCurrentUint8AttributeChangedCallback = void (EnergyPreference::Delegate::*)(EndpointId, uint8_t);
    CHIP_ERROR SetCurrentUint8Attribute(uint8_t & attributeValue, uint8_t newValue, size_t correspondingArraySize,
                                        AttributeId attributeId, EnergyPreference::Feature featureGate,
                                        OnCurrentUint8AttributeChangedCallback onChangedCallback);

    using BalanceStructAtIndexGetter = CHIP_ERROR (EnergyPreference::Delegate::*)(chip::EndpointId, size_t, chip::Percent &,
                                                                                  chip::Optional<chip::MutableCharSpan> &);
    CHIP_ERROR ReadBalanceStructListAttribute(const ConcreteAttributePath & path, AttributeValueEncoder & encoder,
                                              BalanceStructAtIndexGetter getter);
    CHIP_ERROR ReadEnergyBalances(const ConcreteAttributePath & path, AttributeValueEncoder & encoder)
    {
        return ReadBalanceStructListAttribute(path, encoder, &EnergyPreference::Delegate::GetEnergyBalanceAtIndex);
    }
    CHIP_ERROR ReadEnergyPriorities(const ConcreteAttributePath & path, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadLowPowerModeSensitivities(const ConcreteAttributePath & path, AttributeValueEncoder & encoder)
    {
        return ReadBalanceStructListAttribute(path, encoder, &EnergyPreference::Delegate::GetLowPowerModeSensitivityAtIndex);
    }

    BitFlags<EnergyPreference::Feature> mFeatures;

    uint8_t mCurrentEnergyBalance{};
    uint8_t mCurrentLowPowerModeSensitivity{};

    inline static EnergyPreference::Delegate * sDelegate = nullptr;
};

} // namespace chip::app::Clusters
