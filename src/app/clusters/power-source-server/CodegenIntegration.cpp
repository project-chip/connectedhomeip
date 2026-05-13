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
#include <app/util/ember-strings.h>
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

#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT > 0
StringAttributeStorageModule<kAllAttributes.Raw()> gStringAttributeStorage[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT > 0

CharSpan GetCharStringDefaultValueDirectlyFromEndpointConfig(EndpointId endpointId, AttributeId attributeId)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpointId, PowerSource::Id, attributeId);
    VerifyOrDie(metadata != nullptr);
    VerifyOrDie(!metadata->IsExternal());
    VerifyOrDie(metadata->attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (metadata->defaultValue.ptrToDefaultValue == nullptr)
    {
        return CharSpan();
    }

    auto length = emberAfStringLength(metadata->defaultValue.ptrToDefaultValue);

    return CharSpan(reinterpret_cast<const char *>(metadata->defaultValue.ptrToDefaultValue) + 1, length);
}

#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT == 0
// This function can handle non nullable signed integer and boolean attributes, up to uint32_t.
uint32_t GetSimpleIntegerDefaultValueDirectlyFromEndpointConfig(EndpointId endpointId, AttributeId attributeId)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpointId, PowerSource::Id, attributeId);
    VerifyOrDie(metadata != nullptr);
    VerifyOrDie(!metadata->IsExternal());
    VerifyOrDie(metadata->attributeType == ZCL_BOOLEAN_ATTRIBUTE_TYPE || metadata->attributeType == ZCL_INT8U_ATTRIBUTE_TYPE ||
                metadata->attributeType == ZCL_INT16U_ATTRIBUTE_TYPE || metadata->attributeType == ZCL_INT32U_ATTRIBUTE_TYPE);
    return metadata->defaultValue.defaultValue;
}
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT == 0

template <class GetAccessor>
CharSpan GetCharStringDefaultValueFromEmber(GetAccessor getter, EndpointId endpointId, std::string & storage)
{
    static constexpr size_t maxLength = 60; // maximum length of a string attribute in the PowerSource cluster.
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
template <bool wiredSupported = wiredSupportNeeded, bool batterySupported = batterySupportNeeded,
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
#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT > 0
// The static cast here does nothing, it is here to make the code template dependent on class template, so the code will
// compile.
#define GetStringAttribute(attr_name, storage_field_name)                                                                          \
    ((clusterInstanceIndex >= kPowerSourceFixedClusterCount)                                                                       \
         ? GetCharStringDefaultValueFromEmber(                                                                                     \
               attr_name::GetDefault, endpointId,                                                                                  \
               static_cast<StringStorageModuleT &>(gStringAttributeStorage[clusterInstanceIndex - kPowerSourceFixedClusterCount])  \
                   .storage_field_name)                                                                                            \
         : GetCharStringDefaultValueDirectlyFromEndpointConfig(endpointId, attr_name::Id))
#else
#define GetStringAttribute(attr_name, storage_field_name)                                                                          \
    GetCharStringDefaultValueDirectlyFromEndpointConfig(endpointId, attr_name::Id)
#endif

#define SetAttributeDefaultFromEmber(power_source_type, attr_type, attr_name, config_field_name)                                   \
    if constexpr (Ember##power_source_type##PowerSourceClusterT::supportedOptionalAttributeSet.IsSet(attr_name::Id))               \
    {                                                                                                                              \
        if (attr_type val{}; attr_name::GetDefault(endpointId, &val) == InteractionModel::Status::Success)                         \
        {                                                                                                                          \
            config.config_field_name = val;                                                                                        \
        }                                                                                                                          \
    }

#define SetNullableAttributeDefaultFromEmber(power_source_type, attr_type, attr_name, config_field_name)                           \
    if constexpr (Ember##power_source_type##PowerSourceClusterT::supportedOptionalAttributeSet.IsSet(attr_name::Id))               \
    {                                                                                                                              \
        if (DataModel::Nullable<attr_type> val{}; attr_name::GetDefault(endpointId, val) == InteractionModel::Status::Success)     \
        {                                                                                                                          \
            if (!val.IsNull())                                                                                                     \
            {                                                                                                                      \
                config.config_field_name = val.Value();                                                                            \
            }                                                                                                                      \
        }                                                                                                                          \
    }

#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT == 0
#define SetSimpleIntegerDefault(power_source_type, attr_type, attr_name, config_field_name)                                        \
    if constexpr (Ember##power_source_type##PowerSourceClusterT::supportedOptionalAttributeSet.IsSet(attr_name::Id))               \
    {                                                                                                                              \
        config.config_field_name =                                                                                                 \
            static_cast<attr_type>(GetSimpleIntegerDefaultValueDirectlyFromEndpointConfig(endpointId, attr_name::Id));             \
    }
#else // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT > 0
#define SetSimpleIntegerDefault(power_source_type, attr_type, attr_name, config_field_name)                                        \
    SetAttributeDefaultFromEmber(power_source_type, attr_type, attr_name, config_field_name)
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT == 0

        CharSpan description = GetStringAttribute(Description, description);
        if constexpr (wiredSupported)
        {
            if (features.Has(Feature::kWired))
            {
                typename EmberWiredPowerSourceClusterT::WiredCurrentTypeEnum currentType;
                VerifyOrDie(WiredCurrentType::GetDefault(endpointId, &currentType) == InteractionModel::Status::Success);

                typename EmberWiredPowerSourceClusterT::ConfigType config(endpointId, description, currentType);
                if constexpr (EmberWiredPowerSourceClusterT::supportedOptionalAttributeSet.IsSet(WiredAssessedInputVoltage::Id))
                {
                    if (DataModel::Nullable<uint32_t> val{};
                        WiredAssessedInputVoltage::GetDefault(endpointId, val) == InteractionModel::Status::Success)
                    {
                        if (!val.IsNull())
                        {
                            config.wiredAssessedInputVoltage = val.Value();
                        }
                    }
                }

                SetAttributeDefaultFromEmber(Wired, typename EmberWiredPowerSourceClusterT::PowerSourceStatusEnum, Status, status);
                SetSimpleIntegerDefault(Wired, uint8_t, Order, order);
                SetNullableAttributeDefaultFromEmber(Wired, uint32_t, WiredAssessedInputVoltage, wiredAssessedInputVoltage);
                SetNullableAttributeDefaultFromEmber(Wired, uint16_t, WiredAssessedInputFrequency, wiredAssessedInputFrequency);
                SetNullableAttributeDefaultFromEmber(Wired, uint32_t, WiredAssessedCurrent, wiredAssessedCurrent);
                SetSimpleIntegerDefault(Wired, uint32_t, WiredNominalVoltage, wiredNominalVoltage);
                SetSimpleIntegerDefault(Wired, uint32_t, WiredMaximumCurrent, wiredMaximumCurrent);
                SetSimpleIntegerDefault(Wired, bool, WiredPresent, wiredPresent);

                config.usedOptionalAttributes = optionalAttributeSet;
                if constexpr (batterySupported)
                {
                    auto & gServer = static_cast<LazyRegisteredPowerSourceClusterT &>(gServers[clusterInstanceIndex]);
                    auto & server  = gServer.template emplace<LazyRegisteredWiredSourceClusterT>();
                    server.Create(config);
                    return server.Registration();
                }
                else
                {
                    auto & gServer = static_cast<LazyRegisteredPowerSourceClusterT &>(gServers[clusterInstanceIndex]);
                    gServer.Create(config);
                    return gServer.Registration();
                }
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
                BatReplaceability::GetDefault(endpointId, &replaceability);

                typename EmberBatteryPowerSourceClusterT::ConfigType config(endpointId, description, replaceability,
                                                                            gTimerDelegate);

                SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::PowerSourceStatusEnum, Status,
                                             status);
                SetSimpleIntegerDefault(Battery, uint8_t, Order, order);
                SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatVoltage, batVoltage);
                SetNullableAttributeDefaultFromEmber(Battery, uint8_t, BatPercentRemaining, batPercentRemaining);
                SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatTimeRemaining, batTimeRemaining);
                SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::BatChargeLevelEnum, BatChargeLevel,
                                             batChargeLevel);
                SetSimpleIntegerDefault(Battery, bool, BatReplacementNeeded, batReplacementNeeded);
                SetSimpleIntegerDefault(Battery, bool, BatPresent, batPresent);

                if constexpr (batteryFeatures.Has(Feature::kReplaceable))
                {
                    if (features.Has(Feature::kReplaceable))
                    {
                        CharSpan replacementDescription = GetStringAttribute(BatReplacementDescription, batReplacementDescription);

                        uint8_t quantity;
#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT > 0
                        VerifyOrDie(BatQuantity::GetDefault(endpointId, &quantity) == InteractionModel::Status::Success);
#else  // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT == 0
                        quantity = static_cast<uint8_t>(
                            GetSimpleIntegerDefaultValueDirectlyFromEndpointConfig(endpointId, BatQuantity::Id));
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT == 0
                        config.MakeReplaceable(replacementDescription, quantity);

                        SetAttributeDefaultFromEmber(Battery, typename EmberBatteryPowerSourceClusterT::BatCommonDesignationEnum,
                                                     BatCommonDesignation, batCommonDesignation);

                        if constexpr (EmberBatteryPowerSourceClusterT::supportedOptionalAttributeSet.IsSet(BatANSIDesignation::Id))
                        {
                            config.batANSIDesignation = GetStringAttribute(BatANSIDesignation, batANSIDesignation);
                        }

                        if constexpr (EmberBatteryPowerSourceClusterT::supportedOptionalAttributeSet.IsSet(BatIECDesignation::Id))
                        {
                            config.batIECDesignation = GetStringAttribute(BatIECDesignation, batIECDesignation);
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
                        SetSimpleIntegerDefault(Battery, bool, BatFunctionalWhileCharging, batFunctionalWhileCharging);
                        SetNullableAttributeDefaultFromEmber(Battery, uint32_t, BatChargingCurrent, batChargingCurrent);
                    }
                }

                SetSimpleIntegerDefault(Battery, uint32_t, BatCapacity, batCapacity);

                config.usedOptionalAttributes = optionalAttributeSet;
                if constexpr (wiredSupported)
                {
                    auto & gServer = static_cast<LazyRegisteredPowerSourceClusterT &>(gServers[clusterInstanceIndex]);
                    auto & server  = gServer.template emplace<LazyRegisteredBatterySourceClusterT>();
                    server.Create(config);
                    return server.Registration();
                }
                else
                {
                    auto & gServer = static_cast<LazyRegisteredPowerSourceClusterT &>(gServers[clusterInstanceIndex]);
                    gServer.Create(config);
                    return gServer.Registration();
                }
            }
        }

#undef SetAttributeDefaultFromEmber
#undef SetNullableAttributeDefaultFromEmber
#undef SetSimpleIntegerDefault

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
            if (auto * wiredServer = std::get_if<LazyRegisteredWiredSourceClusterT>(&gServer))
            {
                VerifyOrReturnValue(wiredServer->IsConstructed(), nullptr);
                return &wiredServer->Cluster();
            }
            else
            {
                auto * batteryServer = std::get_if<LazyRegisteredBatterySourceClusterT>(&gServer);
                VerifyOrReturnValue(batteryServer->IsConstructed(), nullptr);
                return &batteryServer->Cluster();
            }
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
            if (auto * wiredServer = std::get_if<LazyRegisteredWiredSourceClusterT>(&gServer))
            {
                wiredServer->Destroy();
            }
            else
            {
                auto * batteryServer = std::get_if<LazyRegisteredBatterySourceClusterT>(&gServer);
                batteryServer->Destroy();
            }
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
    FeatureMap::GetDefault(endpointId, &featureBits);
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
