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
using namespace chip::app::CommodityTariffContainers;
using namespace CommodityTariffConsts;

#define VerifyOrReturnError_LogSend(expr, code, ...)                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            ChipLogError(AppServer, __VA_ARGS__);                                                                                  \
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

template <size_t Capacity>
static bool HasDuplicateIDs(const DataModel::List<const uint32_t> & IDs,
                            chip::app::CommodityTariffContainers::CTC_UnorderedSet<uint32_t, Capacity> & seen)
{
    for (auto id : IDs)
    {
        if (!seen.insert(id))
        {
            ChipLogError(AppServer, "Duplicate ID %" PRIu32 " - ID already exists", id);
            return true; // Duplicate found
        }
    }
    return false;
}
}; // namespace CommonUtilities
namespace chip {
namespace app {
namespace CommodityTariffAttrsDataMgmt {

using namespace CommodityTariffConsts;
using namespace chip::app::Clusters::CommodityTariff::Structs;

template <typename T>
CHIP_ERROR CTC_BaseDataClass<T>::CopyData(const StructType & input, StructType & output)
{
    // Log error since this base implementation doesn't do anything meaningful
    ChipLogError(DataManagement, "CopyData() called on base class - this should be overridden!");

    // Return appropriate error code
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<TariffInformationStruct::Type>>::CopyData(const StructType & input,
                                                                                           StructType & output)
{
    output.tariffLabel.SetNull();
    output.providerName.SetNull();
    output.currency.ClearValue();
    output.blockMode.SetNull();

    if (!input.tariffLabel.IsNull())
    {
        ReturnErrorOnFailure(
            SpanCopier<char>::Copy(input.tariffLabel.Value(), output.tariffLabel, input.tariffLabel.Value().size()));
    }

    if (!input.providerName.IsNull())
    {
        ReturnErrorOnFailure(
            SpanCopier<char>::Copy(input.providerName.Value(), output.providerName, input.providerName.Value().size()));
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
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayEntryStruct::Type>>>::CopyData(const StructType & input,
                                                                                                   StructType & output)
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
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<TariffComponentStruct::Type>>>::CopyData(const StructType & input,
                                                                                                          StructType & output)
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
            ReturnErrorOnFailure(
                SpanCopier<char>::Copy(input.label.Value().Value(), output.label.Value(), input.label.Value().Value().size()));
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
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<TariffPeriodStruct::Type>>>::CopyData(const StructType & input,
                                                                                                       StructType & output)
{
    output.label.SetNull();
    if (!input.label.IsNull())
    {
        ReturnErrorOnFailure(SpanCopier<char>::Copy(input.label.Value(), output.label, input.label.Value().size()));
    }

    ReturnErrorOnFailure(SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()),
                                                    output.dayEntryIDs, input.dayEntryIDs.size()));

    ReturnErrorOnFailure(
        SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.tariffComponentIDs.data(), input.tariffComponentIDs.size()),
                                   output.tariffComponentIDs, input.tariffComponentIDs.size()));

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayPatternStruct::Type>>>::CopyData(const StructType & input,
                                                                                                     StructType & output)
{
    output.dayPatternID = input.dayPatternID;
    output.daysOfWeek   = input.daysOfWeek;

    ReturnErrorOnFailure(SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()),
                                                    output.dayEntryIDs, input.dayEntryIDs.size()));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayStruct::Type>>>::CopyData(const StructType & input,
                                                                                              StructType & output)
{
    output.date    = input.date;
    output.dayType = input.dayType;

    ReturnErrorOnFailure(SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()),
                                                    output.dayEntryIDs, input.dayEntryIDs.size()));

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<CalendarPeriodStruct::Type>>>::CopyData(const StructType & input,
                                                                                                         StructType & output)
{
    output.startDate.SetNull();
    if (!input.startDate.IsNull())
    {
        output.startDate.SetNonNull(input.startDate.Value());
    }

    ReturnErrorOnFailure(
        SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayPatternIDs.data(), input.dayPatternIDs.size()),
                                   output.dayPatternIDs, input.dayPatternIDs.size()));

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ValidateListEntry(const T & entryNewValue, void * aCtx)
{
    // Log error since this base implementation doesn't do anything meaningful
    ChipLogError(DataManagement, "CopyData() called on base class - this should be overridden!");

    // Return appropriate error code
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ValidateListEntry(const DayPatternStruct::Type & entryNewValue, void * aCtx)
{
    auto * ctx = static_cast<TariffUpdateCtx *>(aCtx);

    if (entryNewValue.dayEntryIDs.empty() || entryNewValue.dayEntryIDs.size() > kDayPatternItemMaxDayEntryIDs)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Check that the current day pattern item has no duplicated dayEntryIDs

    if (CommonUtilities::HasDuplicateIDs(entryNewValue.dayEntryIDs, ctx->RefsToDayEntryIDsFromDays))
    {
        ChipLogError(AppServer, "The DayPattern entry Duplicate DayEntryID found ");
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateListEntry(const DayEntryStruct::Type & entryNewValue, void * aCtx)
{
    auto * ctx = static_cast<TariffUpdateCtx *>(aCtx);

    VerifyOrReturnError_LogSend(entryNewValue.startTime < kDayEntryDurationLimit, CHIP_ERROR_INVALID_ARGUMENT,
                                "DayEntry startTime must be less than %u", kDayEntryDurationLimit);

    if (CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kRandomization))
    {
        if (entryNewValue.randomizationOffset.HasValue() && entryNewValue.randomizationType.HasValue())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(entryNewValue.randomizationType.Value()) !=
                                    DayEntryRandomizationTypeEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogError(AppServer, "If the RNDM feature is enabled, the randomization* field is required!");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR ValidateListEntry(const TariffComponentStruct::Type & entryNewValue, void * aCtx)
{
    BitMask<Feature> entryFeatures;
    auto * ctx = static_cast<TariffUpdateCtx *>(aCtx);

    VerifyOrReturnError(entryNewValue.tariffComponentID > 0, CHIP_ERROR_INVALID_ARGUMENT);

    if ((ctx->blockMode == BlockModeEnum::kNoBlock) == !entryNewValue.threshold.IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

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
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kPricing), CHIP_ERROR_INVALID_ARGUMENT);

        const auto & price = entryNewValue.price.Value().Value();
        VerifyOrReturnError(EnsureKnownEnumValue(price.priceType) != TariffPriceTypeEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);

        entryFeatures.Set(CommodityTariff::Feature::kPricing);
    }

    // Validate friendlyCredit field
    if (entryNewValue.friendlyCredit.HasValue())
    {
        // If friendlyCredit is provided, the FCRED feature MUST be supported
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kFriendlyCredit),
                            CHIP_ERROR_INVALID_ARGUMENT);

        entryFeatures.Set(CommodityTariff::Feature::kFriendlyCredit);
    }

    // Validate auxiliaryLoad field
    if (entryNewValue.auxiliaryLoad.HasValue())
    {
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kAuxiliaryLoad),
                            CHIP_ERROR_INVALID_ARGUMENT);

        const auto & auxiliaryLoad = entryNewValue.auxiliaryLoad.Value();
        VerifyOrReturnError(EnsureKnownEnumValue(auxiliaryLoad.requiredState) != AuxiliaryLoadSettingEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);

        entryFeatures.Set(CommodityTariff::Feature::kAuxiliaryLoad);
    }

    // Validate peakPeriod field
    if (entryNewValue.peakPeriod.HasValue())
    {
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kPeakPeriod),
                            CHIP_ERROR_INVALID_ARGUMENT);

        const auto & peakPeriod = entryNewValue.peakPeriod.Value();
        VerifyOrReturnError(EnsureKnownEnumValue(peakPeriod.severity) != PeakPeriodSeverityEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(peakPeriod.peakPeriod > 0, CHIP_ERROR_INVALID_ARGUMENT);

        entryFeatures.Set(CommodityTariff::Feature::kPeakPeriod);
    }

    // Validate powerThreshold field
    if (entryNewValue.powerThreshold.HasValue())
    {
        VerifyOrReturnError(CommonUtilities::HasFeatureInCtx(ctx, CommodityTariff::Feature::kPowerThreshold),
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

        entryFeatures.Set(CommodityTariff::Feature::kPowerThreshold);
    }

    if (entryNewValue.predicted.HasValue())
    {
        // No need to check the value itself since it's just a bool
        // But we can add debug logging if needed:
        ChipLogDetail(NotSpecified, "Predicted flag set to %s", entryNewValue.predicted.Value() ? "true" : "false");
    }

    if (!ctx->TariffComponentKeyIDsFeatureMap.insert(entryNewValue.tariffComponentID, entryFeatures.Raw()))
    {
        ChipLogError(AppServer, "Duplicate tariffComponentID found");
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateListEntry(const TariffPeriodStruct::Type & entryNewValue, void * aCtx)
{
    auto * ctx = static_cast<TariffUpdateCtx *>(aCtx);
    CTC_UnorderedSet<uint32_t, kTariffPeriodItemMaxIDs> entryTcIDs;

    if (!entryNewValue.label.IsNull())
    {
        const auto & labelSpan = entryNewValue.label.Value();
        if (labelSpan.size() > kDefaultStringValuesMaxBufLength)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (labelSpan.empty())
        {
            ChipLogError(AppServer, "TariffPeriod label must not be empty if present");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (entryNewValue.dayEntryIDs.empty() || entryNewValue.dayEntryIDs.size() > kTariffPeriodItemMaxIDs)
        return CHIP_ERROR_INVALID_ARGUMENT;

    if (entryNewValue.tariffComponentIDs.empty() || entryNewValue.tariffComponentIDs.size() > kTariffPeriodItemMaxIDs)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Checks that dayEntryIDs references has no duplicates among another TP entries
    if (CommonUtilities::HasDuplicateIDs(entryNewValue.dayEntryIDs, ctx->RefsToDayEntryIDsFromTariffPeriods))
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    // Checks that the current period item has no duplicated tariffComponentIDs
    if (CommonUtilities::HasDuplicateIDs(entryNewValue.tariffComponentIDs, entryTcIDs))
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    ctx->RefsToTariffComponentIDsFromTariffPeriods.merge(entryTcIDs);

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR CTC_BaseDataClass<T>::ValidateNewValue()
{
    // Log error since this base implementation doesn't do anything meaningful
    ChipLogError(DataManagement, "CopyData() called on base class - this should be overridden!");

    // Return appropriate error code
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<Globals::TariffUnitEnum>>::ValidateNewValue()
{
    VerifyOrReturnError(EnsureKnownEnumValue(GetNewValueRef().Value()) != Globals::TariffUnitEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<int16_t>>::ValidateNewValue()
{
    if (mAttrId == Clusters::CommodityTariff::Attributes::DefaultRandomizationOffset::Id)
    {
        if (CommonUtilities::HasFeatureInCtx(static_cast<TariffUpdateCtx *>(mAuxData), CommodityTariff::Feature::kRandomization))
        {
            ChipLogDetail(NotSpecified, "DefaultRandomizationOffset: %u", GetNewValueRef().Value());
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<uint32_t>>::ValidateNewValue()
{
    if (mAttrId == Clusters::CommodityTariff::Attributes::StartDate::Id)
    {
        const auto & newValue = GetNewValueRef();

        if (!newValue.IsNull() && newValue.Value() != 0)
        {
            VerifyOrReturnError((static_cast<TariffUpdateCtx *>(mAuxData)->TariffUpdateTimestamp <= newValue.Value()),
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DayEntryRandomizationTypeEnum>>::ValidateNewValue()
{
    VerifyOrReturnError(EnsureKnownEnumValue(GetNewValueRef().Value()) != DayEntryRandomizationTypeEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<TariffInformationStruct::Type>>::ValidateNewValue()
{
    if (GetNewValueRef().IsNull())
    {
        return CHIP_NO_ERROR;
    }

    const auto & newValue = GetNewValueRef().Value();
    auto * ctx            = static_cast<TariffUpdateCtx *>(mAuxData);

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
        ctx->blockMode = newValue.blockMode.Value();
    }
    else
    {
        ctx->blockMode = BlockModeEnum::kNoBlock;
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
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayEntryStruct::Type>>>::ValidateNewValue()
{
    // Temporary DE's ID values storage just for dups checking
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength> dayEntryKeyIDs;

    // Required field check
    if (GetNewValueRef().IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = GetNewValueRef().Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(mAuxData);

    // Validate list length
    if (newList.size() == 0 || newList.size() > kDayEntriesAttrMaxLength)
    {
        ChipLogError(AppServer, "Incorrect DayEntries length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each entry
    for (const auto & item : newList)
    {
        // Check for duplicate IDs
        if (!dayEntryKeyIDs.insert(item.dayEntryID))
        {
            ChipLogError(AppServer, "Duplicate dayEntryID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        // Validate entry contents
        CHIP_ERROR entryErr = ValidateListEntry(item, ctx);
        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayPatternStruct::Type>>>::ValidateNewValue()
{
    // DayPattern ID Tracking
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayPatternsAttrMaxLength> dayPatternKeyIDs;

    if (GetNewValueRef().IsNull())
    {
        return CHIP_NO_ERROR; // Assuming null is valid for day patterns
    }

    const auto & newList   = GetNewValueRef().Value();
    auto * ctx             = static_cast<TariffUpdateCtx *>(mAuxData);
    uint8_t daysOfWeekMask = 0;

    // Validate list length
    if (newList.size() == 0 || newList.size() > kDayPatternsAttrMaxLength)
    {
        ChipLogError(AppServer, "Incorrect dayPatterns length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each pattern
    for (const auto & item : newList)
    {
        if (!dayPatternKeyIDs.insert(item.dayPatternID))
        {
            ChipLogError(AppServer, "Duplicate dayPatternID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        daysOfWeekMask |= item.daysOfWeek.Raw();

        CHIP_ERROR entryErr = ValidateListEntry(item, ctx);
        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    // Validate week coverage
    const bool isValidSingleRotatingDay = (!daysOfWeekMask && !newList.empty());
    const bool isValidFullWeekCoverage  = (daysOfWeekMask == kFullWeekMask);

    if (!(isValidSingleRotatingDay || isValidFullWeekCoverage))
    {
        ChipLogError(AppServer, "Invalid day pattern coverage");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<TariffComponentStruct::Type>>>::ValidateNewValue()
{
    // Required field check
    if (GetNewValueRef().IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = GetNewValueRef().Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(mAuxData);

    // Validate list length
    if (newList.size() == 0 || newList.size() > kTariffComponentsAttrMaxLength)
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each component
    for (const auto & item : newList)
    {
        CHIP_ERROR entryErr = ValidateListEntry(item, ctx);
        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<TariffPeriodStruct::Type>>>::ValidateNewValue()
{
    // Required field check
    if (GetNewValueRef().IsNull())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const auto & newList = GetNewValueRef().Value();
    auto * ctx           = static_cast<TariffUpdateCtx *>(mAuxData);

    // Validate list length
    if (newList.size() == 0 || newList.size() > kTariffPeriodsAttrMaxLength)
    {
        ChipLogError(AppServer, "Incorrect TariffPeriods length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each period
    for (const auto & item : newList)
    {
        CHIP_ERROR entryErr = ValidateListEntry(item, ctx);

        if (entryErr != CHIP_NO_ERROR)
        {
            return entryErr;
        }
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayStruct::Type>>>::ValidateNewValue()
{
    // Early return for null case (valid)
    if (GetNewValueRef().IsNull())
    {
        return CHIP_NO_ERROR;
    }

    const auto & newList  = GetNewValueRef().Value();
    auto * ctx            = static_cast<TariffUpdateCtx *>(mAuxData);
    uint32_t previousDate = 0;

    // Validate list length
    if (newList.size() == 0 || newList.size() > kIndividualDaysAttrMaxLength)
    {
        ChipLogError(AppServer, "Incorrect IndividualDays length");
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Validate each item
    for (const auto & item : newList)
    {
        // Check date ordering
        if (item.date <= previousDate)
        {
            ChipLogError(AppServer, "IndividualDays must be ordered by date");
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
        if (CommonUtilities::HasDuplicateIDs(item.dayEntryIDs, ctx->RefsToDayEntryIDsFromDays))
        {
            ChipLogError(AppServer, "Duplicate dayEntryID found");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        previousDate = item.date;
    }

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<DataModel::List<CalendarPeriodStruct::Type>>>::ValidateNewValue()
{
    // If calendar is null, it's always valid
    if (GetNewValueRef().IsNull())
    {
        return CHIP_NO_ERROR;
    }

    auto & newList   = GetNewValueRef().Value();
    bool isFirstItem = true;
    Nullable<uint32_t> previousStartDate;

    TariffUpdateCtx * ctx = static_cast<TariffUpdateCtx *>(mAuxData);

    auto & RefsToDayPatternIDsFromCalendarPeriods = ctx->RefsToDayPatternIDsFromCalendarPeriods;

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
            ChipLogError(AppServer, "DayPatternIDs count must be between 1 and %" PRIu32,
                         (uint32_t) kCalendarPeriodItemMaxDayPatternIDs);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check for duplicate dayPatternIDs
        if (CommonUtilities::HasDuplicateIDs(item.dayPatternIDs, RefsToDayPatternIDsFromCalendarPeriods))
        {
            ChipLogError(AppServer, "Duplicate dayPatternID found in CalendarPeriods");
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        // Special handling for first item
        if (isFirstItem)
        {
            // Case 1: Invalid - tariff start date is 0 but item has a positive start date
            if (tariffStartDate.Value() == 0 && (!item.startDate.IsNull() && item.startDate.Value() > 0))
            {
                ChipLogError(AppServer,
                             "The first StartDate in CalendarPeriods can't have a value if the StartDate of tariff is 0");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            // Case 2: Invalid - tariff has positive start date but item has no valid start date
            if (tariffStartDate.Value() > 0 && (item.startDate.IsNull() || item.startDate.Value() == 0))
            {
                ChipLogError(AppServer,
                             "The first StartDate in CalendarPeriods can't be not set if the StartDate of tariff is specified");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            previousStartDate = item.startDate;
            isFirstItem       = false;
            // First item can have null StartDate
            continue;
        }

        // Subsequent items must have non-null StartDate
        if (item.startDate.IsNull())
        {
            ChipLogError(AppServer, "Only first CalendarPeriodStruct can have null StartDate");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Validate StartDate ordering
        if (!previousStartDate.IsNull() && !item.startDate.IsNull() && item.startDate.Value() <= previousStartDate.Value())
        {
            ChipLogError(AppServer, "CalendarPeriodStructs must be in increasing StartDate order");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        previousStartDate = item.startDate;
    }

    return CHIP_NO_ERROR;
}

// Define the template method
template <typename T>
void CTC_BaseDataClass<T>::CleanupStruct(StructType & aValue)
{
    // Implementation for all derived classes
}

template <>
void CTC_BaseDataClass<DataModel::Nullable<TariffInformationStruct::Type>>::CleanupStruct(StructType & aValue)
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
void CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayEntryStruct::Type>>>::CleanupStruct(StructType & aValue)
{
    aValue.duration.ClearValue();
    aValue.randomizationOffset.ClearValue();
    aValue.randomizationType.ClearValue();
}

template <>
void CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayPatternStruct::Type>>>::CleanupStruct(StructType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

template <>
void CTC_BaseDataClass<DataModel::Nullable<DataModel::List<TariffComponentStruct::Type>>>::CleanupStruct(StructType & aValue)
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
void CTC_BaseDataClass<DataModel::Nullable<DataModel::List<TariffPeriodStruct::Type>>>::CleanupStruct(StructType & aValue)
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
void CTC_BaseDataClass<DataModel::Nullable<DataModel::List<DayStruct::Type>>>::CleanupStruct(StructType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

template <>
void CTC_BaseDataClass<DataModel::Nullable<DataModel::List<CalendarPeriodStruct::Type>>>::CleanupStruct(StructType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayPatternIDs);
}

} // namespace CommodityTariffAttrsDataMgmt
} // namespace app
} // namespace chip
