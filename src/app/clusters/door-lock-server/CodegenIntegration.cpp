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

#include <app/clusters/door-lock-server/CodegenIntegration.h>

#include <app/ConcreteAttributePath.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/static-cluster-config/DoorLock.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>

#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

using namespace DoorLock;

constexpr size_t kDoorLockFixedClusterCount = DoorLock::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kDoorLockMaxClusterCount   = kDoorLockFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<DoorLock::DoorLockCluster> gServers[kDoorLockMaxClusterCount];

DefaultTimerDelegate gTimerDelegate;

// Per-endpoint application wiring (delegate and configuration overrides).
struct EndpointEntry
{
    EndpointId endpointId         = kInvalidEndpointId;
    DoorLock::Delegate * delegate = nullptr;
    DoorLock::ServerConfigOverrides overrides;
};
EndpointEntry gEndpointEntries[kDoorLockMaxClusterCount];

/// Delegate used when the application does not register one: answers all
/// Aliro attributes with neutral defaults (no reader configuration).
class NoOpDoorLockDelegate : public DoorLock::Delegate
{
public:
    CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) override
    {
        verificationKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) override
    {
        groupIdentifier.reduce_size(0);
        return CHIP_NO_ERROR;
    }
    // Not nullable: the empty (all-zero) subidentifier for a stub configuration.
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) override
    {
        memset(groupSubIdentifier.data(), 0, groupSubIdentifier.size());
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) override
    {
        groupResolvingKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    uint8_t GetAliroBLEAdvertisingVersion() override { return 0; }
    uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override { return 0; }
    uint16_t GetNumberOfAliroEndpointKeysSupported() override { return 0; }
    CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey, const ByteSpan & groupIdentifier,
                                    const Optional<ByteSpan> & groupResolvingKey) override
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    CHIP_ERROR ClearAliroReaderConfig() override { return CHIP_NO_ERROR; }
};

NoOpDoorLockDelegate gNoOpDelegate;

EndpointEntry * GetEntryForEndpoint(EndpointId endpointId)
{
    for (EndpointEntry & entry : gEndpointEntries)
    {
        if (entry.endpointId == endpointId)
        {
            return &entry;
        }
    }
    return nullptr;
}

DoorLock::Delegate * GetDelegateForEndpoint(EndpointId endpointId)
{
    if (EndpointEntry * entry = GetEntryForEndpoint(endpointId))
    {
        if (entry->delegate != nullptr)
        {
            return entry->delegate;
        }
    }
    return &gNoOpDelegate;
}

EndpointEntry & FindOrCreateEndpointEntry(EndpointId endpointId)
{
    if (EndpointEntry * entry = GetEntryForEndpoint(endpointId))
    {
        return *entry;
    }
    for (EndpointEntry & entry : gEndpointEntries)
    {
        if (entry.endpointId == kInvalidEndpointId)
        {
            entry.endpointId = endpointId;
            return entry;
        }
    }
    VerifyOrDieWithMsg(false, NotSpecified, "DoorLock: no free endpoint entry for endpoint %u", endpointId);
}

DoorLock::OptionalAttributes CreateOptionalAttributes()
{
    using namespace DoorLock::Attributes;
    using DoorLock::StaticApplicationConfig::IsAttributeEnabledOnSomeEndpoint;

    DoorLock::OptionalAttributes optionalAttributes;
    optionalAttributes.language                     = IsAttributeEnabledOnSomeEndpoint(Language::Id);
    optionalAttributes.ledSettings                  = IsAttributeEnabledOnSomeEndpoint(LEDSettings::Id);
    optionalAttributes.autoRelockTime               = IsAttributeEnabledOnSomeEndpoint(AutoRelockTime::Id);
    optionalAttributes.soundVolume                  = IsAttributeEnabledOnSomeEndpoint(SoundVolume::Id);
    optionalAttributes.defaultConfigurationRegister = IsAttributeEnabledOnSomeEndpoint(DefaultConfigurationRegister::Id);
    optionalAttributes.enableLocalProgramming       = IsAttributeEnabledOnSomeEndpoint(EnableLocalProgramming::Id);
    optionalAttributes.enableOneTouchLocking        = IsAttributeEnabledOnSomeEndpoint(EnableOneTouchLocking::Id);
    optionalAttributes.enableInsideStatusLED        = IsAttributeEnabledOnSomeEndpoint(EnableInsideStatusLED::Id);
    optionalAttributes.enablePrivacyModeButton      = IsAttributeEnabledOnSomeEndpoint(EnablePrivacyModeButton::Id);
    optionalAttributes.localProgrammingFeatures     = IsAttributeEnabledOnSomeEndpoint(LocalProgrammingFeatures::Id);
    return optionalAttributes;
}

/// TODO(DO NOT MERGE): the values below mirror the legacy zap defaults of the
/// lock app. In the real conversion they must either come from the generated
/// static application configuration or from ZAP attribute defaults, once the
/// legacy file is fully transitioned.
void ApplyDefaultConfigValues(DoorLock::Config & config)
{
    static constexpr char kDefaultLanguage[] = "en";

    // lockState intentionally left null, matching Legacy InitServer (SetNull).
    config.lockType                            = DlLockType::kDeadBolt;
    config.supportedOperatingModes             = BitMask<DlSupportedOperatingModes>(0xFFF6);
    config.numberOfTotalUsersSupported         = 10;
    config.numberOfPINUsersSupported           = 10;
    config.numberOfRFIDUsersSupported          = 10;
    config.numberOfWeekDaySchedulesPerUser     = 10;
    config.numberOfYearDaySchedulesPerUser     = 10;
    config.numberOfHolidaySchedulesSupported   = 10;
    config.numberOfCredentialsSupportedPerUser = 5;
    config.credentialRulesSupport              = BitMask<DlCredentialRuleMask>(1);
    config.maxPINCodeLength                    = 8;
    config.minPINCodeLength                    = 6;
    config.maxRFIDCodeLength                   = 20;
    config.minRFIDCodeLength                   = 10;
    config.language                            = CharSpan::fromCharString(kDefaultLanguage);
    config.autoRelockTime                      = 60;
    config.wrongCodeEntryLimit                 = 3;
    config.userCodeTemporaryDisableTime        = 10;
}

void ApplyOverrides(const DoorLock::ServerConfigOverrides & overrides, DoorLock::Config & config)
{
    if (overrides.features.has_value())
    {
        config.features = overrides.features.value();
    }
    if (overrides.numberOfTotalUsersSupported.has_value())
    {
        config.numberOfTotalUsersSupported = overrides.numberOfTotalUsersSupported.value();
    }
    if (overrides.numberOfPINUsersSupported.has_value())
    {
        config.numberOfPINUsersSupported = overrides.numberOfPINUsersSupported.value();
    }
    if (overrides.numberOfRFIDUsersSupported.has_value())
    {
        config.numberOfRFIDUsersSupported = overrides.numberOfRFIDUsersSupported.value();
    }
    if (overrides.numberOfCredentialsSupportedPerUser.has_value())
    {
        config.numberOfCredentialsSupportedPerUser = overrides.numberOfCredentialsSupportedPerUser.value();
    }
    if (overrides.autoRelockTime.has_value())
    {
        config.autoRelockTime = overrides.autoRelockTime.value();
    }
    if (overrides.lockState.has_value())
    {
        config.lockState = overrides.lockState.value();
    }
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        DoorLock::Config config(gTimerDelegate);
        config.features           = BitFlags<DoorLock::Feature>(featureMap);
        config.optionalAttributes = CreateOptionalAttributes();
        ApplyDefaultConfigValues(config);

        EndpointEntry * entry = GetEntryForEndpoint(endpointId);
        if (entry != nullptr)
        {
            ApplyOverrides(entry->overrides, config);
        }

        gServers[clusterInstanceIndex].Create(endpointId, *GetDelegateForEndpoint(endpointId), config);
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

// ==================== Generated cluster callbacks ====================
// These are the callbacks used by the ZAP generated glue for Door Lock
// clusters.

void MatterDoorLockClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // NOTE: fetchOptionalAttributes is false because door lock attribute IDs
    // exceed the 32-bit range supported by the generic bit-fetch, see
    // ClusterIntegration.cpp. Optional attributes are derived from the static
    // application configuration instead.
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = DoorLock::Id,
            .fixedClusterInstanceCount = kDoorLockFixedClusterCount,
            .maxClusterInstanceCount   = kDoorLockMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterDoorLockClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = DoorLock::Id,
            .fixedClusterInstanceCount = kDoorLockFixedClusterCount,
            .maxClusterInstanceCount   = kDoorLockMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterDoorLockPluginServerInitCallback()
{
    // TODO: the legacy implementation registered a fabric table delegate used
    // to clear lock bookkeeping on fabric removal. This experimental
    // integration does not provide the equivalent behavior yet.
}

void MatterDoorLockClusterServerAttributeChangedCallback(const ConcreteAttributePath &) {}
void MatterDoorLockClusterServerShutdownCallback(EndpointId) {}
Protocols::InteractionModel::Status
MatterDoorLockClusterServerPreAttributeChangedCallback(const ConcreteAttributePath &, EmberAfAttributeType, uint16_t, uint8_t *)
{
    return Protocols::InteractionModel::Status::Success;
}

namespace chip::app::Clusters::DoorLock {

void SetDelegate(EndpointId endpointId, Delegate * delegate)
{
    EndpointEntry & entry = FindOrCreateEndpointEntry(endpointId);
    entry.delegate        = delegate;
}

void ApplyServerConfigOverrides(EndpointId endpointId, const ServerConfigOverrides & overrides)
{
    EndpointEntry & entry = FindOrCreateEndpointEntry(endpointId);
    entry.overrides       = overrides;
}

DoorLockCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = DoorLock::Id,
            .fixedClusterInstanceCount = kDoorLockFixedClusterCount,
            .maxClusterInstanceCount   = kDoorLockMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<DoorLockCluster *>(cluster);
}

} // namespace chip::app::Clusters::DoorLock
