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

#pragma once

#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>

namespace chip::app {

/**
 * Electrical Energy Tariff device type (0x0513).
 *
 * Cluster requirements:
 *   - Commodity Price ([ActiveTariff].a+) and Commodity Tariff (O.a+) form a choice group: every
 *     endpoint SHALL have at least one of them. Commodity Price is only allowed on the active tariff.
 *   - Electrical Grid Conditions is optional.
 * Subclasses register the clusters they support through RegisterOptionalClusters().
 *
 * Descriptor TagList requirements, checked by Register():
 *   - The ElectricalEnergy tag (Commodity Tariff Commodity namespace) is mandatory.
 *   - The Current tag (Commodity Tariff Chronology namespace) is mandatory when IsActiveTariff()
 *     returns true.
 */
class ElectricalEnergyTariff : public SingleEndpoint
{
public:
    /// Semantic tag values from the Commodity Tariff namespaces of the Matter specification that
    /// are not yet generated into Clusters::Globals. The Commodity Tariff Chronology namespace ID
    /// is available as Clusters::Globals::SemanticTagNamespaceId::kCommodityTariffChronology.
    // TODO: Replace these constants with the generated Clusters::Globals enums once
    //       semantic-tag-namespace-enums.xml matches the current spec (0x0D is the Commodity Tariff
    //       Commodity namespace, and the Commodity Tariff tag enums are missing).
    static constexpr uint8_t kCommodityTariffChronologyCurrentTag         = 0x00;
    static constexpr uint8_t kCommodityTariffCommodityNamespaceId         = 0x0D;
    static constexpr uint8_t kCommodityTariffCommodityElectricalEnergyTag = 0x00;

    /// Minimal TagList for a tariff that is not the active tariff: ElectricalEnergy.
    static Span<const EndpointComposition::SemanticTag> DefaultTagList();

    /// Minimal TagList for the active tariff: Current and ElectricalEnergy.
    static Span<const EndpointComposition::SemanticTag> ActiveTariffTagList();

    /// `tagList` is the default Descriptor TagList, used when Register() receives a composition
    /// without tags. The caller must keep the tag storage valid for the lifetime of this object.
    explicit ElectricalEnergyTariff(Span<const EndpointComposition::SemanticTag> tagList = DefaultTagList());
    ~ElectricalEnergyTariff() override = default;

    using SingleEndpoint::Register;

    /// Returns CHIP_ERROR_INVALID_ARGUMENT, without registering anything, if the TagList in use does
    /// not meet the requirements listed in the class comment.
    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    /// Returns true if this endpoint represents the currently active tariff (the ActiveTariff
    /// condition of the device type). Subclasses that register Commodity Price SHALL return true.
    virtual bool IsActiveTariff() const { return false; }

    /// Called by Register() after the Descriptor is registered and before the endpoint is added.
    /// Implementations SHALL register Commodity Price, Commodity Tariff, or both.
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) = 0;

    /// Called by Unregister(), also when Register() fails partway. Implementations must only remove
    /// clusters that are constructed, and must not rely on GetEndpointId() since the endpoint ID is
    /// already reset at this point.
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) = 0;

private:
    CHIP_ERROR ValidateTagList(Span<const EndpointComposition::SemanticTag> tagList) const;

    const Span<const EndpointComposition::SemanticTag> mTagList;
};

} // namespace chip::app
