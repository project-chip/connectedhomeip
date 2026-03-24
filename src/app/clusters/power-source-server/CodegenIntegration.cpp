/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "CodegenIntegration.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>
#include <app/static-cluster-config/PowerSource.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <include/platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;

namespace {

constexpr size_t kPowerSourceFixedClusterCount = PowerSource::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kPowerSourceMaxClusterCount   = kPowerSourceFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<PowerSourceCluster> gServers[kPowerSourceMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        PowerSourceCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        BitFlags<Feature> features(featureMap);
        using namespace chip::Protocols;

        // Enforce a valid configuration from ember

        char descriptionBuffer[Description::TypeInfo::MaxLength()];
        MutableCharSpan description(descriptionBuffer);
        if (Description::Get(endpointId, description) != InteractionModel::Status::Success)
        {
            // an acceptable default of empty string
            description.reduce_size(0);
        }

        if (features.Has(Feature::kWired))
        {
            PowerSourceCluster::WiredCurrentTypeEnum currentType;
            VerifyOrDie(optionalAttributeSet.IsSet(WiredCurrentType::Id));
            VerifyOrDie(WiredCurrentType::Get(endpointId, &currentType) == InteractionModel::Status::Success);

            PowerSourceCluster::WiredConfiguration config(description, currentType);

            uint32_t nominalVoltage{};
            if (WiredNominalVoltage::Get(endpointId, &nominalVoltage) == InteractionModel::Status::Success)
            {
                config.nominalVoltage = nominalVoltage;
            }

            uint32_t maximumCurrent{};
            if (WiredMaximumCurrent::Get(endpointId, &maximumCurrent) == InteractionModel::Status::Success)
            {
                config.maximumCurrent = maximumCurrent;
            }

            gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeSet, DeviceLayer::SystemLayer(), config);
        }
        else if (features.Has(Feature::kBattery))
        {
            // default value
            PowerSourceCluster::BatReplaceabilityEnum replaceability = PowerSourceCluster::BatReplaceabilityEnum::kUnspecified;
            VerifyOrDie(optionalAttributeSet.IsSet(BatReplaceability::Id));
            // try to read, if fails, default will be used
            BatReplaceability::Get(endpointId, &replaceability);

            PowerSourceCluster::BatteryConfiguration config(description, replaceability);

            uint32_t capacity{};
            if (BatCapacity::Get(endpointId, &capacity) == InteractionModel::Status::Success)
            {
                config.capacity = capacity;
            }

            if (features.Has(Feature::kReplaceable))
            {
                char replacementDescriptionBuffer[BatReplacementDescription::TypeInfo::MaxLength()];
                MutableCharSpan replacementDescription(replacementDescriptionBuffer);
                VerifyOrDie(optionalAttributeSet.IsSet(BatReplacementDescription::Id));
                VerifyOrDie(BatReplacementDescription::Get(endpointId, replacementDescription) ==
                            InteractionModel::Status::Success);

                uint8_t quantity;
                VerifyOrDie(optionalAttributeSet.IsSet(BatQuantity::Id));
                VerifyOrDie(BatQuantity::Get(endpointId, &quantity) == InteractionModel::Status::Success);

                config.MakeReplaceable(replacementDescription, quantity);

                PowerSourceCluster::BatCommonDesignationEnum commonDesignation{};
                if (BatCommonDesignation::Get(endpointId, &commonDesignation) == InteractionModel::Status::Success)
                {
                    config.commonDesignation = commonDesignation;
                }

                char ansiDesignationBuffer[BatANSIDesignation::TypeInfo::MaxLength()];
                MutableCharSpan ansiDesignation(ansiDesignationBuffer);
                if (BatANSIDesignation::Get(endpointId, ansiDesignation) == InteractionModel::Status::Success)
                {
                    config.ansiDesignation = ansiDesignation;
                }

                char iecDesignationBuffer[BatIECDesignation::TypeInfo::MaxLength()];
                MutableCharSpan iecDesignation(iecDesignationBuffer);
                if (BatIECDesignation::Get(endpointId, iecDesignation) == InteractionModel::Status::Success)
                {
                    config.iecDesignation = iecDesignation;
                }

                PowerSourceCluster::BatApprovedChemistryEnum approvedChemistry{};
                if (BatApprovedChemistry::Get(endpointId, &approvedChemistry) == InteractionModel::Status::Success)
                {
                    config.approvedChemistry = approvedChemistry;
                }
            }
            if (features.Has(Feature::kRechargeable))
            {
                config.MakeRechargeable();
            }
            gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeSet, DeviceLayer::SystemLayer(), config);
        }
        else
        {
            // power source is not wired and is not battery, this is invalid, so we should die
            // using `VerifyOrDieWitMsg` to print a proper log and die with one line
            VerifyOrDieWithMsg(false, Zcl, "Invalid FeatureMap from ember for the PowerSource cluster");
        }

        PowerSourceCluster & cluster = gServers[clusterInstanceIndex].Cluster();

        // Get all set defaults for attributes from ember.

#define DieIfInvalidValue(expr, attr_name)                                                                                         \
    {                                                                                                                              \
        CHIP_ERROR error_val = (expr);                                                                                             \
        VerifyOrDieWithMsg(error_val == CHIP_NO_ERROR || error_val == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Zcl,                    \
                           "Unexpected error %" CHIP_ERROR_FORMAT " when trying to set attribute `" #attr_name "`.",               \
                           error_val.Format());                                                                                    \
    }

#define SetAttributeDefaultFromEmber(type, attr_name)                                                                              \
    if (type val{}; attr_name::Get(endpointId, &val) == InteractionModel::Status::Success)                                         \
    {                                                                                                                              \
        DieIfInvalidValue(cluster.Set##attr_name(val), attr_name);                                                                 \
    }

#define SetNullableAttributeDefaultFromEmber(type, attr_name)                                                                      \
    if (DataModel::Nullable<type> val{}; attr_name::Get(endpointId, val) == InteractionModel::Status::Success)                     \
    {                                                                                                                              \
        if (val.IsNull())                                                                                                          \
        {                                                                                                                          \
            (void) cluster.Set##attr_name(NullOptional); /* null is valid, can ignore the error */                                 \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            DieIfInvalidValue(cluster.Set##attr_name(Optional(val.Value())), attr_name);                                           \
        }                                                                                                                          \
    }

        SetAttributeDefaultFromEmber(PowerSourceCluster::PowerSourceStatusEnum, Status);
        SetAttributeDefaultFromEmber(uint8_t, Order);
        SetNullableAttributeDefaultFromEmber(uint32_t, WiredAssessedInputVoltage);
        SetNullableAttributeDefaultFromEmber(uint16_t, WiredAssessedInputFrequency);
        SetNullableAttributeDefaultFromEmber(uint32_t, WiredAssessedCurrent);
        SetAttributeDefaultFromEmber(bool, WiredPresent);
        SetNullableAttributeDefaultFromEmber(uint32_t, BatVoltage);
        SetNullableAttributeDefaultFromEmber(uint8_t, BatPercentRemaining);
        SetNullableAttributeDefaultFromEmber(uint32_t, BatTimeRemaining);
        SetAttributeDefaultFromEmber(PowerSourceCluster::BatChargeLevelEnum, BatChargeLevel);
        SetAttributeDefaultFromEmber(bool, BatReplacementNeeded);
        SetAttributeDefaultFromEmber(bool, BatPresent);
        SetAttributeDefaultFromEmber(PowerSourceCluster::BatChargeStateEnum, BatChargeState);
        SetNullableAttributeDefaultFromEmber(uint32_t, BatTimeToFullCharge);
        SetAttributeDefaultFromEmber(bool, BatFunctionalWhileCharging);
        SetNullableAttributeDefaultFromEmber(uint32_t, BatChargingCurrent);

#undef DieIfInvalidValue
#undef SetAttributeDefaultFromEmber
#undef SetNullableAttributeDefaultFromEmber

        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterPowerSourceClusterInitCallback(EndpointId endpointId)
{
    // If the cluster was already registered manually, don't create and register it from ember.
    auto clusterList = CodegenDataModelProvider::Instance().Registry().ClustersOnEndpoint(endpointId);
    if (std::find(clusterList.begin(), clusterList.end(), PowerSource::Id) != clusterList.end())
    {
        return;
    }

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PowerSource::Id,
            .fixedClusterInstanceCount = kPowerSourceFixedClusterCount,
            .maxClusterInstanceCount   = kPowerSourceMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterPowerSourceClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PowerSource::Id,
            .fixedClusterInstanceCount = kPowerSourceFixedClusterCount,
            .maxClusterInstanceCount   = kPowerSourceMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterPowerSourcePluginServerInitCallback() {}

namespace chip::app::Clusters::PowerSource {

PowerSourceCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * powerSource = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = PowerSource::Id,
            .fixedClusterInstanceCount = kPowerSourceFixedClusterCount,
            .maxClusterInstanceCount   = kPowerSourceMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<PowerSourceCluster *>(powerSource);
}

} // namespace chip::app::Clusters::PowerSource
