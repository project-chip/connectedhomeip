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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/data-model/List.h>
#include <cstdint>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitLocalization {

inline constexpr uint16_t kClusterRevision              = 2;
inline constexpr uint8_t kMinSupportedLocalizationUnits = 2;
inline constexpr uint8_t kMaxSupportedLocalizationUnits = 3;

class UnitLocalizationServer : public AttributeAccessInterface
{
public:
    // Register for the UnitLocalization cluster on all endpoints.
    CHIP_ERROR Init();
    static UnitLocalizationServer & Instance();

    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    CHIP_ERROR SetSupportedTemperatureUnits(DataModel::List<TempUnitEnum> & units);
    const DataModel::List<TempUnitEnum> & GetSupportedTemperatureUnits(void) { return mSupportedTemperatureUnits; }
    TempUnitEnum GetTemperatureUnit(void) { return mTemperatureUnit; }
    CHIP_ERROR SetTemperatureUnit(TempUnitEnum unit);

private:
    UnitLocalizationServer() : AttributeAccessInterface(Optional<EndpointId>::Missing(), UnitLocalization::Id) {}

    static UnitLocalizationServer mInstance;
    DataModel::List<TempUnitEnum> mSupportedTemperatureUnits{ DataModel::List<TempUnitEnum>(mUnitsBuffer) };
    TempUnitEnum mUnitsBuffer[kMaxSupportedLocalizationUnits] = { TempUnitEnum::kFahrenheit, TempUnitEnum::kCelsius,
                                                                  TempUnitEnum::kKelvin };
    TempUnitEnum mTemperatureUnit                             = TempUnitEnum::kCelsius;
};

} // namespace UnitLocalization
} // namespace Clusters
} // namespace app
} // namespace chip
