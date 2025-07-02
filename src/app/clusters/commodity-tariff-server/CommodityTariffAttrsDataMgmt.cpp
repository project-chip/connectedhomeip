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

#include "commodity-tariff-server.h"
#include <cassert>
#include <cstdint>

namespace chip {

// Ensure all operators are in the same namespace as their types

// Span comparison operators
inline bool operator==(const Span<const char> & a, const Span<const char> & b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

inline bool operator!=(const Span<const char> & a, const Span<const char> & b)
{
    return !(a == b);
}

inline bool operator==(const Span<const uint32_t> & a, const Span<const uint32_t> & b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

inline bool operator!=(const Span<const uint32_t> & a, const Span<const uint32_t> & b)
{
    return !(a == b);
}

namespace app {
namespace Clusters {
namespace Globals {
namespace Structs {

// CurrencyStruct
inline bool operator==(const CurrencyStruct::Type & lhs, const CurrencyStruct::Type & rhs)
{
    return (lhs.currency == rhs.currency) && (lhs.decimalPoints == rhs.decimalPoints);
}

inline bool operator!=(const CurrencyStruct::Type & lhs, const CurrencyStruct::Type & rhs)
{
    return !(lhs == rhs);
}

// PowerThresholdStruct
inline bool operator==(const PowerThresholdStruct::Type & lhs, const PowerThresholdStruct::Type & rhs)
{
    return (lhs.powerThresholdSource == rhs.powerThresholdSource) && (lhs.powerThreshold == rhs.powerThreshold) &&
        (lhs.apparentPowerThreshold == rhs.apparentPowerThreshold);
}

inline bool operator!=(const PowerThresholdStruct::Type & lhs, const PowerThresholdStruct::Type & rhs)
{
    return !(lhs == rhs);
}

} // namespace Structs
} // namespace Globals

namespace CommodityTariff {
namespace Structs {

// TariffPriceStruct
inline bool operator==(const TariffPriceStruct::Type & lhs, const TariffPriceStruct::Type & rhs)
{
    return (lhs.priceType == rhs.priceType) && (lhs.price == rhs.price) && (lhs.priceLevel == rhs.priceLevel);
}

inline bool operator!=(const TariffPriceStruct::Type & lhs, const TariffPriceStruct::Type & rhs)
{
    return !(lhs == rhs);
}

// AuxiliaryLoadSwitchSettingsStruct
inline bool operator==(const AuxiliaryLoadSwitchSettingsStruct::Type & lhs, const AuxiliaryLoadSwitchSettingsStruct::Type & rhs)
{
    return (lhs.number == rhs.number) && (lhs.requiredState == rhs.requiredState);
}

inline bool operator!=(const AuxiliaryLoadSwitchSettingsStruct::Type & lhs, const AuxiliaryLoadSwitchSettingsStruct::Type & rhs)
{
    return !(lhs == rhs);
}

// PeakPeriodStruct
inline bool operator==(const PeakPeriodStruct::Type & lhs, const PeakPeriodStruct::Type & rhs)
{
    return (lhs.severity == rhs.severity) && (lhs.peakPeriod == rhs.peakPeriod);
}

inline bool operator!=(const PeakPeriodStruct::Type & lhs, const PeakPeriodStruct::Type & rhs)
{
    return !(lhs == rhs);
}

} // namespace Structs
} // namespace CommodityTariff

} // namespace Clusters

} // namespace app
} // namespace chip

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

static void CleanUpIDs(DataModel::List<const uint32_t> & IDs)
{
    if (!IDs.empty() && IDs.data())
    {
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

template <typename T>
static bool AreOptionalNullableEqual(const Optional<DataModel::Nullable<T>> & lhs, const Optional<DataModel::Nullable<T>> & rhs)
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

    // Both present and not null -> return false if equal
    return (lhs.Value().Value() == rhs.Value().Value());
}

template <typename T>
static bool AreOptionalEqual(const Optional<T> & lhs, const Optional<T> & rhs)
{
    if (lhs.HasValue() != rhs.HasValue())
        return false;
    if (!lhs.HasValue())
        return true;
    return (lhs.Value() == rhs.Value());
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
    if (!aValue.IsNull())
    {
        const PayloadType & newValue = aValue.Value();

        if (!newValue.tariffLabel.IsNull() && newValue.tariffLabel.Value().size() > kTariffInfoMaxLabelLength)
            return CHIP_ERROR_INVALID_ARGUMENT;

        if (!newValue.providerName.IsNull() && newValue.providerName.Value().size() > kTariffInfoMaxProviderLength)
            return CHIP_ERROR_INVALID_ARGUMENT;

        if (!newValue.blockMode.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(newValue.blockMode.Value()) != BlockModeEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        if (CommonUtilities::HasFeatureInCtx((TariffUpdateCtx *) mAuxData, CommodityTariff::Feature::kPricing))
        {
            if (newValue.currency.HasValue() && !newValue.currency.Value().IsNull())
            {
                const auto & currency = newValue.currency.Value().Value();
                if (currency.currency >= kMaxCurrencyValue)
                    return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                // If the pricing feature is enabled, the currency field is required!
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        // The tariff info is required, but not present.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

bool TariffInfoDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    if (!CommonUtilities::AreOptionalNullableEqual(source.currency, destination.currency))
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
        auto & tmp_label = aValue.providerName;
        MemoryFree(const_cast<char *>(tmp_label.Value().data()));
        tmp_label.SetNull();
    }

    aValue.currency.ClearValue();
}

// DayEntriesDataClass
namespace DayEntriesDataClass_Utils {

static CHIP_ERROR ValidateListEntry(const DayEntryStruct::Type & entryNewValue, TariffUpdateCtx * aCtx)
{
    VerifyOrReturnError_LogSend(entryNewValue.startTime < kDayEntryDurationLimit, CHIP_ERROR_INVALID_ARGUMENT,
                                "DayEntry startTime must be less 1499");

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!aValue.IsNull())
    {
        auto & newList                                = aValue.Value();
        std::unordered_set<uint32_t> & DayEntryKeyIDs = ((TariffUpdateCtx *) mAuxData)->DayEntryKeyIDs;

        VerifyOrReturnError_LogSend(newList.size() > 0 && newList.size() <= kDayEntriesAttrMaxLength,
                                    CHIP_ERROR_INVALID_LIST_LENGTH, "Incorrect DayEntries length");

        for (const auto & item : newList)
        {
            if (!DayEntryKeyIDs.insert(item.dayEntryID).second)
            {
                err = CHIP_ERROR_DUPLICATE_KEY_ID;
                break; // Duplicate found
            }

            err = DayEntriesDataClass_Utils::ValidateListEntry(item, (TariffUpdateCtx *) mAuxData);

            if (err != CHIP_NO_ERROR)
            {
                break;
            }
        }
    }
    else
    {
        // The DayEntries is required, but not present.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

bool DayEntriesDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    if (!CommonUtilities::AreOptionalEqual(source.duration, destination.duration))
    {
        return true;
    }

    if (!CommonUtilities::AreOptionalEqual(source.randomizationOffset, destination.randomizationOffset) ||
        !CommonUtilities::AreOptionalEqual(source.randomizationType, destination.randomizationType))
    {
        return true;
    }
    return false;
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

    VerifyOrReturnError_LogSend(entryNewValue.daysOfWeek.HasAny(), CHIP_ERROR_INVALID_ARGUMENT,
                                "The daysOfWeek must have least one bit set!");

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!aValue.IsNull())
    {
        auto & newList                                        = aValue.Value();
        std::unordered_set<uint32_t> & DayPatternKeyIDs       = ((TariffUpdateCtx *) mAuxData)->DayPatternKeyIDs;
        std::unordered_set<uint32_t> & DayPatternsDayEntryIDs = ((TariffUpdateCtx *) mAuxData)->DayPatternsDayEntryIDs;
        uint8_t tmpDoW                                        = 0;

        VerifyOrReturnError_LogSend((newList.size() > 0 && newList.size() <= kDayPatternsAttrMaxLength),
                                    CHIP_ERROR_INVALID_LIST_LENGTH, "Incorrect dayPatterns length");

        for (const auto & item : newList)
        {
            if (!DayPatternKeyIDs.insert(item.dayPatternID).second)
            {
                err = CHIP_ERROR_DUPLICATE_KEY_ID;
                break; // Duplicate found
            }

            tmpDoW |= item.daysOfWeek.Raw();

            err = DayPatternsDataClass_Utils::ValidateListEntry(item, DayPatternsDayEntryIDs);
            if (err != CHIP_NO_ERROR)
            {
                break;
            }
        }

        const bool isValidSingleRotatingDay = (!tmpDoW && newList.size() == 1); // Single rotating day pattern
        const bool isValidFullWeekCoverage  = (tmpDoW == kFullWeekMask);        // Complete week coverage

        if (!(isValidSingleRotatingDay || isValidFullWeekCoverage))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return err;
}

bool DayPatternsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    if ((source.dayPatternID != destination.dayPatternID) || (source.daysOfWeek.Raw() != destination.daysOfWeek.Raw()))
    {
        return true;
    }

    return (source.dayEntryIDs != destination.dayEntryIDs);
    ;
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
                ChipLogError(NotSpecified, "Current dayEntryID  already attached to the tariffComponentID");
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!aValue.IsNull())
    {
        auto & newList                                          = aValue.Value();
        std::unordered_set<uint32_t> & TariffPeriodsDayEntryIDs = ((TariffUpdateCtx *) mAuxData)->TariffPeriodsDayEntryIDs;
        std::unordered_set<uint32_t> & TariffPeriodsTariffComponentIDs =
            ((TariffUpdateCtx *) mAuxData)->TariffPeriodsTariffComponentIDs;

        std::map<uint32_t, std::unordered_set<uint32_t>> tariffComponentsMap;

        VerifyOrReturnError_LogSend((newList.size() > 0 && newList.size() <= kTariffPeriodsAttrMaxLength),
                                    CHIP_ERROR_INVALID_LIST_LENGTH, "Incorrect TariffPeriods length");

        for (const auto & item : newList)
        {
            err = TariffPeriodsDataClass_Utils::ValidateListEntry(item, TariffPeriodsDayEntryIDs, TariffPeriodsTariffComponentIDs,
                                                                  tariffComponentsMap);
            if (err != CHIP_NO_ERROR)
            {
                break;
            }
        }
    }
    else
    {
        // The TariffPeriods is required, but not present.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
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
        auto & tmp_label = aValue.label;
        MemoryFree(const_cast<char *>(tmp_label.Value().data()));
        tmp_label.SetNull();
    }
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
    CommonUtilities::CleanUpIDs(aValue.tariffComponentIDs);
}

// TariffComponentsDataClass
namespace TariffComponentsDataClass_Utils {
static CHIP_ERROR ValidateListEntry(const TariffComponentStruct::Type & entryNewValue, TariffUpdateCtx * aCtx)
{

    VerifyOrReturnError(entryNewValue.tariffComponentID > 0, CHIP_ERROR_INVALID_ARGUMENT);

    /*if (!entryNewValue.threshold.IsNull())
    {

    }*/

    // entryNewValue.label
    if (entryNewValue.label.HasValue() && !entryNewValue.label.Value().IsNull())
    {
        VerifyOrReturnError(entryNewValue.label.Value().Value().size() <= kTariffComponentMaxLabelLength,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    // If PRICE feature are supported
    if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPricing))
    {
        if (entryNewValue.price.HasValue() && !entryNewValue.price.Value().IsNull())
        {
            const auto & price = entryNewValue.price.Value().Value();
            VerifyOrReturnError(EnsureKnownEnumValue(price.priceType) != TariffPriceTypeEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            // TODO - maybe we should implement deeper validation for these fields:
            // if (price.price.HasValue() && price.price.Value() == 0)
            //    return CHIP_ERROR_INVALID_ARGUMENT;
            // if (price.priceLevel.HasValue() && price.priceLevel.Value() == 0)
            //    return CHIP_ERROR_INVALID_ARGUMENT;
        }
        else
        {
            // If the pricing feature is enabled, the price field is required!
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    // If FCRED feature are supported
    if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kFriendlyCredit))
    {
        if (!entryNewValue.friendlyCredit.HasValue())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    // If AUXLD feature are supported
    if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kAuxiliaryLoad))
    {
        if (entryNewValue.auxiliaryLoad.HasValue())
        {
            const auto & auxiliaryLoad = entryNewValue.auxiliaryLoad.Value();

            /*VerifyOrReturnError(auxiliaryLoadList.size() <= kAuxSwitchesSettingsMax,
                CHIP_ERROR_INVALID_ARGUMENT);*/

            VerifyOrReturnError(EnsureKnownEnumValue(auxiliaryLoad.requiredState) != AuxiliaryLoadSettingEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            // If the AUXLD feature is enabled, the auxiliaryLoad field is required!
        }
    }

    // If PEAKP feature are supported
    if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPeakPeriod))
    {
        if (entryNewValue.peakPeriod.HasValue())
        {
            const auto & peakPeriod = entryNewValue.peakPeriod.Value();

            VerifyOrReturnError(EnsureKnownEnumValue(peakPeriod.severity) != PeakPeriodSeverityEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);

            VerifyOrReturnError(peakPeriod.peakPeriod > 0, CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            // If the PEAKP feature is enabled, the peakPeriod field is required!
        }
    }

    // If PWRTHLD feature are supported
    if (CommonUtilities::HasFeatureInCtx(aCtx, CommodityTariff::Feature::kPowerThreshold))
    {
        if (entryNewValue.powerThreshold.HasValue())
        {
            const auto & powerThreshold = entryNewValue.powerThreshold.Value();

            if (!powerThreshold.powerThresholdSource.IsNull())
            {
                VerifyOrReturnError(EnsureKnownEnumValue(powerThreshold.powerThresholdSource.Value()) !=
                                        PowerThresholdSourceEnum::kUnknownEnumValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }

            // TODO - maybe we should implement deeper validation for these fields:
            if (powerThreshold.powerThreshold.HasValue() && powerThreshold.powerThreshold.Value() == 0)
                return CHIP_ERROR_INVALID_ARGUMENT;
            if (powerThreshold.apparentPowerThreshold.HasValue() && powerThreshold.apparentPowerThreshold.Value() == 0)
                return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    /*if (entryNewValue.predicted.HasValue())
    {

    }*/
    return CHIP_NO_ERROR;
}
} // namespace TariffComponentsDataClass_Utils

CHIP_ERROR TariffComponentsDataClass::Validate(const ValueType & aValue) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!aValue.IsNull())
    {
        auto & newList                                       = aValue.Value();
        std::unordered_set<uint32_t> & TariffComponentKeyIDs = ((TariffUpdateCtx *) mAuxData)->TariffComponentKeyIDs;

        VerifyOrReturnError((newList.size() > 0 && newList.size() <= kTariffComponentsAttrMaxLength),
                            CHIP_ERROR_INVALID_LIST_LENGTH);

        for (const auto & item : newList)
        {
            if (!TariffComponentKeyIDs.insert(item.tariffComponentID).second)
            {
                err = CHIP_ERROR_DUPLICATE_KEY_ID;
                break; // Duplicate found
            }

            err = TariffComponentsDataClass_Utils::ValidateListEntry(item, (TariffUpdateCtx *) mAuxData);
            if (err != CHIP_NO_ERROR)
            {
                break;
            }
        }
    }
    else
    {
        // The TariffComponents is required, but not present.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

bool TariffComponentsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{

    if (!CommonUtilities::AreOptionalEqual(source.label, destination.label))
        return true;

    // If PRICE feature are supported
    if (!CommonUtilities::AreOptionalNullableEqual(source.price, destination.price))
    {
        return true;
    }

    // If FCRED feature are supported
    if (!CommonUtilities::AreOptionalEqual(source.friendlyCredit, destination.friendlyCredit))
    {
        return true;
    }

    // If AUXLD feature are supported
    if (!CommonUtilities::AreOptionalEqual(source.auxiliaryLoad, destination.auxiliaryLoad))
    {
        return true;
    }

    // If PEAKP feature are supported
    if (!CommonUtilities::AreOptionalEqual(source.peakPeriod, destination.peakPeriod))
    {
        return true;
    }

    // If PWRTHLD feature are supported
    if (!CommonUtilities::AreOptionalEqual(source.powerThreshold, destination.powerThreshold))
    {
        return true;
    }

    if (!CommonUtilities::AreOptionalEqual(source.predicted, destination.predicted))
    {
        return true;
    }

    return !(source.threshold == destination.threshold);
}

void TariffComponentsDataClass::CleanupStructValue(PayloadType & aValue)
{

    if (aValue.label.HasValue() && !aValue.label.Value().IsNull())
    {
        auto & tmp_label = aValue.label.Value();
        MemoryFree(const_cast<char *>(tmp_label.Value().data()));
        // tmp_label.SetNull();
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!aValue.IsNull())
    {
        auto & newList                                           = aValue.Value();
        uint32_t tmpDate                                         = 0;
        std::unordered_set<uint32_t> & IndividualDaysDayEntryIDs = ((TariffUpdateCtx *) mAuxData)->IndividualDaysDayEntryIDs;

        VerifyOrReturnError_LogSend((newList.size() > 0 && newList.size() <= kIndividualDaysAttrMaxLength),
                                    CHIP_ERROR_INVALID_LIST_LENGTH, "Incorrect IndividualDays length");

        for (const auto & item : newList)
        {
            VerifyOrReturnError_LogSend(item.date > tmpDate, CHIP_ERROR_INVALID_ARGUMENT,
                                        "IndividualDays must be order by startTime");
            VerifyOrReturnError(EnsureKnownEnumValue(item.dayType) != DayTypeEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

            if (item.dayEntryIDs.empty() || item.dayEntryIDs.size() > kDayStructItemMaxDayEntryIDs)
                return CHIP_ERROR_INVALID_ARGUMENT;

            if (CommonUtilities::HasDuplicateIDs(item.dayEntryIDs, IndividualDaysDayEntryIDs))
            {
                err = CHIP_ERROR_DUPLICATE_KEY_ID;
                break;
            }
            tmpDate = item.date;
        }
    }

    return err;
}

bool IndividualDaysDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    return (source.date != destination.date || source.dayType != destination.dayType ||
            source.dayEntryIDs != destination.dayEntryIDs);
}

void IndividualDaysDataClass::CleanupStructValue(PayloadType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

// CalendarPeriodsDataClass

CHIP_ERROR CalendarPeriodsDataClass::Validate(const ValueType & aValue) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If calendar is unavailable, the attribute should be null
    if (!aValue.IsNull())
    {
        auto & newList   = aValue.Value();
        bool isFirstItem = true;
        Nullable<uint32_t> previousStartDate;
        std::unordered_set<uint32_t> & CalendarPeriodsDayPatternIDs = ((TariffUpdateCtx *) mAuxData)->CalendarPeriodsDayPatternIDs;

        for (const auto & item : newList)
        {
            // Validate dayPatternIDs
            VerifyOrReturnError_LogSend(
                !item.dayPatternIDs.empty() && item.dayPatternIDs.size() <= kCalendarPeriodItemMaxDayPatternIDs,
                CHIP_ERROR_INVALID_ARGUMENT, "DayPatternIDs count must be between 1 and %zu", kCalendarPeriodItemMaxDayPatternIDs);

            // Check for duplicate dayPatternIDs
            VerifyOrReturnError_LogSend(!CommonUtilities::HasDuplicateIDs(item.dayPatternIDs, CalendarPeriodsDayPatternIDs),
                                        CHIP_ERROR_DUPLICATE_KEY_ID, "Duplicate dayPatternID found in CalendarPeriods");

            // Special handling for first item
            if (isFirstItem)
            {
                isFirstItem = false;
                // First item can have null StartDate
                if (item.startDate.IsNull())
                {
                    previousStartDate = item.startDate;
                    continue;
                }
            }
            else
            {
                // Subsequent items must have non-null StartDate
                VerifyOrReturnError_LogSend(!item.startDate.IsNull(), CHIP_ERROR_INVALID_ARGUMENT,
                                            "Only first CalendarPeriodStruct can have null StartDate");
            }

            // Validate StartDate ordering
            if (!previousStartDate.IsNull() && !item.startDate.IsNull())
            {
                VerifyOrReturnError_LogSend(item.startDate.Value() > previousStartDate.Value(), CHIP_ERROR_INVALID_ARGUMENT,
                                            "CalendarPeriodStructs must be in increasing StartDate order");
            }

            previousStartDate = item.startDate;
        }
    }

    return err;
}

bool CalendarPeriodsDataClass::CompareStructValue(const PayloadType & source, const PayloadType & destination) const
{
    return (source.startDate != destination.startDate || source.dayPatternIDs != destination.dayPatternIDs);
}

void CalendarPeriodsDataClass::CleanupStructValue(PayloadType & aValue)
{
    CommonUtilities::CleanUpIDs(aValue.dayPatternIDs);
}
