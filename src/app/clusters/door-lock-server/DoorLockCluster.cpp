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

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

#include <algorithm>
#include <cinttypes>
#include <cstring>

using namespace chip::Protocols::InteractionModel;
using namespace chip::app::Clusters::DoorLock::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

namespace {

/// Fabric index of the invoking session, if any.
///
/// The delegate interface reports the operation initiator through nullable
/// fabric/node-id pairs, because local operations have neither; a remote
/// operation therefore presents its always-known fabric through the same
/// nullable types.
DataModel::Nullable<FabricIndex> GetOperationFabric(const DataModel::InvokeRequest & request)
{
    if (request.subjectDescriptor.fabricIndex == kUndefinedFabricIndex)
    {
        return DataModel::Nullable<FabricIndex>();
    }
    return DataModel::Nullable<FabricIndex>(request.subjectDescriptor.fabricIndex);
}

/// Node id of the invoking session: only CASE sessions have one, every other
/// session type (and any local operation) reports a null node id.
DataModel::Nullable<NodeId> GetOperationNode(const DataModel::InvokeRequest & request)
{
    if (request.subjectDescriptor.fabricIndex == kUndefinedFabricIndex ||
        request.subjectDescriptor.authMode != Access::AuthMode::kCase)
    {
        return DataModel::Nullable<NodeId>();
    }
    return DataModel::Nullable<NodeId>(request.subjectDescriptor.subject);
}

/// Cap so the auto-relock timeout always fits the timer representation.
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
    mMinRFIDCodeLength(config.minRFIDCodeLength),
    mNumberOfFingerprintCredentialsSupported(config.numberOfFingerprintCredentialsSupported),
    mNumberOfFingerVeinCredentialsSupported(config.numberOfFingerVeinCredentialsSupported),
    mNumberOfFaceCredentialsSupported(config.numberOfFaceCredentialsSupported),
    mMinFingerprintCredentialLength(config.minFingerprintCredentialLength),
    mMaxFingerprintCredentialLength(config.maxFingerprintCredentialLength),
    mMinFingerVeinCredentialLength(config.minFingerVeinCredentialLength),
    mMaxFingerVeinCredentialLength(config.maxFingerVeinCredentialLength), mMinFaceCredentialLength(config.minFaceCredentialLength),
    mMaxFaceCredentialLength(config.maxFaceCredentialLength), mLockState(config.lockState), mDoorState(config.doorState),
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

void DoorLockCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mTimerDelegate.CancelTimer(&mAutoRelockTimerContext);
    DefaultServerCluster::Shutdown(shutdownType);
}

CHIP_ERROR DoorLockCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        // Door position sensor [DPS]; the counters keep their own optional flags.
        { mFeatures.Has(Feature::kDoorPositionSensor), DoorState::kMetadataEntry },
        { mFeatures.Has(Feature::kDoorPositionSensor) && mOptionalAttributes.doorOpenEvents, DoorOpenEvents::kMetadataEntry },
        { mFeatures.Has(Feature::kDoorPositionSensor) && mOptionalAttributes.doorClosedEvents, DoorClosedEvents::kMetadataEntry },
        { mFeatures.Has(Feature::kDoorPositionSensor) && mOptionalAttributes.openPeriod, OpenPeriod::kMetadataEntry },
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
        // SendPINOverTheAir [!USR & PIN]: with USR the credentials belong to
        // user records, so the attribute only exists when USR is NOT supported.
        { mOptionalAttributes.sendPINOverTheAir && !mFeatures.Has(Feature::kUser) && mFeatures.Has(Feature::kPinCredential),
          SendPINOverTheAir::kMetadataEntry },
        { mFeatures.Has(Feature::kCredentialsOverTheAirAccess) && mFeatures.Has(Feature::kPinCredential),
          RequirePINforRemoteOperation::kMetadataEntry },
        { mOptionalAttributes.expiringUserTimeout && mFeatures.Has(Feature::kUser), ExpiringUserTimeout::kMetadataEntry },
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
    // Feature-gated slices; assembled in ascending command-id order.
    // clang-format off
    static constexpr DataModel::AcceptedCommandEntry kLockCommands[] = {
        Commands::LockDoor::kMetadataEntry,
        Commands::UnlockDoor::kMetadataEntry,
        Commands::UnlockWithTimeout::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kUnboltCommand[] = {
        Commands::UnboltDoor::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kUserCommands[] = {
        Commands::SetUser::kMetadataEntry,
        Commands::GetUser::kMetadataEntry,
        Commands::ClearUser::kMetadataEntry,
    };
    // clang-format on

    ReturnErrorOnFailure(builder.AppendElements(Span(kLockCommands)));
    if (mFeatures.Has(Feature::kUnbolt))
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kUnboltCommand)));
    }
    if (SupportsUSR())
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kUserCommands)));
    }
    return CHIP_NO_ERROR;
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
        if (!value.data_equal(CharSpan(mLanguage.data(), mLanguageLength)))
        {
            if (!value.empty())
            {
                memcpy(mLanguage.data(), value.data(), value.size());
            }
            mLanguageLength = value.size();
            NotifyAttributeChanged(Language::Id);
        }
        return CHIP_NO_ERROR;
    }
    case LEDSettings::Id: {
        uint8_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value <= 2, Protocols::InteractionModel::Status::ConstraintError);
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
        VerifyOrReturnError(value <= 3, Protocols::InteractionModel::Status::ConstraintError);
        SetAttributeValue(mSoundVolume, value, SoundVolume::Id);
        return CHIP_NO_ERROR;
    }
    case OperatingMode::Id: {
        OperatingModeEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value != OperatingModeEnum::kUnknownEnumValue, Protocols::InteractionModel::Status::ConstraintError);
        // The OperatingModesBitmap uses inverted polarity: a `0` bit marks a
        // supported mode, so the mode is rejected when its bit is set.
        VerifyOrReturnError(!mSupportedOperatingModes.Has(static_cast<DlSupportedOperatingModes>(1u << to_underlying(value))),
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

    // Only writable attributes have a case above: this default is reached for
    // listed-but-read-only attributes, so reject those as non-writable.
    return Protocols::InteractionModel::Status::UnsupportedWrite;
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
        auto status = HandleRemoteLockOperation(
            request, mFeatures.Has(Feature::kUnbolt) ? LockOperationTypeEnum::kUnlatch : LockOperationTypeEnum::kUnlock,
            &Delegate::HandleDoorUnlockCommand, commandData.PINCode);
        // Spec 5.3.3.25: a successful remote unlock (re)starts the auto-relock
        // countdown using AutoRelockTime (0 disables it).
        if (status.has_value() && status->IsSuccess() && mAutoRelockTime != 0)
        {
            ScheduleAutoRelock(mAutoRelockTime);
        }
        return status;
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
    case Commands::SetUser::Id: {
        Commands::SetUser::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleSetUser(request, commandData, handler, request.path);
        return std::nullopt;
    }
    case Commands::GetUser::Id: {
        Commands::GetUser::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleGetUser(request, commandData.userIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::ClearUser::Id: {
        Commands::ClearUser::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleClearUser(request, commandData.userIndex, handler, request.path);
        return std::nullopt;
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
    SetAttributeValue(mLockState, newState, LockState::Id);
    StoreLockState();

    // DlLockState::kNotFullyLocked has no appropriate event and unclear
    // auto-relocking semantics, so skip it.
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

    // Any pending auto-relock is obsolete once a lock operation takes effect: a
    // stale timer would otherwise relock a door that was unlocked (with
    // AutoRelockTime = 0) in the meantime.
    if (opType == LockOperationTypeEnum::kLock)
    {
        mTimerDelegate.CancelTimer(&mAutoRelockTimerContext);
    }

    if (opSource == OperationSourceEnum::kRemote && (fabricIdx.IsNull() || nodeId.IsNull()))
    {
        ChipLogError(Zcl, "Received SetLockState for remote operation without fabricIdx or nodeId");
    }

    SendLockOperationEvent(opType, opSource, OperationErrorEnum::kUnspecified, userIndex, credentials, fabricIdx, nodeId,
                           /* opSuccess */ true);

    // Reset the wrong-code attempts when a lock/unlock succeeded with credentials.
    if (!credentials.empty())
    {
        ResetWrongCodeEntryAttempts();
    }

    // Spec 5.3.3.25: 0 = disabled; unlock operations from any source are timed.
    // Lock and unlatch operations are not relocked here, and UnlockWithTimeout
    // schedules its own one-shot timer instead of using AutoRelockTime.
    if (opType == LockOperationTypeEnum::kUnlock && mAutoRelockTime != 0)
    {
        ScheduleAutoRelock(mAutoRelockTime);
    }

    return CHIP_NO_ERROR;
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
    // Wrong-code tracking only applies when PIN or RFID credentials are
    // supported: without credentials there is nothing to count.
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
    // A zero lockout end timestamp means no lockout was ever engaged. The
    // comparison is strict so that a (mock) clock that has not advanced past
    // its zero point yet does not report an engaged lockout.
    // A zero lockout end timestamp means no lockout was ever engaged. The
    // comparison is strict so that a (mock) clock that has not advanced past
    // its zero point yet does not report an engaged lockout. Legacy compares
    // with >=; the only behavioral difference is at the exact expiry second,
    // where a mock clock would otherwise report a spurious lockout at boot.
    return mLockoutEndTimestamp > mTimerDelegate.GetCurrentMonotonicTimestamp();
}

CHIP_ERROR DoorLockCluster::EngageLockout()
{
    mWrongCodeEntryAttempts = 0;
    mLockoutEndTimestamp = mTimerDelegate.GetCurrentMonotonicTimestamp() + System::Clock::Seconds32(mUserCodeTemporaryDisableTime);

    ChipLogProgress(Zcl, "Lockout engaged [endpoint=%d,lockoutTimeout=%u]", mPath.mEndpointId, mUserCodeTemporaryDisableTime);

    SendEvent(Events::DoorLockAlarm::Type{ AlarmCodeEnum::kWrongCodeEntryLimit });

    mDelegate.OnLockoutStarted(mLockoutEndTimestamp);
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

    mDelegate.OnAutoRelock();
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
        // hardware actuation and reports the failure reason (nullopt = success).
        std::optional<OperationErrorEnum> operationResult =
            (mDelegate.*handler)(LockOperationRequest{ GetOperationFabric(request), GetOperationNode(request), pinCode });
        success = !operationResult.has_value();
        reason  = operationResult.value_or(OperationErrorEnum::kUnspecified);
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

CHIP_ERROR DoorLockCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    LoadPersistentAttributes(context);
    return CHIP_NO_ERROR;
}

void DoorLockCluster::LoadPersistentAttributes(ServerClusterContext & context)
{
    // The lockState attribute is persisted (`.matter` persist attribute): a
    // stored value replaces the initial configuration value.
    AttributePersistence attributePersistence(context.attributeStorage);
    if (!attributePersistence.LoadNativeEndianValue<DlLockState>(ConcreteAttributePath(mPath.mEndpointId, Id, LockState::Id),
                                                                 mLockState, mLockState))
    {
        ChipLogProgress(Zcl, "No persisted lock state; keep the initial value [endpoint=%d]", mPath.mEndpointId);
    }
}

void DoorLockCluster::StoreLockState() const
{
    VerifyOrReturn(IsStarted());
    AttributePersistence attributePersistence(mContext->attributeStorage);
    TEMPORARY_RETURN_IGNORED attributePersistence.StoreNativeEndianValue<DlLockState>(
        ConcreteAttributePath(mPath.mEndpointId, Id, LockState::Id), mLockState);
}

// =============================================================================
// User / credential / schedule storage access (via the delegate)
// =============================================================================

namespace {

// Wildcard index for the clear-all variant of ClearUser.
constexpr uint16_t DOOR_LOCK_ALL_USERS_INDEX = 0xFFFE;

bool IsValidUserStatusForSet(const DataModel::Nullable<UserStatusEnum> & userStatus)
{
    return userStatus.IsNull() || (userStatus.Value() == UserStatusEnum::kOccupiedEnabled) ||
        (userStatus.Value() == UserStatusEnum::kOccupiedDisabled);
}

/// Fabric index of the invoking session or kUndefinedFabricIndex for local
/// (group / PASE-adjacent) operations that can not run user management.
FabricIndex CommandFabricIndex(const DataModel::InvokeRequest & request)
{
    return request.subjectDescriptor.fabricIndex;
}

/// Node id of the invoking session or kUndefinedNodeId when not a CASE session.
NodeId CommandNodeId(const DataModel::InvokeRequest & request)
{
    if (request.subjectDescriptor.fabricIndex == kUndefinedFabricIndex ||
        request.subjectDescriptor.authMode != Access::AuthMode::kCase)
    {
        return kUndefinedNodeId;
    }
    return request.subjectDescriptor.subject;
}

} // namespace

// =============================================================================
// Storage lookup helpers
// =============================================================================

bool DoorLockCluster::UserIndexValid(uint16_t userIndex, uint16_t & maxNumberOfUsers) const
{
    maxNumberOfUsers = mNumberOfTotalUsersSupported;

    // appclusters, 5.2.4.34-37: user index changes from 1 to maxNumberOfUsers
    return userIndex != 0 && userIndex <= maxNumberOfUsers;
}

bool DoorLockCluster::UserIndexValid(uint16_t userIndex) const
{
    uint16_t maxNumberOfUsers;
    return UserIndexValid(userIndex, maxNumberOfUsers);
}

bool DoorLockCluster::UserExists(uint16_t userIndex) const
{
    UserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogError(Zcl, "[UserExists] Unable to get the user - internal error [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
        return false;
    }
    return UserStatusEnum::kAvailable != user.userStatus;
}

bool DoorLockCluster::FindOccupiedUserSlot(uint16_t startIndex, uint16_t & userIndex) const
{
    userIndex = 0;
    for (uint16_t i = startIndex; i <= mNumberOfTotalUsersSupported; ++i)
    {
        UserInfo user;
        if (!mDelegate.GetUser(i, user))
        {
            ChipLogError(Zcl, "Unable to get user to check if slot is occupied: app error [userIndex=%d]", i);
            return false;
        }

        if (UserStatusEnum::kAvailable != user.userStatus)
        {
            userIndex = i;
            return true;
        }
    }
    return false;
}

bool DoorLockCluster::FindUnoccupiedUserSlot(uint16_t & userIndex) const
{
    return FindUnoccupiedUserSlot(1, userIndex);
}

bool DoorLockCluster::FindUnoccupiedUserSlot(uint16_t startIndex, uint16_t & userIndex) const
{
    userIndex = 0;
    for (uint16_t i = startIndex; i <= mNumberOfTotalUsersSupported; ++i)
    {
        UserInfo user;
        if (!mDelegate.GetUser(i, user))
        {
            ChipLogError(Zcl, "Unable to get user to check if slot is occupied: app error [userIndex=%d]", i);
            return false;
        }

        if (UserStatusEnum::kAvailable == user.userStatus)
        {
            userIndex = i;
            return true;
        }
    }
    return false;
}

bool DoorLockCluster::WeekDayIndexValid(uint8_t weekDayIndex) const
{
    // appclusters, 5.2.4.14-17: weekday index changes from 1 to maxNumberOfUsers
    return weekDayIndex != 0 && weekDayIndex <= mNumberOfWeekDaySchedulesPerUser;
}

bool DoorLockCluster::YearDayIndexValid(uint8_t yearDayIndex) const
{
    // appclusters, 5.2.4.18-21: year day index changes from 1 to maxNumberOfUsers
    return yearDayIndex != 0 && yearDayIndex <= mNumberOfYearDaySchedulesPerUser;
}

bool DoorLockCluster::HolidayIndexValid(uint8_t holidayIndex) const
{
    // appclusters, 5.2.4.22-25: holiday index changes from 1 to maxNumberOfHolidaySchedules
    return holidayIndex != 0 && holidayIndex <= mNumberOfHolidaySchedulesSupported;
}

bool DoorLockCluster::IsUserScheduleRestricted(const UserInfo & user) const
{
    return false;
}

void DoorLockCluster::SendRemoteLockUserChange(LockDataTypeEnum dataType, DataOperationTypeEnum operation, NodeId nodeId,
                                               FabricIndex fabricIndex, uint16_t userIndex, uint16_t dataIndex) const
{
    Events::LockUserChange::Type event;
    event.lockDataType      = dataType;
    event.dataOperationType = operation;
    event.operationSource   = OperationSourceEnum::kRemote;
    if (0 != userIndex)
    {
        event.userIndex.SetNonNull(userIndex);
    }
    event.fabricIndex.SetNonNull(fabricIndex);
    event.sourceNode.SetNonNull(nodeId);
    if (0 != dataIndex)
    {
        event.dataIndex.SetNonNull(dataIndex);
    }

    SendEvent(event);
}

// =============================================================================
// User management (create / modify / clear)
// =============================================================================

ClusterStatusCode DoorLockCluster::CreateUser(FabricIndex creatorFabricIdx, NodeId sourceNodeId, uint16_t userIndex,
                                              const DataModel::Nullable<CharSpan> & userName,
                                              const DataModel::Nullable<uint32_t> & userUniqueId,
                                              const DataModel::Nullable<UserStatusEnum> & userStatus,
                                              const DataModel::Nullable<UserTypeEnum> & userType,
                                              const DataModel::Nullable<CredentialRuleEnum> & credentialRule,
                                              const DataModel::Nullable<CredentialStruct> & credential)
{
    UserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogError(Zcl, "[createUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
        return ClusterStatusCode(Status::Failure);
    }

    // appclusters, 5.2.4.34: to modify user its status should be set to Available. If it is we should return OCCUPIED.
    if (UserStatusEnum::kAvailable != user.userStatus)
    {
        ChipLogProgress(Zcl, "[createUser] Unable to overwrite existing user [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                        userIndex);
        return ClusterStatusCode::ClusterSpecificFailure(DlStatus::kOccupied);
    }

    const auto & newUserName                = !userName.IsNull() ? userName.Value() : ""_span;
    auto newUserUniqueId                    = userUniqueId.IsNull() ? 0xFFFFFFFF : userUniqueId.Value();
    auto newUserStatus                      = userStatus.IsNull() ? UserStatusEnum::kOccupiedEnabled : userStatus.Value();
    auto newUserType                        = userType.IsNull() ? UserTypeEnum::kUnrestrictedUser : userType.Value();
    auto newCredentialRule                  = credentialRule.IsNull() ? CredentialRuleEnum::kSingle : credentialRule.Value();
    const CredentialStruct * newCredentials = nullptr;
    size_t newTotalCredentials              = 0;
    if (!credential.IsNull())
    {
        newCredentials      = &credential.Value();
        newTotalCredentials = 1;
    }

    if (!mDelegate.SetUser(userIndex, creatorFabricIdx, creatorFabricIdx, newUserName, newUserUniqueId, newUserStatus, newUserType,
                           newCredentialRule, newCredentials, newTotalCredentials))
    {
        ChipLogProgress(Zcl,
                        "[createUser] Unable to create user: app error "
                        "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userUniqueId=0x%" PRIx32 ",userStatus=%u,"
                        "userType=%u,credentialRule=%u,totalCredentials=%u]",
                        mPath.mEndpointId, creatorFabricIdx, userIndex, newUserUniqueId, to_underlying(newUserStatus),
                        to_underlying(newUserType), to_underlying(newCredentialRule), static_cast<unsigned>(newTotalCredentials));
        return ClusterStatusCode(Status::Failure);
    }

    ChipLogProgress(Zcl,
                    "[createUser] User created "
                    "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userUniqueId=0x%" PRIx32 ",userStatus=%u,userType=%u,"
                    "credentialRule=%u,totalCredentials=%u]",
                    mPath.mEndpointId, creatorFabricIdx, userIndex, newUserUniqueId, to_underlying(newUserStatus),
                    to_underlying(newUserType), to_underlying(newCredentialRule), static_cast<unsigned>(newTotalCredentials));
    SendRemoteLockUserChange(LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kAdd, sourceNodeId, creatorFabricIdx, userIndex,
                             userIndex);

    return ClusterStatusCode(Status::Success);
}

Status DoorLockCluster::ModifyUser(FabricIndex modifierFabricIndex, NodeId sourceNodeId, uint16_t userIndex,
                                   const DataModel::Nullable<CharSpan> & userName,
                                   const DataModel::Nullable<uint32_t> & userUniqueId,
                                   const DataModel::Nullable<UserStatusEnum> & userStatus,
                                   const DataModel::Nullable<UserTypeEnum> & userType,
                                   const DataModel::Nullable<CredentialRuleEnum> & credentialRule)
{
    // We should get the user by that index first
    UserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogError(Zcl, "[modifyUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
        return Status::Failure;
    }

    // appclusters, 5.2.4.34: to modify user its status should NOT be set to Available. If it is we should return INVALID_COMMAND.
    if (UserStatusEnum::kAvailable == user.userStatus)
    {
        ChipLogProgress(Zcl, "[modifyUser] Unable to modify non-existing user [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                        userIndex);
        return Status::InvalidCommand;
    }

    // appclusters, 5.2.4.34: UserName SHALL be null if modifying a user record that was not created by the accessing fabric
    if (user.createdBy != modifierFabricIndex && !userName.IsNull())
    {
        ChipLogProgress(Zcl,
                        "[modifyUser] Unable to modify name of user created by different fabric "
                        "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                        mPath.mEndpointId, userIndex, user.createdBy, modifierFabricIndex);
        return Status::InvalidCommand;
    }

    // appclusters, 5.2.4.34: UserUniqueID SHALL be null if modifying the user record that was not created by the accessing fabric.
    if (user.createdBy != modifierFabricIndex && !userUniqueId.IsNull())
    {
        ChipLogProgress(Zcl,
                        "[modifyUser] Unable to modify UUID of user created by different fabric "
                        "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                        mPath.mEndpointId, userIndex, user.createdBy, modifierFabricIndex);
        return Status::InvalidCommand;
    }

    const auto & newUserName = !userName.IsNull() ? userName.Value() : user.userName;
    auto newUserUniqueId     = userUniqueId.IsNull() ? user.userUniqueId : userUniqueId.Value();
    auto newUserStatus       = userStatus.IsNull() ? user.userStatus : userStatus.Value();
    auto newUserType         = userType.IsNull() ? user.userType : userType.Value();
    auto newCredentialRule   = credentialRule.IsNull() ? user.credentialRule : credentialRule.Value();

    if (!mDelegate.SetUser(userIndex, user.createdBy, modifierFabricIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                           newCredentialRule, user.credentials.data(), user.credentials.size()))
    {
        ChipLogError(Zcl,
                     "[modifyUser] Unable to modify the user: app error "
                     "[endpointId=%d,modifierFabric=%d,userIndex=%d,userUniqueId=0x%" PRIx32 ",userStatus=%u,userType=%u"
                     ",credentialRule=%u]",
                     mPath.mEndpointId, modifierFabricIndex, userIndex, newUserUniqueId, to_underlying(newUserStatus),
                     to_underlying(newUserType), to_underlying(newCredentialRule));
        return Status::Failure;
    }

    ChipLogProgress(Zcl,
                    "[modifyUser] User modified "
                    "[endpointId=%d,modifierFabric=%d,userIndex=%d,userUniqueId=0x%" PRIx32 ",userStatus=%u"
                    ",userType=%u,credentialRule=%u]",
                    mPath.mEndpointId, modifierFabricIndex, userIndex, newUserUniqueId, to_underlying(newUserStatus),
                    to_underlying(newUserType), to_underlying(newCredentialRule));
    SendRemoteLockUserChange(LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kModify, sourceNodeId, modifierFabricIndex,
                             userIndex, userIndex);

    return Status::Success;
}

Status DoorLockCluster::ClearUser(FabricIndex modifierFabricId, NodeId sourceNodeId, uint16_t userIndex, bool sendUserChangeEvent)
{
    UserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        return Status::Failure;
    }

    return ClearUser(modifierFabricId, sourceNodeId, userIndex, user, sendUserChangeEvent);
}

Status DoorLockCluster::ClearUser(FabricIndex modifierFabricId, NodeId sourceNodeId, uint16_t userIndex, const UserInfo & user,
                                  bool sendUserChangeEvent)
{
    // appclusters, 5.2.4.37: all the credentials associated with user should be cleared when clearing the user
    for (const auto & credential : user.credentials)
    {
        ChipLogProgress(
            Zcl, "[ClearUser] Clearing associated credential [endpointId=%d,userIndex=%d,credentialType=%u,credentialIndex=%d]",
            mPath.mEndpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex);

        if (!mDelegate.SetCredential(credential.credentialIndex, kUndefinedFabricIndex, kUndefinedFabricIndex,
                                     DlCredentialStatus::kAvailable, credential.credentialType, ByteSpan()))
        {
            ChipLogError(Zcl,
                         "[ClearUser] Unable to remove credentials associated with user - internal error "
                         "[endpointId=%d,userIndex=%d,credentialIndex=%d,credentialType=%u]",
                         mPath.mEndpointId, userIndex, credential.credentialIndex, to_underlying(credential.credentialType));
            return Status::Failure;
        }
    }

    // Clear all the user schedules
    auto status = ClearSchedules(userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to delete schedules - internal error [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
    }

    // Remove the user entry
    if (!mDelegate.SetUser(userIndex, kUndefinedFabricIndex, kUndefinedFabricIndex, ""_span, 0, UserStatusEnum::kAvailable,
                           UserTypeEnum::kUnrestrictedUser, CredentialRuleEnum::kSingle, nullptr, 0))
    {
        return Status::Failure;
    }

    if (sendUserChangeEvent)
    {
        SendRemoteLockUserChange(LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kClear, sourceNodeId, modifierFabricId,
                                 userIndex, userIndex);
    }
    return Status::Success;
}

bool DoorLockCluster::ClearFabricFromUsers(FabricIndex fabricIndex) const
{
    for (uint16_t userIndex = 1; userIndex <= mNumberOfTotalUsersSupported; ++userIndex)
    {
        UserInfo user;
        if (!mDelegate.GetUser(userIndex, user))
        {
            ChipLogError(Zcl,
                         "[OnFabricRemoved] Unable to get the user - internal error [endpointId=%d,fabricIndex=%d,userIndex=%d]",
                         mPath.mEndpointId, fabricIndex, userIndex);
            continue;
        }

        // Filter out unoccupied slots and users that don't have corresponding fabricIndex in the created/modified fields
        if (UserStatusEnum::kAvailable == user.userStatus || (fabricIndex != user.createdBy && fabricIndex != user.lastModifiedBy))
        {
            continue;
        }

        if (user.createdBy == fabricIndex)
        {
            user.createdBy = kUndefinedFabricIndex;
        }

        if (user.lastModifiedBy == fabricIndex)
        {
            user.lastModifiedBy = kUndefinedFabricIndex;
        }

        if (!mDelegate.SetUser(userIndex, user.createdBy, user.lastModifiedBy, user.userName, user.userUniqueId, user.userStatus,
                               user.userType, user.credentialRule, user.credentials.data(), user.credentials.size()))
        {
            ChipLogError(
                Zcl,
                "[OnFabricRemoved] Unable to update the user fabrics - internal error [endpointId=%d,fabricIndex=%d,userIndex=%d]",
                mPath.mEndpointId, fabricIndex, userIndex);
        }
    }
    return true;
}

// =============================================================================
// Schedule clear helpers
// =============================================================================

DlStatus DoorLockCluster::ClearWeekDaySchedule(uint16_t userIndex, uint8_t weekDayIndex) const
{
    auto status = mDelegate.SetSchedule(weekDayIndex, userIndex, DlScheduleStatus::kAvailable, DaysMaskMap(0), 0, 0, 0, 0);
    if (DlStatus::kSuccess != status && DlStatus::kNotFound != status)
    {
        ChipLogError(Zcl,
                     "[ClearWeekDaySchedule] Unable to clear the schedule - internal error "
                     "[endpointId=%d,userIndex=%d,scheduleIndex=%d,status=%u]",
                     mPath.mEndpointId, userIndex, weekDayIndex, to_underlying(status));
        return status;
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ClearWeekDaySchedules(uint16_t userIndex) const
{
    for (uint8_t i = 1; i <= mNumberOfWeekDaySchedulesPerUser; ++i)
    {
        auto status = ClearWeekDaySchedule(userIndex, i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ClearSchedules(uint16_t userIndex) const
{
    auto status = ClearWeekDaySchedules(userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[ClearAllSchedules] Unable to clear week day schedules for user - internal error "
                     "[endpointId=%d,userIndex=%d,status=%u]",
                     mPath.mEndpointId, userIndex, to_underlying(status));
        return status;
    }

    status = ClearYearDaySchedules(userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[ClearAllSchedules] Unable to clear year day schedules for user - internal error "
                     "[endpointId=%d,userIndex=%d,status=%u]",
                     mPath.mEndpointId, userIndex, to_underlying(status));
        return status;
    }

    // No need to clear the holiday schedules here as they are not specific for user.
    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ClearYearDaySchedule(uint16_t userIndex, uint8_t yearDayIndex) const
{
    auto status = mDelegate.SetSchedule(yearDayIndex, userIndex, DlScheduleStatus::kAvailable, 0, 0);
    if (DlStatus::kSuccess != status && DlStatus::kNotFound != status)
    {
        ChipLogError(Zcl,
                     "[ClearYearDaySchedule] Unable to clear the schedule - internal error "
                     "[endpointId=%d,userIndex=%d,scheduleIndex=%d,status=%u]",
                     mPath.mEndpointId, userIndex, yearDayIndex, to_underlying(status));
        return status;
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ClearYearDaySchedules(uint16_t userIndex) const
{
    for (uint8_t i = 1; i <= mNumberOfYearDaySchedulesPerUser; ++i)
    {
        auto status = ClearYearDaySchedule(userIndex, i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ClearHolidaySchedule(uint8_t holidayIndex) const
{
    auto status = mDelegate.SetSchedule(holidayIndex, DlScheduleStatus::kAvailable, 0, 0, OperatingModeEnum::kNormal);
    if (DlStatus::kSuccess != status && DlStatus::kNotFound != status)
    {
        ChipLogError(
            Zcl, "[ClearHolidaySchedule] Unable to clear the schedule - internal error [endpointId=%d,scheduleIndex=%d,status=%u]",
            mPath.mEndpointId, holidayIndex, to_underlying(status));
        return status;
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ClearHolidaySchedules() const
{
    for (uint8_t i = 1; i <= mNumberOfHolidaySchedulesSupported; ++i)
    {
        auto status = ClearHolidaySchedule(i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

// =============================================================================
// User management command handlers
// =============================================================================

void DoorLockCluster::HandleSetUser(const DataModel::InvokeRequest & request, const Commands::SetUser::DecodableType & commandData,
                                    CommandHandler * handler, const ConcreteCommandPath & path)
{
    auto & userIndex = commandData.userIndex;
    ChipLogProgress(Zcl, "[SetUser] Incoming command [endpointId=%d,userIndex=%d]", mPath.mEndpointId, userIndex);

    if (!SupportsUSR())
    {
        ChipLogProgress(Zcl, "[SetUser] User management is not supported [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::UnsupportedCommand);
        return;
    }

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", mPath.mEndpointId, userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto & operationType  = commandData.operationType;
    auto & userName       = commandData.userName;
    auto & userUniqueId   = commandData.userUniqueID;
    auto & userStatus     = commandData.userStatus;
    auto & userType       = commandData.userType;
    auto & credentialRule = commandData.credentialRule;

    if (!UserIndexValid(userIndex))
    {
        ChipLogProgress(Zcl, "[SetUser] User index out of bounds [endpointId=%d,userIndex=%d]", mPath.mEndpointId, userIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    // appclusters, 5.2.4.34: UserName has maximum kDoorLockMaxUsernameSize (10) characters excluding NUL terminator in it.
    if (!userName.IsNull() && userName.Value().size() > kDoorLockMaxUsernameSize)
    {
        ChipLogProgress(Zcl, "[SetUser] Unable to set user: userName too long [endpointId=%d,userIndex=%d,userNameSize=%u]",
                        mPath.mEndpointId, userIndex, static_cast<unsigned>(userName.Value().size()));

        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (!IsValidUserStatusForSet(userStatus))
    {
        ChipLogProgress(Zcl,
                        "[SetUser] Unable to set the user: user status is out of range [endpointId=%d,userIndex=%d,userStatus=%u]",
                        mPath.mEndpointId, userIndex, to_underlying(userStatus.Value()));

        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (userType == UserTypeEnum::kUnknownEnumValue)
    {
        ChipLogProgress(Zcl, "[SetUser] Unable to set the user: user type is unknown [endpointId=%d,userIndex=%d,userType=%u]",
                        mPath.mEndpointId, userIndex, to_underlying(userType.Value()));

        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    ClusterStatusCode status(Status::Success);
    switch (operationType)
    {
    case DataOperationTypeEnum::kAdd:
        status = CreateUser(fabricIdx, sourceNodeId, userIndex, userName, userUniqueId, userStatus, userType, credentialRule);
        break;
    case DataOperationTypeEnum::kModify:
        status = ClusterStatusCode(
            ModifyUser(fabricIdx, sourceNodeId, userIndex, userName, userUniqueId, userStatus, userType, credentialRule));
        break;
    case DataOperationTypeEnum::kClear:
    default:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        // or anything else
        status = ClusterStatusCode(Status::InvalidCommand);
        ChipLogProgress(Zcl, "[SetUser] Invalid operation type [endpointId=%d,operationType=%u]", mPath.mEndpointId,
                        to_underlying(operationType));
        break;
    }

    if (const auto clusterStatus = status.GetClusterSpecificCode(); clusterStatus.has_value())
    {
        SuccessOrDie(handler->AddClusterSpecificFailure(path, *clusterStatus));
        return;
    }
    handler->AddStatus(path, status.GetStatus());
}

void DoorLockCluster::HandleGetUser(const DataModel::InvokeRequest & request, uint16_t userIndex, CommandHandler * handler,
                                    const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[GetUser] Incoming command [endpointId=%d,userIndex=%d]", mPath.mEndpointId, userIndex);

    if (!SupportsUSR())
    {
        ChipLogProgress(Zcl, "[GetUser] User management is not supported [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::UnsupportedCommand);
        return;
    }

    uint16_t maxNumberOfUsers = 0;
    if (!UserIndexValid(userIndex, maxNumberOfUsers))
    {
        ChipLogProgress(Zcl, "[GetUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]", userIndex,
                        maxNumberOfUsers);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    Commands::GetUserResponse::Type response;

    // appclusters, 5.2.4.36.1: We need to add next occupied user after userIndex if any.
    //
    // We want to do this before we call mDelegate.GetUser, because the
    // UserInfo might point into some application-static
    // buffers (for its credentials and whatnot).
    uint16_t nextAvailableUserIndex = 0;
    if (FindOccupiedUserSlot(static_cast<uint16_t>(userIndex + 1), nextAvailableUserIndex))
    {
        response.nextUserIndex.SetNonNull(nextAvailableUserIndex);
    }

    UserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogProgress(Zcl, "[GetUser] Could not get user info [userIndex=%d]", userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    response.userIndex = userIndex;

    // appclusters, 5.2.4.36: we should not set user-specific fields to non-null if the user status is set to Available
    if (UserStatusEnum::kAvailable != user.userStatus)
    {
        ChipLogProgress(Zcl,
                        "Found user in storage: "
                        "[userIndex=%d,userStatus=%u,userType=%u,credentialRule=%u,createdBy=%u,modifiedBy=%u]",
                        userIndex, to_underlying(user.userStatus), to_underlying(user.userType), to_underlying(user.credentialRule),
                        user.createdBy, user.lastModifiedBy);

        response.userName.SetNonNull(user.userName);
        if (0xFFFFFFFFU != user.userUniqueId)
        {
            response.userUniqueID.SetNonNull(user.userUniqueId);
        }
        response.userStatus.SetNonNull(user.userStatus);
        response.userType.SetNonNull(user.userType);
        response.credentialRule.SetNonNull(user.credentialRule);
        response.credentials.SetNonNull(user.credentials);
        // Set fabric indices only if the user was created/modified by matter.
        if (user.creationSource == DlAssetSource::kMatterIM)
        {
            response.creatorFabricIndex.SetNonNull(user.createdBy);
        }
        if (user.modificationSource == DlAssetSource::kMatterIM)
        {
            response.lastModifiedFabricIndex.SetNonNull(user.lastModifiedBy);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "[GetUser] User not found [userIndex=%d]", userIndex);
    }

    handler->AddResponse(path, response);
}

void DoorLockCluster::HandleClearUser(const DataModel::InvokeRequest & request, uint16_t userIndex, CommandHandler * handler,
                                      const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[ClearUser] Incoming command [endpointId=%d,userIndex=%d]", mPath.mEndpointId, userIndex);

    if (!SupportsUSR())
    {
        ChipLogProgress(Zcl, "[ClearUser] User management is not supported [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::UnsupportedCommand);
        return;
    }

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", mPath.mEndpointId, userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    uint16_t maxNumberOfUsers = 0;
    if (!UserIndexValid(userIndex, maxNumberOfUsers) && userIndex != DOOR_LOCK_ALL_USERS_INDEX)
    {
        ChipLogProgress(Zcl, "[ClearUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]", userIndex,
                        maxNumberOfUsers);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (DOOR_LOCK_ALL_USERS_INDEX != userIndex)
    {
        auto status = ClearUser(fabricIdx, sourceNodeId, userIndex, true);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", userIndex,
                         to_underlying(status));
        }
        handler->AddStatus(path, status);
        return;
    }

    ChipLogProgress(Zcl, "[ClearUser] Removing all users from storage");
    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        auto status = ClearUser(fabricIdx, sourceNodeId, i, false);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", i,
                         to_underlying(status));

            handler->AddStatus(path, Status::Failure);
            return;
        }
    }
    ChipLogProgress(Zcl, "[ClearUser] Removed all users from storage [users=%d]", maxNumberOfUsers);

    SendRemoteLockUserChange(LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kClear, sourceNodeId, fabricIdx,
                             DOOR_LOCK_ALL_USERS_INDEX, DOOR_LOCK_ALL_USERS_INDEX);

    handler->AddStatus(path, Status::Success);
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
