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

#include <app/server-cluster/DefaultServerCluster.h>

#include <clusters/ElectricalDistribution/Attributes.h>
#include <clusters/ElectricalDistribution/Enums.h>

namespace chip::app::Clusters {

/// Server for the Electrical Distribution cluster (0x00A2).
///
/// All five mandatory attributes are read-only (View access) and Nullable; their values are
/// supplied at construction via StartupConfiguration. EndOfLife additionally exposes a setter
/// so a test-event-trigger (or a real delegate) can drive its transitions and generate the
/// corresponding attribute-changed notifications.
class ElectricalDistributionCluster : public DefaultServerCluster
{
public:
    struct StartupConfiguration
    {
        ElectricalDistribution::Attributes::MaxContinuousCurrent::TypeInfo::Type maxContinuousCurrent;
        ElectricalDistribution::Attributes::MaxVoltage::TypeInfo::Type maxVoltage;
        ElectricalDistribution::Attributes::NumberOfPoles::TypeInfo::Type numberOfPoles;
        ElectricalDistribution::Attributes::EndOfLife::TypeInfo::Type endOfLife;
        ElectricalDistribution::Attributes::ServiceEntranceRated::TypeInfo::Type serviceEntranceRated;
    };

    ElectricalDistributionCluster(EndpointId endpointId, const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /// Update the EndOfLife attribute value (emits an attribute-changed notification).
    CHIP_ERROR SetEndOfLife(const ElectricalDistribution::Attributes::EndOfLife::TypeInfo::Type & endOfLife);

protected:
    ElectricalDistribution::Attributes::MaxContinuousCurrent::TypeInfo::Type mMaxContinuousCurrent;
    ElectricalDistribution::Attributes::MaxVoltage::TypeInfo::Type mMaxVoltage;
    ElectricalDistribution::Attributes::NumberOfPoles::TypeInfo::Type mNumberOfPoles;
    ElectricalDistribution::Attributes::EndOfLife::TypeInfo::Type mEndOfLife;
    ElectricalDistribution::Attributes::ServiceEntranceRated::TypeInfo::Type mServiceEntranceRated;
};

} // namespace chip::app::Clusters
