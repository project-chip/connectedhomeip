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

// NOTE: <app/CommandHandlerInterface.h> and <app/util/util.h> have been
// removed. They were ember/AAI dependencies. This cluster has no commands
// and does not use AttributeAccessInterface.

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

enum class LevelValueEnum : uint8_t
{
    kUnknown  = 0x00,
    kLow      = 0x01,
    kMedium   = 0x02,
    kHigh     = 0x03,
    kCritical = 0x04,
    // All received enum values not listed above map to kUnknownEnumValue.
    // Never transmit this value.
    kUnknownEnumValue = 5,
};

enum class MeasurementMediumEnum : uint8_t
{
    kAir   = 0x00,
    kWater = 0x01,
    kSoil  = 0x02,
    // Never transmit this value.
    kUnknownEnumValue = 3,
};

enum class MeasurementUnitEnum : uint8_t
{
    kPpm  = 0x00,
    kPpb  = 0x01,
    kPpt  = 0x02,
    kMgm3 = 0x03,
    kUgm3 = 0x04,
    kNgm3 = 0x05,
    kPm3  = 0x06,
    kBqm3 = 0x07,
    // Never transmit this value.
    kUnknownEnumValue = 8,
};

// Feature bitmap. Pass as BitFlags<Feature> to ConcentrationMeasurementCluster.
// kMediumLevel and kCriticalLevel are sub-features of kLevelIndication.
// kPeakMeasurement and kAverageMeasurement require kNumericMeasurement.
enum class Feature : uint32_t
{
    kNumericMeasurement = 0x01,
    kLevelIndication    = 0x02,
    kMediumLevel        = 0x04,
    kCriticalLevel      = 0x08,
    kPeakMeasurement    = 0x10,
    kAverageMeasurement = 0x20,
};

// ── Attribute ID constants ────────────────────────────────────────────────────
// These are the attribute IDs used in ReadAttribute() switch statements and
// in NotifyChanged() calls from delegates. No TypeInfo structs are needed —
// the cluster encodes primitive types directly (float, uint32_t, enum).

namespace Attributes {

namespace MeasuredValue            { static constexpr AttributeId Id = 0x00000000; }
namespace MinMeasuredValue         { static constexpr AttributeId Id = 0x00000001; }
namespace MaxMeasuredValue         { static constexpr AttributeId Id = 0x00000002; }
namespace PeakMeasuredValue        { static constexpr AttributeId Id = 0x00000003; }
namespace PeakMeasuredValueWindow  { static constexpr AttributeId Id = 0x00000004; }
namespace AverageMeasuredValue     { static constexpr AttributeId Id = 0x00000005; }
namespace AverageMeasuredValueWindow { static constexpr AttributeId Id = 0x00000006; }
namespace Uncertainty              { static constexpr AttributeId Id = 0x00000007; }
namespace MeasurementUnit          { static constexpr AttributeId Id = 0x00000008; }
namespace MeasurementMedium        { static constexpr AttributeId Id = 0x00000009; }
namespace LevelValue               { static constexpr AttributeId Id = 0x0000000A; }

// Global attributes — IDs are cluster-independent
namespace GeneratedCommandList { static constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id; }
namespace AcceptedCommandList  { static constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;  }
namespace AttributeList        { static constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;        }
namespace FeatureMap           { static constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;           }
namespace ClusterRevision      { static constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;      }

} // namespace Attributes
} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
