/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "PowerSourceCluster.h"

#include <app/static-cluster-config/PowerSource.h>
#include <lib/core/CHIPError.h>

#include <optional>
namespace chip {
namespace app {
namespace Clusters {
namespace PowerSource {
namespace detail {

constexpr BitFlags<Feature> GetAllFeaturesEnabledOnSomeEndpoint()
{
    BitFlags<Feature> features;
    for (const auto & clusterConfig : PowerSource::StaticApplicationConfig::kFixedClusterConfig)
    {
        features.Set(clusterConfig.featureMap);
    }
    return features;
}

constexpr BitFlags<Feature> kAllFeatures     = GetAllFeaturesEnabledOnSomeEndpoint();
constexpr BitFlags<Feature> kWiredFeature    = BitFlags<Feature>(kAllFeatures.Raw() & to_underlying(Feature::kWired));
constexpr BitFlags<Feature> kBatteryFeatures = BitFlags<Feature>(kAllFeatures.Raw() & ~to_underlying(Feature::kWired));

constexpr bool wiredSupportNeeded   = kWiredFeature.Has(Feature::kWired);
constexpr bool batterySupportNeeded = kBatteryFeatures.Has(Feature::kBattery);

static_assert(wiredSupportNeeded || batterySupportNeeded,
              "At least one of Wired or Battery features must be used by some endpoint");

constexpr AttributeSet GetAllAttributesEnabledOnSomeEndpoint()
{
    uint32_t attributeBits = 0;
    for (AttributeId id = 0; id < 32; id++)
    {
        if (PowerSource::StaticApplicationConfig::IsAttributeEnabledOnSomeEndpoint(id))
        {
            attributeBits |= (1 << id);
        }
    }
    return GetValidOptionalAttributeSet(AttributeSet(attributeBits), kAllFeatures);
}

constexpr AttributeSet kAllAttributes = GetAllAttributesEnabledOnSomeEndpoint();

} // namespace detail

// Dummy class to allow well-formed code when wired or battery support is not needed/supported but the code is written for them to
// be needed/supported. For example see the code in
// `src/app/clusters/power-source-configuration-server/power-source-configuration-server.cpp` lines 78-91. This code needs to work
// regardless of whether wired or battery support is there. And a `GetOrder` member function needs to be defined. Note that this
// class has all constructors deleted, so it will not actually be instantiated and used. Only a nullptr of this type will be used.
// This class is also an overview of functionality that the actuall PowerSourceCluster can have.
class DummyPowerSourceCluster final : public ::chip::app::DefaultServerCluster
{
public:
    using PowerSourceStatusEnum    = PowerSource::PowerSourceStatusEnum;
    using WiredCurrentTypeEnum     = PowerSource::WiredCurrentTypeEnum;
    using WiredFaultEnum           = PowerSource::WiredFaultEnum;
    using BatChargeLevelEnum       = PowerSource::BatChargeLevelEnum;
    using BatReplaceabilityEnum    = PowerSource::BatReplaceabilityEnum;
    using BatFaultEnum             = PowerSource::BatFaultEnum;
    using BatCommonDesignationEnum = PowerSource::BatCommonDesignationEnum;
    using BatApprovedChemistryEnum = PowerSource::BatApprovedChemistryEnum;
    using BatChargeStateEnum       = PowerSource::BatChargeStateEnum;
    using BatChargeFaultEnum       = PowerSource::BatChargeFaultEnum;

    DummyPowerSourceCluster()                                            = delete;
    DummyPowerSourceCluster(const DummyPowerSourceCluster &)             = delete;
    DummyPowerSourceCluster & operator=(const DummyPowerSourceCluster &) = delete;

    PowerSourceStatusEnum GetStatus() const { return {}; }
    uint8_t GetOrder() const { return {}; }
    CharSpan GetDescription() const { return {}; }
    std::optional<uint32_t> GetWiredAssessedInputVoltage() const { return {}; }
    std::optional<uint16_t> GetWiredAssessedInputFrequency() const { return {}; }
    WiredCurrentTypeEnum GetWiredCurrentType() const { return {}; }
    std::optional<uint32_t> GetWiredAssessedCurrent() const { return {}; }
    uint32_t GetWiredNominalVoltage() const { return {}; }
    uint32_t GetWiredMaximumCurrent() const { return {}; }
    bool GetWiredPresent() const { return {}; }
    void GetActiveWiredFaults(Span<WiredFaultEnum> &) const {}
    std::optional<uint32_t> GetBatVoltage() const { return {}; }
    std::optional<uint8_t> GetBatPercentRemaining() const { return {}; }
    std::optional<uint32_t> GetBatTimeRemaining() const { return {}; }
    BatChargeLevelEnum GetBatChargeLevel() const { return {}; }
    bool GetBatReplacementNeeded() const { return {}; }
    BatReplaceabilityEnum GetBatReplaceability() const { return {}; }
    bool GetBatPresent() const { return {}; }
    void GetActiveBatFaults(Span<BatFaultEnum> &) const {}
    CharSpan GetBatReplacementDescription() const { return {}; }
    BatCommonDesignationEnum GetBatCommonDesignation() const { return {}; }
    CharSpan GetBatANSIDesignation() const { return {}; }
    CharSpan GetBatIECDesignation() const { return {}; }
    BatApprovedChemistryEnum GetBatApprovedChemistry() const { return {}; }
    uint32_t GetBatCapacity() const { return {}; }
    uint8_t GetBatQuantity() const { return {}; }
    BatChargeStateEnum GetBatChargeState() const { return {}; }
    std::optional<uint32_t> GetBatTimeToFullCharge() const { return {}; }
    bool GetBatFunctionalWhileCharging() const { return {}; }
    std::optional<uint32_t> GetBatChargingCurrent() const { return {}; }
    void GetActiveBatChargeFaults(Span<BatChargeFaultEnum> &) const {}
    Span<const EndpointId> GetEndpointList() const { return {}; }

    CHIP_ERROR SetStatus(PowerSourceStatusEnum) { return {}; }
    CHIP_ERROR SetOrder(uint8_t) { return {}; }
    CHIP_ERROR SetWiredAssessedInputVoltage(std::optional<uint32_t>) { return {}; }
    CHIP_ERROR SetWiredAssessedInputFrequency(std::optional<uint16_t>) { return {}; }
    CHIP_ERROR SetWiredAssessedCurrent(std::optional<uint32_t>) { return {}; }
    CHIP_ERROR SetWiredPresent(bool) { return {}; }
    CHIP_ERROR SetActiveWiredFaults(Span<const WiredFaultEnum>) { return {}; }
    CHIP_ERROR AddActiveWiredFault(WiredFaultEnum) { return {}; }
    CHIP_ERROR RemoveActiveWiredFault(WiredFaultEnum) { return {}; }
    CHIP_ERROR SetBatVoltage(std::optional<uint32_t>) { return {}; }
    CHIP_ERROR SetBatPercentRemaining(std::optional<uint8_t>) { return {}; }
    CHIP_ERROR SetBatTimeRemaining(std::optional<uint32_t>) { return {}; }
    CHIP_ERROR SetBatChargeLevel(BatChargeLevelEnum) { return {}; }
    CHIP_ERROR SetBatReplacementNeeded(bool) { return {}; }
    CHIP_ERROR SetBatPresent(bool) { return {}; }
    CHIP_ERROR SetActiveBatFaults(Span<const BatFaultEnum>) { return {}; }
    CHIP_ERROR AddActiveBatFault(BatFaultEnum) { return {}; }
    CHIP_ERROR RemoveActiveBatFault(BatFaultEnum) { return {}; }
    CHIP_ERROR SetBatChargeState(BatChargeStateEnum) { return {}; }
    CHIP_ERROR SetBatTimeToFullCharge(std::optional<uint32_t>) { return {}; }
    CHIP_ERROR SetBatFunctionalWhileCharging(bool) { return {}; }
    CHIP_ERROR SetBatChargingCurrent(std::optional<uint32_t>) { return {}; }
    CHIP_ERROR SetActiveBatChargeFaults(Span<const BatChargeFaultEnum>) { return {}; }
    CHIP_ERROR AddActiveBatChargeFault(BatChargeFaultEnum) { return {}; }
    CHIP_ERROR RemoveActiveBatChargeFault(BatChargeFaultEnum) { return {}; }
    CHIP_ERROR SetEndpointList(Span<const EndpointId>) { return {}; }
};

using EmberWiredPowerSourceCluster =
    std::conditional_t<detail::wiredSupportNeeded, PowerSourceCluster<detail::kWiredFeature.Raw(), detail::kAllAttributes.Raw()>,
                       DummyPowerSourceCluster>;
using EmberBatteryPowerSourceCluster =
    std::conditional_t<detail::batterySupportNeeded,
                       PowerSourceCluster<detail::kBatteryFeatures.Raw(), detail::kAllAttributes.Raw()>, DummyPowerSourceCluster>;

EmberWiredPowerSourceCluster * FindWiredClusterOnEndpoint(EndpointId id);
EmberBatteryPowerSourceCluster * FindBatteryClusterOnEndpoint(EndpointId id);

} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace chip
