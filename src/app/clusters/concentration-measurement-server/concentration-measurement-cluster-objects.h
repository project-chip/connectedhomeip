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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/util/util.h>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

static constexpr std::array<ClusterId, 10> AliasedClusters = { CarbonDioxideConcentrationMeasurement::Id,
                                                               CarbonMonoxideConcentrationMeasurement::Id,
                                                               NitrogenDioxideConcentrationMeasurement::Id,
                                                               Pm1ConcentrationMeasurement::Id,
                                                               Pm10ConcentrationMeasurement::Id,
                                                               Pm25ConcentrationMeasurement::Id,
                                                               RadonConcentrationMeasurement::Id,
                                                               TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                               OzoneConcentrationMeasurement::Id,
                                                               FormaldehydeConcentrationMeasurement::Id };

enum class LevelValueEnum : uint8_t
{
    kUnknown  = 0x00,
    kLow      = 0x01,
    kMedium   = 0x02,
    kHigh     = 0x03,
    kCritical = 0x04,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 5,
};

enum class MeasurementMediumEnum : uint8_t
{
    kAir   = 0x00,
    kWater = 0x01,
    kSoil  = 0x02,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
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
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 8,
};

enum class Feature : uint32_t
{
    kNumericMeasurement = 0x1,
    kLevelIndication    = 0x2,
    kMediumLevel        = 0x4,
    kCriticalLevel      = 0x8,
    kPeakMeasurement    = 0x10,
    kAverageMeasurement = 0x20,
};

namespace Attributes {

namespace MeasuredValue {
static constexpr AttributeId Id = 0x00000000;
// struct TypeInfo
// {
//     using Type             = DataModel::Nullable<float>;
//     using DecodableType    = DataModel::Nullable<float>;
//     using DecodableArgType = const DataModel::Nullable<float> &;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace MeasuredValue

namespace MinMeasuredValue {
static constexpr AttributeId Id = 0x00000001;
// struct TypeInfo
// {
//     using Type             = DataModel::Nullable<float>;
//     using DecodableType    = DataModel::Nullable<float>;
//     using DecodableArgType = const DataModel::Nullable<float> &;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace MinMeasuredValue

namespace MaxMeasuredValue {
static constexpr AttributeId Id = 0x00000002;
// struct TypeInfo
// {
//     using Type             = DataModel::Nullable<float>;
//     using DecodableType    = DataModel::Nullable<float>;
//     using DecodableArgType = const DataModel::Nullable<float> &;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace MaxMeasuredValue

namespace PeakMeasuredValue {
static constexpr AttributeId Id = 0x00000003;
// struct TypeInfo
// {
//     using Type             = DataModel::Nullable<float>;
//     using DecodableType    = DataModel::Nullable<float>;
//     using DecodableArgType = const DataModel::Nullable<float> &;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace PeakMeasuredValue

namespace PeakMeasuredValueWindow {
static constexpr AttributeId Id = 0x00000004;
// struct TypeInfo
// {
//     using Type             = uint32_t;
//     using DecodableType    = uint32_t;
//     using DecodableArgType = uint32_t;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace PeakMeasuredValueWindow

namespace AverageMeasuredValue {
static constexpr AttributeId Id = 0x00000005;
// struct TypeInfo
// {
//     using Type             = DataModel::Nullable<float>;
//     using DecodableType    = DataModel::Nullable<float>;
//     using DecodableArgType = const DataModel::Nullable<float> &;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace AverageMeasuredValue

namespace AverageMeasuredValueWindow {
static constexpr AttributeId Id = 0x00000006;
// struct TypeInfo
// {
//     using Type             = uint32_t;
//     using DecodableType    = uint32_t;
//     using DecodableArgType = uint32_t;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace AverageMeasuredValueWindow

namespace Uncertainty {
static constexpr AttributeId Id = 0x00000007;
// struct TypeInfo
// {
//     using Type             = float;
//     using DecodableType    = float;
//     using DecodableArgType = float;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace Uncertainty

namespace MeasurementUnit {
static constexpr AttributeId Id = 0x00000008;
// struct TypeInfo
// {
//     using Type             = MeasurementUnitEnum;
//     using DecodableType    = MeasurementUnitEnum;
//     using DecodableArgType = MeasurementUnitEnum;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace MeasurementUnit

namespace MeasurementMedium {
static constexpr AttributeId Id = 0x00000009;
// struct TypeInfo
// {
//     using Type             = MeasurementMediumEnum;
//     using DecodableType    = MeasurementMediumEnum;
//     using DecodableArgType = MeasurementMediumEnum;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace MeasurementMedium

namespace LevelValue {
static constexpr AttributeId Id = 0x0000000A;
// struct TypeInfo
// {
//     using Type             = LevelValueEnum;
//     using DecodableType    = LevelValueEnum;
//     using DecodableArgType = LevelValueEnum;

//     static constexpr AttributeId GetAttributeId() { return Id; }
//     static constexpr bool MustUseTimedWrite() { return false; }
// };
} // namespace LevelValue

namespace GeneratedCommandList {
static constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
static constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace EventList {
static constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList

namespace AttributeList {
static constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
static constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
static constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
