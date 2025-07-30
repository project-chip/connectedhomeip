// TariffDataSamples.h
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <cstdint>

namespace TariffDataSamples {

using namespace chip;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;

// Helper function to create CharSpan from string literal
inline CharSpan MakeCharSpan(const char * str) {
    return CharSpan(str, strlen(str));
}

// Sample 1 Data
namespace Sample1 {

TariffInformationStruct::Type TariffInfo() {
    TariffInformationStruct::Type info;
    info.tariffLabel.SetNonNull(MakeCharSpan("Full Tariff One"));
    info.providerName.SetNonNull(MakeCharSpan("Default Provider"));
    
    CurrencyStruct::Type currency;
    currency.currency = 120;
    currency.decimalPoints = 0;
    info.currency = MakeOptional(Nullable<CurrencyStruct::Type>(currency));
    
    info.blockMode.SetNonNull(static_cast<BlockModeEnum>(0));
    return info;
}

const uint32_t dayEntryIDs1[] = {10, 11, 12, 13, 14, 15, 16};
const uint32_t dayEntryIDs2[] = {20, 21, 22, 23, 24, 25, 26};
const uint32_t dayEntryIDs3[] = {30};

DayEntryStruct::Type DayEntries[] = {
    {10, 0, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {11, 240, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {12, 480, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {13, 720, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {14, 960, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {15, 1200, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {16, 1440, MakeOptional(static_cast<uint16_t>(60)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {20, 0, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {21, 240, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {22, 480, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {23, 720, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {24, 960, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {25, 1200, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {26, 1440, MakeOptional(static_cast<uint16_t>(60)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {30, 0, MakeOptional(static_cast<uint16_t>(1500)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))}
};

DayPatternStruct::Type DayPatterns[] = {
    {0, BitMask<DayPatternDayOfWeekBitmap>(0x55), List<const uint32_t>(dayEntryIDs1)},
    {1, BitMask<DayPatternDayOfWeekBitmap>(0x2A), List<const uint32_t>(dayEntryIDs2)}
};

const uint32_t dayPatternIDs[] = {0, 1};

CalendarPeriodStruct::Type CalendarPeriods[] = {
    {Nullable<uint32_t>(0), List<const uint32_t>(dayPatternIDs)}
};

DayStruct::Type IndividualDays[] = {
    {1745427104, static_cast<DayTypeEnum>(1), List<const uint32_t>(dayEntryIDs3)}
};

TariffComponentStruct::Type TariffComponents[] = {
    {
        10,
        MakeOptional(Nullable<TariffPriceStruct::Type>({
            static_cast<TariffPriceTypeEnum>(0),
            MakeOptional(static_cast<int64_t>(15)), // 0.15 represented as fixed point
            MakeOptional(static_cast<int16_t>(1))
        })),
        MakeOptional(false),
        MakeOptional(AuxiliaryLoadSwitchSettingsStruct::Type{
            1,
            static_cast<AuxiliaryLoadSettingEnum>(0)
        }),
        MakeOptional(PeakPeriodStruct::Type{
            static_cast<PeakPeriodSeverityEnum>(1),
            1
        }),
        MakeOptional(PowerThresholdStruct::Type{
            MakeOptional(2400000),
            MakeOptional(120),
            static_cast<PowerThresholdSourceEnum>(0)
        }),
        Nullable<uint32_t>(120),
        MakeOptional(Nullable<CharSpan>(MakeCharSpan("Tariff Component 1"))),
        MakeOptional(false)
    },
    {
        20,
        MakeOptional(Nullable<TariffPriceStruct::Type>({
            static_cast<TariffPriceTypeEnum>(0),
            MakeOptional(static_cast<int64_t>(20)), // 0.20 represented as fixed point
            MakeOptional(static_cast<int16_t>(0))
        })),
        MakeOptional(false),
        MakeOptional(AuxiliaryLoadSwitchSettingsStruct::Type{
            1,
            static_cast<AuxiliaryLoadSettingEnum>(0)
        }),
        MakeOptional(PeakPeriodStruct::Type{
            static_cast<PeakPeriodSeverityEnum>(1),
            1
        }),
        MakeOptional(PowerThresholdStruct::Type{
            MakeOptional(4800000),
            MakeOptional(240),
            static_cast<PowerThresholdSourceEnum>(0)
        }),
        Nullable<uint32_t>(240),
        MakeOptional(Nullable<CharSpan>(MakeCharSpan("Tariff Component 2"))),
        MakeOptional(false)
    }
};

const uint32_t period1DayEntries[] = {10, 14, 21, 25};
const uint32_t period1Components[] = {10};
const uint32_t period2DayEntries[] = {11, 15, 22, 26};
const uint32_t period2Components[] = {20};
const uint32_t period3DayEntries[] = {12, 16, 23};
const uint32_t period3Components[] = {10};
const uint32_t period4DayEntries[] = {13, 20, 24};
const uint32_t period4Components[] = {20};
const uint32_t period5DayEntries[] = {30};
const uint32_t period5Components[] = {10};

TariffPeriodStruct::Type TariffPeriods[] = {
    {Nullable<CharSpan>(MakeCharSpan("Period 1")), List<const uint32_t>(period1DayEntries), List<const uint32_t>(period1Components)},
    {Nullable<CharSpan>(MakeCharSpan("Period 2")), List<const uint32_t>(period2DayEntries), List<const uint32_t>(period2Components)},
    {Nullable<CharSpan>(MakeCharSpan("Period 3")), List<const uint32_t>(period3DayEntries), List<const uint32_t>(period3Components)},
    {Nullable<CharSpan>(MakeCharSpan("Period 4")), List<const uint32_t>(period4DayEntries), List<const uint32_t>(period4Components)},
    {Nullable<CharSpan>(MakeCharSpan("Period 5")), List<const uint32_t>(period5DayEntries), List<const uint32_t>(period5Components)}
};

} // namespace Sample1

// Sample 2 Data
namespace Sample2 {

TariffInformationStruct::Type TariffInfo() {
    TariffInformationStruct::Type info;
    info.tariffLabel.SetNonNull(MakeCharSpan("Full Tariff Two"));
    info.providerName.SetNonNull(MakeCharSpan("Example Provider"));
    
    CurrencyStruct::Type currency;
    currency.currency = 200;
    currency.decimalPoints = 1;
    info.currency = MakeOptional(Nullable<CurrencyStruct::Type>(currency));
    
    info.blockMode.SetNonNull(static_cast<BlockModeEnum>(1));
    return info;
}

const uint32_t dayEntryIDs[] = {10, 11, 12, 13, 14, 15, 16};

DayEntryStruct::Type DayEntries[] = {
    {10, 0, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {11, 240, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {12, 480, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {13, 720, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {14, 960, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {15, 1200, MakeOptional(static_cast<uint16_t>(240)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))},
    {16, 1440, MakeOptional(static_cast<uint16_t>(60)), MakeOptional(static_cast<int16_t>(0)), MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(0))}
};

DayPatternStruct::Type DayPatterns[] = {
    {0, BitMask<DayPatternDayOfWeekBitmap>(0x7F), List<const uint32_t>(dayEntryIDs)}
};

const uint32_t dayPatternIDs[] = {0};

CalendarPeriodStruct::Type CalendarPeriods[] = {
    {Nullable<uint32_t>(0), List<const uint32_t>(dayPatternIDs)}
};

TariffComponentStruct::Type TariffComponents[] = {
    {
        10,
        MakeOptional(Nullable<TariffPriceStruct::Type>({
            static_cast<TariffPriceTypeEnum>(0),
            MakeOptional(static_cast<int64_t>(15)), // 0.15 represented as fixed point
            MakeOptional(static_cast<int16_t>(1))
        })),
        MakeOptional(false),
        MakeOptional(AuxiliaryLoadSwitchSettingsStruct::Type{
            1,
            static_cast<AuxiliaryLoadSettingEnum>(0)
        }),
        MakeOptional(PeakPeriodStruct::Type{
            static_cast<PeakPeriodSeverityEnum>(1),
            1
        }),
        MakeOptional(PowerThresholdStruct::Type{
            MakeOptional(2400000),
            MakeOptional(120),
            static_cast<PowerThresholdSourceEnum>(0)
        }),
        Nullable<uint32_t>(120),
        MakeOptional(Nullable<CharSpan>(MakeCharSpan("Tariff Component 1"))),
        MakeOptional(false)
    },
    {
        20,
        MakeOptional(Nullable<TariffPriceStruct::Type>({
            static_cast<TariffPriceTypeEnum>(0),
            MakeOptional(static_cast<int64_t>(20)), // 0.20 represented as fixed point
            MakeOptional(static_cast<int16_t>(0))
        })),
        MakeOptional(false),
        MakeOptional(AuxiliaryLoadSwitchSettingsStruct::Type{
            1,
            static_cast<AuxiliaryLoadSettingEnum>(0)
        }),
        MakeOptional(PeakPeriodStruct::Type{
            static_cast<PeakPeriodSeverityEnum>(1),
            1
        }),
        MakeOptional(PowerThresholdStruct::Type{
            MakeOptional(4800000),
            MakeOptional(240),
            static_cast<PowerThresholdSourceEnum>(0)
        }),
        Nullable<uint32_t>(240),
        MakeOptional(Nullable<CharSpan>(MakeCharSpan("Tariff Component 2"))),
        MakeOptional(false)
    },
    {
        30,
        MakeOptional(Nullable<TariffPriceStruct::Type>({
            static_cast<TariffPriceTypeEnum>(0),
            MakeOptional(static_cast<int64_t>(50)), // 0.50 represented as fixed point
            MakeOptional(static_cast<int16_t>(0))
        })),
        MakeOptional(true),
        MakeOptional(AuxiliaryLoadSwitchSettingsStruct::Type{
            1,
            static_cast<AuxiliaryLoadSettingEnum>(0)
        }),
        MakeOptional(PeakPeriodStruct::Type{
            static_cast<PeakPeriodSeverityEnum>(1),
            1
        }),
        MakeOptional(PowerThresholdStruct::Type{
            MakeOptional(3200000),
            MakeOptional(320),
            static_cast<PowerThresholdSourceEnum>(0)
        }),
        Nullable<uint32_t>(320),
        MakeOptional(Nullable<CharSpan>(MakeCharSpan("Tariff Component 3"))),
        MakeOptional(true)
    }
};

const uint32_t period1DayEntries[] = {10, 13, 16};
const uint32_t period1Components[] = {10};
const uint32_t period2DayEntries[] = {11, 14};
const uint32_t period2Components[] = {20};
const uint32_t period3DayEntries[] = {12, 15};
const uint32_t period3Components[] = {30};

TariffPeriodStruct::Type TariffPeriods[] = {
    {Nullable<CharSpan>(MakeCharSpan("Period 1")), List<const uint32_t>(period1DayEntries), List<const uint32_t>(period1Components)},
    {Nullable<CharSpan>(MakeCharSpan("Period 2")), List<const uint32_t>(period2DayEntries), List<const uint32_t>(period2Components)},
    {Nullable<CharSpan>(MakeCharSpan("Period 3")), List<const uint32_t>(period3DayEntries), List<const uint32_t>(period3Components)}
};

} // namespace Sample2

// First, define your attribute macro list
#define COMMODITY_TARIFF_ATTRIBUTES                                                         \
    X(TariffUnit,                   Nullable<TariffUnitEnum>)                      \
    X(StartDate,                    Nullable<uint32_t>)                                     \
    X(DefaultRandomizationOffset,   Nullable<int16_t>)                                      \
    X(DefaultRandomizationType,     Nullable<DayEntryRandomizationTypeEnum>)                \
    X(TariffInfo,                   Nullable<TariffInformationStruct::Type>)       \
    X(DayEntries,                   Nullable<List<DayEntryStruct::Type>>)          \
    X(DayPatterns,                  Nullable<List<DayPatternStruct::Type>>)        \
    X(TariffComponents,             Nullable<List<TariffComponentStruct::Type>>)   \
    X(TariffPeriods,                Nullable<List<TariffPeriodStruct::Type>>)      \
    X(IndividualDays,               Nullable<List<DayStruct::Type>>)               \
    X(CalendarPeriods,              Nullable<List<CalendarPeriodStruct::Type>>)

// Then define your struct using the macro
struct TariffDataSet {
#define X(attrName, attrType) attrType attrName;
    COMMODITY_TARIFF_ATTRIBUTES
#undef X
    uint32_t TariffTestTimestamp;
};

static constexpr size_t kCount = 2;

// Array of all presets
TariffDataSet kTariffPresets[] = {
    // Sample 1
    {
        .TariffInfo = MakeNullable(Sample1::TariffInfo()),
        .TariffUnit = Nullable<TariffUnitEnum>(TariffUnitEnum::kKWh),
        .StartDate =  Nullable<uint32_t>(0),
        .DefaultRandomizationOffset = Nullable<int16_t>(0),
        .DefaultRandomizationType = Nullable<DayEntryRandomizationTypeEnum>(DayEntryRandomizationTypeEnum::kNone),

        .DayEntries = MakeNullable(List<DayEntryStruct::Type>(
            Sample1::DayEntries,
            std::size(Sample1::DayEntries)
        )),
        .DayPatterns = MakeNullable(List<DayPatternStruct::Type>(
           Sample1::DayPatterns,
            std::size(TariffDataSamples::Sample1::DayPatterns))),
        .CalendarPeriods = MakeNullable(List<CalendarPeriodStruct::Type>(
           Sample1::CalendarPeriods,
            std::size(TariffDataSamples::Sample1::CalendarPeriods))),
        .IndividualDays = MakeNullable(List<DayStruct::Type>(
           Sample1::IndividualDays,
            std::size(TariffDataSamples::Sample1::IndividualDays))),
        .TariffComponents = MakeNullable(List<TariffComponentStruct::Type>(
           Sample1::TariffComponents,
            std::size(TariffDataSamples::Sample1::TariffComponents))),
        .TariffPeriods = MakeNullable(List<TariffPeriodStruct::Type>(
           Sample1::TariffPeriods,
            std::size(TariffDataSamples::Sample1::TariffPeriods))),

        .TariffTestTimestamp = 0, //01-Dec-2024
    },
    // Sample 2
    {
        .TariffInfo = MakeNullable(TariffDataSamples::Sample2::TariffInfo()),
        .TariffUnit = Nullable<TariffUnitEnum>(TariffUnitEnum::kKWh),
        .StartDate =  Nullable<uint32_t>(0),
        .DefaultRandomizationOffset = Nullable<int16_t>(0),
        .DefaultRandomizationType = Nullable<DayEntryRandomizationTypeEnum>(DayEntryRandomizationTypeEnum::kNone),

        .DayEntries = MakeNullable(List<DayEntryStruct::Type>()),
        .DayPatterns = MakeNullable(List<DayPatternStruct::Type>(
           Sample2::DayPatterns,
            std::size(TariffDataSamples::Sample2::DayPatterns))),
        .CalendarPeriods = MakeNullable(List<CalendarPeriodStruct::Type>(
           Sample2::CalendarPeriods,
            std::size(TariffDataSamples::Sample2::CalendarPeriods))),
        .IndividualDays = NullOptional, // No individual days in sample 2
        .TariffComponents = MakeNullable(List<TariffComponentStruct::Type>(
           Sample2::TariffComponents,
            std::size(TariffDataSamples::Sample2::TariffComponents))),
        .TariffPeriods = MakeNullable(List<TariffPeriodStruct::Type>(
           Sample2::TariffPeriods,
            std::size(TariffDataSamples::Sample2::TariffPeriods))),

        .TariffTestTimestamp = 0, //01-Jun-2025
    }
};

// Clean up the macro
#undef COMMODITY_TARIFF_ATTRIBUTES

} // namespace TariffDataSamples