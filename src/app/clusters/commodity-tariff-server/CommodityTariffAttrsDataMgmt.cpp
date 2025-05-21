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
constexpr uint16_t kMaxCurrencyValue = 999; // From spec

static constexpr size_t kTariffInfoMaxLabelLength = kDefaultStringValuesMaxBufLength;
static constexpr size_t kTariffInfoMaxProviderLength = kDefaultStringValuesMaxBufLength;

namespace CommonUtilities {
    static void CleanUpIDs(DataModel::List<const uint32_t> & IDs)
    {
        if (!IDs.empty() && IDs.data()) {
            MemoryFree(const_cast<uint32_t*>(IDs.data()));
            IDs = DataModel::List<const uint32_t>();
        }
    }
};

// TariffInfoDataClass

constexpr bool operator!=(const Globals::Structs::CurrencyStruct::Type & lhs, const Globals::Structs::CurrencyStruct::Type & rhs)
{
    return ((lhs.currency != rhs.currency) || (lhs.decimalPoints != rhs.decimalPoints));
}

CHIP_ERROR TariffInfoDataClass::ValidateStructValue(const PayloadType& newValue) const {
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

    //If PRICE feature are supported
    if (newValue.currency.HasValue() && !newValue.currency.Value().IsNull()) {
        const auto &currency = newValue.currency.Value().Value();
        if (currency.currency >= kMaxCurrencyValue) return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void TariffInfoDataClass::CleanupStructValue(PayloadType& aValue) {
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

    //If PRICE feature are supported
    aValue.currency.ClearValue();
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
