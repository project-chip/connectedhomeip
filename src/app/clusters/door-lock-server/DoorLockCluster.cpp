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

#include <app/clusters/door-lock-server/DoorLockCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>
#include <cstring>

using namespace chip::Protocols::InteractionModel;
using namespace chip::app::Clusters::DoorLock::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

namespace {

/// Mirrors the legacy `getFabricIndex`: the fabric of the invoking session (if any).
DataModel::Nullable<FabricIndex> GetOperationFabric(const DataModel::InvokeRequest & request)
{
    return DataModel::Nullable<FabricIndex>(request.subjectDescriptor.fabricIndex);
}

/// Mirrors the legacy `getNodeId`: the node id of the invoking session, only
/// meaningful for CASE sessions.
DataModel::Nullable<NodeId> GetOperationNode(const DataModel::InvokeRequest & request)
{
    if (request.subjectDescriptor.fabricIndex == kUndefinedFabricIndex ||
        request.subjectDescriptor.authMode != Access::AuthMode::kCase)
    {
        return DataModel::Nullable<NodeId>();
    }
    return DataModel::Nullable<NodeId>(request.subjectDescriptor.subject);
}

/// Cap so the auto-relock timeout always fits the timer representation
/// (legacy caps for the same reason before converting to milliseconds).
constexpr uint32_t kMaxAutoRelockTimeoutSec = 7 * 24 * 60 * 60;

/// Encodes a nullable Aliro key: an empty span from the delegate means
/// "not configured", which the spec expresses as a null attribute value.
DataModel::ActionReturnStatus EncodeNullableAliroKey(const ByteSpan & key, AttributeValueEncoder & encoder)
{
    if (key.empty())
    {
        return encoder.Encode(DataModel::Nullable<ByteSpan>());
    }
    return encoder.Encode(DataModel::Nullable<ByteSpan>(key));
}

} // namespace

DoorLockCluster::DoorLockCluster(EndpointId endpointId, Delegate & delegate, const Config & config) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, DoorLock::Id)), mDelegate(delegate), mTimerDelegate(config.timerDelegate),
    mFeatures(config.features), mOptionalAttributes(config.optionalAttributes), mLockType(config.lockType),
    mSupportedOperatingModes(config.supportedOperatingModes), mNumberOfTotalUsersSupported(config.numberOfTotalUsersSupported),
    mNumberOfPINUsersSupported(config.numberOfPINUsersSupported), mNumberOfRFIDUsersSupported(config.numberOfRFIDUsersSupported),
    mNumberOfWeekDaySchedulesPerUser(config.numberOfWeekDaySchedulesPerUser),
    mNumberOfYearDaySchedulesPerUser(config.numberOfYearDaySchedulesPerUser),
    mNumberOfHolidaySchedulesSupported(config.numberOfHolidaySchedulesSupported),
    mNumberOfCredentialsSupportedPerUser(config.numberOfCredentialsSupportedPerUser),
    mCredentialRulesSupport(config.credentialRulesSupport), mMaxPINCodeLength(config.maxPINCodeLength),
    mMinPINCodeLength(config.minPINCodeLength), mMaxRFIDCodeLength(config.maxRFIDCodeLength),
    mMinRFIDCodeLength(config.minRFIDCodeLength), mLockState(config.lockState), mDoorState(config.doorState),
    mDoorOpenEvents(config.doorOpenEvents), mDoorClosedEvents(config.doorClosedEvents), mOpenPeriod(config.openPeriod),
    mActuatorEnabled(config.actuatorEnabled), mOperatingMode(config.operatingMode), mLEDSettings(config.ledSettings),
    mAutoRelockTime(config.autoRelockTime), mSoundVolume(config.soundVolume),
    mDefaultConfigurationRegister(config.defaultConfigurationRegister), mEnableLocalProgramming(config.enableLocalProgramming),
    mEnableOneTouchLocking(config.enableOneTouchLocking), mEnableInsideStatusLED(config.enableInsideStatusLED),
    mEnablePrivacyModeButton(config.enablePrivacyModeButton), mLocalProgrammingFeatures(config.localProgrammingFeatures),
    mWrongCodeEntryLimit(config.wrongCodeEntryLimit), mUserCodeTemporaryDisableTime(config.userCodeTemporaryDisableTime),
    mSendPINOverTheAir(config.sendPINOverTheAir), mRequirePINforRemoteOperation(config.requirePINforRemoteOperation),
    mExpiringUserTimeout(config.expiringUserTimeout)
{
    VerifyOrDie(config.language.size() <= Internal::kMaxLanguageLength);
    if (config.language.size() != 0)
    {
        memcpy(mLanguage.data(), config.language.data(), config.language.size());
    }
    mLanguageLength = config.language.size();

    mAutoRelockTimerContext.cluster = this;
}

CHIP_ERROR DoorLockCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // A restarted cluster must not fire a stale auto-relock timer.
    mTimerDelegate.CancelTimer(&mAutoRelockTimerContext);

    return CHIP_NO_ERROR;
}

void DoorLockCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mTimerDelegate.CancelTimer(&mAutoRelockTimerContext);
    DefaultServerCluster::Shutdown(shutdownType);
}

CHIP_ERROR DoorLockCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        // Door position sensor [DPS]
        { mFeatures.Has(Feature::kDoorPositionSensor), DoorState::kMetadataEntry },
        { mFeatures.Has(Feature::kDoorPositionSensor), DoorOpenEvents::kMetadataEntry },
        { mFeatures.Has(Feature::kDoorPositionSensor), DoorClosedEvents::kMetadataEntry },
        { mFeatures.Has(Feature::kDoorPositionSensor), OpenPeriod::kMetadataEntry },
        // User management [USR]
        { mFeatures.Has(Feature::kUser), NumberOfTotalUsersSupported::kMetadataEntry },
        { mFeatures.Has(Feature::kUser), CredentialRulesSupport::kMetadataEntry },
        { mFeatures.Has(Feature::kUser), NumberOfCredentialsSupportedPerUser::kMetadataEntry },
        // PIN credentials [PIN]
        { mFeatures.Has(Feature::kPinCredential), NumberOfPINUsersSupported::kMetadataEntry },
        { mFeatures.Has(Feature::kPinCredential), MaxPINCodeLength::kMetadataEntry },
        { mFeatures.Has(Feature::kPinCredential), MinPINCodeLength::kMetadataEntry },
        // RFID credentials [RID]
        { mFeatures.Has(Feature::kRfidCredential), NumberOfRFIDUsersSupported::kMetadataEntry },
        { mFeatures.Has(Feature::kRfidCredential), MaxRFIDCodeLength::kMetadataEntry },
        { mFeatures.Has(Feature::kRfidCredential), MinRFIDCodeLength::kMetadataEntry },
        // Schedules
        { mFeatures.Has(Feature::kWeekDayAccessSchedules), NumberOfWeekDaySchedulesSupportedPerUser::kMetadataEntry },
        { mFeatures.Has(Feature::kYearDayAccessSchedules), NumberOfYearDaySchedulesSupportedPerUser::kMetadataEntry },
        { mFeatures.Has(Feature::kHolidaySchedules), NumberOfHolidaySchedulesSupported::kMetadataEntry },
        // Wrong-code handling [PIN or RID]
        { mFeatures.Has(Feature::kPinCredential) || mFeatures.Has(Feature::kRfidCredential), WrongCodeEntryLimit::kMetadataEntry },
        { mFeatures.Has(Feature::kPinCredential) || mFeatures.Has(Feature::kRfidCredential),
          UserCodeTemporaryDisableTime::kMetadataEntry },
        // SendPINOverTheAir is only available when the User feature is NOT
        // supported (with USR the credentials belong to user records).
        { !mFeatures.Has(Feature::kUser) && mFeatures.Has(Feature::kPinCredential), SendPINOverTheAir::kMetadataEntry },
        { mFeatures.Has(Feature::kCredentialsOverTheAirAccess) && mFeatures.Has(Feature::kPinCredential),
          RequirePINforRemoteOperation::kMetadataEntry },
        { mFeatures.Has(Feature::kUser), ExpiringUserTimeout::kMetadataEntry },
        // Aliro reader provisioning [ALIRO]
        { mFeatures.Has(Feature::kAliroProvisioning), AliroReaderVerificationKey::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroProvisioning), AliroReaderGroupIdentifier::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroProvisioning), AliroReaderGroupSubIdentifier::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroProvisioning), AliroExpeditedTransactionSupportedProtocolVersions::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroProvisioning), NumberOfAliroCredentialIssuerKeysSupported::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroProvisioning), NumberOfAliroEndpointKeysSupported::kMetadataEntry },
        // Aliro BLE UWB [ALBU]
        { mFeatures.Has(Feature::kAliroBLEUWB), AliroGroupResolvingKey::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroBLEUWB), AliroSupportedBLEUWBProtocolVersions::kMetadataEntry },
        { mFeatures.Has(Feature::kAliroBLEUWB), AliroBLEAdvertisingVersion::kMetadataEntry },
        // Plain optional attributes (configuration, not feature, gated)
        { mOptionalAttributes.language, Language::kMetadataEntry },
        { mOptionalAttributes.ledSettings, LEDSettings::kMetadataEntry },
        { mOptionalAttributes.autoRelockTime, AutoRelockTime::kMetadataEntry },
        { mOptionalAttributes.soundVolume, SoundVolume::kMetadataEntry },
        { mOptionalAttributes.defaultConfigurationRegister, DefaultConfigurationRegister::kMetadataEntry },
        { mOptionalAttributes.enableLocalProgramming, EnableLocalProgramming::kMetadataEntry },
        { mOptionalAttributes.enableOneTouchLocking, EnableOneTouchLocking::kMetadataEntry },
        { mOptionalAttributes.enableInsideStatusLED, EnableInsideStatusLED::kMetadataEntry },
        { mOptionalAttributes.enablePrivacyModeButton, EnablePrivacyModeButton::kMetadataEntry },
        { mOptionalAttributes.localProgrammingFeatures, LocalProgrammingFeatures::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR DoorLockCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kBaseCommands[] = {
        Commands::LockDoor::kMetadataEntry,
        Commands::UnlockDoor::kMetadataEntry,
        Commands::UnlockWithTimeout::kMetadataEntry,
    };

    if (mFeatures.Has(Feature::kUnbolt))
    {
        // UnboltDoor is only accepted when the UBOLT feature is enabled.
        ReturnErrorOnFailure(builder.AppendElements(Span(kBaseCommands)));
        static constexpr DataModel::AcceptedCommandEntry kUnboltCommands[] = {
            Commands::UnboltDoor::kMetadataEntry,
        };
        return builder.AppendElements(Span(kUnboltCommands));
    }

    return builder.ReferenceExisting(kBaseCommands);
}

DataModel::ActionReturnStatus DoorLockCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);

    case LockState::Id:
        return encoder.Encode(mLockState);
    case LockType::Id:
        return encoder.Encode(mLockType);
    case ActuatorEnabled::Id:
        return encoder.Encode(mActuatorEnabled);
    case OperatingMode::Id:
        return encoder.Encode(mOperatingMode);
    case SupportedOperatingModes::Id:
        return encoder.Encode(mSupportedOperatingModes);
    case DoorState::Id:
        return encoder.Encode(mDoorState);
    case DoorOpenEvents::Id:
        return encoder.Encode(mDoorOpenEvents);
    case DoorClosedEvents::Id:
        return encoder.Encode(mDoorClosedEvents);
    case OpenPeriod::Id:
        return encoder.Encode(mOpenPeriod);
    case NumberOfTotalUsersSupported::Id:
        return encoder.Encode(mNumberOfTotalUsersSupported);
    case NumberOfPINUsersSupported::Id:
        return encoder.Encode(mNumberOfPINUsersSupported);
    case NumberOfRFIDUsersSupported::Id:
        return encoder.Encode(mNumberOfRFIDUsersSupported);
    case NumberOfWeekDaySchedulesSupportedPerUser::Id:
        return encoder.Encode(mNumberOfWeekDaySchedulesPerUser);
    case NumberOfYearDaySchedulesSupportedPerUser::Id:
        return encoder.Encode(mNumberOfYearDaySchedulesPerUser);
    case NumberOfHolidaySchedulesSupported::Id:
        return encoder.Encode(mNumberOfHolidaySchedulesSupported);
    case MaxPINCodeLength::Id:
        return encoder.Encode(mMaxPINCodeLength);
    case MinPINCodeLength::Id:
        return encoder.Encode(mMinPINCodeLength);
    case MaxRFIDCodeLength::Id:
        return encoder.Encode(mMaxRFIDCodeLength);
    case MinRFIDCodeLength::Id:
        return encoder.Encode(mMinRFIDCodeLength);
    case CredentialRulesSupport::Id:
        return encoder.Encode(mCredentialRulesSupport);
    case NumberOfCredentialsSupportedPerUser::Id:
        return encoder.Encode(mNumberOfCredentialsSupportedPerUser);
    case Language::Id:
        return encoder.Encode(CharSpan(mLanguage.data(), mLanguageLength));
    case LEDSettings::Id:
        return encoder.Encode(mLEDSettings);
    case AutoRelockTime::Id:
        return encoder.Encode(mAutoRelockTime);
    case SoundVolume::Id:
        return encoder.Encode(mSoundVolume);
    case DefaultConfigurationRegister::Id:
        return encoder.Encode(mDefaultConfigurationRegister);
    case EnableLocalProgramming::Id:
        return encoder.Encode(mEnableLocalProgramming);
    case EnableOneTouchLocking::Id:
        return encoder.Encode(mEnableOneTouchLocking);
    case EnableInsideStatusLED::Id:
        return encoder.Encode(mEnableInsideStatusLED);
    case EnablePrivacyModeButton::Id:
        return encoder.Encode(mEnablePrivacyModeButton);
    case LocalProgrammingFeatures::Id:
        return encoder.Encode(mLocalProgrammingFeatures);
    case WrongCodeEntryLimit::Id:
        return encoder.Encode(mWrongCodeEntryLimit);
    case UserCodeTemporaryDisableTime::Id:
        return encoder.Encode(mUserCodeTemporaryDisableTime);
    case SendPINOverTheAir::Id:
        return encoder.Encode(mSendPINOverTheAir);
    case RequirePINforRemoteOperation::Id:
        return encoder.Encode(mRequirePINforRemoteOperation);
    case ExpiringUserTimeout::Id:
        return encoder.Encode(mExpiringUserTimeout);

    // Aliro attributes are delegate-provided and read on demand, so keys are
    // never kept in the cluster RAM.
    case AliroReaderVerificationKey::Id: {
        uint8_t buffer[kAliroReaderVerificationKeySize];
        MutableByteSpan span(buffer);
        ReturnErrorOnFailure(mDelegate.GetAliroReaderVerificationKey(span));
        return EncodeNullableAliroKey(span, encoder);
    }
    case AliroReaderGroupIdentifier::Id: {
        uint8_t buffer[kAliroReaderGroupIdentifierSize];
        MutableByteSpan span(buffer);
        ReturnErrorOnFailure(mDelegate.GetAliroReaderGroupIdentifier(span));
        return EncodeNullableAliroKey(span, encoder);
    }
    case AliroReaderGroupSubIdentifier::Id: {
        uint8_t buffer[kAliroReaderGroupSubIdentifierSize];
        MutableByteSpan span(buffer);
        ReturnErrorOnFailure(mDelegate.GetAliroReaderGroupSubIdentifier(span));
        return encoder.Encode(span);
    }
    case AliroExpeditedTransactionSupportedProtocolVersions::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (size_t i = 0;; i++)
            {
                uint8_t buffer[kAliroProtocolVersionSize];
                MutableByteSpan span(buffer);
                CHIP_ERROR err = mDelegate.GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(i, span);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(listEncoder.Encode(span));
            }
        });
    case AliroGroupResolvingKey::Id: {
        uint8_t buffer[kAliroGroupResolvingKeySize];
        MutableByteSpan span(buffer);
        ReturnErrorOnFailure(mDelegate.GetAliroGroupResolvingKey(span));
        return EncodeNullableAliroKey(span, encoder);
    }
    case AliroSupportedBLEUWBProtocolVersions::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (size_t i = 0;; i++)
            {
                uint8_t buffer[kAliroProtocolVersionSize];
                MutableByteSpan span(buffer);
                CHIP_ERROR err = mDelegate.GetAliroSupportedBLEUWBProtocolVersionAtIndex(i, span);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(listEncoder.Encode(span));
            }
        });
    case AliroBLEAdvertisingVersion::Id:
        return encoder.Encode(mDelegate.GetAliroBLEAdvertisingVersion());
    case NumberOfAliroCredentialIssuerKeysSupported::Id:
        return encoder.Encode(mDelegate.GetNumberOfAliroCredentialIssuerKeysSupported());
    case NumberOfAliroEndpointKeysSupported::Id:
        return encoder.Encode(mDelegate.GetNumberOfAliroEndpointKeysSupported());
    }

    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

DataModel::ActionReturnStatus DoorLockCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                              AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    // Spec 9.2.3.5-9.2.3.7: the door position counters are writable (RW VM).
    case DoorOpenEvents::Id: {
        uint32_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mDoorOpenEvents, value, DoorOpenEvents::Id);
        return CHIP_NO_ERROR;
    }
    case DoorClosedEvents::Id: {
        uint32_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mDoorClosedEvents, value, DoorClosedEvents::Id);
        return CHIP_NO_ERROR;
    }
    case OpenPeriod::Id: {
        uint16_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mOpenPeriod, value, OpenPeriod::Id);
        return CHIP_NO_ERROR;
    }
    case Language::Id: {
        CharSpan value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value.size() <= Internal::kMaxLanguageLength, Protocols::InteractionModel::Status::ConstraintError);
        memcpy(mLanguage.data(), value.data(), value.size());
        mLanguageLength = value.size();
        NotifyAttributeChanged(Language::Id);
        return CHIP_NO_ERROR;
    }
    case LEDSettings::Id: {
        uint8_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mLEDSettings, value, LEDSettings::Id);
        return CHIP_NO_ERROR;
    }
    case AutoRelockTime::Id: {
        uint32_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mAutoRelockTime, value, AutoRelockTime::Id);
        return CHIP_NO_ERROR;
    }
    case SoundVolume::Id: {
        uint8_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mSoundVolume, value, SoundVolume::Id);
        return CHIP_NO_ERROR;
    }
    case OperatingMode::Id: {
        OperatingModeEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        // Spec: OperatingMode SHALL be a mode listed in SupportedOperatingModes.
        // Each mode N is represented by bit (1 << N) in the bitmap.
        VerifyOrReturnError(mSupportedOperatingModes.Has(static_cast<DlSupportedOperatingModes>(1u << to_underlying(value))),
                            Protocols::InteractionModel::Status::ConstraintError);
        SetAttributeValue(mOperatingMode, value, OperatingMode::Id);
        return CHIP_NO_ERROR;
    }
    case EnableLocalProgramming::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mEnableLocalProgramming, value, EnableLocalProgramming::Id);
        return CHIP_NO_ERROR;
    }
    case EnableOneTouchLocking::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mEnableOneTouchLocking, value, EnableOneTouchLocking::Id);
        return CHIP_NO_ERROR;
    }
    case EnableInsideStatusLED::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mEnableInsideStatusLED, value, EnableInsideStatusLED::Id);
        return CHIP_NO_ERROR;
    }
    case EnablePrivacyModeButton::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mEnablePrivacyModeButton, value, EnablePrivacyModeButton::Id);
        return CHIP_NO_ERROR;
    }
    case LocalProgrammingFeatures::Id: {
        BitMask<DlLocalProgrammingFeatures> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mLocalProgrammingFeatures, value, LocalProgrammingFeatures::Id);
        return CHIP_NO_ERROR;
    }
    case WrongCodeEntryLimit::Id: {
        uint8_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value >= 1, Protocols::InteractionModel::Status::ConstraintError);
        SetAttributeValue(mWrongCodeEntryLimit, value, WrongCodeEntryLimit::Id);
        return CHIP_NO_ERROR;
    }
    case UserCodeTemporaryDisableTime::Id: {
        uint8_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value >= 1, Protocols::InteractionModel::Status::ConstraintError);
        SetAttributeValue(mUserCodeTemporaryDisableTime, value, UserCodeTemporaryDisableTime::Id);
        return CHIP_NO_ERROR;
    }
    case SendPINOverTheAir::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mSendPINOverTheAir, value, SendPINOverTheAir::Id);
        return CHIP_NO_ERROR;
    }
    case RequirePINforRemoteOperation::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        SetAttributeValue(mRequirePINforRemoteOperation, value, RequirePINforRemoteOperation::Id);
        return CHIP_NO_ERROR;
    }
    case ExpiringUserTimeout::Id: {
        uint16_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value >= 1 && value <= 2880, Protocols::InteractionModel::Status::ConstraintError);
        SetAttributeValue(mExpiringUserTimeout, value, ExpiringUserTimeout::Id);
        return CHIP_NO_ERROR;
    }
    }

    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

std::optional<DataModel::ActionReturnStatus> DoorLockCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                            chip::TLV::TLVReader & input_arguments,
                                                                            CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::LockDoor::Id: {
        Commands::LockDoor::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleRemoteLockOperation(request, LockOperationTypeEnum::kLock, &Delegate::HandleDoorLockCommand,
                                         commandData.PINCode);
    }
    case Commands::UnlockDoor::Id: {
        Commands::UnlockDoor::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleRemoteLockOperation(
            request, mFeatures.Has(Feature::kUnbolt) ? LockOperationTypeEnum::kUnlatch : LockOperationTypeEnum::kUnlock,
            &Delegate::HandleDoorUnlockCommand, commandData.PINCode);
    }
    case Commands::UnlockWithTimeout::Id: {
        Commands::UnlockWithTimeout::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        auto status = HandleRemoteLockOperation(
            request, mFeatures.Has(Feature::kUnbolt) ? LockOperationTypeEnum::kUnlatch : LockOperationTypeEnum::kUnlock,
            &Delegate::HandleDoorUnlockCommand, commandData.PINCode);
        // Spec 5.3.4.3: after the timeout (seconds) expires the lock relocks
        // itself; timeout == 0 disables the one-time relock.
        if (status.has_value() && status->IsSuccess() && commandData.timeout != 0)
        {
            ScheduleAutoRelock(commandData.timeout);
        }
        return status;
    }
    case Commands::UnboltDoor::Id: {
        Commands::UnboltDoor::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        auto status = HandleRemoteLockOperation(request, LockOperationTypeEnum::kUnlock, &Delegate::HandleDoorUnboltCommand,
                                                commandData.PINCode);
        if (status.has_value() && status->IsSuccess() && mAutoRelockTime != 0)
        {
            ScheduleAutoRelock(mAutoRelockTime);
        }
        return status;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR DoorLockCluster::SetLockState(DlLockState newState, OperationSourceEnum opSource,
                                         const DataModel::Nullable<uint16_t> & userIndex,
                                         const Span<const CredentialStruct> & credentials,
                                         const DataModel::Nullable<FabricIndex> & fabricIdx,
                                         const DataModel::Nullable<NodeId> & nodeId)
{
    return ApplyLockStateChange(newState, opSource, userIndex, credentials, fabricIdx, nodeId, /* sendLockOperationEvent */ true);
}

CHIP_ERROR DoorLockCluster::SetDoorState(DataModel::Nullable<DoorStateEnum> doorState)
{
    bool changed = SetAttributeValue(mDoorState, doorState, DoorState::Id);
    // The DoorStateChange event carries a non-nullable door state, so a null
    // attribute update is reported without an event.
    VerifyOrReturnError(changed && !doorState.IsNull(), CHIP_NO_ERROR);

    Events::DoorStateChange::Type event{ doorState.Value() };
    SendEvent(event);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockCluster::SendLockAlarmEvent(AlarmCodeEnum alarmCode)
{
    Events::DoorLockAlarm::Type event{ alarmCode };
    SendEvent(event);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockCluster::HandleWrongCodeEntry()
{
    // Wrong-code tracking only exists with PIN or RFID credentials
    // (legacy: the WrongCodeEntryLimit attribute is not present).
    VerifyOrReturnError(mFeatures.Has(Feature::kPinCredential) || mFeatures.Has(Feature::kRfidCredential), CHIP_NO_ERROR);

    mWrongCodeEntryAttempts++;
    if (mWrongCodeEntryAttempts < mWrongCodeEntryLimit)
    {
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(Zcl, "Too many wrong code entry attempts, engaging lockout [endpoint=%d,wrongCodeAttempts=%d]",
                    mPath.mEndpointId, mWrongCodeEntryAttempts);
    return EngageLockout();
}

void DoorLockCluster::ResetWrongCodeEntryAttempts()
{
    mWrongCodeEntryAttempts = 0;
}

bool DoorLockCluster::IsLockoutEngaged() const
{
    // A zero lockout end timestamp means no lockout was ever engaged (legacy
    // relies on the real monotonic clock being past epoch 0; the mock clock in
    // tests starts at 0, so the comparison must be strict).
    return mLockoutEndTimestamp > mTimerDelegate.GetCurrentMonotonicTimestamp();
}

CHIP_ERROR DoorLockCluster::EngageLockout()
{
    mWrongCodeEntryAttempts = 0;
    mLockoutEndTimestamp = mTimerDelegate.GetCurrentMonotonicTimestamp() + System::Clock::Seconds32(mUserCodeTemporaryDisableTime);

    ChipLogProgress(Zcl, "Lockout engaged [endpoint=%d,lockoutTimeout=%u]", mPath.mEndpointId, mUserCodeTemporaryDisableTime);

    SendEvent(Events::DoorLockAlarm::Type{ AlarmCodeEnum::kWrongCodeEntryLimit });

    mDelegate.OnLockoutStarted(mPath.mEndpointId, mLockoutEndTimestamp);
    return CHIP_NO_ERROR;
}

void DoorLockCluster::ScheduleAutoRelock(uint32_t timeoutSec)
{
    // TimerDelegate::StartTimer requires cancelling a pending timer for the
    // same context first: a new unlock replaces the previous auto-relock.
    mTimerDelegate.CancelTimer(&mAutoRelockTimerContext);
    LogErrorOnFailure(mTimerDelegate.StartTimer(&mAutoRelockTimerContext,
                                                System::Clock::Seconds32(std::min(timeoutSec, kMaxAutoRelockTimeoutSec))));
}

void DoorLockCluster::OnAutoRelockTimerExpired()
{
    if (GetLockState() == DlLockState::kLocked)
    {
        ChipLogProgress(Zcl, "Door auto relock timer expired. Lock already locked.");
        return;
    }

    ChipLogProgress(Zcl, "Door auto relock timer expired. Locking...");
    DataModel::Nullable<uint16_t> noUser;
    DataModel::Nullable<chip::FabricIndex> noFabric;
    DataModel::Nullable<chip::NodeId> noNode;
    CHIP_ERROR err =
        SetLockState(DlLockState::kLocked, OperationSourceEnum::kAuto, noUser, Span<const CredentialStruct>(), noFabric, noNode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to auto-relock the door: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    mDelegate.OnAutoRelock(mPath.mEndpointId);
}

void DoorLockCluster::AutoRelockTimerContext::TimerFired()
{
    cluster->OnAutoRelockTimerExpired();
}

void DoorLockCluster::SendLockOperationEvent(LockOperationTypeEnum opType, OperationSourceEnum opSource, OperationErrorEnum opErr,
                                             const DataModel::Nullable<uint16_t> & userIndex,
                                             const Span<const CredentialStruct> & credentials,
                                             const DataModel::Nullable<FabricIndex> & fabricIdx,
                                             const DataModel::Nullable<NodeId> & nodeId, bool opSuccess)
{
    // Spec 5.3.5.3/5.3.5.4: the credentials list SHALL be null if no
    // credentials were involved in the operation.
    DataModel::Nullable<DataModel::List<const CredentialStruct>> credentialsList;
    if (!credentials.empty())
    {
        credentialsList.SetNonNull(DataModel::List<const CredentialStruct>(credentials.data(), credentials.size()));
    }

    if (opSuccess)
    {
        Events::LockOperation::Type event{ opType, opSource, userIndex, fabricIdx, nodeId, MakeOptional(credentialsList) };
        SendEvent(event);
    }
    else
    {
        Events::LockOperationError::Type event{
            opType, opSource, opErr, userIndex, fabricIdx, nodeId, MakeOptional(credentialsList)
        };
        SendEvent(event);
    }
}

std::optional<DataModel::ActionReturnStatus> DoorLockCluster::HandleRemoteLockOperation(const DataModel::InvokeRequest & request,
                                                                                        LockOperationTypeEnum opType,
                                                                                        DelegateLockOpHandler handler,
                                                                                        const Optional<chip::ByteSpan> & pinCode)
{
    OperationErrorEnum reason = OperationErrorEnum::kUnspecified;
    bool success              = false;
    bool sendEvent            = true;

    VerifyOrReturnError(IsStarted(), CHIP_ERROR_INCORRECT_STATE);

    if (!RemoteOperationEnabled())
    {
        ChipLogProgress(Zcl, "Rejecting remote lock operation: operating mode disallows remote operations [endpoint=%d]",
                        mPath.mEndpointId);
    }
    else if (IsLockoutEngaged())
    {
        // Spec 5.3.4.1: attempts during UserCodeTemporaryDisableTime are
        // silently ignored (no command response event).
        ChipLogProgress(Zcl, "Rejecting remote lock operation -- lockout is in action [endpoint=%d]", mPath.mEndpointId);
        sendEvent = false;
    }
    else
    {
        // Credential verification (PIN lookup, RequirePINForRemoteOperation) is
        // layered on the delegate in a later phase; the delegate performs the
        // hardware actuation and reports the failure reason.
        success = (mDelegate.*handler)(mPath.mEndpointId, GetOperationFabric(request), GetOperationNode(request), pinCode, reason);
    }

    // Spec 5.3.4.1: an invalid PIN counts towards WrongCodeEntryLimit and may
    // trigger UserCodeTemporaryDisableTime.
    if (!success && reason == OperationErrorEnum::kInvalidCredential)
    {
        LogErrorOnFailure(HandleWrongCodeEntry());
    }

    // Reset the wrong-code retry attempts if a valid credential was presented.
    if (success && pinCode.HasValue())
    {
        ResetWrongCodeEntryAttempts();
    }

    // Most of the time we want to send the lock operation event, but during an
    // active lockout the operation is silently ignored. On success the event is
    // emitted by SetLockState once the hardware reports the new lock state.
    if (!success && sendEvent)
    {
        // Spec: failed Unlatch requests generate a LockOperationError event with
        // LockOperationType set to Unlock.
        SendLockOperationEvent(opType == LockOperationTypeEnum::kUnlatch ? LockOperationTypeEnum::kUnlock : opType,
                               OperationSourceEnum::kRemote, reason, DataModel::Nullable<uint16_t>(),
                               Span<const CredentialStruct>(), GetOperationFabric(request), GetOperationNode(request),
                               /* opSuccess */ false);
    }

    return success ? DataModel::ActionReturnStatus(Status::Success) : DataModel::ActionReturnStatus(Status::Failure);
}

CHIP_ERROR DoorLockCluster::ApplyLockStateChange(DlLockState newState, OperationSourceEnum opSource,
                                                 const DataModel::Nullable<uint16_t> & userIndex,
                                                 const Span<const CredentialStruct> & credentials,
                                                 const DataModel::Nullable<FabricIndex> & fabricIdx,
                                                 const DataModel::Nullable<NodeId> & nodeId, bool sendLockOperationEvent)
{
    SetAttributeValue(mLockState, newState, LockState::Id);

    // DlLockState::kNotFullyLocked has no appropriate event and unclear
    // auto-relocking semantics, so skip it (mirrors the legacy server).
    VerifyOrReturnError(newState == DlLockState::kLocked || newState == DlLockState::kUnlocked ||
                            newState == DlLockState::kUnlatched,
                        CHIP_NO_ERROR);

    LockOperationTypeEnum opType = LockOperationTypeEnum::kUnlock;
    if (newState == DlLockState::kLocked)
    {
        opType = LockOperationTypeEnum::kLock;
    }
    else if (newState == DlLockState::kUnlatched)
    {
        opType = LockOperationTypeEnum::kUnlatch;
    }

    if (opSource == OperationSourceEnum::kRemote && (fabricIdx.IsNull() || nodeId.IsNull()))
    {
        ChipLogError(Zcl, "Received SetLockState for remote operation without fabricIdx or nodeId");
    }

    if (sendLockOperationEvent)
    {
        SendLockOperationEvent(opType, opSource, OperationErrorEnum::kUnspecified, userIndex, credentials, fabricIdx, nodeId,
                               /* opSuccess */ true);
    }

    // Reset the wrong-code attempts when a lock/unlock succeeded with credentials.
    if (!credentials.empty())
    {
        ResetWrongCodeEntryAttempts();
    }

    // Spec 5.3.3.25: 0 = disabled; when set, unlock operations from any source
    // are timed (legacy: auto-relock only applies to plain unlocks, the
    // UnlockWithTimeout command schedules its own timer).
    if (opType == LockOperationTypeEnum::kUnlock && mAutoRelockTime != 0)
    {
        ScheduleAutoRelock(mAutoRelockTime);
    }

    return CHIP_NO_ERROR;
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
