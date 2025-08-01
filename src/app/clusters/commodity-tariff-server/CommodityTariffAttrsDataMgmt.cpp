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

#include "CommodityTariffConsts.h"
#include <cassert>

#include "commodity-tariff-server.h" //Used for TariffUpdateCtx

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

// static constexpr size_t kAuxSwitchesSettingsMax = 8;

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

/**
 * @brief Custom comparator for Optional-wrapped struct types.
 *
 * @tparam T The struct type being compared
 * @param lhs Left-hand Optional<T> to compare
 * @param rhs Right-hand Optional<T> to compare
 * @return bool True if equal, false otherwise
 *
 * @note This exists because:
 *       1. Optional<T>::operator== requires T to implement operator==
 *       2. Many Matter structs don't implement operator== by design
 *       3. We need to compare structs field-by-field for accurate change detection
 *       4. The default Optional comparison would fail to compile for non-comparable types
 */
template <typename T>
static bool OptionalStructsAreEqual(const Optional<T> & lhs, const Optional<T> & rhs)
{
    if (lhs.HasValue() != rhs.HasValue())
        return false;
    if (!lhs.HasValue())
        return true;
    return (lhs.Value() == rhs.Value());
}

/**
 * @brief Custom comparator for Optional<Nullable<T>> wrapped struct types.
 *
 * @tparam T The struct type being compared
 * @param lhs Left-hand Optional<Nullable<T>> to compare
 * @param rhs Right-hand Optional<Nullable<T>> to compare
 * @return bool True if equal, false otherwise
 *
 * @note This exists because:
 *       1. Combines three levels of wrapping (Optional+Nullable+Struct)
 *       2. Handles all null/missing combinations explicitly
 *       3. Provides consistent behavior when T lacks operator==
 *       4. Needed for attribute change detection in Matter's data model
 *       5. The nested wrappers make default comparisons impractical
 */

template <typename T>
static bool OptionalNullableStructsAreEqual(const Optional<DataModel::Nullable<T>> & lhs,
                                            const Optional<DataModel::Nullable<T>> & rhs)
{
    // Both missing -> true
    if (!lhs.HasValue() && !rhs.HasValue())
    {
        return true;
    }

    // One present, one missing -> false
    if (lhs.HasValue() != rhs.HasValue())
    {
        return false;
    }

    // Both present and both null -> true
    if (lhs.Value().IsNull() && rhs.Value().IsNull())
    {
        return true;
    }

    // One null, one not null -> false
    if (lhs.Value().IsNull() != rhs.Value().IsNull())
    {
        return false;
    }

    // Both present and not null -> return true if equal
    return (lhs.Value().Value() == rhs.Value().Value());
}

}; // namespace CommonUtilities

CHIP_ERROR TariffUnitDataClass::Validate(const ValueType & aValue) const
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR StartDateDataClass::Validate(const ValueType & aValue) const
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultRandomizationOffsetDataClass::Validate(const ValueType & aValue) const
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultRandomizationTypeDataClass::Validate(const ValueType & aValue) const
{
    return CHIP_NO_ERROR;
}

// TariffInfoDataClass

CHIP_ERROR TariffInfoDataClass::Validate(const ValueType & aValue) const
{
    // The tariff info is always required (cannot be null)
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    const PayloadType & newValue = aValue.Value();
    auto * ctx                   = static_cast<TariffUpdateCtx *>(mAuxData);

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

bool TariffInfoDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    if (!CommonUtilities::OptionalNullableStructsAreEqual(source.currency, destination.currency))
    {
        return true;
    }

    return (source.tariffLabel != destination.tariffLabel || source.providerName != destination.providerName ||
            source.blockMode != destination.blockMode);
}

void TariffInfoDataClass::CleanupStructValue(PayloadType & aValue)
{
    if (!aValue.tariffLabel.IsNull() && aValue.tariffLabel.Value().data())
    {
        MemoryFree(const_cast<char *>(aValue.tariffLabel.Value().data()));
        aValue.tariffLabel.SetNull();
    }

    if (!aValue.providerName.IsNull() && aValue.providerName.Value().data())
    {
        MemoryFree(const_cast<char *>(aValue.providerName.Value().data()));
        aValue.providerName.SetNull();
    }

    aValue.currency.ClearValue();
}

// DayEntriesDataClass
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

CHIP_ERROR DayEntriesDataClass::Validate(const ValueType & aValue) const
{
    // Required field check
    if (aValue.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = aValue.Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(mAuxData);

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

bool DayEntriesDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    using chip::app::Clusters::CommodityTariff::Structs::operator!=;

    return source != destination;
}

void DayEntriesDataClass::CleanupStructValue(PayloadType & aValue)
{
    aValue.duration.ClearValue();

    aValue.randomizationOffset.ClearValue();
    aValue.randomizationType.ClearValue();
}

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

CHIP_ERROR DayPatternsDataClass::Validate(const ValueType & aValue) const
{
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR; // Assuming null is valid for day patterns
    }

    const auto & newList   = aValue.Value();
    auto * ctx             = static_cast<TariffUpdateCtx *>(mAuxData);
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

bool DayPatternsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    if ((source.dayPatternID != destination.dayPatternID) || (source.daysOfWeek.Raw() != destination.daysOfWeek.Raw()))
    {
        return true;
    }

    return (source.dayEntryIDs != destination.dayEntryIDs);
}

void DayPatternsDataClass::CleanupStructValue(PayloadType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

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

CHIP_ERROR TariffPeriodsDataClass::Validate(const ValueType & aValue) const
{
    // Required field check
    if (aValue.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = aValue.Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(mAuxData);
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

bool TariffPeriodsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    return (source.tariffComponentIDs != destination.tariffComponentIDs || source.dayEntryIDs != destination.dayEntryIDs ||
            source.label != destination.label);
}

void TariffPeriodsDataClass::CleanupStructValue(PayloadType & aValue)
{
    if (!aValue.label.IsNull() && aValue.label.Value().data())
    {
        MemoryFree(const_cast<char *>(aValue.label.Value().data()));
        aValue.label.SetNull();
    }
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
    CommonUtilities::CleanUpIDs(aValue.tariffComponentIDs);
}

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

CHIP_ERROR TariffComponentsDataClass::Validate(const ValueType & aValue) const
{
    // Required field check
    if (aValue.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = aValue.Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(mAuxData);

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

bool TariffComponentsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{

    if (source.label != destination.label)
        return true;

    // If PRICE feature are supported
    if (!CommonUtilities::OptionalNullableStructsAreEqual(source.price, destination.price))
    {
        return true;
    }

    // If FCRED feature are supported
    if (source.friendlyCredit != destination.friendlyCredit)
    {
        return true;
    }

    // If AUXLD feature are supported
    if (!CommonUtilities::OptionalStructsAreEqual(source.auxiliaryLoad, destination.auxiliaryLoad))
    {
        return true;
    }

    // If PEAKP feature are supported
    if (!CommonUtilities::OptionalStructsAreEqual<>(source.peakPeriod, destination.peakPeriod))
    {
        return true;
    }

    // If PWRTHLD feature are supported
    if (!CommonUtilities::OptionalStructsAreEqual(source.powerThreshold, destination.powerThreshold))
    {
        return true;
    }

    if (source.predicted != destination.predicted)
    {
        return true;
    }

    return !(source.threshold == destination.threshold);
}

void TariffComponentsDataClass::CleanupStructValue(PayloadType & aValue)
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

// IndividualDaysDataClass
CHIP_ERROR IndividualDaysDataClass::Validate(const ValueType & aValue) const
{
    // Early return for null case (valid)
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    const auto & newList  = aValue.Value();
    auto * ctx            = static_cast<TariffUpdateCtx *>(mAuxData);
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

bool IndividualDaysDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    using chip::app::Clusters::CommodityTariff::Structs::operator!=;
    return (source != destination);
}

void IndividualDaysDataClass::CleanupStructValue(PayloadType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

// CalendarPeriodsDataClass

CHIP_ERROR CalendarPeriodsDataClass::Validate(const ValueType & aValue) const
{
    // If calendar is null, it's always valid
    if (aValue.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    auto & newList   = aValue.Value();
    bool isFirstItem = true;
    Nullable<uint32_t> previousStartDate;
    std::unordered_set<uint32_t> & CalendarPeriodsDayPatternIDs =
        static_cast<TariffUpdateCtx *>(mAuxData)->CalendarPeriodsDayPatternIDs;

    for (const auto & item : newList)
    {
        // Validate dayPatternIDs count
        if (item.dayPatternIDs.empty() || item.dayPatternIDs.size() > kCalendarPeriodItemMaxDayPatternIDs)
        {
            ChipLogError(NotSpecified, "DayPatternIDs count must be between 1 and %" PRIuMAX, kCalendarPeriodItemMaxDayPatternIDs);
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
            isFirstItem = false;
            // First item can have null StartDate
            previousStartDate = item.startDate;
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

bool CalendarPeriodsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    return (source.startDate != destination.startDate || source.dayPatternIDs != destination.dayPatternIDs);
}

void CalendarPeriodsDataClass::CleanupStructValue(PayloadType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayPatternIDs);
}
