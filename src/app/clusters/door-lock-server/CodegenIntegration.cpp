/**
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

#include <app/clusters/door-lock-server/CodegenIntegration.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/clusters/door-lock-server/DoorLockCluster.h>
#include <app/clusters/door-lock-server/door-lock-attr-compat.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>

#include <cstring>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using namespace chip::app::Clusters::DoorLock::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kDoorLockFixedClusterCount = MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT;
constexpr size_t kDoorLockMaxClusterCount   = kDoorLockFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<DoorLockCluster> gServers[kDoorLockMaxClusterCount];

DefaultTimerDelegate gTimerDelegate;

/// Per-endpoint application state: the (legacy-style) delegate used for Aliro
/// attribute reads and runtime configuration overrides. Filled in by
/// application code before endpoint initialization (during application init or
/// the legacy `SetDelegate` compat call).
struct AppEndpointEntry
{
    EndpointId endpointId = kInvalidEndpointId;
    Delegate * delegate   = nullptr;
    ServerConfigOverrides overrides;
};

AppEndpointEntry gAppEndpoints[kDoorLockMaxClusterCount];

AppEndpointEntry & FindOrCreateAppEndpointEntry(EndpointId endpointId)
{
    VerifyOrDie(endpointId != kInvalidEndpointId);
    for (auto & entry : gAppEndpoints)
    {
        if (entry.endpointId == endpointId)
        {
            return entry;
        }
    }
    for (auto & entry : gAppEndpoints)
    {
        if (entry.endpointId == kInvalidEndpointId)
        {
            entry.endpointId = endpointId;
            return entry;
        }
    }
    ChipLogError(AppServer, "Too many door lock endpoint registrations");
    VerifyOrDie(false);
    return gAppEndpoints[0];
}

Delegate * FindAppEndpointDelegate(EndpointId endpointId)
{
    for (auto & entry : gAppEndpoints)
    {
        if (entry.endpointId == endpointId)
        {
            return entry.delegate;
        }
    }
    return nullptr;
}

/// Compatibility delegate for the ember application-hook contract
/// (`emberAfPluginDoorLockOnDoorLockCommand` / `OnDoorUnlockCommand` /
/// `OnDoorUnboltCommand` / `OnAutoRelock` / `LockoutStarted`): hardware
/// actuation continues to flow through the application hooks, while Aliro
/// attribute reads forward to the per-endpoint delegate when one is installed.
class AppHooksDoorLockDelegate : public Delegate
{
public:
    AppHooksDoorLockDelegate() = default;

    void Configure(EndpointId endpointId) { mEndpointId = endpointId; }

    std::optional<OperationErrorEnum> HandleDoorLockCommand(const LockOperationRequest & request) override
    {
        OperationErrorEnum err = OperationErrorEnum::kUnspecified;
        bool success = emberAfPluginDoorLockOnDoorLockCommand(mEndpointId, request.fabricIdx, request.nodeId, request.pinCode, err);
        return success ? std::optional<OperationErrorEnum>() : std::optional<OperationErrorEnum>(err);
    }

    std::optional<OperationErrorEnum> HandleDoorUnlockCommand(const LockOperationRequest & request) override
    {
        OperationErrorEnum err = OperationErrorEnum::kUnspecified;
        bool success =
            emberAfPluginDoorLockOnDoorUnlockCommand(mEndpointId, request.fabricIdx, request.nodeId, request.pinCode, err);
        return success ? std::optional<OperationErrorEnum>() : std::optional<OperationErrorEnum>(err);
    }

    std::optional<OperationErrorEnum> HandleDoorUnboltCommand(const LockOperationRequest & request) override
    {
        OperationErrorEnum err = OperationErrorEnum::kUnspecified;
        bool success =
            emberAfPluginDoorLockOnDoorUnboltCommand(mEndpointId, request.fabricIdx, request.nodeId, request.pinCode, err);
        return success ? std::optional<OperationErrorEnum>() : std::optional<OperationErrorEnum>(err);
    }

    void OnAutoRelock() override { emberAfPluginDoorLockOnAutoRelock(mEndpointId); }

    void OnLockoutStarted(chip::System::Clock::Timestamp lockoutEndTimestamp) override
    {
        emberAfPluginDoorLockLockoutStarted(mEndpointId, lockoutEndTimestamp);
    }

    CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) override
    {
        return ForwardAliroRead(&Delegate::GetAliroReaderVerificationKey, verificationKey);
    }
    CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) override
    {
        return ForwardAliroRead(&Delegate::GetAliroReaderGroupIdentifier, groupIdentifier);
    }
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) override
    {
        return ForwardAliroRead(&Delegate::GetAliroReaderGroupSubIdentifier, groupSubIdentifier);
    }
    CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override
    {
        Delegate * aliroDelegate = AliroDelegate();
        VerifyOrReturnValue(aliroDelegate != nullptr, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return aliroDelegate->GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(index, protocolVersion);
    }
    CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) override
    {
        return ForwardAliroRead(&Delegate::GetAliroGroupResolvingKey, groupResolvingKey);
    }
    CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override
    {
        Delegate * aliroDelegate = AliroDelegate();
        VerifyOrReturnValue(aliroDelegate != nullptr, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return aliroDelegate->GetAliroSupportedBLEUWBProtocolVersionAtIndex(index, protocolVersion);
    }
    uint8_t GetAliroBLEAdvertisingVersion() override
    {
        Delegate * aliroDelegate = AliroDelegate();
        return aliroDelegate != nullptr ? aliroDelegate->GetAliroBLEAdvertisingVersion() : 0;
    }
    uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override
    {
        Delegate * aliroDelegate = AliroDelegate();
        return aliroDelegate != nullptr ? aliroDelegate->GetNumberOfAliroCredentialIssuerKeysSupported() : 0;
    }
    uint16_t GetNumberOfAliroEndpointKeysSupported() override
    {
        Delegate * aliroDelegate = AliroDelegate();
        return aliroDelegate != nullptr ? aliroDelegate->GetNumberOfAliroEndpointKeysSupported() : 0;
    }
    CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey, const ByteSpan & groupIdentifier,
                                    const Optional<ByteSpan> & groupResolvingKey) override
    {
        Delegate * aliroDelegate = AliroDelegate();
        VerifyOrReturnValue(aliroDelegate != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);
        return aliroDelegate->SetAliroReaderConfig(signingKey, verificationKey, groupIdentifier, groupResolvingKey);
    }
    CHIP_ERROR ClearAliroReaderConfig() override
    {
        Delegate * aliroDelegate = AliroDelegate();
        VerifyOrReturnValue(aliroDelegate != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);
        return aliroDelegate->ClearAliroReaderConfig();
    }

private:
    /// The Aliro delegate is looked up on every read: application code installs
    /// it during its cluster init hook, which runs after the cluster is created.
    Delegate * AliroDelegate() { return FindAppEndpointDelegate(mEndpointId); }

    CHIP_ERROR ForwardAliroRead(CHIP_ERROR (Delegate::*getter)(MutableByteSpan &), MutableByteSpan & span)
    {
        Delegate * aliroDelegate = AliroDelegate();
        if (aliroDelegate != nullptr)
        {
            return (aliroDelegate->*getter)(span);
        }
        // Not configured: report the empty span, which the cluster encodes as a
        // null attribute value (the group sub-identifier is not nullable, so
        // the cluster rejects... see DoorLockCluster::ReadAttribute).
        span.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    EndpointId mEndpointId = kInvalidEndpointId;
};

AppHooksDoorLockDelegate gDelegates[kDoorLockMaxClusterCount];

bool IsOptionalAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    return emberAfContainsAttribute(endpointId, DoorLock::Id, attributeId);
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override;
    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override;
    void ReleaseRegistration(unsigned clusterInstanceIndex) override;
};

} // namespace

ServerClusterRegistration & IntegrationDelegate::CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                                    uint32_t optionalAttributeBits, uint32_t featureMap)
{
    (void) optionalAttributeBits;
    AppEndpointEntry & appEndpoint = FindOrCreateAppEndpointEntry(endpointId);

    DoorLock::Config config(gTimerDelegate);
    config.features = appEndpoint.overrides.features.value_or(BitFlags<Feature>(featureMap));

    // Attribute conformance ([DPS], [!USR & PIN], [USR] and plain optional
    // attributes): attributes with ids beyond the 32-bit mask are derived from
    // the static application configuration. The cluster feature-gates the
    // [DPS]/[!USR]/[USR] ones further.
    config.optionalAttributes.language       = IsOptionalAttributeEnabled(endpointId, Language::Id);
    config.optionalAttributes.ledSettings    = IsOptionalAttributeEnabled(endpointId, LEDSettings::Id);
    config.optionalAttributes.autoRelockTime = IsOptionalAttributeEnabled(endpointId, AutoRelockTime::Id);
    config.optionalAttributes.soundVolume    = IsOptionalAttributeEnabled(endpointId, SoundVolume::Id);
    config.optionalAttributes.defaultConfigurationRegister =
        IsOptionalAttributeEnabled(endpointId, DefaultConfigurationRegister::Id);
    config.optionalAttributes.enableLocalProgramming   = IsOptionalAttributeEnabled(endpointId, EnableLocalProgramming::Id);
    config.optionalAttributes.enableOneTouchLocking    = IsOptionalAttributeEnabled(endpointId, EnableOneTouchLocking::Id);
    config.optionalAttributes.enableInsideStatusLED    = IsOptionalAttributeEnabled(endpointId, EnableInsideStatusLED::Id);
    config.optionalAttributes.enablePrivacyModeButton  = IsOptionalAttributeEnabled(endpointId, EnablePrivacyModeButton::Id);
    config.optionalAttributes.localProgrammingFeatures = IsOptionalAttributeEnabled(endpointId, LocalProgrammingFeatures::Id);
    config.optionalAttributes.doorOpenEvents           = IsOptionalAttributeEnabled(endpointId, DoorOpenEvents::Id);
    config.optionalAttributes.doorClosedEvents         = IsOptionalAttributeEnabled(endpointId, DoorClosedEvents::Id);
    config.optionalAttributes.openPeriod               = IsOptionalAttributeEnabled(endpointId, OpenPeriod::Id);
    config.optionalAttributes.sendPINOverTheAir        = IsOptionalAttributeEnabled(endpointId, SendPINOverTheAir::Id);
    config.optionalAttributes.expiringUserTimeout      = IsOptionalAttributeEnabled(endpointId, ExpiringUserTimeout::Id);

    // Initial attribute values: ZAP configured defaults where available, with
    // spec/legacy-compatible fallbacks otherwise.
    DataModel::Nullable<DlLockState> lockState;
    LockState::GetDefaultOr(endpointId, lockState, DataModel::NullNullable);

    DataModel::Nullable<DoorStateEnum> doorState;
    DoorState::GetDefaultOr(endpointId, doorState, DataModel::NullNullable);

    uint32_t doorOpenEvents = 0;
    DoorOpenEvents::GetDefaultOr(endpointId, doorOpenEvents, 0);
    uint32_t doorClosedEvents = 0;
    DoorClosedEvents::GetDefaultOr(endpointId, doorClosedEvents, 0);
    uint16_t openPeriod = 0;
    OpenPeriod::GetDefaultOr(endpointId, openPeriod, 0);

    // Parity with the ember default (0/false) for attributes without a
    // configured default, even where the spec default of the Config struct
    // would be different.
    bool actuatorEnabled = false;
    ActuatorEnabled::GetDefaultOr(endpointId, actuatorEnabled, false);

    OperatingModeEnum operatingMode = OperatingModeEnum::kNormal;
    OperatingMode::GetDefaultOr(endpointId, operatingMode, OperatingModeEnum::kNormal);

    CharSpan language;
    Language::GetDefaultOr(endpointId, language, CharSpan());

    uint8_t ledSettings = 0;
    LEDSettings::GetDefaultOr(endpointId, ledSettings, 0);
    uint32_t autoRelockTime = 0;
    AutoRelockTime::GetDefaultOr(endpointId, autoRelockTime, 0);
    uint8_t soundVolume = 0;
    SoundVolume::GetDefaultOr(endpointId, soundVolume, 0);
    BitMask<DlDefaultConfigurationRegister> defaultConfigurationRegister;
    DefaultConfigurationRegister::GetDefaultOr(endpointId, defaultConfigurationRegister, BitMask<DlDefaultConfigurationRegister>());
    bool enableLocalProgramming = true;
    EnableLocalProgramming::GetDefaultOr(endpointId, enableLocalProgramming, true);
    bool enableOneTouchLocking = false;
    EnableOneTouchLocking::GetDefaultOr(endpointId, enableOneTouchLocking, false);
    bool enableInsideStatusLED = false;
    EnableInsideStatusLED::GetDefaultOr(endpointId, enableInsideStatusLED, false);
    bool enablePrivacyModeButton = false;
    EnablePrivacyModeButton::GetDefaultOr(endpointId, enablePrivacyModeButton, false);
    BitMask<DlLocalProgrammingFeatures> localProgrammingFeatures;
    LocalProgrammingFeatures::GetDefaultOr(endpointId, localProgrammingFeatures, BitMask<DlLocalProgrammingFeatures>());
    uint8_t wrongCodeEntryLimit = 3;
    WrongCodeEntryLimit::GetDefaultOr(endpointId, wrongCodeEntryLimit, 3);
    uint8_t userCodeTemporaryDisableTime = 10;
    UserCodeTemporaryDisableTime::GetDefaultOr(endpointId, userCodeTemporaryDisableTime, 10);
    bool sendPINOverTheAir = false;
    SendPINOverTheAir::GetDefaultOr(endpointId, sendPINOverTheAir, false);
    bool requirePINforRemoteOperation = false;
    RequirePINforRemoteOperation::GetDefaultOr(endpointId, requirePINforRemoteOperation, false);
    uint16_t expiringUserTimeout = 10;
    ExpiringUserTimeout::GetDefaultOr(endpointId, expiringUserTimeout, 10);

    uint16_t numberOfTotalUsersSupported = 0;
    NumberOfTotalUsersSupported::GetDefaultOr(endpointId, numberOfTotalUsersSupported, 0);
    uint16_t numberOfPINUsersSupported = 0;
    NumberOfPINUsersSupported::GetDefaultOr(endpointId, numberOfPINUsersSupported, 0);
    uint16_t numberOfRFIDUsersSupported = 0;
    NumberOfRFIDUsersSupported::GetDefaultOr(endpointId, numberOfRFIDUsersSupported, 0);
    uint8_t numberOfWeekDaySchedulesPerUser = 0;
    NumberOfWeekDaySchedulesSupportedPerUser::GetDefaultOr(endpointId, numberOfWeekDaySchedulesPerUser, 0);
    uint8_t numberOfYearDaySchedulesPerUser = 0;
    NumberOfYearDaySchedulesSupportedPerUser::GetDefaultOr(endpointId, numberOfYearDaySchedulesPerUser, 0);
    uint8_t numberOfHolidaySchedulesSupported = 0;
    NumberOfHolidaySchedulesSupported::GetDefaultOr(endpointId, numberOfHolidaySchedulesSupported, 0);
    uint8_t numberOfCredentialsSupportedPerUser = 0;
    NumberOfCredentialsSupportedPerUser::GetDefaultOr(endpointId, numberOfCredentialsSupportedPerUser, 0);
    BitMask<DlCredentialRuleMask> credentialRulesSupport;
    CredentialRulesSupport::GetDefaultOr(endpointId, credentialRulesSupport, BitMask<DlCredentialRuleMask>());
    uint8_t maxPINCodeLength = 0;
    MaxPINCodeLength::GetDefaultOr(endpointId, maxPINCodeLength, 0);
    uint8_t minPINCodeLength = 0;
    MinPINCodeLength::GetDefaultOr(endpointId, minPINCodeLength, 0);
    uint8_t maxRFIDCodeLength = 0;
    MaxRFIDCodeLength::GetDefaultOr(endpointId, maxRFIDCodeLength, 0);
    uint8_t minRFIDCodeLength = 0;
    MinRFIDCodeLength::GetDefaultOr(endpointId, minRFIDCodeLength, 0);

    // Application overrides win over the ZAP defaults.
    if (appEndpoint.overrides.lockState.has_value())
    {
        lockState = appEndpoint.overrides.lockState.value();
    }
    if (appEndpoint.overrides.autoRelockTime.has_value())
    {
        autoRelockTime = appEndpoint.overrides.autoRelockTime.value();
    }
    if (appEndpoint.overrides.features.has_value())
    {
        config.features = appEndpoint.overrides.features.value();
    }
    if (appEndpoint.overrides.numberOfTotalUsersSupported.has_value())
    {
        numberOfTotalUsersSupported = appEndpoint.overrides.numberOfTotalUsersSupported.value();
    }
    if (appEndpoint.overrides.numberOfPINUsersSupported.has_value())
    {
        numberOfPINUsersSupported = appEndpoint.overrides.numberOfPINUsersSupported.value();
    }
    if (appEndpoint.overrides.numberOfRFIDUsersSupported.has_value())
    {
        numberOfRFIDUsersSupported = appEndpoint.overrides.numberOfRFIDUsersSupported.value();
    }
    if (appEndpoint.overrides.numberOfCredentialsSupportedPerUser.has_value())
    {
        numberOfCredentialsSupportedPerUser = appEndpoint.overrides.numberOfCredentialsSupportedPerUser.value();
    }

    config.lockState                           = lockState;
    config.doorState                           = doorState;
    config.doorOpenEvents                      = doorOpenEvents;
    config.doorClosedEvents                    = doorClosedEvents;
    config.openPeriod                          = openPeriod;
    config.actuatorEnabled                     = actuatorEnabled;
    config.operatingMode                       = operatingMode;
    config.language                            = language;
    config.ledSettings                         = ledSettings;
    config.autoRelockTime                      = autoRelockTime;
    config.soundVolume                         = soundVolume;
    config.defaultConfigurationRegister        = defaultConfigurationRegister;
    config.enableLocalProgramming              = enableLocalProgramming;
    config.enableOneTouchLocking               = enableOneTouchLocking;
    config.enableInsideStatusLED               = enableInsideStatusLED;
    config.enablePrivacyModeButton             = enablePrivacyModeButton;
    config.localProgrammingFeatures            = localProgrammingFeatures;
    config.wrongCodeEntryLimit                 = wrongCodeEntryLimit;
    config.userCodeTemporaryDisableTime        = userCodeTemporaryDisableTime;
    config.sendPINOverTheAir                   = sendPINOverTheAir;
    config.requirePINforRemoteOperation        = requirePINforRemoteOperation;
    config.expiringUserTimeout                 = expiringUserTimeout;
    config.numberOfTotalUsersSupported         = numberOfTotalUsersSupported;
    config.numberOfPINUsersSupported           = numberOfPINUsersSupported;
    config.numberOfRFIDUsersSupported          = numberOfRFIDUsersSupported;
    config.numberOfWeekDaySchedulesPerUser     = numberOfWeekDaySchedulesPerUser;
    config.numberOfYearDaySchedulesPerUser     = numberOfYearDaySchedulesPerUser;
    config.numberOfHolidaySchedulesSupported   = numberOfHolidaySchedulesSupported;
    config.numberOfCredentialsSupportedPerUser = numberOfCredentialsSupportedPerUser;
    config.credentialRulesSupport              = credentialRulesSupport;
    config.maxPINCodeLength                    = maxPINCodeLength;
    config.minPINCodeLength                    = minPINCodeLength;
    config.maxRFIDCodeLength                   = maxRFIDCodeLength;
    config.minRFIDCodeLength                   = minRFIDCodeLength;

    gDelegates[clusterInstanceIndex].Configure(endpointId);
    gServers[clusterInstanceIndex].Create(endpointId, gDelegates[clusterInstanceIndex], config);
    return gServers[clusterInstanceIndex].Registration();
}

ServerClusterInterface * IntegrationDelegate::FindRegistration(unsigned clusterInstanceIndex)
{
    VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
    return &gServers[clusterInstanceIndex].Cluster();
}

void IntegrationDelegate::ReleaseRegistration(unsigned clusterInstanceIndex)
{
    gServers[clusterInstanceIndex].Destroy();
}

void MatterDoorLockClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

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

void MatterDoorLockPluginServerInitCallback() {}
void MatterDoorLockPluginServerShutdownCallback() {}

// =============================================================================
// Compatibility layer: the legacy DoorLockServer keeps its instance API and
// forwards to the code-driven cluster, so application integrations continue to
// compile and run while they migrate.
// =============================================================================

namespace {

DoorLockServer gLegacyDoorLockServer;

DoorLockCluster * FindClusterForCompat(EndpointId endpointId)
{
    return DoorLock::FindClusterOnEndpoint(endpointId);
}

} // namespace

DoorLockServer & DoorLockServer::Instance()
{
    return gLegacyDoorLockServer;
}

void DoorLockServer::InitServer(chip::EndpointId endpointId)
{
    (void) endpointId;
    // Registration is handled by MatterDoorLockClusterInitCallback; the legacy
    // startup marked the actuator as enabled, so preserve that behavior.
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    if (cluster != nullptr)
    {
        LogErrorOnFailure(cluster->SetActuatorEnabled(true));
    }
}

chip::BitFlags<DoorLock::Feature> DoorLockServer::GetFeatures(chip::EndpointId endpointId)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    if (cluster != nullptr)
    {
        return cluster->Features();
    }
    return chip::BitFlags<DoorLock::Feature>(0);
}

bool DoorLockServer::SetLockState(
    chip::EndpointId endpointId, DoorLock::DlLockState newLockState, DoorLock::OperationSourceEnum opSource,
    const chip::app::DataModel::Nullable<uint16_t> & userIndex,
    const chip::app::DataModel::Nullable<chip::app::DataModel::List<const LockOpCredentials>> & credentials,
    const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
    const chip::app::DataModel::Nullable<chip::NodeId> & nodeId)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);

    Span<const LockOpCredentials> credentialsSpan;
    if (!credentials.IsNull())
    {
        credentialsSpan = Span<const LockOpCredentials>(credentials.Value().data(), credentials.Value().size());
    }

    return cluster->SetLockState(newLockState, opSource, userIndex, credentialsSpan, fabricIdx, nodeId) == CHIP_NO_ERROR;
}

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, DoorLock::DlLockState newLockState)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);

    return cluster->SetLockState(newLockState, DoorLock::OperationSourceEnum::kManual, chip::app::DataModel::Nullable<uint16_t>(),
                                 Span<const LockOpCredentials>(), chip::app::DataModel::Nullable<chip::FabricIndex>(),
                                 chip::app::DataModel::Nullable<chip::NodeId>(),
                                 /*sendLockOperationEvent=*/false) == CHIP_NO_ERROR;
}

bool DoorLockServer::SetActuatorEnabled(chip::EndpointId endpointId, bool newActuatorState)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);
    return cluster->SetActuatorEnabled(newActuatorState) == CHIP_NO_ERROR;
}

bool DoorLockServer::SetDoorState(chip::EndpointId endpointId, DoorLock::DoorStateEnum newDoorState)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);
    return cluster->SetDoorState(chip::app::DataModel::Nullable<DoorLock::DoorStateEnum>(newDoorState)) == CHIP_NO_ERROR;
}

bool DoorLockServer::SetAutoRelockTime(chip::EndpointId endpointId, uint32_t newAutoRelockTimeSec)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);
    return cluster->SetAutoRelockTime(newAutoRelockTimeSec) == CHIP_NO_ERROR;
}

bool DoorLockServer::HandleWrongCodeEntry(chip::EndpointId endpointId)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);
    return cluster->HandleWrongCodeEntry() == CHIP_NO_ERROR;
}

bool DoorLockServer::GetAutoRelockTime(chip::EndpointId endpointId, uint32_t & autoRelockTime)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);
    autoRelockTime = cluster->GetAutoRelockTime();
    return true;
}

bool DoorLockServer::GetNumberOfUserSupported(chip::EndpointId endpointId, uint16_t & numberOfUsersSupported)
{
    return Attributes::NumberOfTotalUsersSupported::GetDefault(endpointId, numberOfUsersSupported) == Status::Success;
}

bool DoorLockServer::GetNumberOfPINCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfPINCredentials)
{
    return Attributes::NumberOfPINUsersSupported::GetDefault(endpointId, numberOfPINCredentials) == Status::Success;
}

bool DoorLockServer::GetNumberOfRFIDCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfRFIDCredentials)
{
    return Attributes::NumberOfRFIDUsersSupported::GetDefault(endpointId, numberOfRFIDCredentials) == Status::Success;
}

bool DoorLockServer::GetNumberOfWeekDaySchedulesPerUserSupported(chip::EndpointId endpointId,
                                                                 uint8_t & numberOfWeekDaySchedulesPerUser)
{
    return Attributes::NumberOfWeekDaySchedulesSupportedPerUser::GetDefault(endpointId, numberOfWeekDaySchedulesPerUser) ==
        Status::Success;
}

bool DoorLockServer::GetNumberOfYearDaySchedulesPerUserSupported(chip::EndpointId endpointId,
                                                                 uint8_t & numberOfYearDaySchedulesPerUser)
{
    return Attributes::NumberOfYearDaySchedulesSupportedPerUser::GetDefault(endpointId, numberOfYearDaySchedulesPerUser) ==
        Status::Success;
}

bool DoorLockServer::GetNumberOfCredentialsSupportedPerUser(chip::EndpointId endpointId,
                                                            uint8_t & numberOfCredentialsSupportedPerUser)
{
    return Attributes::NumberOfCredentialsSupportedPerUser::GetDefault(endpointId, numberOfCredentialsSupportedPerUser) ==
        Status::Success;
}

bool DoorLockServer::GetNumberOfHolidaySchedulesSupported(chip::EndpointId endpointId, uint8_t & numberOfHolidaySchedules)
{
    return Attributes::NumberOfHolidaySchedulesSupported::GetDefault(endpointId, numberOfHolidaySchedules) == Status::Success;
}

bool DoorLockServer::SendLockAlarmEvent(chip::EndpointId endpointId, DoorLock::AlarmCodeEnum alarmCode)
{
    DoorLockCluster * cluster = FindClusterForCompat(endpointId);
    VerifyOrReturnValue(cluster != nullptr, false);
    return cluster->SendLockAlarmEvent(alarmCode) == CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockServer::SetDelegate(chip::EndpointId endpointId, DoorLock::Delegate * delegate)
{
    DoorLock::SetDelegate(endpointId, delegate);
    return CHIP_NO_ERROR;
}

// The compat object is never registered as an AttributeAccessInterface (the
// code-driven cluster serves attributes through the data-model provider), but
// the legacy class declaration overrides this pure virtual, so a concrete
// definition is still required to build the vtable and construct Instance().
CHIP_ERROR DoorLockServer::Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
{
    (void) aPath;
    (void) aEncoder;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

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

void SetDelegate(EndpointId endpointId, Delegate * delegate)
{
    FindOrCreateAppEndpointEntry(endpointId).delegate = delegate;
}

void ApplyServerConfigOverrides(EndpointId endpointId, const ServerConfigOverrides & overrides)
{
    FindOrCreateAppEndpointEntry(endpointId).overrides = overrides;
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
// =============================================================================
// Compatibility accessors (door-lock-attr-compat.h): the generated ember
// accessors are not produced for code-driven clusters, so the legacy
// application state access is routed into the code-driven cluster here.
// Fixed-configuration writes (lock type, feature map, capacities) are no-ops:
// those values come from the endpoint configuration.
// =============================================================================

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {
namespace Attributes {

namespace {

DoorLockCluster * CompatCluster(EndpointId endpointId)
{
    return DoorLock::FindClusterOnEndpoint(endpointId);
}

/// The legacy ember attribute write also fired the platform
/// MatterPostAttributeChangeCallback hook; some applications actuate the bolt
/// from there, so the compat accessors keep firing it.
void FireLegacyPostChange(EndpointId endpoint, AttributeId attributeId, EmberAfAttributeType type, uint8_t rawValue)
{
    constexpr uint16_t kOneByte = 1;
    MatterPostAttributeChangeCallback(ConcreteAttributePath(endpoint, DoorLock::Id, attributeId), static_cast<uint8_t>(type),
                                      kOneByte, &rawValue);
}

Status FixedConfigWrite(EndpointId endpointId, const char * attributeName)
{
    ChipLogProgress(Zcl, "Door Lock %s is configured by the endpoint configuration, ignoring the write [endpoint=%d]",
                    attributeName, endpointId);
    return Status::Success;
}

} // namespace

namespace LockState {

Status Get(EndpointId endpoint, DataModel::Nullable<DlLockState> & value)
{
    DoorLockCluster * cluster = CompatCluster(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::NotFound);
    value = cluster->GetLockStateNullable();
    return Status::Success;
}

Status Set(EndpointId endpoint, DlLockState value)
{
    DoorLockCluster * cluster = CompatCluster(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::NotFound);
    VerifyOrReturnError(cluster->SetLockState(value, OperationSourceEnum::kManual, DataModel::NullNullable,
                                              Span<const CredentialStruct>(), DataModel::Nullable<FabricIndex>(),
                                              DataModel::Nullable<NodeId>(), /*sendLockOperationEvent=*/false) == CHIP_NO_ERROR,
                        Status::Failure);

    // The compat update keeps the same contract as the legacy ember write
    // (attribute update without a LockOperation event); some applications actuate
    // the bolt from the platform MatterPostAttributeChangeCallback, so keep
    // firing that hook with the raw attribute value.
    FireLegacyPostChange(endpoint, Attributes::LockState::Id, ZCL_ENUM8_ATTRIBUTE_TYPE, chip::to_underlying(value));
    return Status::Success;
}

} // namespace LockState

namespace RequirePINforRemoteOperation {

Status Get(EndpointId endpoint, bool * value)
{
    DoorLockCluster * cluster = CompatCluster(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::NotFound);
    if (value != nullptr)
    {
        *value = cluster->GetRequirePINforRemoteOperation();
    }
    return Status::Success;
}

} // namespace RequirePINforRemoteOperation

namespace LockType {

Status Set(EndpointId endpoint, DlLockType value)
{
    (void) value;
    return FixedConfigWrite(endpoint, "LockType");
}

} // namespace LockType

namespace FeatureMap {

Status Set(EndpointId endpoint, uint32_t value)
{
    (void) value;
    return FixedConfigWrite(endpoint, "FeatureMap");
}

} // namespace FeatureMap

namespace NumberOfTotalUsersSupported {

Status Set(EndpointId endpoint, uint16_t value)
{
    (void) value;
    return FixedConfigWrite(endpoint, "NumberOfTotalUsersSupported");
}

} // namespace NumberOfTotalUsersSupported

namespace NumberOfPINUsersSupported {

Status Set(EndpointId endpoint, uint16_t value)
{
    (void) value;
    return FixedConfigWrite(endpoint, "NumberOfPINUsersSupported");
}

} // namespace NumberOfPINUsersSupported

namespace NumberOfRFIDUsersSupported {

Status Set(EndpointId endpoint, uint16_t value)
{
    (void) value;
    return FixedConfigWrite(endpoint, "NumberOfRFIDUsersSupported");
}

} // namespace NumberOfRFIDUsersSupported

namespace NumberOfCredentialsSupportedPerUser {

Status Set(EndpointId endpoint, uint8_t value)
{
    (void) value;
    return FixedConfigWrite(endpoint, "NumberOfCredentialsSupportedPerUser");
}

} // namespace NumberOfCredentialsSupportedPerUser

namespace OperatingMode {

Status Set(EndpointId endpoint, OperatingModeEnum value)
{
    DoorLockCluster * cluster = CompatCluster(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::NotFound);
    DataModel::ActionReturnStatus status = cluster->SetOperatingMode(value);
    return status.IsSuccess() ? Status::Success : Status::Failure;
}

} // namespace OperatingMode

namespace SupportedOperatingModes {

Status Set(EndpointId endpoint, chip::BitMask<DlSupportedOperatingModes> value)
{
    (void) endpoint;
    (void) value;
    return FixedConfigWrite(endpoint, "SupportedOperatingModes");
}

} // namespace SupportedOperatingModes

} // namespace Attributes
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
