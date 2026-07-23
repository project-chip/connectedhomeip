/*
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

    CHIP_ERROR SetCurrentEnergyBalance(uint8_t currentEnergyBalance);
    CHIP_ERROR SetCurrentLowPowerModeSensitivity(uint8_t currentLowPowerModeSensitivity);
    uint8_t GetCurrentEnergyBalance() const { return mCurrentEnergyBalance; }
    uint8_t GetCurrentLowPowerModeSensitivity() const { return mCurrentLowPowerModeSensitivity; }

    static void SetDelegate(EnergyPreference::Delegate * aDelegate);
    static EnergyPreference::Delegate * GetDelegate();

private:
    EndpointId GetEndpointId() const { return mPath.mEndpointId; }
    CHIP_ERROR ReadEnergyBalances(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadEnergyPriorities(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadLowPowerModeSensitivities(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);

    BitFlags<EnergyPreference::Feature> mFeatures;

    uint8_t mCurrentEnergyBalance{};
    uint8_t mCurrentLowPowerModeSensitivity{};

    inline static EnergyPreference::Delegate * sDelegate = nullptr;
};

} // namespace chip::app::Clusters
