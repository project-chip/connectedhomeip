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
#include "PowerSourceCluster.h"

#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app-common/zap-generated/attributes/Accessors.h>
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
constexpr size_t kPowerSourceMaxClusterCount = kPowerSourceFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<PowerSourceCluster> gServers[kPowerSourceMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        PowerSourceCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        BitFlags<Feature> features(featureMap);
        using namespace chip::Protocols::InteractionModel;

        // Enforce a valid configuration from ember

        char descriptionBuffer[Description::TypeInfo::MaxLength()];
        MutableCharSpan description(descriptionBuffer);
        VerifyOrDie(Description::Get(endpointId, description) == Status::Success);

        if (features.Has(Feature::kWired))
        {
            PowerSourceCluster::WiredCurrentTypeEnum currentType;
            VerifyOrDie(optionalAttributeSet.IsSet(WiredCurrentType::Id));
            VerifyOrDie(WiredCurrentType::Get(endpointId, &currentType) == Status::Success);

            PowerSourceCluster::WiredConfiguration config(description, currentType);

            uint32_t nominalVoltage{};
            if (WiredNominalVoltage::Get(endpointId, &nominalVoltage) == Status::Success)
            {
                config.nominalVoltage = nominalVoltage;
            }

            uint32_t maximumCurrent{};
            if (WiredMaximumCurrent::Get(endpointId, &maximumCurrent) == Status::Success)
            {
                config.maximumCurrent = maximumCurrent;
            }

            gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeBits, DeviceLayer::SystemLayer(), config);
        }
        else if (features.Has(Feature::kBattery))
        {
            PowerSourceCluster::BatReplaceabilityEnum replaceability;
            VerifyOrDie(optionalAttributeSet.IsSet(BatReplaceability::Id));
            VerifyOrDie(BatReplaceability::Get(endpointId, &replaceability) == Status::Success);

            PowerSourceCluster::BatteryConfiguration config(description, replaceability);

            uint32_t capacity{};
            if (BatCapacity::Get(endpointId, &capacity) == Status::Success)
            {
                config.capacity = capacity;
            }

            if (features.Has(Feature::kReplaceable))
            {
                char replacementDescriptionBuffer[BatReplacementDescription::TypeInfo::MaxLength()];
                MutableCharSpan replacementDescription(replacementDescriptionBuffer);
                VerifyOrDie(optionalAttributeSet.IsSet(BatReplacementDescription::Id));
                VerifyOrDie(BatReplacementDescription::Get(endpointId, replacementDescription) == Status::Success);

                uint8_t quantity;
                VerifyOrDie(optionalAttributeSet.IsSet(BatQuantity::Id));
                VerifyOrDie(BatQuantity::Get(endpointId, &quantity) == Status::Success);

                config.MakeReplaceable(replacementDescription, quantity);

                PowerSourceCluster::BatCommonDesignationEnum commonDesignation{};
                if (BatCommonDesignation::Get(endpointId, &commonDesignation) == Status::Success)
                {
                    config.commonDesignation = commonDesignation;
                }

                char ansiDesignationBuffer[BatANSIDesignation::TypeInfo::MaxLength()];
                MutableCharSpan ansiDesignation{};
                if (BatANSIDesignation::Get(endpointId, ansiDesignation) == Status::Success)
                {
                    config.ansiDesignation = ansiDesignation;
                }

                char iecDesignationBuffer[BatIECDesignation::TypeInfo::MaxLength()];
                MutableCharSpan iecDesignation{};
                if (BatIECDesignation::Get(endpointId, iecDesignation) == Status::Success)
                {
                    config.iecDesignation = iecDesignation;
                }

                PowerSourceCluster::BatApprovedChemistryEnum approvedChemistry{};
                if (BatApprovedChemistry::Get(endpointId, &approvedChemistry) == Status::Success)
                {
                    config.approvedChemistry = approvedChemistry;
                }
            }
            if (features.Has(Feature::kRechargeable))
            {
                config.MakeRechargeable();
            }
            gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeBits, DeviceLayer::SystemLayer(), config);
        }

        PowerSourceCluster cluster = gServers[clusterInstanceIndex].Cluster();

        // Get all set defaults for attributes from ember.

#define DieIfInvalidValue(expr, attr_name)\
        {\
            CHIP_ERROR err = (expr);\
            VerifyOrDieWithMsg(err == CHIP_NO_ERROR || err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, NotSpecified, "Unexpected error %" CHIP_ERROR_FORMAT " when trying to set attribute `" #attr_name "`.", err);\
        }

#define SetAttributeDefaultFromEmber(type, attr_name)\
        if (type val{}; attr_name::Get(endpointId, &val) == Status::Success)\
        {\
            DieIfInvalidValue(cluster.Set##attr_name(val));\
        }

#define SetNullableAttributeDefaultFromEmber(type, attr_name)\
        if (DataModel::Nullable<type> val{}; attr_name::Get(endpointId, val) == Status::Success)\
        {\
            if (val.isNull())\
            {\
                cluster.Set##attr_name(NullOptional); /* null is valid. */\
            }\
            else\
            {\
                DieIfInvalidValue(cluster.Set##attr_name(Optional(val.value())));\
            }\
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
