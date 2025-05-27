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

using namespace chip;
using namespace chip::app;
using namespace chip::Platform;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;

static constexpr size_t kDefaultStringValuesMaxBufLength = 128u;
static constexpr size_t kDefaultListAttrMaxLength = 128u;
constexpr uint16_t kMaxCurrencyValue = 999; // From spec

static constexpr size_t kTariffInfoMaxLabelLength = kDefaultStringValuesMaxBufLength;
static constexpr size_t kTariffInfoMaxProviderLength = kDefaultStringValuesMaxBufLength;
static constexpr size_t kTariffComponentMaxLabelLength = kDefaultStringValuesMaxBufLength;

static constexpr size_t kDayEntriesAttrMaxLength = kDefaultListAttrMaxLength;
static constexpr size_t kDayPatternsAttrMaxLength = kDefaultListAttrMaxLength;
static constexpr size_t kTariffComponentsAttrMaxLength = kDefaultListAttrMaxLength;
static constexpr size_t kTariffPeriodsAttrMaxLength = kDefaultListAttrMaxLength;

//static constexpr size_t kCalendarPeriodsAttrMaxLength = 4;
//static constexpr size_t kIndividualDaysAttrMaxLength = 50;

//static constexpr size_t kCalendarPeriodItemMaxDayPatternIDs = 7;
//static constexpr size_t kDayStructItemMaxDayEntryIDs = 96;
//static constexpr size_t kDayPatternItemMaxDayEntryIDs = kDayStructItemMaxDayEntryIDs;
static constexpr size_t kTariffPeriodItemMaxIDs = 20;

//static constexpr size_t kAuxSwitchesSettingsMax = 8;

namespace CommonUtilities {
    static void CleanUpIDs(DataModel::List<const uint32_t> & IDs)
    {
        if (!IDs.empty() && IDs.data()) {
            MemoryFree(const_cast<uint32_t*>(IDs.data()));
            IDs = DataModel::List<const uint32_t>();
        }
    }

    static bool HasDuplicateIDs(const DataModel::List<const uint32_t>& IDs,
        std::unordered_set<uint32_t>& seen)
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
};

// TariffInfoDataClass

CHIP_ERROR TariffInfoDataClass::Validate(const ValueType & aValue) const {

    if (!aValue.IsNull())
    {
        const PayloadType& newValue = aValue.Value();

        if (!newValue.tariffLabel.IsNull() && newValue.tariffLabel.Value().size() > kTariffInfoMaxLabelLength)
            return CHIP_ERROR_INVALID_ARGUMENT;

        if (!newValue.providerName.IsNull() && newValue.providerName.Value().size() > kTariffInfoMaxProviderLength)
            return CHIP_ERROR_INVALID_ARGUMENT;

        if (!newValue.blockMode.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(newValue.blockMode.Value()) !=
                BlockModeEnum::kUnknownEnumValue,
                CHIP_ERROR_INVALID_ARGUMENT);
        }

        if (mFeatureMap.Has(CommodityTariff::Feature::kPricing))
        {
            if (newValue.currency.HasValue() && !newValue.currency.Value().IsNull())
            {
                const auto &currency = newValue.currency.Value().Value();
                if (currency.currency >= kMaxCurrencyValue) return CHIP_ERROR_INVALID_ARGUMENT;
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

bool TariffInfoDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    if (source.tariffLabel.IsNull() != destination.tariffLabel.IsNull())
        return true;
    if (!source.tariffLabel.IsNull() && source.tariffLabel.Value() != destination.tariffLabel.Value())
        return true;

    if (source.providerName.IsNull() != destination.providerName.IsNull())
        return true;
    if (!source.providerName.IsNull() && source.providerName.Value() != destination.providerName.Value())
        return true;

    if (mFeatureMap.Has(CommodityTariff::Feature::kPricing))
    {
        if (source.currency.HasValue() != destination.currency.HasValue()) return true;
        if (source.currency.HasValue()) {
            if (source.currency.Value().IsNull() != destination.currency.Value().IsNull()) return true;
            if (!source.currency.Value().IsNull() && (source.currency.Value().Value() != destination.currency.Value().Value()))
                return true;
        }
    }

    if (source.blockMode.IsNull() != destination.blockMode.IsNull())
        return true;

    return source.blockMode.IsNull() || source.blockMode.Value() != destination.blockMode.Value();
}

void TariffInfoDataClass::CleanupStructValue(PayloadType& aValue)
{
    if (!aValue.tariffLabel.IsNull() && aValue.tariffLabel.Value().data())
    {
        MemoryFree(const_cast<char*>(aValue.tariffLabel.Value().data()));
        aValue.tariffLabel.SetNull();
    }

    if (!aValue.providerName.IsNull() && aValue.providerName.Value().data())
    {
        auto& tmp_label = aValue.providerName;
        MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        tmp_label.SetNull();
    }

    if (mFeatureMap.Has(CommodityTariff::Feature::kPricing))
    {
        aValue.currency.ClearValue();
    }
}

// DayPatternsDataClass
namespace DayPatternsDataClass_Utils {
    static CHIP_ERROR ValidateListEntry(const DayPatternStruct::Type& newValue,
                                        std::unordered_set<uint32_t>& seenDeIDs) {

        VerifyOrReturnError(newValue.daysOfWeek.HasAny(),
            CHIP_ERROR_INVALID_ARGUMENT);

        // Check that the current day pattern item has no duplicated dayEntryIDs
        if (CommonUtilities::HasDuplicateIDs(newValue.dayEntryIDs, seenDeIDs))
        {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }

        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR DayPatternsDataClass::Validate(const ValueType & aValue) const {
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto & newList = aValue;
    std::unordered_set<uint32_t> KeyIDs;
    std::unordered_set<uint32_t> seenDeIDs;

    VerifyOrReturnError( (newList.size() > 0 && newList.size() <= kDayPatternsAttrMaxLength), 
        CHIP_ERROR_INVALID_LIST_LENGTH);

    for (const auto& item : newList) {
        if (!KeyIDs.insert(item.dayPatternID).second)
        {
            err = CHIP_ERROR_DUPLICATE_KEY_ID;
            break; // Duplicate found
        }
        
        err = DayPatternsDataClass_Utils::ValidateListEntry(item, seenDeIDs);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
    }
    return err;
}

bool DayPatternsDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    return true;
}

void DayPatternsDataClass::CleanupStructValue(PayloadType& aValue) {
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}


//DayEntriesDataClass
namespace DayEntriesDataClass_Utils {
    static CHIP_ERROR ValidateListEntry(const DayEntryStruct::Type& newValue) {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR DayEntriesDataClass::Validate(const ValueType & aValue) const {
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto & newList = aValue;
    std::unordered_set<uint32_t> KeyIDs;

    VerifyOrReturnError( (newList.size() > 0 && newList.size() <= kDayEntriesAttrMaxLength), 
        CHIP_ERROR_INVALID_LIST_LENGTH);

    for (const auto& item : newList) {
        if (!KeyIDs.insert(item.dayEntryID).second)
        {
            err = CHIP_ERROR_DUPLICATE_KEY_ID;
            break; // Duplicate found
        }

        err = DayEntriesDataClass_Utils::ValidateListEntry(item);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
    }
    return err;
}

bool DayEntriesDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    return true;
}

void DayEntriesDataClass::CleanupStructValue(PayloadType& aValue) {
    //If RNDM feature are supported
    aValue.randomizationOffset.ClearValue();
    aValue.randomizationType.ClearValue();

    aValue.duration.ClearValue();
}

//TariffPeriodsDataClass

namespace TariffPeriodsDataClass_Utils {
    static CHIP_ERROR ValidateListEntry(const TariffPeriodStruct::Type& newValue,
                                        std::unordered_set<uint32_t>& seenDeIDs,
                                        std::unordered_set<uint32_t>& seenTcIDs) {
        if (!newValue.label.IsNull() && newValue.label.Value().size() > kDefaultStringValuesMaxBufLength)
            return CHIP_ERROR_INVALID_ARGUMENT;
        else if (newValue.label.Value().empty())
        {
            ChipLogError(NotSpecified, "TariffPeriod label must not be empty if present");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    
        if (newValue.dayEntryIDs.empty() || newValue.dayEntryIDs.size() > kTariffPeriodItemMaxIDs)
            return CHIP_ERROR_INVALID_ARGUMENT;
    
        if (newValue.tariffComponentIDs.empty()|| newValue.tariffComponentIDs.size() > kTariffPeriodItemMaxIDs)
            return CHIP_ERROR_INVALID_ARGUMENT;
    
        // Check that the current period item has no duplicated dayEntryIDs
        if (CommonUtilities::HasDuplicateIDs(newValue.dayEntryIDs, seenDeIDs))
        {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
    
        // Check that the current period item has no duplicated dayEntryIDs
        if (CommonUtilities::HasDuplicateIDs(newValue.tariffComponentIDs, seenTcIDs))
        {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
    
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR TariffPeriodsDataClass::Validate(const ValueType & aValue)  const {
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto & newList = aValue;
    std::unordered_set<uint32_t> seenDeIDs;
    std::unordered_set<uint32_t> seenTcIDs;

    VerifyOrReturnError( (newList.size() > 0 && newList.size() <= kTariffPeriodsAttrMaxLength), 
        CHIP_ERROR_INVALID_LIST_LENGTH);

    for (const auto& item : newList) {
        err = TariffPeriodsDataClass_Utils::ValidateListEntry(item, seenDeIDs, seenTcIDs);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
    }
    return err;
}

bool TariffPeriodsDataClass::CompareStructValue(const PayloadType &a, const PayloadType &b) const {
    if (a.label.IsNull() != b.label.IsNull()) return true;
    if (!a.label.IsNull() && a.label.Value() != b.label.Value()) return true;
    if (a.dayEntryIDs != b.dayEntryIDs) return true;
    return (a.tariffComponentIDs != b.tariffComponentIDs); //Both structs are equal
}

void TariffPeriodsDataClass::CleanupStructValue(PayloadType& aValue) {
    if (!aValue.label.IsNull() && aValue.label.Value().data()) {
        auto& tmp_label = aValue.label;
        MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        tmp_label.SetNull();
    }
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
    CommonUtilities::CleanUpIDs(aValue.tariffComponentIDs);
}

// TariffComponentsDataClass
namespace TariffComponentsDataClass_Utils {
    static CHIP_ERROR ValidateListEntry(const TariffComponentStruct::Type& newValue, const BitMask<Feature>& aFeatureMap) {

    VerifyOrReturnError(newValue.tariffComponentID > 0,
        CHIP_ERROR_INVALID_ARGUMENT);

    /*if (!newValue.threshold.IsNull())
    {

    }*/

    //newValue.label
    if (newValue.label.HasValue() && !newValue.label.Value().IsNull())
    {
        VerifyOrReturnError(newValue.label.Value().Value().size() > kTariffComponentMaxLabelLength,
            CHIP_ERROR_INVALID_ARGUMENT);
    }
    

    //If PRICE feature are supported
    if (aFeatureMap.Has(CommodityTariff::Feature::kPricing) &&
        newValue.price.HasValue() && !newValue.price.Value().IsNull())
    {
        const auto & price = newValue.price.Value().Value();
        VerifyOrReturnError(EnsureKnownEnumValue(price.priceType) !=
            TariffPriceTypeEnum::kUnknownEnumValue,
            CHIP_ERROR_INVALID_ARGUMENT);
        //TODO - maybe we should implement deeper validation for these fields:
        if (price.price.HasValue() && price.price.Value() == 0) return CHIP_ERROR_INVALID_ARGUMENT;
        if (price.priceLevel.HasValue() && price.priceLevel.Value() == 0) return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        // If the pricing feature is enabled, the price field is required!
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    //If FCRED feature are supported
    if (aFeatureMap.Has(CommodityTariff::Feature::kFriendlyCredit) &&
        !newValue.friendlyCredit.HasValue())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }


    //If AUXLD feature are supported
    if (aFeatureMap.Has(CommodityTariff::Feature::kAuxiliaryLoad) &&
        newValue.auxiliaryLoad.HasValue())
    {
        const auto & auxiliaryLoad = newValue.auxiliaryLoad.Value();

        /*VerifyOrReturnError(auxiliaryLoadList.size() <= kAuxSwitchesSettingsMax,
            CHIP_ERROR_INVALID_ARGUMENT);*/

        VerifyOrReturnError(EnsureKnownEnumValue(auxiliaryLoad.requiredState) !=
            AuxiliaryLoadSettingEnum::kUnknownEnumValue,
            CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        // If the AUXLD feature is enabled, the price field is required!
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    //If PEAKP feature are supported
    if (aFeatureMap.Has(CommodityTariff::Feature::kPeakPeriod) &&
        newValue.peakPeriod.HasValue())
    {
        const auto & peakPeriod = newValue.peakPeriod.Value();

        VerifyOrReturnError(EnsureKnownEnumValue(peakPeriod.severity) !=
            PeakPeriodSeverityEnum::kUnknownEnumValue,
            CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(peakPeriod.peakPeriod > 0, CHIP_ERROR_INVALID_ARGUMENT);
    }

    //If PWRTHLD feature are supported
    if (aFeatureMap.Has(CommodityTariff::Feature::kPowerThreshold) &&
        newValue.powerThreshold.HasValue())
    {
        const auto & powerThreshold = newValue.powerThreshold.Value();

        if (!powerThreshold.powerThresholdSource.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(powerThreshold.powerThresholdSource.Value()) !=
                PowerThresholdSourceEnum::kUnknownEnumValue,
                CHIP_ERROR_INVALID_ARGUMENT);
        }

        //TODO - maybe we should implement deeper validation for these fields:
        if (powerThreshold.powerThreshold.HasValue() && powerThreshold.powerThreshold.Value() == 0) return CHIP_ERROR_INVALID_ARGUMENT;
        if (powerThreshold.apparentPowerThreshold.HasValue() && powerThreshold.apparentPowerThreshold.Value() == 0) return CHIP_ERROR_INVALID_ARGUMENT;
    }

    /*if (newValue.predicted.HasValue())
    {

    }*/
        return  CHIP_NO_ERROR;
    }
}

CHIP_ERROR TariffComponentsDataClass::Validate(const ValueType & aValue) const {
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto & newList = aValue;
    std::unordered_set<uint32_t> KeyIDs;

    VerifyOrReturnError( (newList.size() > 0 && newList.size() <= kTariffComponentsAttrMaxLength), 
        CHIP_ERROR_INVALID_LIST_LENGTH);

    for (const auto& item : newList) {
        if (!KeyIDs.insert(item.tariffComponentID).second)
        {
            err = CHIP_ERROR_DUPLICATE_KEY_ID;
            break; // Duplicate found
        }

        err = TariffComponentsDataClass_Utils::ValidateListEntry(item, mFeatureMap);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
    }

    return err;
}

bool TariffComponentsDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    //If PRICE feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kPricing))
    {
        //aValue.price.ClearValue();        
    }


    //If FCRED feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kFriendlyCredit))
    {
        //aValue.friendlyCredit.ClearValue();
    }


    //If AUXLD feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kAuxiliaryLoad))
    {
        //aValue.auxiliaryLoad.ClearValue();
    }

    //If PEAKP feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kPeakPeriod))
    {
        //aValue.peakPeriod.ClearValue();
    }

    //If PWRTHLD feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kPowerThreshold))
    {
        //aValue.powerThreshold.ClearValue();        
    }

    return true;
}

void TariffComponentsDataClass::CleanupStructValue(PayloadType& aValue) {

    if (aValue.label.HasValue() && !aValue.label.Value().IsNull()) {
        auto& tmp_label = aValue.label.Value();
        MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        //tmp_label.SetNull();
        aValue.label.ClearValue();
    }

    //If PRICE feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kPricing))
    {
        aValue.price.ClearValue();        
    }


    //If FCRED feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kFriendlyCredit))
    {
        aValue.friendlyCredit.ClearValue();
    }


    //If AUXLD feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kAuxiliaryLoad))
    {
        aValue.auxiliaryLoad.ClearValue();
    }

    //If PEAKP feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kPeakPeriod))
    {
        aValue.peakPeriod.ClearValue();
    }

    //If PWRTHLD feature are supported
    if (mFeatureMap.Has(CommodityTariff::Feature::kPowerThreshold))
    {
        aValue.powerThreshold.ClearValue();        
    }
}

//IndividualDaysDataClass

CHIP_ERROR IndividualDaysDataClass::Validate(const ValueType & aValue) const {

    return CHIP_NO_ERROR;
}


bool IndividualDaysDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    return true;
}

void IndividualDaysDataClass::CleanupStructValue(PayloadType& aValue) {
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

//CalendarPeriodsDataClass

CHIP_ERROR CalendarPeriodsDataClass::Validate(const ValueType & aValue) const {

    return CHIP_NO_ERROR;
}


bool CalendarPeriodsDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    return true;
}

void CalendarPeriodsDataClass::CleanupStructValue(PayloadType& aValue) {
    CommonUtilities::CleanUpIDs(aValue.dayPatternIDs);
}
