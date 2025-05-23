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

//static constexpr size_t kDayEntriesAttrMaxLength = kDefaultListAttrMaxLength;
//static constexpr size_t kDayPatternsAttrMaxLength = kDefaultListAttrMaxLength;
//static constexpr size_t kTariffComponentsAttrMaxLength = kDefaultListAttrMaxLength;
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

    static bool HasDuplicateIDs(const DataModel::List<const uint32_t>& IDs)
    {
        std::unordered_set<uint32_t> seen;
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

CHIP_ERROR TariffInfoDataClass::ValidateStructValue(const PayloadType& newValue, bool is_first_item, const WrappedType * owner) const {
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

    return CHIP_NO_ERROR;
}

bool TariffInfoDataClass::CompareStructValue(const PayloadType &source, const PayloadType &destination) const {
    if (source.tariffLabel.IsNull() != destination.tariffLabel.IsNull())
        return false;
    if (!source.tariffLabel.IsNull() && source.tariffLabel.Value() != destination.tariffLabel.Value())
        return false;

    if (source.providerName.IsNull() != destination.providerName.IsNull())
        return false;
    if (!source.providerName.IsNull() && source.providerName.Value() != destination.providerName.Value())
        return false;

    if (mFeatureMap.Has(CommodityTariff::Feature::kPricing))
    {
        if (source.currency.HasValue() != destination.currency.HasValue()) return false;
        if (source.currency.HasValue()) {
            if (source.currency.Value().IsNull() != destination.currency.Value().IsNull()) return false;
            if (!source.currency.Value().IsNull() && (source.currency.Value().Value() != destination.currency.Value().Value()))
                return false;
        }
    }

    if (source.blockMode.IsNull() != destination.blockMode.IsNull())
        return false;

    return source.blockMode.IsNull() || source.blockMode.Value() == destination.blockMode.Value();
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

//TariffPeriodsDataClass

CHIP_ERROR TariffPeriodsDataClass::ValidateStructValue(const PayloadType& newValue, bool is_first_item, const WrappedType * owner) const {
    assert(owner); // Here the new value is list item element and the owning list must be present to check for duplicate elements

    if (is_first_item) // If lists case tis flag indicates first item of list
    {
        std::unordered_set<uint32_t> usedDayEntryIds;
    
        if (owner->size() > kTariffPeriodsAttrMaxLength)
        {
            return CHIP_ERROR_INVALID_LIST_LENGTH;
        }

        /*if (owner->size() > 1)
        {
            for(auto item : *owner)
            {
                auto next = std::next(item);

                if (next)
                {
                    if (!item.label.Value().empty())
                    {
                        break;
                    }  
                }
            }
        }*/
    }

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
    if (CommonUtilities::HasDuplicateIDs(newValue.dayEntryIDs))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check that the current period item has no duplicated dayEntryIDs
    if (CommonUtilities::HasDuplicateIDs(newValue.tariffComponentIDs))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}
/*
bool TariffPeriodsDataClass::CompareStructValue(const PayloadType &a, const PayloadType &b) const {
    if (a.label.IsNull() != b.label.IsNull()) return false;
    if (!a.label.IsNull() && a.label.Value() != b.label.Value()) return false;
    return a.dayEntryIDs == b.dayEntryIDs && a.tariffComponentIDs == b.tariffComponentIDs;
}
*/
void TariffPeriodsDataClass::CleanupStructValue(PayloadType& aValue) {
    if (!aValue.label.IsNull() && aValue.label.Value().data()) {
        auto& tmp_label = aValue.label;
        MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        tmp_label.SetNull();
    }
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
    CommonUtilities::CleanUpIDs(aValue.tariffComponentIDs);
}

void TariffComponentsDataClass::CleanupStructValue(PayloadType& aValue) {

    if (aValue.label.HasValue() && !aValue.label.Value().IsNull()) {
        auto& tmp_label = aValue.label.Value();
        MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        //tmp_label.SetNull();
        aValue.label.ClearValue();
    }

    //If PRICE feature are supported
    aValue.price.ClearValue();

    //If FCRED feature are supported
    aValue.friendlyCredit.ClearValue();

    //If AUXLD feature are supported
    aValue.auxiliaryLoad.ClearValue();

    //If PEAKP feature are supported
    aValue.peakPeriod.ClearValue();

    //If PWRTHLD feature are supported
    aValue.powerThreshold.ClearValue();
}

void DayEntriesDataClass::CleanupStructValue(PayloadType& aValue) {
    //If RNDM feature are supported
    aValue.randomizationOffset.ClearValue();
    aValue.randomizationType.ClearValue();

    aValue.duration.ClearValue();
}

void IndividualDaysDataClass::CleanupStructValue(PayloadType& aValue) {
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}

void CalendarPeriodsDataClass::CleanupStructValue(PayloadType& aValue) {
    CommonUtilities::CleanUpIDs(aValue.dayPatternIDs);
}

void DayPatternsDataClass::CleanupStructValue(PayloadType& aValue) {
    CommonUtilities::CleanUpIDs(aValue.dayEntryIDs);
}
