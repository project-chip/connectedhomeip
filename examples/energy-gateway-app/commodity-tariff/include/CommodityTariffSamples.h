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

#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {
namespace TariffDataSamples {

// Sample 1 Data
namespace Sample1 {

// Tariff Information
static inline Structs::TariffInformationStruct::Type TariffInfo()
{
    return { .tariffLabel  = DataModel::MakeNullable(CharSpan::fromCharString("Full Tariff One")),
             .providerName = DataModel::MakeNullable(CharSpan::fromCharString("Default Provider")),
             .currency     = MakeOptional(
                 DataModel::MakeNullable<Globals::Structs::CurrencyStruct::Type>({ .currency = 120, .decimalPoints = 0 })),
             .blockMode = DataModel::MakeNullable(static_cast<BlockModeEnum>(0)) };
}

static const uint32_t dayEntryIDs1[] = { 10, 11, 12, 13, 14, 15, 16 };
static const uint32_t dayEntryIDs2[] = { 20, 21, 22, 23, 24, 25, 26 };
static const uint32_t dayEntryIDs3[] = { 30 };

// Day Entries
static inline Structs::DayEntryStruct::Type DayEntries[] = {
    { .dayEntryID          = 10,
      .startTime           = 0,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 11,
      .startTime           = 240,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 12,
      .startTime           = 480,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 13,
      .startTime           = 720,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 14,
      .startTime           = 960,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 15,
      .startTime           = 1200,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 16,
      .startTime           = 1440,
      .duration            = MakeOptional(static_cast<uint16_t>(60)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 20,
      .startTime           = 0,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 21,
      .startTime           = 240,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 22,
      .startTime           = 480,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 23,
      .startTime           = 720,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 24,
      .startTime           = 960,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 25,
      .startTime           = 1200,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 26,
      .startTime           = 1440,
      .duration            = MakeOptional(static_cast<uint16_t>(60)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 30,
      .startTime           = 0,
      .duration            = MakeOptional(static_cast<uint16_t>(1500)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) }
};

// Day Patterns
static inline Structs::DayPatternStruct::Type DayPatterns[] = {
    { .dayPatternID = 0,
      .daysOfWeek   = BitMask<DayPatternDayOfWeekBitmap>(0x55),
      .dayEntryIDs  = Span<const uint32_t>(dayEntryIDs1) },
    { .dayPatternID = 1, .daysOfWeek = BitMask<DayPatternDayOfWeekBitmap>(0x2A), .dayEntryIDs = Span<const uint32_t>(dayEntryIDs2) }
};

static const uint32_t dayPatternIDs[] = { 0, 1 };

// Calendar Periods
static inline Structs::CalendarPeriodStruct::Type CalendarPeriods[] = { { .startDate     = DataModel::Nullable<uint32_t>(0),
                                                                          .dayPatternIDs = Span<const uint32_t>(dayPatternIDs) } };

static inline Structs::DayStruct::Type IndividualDays[] = {
    { .date = 1745427104, .dayType = static_cast<DayTypeEnum>(1), .dayEntryIDs = Span<const uint32_t>(dayEntryIDs3) }
};

static inline Structs::TariffComponentStruct::Type TariffComponents[] = {
    { .tariffComponentID = 10,
      .price             = MakeOptional(
          DataModel::Nullable<Structs::TariffPriceStruct::Type>({ .priceType  = static_cast<Globals::TariffPriceTypeEnum>(0),
                                                                              .price      = MakeOptional(static_cast<int64_t>(15)),
                                                                              .priceLevel = MakeOptional(static_cast<int16_t>(1)) })),
      .friendlyCredit = MakeOptional(false),
      .auxiliaryLoad  = MakeOptional(Structs::AuxiliaryLoadSwitchSettingsStruct::Type{
           .number = 1, .requiredState = static_cast<AuxiliaryLoadSettingEnum>(0) }),
      .peakPeriod =
          MakeOptional(Structs::PeakPeriodStruct::Type{ .severity = static_cast<PeakPeriodSeverityEnum>(1), .peakPeriod = 1 }),
      .powerThreshold = MakeOptional(Globals::Structs::PowerThresholdStruct::Type{
          .powerThreshold         = MakeOptional(2400000),
          .apparentPowerThreshold = MakeOptional(120),
          .powerThresholdSource   = static_cast<Globals::PowerThresholdSourceEnum>(0) }),
      .threshold      = NullOptional,
      .label          = MakeOptional(DataModel::MakeNullable(CharSpan::fromCharString("Tariff Component 1"))),
      .predicted      = MakeOptional(false) },
    { .tariffComponentID = 20,
      .price             = MakeOptional(
          DataModel::Nullable<Structs::TariffPriceStruct::Type>({ .priceType  = static_cast<Globals::TariffPriceTypeEnum>(0),
                                                                              .price      = MakeOptional(static_cast<int64_t>(20)),
                                                                              .priceLevel = MakeOptional(static_cast<int16_t>(0)) })),
      .friendlyCredit = MakeOptional(false),
      .auxiliaryLoad  = MakeOptional(Structs::AuxiliaryLoadSwitchSettingsStruct::Type{
           .number = 1, .requiredState = static_cast<AuxiliaryLoadSettingEnum>(0) }),
      .peakPeriod =
          MakeOptional(Structs::PeakPeriodStruct::Type{ .severity = static_cast<PeakPeriodSeverityEnum>(1), .peakPeriod = 1 }),
      .powerThreshold = MakeOptional(Globals::Structs::PowerThresholdStruct::Type{
          .powerThreshold         = MakeOptional(4800000),
          .apparentPowerThreshold = MakeOptional(240),
          .powerThresholdSource   = static_cast<Globals::PowerThresholdSourceEnum>(0) }),
      .threshold      = NullOptional,
      .label          = MakeOptional(DataModel::MakeNullable(CharSpan::fromCharString("Tariff Component 2"))),
      .predicted      = MakeOptional(false) }
};

static const uint32_t period1DayEntries[] = { 10, 14, 21, 25 };
static const uint32_t period1Components[] = { 10 };
static const uint32_t period2DayEntries[] = { 11, 15, 22, 26 };
static const uint32_t period2Components[] = { 20 };
static const uint32_t period3DayEntries[] = { 12, 16, 23 };
static const uint32_t period3Components[] = { 10 };
static const uint32_t period4DayEntries[] = { 13, 20, 24 };
static const uint32_t period4Components[] = { 20 };
static const uint32_t period5DayEntries[] = { 30 };
static const uint32_t period5Components[] = { 10 };

static inline Structs::TariffPeriodStruct::Type TariffPeriods[] = {
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 1")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period1DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period1Components) },
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 2")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period2DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period2Components) },
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 3")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period3DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period3Components) },
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 4")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period4DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period4Components) },
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 5")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period5DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period5Components) }
};

} // namespace Sample1

// Sample 2 Data
namespace Sample2 {

// Tariff Information
static inline Structs::TariffInformationStruct::Type TariffInfo()
{
    return { .tariffLabel  = DataModel::MakeNullable(CharSpan::fromCharString("Full Tariff Two")),
             .providerName = DataModel::MakeNullable(CharSpan::fromCharString("Example Provider")),
             .currency =
                 MakeOptional(DataModel::Nullable<Globals::Structs::CurrencyStruct::Type>({ .currency = 200, .decimalPoints = 1 })),
             .blockMode = DataModel::MakeNullable(static_cast<BlockModeEnum>(1)) };
}

static const uint32_t dayEntryIDs[] = { 10, 11, 12, 13, 14, 15, 16 };

// Day Entries
static inline Structs::DayEntryStruct::Type DayEntries[] = {
    { .dayEntryID          = 10,
      .startTime           = 0,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 11,
      .startTime           = 240,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 12,
      .startTime           = 480,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 13,
      .startTime           = 720,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 14,
      .startTime           = 960,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 15,
      .startTime           = 1200,
      .duration            = MakeOptional(static_cast<uint16_t>(240)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) },
    { .dayEntryID          = 16,
      .startTime           = 1440,
      .duration            = MakeOptional(static_cast<uint16_t>(60)),
      .randomizationOffset = MakeOptional(static_cast<int16_t>(0)),
      .randomizationType   = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0)) }
};

// Day Patterns
static inline Structs::DayPatternStruct::Type DayPatterns[] = { { .dayPatternID = 0,
                                                                  .daysOfWeek   = BitMask<DayPatternDayOfWeekBitmap>(0x7F),
                                                                  .dayEntryIDs  = DataModel::List<const uint32_t>(dayEntryIDs) } };

static const uint32_t dayPatternIDs[] = { 0 };

// Calendar Periods
static inline Structs::CalendarPeriodStruct::Type CalendarPeriods[] = {
    { .startDate = DataModel::Nullable<uint32_t>(0), .dayPatternIDs = DataModel::List<const uint32_t>(dayPatternIDs) }
};

// Tariff Components
static inline Structs::TariffComponentStruct::Type TariffComponents[] = {
    { .tariffComponentID = 10,
      .price             = MakeOptional(
          DataModel::Nullable<Structs::TariffPriceStruct::Type>({ .priceType  = static_cast<Globals::TariffPriceTypeEnum>(0),
                                                                              .price      = MakeOptional(static_cast<int64_t>(15)),
                                                                              .priceLevel = MakeOptional(static_cast<int16_t>(1)) })),
      .friendlyCredit = MakeOptional(false),
      .auxiliaryLoad  = MakeOptional(Structs::AuxiliaryLoadSwitchSettingsStruct::Type{
           .number = 1, .requiredState = static_cast<AuxiliaryLoadSettingEnum>(0) }),
      .peakPeriod =
          MakeOptional(Structs::PeakPeriodStruct::Type{ .severity = static_cast<PeakPeriodSeverityEnum>(1), .peakPeriod = 1 }),
      .powerThreshold = MakeOptional(Globals::Structs::PowerThresholdStruct::Type{
          .powerThreshold         = MakeOptional(2400000),
          .apparentPowerThreshold = MakeOptional(120),
          .powerThresholdSource   = static_cast<Globals::PowerThresholdSourceEnum>(0) }),
      .threshold      = DataModel::Nullable<int64_t>(120),
      .label          = MakeOptional(DataModel::MakeNullable(CharSpan::fromCharString("Tariff Component 1"))),
      .predicted      = MakeOptional(false) },
    { .tariffComponentID = 20,
      .price             = MakeOptional(
          DataModel::Nullable<Structs::TariffPriceStruct::Type>({ .priceType  = static_cast<Globals::TariffPriceTypeEnum>(0),
                                                                              .price      = MakeOptional(static_cast<int64_t>(20)),
                                                                              .priceLevel = MakeOptional(static_cast<int16_t>(0)) })),
      .friendlyCredit = MakeOptional(false),
      .auxiliaryLoad  = MakeOptional(Structs::AuxiliaryLoadSwitchSettingsStruct::Type{
           .number = 1, .requiredState = static_cast<AuxiliaryLoadSettingEnum>(0) }),
      .peakPeriod =
          MakeOptional(Structs::PeakPeriodStruct::Type{ .severity = static_cast<PeakPeriodSeverityEnum>(1), .peakPeriod = 1 }),
      .powerThreshold = MakeOptional(Globals::Structs::PowerThresholdStruct::Type{
          .powerThreshold         = MakeOptional(4800000),
          .apparentPowerThreshold = MakeOptional(240),
          .powerThresholdSource   = static_cast<Globals::PowerThresholdSourceEnum>(0) }),
      .threshold      = DataModel::Nullable<int64_t>(240),
      .label          = MakeOptional(DataModel::MakeNullable(CharSpan::fromCharString("Tariff Component 2"))),
      .predicted      = MakeOptional(false) },
    { .tariffComponentID = 30,
      .price             = MakeOptional(
          DataModel::Nullable<Structs::TariffPriceStruct::Type>({ .priceType  = static_cast<Globals::TariffPriceTypeEnum>(0),
                                                                              .price      = MakeOptional(static_cast<int64_t>(50)),
                                                                              .priceLevel = MakeOptional(static_cast<int16_t>(0)) })),
      .friendlyCredit = MakeOptional(true),
      .auxiliaryLoad  = MakeOptional(Structs::AuxiliaryLoadSwitchSettingsStruct::Type{
           .number = 1, .requiredState = static_cast<AuxiliaryLoadSettingEnum>(0) }),
      .peakPeriod =
          MakeOptional(Structs::PeakPeriodStruct::Type{ .severity = static_cast<PeakPeriodSeverityEnum>(1), .peakPeriod = 1 }),
      .powerThreshold = MakeOptional(Globals::Structs::PowerThresholdStruct::Type{
          .powerThreshold         = MakeOptional(3200000),
          .apparentPowerThreshold = MakeOptional(320),
          .powerThresholdSource   = static_cast<Globals::PowerThresholdSourceEnum>(0) }),
      .threshold      = DataModel::Nullable<int64_t>(320),
      .label          = MakeOptional(DataModel::MakeNullable(CharSpan::fromCharString("Tariff Component 3"))),
      .predicted      = MakeOptional(true) }
};

static const uint32_t period1DayEntries[] = { 10, 13, 16 };
static const uint32_t period1Components[] = { 10 };
static const uint32_t period2DayEntries[] = { 11, 14 };
static const uint32_t period2Components[] = { 20 };
static const uint32_t period3DayEntries[] = { 12, 15 };
static const uint32_t period3Components[] = { 30 };

// Tariff Periods
static inline Structs::TariffPeriodStruct::Type TariffPeriods[] = {
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 1")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period1DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period1Components) },
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 2")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period2DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period2Components) },
    { .label              = DataModel::MakeNullable(CharSpan::fromCharString("Period 3")),
      .dayEntryIDs        = DataModel::List<const uint32_t>(period3DayEntries),
      .tariffComponentIDs = DataModel::List<const uint32_t>(period3Components) }
};

} // namespace Sample2

struct TariffDataSet
{
    DataModel::Nullable<Globals::TariffUnitEnum> TariffUnit;
    DataModel::Nullable<uint32_t> StartDate;
    DataModel::Nullable<int16_t> DefaultRandomizationOffset;
    DataModel::Nullable<DayEntryRandomizationTypeEnum> DefaultRandomizationType;
    DataModel::Nullable<Structs::TariffInformationStruct ::Type> TariffInfo;
    DataModel::Nullable<DataModel::List<Structs::DayEntryStruct ::Type>> DayEntries;
    DataModel::Nullable<DataModel::List<Structs::DayPatternStruct ::Type>> DayPatterns;
    DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct ::Type>> TariffComponents;
    DataModel::Nullable<DataModel::List<Structs::TariffPeriodStruct ::Type>> TariffPeriods;
    DataModel::Nullable<DataModel::List<Structs::DayStruct ::Type>> IndividualDays;
    DataModel::Nullable<DataModel::List<Structs::CalendarPeriodStruct ::Type>> CalendarPeriods;
    uint32_t TariffTestTimestamp;
};

static constexpr size_t kCount = 2;

// Array of all presets
const TariffDataSet kTariffPresets[] = {
    // Sample 1
    {
        .TariffUnit                 = DataModel::Nullable<Globals::TariffUnitEnum>(Globals::TariffUnitEnum::kKWh),
        .StartDate                  = DataModel::Nullable<uint32_t>(0),
        .DefaultRandomizationOffset = DataModel::Nullable<int16_t>(0),
        .DefaultRandomizationType   = DataModel::Nullable<DayEntryRandomizationTypeEnum>(DayEntryRandomizationTypeEnum::kNone),
        .TariffInfo                 = DataModel::MakeNullable(Sample1::TariffInfo()),
        .DayEntries                 = DataModel::MakeNullable(
            DataModel::List<Structs::DayEntryStruct::Type>(Sample1::DayEntries, std::size(Sample1::DayEntries))),
        .DayPatterns = DataModel::MakeNullable(
            DataModel::List<Structs::DayPatternStruct::Type>(Sample1::DayPatterns, std::size(Sample1::DayPatterns))),
        .TariffComponents = DataModel::MakeNullable(
            DataModel::List<Structs::TariffComponentStruct::Type>(Sample1::TariffComponents, std::size(Sample1::TariffComponents))),
        .TariffPeriods = DataModel::MakeNullable(
            DataModel::List<Structs::TariffPeriodStruct::Type>(Sample1::TariffPeriods, std::size(Sample1::TariffPeriods))),
        .IndividualDays = DataModel::MakeNullable(
            DataModel::List<Structs::DayStruct::Type>(Sample1::IndividualDays, std::size(Sample1::IndividualDays))),
        .CalendarPeriods = DataModel::MakeNullable(
            DataModel::List<Structs::CalendarPeriodStruct::Type>(Sample1::CalendarPeriods, std::size(Sample1::CalendarPeriods))),
        .TariffTestTimestamp = 1733011200, // 01-Dec-2024
    },
    // Sample 2
    {
        .TariffUnit                 = DataModel::Nullable<Globals::TariffUnitEnum>(Globals::TariffUnitEnum::kKWh),
        .StartDate                  = DataModel::Nullable<uint32_t>(0),
        .DefaultRandomizationOffset = DataModel::Nullable<int16_t>(0),
        .DefaultRandomizationType   = DataModel::Nullable<DayEntryRandomizationTypeEnum>(DayEntryRandomizationTypeEnum::kNone),
        .TariffInfo                 = DataModel::MakeNullable(Sample2::TariffInfo()),
        .DayEntries                 = DataModel::MakeNullable(
            DataModel::List<Structs::DayEntryStruct::Type>(Sample2::DayEntries, std::size(Sample2::DayEntries))),
        .DayPatterns = DataModel::MakeNullable(
            DataModel::List<Structs::DayPatternStruct::Type>(Sample2::DayPatterns, std::size(Sample2::DayPatterns))),
        .TariffComponents = DataModel::MakeNullable(
            DataModel::List<Structs::TariffComponentStruct::Type>(Sample2::TariffComponents, std::size(Sample2::TariffComponents))),
        .TariffPeriods = DataModel::MakeNullable(
            DataModel::List<Structs::TariffPeriodStruct::Type>(Sample2::TariffPeriods, std::size(Sample2::TariffPeriods))),
        .IndividualDays  = NullOptional, // No individual days in sample 2
        .CalendarPeriods = DataModel::MakeNullable(
            DataModel::List<Structs::CalendarPeriodStruct::Type>(Sample2::CalendarPeriods, std::size(Sample2::CalendarPeriods))),
        .TariffTestTimestamp = 1748736000, // 01-Jun-2025
    }
};

} // namespace TariffDataSamples
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
