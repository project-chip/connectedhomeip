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

#include <app/persistence/AttributePersistenceMigration.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/UnitLocalization/ClusterId.h>
#include <clusters/UnitLocalization/Enums.h>

namespace chip {
namespace app {
namespace Clusters {

namespace UnitLocalization {
inline constexpr uint16_t kClusterRevision              = 2;
inline constexpr uint8_t kMinSupportedLocalizationUnits = 2;
inline constexpr uint8_t kMaxSupportedLocalizationUnits = 3;
} // namespace UnitLocalization

class UnitLocalizationCluster : public DefaultServerCluster
{
public:
    UnitLocalizationCluster(EndpointId endpointId, BitFlags<UnitLocalization::Feature> feature);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetSupportedTemperatureUnits(DataModel::List<UnitLocalization::TempUnitEnum> & units);

private:
    BitFlags<UnitLocalization::Feature> mFeatures   = {};
    UnitLocalization::TempUnitEnum mTemperatureUnit = UnitLocalization::TempUnitEnum::kCelsius;
    UnitLocalization::TempUnitEnum mUnitsBuffer[UnitLocalization::kMaxSupportedLocalizationUnits] = {
        UnitLocalization::TempUnitEnum::kFahrenheit, UnitLocalization::TempUnitEnum::kCelsius,
        UnitLocalization::TempUnitEnum::kKelvin
    };
    DataModel::List<UnitLocalization::TempUnitEnum> mSupportedTemperatureUnits{ mUnitsBuffer };
    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);
};

class UnitLocalizationClusterWithMigration : public UnitLocalizationCluster
{
public:
    UnitLocalizationClusterWithMigration(EndpointId endpointId, BitFlags<UnitLocalization::Feature> feature);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
};

} // namespace Clusters
} // namespace app
} // namespace chip
