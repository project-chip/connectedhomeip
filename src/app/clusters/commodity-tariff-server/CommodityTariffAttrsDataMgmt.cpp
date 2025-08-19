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

#include "CommodityTariffAttrsDataMgmt.h"

using namespace chip;
using namespace chip::app;
using namespace chip::Platform;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;
using namespace chip::app::CommodityTariffAttrsDataMgmt;
using namespace CommodityTariffConsts;

#define VerifyOrReturnError_LogSend(expr, code, ...)                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            ChipLogError(NotSpecified, __VA_ARGS__);                                                                               \
            VerifyOrReturnError(expr, code);                                                                                       \
        }                                                                                                                          \
    } while (false)

namespace CommonUtilities {
static bool HasFeatureInCtx(TariffUpdateCtx * aCtx, Feature aFeature)
{
    return aCtx->mFeature.Has(aFeature);
}

/**
 * @brief Releases memory allocated for a list of IDs and resets the list
 * @param IDs List containing allocated IDs buffer to free. After this call,
 *            the list will be empty and the buffer pointer nulled.
 *
 * @note The const_cast is safe because:
 *       1. We own the memory allocation (allocated via MemoryAlloc in non-const context)
 *       2. MemoryFree doesn't modify the memory, just deallocates it
 *       3. The const-ness was only added for interface safety
 *       4. This matches the symmetric Alloc/Free pattern we established
 */
static void CleanUpIDs(DataModel::List<const uint32_t> & IDs)
{
    if (!IDs.empty() && IDs.data())
    {
        // Safe const_cast because:
        // - We allocated this memory ourselves via non-const allocation
        // - MemoryFree won't actually modify the contents
        // - The original allocation wasn't truly const (just interface const)
        MemoryFree(const_cast<uint32_t *>(IDs.data()));
        IDs = DataModel::List<const uint32_t>();
    }
}

static bool HasDuplicateIDs(const DataModel::List<const uint32_t> & IDs, std::unordered_set<uint32_t> & seen)
{
    for (auto id : IDs)
    {
        if (!seen.insert(id).second)
        {
            return true; // Duplicate found
        }
    }
    return false;
}
}; // namespace CommonUtilities

/*
CHIP_ERROR StartDateDataClass::Validate(const ValueType & aValue) const
{
    if (!aValue.IsNull() && aValue.Value() != 0)
    {
        VerifyOrReturnError((static_cast<TariffUpdateCtx *>(aCtx)->TariffUpdateTimestamp <= aValue.Value()),
            CHIP_ERROR_INVALID_ARGUMENT);
    }

    return CHIP_NO_ERROR;
}*/

namespace DayEntriesDataClass_Utils {

static CHIP_ERROR ValidateListEntry(const DayEntryStruct::Type & entryNewValue, TariffUpdateCtx * aCtx)
{
    VerifyOrReturnError_LogSend(entryNewValue.startTime < kDayEntryDurationLimit, CHIP_ERROR_INVALID_ARGUMENT,
                                "DayEntry startTime must be less than %u", kDayEntryDurationLimit);

    if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kRandomization))
    {
        if (entryNewValue.randomizationOffset.HasValue() && entryNewValue.randomizationType.HasValue())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(entryNewValue.randomizationType.Value()) !=
                                    DayEntryRandomizationTypeEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogError(NotSpecified, "If the RNDM feature is enabled, the randomization* field is required!");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return CHIP_NO_ERROR;
}
} // namespace DayEntriesDataClass_Utils

// DayPatternsDataClass
namespace DayPatternsDataClass_Utils {
static CHIP_ERROR ValidateListEntry(const DayPatternStruct::Type & entryNewValue, std::unordered_set<uint32_t> & seenDeIDs)
{
    if (entryNewValue.dayEntryIDs.empty() || entryNewValue.dayEntryIDs.size() > kDayPatternItemMaxDayEntryIDs)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Check that the current day pattern item has no duplicated dayEntryIDs
    if (CommonUtilities::HasDuplicateIDs(entryNewValue.dayEntryIDs, seenDeIDs))
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    return CHIP_NO_ERROR;
}
} // namespace DayPatternsDataClass_Utils

// TariffPeriodsDataClass
namespace TariffPeriodsDataClass_Utils {
static CHIP_ERROR ValidateListEntry(const TariffPeriodStruct::Type & entryNewValue, std::unordered_set<uint32_t> & seenDeIDs,
                                    std::unordered_set<uint32_t> & seenTcIDs,
                                    std::map<uint32_t, std::unordered_set<uint32_t>> & componentMap)
{
    if (!entryNewValue.label.IsNull())
    {
        const auto & labelSpan = entryNewValue.label.Value();
        if (labelSpan.size() > kDefaultStringValuesMaxBufLength)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (labelSpan.empty())
        {
            ChipLogError(NotSpecified, "TariffPeriod label must not be empty if present");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (entryNewValue.dayEntryIDs.empty() || entryNewValue.dayEntryIDs.size() > kTariffPeriodItemMaxIDs)
        return CHIP_ERROR_INVALID_ARGUMENT;

    if (entryNewValue.tariffComponentIDs.empty() || entryNewValue.tariffComponentIDs.size() > kTariffPeriodItemMaxIDs)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Check that the current period item has no duplicated dayEntryIDs
    if (CommonUtilities::HasDuplicateIDs(entryNewValue.dayEntryIDs, seenDeIDs))
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    for (const auto & tcId : entryNewValue.tariffComponentIDs)
    {
        for (const auto & deId : entryNewValue.dayEntryIDs)
        {
            if (!componentMap[tcId].insert(deId).second)
            {
                ChipLogError(NotSpecified, "Current dayEntryID already attached to the tariffComponentID");
                return CHIP_ERROR_DUPLICATE_KEY_ID;
            }
        }

        // Here we save the tariff component IDs in the context for the next cross-checks
        seenTcIDs.insert(tcId);
    }

    return CHIP_NO_ERROR;
}
} // namespace TariffPeriodsDataClass_Utils

// TariffComponentsDataClass
namespace TariffComponentsDataClass_Utils {
static CHIP_ERROR ValidateListEntry(const TariffComponentStruct::Type & entryNewValue, TariffUpdateCtx * aCtx)
{

    VerifyOrReturnError(entryNewValue.tariffComponentID > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // entryNewValue.label
    if (entryNewValue.label.HasValue() && !entryNewValue.label.Value().IsNull())
    {
        VerifyOrReturnError(entryNewValue.label.Value().Value().size() <= kTariffComponentMaxLabelLength,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Handle price field validation based on feature support
    if (entryNewValue.price.HasValue() && !entryNewValue.price.Value().IsNull())
    {
        // If price is provided, the Pricing feature MUST be supported
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPricing),
                            CHIP_ERROR_INVALID_ARGUMENT);

        const auto & price = entryNewValue.price.Value().Value();
        VerifyOrReturnError(EnsureKnownEnumValue(price.priceType) != TariffPriceTypeEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }
    else if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPricing))
    {
        // If Pricing feature is enabled but no price provided, that's only acceptable
        // if the field is explicitly set to null (which we already checked above)
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Else: No price provided and feature not enabled - valid case

    // Validate friendlyCredit field
    if (entryNewValue.friendlyCredit.HasValue())
    {
        // If friendlyCredit is provided, the FCRED feature MUST be supported
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kFriendlyCredit),
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Validate auxiliaryLoad field
    if (entryNewValue.auxiliaryLoad.HasValue())
    {
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kAuxiliaryLoad),
                            CHIP_ERROR_INVALID_ARGUMENT);

        const auto & auxiliaryLoad = entryNewValue.auxiliaryLoad.Value();
        VerifyOrReturnError(EnsureKnownEnumValue(auxiliaryLoad.requiredState) != AuxiliaryLoadSettingEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }
    else if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kAuxiliaryLoad))
    {
        // AUXLD feature enabled but no auxiliaryLoad provided (or explicitly null)
        ChipLogError(NotSpecified, "The auxiliaryLoad field must be provided and non-null when AUXLD feature is enabled");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Validate peakPeriod field
    if (entryNewValue.peakPeriod.HasValue())
    {
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPeakPeriod),
                            CHIP_ERROR_INVALID_ARGUMENT);

        const auto & peakPeriod = entryNewValue.peakPeriod.Value();
        VerifyOrReturnError(EnsureKnownEnumValue(peakPeriod.severity) != PeakPeriodSeverityEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(peakPeriod.peakPeriod > 0, CHIP_ERROR_INVALID_ARGUMENT);
    }
    else if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPeakPeriod))
    {
        // PEAKP feature enabled but no peakPeriod provided (or explicitly null)
        ChipLogError(NotSpecified, "The peakPeriod field must be provided and non-null when PEAKP feature is enabled");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Validate powerThreshold field
    if (entryNewValue.powerThreshold.HasValue())
    {
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPowerThreshold),
                            CHIP_ERROR_INVALID_ARGUMENT);

        const auto & powerThreshold = entryNewValue.powerThreshold.Value();
        if (!powerThreshold.powerThresholdSource.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(powerThreshold.powerThresholdSource.Value()) !=
                                    PowerThresholdSourceEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // Additional power threshold validations
        if (powerThreshold.powerThreshold.HasValue())
        {
            VerifyOrReturnError(powerThreshold.powerThreshold.Value() > 0, CHIP_ERROR_INVALID_ARGUMENT);
        }
        if (powerThreshold.apparentPowerThreshold.HasValue())
        {
            VerifyOrReturnError(powerThreshold.apparentPowerThreshold.Value() > 0, CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    if (entryNewValue.predicted.HasValue())
    {
        // No need to check the value itself since it's just a bool
        // But we can add debug logging if needed:
        ChipLogDetail(NotSpecified, "Predicted flag set to %s", entryNewValue.predicted.Value() ? "true" : "false");
    }

    return CHIP_NO_ERROR;
}
} // namespace TariffComponentsDataClass_Utils

namespace chip {
namespace app {
namespace CommodityTariffAttrsDataMgmt {

using namespace CommodityTariffConsts;
using namespace chip::app::Clusters::CommodityTariff::Structs;

template <>
CHIP_ERROR Validate<DataModel::Nullable<TariffUnitEnum>>(const DataModel::Nullable<TariffUnitEnum> & aValue, void * aCtx)
{
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<uint32_t>>(const DataModel::Nullable<uint32_t> & aValue, void * aCtx)
{
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<int16_t>>(const DataModel::Nullable<int16_t> & aValue, void * aCtx)
{
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR
Validate<DataModel::Nullable<DayEntryRandomizationTypeEnum>>(const DataModel::Nullable<DayEntryRandomizationTypeEnum> & aValue,
                                                             void * aCtx)
{
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR
Validate<DataModel::Nullable<TariffInformationStruct::Type>>(const DataModel::Nullable<TariffInformationStruct::Type> & aValue,
                                                             void * aCtx)
{
    // The tariff info is always required (cannot be null)
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    const TariffInformationStruct::Type & newValue = aValue.Value();
    auto * ctx                                     = static_cast<TariffUpdateCtx *>(aCtx);

    // Validate string lengths (if fields are provided)
    VerifyOrReturnError(newValue.tariffLabel.IsNull() || newValue.tariffLabel.Value().size() <= kTariffInfoMaxLabelLength,
                        CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(newValue.providerName.IsNull() || newValue.providerName.Value().size() <= kTariffInfoMaxProviderLength,
                        CHIP_ERROR_INVALID_ARGUMENT);

    // Validate enum values
    if (!newValue.blockMode.IsNull())
    {
        VerifyOrReturnError(EnsureKnownEnumValue(newValue.blockMode.Value()) != BlockModeEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Handle currency validation based on pricing feature
    const bool pricingEnabled = CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kPricing);

    if (pricingEnabled)
    {
        // Pricing enabled - currency must be provided (null or non-null)
        VerifyOrReturnError(newValue.currency.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

        // If non-null, validate contents
        if (!newValue.currency.Value().IsNull())
        {
            const auto & currency = newValue.currency.Value().Value();
            VerifyOrReturnError(currency.currency < kMaxCurrencyValue, CHIP_ERROR_INVALID_ARGUMENT);
        }
        // Else: null is valid when pricing enabled
    }
    else
    {
        // Pricing disabled - currency must not be provided at all
        VerifyOrReturnError(!newValue.currency.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<DataModel::List<DayEntryStruct::Type>>>(
    const DataModel::Nullable<DataModel::List<DayEntryStruct::Type>> & aValue, void * aCtx)
{
    // Required field check
    if (aValue.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = aValue.Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(aCtx);

    // Validate list length
    if (newList.size() == 0 || newList.size() > kDayEntriesAttrMaxLength)
    {
        ChipLogError(NotSpecified, "Incorrect DayEntries length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each entry
    for (const auto & item : newList)
    {
        // Check for duplicate IDs
        if (!ctx->DayEntryKeyIDs.insert(item.dayEntryID).second)
        {
            ChipLogError(NotSpecified, "Duplicate dayEntryID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        // Validate entry contents
        CHIP_ERROR entryErr = DayEntriesDataClass_Utils::ValidateListEntry(item, ctx);
        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<DataModel::List<DayPatternStruct::Type>>>(
    const DataModel::Nullable<DataModel::List<DayPatternStruct::Type>> & aValue, void * aCtx)
{
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR; // Assuming null is valid for day patterns
    }

    const auto & newList   = aValue.Value();
    auto * ctx             = static_cast<TariffUpdateCtx *>(aCtx);
    uint8_t daysOfWeekMask = 0;

    // Validate list length
    if (newList.size() == 0 || newList.size() > kDayPatternsAttrMaxLength)
    {
        ChipLogError(NotSpecified, "Incorrect dayPatterns length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each pattern
    for (const auto & item : newList)
    {
        if (!ctx->DayPatternKeyIDs.insert(item.dayPatternID).second)
        {
            ChipLogError(NotSpecified, "Duplicate dayPatternID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        daysOfWeekMask |= item.daysOfWeek.Raw();

        CHIP_ERROR entryErr = DayPatternsDataClass_Utils::ValidateListEntry(item, ctx->DayPatternsDayEntryIDs);
        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    // Validate week coverage
    const bool isValidSingleRotatingDay = (!daysOfWeekMask && newList.size() == 1);
    const bool isValidFullWeekCoverage  = (daysOfWeekMask == kFullWeekMask);

    if (!(isValidSingleRotatingDay || isValidFullWeekCoverage))
    {
        ChipLogError(NotSpecified, "Invalid day pattern coverage");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<DataModel::List<TariffComponentStruct::Type>>>(
    const DataModel::Nullable<DataModel::List<TariffComponentStruct::Type>> & aValue, void * aCtx)
{
    // Required field check
    if (aValue.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = aValue.Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(aCtx);

    // Validate list length
    if (newList.size() == 0 || newList.size() > kTariffComponentsAttrMaxLength)
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each component
    for (const auto & item : newList)
    {
        if (!ctx->TariffComponentKeyIDs.insert(item.tariffComponentID).second)
        {
            ChipLogError(NotSpecified, "Duplicate tariffComponentID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        CHIP_ERROR entryErr = TariffComponentsDataClass_Utils::ValidateListEntry(item, ctx);
        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<DataModel::List<TariffPeriodStruct::Type>>>(
    const DataModel::Nullable<DataModel::List<TariffPeriodStruct::Type>> & aValue, void * aCtx)
{
    // Required field check
    if (aValue.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = aValue.Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(aCtx);
    std::map<uint32_t, std::unordered_set<uint32_t>> tariffComponentsMap;

    // Validate list length
    if (newList.size() == 0 || newList.size() > kTariffPeriodsAttrMaxLength)
    {
        ChipLogError(NotSpecified, "Incorrect TariffPeriods length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each period
    for (const auto & item : newList)
    {
        CHIP_ERROR entryErr = TariffPeriodsDataClass_Utils::ValidateListEntry(
            item, ctx->TariffPeriodsDayEntryIDs, ctx->TariffPeriodsTariffComponentIDs, tariffComponentsMap);

        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<DataModel::List<DayStruct::Type>>>(
    const DataModel::Nullable<DataModel::List<DayStruct::Type>> & aValue, void * aCtx)
{
    // Early return for null case (valid)
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    const auto & newList  = aValue.Value();
    auto * ctx            = static_cast<TariffUpdateCtx *>(aCtx);
    uint32_t previousDate = 0;

    // Validate list length
    if (newList.size() == 0 || newList.size() > kIndividualDaysAttrMaxLength)
    {
        ChipLogError(NotSpecified, "Incorrect IndividualDays length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each item
    for (const auto & item : newList)
    {
        // Check date ordering
        if (item.date <= previousDate)
        {
            ChipLogError(NotSpecified, "IndividualDays must be ordered by date");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Validate day type enum
        if (EnsureKnownEnumValue(item.dayType) == DayTypeEnum::kUnknownEnumValue)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Validate dayEntryIDs
        if (item.dayEntryIDs.empty() || item.dayEntryIDs.size() > kDayStructItemMaxDayEntryIDs)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check for duplicates
        if (CommonUtilities::HasDuplicateIDs(item.dayEntryIDs, ctx->IndividualDaysDayEntryIDs))
        {
            ChipLogError(NotSpecified, "Duplicate dayEntryID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        previousDate = item.date;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Validate<DataModel::Nullable<DataModel::List<CalendarPeriodStruct::Type>>>(
    const DataModel::Nullable<DataModel::List<CalendarPeriodStruct::Type>> & aValue, void * aCtx)
{
    // If calendar is null, it's always valid
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    auto & newList   = aValue.Value();
    bool isFirstItem = true;
    Nullable<uint32_t> previousStartDate;

    TariffUpdateCtx * ctx = static_cast<TariffUpdateCtx *>(aCtx);

    std::unordered_set<uint32_t> & CalendarPeriodsDayPatternIDs = ctx->CalendarPeriodsDayPatternIDs;

    auto & tariffStartDate = ctx->TariffStartTimestamp;

    if (tariffStartDate.IsNull())
    {
        // StartDate is Null - tariff is unavailable;
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & item : newList)
    {
        // Validate dayPatternIDs count
        if (item.dayPatternIDs.empty() || item.dayPatternIDs.size() > kCalendarPeriodItemMaxDayPatternIDs)
        {
            ChipLogError(NotSpecified, "DayPatternIDs count must be between 1 and %" PRIu32, (uint32_t) kCalendarPeriodItemMaxDayPatternIDs);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check for duplicate dayPatternIDs
        if (CommonUtilities::HasDuplicateIDs(item.dayPatternIDs, CalendarPeriodsDayPatternIDs))
        {
            ChipLogError(NotSpecified, "Duplicate dayPatternID found in CalendarPeriods");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        // Special handling for first item
        if (isFirstItem)
        {
            if (tariffStartDate.Value() == 0 && (!item.startDate.IsNull() && item.startDate.Value() > 0))
            {
                ChipLogError(NotSpecified,
                             "The first StartDate in CalendarPeriods can't have a value if the StartDate of tariff is 0");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else if (tariffStartDate.Value() > 0 && (item.startDate.IsNull() || item.startDate.Value() == 0))
            {
                ChipLogError(NotSpecified,
                             "The first StartDate in CalendarPeriods can't be not set if the StartDate of tariff is specified");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                previousStartDate = item.startDate;
            }

            isFirstItem = false;
            // First item can have null StartDate
            continue;
        }

        // Subsequent items must have non-null StartDate
        if (item.startDate.IsNull())
        {
            ChipLogError(NotSpecified, "Only first CalendarPeriodStruct can have null StartDate");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Validate StartDate ordering
        if (!previousStartDate.IsNull() && !item.startDate.IsNull() && item.startDate.Value() <= previousStartDate.Value())
        {
            ChipLogError(NotSpecified, "CalendarPeriodStructs must be in increasing StartDate order");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        previousStartDate = item.startDate;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<TariffInformationStruct::Type>(const TariffInformationStruct::Type & input,
                                                   TariffInformationStruct::Type & output)
{
    output.tariffLabel.SetNull();
    output.providerName.SetNull();
    output.currency.ClearValue();
    output.blockMode.SetNull();

    if (!input.tariffLabel.IsNull())
    {
        if (!SpanCopier<char>::Copy(input.tariffLabel.Value(), output.tariffLabel, input.tariffLabel.Value().size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (!input.providerName.IsNull())
    {
        if (!SpanCopier<char>::Copy(input.providerName.Value(), output.providerName, input.providerName.Value().size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (input.currency.HasValue())
    {
        output.currency.Emplace();
        if (input.currency.Value().IsNull())
        {
            output.currency.Value().SetNull();
        }
        else
        {
            output.currency.Value().SetNonNull(input.currency.Value().Value());
        }
    }

    if (!input.blockMode.IsNull())
    {
        output.blockMode.SetNonNull(input.blockMode.Value());
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<DayEntryStruct::Type>(const DayEntryStruct::Type & input, DayEntryStruct::Type & output)
{
    output.dayEntryID = input.dayEntryID;
    output.startTime  = input.startTime;

    output.duration.ClearValue();
    if (input.duration.HasValue())
    {
        output.duration.SetValue(input.duration.Value());
    }

    output.randomizationOffset.ClearValue();
    if (input.randomizationOffset.HasValue())
    {
        output.randomizationOffset.SetValue(input.randomizationOffset.Value());
    }

    output.randomizationType.ClearValue();
    if (input.randomizationType.HasValue())
    {
        output.randomizationType.SetValue(input.randomizationType.Value());
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<TariffComponentStruct::Type>(const TariffComponentStruct::Type & input, TariffComponentStruct::Type & output)
{
    output.tariffComponentID = input.tariffComponentID;

    output.price.ClearValue();
    if (input.price.HasValue())
    {
        output.price.Emplace();
        output.price.Value().SetNull();
        if (!input.price.Value().IsNull())
        {
            auto & priceInput = input.price.Value().Value();
            TariffPriceStruct::Type tmp_price;

            tmp_price.priceType = priceInput.priceType;

            if (priceInput.price.HasValue())
            {
                tmp_price.price.SetValue(priceInput.price.Value());
            }

            if (priceInput.priceLevel.HasValue())
            {
                tmp_price.priceLevel.SetValue(priceInput.priceLevel.Value());
            }

            output.price.Value().SetNonNull(tmp_price);
        }
    }

    output.friendlyCredit.ClearValue();
    if (input.friendlyCredit.HasValue())
    {
        output.friendlyCredit.SetValue(input.friendlyCredit.Value());
    }

    output.auxiliaryLoad.ClearValue();
    if (input.auxiliaryLoad.HasValue())
    {
        output.auxiliaryLoad.Emplace();
        output.auxiliaryLoad.Value().number        = input.auxiliaryLoad.Value().number;
        output.auxiliaryLoad.Value().requiredState = input.auxiliaryLoad.Value().requiredState;
    }

    output.peakPeriod.ClearValue();
    if (input.peakPeriod.HasValue())
    {
        output.peakPeriod.Emplace();
        output.peakPeriod.Value().severity   = input.peakPeriod.Value().severity;
        output.peakPeriod.Value().peakPeriod = input.peakPeriod.Value().peakPeriod;
    }

    output.powerThreshold.ClearValue();
    if (input.powerThreshold.HasValue())
    {
        output.powerThreshold.Emplace();
        output.powerThreshold.Value() = input.powerThreshold.Value();
    }

    output.threshold.SetNull();
    if (!input.threshold.IsNull())
    {
        output.threshold.SetNonNull(input.threshold.Value());
    }

    output.label.ClearValue();
    if (input.label.HasValue())
    {
        output.label.Emplace();
        output.label.Value().SetNull();
        if (!input.label.Value().IsNull())
        {
            if (!SpanCopier<char>::Copy(input.label.Value().Value(), output.label.Value(), input.label.Value().Value().size()))
            {
                return CHIP_ERROR_NO_MEMORY;
            }
        }
    }

    output.predicted.ClearValue();
    if (input.predicted.HasValue())
    {
        output.predicted.SetValue(input.predicted.Value());
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<TariffPeriodStruct::Type>(const TariffPeriodStruct::Type & input, TariffPeriodStruct::Type & output)
{
    output.label.SetNull();
    if (!input.label.IsNull())
    {
        if (!SpanCopier<char>::Copy(input.label.Value(), output.label, input.label.Value().size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()),
                                    output.dayEntryIDs, kTariffPeriodItemMaxIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.tariffComponentIDs.data(), input.tariffComponentIDs.size()),
                                    output.tariffComponentIDs, kTariffPeriodItemMaxIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<DayPatternStruct::Type>(const DayPatternStruct::Type & input, DayPatternStruct::Type & output)
{
    output.dayPatternID = input.dayPatternID;
    output.daysOfWeek   = input.daysOfWeek;

    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()),
                                    output.dayEntryIDs, kDayPatternItemMaxDayEntryIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<DayStruct::Type>(const DayStruct::Type & input, DayStruct::Type & output)
{
    output.date    = input.date;
    output.dayType = input.dayType;

    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()),
                                    output.dayEntryIDs, kDayStructItemMaxDayEntryIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CopyData<CalendarPeriodStruct::Type>(const CalendarPeriodStruct::Type & input, CalendarPeriodStruct::Type & output)
{
    output.startDate.SetNull();
    if (!input.startDate.IsNull())
    {
        output.startDate.SetNonNull(input.startDate.Value());
    }

    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayPatternIDs.data(), input.dayPatternIDs.size()),
                                    output.dayPatternIDs, kCalendarPeriodItemMaxDayPatternIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

template <>
void CleanupStructValue<TariffInformationStruct::Type>(TariffInformationStruct::Type & aValue)
{
    if (!aValue.tariffLabel.IsNull() && aValue.tariffLabel.Value().data())
    {
        Platform::MemoryFree(const_cast<char *>(aValue.tariffLabel.Value().data()));
        aValue.tariffLabel.SetNull();
    }

    if (!aValue.providerName.IsNull() && aValue.providerName.Value().data())
    {
        Platform::MemoryFree(const_cast<char *>(aValue.providerName.Value().data()));
        aValue.providerName.SetNull();
    }

    aValue.currency.ClearValue();
}

template <>
void CleanupStructValue<DayEntryStruct::Type>(DayEntryStruct::Type & aValue)
{
    aValue.duration.ClearValue();
    aValue.randomizationOffset.ClearValue();
    aValue.randomizationType.ClearValue();
}

template <>
void CleanupStructValue<TariffPeriodStruct::Type>(TariffPeriodStruct::Type & aValue)
{
    if (!aValue.label.IsNull() && aValue.label.Value().data())
    {
        Platform::MemoryFree(const_cast<char *>(aValue.label.Value().data()));
        aValue.label.SetNull();
    }
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
    CommonUtilities::CleanUpIDs(aValue.tariffComponentIDs);
}

template <>
void CleanupStructValue<TariffComponentStruct::Type>(TariffComponentStruct::Type & aValue)
{

    if (aValue.label.HasValue() && !aValue.label.Value().IsNull())
    {
        MemoryFree(const_cast<char *>(aValue.label.Value().Value().data()));
        aValue.label.ClearValue();
    }

    aValue.price.ClearValue();
    aValue.friendlyCredit.ClearValue();
    aValue.auxiliaryLoad.ClearValue();
    aValue.peakPeriod.ClearValue();
    aValue.powerThreshold.ClearValue();
}

template <>
void CleanupStructValue<DayPatternStruct::Type>(DayPatternStruct::Type & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

template <>
void CleanupStructValue<DayStruct::Type>(DayStruct::Type & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

template <>
void CleanupStructValue<CalendarPeriodStruct::Type>(CalendarPeriodStruct::Type & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayPatternIDs);
}

} // namespace CommodityTariffAttrsDataMgmt
} // namespace app
} // namespace chip
