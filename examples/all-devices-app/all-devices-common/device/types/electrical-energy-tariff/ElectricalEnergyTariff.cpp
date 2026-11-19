/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/electrical-energy-tariff/ElectricalEnergyTariff.h>

#include <devices/Types.h>
#include <lib/support/CodeUtils.h>

namespace chip::app {

namespace {

using SemanticTag = EndpointComposition::SemanticTag;

constexpr uint8_t kCommodityTariffChronologyNamespaceId =
    to_underlying(Clusters::Globals::SemanticTagNamespaceId::kCommodityTariffChronology);

bool ContainsStandardTag(Span<const SemanticTag> tagList, uint8_t namespaceId, uint8_t tag)
{
    for (const auto & entry : tagList)
    {
        if (entry.mfgCode.IsNull() && entry.namespaceID == namespaceId && entry.tag == tag)
        {
            return true;
        }
    }
    return false;
}

} // namespace

Span<const SemanticTag> ElectricalEnergyTariff::DefaultTagList()
{
    static const SemanticTag kTags[] = {
        {
            .mfgCode     = DataModel::NullNullable,
            .namespaceID = kCommodityTariffCommodityNamespaceId,
            .tag         = kCommodityTariffCommodityElectricalEnergyTag,
        },
    };
    return Span(kTags);
}

Span<const SemanticTag> ElectricalEnergyTariff::ActiveTariffTagList()
{
    static const SemanticTag kTags[] = {
        {
            .mfgCode     = DataModel::NullNullable,
            .namespaceID = kCommodityTariffChronologyNamespaceId,
            .tag         = kCommodityTariffChronologyCurrentTag,
        },
        {
            .mfgCode     = DataModel::NullNullable,
            .namespaceID = kCommodityTariffCommodityNamespaceId,
            .tag         = kCommodityTariffCommodityElectricalEnergyTag,
        },
    };
    return Span(kTags);
}

ElectricalEnergyTariff::ElectricalEnergyTariff(Span<const SemanticTag> tagList) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kElectricalEnergyTariff, 1)), mTagList(tagList)
{}

CHIP_ERROR ElectricalEnergyTariff::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                            EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    if (composition.tagList.empty())
    {
        composition.tagList = mTagList;
    }
    ReturnErrorOnFailure(ValidateTagList(composition.tagList));

    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));
    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ElectricalEnergyTariff::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    UnregisterOptionalClusters(provider);
}

CHIP_ERROR ElectricalEnergyTariff::ValidateTagList(Span<const SemanticTag> tagList) const
{
    VerifyOrReturnError(
        ContainsStandardTag(tagList, kCommodityTariffCommodityNamespaceId, kCommodityTariffCommodityElectricalEnergyTag),
        CHIP_ERROR_INVALID_ARGUMENT);
    if (IsActiveTariff())
    {
        VerifyOrReturnError(
            ContainsStandardTag(tagList, kCommodityTariffChronologyNamespaceId, kCommodityTariffChronologyCurrentTag),
            CHIP_ERROR_INVALID_ARGUMENT);
    }
    return CHIP_NO_ERROR;
}

} // namespace chip::app
