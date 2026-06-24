/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

// Provides ClusterId constants like CarbonDioxideConcentrationMeasurement::Id,
// and Globals::Attributes::FeatureMap::Id etc.
#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/AttributeIds.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Enums.h>

#include <array>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

// The 10 cluster IDs that share this attribute structure.
// Useful for iterating over all aliased clusters (e.g. in tests or bridge apps).
static constexpr std::array<ClusterId, 10> AliasedClusters = {
    CarbonDioxideConcentrationMeasurement::Id,
    CarbonMonoxideConcentrationMeasurement::Id,
    NitrogenDioxideConcentrationMeasurement::Id,
    Pm1ConcentrationMeasurement::Id,
    Pm10ConcentrationMeasurement::Id,
    Pm25ConcentrationMeasurement::Id,
    RadonConcentrationMeasurement::Id,
    TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
    OzoneConcentrationMeasurement::Id,
    FormaldehydeConcentrationMeasurement::Id,
};

// All aliased clusters share identical enums, features, and attributes.
// Alias from CarbonDioxideConcentrationMeasurement as the canonical codegen source.
using Feature               = CarbonDioxideConcentrationMeasurement::Feature;
using LevelValueEnum        = CarbonDioxideConcentrationMeasurement::LevelValueEnum;
using MeasurementMediumEnum = CarbonDioxideConcentrationMeasurement::MeasurementMediumEnum;
using MeasurementUnitEnum   = CarbonDioxideConcentrationMeasurement::MeasurementUnitEnum;

namespace Attributes = CarbonDioxideConcentrationMeasurement::Attributes;

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
