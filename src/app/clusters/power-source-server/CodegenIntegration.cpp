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
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DefaultTimerDelegate.h>

#include <type_traits>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;
using namespace chip::app::Clusters::PowerSource::detail;

namespace {

DefaultTimerDelegate gTimerDelegate;

constexpr size_t kPowerSourceFixedClusterCount = PowerSource::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kPowerSourceMaxClusterCount   = kPowerSourceFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

using LazyRegisteredWiredSourceCluster =
    std::conditional_t<wiredSupportNeeded, LazyRegisteredServerCluster<EmberWiredPowerSourceCluster>, void>;
using LazyRegisteredBatterySourceCluster =
    std::conditional_t<batterySupportNeeded, LazyRegisteredServerCluster<EmberBatteryPowerSourceCluster>, void>;

using LazyRegisteredPowerSourceCluster = std::conditional_t<
    wiredSupportNeeded,
    std::conditional_t<batterySupportNeeded, std::variant<LazyRegisteredWiredSourceCluster, LazyRegisteredBatterySourceCluster>,
                       LazyRegisteredWiredSourceCluster>,
    std::conditional_t<batterySupportNeeded, LazyRegisteredBatterySourceCluster, std::monostate>>;

LazyRegisteredPowerSourceCluster gServers[kPowerSourceMaxClusterCount];
std::array<StringAttributeStorageModule<kAllAttributes.Raw()>, CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT> gStringAttributeStorage;

CharSpan GetCharStringDefaultValueDirectlyFromEndpointConfig(EndpointId endpointId, AttributeId attributeId)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpointId, PowerSource::Id, attributeId);
    VerifyOrDie(metadata != nullptr);
    VerifyOrDie(!metadata->IsExternal());
    VerifyOrDie(metadata->attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE ||
                metadata->attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
    if (metadata->defaultValue.ptrToDefaultValue == nullptr)
    {
        return CharSpan();
    }

    size_t bytesForLength = (metadata->attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) ? 1 : 2;
    size_t size           = (metadata->attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE
                                 ? static_cast<size_t>(metadata->defaultValue.ptrToDefaultValue[0])
                                 : static_cast<size_t>(reinterpret_cast<const uint16_t *>(metadata->defaultValue.ptrToDefaultValue)[0]));

    return CharSpan(reinterpret_cast<const char *>(metadata->defaultValue.ptrToDefaultValue) + bytesForLength, size);
}

template <size_t maxLength, class GetAccessor>
CharSpan GetCharStringDefaultValueFromEmber(GetAccessor getter, EndpointId endpointId, std::string & storage)
{
    char buffer[maxLength];
    MutableCharSpan span(buffer);
    if (getter(endpointId, span) != Protocols::InteractionModel::Status::Success)
    {
        // an acceptable default of empty string
        span.reduce_size(0);
    }
    storage = std::string(span.data(), span.size());
    return CharSpan(storage.data(), storage.size());
}

// templated to be able to use `if constexpr` inside the functions
template <size_t dynamicEndpointCount = CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT, bool wiredSupported = wiredSupportNeeded, bool batterySupported = batterySupportNeeded,
          uint32_t batteryFeatureBits = kBatteryFeatures.Raw(), class EmberWiredPowerSourceClusterT = EmberWiredPowerSourceCluster,
          class EmberBatteryPowerSourceClusterT     = EmberBatteryPowerSourceCluster,
          class LazyRegisteredWiredSourceClusterT   = LazyRegisteredWiredSourceCluster,
          class LazyRegisteredBatterySourceClusterT = LazyRegisteredBatterySourceCluster,
          class LazyRegisteredPowerSourceClusterT   = LazyRegisteredPowerSourceCluster,
          class StringStorageModuleT                = StringAttributeStorageModule<kAllAttributes.Raw()>>
class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    constexpr static BitFlags<Feature> batteryFeatures{ batteryFeatureBits };

public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        AttributeSet optionalAttributeSet(PowerSource::detail::GetValidOptionalAttributeSet(
            AttributeSet(optionalAttributeBits), BitFlags<PowerSource::Feature>(featureMap)));
        BitFlags<Feature> features(featureMap);
        using namespace chip::Protocols;

        // Enforce a valid configuration from ember

        VerifyOrDieWithMsg(
            // Exactly one of wired or battery features must be set
            features.Has(Feature::kWired) ^ features.Has(Feature::kBattery) &&
                // features reported by ember must be a subset of features that are enabled on any endpoint in zap tool
                (features.Raw() & kAllFeatures.Raw()) == features.Raw(),
            Zcl, "CreateRegistration called with invalid feature map");

        CharSpan description{};
        if (clusterInstanceIndex < kPowerSourceFixedClusterCount)
        {
            // for fixed endpoints
            description = GetCharStringDefaultValueDirectlyFromEndpointConfig(endpointId, Description::Id);
        }
        else
        {
            // for dynamic endpoints
            description = GetCharStringDefaultValueFromEmber<Description::TypeInfo::MaxLength()>(
                Description::Get, endpointId,
                // this static cast does nothing, it is here to make the code template dependent on class template, so the code will
                // compile.
                static_cast<StringStorageModuleT &>(gStringAttributeStorage[clusterInstanceIndex - kPowerSourceFixedClusterCount])
                    .description);
        }

#define SetAttributeDefaultFromEmber(power_source_type, attr_type, attr_name, config_field_name)                                   \
    if constexpr (Ember##power_source_type##PowerSourceClusterT::supportedOptionalAttributeSet.IsSet(attr_name::Id))               \
    {                                                                                                                              \
        if (attr_type val{}; attr_name::Get(endpointId, &val) == InteractionModel::Status::Success)                                \
        {                                                                                                                          \
            config.config_field_name = val;                                                                                        \
        }                                                                                                                          \
    }

#define SetNullableAttributeDefaultFromEmber(power_source_type, attr_type, attr_name, config_field_name)                           \
    if constexpr (Ember##power_source_type##PowerSourceClusterT::supportedOptionalAttributeSet.IsSet(attr_name::Id))               \
    {                                                                                                                              \
        if (DataModel::Nullable<attr_type> val{}; attr_name::Get(endpointId, val) == InteractionModel::Status::Success)            \
        {                                                                                                                          \
            if (!val.IsNull())                                                                                                     \
            {                                                                                                                      \
                config.config_field_name = val.Value();                                                                            \
            }                                                                                                                      \
        }                                                                                                                          \
    }

        if constexpr (wiredSupported)
        {
            if (features.Has(Feature::kWired))
            {
                typename EmberWiredPowerSourceClusterT::WiredCurrentTypeEnum currentType;
                VerifyOrDie(WiredCurrentType::Get(endpointId, &currentType) == InteractionModel::Status::Success);

                typename EmberWiredPowerSourceClusterT::ConfigType config(endpointId, description, currentType);
                if constexpr (EmberWiredPowerSourceClusterT::supportedOptionalAttributeSet.IsSet(WiredAssessedInputVoltage::Id))
                {
                    if (DataModel::Nullable<uint32_t> val{};
                        WiredAssessedInputVoltage::Get(endpointId, val) == InteractionModel::Status::Success)
                    {
                        if (!val.IsNull())
                        {
                            config.wiredAssessedInputVoltage = val.Value();
                        }
                    }
                }
                SetNullableAttributeDefaultFromEmber(Wired, uint32_t, WiredAssessedInputVoltage, wiredAssessedInputVoltage);
                SetNullableAttributeDefaultFromEmber(Wired, uint16_t, WiredAssessedInputFrequency, wiredAssessedInputFrequency);
                SetNullableAttributeDefaultFromEmber(Wired, uint32_t, WiredAssessedCurrent, wiredAssessedCurrent);
                SetAttributeDefaultFromEmber(Wired, uint32_t, WiredNominalVoltage, wiredNominalVoltage);
                SetAttributeDefaultFromEmber(Wired, uint32_t, WiredMaximumCurrent, wiredMaximumCurrent);
                SetAttributeDefaultFromEmber(Wired, bool, WiredPresent, wiredPresent);

                config.usedOptionalAttributes = optionalAttributeSet;
                LazyRegisteredWiredSourceClusterT * server;
                if constexpr (batterySupported)
                {
                    gServers[clusterInstanceIndex] = LazyRegisteredWiredSourceClusterT();
                    server = std::get_if<LazyRegisteredWiredSourceClusterT>(&gServers[clusterInstanceIndex]);
                }
                else
                {
                    server = &gServers[clusterInstanceIndex];
                }

                // this should never fail
                VerifyOrDie(server != nullptr);

                server->Create(config);
                return server->Registration();
            }
        }
        if constexpr (batterySupported)
        {
            if (features.Has(Feature::kBattery))
            {
                // default value
                typename EmberBatteryPowerSourceClusterT::BatReplaceabilityEnum replaceability =
                    EmberBatteryPowerSourceClusterT::BatReplaceabilityEnum::kUnspecified;
                // try to read, if fails, default will be used
                BatReplaceability::Get(endpointId, &replaceability);

                typename EmberBatteryPowerSourceClusterT::ConfigType config(endpointId, description, replaceability,
                                                                            gTimerDelegate);

                SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatVoltage, batVoltage);
                SetNullableAttributeDefaultFromEmber(Battery, uint8_t, BatPercentRemaining, batPercentRemaining);
                SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatTimeRemaining, batTimeRemaining);
                SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::BatChargeLevelEnum, BatChargeLevel,
                                             batChargeLevel);
                SetAttributeDefaultFromEmber(Battery, bool, BatReplacementNeeded, batReplacementNeeded);
                SetAttributeDefaultFromEmber(Battery, bool, BatPresent, batPresent);

                if constexpr (batteryFeatures.Has(Feature::kReplaceable))
                {
                    if (features.Has(Feature::kReplaceable))
                    {
                        CharSpan replacementDescription{};
                        if (clusterInstanceIndex < kPowerSourceFixedClusterCount)
                        {
                            // for fixed endpoints
                            replacementDescription =
                                GetCharStringDefaultValueDirectlyFromEndpointConfig(endpointId, BatReplacementDescription::Id);
                        }
                        else
                        {
                            // for dynamic endpoints
                            replacementDescription =
                                GetCharStringDefaultValueFromEmber<BatReplacementDescription::TypeInfo::MaxLength()>(
                                    BatReplacementDescription::Get, endpointId,
                                    // this static cast does nothing, it is here to make the code template dependent on class
                                    // template, so the code will compile.
                                    static_cast<StringStorageModuleT &>(
                                        gStringAttributeStorage[clusterInstanceIndex - kPowerSourceFixedClusterCount])
                                        .batReplacementDescription);
                        }

                        uint8_t quantity;
                        VerifyOrDie(BatQuantity::Get(endpointId, &quantity) == InteractionModel::Status::Success);
                        config.MakeReplaceable(replacementDescription, quantity);

                        SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::BatCommonDesignationEnum,
                                                     BatCommonDesignation, batCommonDesignation);

                        if constexpr (EmberBatteryPowerSourceClusterT::supportedOptionalAttributeSet.IsSet(BatANSIDesignation::Id))
                        {
                            if (clusterInstanceIndex < kPowerSourceFixedClusterCount)
                            {
                                // for fixed endpoints
                                config.batANSIDesignation =
                                    GetCharStringDefaultValueDirectlyFromEndpointConfig(endpointId, BatANSIDesignation::Id);
                            }
                            else
                            {
                                // for dynamic endpoints
                                config.batANSIDesignation =
                                    GetCharStringDefaultValueFromEmber<BatANSIDesignation::TypeInfo::MaxLength()>(
                                        BatANSIDesignation::Get, endpointId,
                                        // this static cast does nothing, it is here to make the code template dependent on class
                                        // template, so the code will compile.
                                        static_cast<StringStorageModuleT &>(
                                            gStringAttributeStorage[clusterInstanceIndex - kPowerSourceFixedClusterCount])
                                            .batANSIDesignation);
                            }
                        }

                        if constexpr (EmberBatteryPowerSourceClusterT::supportedOptionalAttributeSet.IsSet(BatIECDesignation::Id))
                        {
                            if (clusterInstanceIndex < kPowerSourceFixedClusterCount)
                            {
                                // for fixed endpoints
                                config.batIECDesignation =
                                    GetCharStringDefaultValueDirectlyFromEndpointConfig(endpointId, BatIECDesignation::Id);
                            }
                            else
                            {
                                // for dynamic endpoints
                                config.batIECDesignation =
                                    GetCharStringDefaultValueFromEmber<BatIECDesignation::TypeInfo::MaxLength()>(
                                        BatIECDesignation::Get, endpointId,
                                        // this static cast does nothing, it is here to make the code template dependent on class
                                        // template, so the code will compile.
                                        static_cast<StringStorageModuleT &>(
                                            gStringAttributeStorage[clusterInstanceIndex - kPowerSourceFixedClusterCount])
                                            .batIECDesignation);
                            }
                        }

                        SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::BatApprovedChemistryEnum,
                                                     BatApprovedChemistry, batApprovedChemistry);
                    }
                }
                if constexpr (batteryFeatures.Has(Feature::kRechargeable))
                {
                    if (features.Has(Feature::kRechargeable))
                    {
                        config.MakeRechargeable();
                        SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::BatChargeStateEnum,
                                                     BatChargeState, batChargeState);
                        SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatTimeToFullCharge, batTimeToFullCharge);
                        SetAttributeDefaultFromEmber(Battery, bool, BatFunctionalWhileCharging, batFunctionalWhileCharging);
                        SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatChargingCurrent, batChargingCurrent);
                    }
                }

                SetAttributeDefaultFromEmber(Battery, uint32_t, BatCapacity, batCapacity);

                config.usedOptionalAttributes = optionalAttributeSet;
                LazyRegisteredBatterySourceClusterT * server;
                if constexpr (wiredSupported)
                {
                    gServers[clusterInstanceIndex] = LazyRegisteredBatterySourceClusterT();
                    server = std::get_if<LazyRegisteredBatterySourceClusterT>(&gServers[clusterInstanceIndex]);
                }
                else
                {
                    server = &gServers[clusterInstanceIndex];
                }

                // this should never fail
                VerifyOrDie(server != nullptr);
                server->Create(config);

                return server->Registration();
            }
        }

#undef SetAttributeDefaultFromEmber
#undef SetNullableAttributeDefaultFromEmber

        // unreachable
        chipDie();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        // The type is the same, just making this whole section depending to the class tempaltes,
        // so that compiler will not complain on the code inside if constexpr.
        LazyRegisteredPowerSourceClusterT & gServer = gServers[clusterInstanceIndex];
        if constexpr (wiredSupported && batterySupported)
        {
            return std::visit(
                [](auto & server) {
                    return server.IsConstructed() ? static_cast<ServerClusterInterface *>(&server.Cluster()) : nullptr;
                },
                gServer);
        }
        else
        {
            VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
            return &gServer.Cluster();
        }
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        // The type is the same, just making this whole section depending to the class tempaltes,
        // so that compiler will not complain on the code inside if constexpr.
        LazyRegisteredPowerSourceClusterT & gServer = gServers[clusterInstanceIndex];
        if constexpr (wiredSupported && batterySupported)
        {
            std::visit([](auto & server) { server.Destroy(); }, gServer);
        }
        else
        {
            gServer.Destroy();
        }
    }
};

} // namespace

void MatterPowerSourceClusterInitCallback(EndpointId endpointId)
{
    uint32_t featureBits{};
    FeatureMap::Get(endpointId, &featureBits);
    BitFlags<Feature> features(featureBits);
    VerifyOrDieWithMsg(features.HasAny(Feature::kWired, Feature::kBattery), Zcl,
                       "Empty feature map for PowerSource cluster on endpoint %d", endpointId);

    if (features.HasAll(Feature::kWired, Feature::kBattery))
    {
        // both wired and battery are specified, it is likely this is a disabled configuration for dynamic endpoints
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

EmberWiredPowerSourceCluster * FindWiredClusterOnEndpoint(EndpointId endpointId)
{
    if constexpr (!wiredSupportNeeded)
    {
        return nullptr;
    }
    else
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

        return static_cast<EmberWiredPowerSourceCluster *>(powerSource);
    }
}

EmberBatteryPowerSourceCluster * FindBatteryClusterOnEndpoint(EndpointId endpointId)
{
    if constexpr (!batterySupportNeeded)
    {
        return nullptr;
    }
    else
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

        return static_cast<EmberBatteryPowerSourceCluster *>(powerSource);
    }
}

} // namespace chip::app::Clusters::PowerSource
