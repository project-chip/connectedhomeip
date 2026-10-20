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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedMemoryBuffer.h>
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
    static constexpr DataModel::AcceptedCommandEntry kWeekDayScheduleCommands[] = {
        Commands::SetWeekDaySchedule::kMetadataEntry,
        Commands::GetWeekDaySchedule::kMetadataEntry,
        Commands::ClearWeekDaySchedule::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kYearDayScheduleCommands[] = {
        Commands::SetYearDaySchedule::kMetadataEntry,
        Commands::GetYearDaySchedule::kMetadataEntry,
        Commands::ClearYearDaySchedule::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kHolidayScheduleCommands[] = {
        Commands::SetHolidaySchedule::kMetadataEntry,
        Commands::GetHolidaySchedule::kMetadataEntry,
        Commands::ClearHolidaySchedule::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kUserCommands[] = {
        Commands::SetUser::kMetadataEntry,
        Commands::GetUser::kMetadataEntry,
        Commands::ClearUser::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kCredentialCommands[] = {
        Commands::SetCredential::kMetadataEntry,
        Commands::GetCredentialStatus::kMetadataEntry,
        Commands::ClearCredential::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kAliroCommands[] = {
        Commands::SetAliroReaderConfig::kMetadataEntry,
        Commands::ClearAliroReaderConfig::kMetadataEntry,
    };
    // clang-format on

    const bool anyCredentialFeature =
        SupportsPIN() || SupportsRFID() || SupportsFingers() || SupportsFace() || SupportsAliroProvisioning();

    ReturnErrorOnFailure(builder.AppendElements(Span(kLockCommands)));
    if (mFeatures.Has(Feature::kUnbolt))
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kUnboltCommand)));
    }
    if (SupportsWeekDaySchedules())
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kWeekDayScheduleCommands)));
    }
    if (SupportsYearDaySchedules())
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kYearDayScheduleCommands)));
    }
    if (SupportsHolidaySchedules())
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kHolidayScheduleCommands)));
    }
    if (SupportsUSR())
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kUserCommands)));
    }
    if (anyCredentialFeature)
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kCredentialCommands)));
    }
    if (SupportsAliroProvisioning())
    {
        ReturnErrorOnFailure(builder.AppendElements(Span(kAliroCommands)));
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
    case Commands::SetCredential::Id: {
        Commands::SetCredential::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleSetCredential(request, commandData, handler, request.path);
        return std::nullopt;
    }
    case Commands::GetCredentialStatus::Id: {
        Commands::GetCredentialStatus::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleGetCredentialStatus(commandData.credential.credentialType, commandData.credential.credentialIndex, handler,
                                  request.path);
        return std::nullopt;
    }
    case Commands::ClearCredential::Id: {
        Commands::ClearCredential::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleClearCredential(request, commandData, handler, request.path);
        return std::nullopt;
    }
    case Commands::SetWeekDaySchedule::Id: {
        Commands::SetWeekDaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleSetWeekDaySchedule(request, commandData.weekDayIndex, commandData.userIndex, commandData.daysMask,
                                 commandData.startHour, commandData.startMinute, commandData.endHour, commandData.endMinute,
                                 handler, request.path);
        return std::nullopt;
    }
    case Commands::GetWeekDaySchedule::Id: {
        Commands::GetWeekDaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleGetWeekDaySchedule(commandData.weekDayIndex, commandData.userIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::ClearWeekDaySchedule::Id: {
        Commands::ClearWeekDaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleClearWeekDaySchedule(request, commandData.weekDayIndex, commandData.userIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::SetYearDaySchedule::Id: {
        Commands::SetYearDaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleSetYearDaySchedule(request, commandData.yearDayIndex, commandData.userIndex, commandData.localStartTime,
                                 commandData.localEndTime, handler, request.path);
        return std::nullopt;
    }
    case Commands::GetYearDaySchedule::Id: {
        Commands::GetYearDaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleGetYearDaySchedule(commandData.yearDayIndex, commandData.userIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::ClearYearDaySchedule::Id: {
        Commands::ClearYearDaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleClearYearDaySchedule(request, commandData.yearDayIndex, commandData.userIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::SetHolidaySchedule::Id: {
        Commands::SetHolidaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleSetHolidaySchedule(commandData.holidayIndex, commandData.localStartTime, commandData.localEndTime,
                                 commandData.operatingMode, handler, request.path);
        return std::nullopt;
    }
    case Commands::GetHolidaySchedule::Id: {
        Commands::GetHolidaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleGetHolidaySchedule(commandData.holidayIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::ClearHolidaySchedule::Id: {
        Commands::ClearHolidaySchedule::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleClearHolidaySchedule(commandData.holidayIndex, handler, request.path);
        return std::nullopt;
    }
    case Commands::SetAliroReaderConfig::Id: {
        Commands::SetAliroReaderConfig::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleSetAliroReaderConfig(commandData.signingKey, commandData.verificationKey, commandData.groupIdentifier,
                                   commandData.groupResolvingKey, handler, request.path);
        return std::nullopt;
    }
    case Commands::ClearAliroReaderConfig::Id: {
        Commands::ClearAliroReaderConfig::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        (void) commandData;
        HandleClearAliroReaderConfig(handler, request.path);
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
    bool performOperation     = false;

    // Matched credential details, reported in the LockOperationError event when
    // the attempt fails.
    DataModel::Nullable<uint16_t> pinUserIdx;
    Span<const CredentialStruct> presentedCredentials;

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
    else if (pinCode.HasValue())
    {
        // appclusters 5.3.4.1: If the PINCode field is provided, the door lock
        // SHALL verify PINCode before granting access regardless of the value of
        // RequirePINForRemoteOperation.
        if (!SupportsPIN() || !SupportsUSR())
        {
            ChipLogProgress(Zcl, "PIN code is supplied while USR/PIN features are disabled. Exiting [endpoint=%d, lock_op=%d]",
                            mPath.mEndpointId, to_underlying(opType));
        }
        else
        {
            performOperation = true;
            EmberAfPluginDoorLockUserInfo user;
            uint16_t userIdx    = 0;
            uint16_t pinCredIdx = 0;
            if (FindUserIndexByCredential(CredentialTypeEnum::kPin, pinCode.Value(), userIdx, pinCredIdx, user))
            {
                pinUserIdx.SetNonNull(userIdx);
                CredentialStruct foundCred = { CredentialTypeEnum::kPin, pinCredIdx };
                presentedCredentials       = Span<const CredentialStruct>(&foundCred, 1);

                // If the user status is OccupiedDisabled we should deny the access and send out the appropriate event
                if (user.userStatus == UserStatusEnum::kOccupiedDisabled)
                {
                    ChipLogProgress(
                        Zcl, "Unable to perform remote lock operation: user is disabled [endpoint=%d, lock_op=%d, userIndex=%d]",
                        mPath.mEndpointId, to_underlying(opType), userIdx);
                    reason = OperationErrorEnum::kDisabledUserDenied;
                }
            }
            else
            {
                ChipLogProgress(Zcl, "Rejecting lock operation: unknown PIN provided [endpoint=%d, lock_op=%d]", mPath.mEndpointId,
                                to_underlying(opType));
                reason = OperationErrorEnum::kInvalidCredential;
            }
        }
    }
    else
    {
        // appclusters 5.3.4.1: If the RequirePINForRemoteOperation attribute is
        // True then PINCode field SHALL be provided and the door lock SHALL NOT
        // grant access if it is not provided. This attribute exists when COTA
        // and PIN features are both enabled.
        // No PIN: proceed with the actuation unless a PIN is required.
        performOperation = true;
        if (mFeatures.Has(Feature::kCredentialsOverTheAirAccess) && SupportsPIN() && mRequirePINforRemoteOperation)
        {
            ChipLogProgress(Zcl, "Checking credentials failed: PIN is not provided when it is required [endpoint=%d]",
                            mPath.mEndpointId);
            reason           = OperationErrorEnum::kInvalidCredential;
            performOperation = false;
        }
    }

    if (performOperation && reason == OperationErrorEnum::kUnspecified)
    {
        // Credentials check succeeded (or was not required): actuate the
        // hardware; the delegate reports the failure reason (nullopt = success).
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
                               OperationSourceEnum::kRemote, reason, pinUserIdx, presentedCredentials, GetOperationFabric(request),
                               GetOperationNode(request), /* opSuccess */ false);
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

constexpr uint8_t DOOR_LOCK_SCHEDULE_MAX_HOUR               = 23;
constexpr uint8_t DOOR_LOCK_SCHEDULE_MAX_MINUTE             = 59;
constexpr uint8_t DOOR_LOCK_ALIRO_CREDENTIAL_SIZE           = 65;
constexpr uint8_t DOOR_LOCK_ALIRO_UNCOMPRESSED_POINT_PREFIX = 0x04;

// Wildcard indices for the clear-all variants of the user / credential /
// schedule "clear" commands.
constexpr uint16_t DOOR_LOCK_ALL_USERS_INDEX    = 0xFFFE;
constexpr uint8_t DOOR_LOCK_ALL_SCHEDULES_INDEX = 0xFE;

bool IsValidUserStatusForSet(const DataModel::Nullable<UserStatusEnum> & userStatus)
{
    return userStatus.IsNull() || (userStatus.Value() == UserStatusEnum::kOccupiedEnabled) ||
        (userStatus.Value() == UserStatusEnum::kOccupiedDisabled);
}

bool CredentialDataEqualConstantTime(const ByteSpan & a, const ByteSpan & b)
{
    return a.size() == b.size() && (a.empty() || Crypto::IsBufferContentEqualConstantTime(a.data(), b.data(), a.size()));
}

bool IsAliroCredentialType(CredentialTypeEnum credentialType)
{
    switch (credentialType)
    {
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        return true;
    default:
        return false;
    }
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
    EmberAfPluginDoorLockUserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogError(Zcl, "[UserExists] Unable to get the user - internal error [endpointId=%d,userIndex=%d]", mPath.mEndpointId,
                     userIndex);
        return false;
    }
    return UserStatusEnum::kAvailable != user.userStatus;
}

bool DoorLockCluster::GetMaxNumberOfCredentials(CredentialTypeEnum credentialType, uint16_t & maxNumberOfCredentials) const
{
    maxNumberOfCredentials = 0;
    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        maxNumberOfCredentials = 1;
        return true;
    case CredentialTypeEnum::kPin:
        maxNumberOfCredentials = mNumberOfPINUsersSupported;
        return SupportsPIN();
    case CredentialTypeEnum::kRfid:
        maxNumberOfCredentials = mNumberOfRFIDUsersSupported;
        return SupportsRFID();
    case CredentialTypeEnum::kFingerprint:
        return mDelegate.GetNumberOfFingerprintCredentialsSupported(maxNumberOfCredentials);
    case CredentialTypeEnum::kFingerVein:
        return mDelegate.GetNumberOfFingerVeinCredentialsSupported(maxNumberOfCredentials);
    case CredentialTypeEnum::kFace:
        return mDelegate.GetNumberOfFaceCredentialsSupported(maxNumberOfCredentials);
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
        maxNumberOfCredentials = mDelegate.GetNumberOfAliroCredentialIssuerKeysSupported();
        return true;
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        // For the Evictable / NonEvictable endpoint keys the count of the
        // total endpoint keys applies.
        maxNumberOfCredentials = mDelegate.GetNumberOfAliroEndpointKeysSupported();
        return true;
    default:
        return false;
    }
}

bool DoorLockCluster::CredentialIndexValid(CredentialTypeEnum type, uint16_t credentialIndex,
                                           uint16_t & maxNumberOfCredentials) const
{
    if (!GetMaxNumberOfCredentials(type, maxNumberOfCredentials))
    {
        return false;
    }

    // appclusters, 5.2.6.3.1: 0 is allowed index for Programming PIN credential only
    if (CredentialTypeEnum::kProgrammingPIN == type)
    {
        return (0 == credentialIndex);
    }

    return credentialIndex != 0 && credentialIndex <= maxNumberOfCredentials;
}

DlStatus DoorLockCluster::CredentialLengthWithinRange(CredentialTypeEnum type, const ByteSpan & credentialData) const
{
    bool statusMin = false, statusMax = false;
    uint8_t minLen = 0, maxLen = 0;

    switch (type)
    {
    case CredentialTypeEnum::kProgrammingPIN:
    case CredentialTypeEnum::kPin:
        statusMin = statusMax = true;
        minLen                = mMinPINCodeLength;
        maxLen                = mMaxPINCodeLength;
        break;
    case CredentialTypeEnum::kRfid:
        statusMin = statusMax = true;
        minLen                = mMinRFIDCodeLength;
        maxLen                = mMaxRFIDCodeLength;
        break;
    case CredentialTypeEnum::kFingerprint:
        statusMin = statusMax = mDelegate.GetFingerprintCredentialLengthConstraints(minLen, maxLen);
        break;
    case CredentialTypeEnum::kFingerVein:
        statusMin = statusMax = mDelegate.GetFingerVeinCredentialLengthConstraints(minLen, maxLen);
        break;
    case CredentialTypeEnum::kFace:
        statusMin = statusMax = mDelegate.GetFaceCredentialLengthConstraints(minLen, maxLen);
        break;
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        minLen = maxLen = DOOR_LOCK_ALIRO_CREDENTIAL_SIZE;
        statusMin = statusMax = true;
        break;
    default:
        return DlStatus::kFailure;
    }

    if (!statusMin || !statusMax)
    {
        ChipLogError(Zcl, "Unable to read attributes to get min/max length for credentials [endpointId=%d,credentialType=%u]",
                     mPath.mEndpointId, to_underlying(type));
        return DlStatus::kFailure;
    }

    if (credentialData.size() < minLen || credentialData.size() > maxLen)
    {
        ChipLogProgress(Zcl,
                        "Credential data size is out of range "
                        "[endpointId=%d,credentialType=%u,minLength=%u,maxLength=%u,length=%u]",
                        mPath.mEndpointId, to_underlying(type), minLen, maxLen, static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kInvalidField;
    }

    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::AliroCredentialDataValid(CredentialTypeEnum type, const ByteSpan & credentialData) const
{
    switch (type)
    {
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        // Aliro credentials must be an uncompressed EC P-256 public key (0x04 || X || Y).
        if (credentialData.empty() || credentialData[0] != DOOR_LOCK_ALIRO_UNCOMPRESSED_POINT_PREFIX)
        {
            ChipLogProgress(Zcl, "Aliro credential data must be an uncompressed EC public key [credentialType=%u]",
                            to_underlying(type));
            return DlStatus::kInvalidField;
        }
        break;
    default:
        break;
    }

    return DlStatus::kSuccess;
}

bool DoorLockCluster::FindOccupiedUserSlot(uint16_t startIndex, uint16_t & userIndex) const
{
    userIndex = 0;
    for (uint16_t i = startIndex; i <= mNumberOfTotalUsersSupported; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
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
        EmberAfPluginDoorLockUserInfo user;
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

bool DoorLockCluster::FindOccupiedCredentialSlot(CredentialTypeEnum credentialType, uint16_t startIndex,
                                                 uint16_t & credentialIndex) const
{
    uint16_t maxNumberOfCredentials = 0;
    if (!GetMaxNumberOfCredentials(credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    // Programming PIN index starts with 0, and it is assumed that it is unique. Therefore different bounds checking for that
    // credential type
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        maxNumberOfCredentials--;
    }

    for (uint16_t i = startIndex; i <= maxNumberOfCredentials; ++i)
    {
        EmberAfPluginDoorLockCredentialInfo info;
        if (!mDelegate.GetCredential(i, credentialType, info))
        {
            ChipLogError(Zcl, "Unable to get credential: app error [endpointId=%d,credentialType=%u,credentialIndex=%d]",
                         mPath.mEndpointId, to_underlying(credentialType), i);
            return false;
        }

        if (DlCredentialStatus::kAvailable != info.status)
        {
            credentialIndex = i;
            return true;
        }
    }

    return false;
}

bool DoorLockCluster::FindUnoccupiedCredentialSlot(CredentialTypeEnum credentialType, uint16_t startIndex,
                                                   uint16_t & credentialIndex) const
{
    uint16_t maxNumberOfCredentials = 0;
    if (!GetMaxNumberOfCredentials(credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    // Programming PIN index starts with 0, and it is assumed that it is unique. Therefore different bounds checking for that
    // credential type
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        maxNumberOfCredentials--;
    }

    for (uint16_t i = startIndex; i <= maxNumberOfCredentials; ++i)
    {
        EmberAfPluginDoorLockCredentialInfo info;
        if (!mDelegate.GetCredential(i, credentialType, info))
        {
            ChipLogError(Zcl, "Unable to get credential: app error [endpointId=%d,credentialType=%u,credentialIndex=%d]",
                         mPath.mEndpointId, to_underlying(credentialType), i);
            return false;
        }

        if (DlCredentialStatus::kAvailable == info.status)
        {
            credentialIndex = i;
            return true;
        }
    }

    return false;
}

bool DoorLockCluster::FindUserIndexByCredential(CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                                uint16_t & userIndex) const
{
    for (uint16_t i = 1; i <= mNumberOfTotalUsersSupported; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!mDelegate.GetUser(i, user))
        {
            ChipLogError(Zcl, "[GetCredentialStatus] Unable to get user: app error [userIndex=%d]", i);
            return false;
        }

        // Go through occupied users only
        if (UserStatusEnum::kAvailable == user.userStatus)
        {
            continue;
        }

        for (size_t j = 0; j < user.credentials.size(); ++j)
        {
            if (user.credentials.data()[j].credentialIndex == credentialIndex &&
                user.credentials.data()[j].credentialType == credentialType)
            {
                userIndex = i;
                return true;
            }
        }
    }

    return false;
}

bool DoorLockCluster::FindUserIndexByCredential(CredentialTypeEnum credentialType, ByteSpan credentialData, uint16_t & userIndex,
                                                uint16_t & credentialIndex, EmberAfPluginDoorLockUserInfo & userInfo) const
{
    for (uint16_t i = 1; i <= mNumberOfTotalUsersSupported; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!mDelegate.GetUser(i, user))
        {
            ChipLogError(Zcl, "[findUserIndexByCredential] Unable to get user: app error [userIndex=%d]", i);
            return false;
        }

        // Go through occupied users only
        if (UserStatusEnum::kAvailable == user.userStatus)
        {
            continue;
        }

        for (const auto & credential : user.credentials)
        {
            if (credential.credentialType != credentialType)
            {
                continue;
            }

            EmberAfPluginDoorLockCredentialInfo credentialInfo;
            if (!mDelegate.GetCredential(credential.credentialIndex, credentialType, credentialInfo))
            {
                ChipLogError(Zcl,
                             "[findUserIndexByCredential] Unable to get credential: app error "
                             "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                             i, credential.credentialIndex, to_underlying(credentialType));
                return false;
            }

            if (credentialInfo.status != DlCredentialStatus::kOccupied)
            {
                ChipLogError(Zcl,
                             "[findUserIndexByCredential] Users/Credentials database error: credential index attached to user is "
                             "not occupied "
                             "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                             i, credential.credentialIndex, to_underlying(credentialType));
                return false;
            }

            if (CredentialDataEqualConstantTime(credentialInfo.credentialData, credentialData))
            {
                userIndex       = i;
                credentialIndex = credential.credentialIndex;
                userInfo        = user;
                return true;
            }
        }
    }

    return false;
}

bool DoorLockCluster::CountOccupiedCredentials(CredentialTypeEnum credentialType, size_t & occupiedCount) const
{
    uint16_t maxCredentialCount;

    if (!GetMaxNumberOfCredentials(credentialType, maxCredentialCount))
    {
        return false;
    }

    uint16_t startIndex = 1;
    // Programming PIN is a special case -- it is unique and its index assumed to be 0.
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        startIndex = 0;
        maxCredentialCount--;
    }

    occupiedCount = 0;
    for (uint16_t credentialIndex = startIndex; credentialIndex <= maxCredentialCount; ++credentialIndex)
    {
        EmberAfPluginDoorLockCredentialInfo credential;
        if (!mDelegate.GetCredential(credentialIndex, credentialType, credential))
        {
            return false;
        }

        if (credential.status == DlCredentialStatus::kOccupied)
        {
            ++occupiedCount;
        }
    }

    return true;
}

bool DoorLockCluster::CredentialTypeSupported(CredentialTypeEnum type) const
{
    switch (type)
    {
    case CredentialTypeEnum::kProgrammingPIN:
    case CredentialTypeEnum::kPin:
        return SupportsPIN();
    case CredentialTypeEnum::kRfid:
        return SupportsRFID();
    case CredentialTypeEnum::kFingerprint:
    case CredentialTypeEnum::kFingerVein:
        return SupportsFingers();
    case CredentialTypeEnum::kFace:
        return SupportsFace();
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        return SupportsAliroProvisioning();
    default:
        return false;
    }
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

LockDataTypeEnum DoorLockCluster::CredentialTypeToLockDataType(CredentialTypeEnum credentialType) const
{
    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        return LockDataTypeEnum::kProgrammingCode;
    case CredentialTypeEnum::kPin:
        return LockDataTypeEnum::kPin;
    case CredentialTypeEnum::kRfid:
        return LockDataTypeEnum::kRfid;
    case CredentialTypeEnum::kFingerprint:
        return LockDataTypeEnum::kFingerprint;
    case CredentialTypeEnum::kFingerVein:
        return LockDataTypeEnum::kFingerVein;
    case CredentialTypeEnum::kFace:
        return LockDataTypeEnum::kFace;
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
        return LockDataTypeEnum::kAliroCredentialIssuerKey;
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
        return LockDataTypeEnum::kAliroEvictableEndpointKey;
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        return LockDataTypeEnum::kAliroNonEvictableEndpointKey;
    case CredentialTypeEnum::kUnknownEnumValue:
        return LockDataTypeEnum::kUnspecified;
    }

    return LockDataTypeEnum::kUnspecified;
}

bool DoorLockCluster::IsUserScheduleRestricted(const EmberAfPluginDoorLockUserInfo & user) const
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
    EmberAfPluginDoorLockUserInfo user;
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
    EmberAfPluginDoorLockUserInfo user;
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
    EmberAfPluginDoorLockUserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        return Status::Failure;
    }

    return ClearUser(modifierFabricId, sourceNodeId, userIndex, user, sendUserChangeEvent);
}

Status DoorLockCluster::ClearUser(FabricIndex modifierFabricId, NodeId sourceNodeId, uint16_t userIndex,
                                  const EmberAfPluginDoorLockUserInfo & user, bool sendUserChangeEvent)
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
        EmberAfPluginDoorLockUserInfo user;
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
// Credential management (create / modify / clear)
// =============================================================================

DlStatus DoorLockCluster::CreateNewCredentialAndUser(FabricIndex creatorFabricIdx, NodeId sourceNodeId,
                                                     const DataModel::Nullable<UserStatusEnum> & userStatus,
                                                     const DataModel::Nullable<UserTypeEnum> & userType,
                                                     const CredentialStruct & credential, const ByteSpan & credentialData,
                                                     uint16_t & createdUserIndex)
{
    uint16_t availableUserIndex = 0;
    if (!FindUnoccupiedUserSlot(availableUserIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to create new user for credential: no available user slots "
                        "[endpointId=%d,credentialIndex=%d]",
                        mPath.mEndpointId, credential.credentialIndex);
        return DlStatus::kOccupied;
    }

    ClusterStatusCode status = CreateUser(
        creatorFabricIdx, sourceNodeId, availableUserIndex, DataModel::Nullable<CharSpan>(), DataModel::Nullable<uint32_t>(),
        userStatus, userType, DataModel::Nullable<CredentialRuleEnum>(), DataModel::Nullable<CredentialStruct>(credential));
    if (!status.IsSuccess())
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to create new user for credential: internal error "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, credential.credentialIndex, availableUserIndex);
        return DlStatus::kFailure;
    }

    if (!mDelegate.SetCredential(credential.credentialIndex, creatorFabricIdx, creatorFabricIdx, DlCredentialStatus::kOccupied,
                                 credential.credentialType, credentialData))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u]",
                        mPath.mEndpointId, credential.credentialIndex, to_underlying(credential.credentialType),
                        static_cast<unsigned>(credentialData.size()));
        return DlStatus::kFailure;
    }

    ChipLogProgress(Zcl,
                    "[SetCredential] Credential and user were created "
                    "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u,userIndex=%d]",
                    mPath.mEndpointId, credential.credentialIndex, to_underlying(credential.credentialType),
                    static_cast<unsigned>(credentialData.size()), availableUserIndex);
    createdUserIndex = availableUserIndex;

    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::CreateNewCredentialAndAddItToUser(FabricIndex modifierFabricIdx, uint16_t userIndex,
                                                            const CredentialStruct & credential, const ByteSpan & credentialData)
{
    if (!UserIndexValid(userIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to add new credential to user: user out of bounds "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, credential.credentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    EmberAfPluginDoorLockUserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogProgress(
            Zcl, "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d,userIndex=%d]",
            mPath.mEndpointId, credential.credentialIndex, userIndex);

        return DlStatus::kFailure;
    }

    // Not in the spec, but common sense: I don't think we need to modify the credential if user slot is not occupied
    if (user.userStatus == UserStatusEnum::kAvailable)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to add credential to user: user slot is empty "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, credential.credentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    // return INVALID_COMMAND if the accessing fabric index doesn't match the CreatorFabricIndex in the user record pointed to by
    // UserIndex
    if (user.createdBy != modifierFabricIdx)
    {
        ChipLogProgress(Zcl,
                        "[createCredential] Unable to create credential for user created by different fabric "
                        "[endpointId=%d,userIndex=%d,creatorIdx=%d,fabricIdx=%d]",
                        mPath.mEndpointId, userIndex, user.createdBy, modifierFabricIdx);
        return DlStatus::kInvalidField;
    }

    // Add new credential to the user
    auto status = AddCredentialToUser(modifierFabricIdx, userIndex, credential);
    if (DlStatus::kSuccess != status)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to add credential to a user: internal error "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%u]",
                        mPath.mEndpointId, credential.credentialIndex, userIndex, to_underlying(status));
        return status;
    }

    if (!mDelegate.SetCredential(credential.credentialIndex, modifierFabricIdx, modifierFabricIdx, DlCredentialStatus::kOccupied,
                                 credential.credentialType, credentialData))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u]",
                        mPath.mEndpointId, credential.credentialIndex, to_underlying(credential.credentialType),
                        static_cast<unsigned>(credentialData.size()));
        return DlStatus::kFailure;
    }

    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::AddCredentialToUser(FabricIndex modifierFabricIdx, uint16_t userIndex,
                                              const CredentialStruct & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogProgress(Zcl, "[AddCredentialToUser] Unable to get the user from app [endpointId=%d,userIndex=%d]",
                        mPath.mEndpointId, userIndex);
        return DlStatus::kFailure;
    }

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: CredentialIndex in CredentialStruct provided SHALL be for an available credential slot.
        // appclusters, 5.6.3.2: This is the index of the specific credential used to authorize the lock operation in the list of
        //                       credentials identified by CredentialType
        if (user.credentials.data()[i].credentialIndex == credential.credentialIndex &&
            user.credentials.data()[i].credentialType == credential.credentialType)
        {
            ChipLogProgress(
                Zcl,
                "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
                "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                mPath.mEndpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex);
            return DlStatus::kInvalidField;
        }
    }

    // appclusters: spec defines up to NumberOfCredentialsSupportedPerUser credentials per user
    size_t maxCredentialsPerUser = mNumberOfCredentialsSupportedPerUser;
    if (user.credentials.size() + 1 > maxCredentialsPerUser)
    {
        ChipLogProgress(Zcl,
                        "[AddCredentialToUser] Unable to add credentials to user: too many credentials "
                        "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                        mPath.mEndpointId, userIndex, static_cast<unsigned>(user.credentials.size()));
        return DlStatus::kResourceExhausted;
    }

    chip::Platform::ScopedMemoryBuffer<CredentialStruct> newCredentials;
    if (!newCredentials.Alloc(user.credentials.size() + 1))
    {
        ChipLogError(Zcl,
                     "[AddCredentialToUser] Unable to allocate the buffer for credentials "
                     "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                     mPath.mEndpointId, userIndex, static_cast<unsigned>(user.credentials.size()));
        return DlStatus::kFailure;
    }

    memcpy(newCredentials.Get(), user.credentials.data(), sizeof(CredentialStruct) * user.credentials.size());
    newCredentials[user.credentials.size()] = credential;

    if (!mDelegate.SetUser(userIndex, user.createdBy, modifierFabricIdx, user.userName, user.userUniqueId, user.userStatus,
                           user.userType, user.credentialRule, newCredentials.Get(), user.credentials.size() + 1))
    {
        ChipLogProgress(Zcl,
                        "[AddCredentialToUser] Unable to add credential to user: app error "
                        "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                        mPath.mEndpointId, userIndex, static_cast<unsigned>(user.credentials.size()));
        return DlStatus::kFailure;
    }

    ChipLogProgress(Zcl,
                    "[AddCredentialToUser] Credential added to user "
                    "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                    mPath.mEndpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                    static_cast<unsigned>(user.credentials.size() + 1));

    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ModifyCredentialForUser(FabricIndex modifierFabricIdx, uint16_t userIndex,
                                                  const CredentialStruct & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!mDelegate.GetUser(userIndex, user))
    {
        ChipLogProgress(Zcl, "[ModifyUserCredential] Unable to get the user from app [endpointId=%d,userIndex=%d]",
                        mPath.mEndpointId, userIndex);
        return DlStatus::kFailure;
    }

    // return INVALID_COMMAND if the accessing fabric index doesn't match the CreatorFabricIndex in the user record pointed to by
    // UserIndex
    if (user.createdBy != modifierFabricIdx)
    {
        ChipLogProgress(Zcl,
                        "[createCredential] Unable to modify credential for user created by different fabric "
                        "[endpointId=%d,userIndex=%d,creatorIdx=%d,fabricIdx=%d]",
                        mPath.mEndpointId, userIndex, user.createdBy, modifierFabricIdx);
        return DlStatus::kInvalidField;
    }

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: user should already be associated with given credential
        if (user.credentials[i].credentialType == credential.credentialType &&
            user.credentials[i].credentialIndex == credential.credentialIndex)
        {
            chip::Platform::ScopedMemoryBuffer<CredentialStruct> newCredentials;
            if (!newCredentials.Alloc(user.credentials.size()))
            {
                ChipLogError(Zcl,
                             "[ModifyUserCredential] Unable to allocate the buffer for credentials "
                             "[endpointId=%d,userIndex=%d,userTotalCredentials=%u,credentialType=%d,credentialIndex=%d]",
                             mPath.mEndpointId, userIndex, static_cast<unsigned>(user.credentials.size()),
                             to_underlying(credential.credentialType), credential.credentialIndex);
                return DlStatus::kFailure;
            }
            memcpy(newCredentials.Get(), user.credentials.data(), sizeof(CredentialStruct) * user.credentials.size());
            newCredentials[i] = credential;

            if (!mDelegate.SetUser(userIndex, user.createdBy, modifierFabricIdx, user.userName, user.userUniqueId, user.userStatus,
                                   user.userType, user.credentialRule, newCredentials.Get(), user.credentials.size()))
            {
                ChipLogProgress(Zcl,
                                "[ModifyUserCredential] Unable to modify user credential: app error "
                                "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                                mPath.mEndpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                                static_cast<unsigned>(user.credentials.size()));
                return DlStatus::kFailure;
            }

            ChipLogProgress(Zcl,
                            "[ModifyUserCredential] User credential modified "
                            "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                            mPath.mEndpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                            static_cast<unsigned>(user.credentials.size()));

            return DlStatus::kSuccess;
        }
    }

    // appclusters, 5.2.4.40: if user is not associated with the given credential we should return INVALID_COMMAND
    ChipLogProgress(Zcl,
                    "[ModifyUserCredential] Unable to modify user credential: user is not associated with credential index "
                    "[endpointId=%d,userIndex=%d,credentialIndex=%d]",
                    mPath.mEndpointId, userIndex, credential.credentialIndex);

    return DlStatus::kInvalidField;
}

DlStatus DoorLockCluster::CreateCredential(FabricIndex creatorFabricIdx, NodeId sourceNodeId, uint16_t credentialIndex,
                                           CredentialTypeEnum credentialType,
                                           const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                           const ByteSpan & credentialData, const DataModel::Nullable<uint16_t> & userIndex,
                                           const DataModel::Nullable<UserStatusEnum> & userStatus,
                                           const DataModel::Nullable<UserTypeEnum> & userType, uint16_t & createdUserIndex)
{
    // appclusters, 5.2.4.41.1: should send the OCCUPIED in the response when the credential is in use
    if (DlCredentialStatus::kAvailable != existingCredential.status)
    {
        ChipLogProgress(
            Zcl, "[SetCredential] Unable to set the credential: credential slot is occupied [endpointId=%d,credentialIndex=%d]",
            mPath.mEndpointId, credentialIndex);

        return DlStatus::kOccupied;
    }

    if (!userType.IsNull() && UserTypeEnum::kProgrammingUser == userType.Value())
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: user type is invalid "
                        "[endpointId=%d,credentialIndex=%d,userType=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(userType.Value()));

        return DlStatus::kInvalidField;
    }

    // For Aliro endpoint keys, there is a single shared count for the total
    // count of evictable and non-evictable keys that can be stored. This needs
    // to be enforced specially, because none of the other logic we have handles that.
    if (credentialType == CredentialTypeEnum::kAliroEvictableEndpointKey ||
        credentialType == CredentialTypeEnum::kAliroNonEvictableEndpointKey)
    {
        size_t maxEndpointKeys = mDelegate.GetNumberOfAliroEndpointKeysSupported();
        size_t evictableEndpointKeys, nonEvictableEndpointKeys;

        if (!CountOccupiedCredentials(CredentialTypeEnum::kAliroEvictableEndpointKey, evictableEndpointKeys))
        {
            ChipLogError(Zcl, "Unable to count Aliro evictable endpoint keys.");
            return DlStatus::kFailure;
        }

        if (!CountOccupiedCredentials(CredentialTypeEnum::kAliroNonEvictableEndpointKey, nonEvictableEndpointKeys))
        {
            ChipLogError(Zcl, "Unable to count Aliro non-evictable endpoint keys.");
            return DlStatus::kFailure;
        }

        if (evictableEndpointKeys + nonEvictableEndpointKeys >= maxEndpointKeys)
        {
            ChipLogError(Zcl, "Unable to create Aliro endpoint key credential; too many exist already [endpointId=%d]",
                         mPath.mEndpointId);
            return DlStatus::kResourceExhausted;
        }
    }

    CredentialStruct credential{ credentialType, credentialIndex };
    // appclusters, 5.2.4.40: if userIndex is not provided we should create new user
    DlStatus status = DlStatus::kSuccess;
    if (userIndex.IsNull())
    {
        ChipLogProgress(Zcl, "[SetCredential] UserIndex is not set, creating new user [endpointId=%d,credentialIndex=%d]",
                        mPath.mEndpointId, credentialIndex);

        status = CreateNewCredentialAndUser(creatorFabricIdx, sourceNodeId, userStatus, userType, credential, credentialData,
                                            createdUserIndex);
    }
    else
    {
        // appclusters, 5.2.4.40: if user index is NULL, we should try to modify the existing user. In this case userStatus and
        // userType shall both be null.
        if (!userStatus.IsNull() || !userType.IsNull())
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to add credential: invalid arguments "
                            "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                            mPath.mEndpointId, credentialIndex, to_underlying(credentialType));
            return DlStatus::kInvalidField;
        }
        status = CreateNewCredentialAndAddItToUser(creatorFabricIdx, userIndex.Value(), credential, credentialData);
    }

    if (DlStatus::kSuccess == status)
    {
        SendRemoteLockUserChange(CredentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kAdd, sourceNodeId,
                                 creatorFabricIdx, createdUserIndex == 0 ? userIndex.Value() : createdUserIndex, credentialIndex);
    }

    return status;
}

DlStatus DoorLockCluster::ModifyProgrammingPIN(FabricIndex modifierFabricIndex, NodeId sourceNodeId, uint16_t credentialIndex,
                                               CredentialTypeEnum credentialType,
                                               const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                               const ByteSpan & credentialData)
{
    if (CredentialTypeEnum::kProgrammingPIN != credentialType || 0 != credentialIndex)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to modify programming PIN: invalid argument [endpointId=%d,credentialIndex=%d]",
                        mPath.mEndpointId, credentialIndex);

        return DlStatus::kInvalidField;
    }

    ChipLogProgress(Zcl, "[SetCredential] Modifying the programming PIN [endpointId=%d,credentialIndex=%d]", mPath.mEndpointId,
                    credentialIndex);

    uint16_t relatedUserIndex = 0;
    if (!FindUserIndexByCredential(CredentialTypeEnum::kProgrammingPIN, 0, relatedUserIndex))
    {
        ChipLogError(Zcl, "[SetCredential] Unable to modify PIN - related user not found (internal error) [endpointId=%d]",
                     mPath.mEndpointId);
        return DlStatus::kFailure;
    }

    if (!mDelegate.SetCredential(credentialIndex, existingCredential.createdBy, modifierFabricIndex, existingCredential.status,
                                 existingCredential.credentialType, credentialData))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to modify the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(credentialType),
                        static_cast<unsigned>(credentialData.size()));
        return DlStatus::kFailure;
    }

    ChipLogProgress(Zcl,
                    "[SetCredential] Successfully modified the credential "
                    "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                    mPath.mEndpointId, credentialIndex, to_underlying(credentialType),
                    static_cast<unsigned>(credentialData.size()));

    SendRemoteLockUserChange(CredentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kModify, sourceNodeId,
                             modifierFabricIndex, relatedUserIndex, credentialIndex);

    return DlStatus::kSuccess;
}

DlStatus DoorLockCluster::ModifyCredential(FabricIndex modifierFabricIndex, NodeId sourceNodeId, uint16_t credentialIndex,
                                           CredentialTypeEnum credentialType,
                                           const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                           const ByteSpan & credentialData, uint16_t userIndex,
                                           const DataModel::Nullable<UserStatusEnum> & userStatus,
                                           const DataModel::Nullable<UserTypeEnum> & userType)
{
    // appclusters, 5.2.4.40: when modifying a credential, userStatus and userType shall both be NULL.
    if (!userStatus.IsNull() || !userType.IsNull())
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to modify the credential: invalid arguments "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(credentialType));
        return DlStatus::kInvalidField;
    }

    CredentialStruct credential{ credentialType, credentialIndex };
    auto status = ModifyCredentialForUser(modifierFabricIndex, userIndex, credential);

    if (DlStatus::kSuccess == status)
    {
        if (!mDelegate.SetCredential(credentialIndex, existingCredential.createdBy, modifierFabricIndex, existingCredential.status,
                                     existingCredential.credentialType, credentialData))
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to modify the credential: app error "
                            "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                            mPath.mEndpointId, credentialIndex, to_underlying(credentialType),
                            static_cast<unsigned>(credentialData.size()));

            return DlStatus::kFailure;
        }

        ChipLogProgress(Zcl,
                        "[SetCredential] Successfully modified the credential "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(credentialType),
                        static_cast<unsigned>(credentialData.size()));

        SendRemoteLockUserChange(CredentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kModify, sourceNodeId,
                                 modifierFabricIndex, userIndex, credentialIndex);
    }
    return status;
}

Status DoorLockCluster::ClearCredential(FabricIndex modifier, NodeId sourceNodeId, CredentialTypeEnum credentialType,
                                        uint16_t credentialIndex, bool sendUserChangeEvent)
{
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Cannot clear programming PIN credentials "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::InvalidCommand;
    }

    uint16_t maxCredentialsForClear = 0;
    if (!CredentialIndexValid(credentialType, credentialIndex, maxCredentialsForClear))
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Cannot clear credential - index out of bounds "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::InvalidCommand;
    }

    // 1. Fetch the credential from storage, so we know what we're deleting
    EmberAfPluginDoorLockCredentialInfo credential;
    if (!mDelegate.GetCredential(credentialIndex, credentialType, credential))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't read credential from database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Failure;
    }

    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Ignored attempt to clear unoccupied credential slot "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Success;
    }

    if (credentialType != credential.credentialType)
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Ignored attempt to clear credential of different type "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,actualCredentialType=%u]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier,
                        to_underlying(credential.credentialType));
        return Status::Success;
    }

    // 2. Get the associated user and if it is the only attached credential -- delete the user. This operation will
    // also remove the associated credential.
    uint16_t relatedUserIndex = 0;
    if (!FindUserIndexByCredential(credentialType, credentialIndex, relatedUserIndex))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear related credential user - couldn't find index of related user "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Failure;
    }
    EmberAfPluginDoorLockUserInfo relatedUser;
    if (!mDelegate.GetUser(relatedUserIndex, relatedUser))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - couldn't get user from database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                     mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);

        return Status::Failure;
    }
    if (1 == relatedUser.credentials.size())
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Clearing related user - no credentials left "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);
        auto clearStatus = ClearUser(modifier, sourceNodeId, relatedUserIndex, relatedUser, true);
        if (Status::Success != clearStatus)
        {
            ChipLogError(Zcl,
                         "[clearCredential] Unable to clear related credential user - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                         mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);

            return Status::Failure;
        }
        ChipLogProgress(Zcl,
                        "[clearCredential] Successfully clear credential and related user "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);
        return Status::Success;
    }

    // 3. If the user wasn't deleted, delete the credential and adjust the list of credentials for related user in the storage
    if (!mDelegate.SetCredential(credentialIndex, kUndefinedFabricIndex, kUndefinedFabricIndex, DlCredentialStatus::kAvailable,
                                 credentialType, ByteSpan()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't write new credential to database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Failure;
    }

    chip::Platform::ScopedMemoryBuffer<CredentialStruct> newCredentials;
    if (!newCredentials.Alloc(relatedUser.credentials.size()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to allocate the buffer for credentials "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,credentialsCount=%u]",
                     mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned>(relatedUser.credentials.size()));
        return Status::Failure;
    }

    size_t newCredentialsCount = 0;
    for (const auto & c : relatedUser.credentials)
    {
        if (c.credentialType == credentialType && c.credentialIndex == credentialIndex)
        {
            continue;
        }
        newCredentials[newCredentialsCount++] = c;
    }

    if (!mDelegate.SetUser(relatedUserIndex, relatedUser.createdBy, modifier, relatedUser.userName, relatedUser.userUniqueId,
                           relatedUser.userStatus, relatedUser.userType, relatedUser.credentialRule, newCredentials.Get(),
                           newCredentialsCount))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - unable to update database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%u]",
                     mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned>(newCredentialsCount));
        return Status::Failure;
    }

    ChipLogProgress(Zcl,
                    "[clearCredential] Successfully clear credential and related user "
                    "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%u]",
                    mPath.mEndpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                    static_cast<unsigned>(newCredentialsCount));

    if (sendUserChangeEvent)
    {
        SendRemoteLockUserChange(CredentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kClear, sourceNodeId,
                                 modifier, relatedUserIndex, credentialIndex);
    }

    return Status::Success;
}

Status DoorLockCluster::ClearCredentials(FabricIndex modifier, NodeId sourceNodeId)
{
    if (SupportsPIN())
    {
        auto status = ClearCredentials(modifier, sourceNodeId, CredentialTypeEnum::kPin);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all PIN credentials [endpointId=%d,status=%d]", mPath.mEndpointId,
                         to_underlying(status));
            return status;
        }

        ChipLogProgress(Zcl, "[clearCredentials] All PIN credentials were cleared [endpointId=%d]", mPath.mEndpointId);
    }

    if (SupportsRFID())
    {
        auto status = ClearCredentials(modifier, sourceNodeId, CredentialTypeEnum::kRfid);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all RFID credentials [endpointId=%d,status=%d]",
                         mPath.mEndpointId, to_underlying(status));
            return status;
        }
        ChipLogProgress(Zcl, "[clearCredentials] All RFID credentials were cleared [endpointId=%d]", mPath.mEndpointId);
    }

    if (SupportsFingers())
    {
        auto status = ClearCredentials(modifier, sourceNodeId, CredentialTypeEnum::kFingerprint);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all Fingerprint credentials [endpointId=%d,status=%d]",
                         mPath.mEndpointId, to_underlying(status));
            return status;
        }

        status = ClearCredentials(modifier, sourceNodeId, CredentialTypeEnum::kFingerVein);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all Finger Vein credentials [endpointId=%d,status=%d]",
                         mPath.mEndpointId, to_underlying(status));
            return status;
        }

        ChipLogProgress(Zcl, "[clearCredentials] All Finger credentials were cleared [endpointId=%d]", mPath.mEndpointId);
    }

    if (SupportsFace())
    {
        auto status = ClearCredentials(modifier, sourceNodeId, CredentialTypeEnum::kFace);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all face credentials [endpointId=%d,status=%d]",
                         mPath.mEndpointId, to_underlying(status));
            return status;
        }
        ChipLogProgress(Zcl, "[clearCredentials] All face credentials were cleared [endpointId=%d]", mPath.mEndpointId);
    }

    if (SupportsAliroProvisioning())
    {
        for (auto & credentialType :
             { CredentialTypeEnum::kAliroEvictableEndpointKey, CredentialTypeEnum::kAliroCredentialIssuerKey,
               CredentialTypeEnum::kAliroNonEvictableEndpointKey })
        {
            auto status = ClearCredentials(modifier, sourceNodeId, credentialType);
            if (Status::Success != status)
            {
                ChipLogError(Zcl,
                             "[clearCredentials] Unable to clear all Aliro credentials [endpointId=%d,credentialType=%d,status=%d]",
                             mPath.mEndpointId, to_underlying(credentialType), to_underlying(status));
                return status;
            }
        }
        ChipLogProgress(Zcl, "[clearCredentials] All Aliro credentials were cleared [endpointId=%d]", mPath.mEndpointId);
    }

    return Status::Success;
}

Status DoorLockCluster::ClearCredentials(FabricIndex modifier, NodeId sourceNodeId, CredentialTypeEnum credentialType)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!GetMaxNumberOfCredentials(credentialType, maxNumberOfCredentials))
    {
        ChipLogError(Zcl,
                     "[clearCredentials] Unable to get max number of credentials to clear - can't get max number of credentials "
                     "[endpointId=%d,credentialType=%u]",
                     mPath.mEndpointId, to_underlying(credentialType));
        return Status::Failure;
    }

    Status status          = Status::Success;
    bool clearedCredential = false;
    for (uint16_t i = 1; i <= maxNumberOfCredentials; ++i)
    {
        Status clearStatus = ClearCredential(modifier, sourceNodeId, credentialType, i, false);
        if (Status::Success != clearStatus)
        {
            ChipLogError(Zcl,
                         "[clearCredentials] Unable to clear the credential - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,status=%d]",
                         mPath.mEndpointId, to_underlying(credentialType), i, to_underlying(status));
            if (status == Status::Success)
            {
                status = clearStatus;
            }
        }
        else
        {
            clearedCredential = true;
        }
    }

    // Generate the event if we cleared any credentials, even if we then had errors
    // clearing other ones, so we don't have credentials silently disappearing.
    if (clearedCredential)
    {
        SendRemoteLockUserChange(CredentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kClear, sourceNodeId,
                                 modifier, DOOR_LOCK_ALL_USERS_INDEX, DOOR_LOCK_ALL_USERS_INDEX);
    }

    return status;
}

bool DoorLockCluster::ClearFabricFromCredentials(CredentialTypeEnum credentialType, FabricIndex fabricToRemove) const
{
    uint16_t maxNumberOfCredentials = 0;
    if (!GetMaxNumberOfCredentials(credentialType, maxNumberOfCredentials))
    {
        ChipLogError(
            Zcl,
            "[clearFabricFromCredentials] Unable to get max number of credentials to clear - can't get max number of credentials "
            "[endpointId=%d,credentialType=%u]",
            mPath.mEndpointId, to_underlying(credentialType));
        return false;
    }

    uint16_t startIndex = 1;
    // Programming PIN is a special case -- it is unique and its index assumed to be 0.
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        startIndex = 0;
        maxNumberOfCredentials--;
    }

    for (uint16_t credentialIndex = startIndex; credentialIndex <= maxNumberOfCredentials; ++credentialIndex)
    {
        EmberAfPluginDoorLockCredentialInfo credential;
        if (!mDelegate.GetCredential(credentialIndex, credentialType, credential))
        {
            ChipLogError(
                Zcl,
                "[clearFabricFromCredentials] Unable to clear fabric from credential - couldn't read credential from database "
                "[endpointId=%d,credentialType=%u,credentialIndex=%d,fabricIdToRemove=%d]",
                mPath.mEndpointId, to_underlying(credentialType), credentialIndex, fabricToRemove);

            // Go on and try to clear all the remaining credentials
            continue;
        }

        if (DlCredentialStatus::kAvailable == credential.status ||
            (credential.createdBy != fabricToRemove && credential.lastModifiedBy != fabricToRemove))
        {
            continue;
        }

        if (credential.createdBy == fabricToRemove)
        {
            credential.createdBy = kUndefinedFabricIndex;
        }

        if (credential.lastModifiedBy == fabricToRemove)
        {
            credential.lastModifiedBy = kUndefinedFabricIndex;
        }

        if (!mDelegate.SetCredential(credentialIndex, credential.createdBy, credential.lastModifiedBy, credential.status,
                                     credential.credentialType, credential.credentialData))
        {
            ChipLogError(Zcl,
                         "[clearFabricFromCredentials] Unable to clear fabric from credential - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,fabricIdToRemove=%d]",
                         mPath.mEndpointId, to_underlying(credentialType), credentialIndex, fabricToRemove);
            return false;
        }
    }

    return true;
}

bool DoorLockCluster::ClearFabricFromCredentials(FabricIndex fabricToRemove) const
{
    if (SupportsRFID())
    {
        ClearFabricFromCredentials(CredentialTypeEnum::kRfid, fabricToRemove);
    }

    if (SupportsPIN())
    {
        ClearFabricFromCredentials(CredentialTypeEnum::kPin, fabricToRemove);
    }

    if (SupportsFingers())
    {
        ClearFabricFromCredentials(CredentialTypeEnum::kFingerprint, fabricToRemove);
        ClearFabricFromCredentials(CredentialTypeEnum::kFingerVein, fabricToRemove);
    }

    if (SupportsFace())
    {
        ClearFabricFromCredentials(CredentialTypeEnum::kFace, fabricToRemove);
    }

    ClearFabricFromCredentials(CredentialTypeEnum::kProgrammingPIN, fabricToRemove);

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
// Response encoding helpers
// =============================================================================

void DoorLockCluster::SendSetCredentialResponse(CommandHandler * handler, const ConcreteCommandPath & path, DlStatus status,
                                                uint16_t userIndex, uint16_t nextCredentialIndex) const
{
    Commands::SetCredentialResponse::Type command{};
    command.status = status;
    if (0 != userIndex)
    {
        command.userIndex.SetNonNull(userIndex);
    }
    if (0 != nextCredentialIndex)
    {
        command.nextCredentialIndex.SetNonNull(nextCredentialIndex);
    }
    handler->AddResponse(path, command);
}

void DoorLockCluster::SendGetCredentialResponse(CommandHandler * handler, const ConcreteCommandPath & path,
                                                CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                                const DataModel::Nullable<uint16_t> & nextCredentialIndex,
                                                uint16_t userIndexWithCredential,
                                                const EmberAfPluginDoorLockCredentialInfo * credentialInfo,
                                                bool credentialExists) const
{
    // Important: nothing in this function may call mDelegate.GetCredential,
    // because that might stomp on the data pointed to by credentialInfo
    // (application-provided static buffers).
    Commands::GetCredentialStatusResponse::Type response{ .credentialExists = credentialExists };
    if (credentialExists && !(nullptr == credentialInfo))
    {
        if (0 != userIndexWithCredential)
        {
            response.userIndex.SetNonNull(userIndexWithCredential);
        }
        if (credentialInfo->creationSource == DlAssetSource::kMatterIM)
        {
            response.creatorFabricIndex.SetNonNull(credentialInfo->createdBy);
        }
        if (credentialInfo->modificationSource == DlAssetSource::kMatterIM)
        {
            response.lastModifiedFabricIndex.SetNonNull(credentialInfo->lastModifiedBy);
        }
        if (IsAliroCredentialType(credentialType))
        {
            response.credentialData.Emplace(credentialInfo->credentialData);
        }
    }
    else
    {
        response.userIndex.SetNull();
        if (IsAliroCredentialType(credentialType))
        {
            response.credentialData.Emplace(NullNullable);
        }
    }
    response.nextCredentialIndex = nextCredentialIndex;
    handler->AddResponse(path, response);
}

void DoorLockCluster::SendGetWeekDayScheduleResponse(CommandHandler * handler, const ConcreteCommandPath & path,
                                                     uint8_t weekDayIndex, uint16_t userIndex, DlStatus status,
                                                     DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                                     uint8_t endMinute) const
{
    Commands::GetWeekDayScheduleResponse::Type response;
    response.weekDayIndex = weekDayIndex;
    response.userIndex    = userIndex;
    response.status       = status;
    if (DlStatus::kSuccess == status)
    {
        response.daysMask    = Optional<chip::BitMask<DaysMaskMap>>(daysMask);
        response.startHour   = Optional<uint8_t>(startHour);
        response.startMinute = Optional<uint8_t>(startMinute);
        response.endHour     = Optional<uint8_t>(endHour);
        response.endMinute   = Optional<uint8_t>(endMinute);
    }

    handler->AddResponse(path, response);
}

void DoorLockCluster::SendGetYearDayScheduleResponse(CommandHandler * handler, const ConcreteCommandPath & path,
                                                     uint8_t yearDayIndex, uint16_t userIndex, DlStatus status,
                                                     uint32_t localStartTime, uint32_t localEndTime) const
{
    Commands::GetYearDayScheduleResponse::Type response;
    response.yearDayIndex = yearDayIndex;
    response.userIndex    = userIndex;
    response.status       = status;
    if (DlStatus::kSuccess == status)
    {
        response.localStartTime = chip::Optional<uint32_t>(localStartTime);
        response.localEndTime   = chip::Optional<uint32_t>(localEndTime);
    }

    handler->AddResponse(path, response);
}

void DoorLockCluster::SendGetHolidayScheduleResponse(CommandHandler * handler, const ConcreteCommandPath & path,
                                                     uint8_t holidayIndex, DlStatus status, uint32_t localStartTime,
                                                     uint32_t localEndTime, OperatingModeEnum operatingMode) const
{
    auto response = Commands::GetHolidayScheduleResponse::Type{ holidayIndex, status };
    if (DlStatus::kSuccess == status)
    {
        response.localStartTime = chip::Optional<uint32_t>(localStartTime);
        response.localEndTime   = chip::Optional<uint32_t>(localEndTime);
        response.operatingMode  = chip::Optional<OperatingModeEnum>(operatingMode);
    }
    handler->AddResponse(path, response);
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

    // appclusters, 5.2.4.34: UserName has maximum DOOR_LOCK_MAX_USER_NAME_SIZE (10) characters excluding NUL terminator in it.
    if (!userName.IsNull() && userName.Value().size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
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
    // EmberAfPluginDoorLockUserInfo might point into some application-static
    // buffers (for its credentials and whatnot).
    uint16_t nextAvailableUserIndex = 0;
    if (FindOccupiedUserSlot(static_cast<uint16_t>(userIndex + 1), nextAvailableUserIndex))
    {
        response.nextUserIndex.SetNonNull(nextAvailableUserIndex);
    }

    EmberAfPluginDoorLockUserInfo user;
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

// =============================================================================
// Credential management command handlers
// =============================================================================

void DoorLockCluster::HandleSetCredential(const DataModel::InvokeRequest & request,
                                          const Commands::SetCredential::DecodableType & commandData, CommandHandler * handler,
                                          const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[SetCredential] Incoming command [endpointId=%d]", mPath.mEndpointId);

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetCredential] Unable to get the fabric IDX [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetCredential] Unable to get the source node index [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto & operationType   = commandData.operationType;
    auto & credentialType  = commandData.credential.credentialType;
    auto & credentialIndex = commandData.credential.credentialIndex;
    auto & credentialData  = commandData.credentialData;
    auto & userIndex       = commandData.userIndex;
    auto & userStatus      = commandData.userStatus;
    auto & userType        = commandData.userType;

    if (!CredentialTypeSupported(credentialType))
    {
        ChipLogProgress(Zcl, "[SetCredential] Credential type is not supported [endpointId=%d,credentialType=%u]",
                        mPath.mEndpointId, to_underlying(credentialType));
        handler->AddStatus(path, Status::UnsupportedCommand);
        return;
    }

    // appclusters, 5.2.4.41: response should contain next available credential slot
    uint16_t nextAvailableCredentialSlot = 0;
    FindUnoccupiedCredentialSlot(credentialType, static_cast<uint16_t>(credentialIndex + 1), nextAvailableCredentialSlot);

    uint16_t maxNumberOfCredentials = 0;
    if (!CredentialIndexValid(credentialType, credentialIndex, maxNumberOfCredentials))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Credential index is out of range [endpointId=%d,credentialType=%u"
                        ",credentialIndex=%d]",
                        mPath.mEndpointId, to_underlying(credentialType), credentialIndex);
        SendSetCredentialResponse(handler, path, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    // appclusters, 5.2.4.40.3: If the credential data length is out of bounds we should return INVALID_COMMAND
    auto status = CredentialLengthWithinRange(credentialType, credentialData);
    if (DlStatus::kSuccess != status)
    {
        SendSetCredentialResponse(handler, path, status, 0, nextAvailableCredentialSlot);
        return;
    }

    // appclusters, SetCredentialResponse Status field (DoorLock.adoc#10211-status-field): return INVALID_COMMAND if
    // CredentialData violates the uncompressed EC public key requirement in 5.2.6.9.
    status = AliroCredentialDataValid(credentialType, credentialData);
    if (DlStatus::kSuccess != status)
    {
        SendSetCredentialResponse(handler, path, status, 0, nextAvailableCredentialSlot);
        return;
    }

    // appclusters, 5.2.4.41.1: we should return DUPLICATE in the response if we're trying to create duplicated credential entry
    for (uint16_t i = 1; CredentialTypeEnum::kProgrammingPIN != credentialType && (i <= maxNumberOfCredentials); ++i)
    {
        // Ignore the slot we are trying to set, because setting a credential to the same value as it already has should be just
        // fine. This is not clearly defined in the spec; spec issue tracker tracks that.
        if (i == credentialIndex)
        {
            continue;
        }

        EmberAfPluginDoorLockCredentialInfo currentCredential;
        if (!mDelegate.GetCredential(i, credentialType, currentCredential))
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to get the credential to exclude duplicated entry "
                            "[endpointId=%d,credentialType=%u,credentialIndex=%d]",
                            mPath.mEndpointId, to_underlying(credentialType), i);
            SendSetCredentialResponse(handler, path, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
            return;
        }
        if (DlCredentialStatus::kAvailable != currentCredential.status && currentCredential.credentialType == credentialType &&
            CredentialDataEqualConstantTime(currentCredential.credentialData, credentialData))
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Credential with the same data and type already exist "
                            "[endpointId=%d,credentialType=%u,dataLength=%u,existingCredentialIndex=%d,credentialIndex=%d]",
                            mPath.mEndpointId, to_underlying(credentialType), static_cast<unsigned>(credentialData.size()), i,
                            credentialIndex);
            SendSetCredentialResponse(handler, path, DlStatus::kDuplicate, 0, nextAvailableCredentialSlot);
            return;
        }
    }

    EmberAfPluginDoorLockCredentialInfo existingCredential;
    if (!mDelegate.GetCredential(credentialIndex, credentialType, existingCredential))
    {
        ChipLogProgress(Zcl, "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d]",
                        mPath.mEndpointId, credentialIndex);

        SendSetCredentialResponse(handler, path, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
        return;
    }

    if (!IsValidUserStatusForSet(userStatus))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: user status is out of range "
                        "[endpointId=%d,credentialIndex=%d,userStatus=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(userStatus.Value()));
        SendSetCredentialResponse(handler, path, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    if (userType == UserTypeEnum::kUnknownEnumValue)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: user type is unknown "
                        "[endpointId=%d,credentialIndex=%d,userType=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(userType.Value()));
        SendSetCredentialResponse(handler, path, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    switch (operationType)
    {
    case DataOperationTypeEnum::kAdd: {
        uint16_t createdUserIndex = 0;

        status = CreateCredential(fabricIdx, sourceNodeId, credentialIndex, credentialType, existingCredential, credentialData,
                                  userIndex, userStatus, userType, createdUserIndex);

        SendSetCredentialResponse(handler, path, status, createdUserIndex, nextAvailableCredentialSlot);
        return;
    }
    case DataOperationTypeEnum::kModify: {
        // appclusters, 5.2.4.41.1: should send the INVALID_COMMAND in the response when the credential is in use
        if (DlCredentialStatus::kAvailable == existingCredential.status)
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to modify the credential: credential slot is not occupied "
                            "[endpointId=%d,credentialIndex=%d]",
                            mPath.mEndpointId, credentialIndex);

            SendSetCredentialResponse(handler, path, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
            return;
        }

        // return INVALID_COMMAND if the accessing fabric index doesn't match the CreatorFabricIndex of the credential being
        // modified
        if (existingCredential.createdBy != fabricIdx)
        {
            ChipLogProgress(Zcl,
                            "[createCredential] Unable to modify credential. Fabric index differs from creator fabric "
                            "[endpointId=%d,credentialIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                            mPath.mEndpointId, credentialIndex, existingCredential.createdBy, fabricIdx);

            SendSetCredentialResponse(handler, path, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
            return;
        }

        // if userIndex is NULL then we're changing the programming user PIN
        if (userIndex.IsNull())
        {
            if (!userStatus.IsNull() || userType != UserTypeEnum::kProgrammingUser)
            {
                ChipLogProgress(Zcl,
                                "[SetCredential] Unable to modify programming PIN: invalid argument "
                                "[endpointId=%d,credentialIndex=%d]",
                                mPath.mEndpointId, credentialIndex);
            }
            status =
                ModifyProgrammingPIN(fabricIdx, sourceNodeId, credentialIndex, credentialType, existingCredential, credentialData);
            SendSetCredentialResponse(handler, path, status, 0, nextAvailableCredentialSlot);
            return;
        }

        status = ModifyCredential(fabricIdx, sourceNodeId, credentialIndex, credentialType, existingCredential, credentialData,
                                  userIndex.Value(), userStatus, userType);
        SendSetCredentialResponse(handler, path, status, 0, nextAvailableCredentialSlot);
        return;
    }
    case DataOperationTypeEnum::kClear:
    default:
        // appclusters, 5.2.4.40: set credential command supports only Add and Modify operational type.
        SendSetCredentialResponse(handler, path, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
    }
}

void DoorLockCluster::HandleGetCredentialStatus(CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                                CommandHandler * handler, const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[GetCredentialStatus] Incoming command [endpointId=%d]", mPath.mEndpointId);

    if (!CredentialTypeSupported(credentialType))
    {
        ChipLogProgress(Zcl, "[GetCredentialStatus] Credential type is not supported [endpointId=%d,credentialType=%u]",
                        mPath.mEndpointId, to_underlying(credentialType));
        handler->AddStatus(path, Status::UnsupportedCommand);
        return;
    }

    // Our response will need to include the index of the next occupied credential slot after credentialIndex, if there is one.
    //
    // We want to figure this out before we call mDelegate.GetCredential, because the
    // EmberAfPluginDoorLockCredentialInfo we get might be pointing into some application-static buffers (for its credential
    // data and whatnot).
    DataModel::Nullable<uint16_t> nextCredentialIndex;
    {
        uint16_t foundNextCredentialIndex;
        if (FindOccupiedCredentialSlot(credentialType, static_cast<uint16_t>(credentialIndex + 1), foundNextCredentialIndex))
        {
            nextCredentialIndex.SetNonNull(foundNextCredentialIndex);
        }
    }

    uint16_t maxNumberOfCredentials = 0;
    if (!CredentialIndexValid(credentialType, credentialIndex, maxNumberOfCredentials))
    {
        SendGetCredentialResponse(handler, path, credentialType, credentialIndex, nextCredentialIndex, 0, nullptr, false);
        return;
    }

    EmberAfPluginDoorLockCredentialInfo credentialInfo;
    if (!mDelegate.GetCredential(credentialIndex, credentialType, credentialInfo))
    {
        ChipLogProgress(Zcl,
                        "[GetCredentialStatus] Unable to get the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                        mPath.mEndpointId, credentialIndex, to_underlying(credentialType));
        handler->AddStatus(path, Status::Failure);
        return;
    }

    bool credentialExists            = DlCredentialStatus::kAvailable != credentialInfo.status;
    uint16_t userIndexWithCredential = 0;
    if (credentialExists)
    {
        if (!FindUserIndexByCredential(credentialType, credentialIndex, userIndexWithCredential))
        {
            // That means that there's some kind of error in our database -- there is an unassociated credential.
            ChipLogError(Zcl,
                         "[GetCredentialStatus] Database possibly corrupted - credential exists without user assigned "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d]",
                         mPath.mEndpointId, to_underlying(credentialType), credentialIndex);
            handler->AddStatus(path, Status::Failure);
            return;
        }
    }

    SendGetCredentialResponse(handler, path, credentialType, credentialIndex, nextCredentialIndex, userIndexWithCredential,
                              &credentialInfo, credentialExists);
}

void DoorLockCluster::HandleClearCredential(const DataModel::InvokeRequest & request,
                                            const Commands::ClearCredential::DecodableType & commandData, CommandHandler * handler,
                                            const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[ClearCredential] Incoming command [endpointId=%d]", mPath.mEndpointId);

    auto modifier = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == modifier)
    {
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        handler->AddStatus(path, Status::Failure);
        return;
    }

    const auto & credential = commandData.credential;
    if (credential.IsNull())
    {
        ChipLogProgress(Zcl, "[ClearCredential] Clearing all credentials [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, ClearCredentials(modifier, sourceNodeId));
        return;
    }

    // Remove all the credentials of the particular type.
    auto credentialType = credential.Value().credentialType;

    if (!CredentialTypeSupported(credentialType))
    {
        ChipLogProgress(Zcl, "[ClearCredential] Credential type is not supported [endpointId=%d,credentialType=%u]",
                        mPath.mEndpointId, to_underlying(credentialType));
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    auto credentialIndex = credential.Value().credentialIndex;
    if (DOOR_LOCK_ALL_USERS_INDEX == credentialIndex)
    {
        handler->AddStatus(path, ClearCredentials(modifier, sourceNodeId, credentialType));
        return;
    }

    handler->AddStatus(path,
                       ClearCredential(modifier, sourceNodeId, credentialType, credentialIndex,
                                       /* sendUserChangeEvent = */ true));
}

// =============================================================================
// Schedule command handlers
// =============================================================================

void DoorLockCluster::HandleSetWeekDaySchedule(const DataModel::InvokeRequest & request, uint8_t weekDayIndex, uint16_t userIndex,
                                               const BitMask<DaysMaskMap> & daysMask, uint8_t startHour, uint8_t startMinute,
                                               uint8_t endHour, uint8_t endMinute, CommandHandler * handler,
                                               const ConcreteCommandPath & path)
{
    if (!SupportsWeekDaySchedules())
    {
        ChipLogProgress(Zcl, "[SetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[SetWeekDaySchedule] Incoming command [endpointId=%d]", mPath.mEndpointId);

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetWeekDaySchedule] Unable to get the fabric IDX [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetWeekDaySchedule] Unable to get the source node index [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    if (!WeekDayIndexValid(weekDayIndex) || !UserIndexValid(userIndex))
    {
        ChipLogProgress(
            Zcl, "[SetWeekDaySchedule] Unable to add schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            mPath.mEndpointId, weekDayIndex, userIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (!UserExists(userIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - user does not exist "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, weekDayIndex, userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    uint8_t rawDaysMask = daysMask.Raw();

    // Check that bits are within range
    if ((0 == rawDaysMask) || (rawDaysMask & 0x80))
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - daysMask is out of range "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%x]",
                        mPath.mEndpointId, weekDayIndex, userIndex, daysMask.Raw());
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (startHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || startMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE ||
        endHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || endMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE)
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - start time out of range "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                        mPath.mEndpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (startHour > endHour || (startHour == endHour && startMinute >= endMinute))
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - invalid time "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                        mPath.mEndpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    auto status = mDelegate.SetSchedule(weekDayIndex, userIndex, DlScheduleStatus::kOccupied, daysMask, startHour, startMinute,
                                        endHour, endMinute);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[SetWeekDaySchedule] Unable to add schedule - internal error "
                     "[endpointId=%d,weekDayIndex=%d,userIndex=%d,status=%u]",
                     mPath.mEndpointId, weekDayIndex, userIndex, to_underlying(status));
        handler->AddStatus(path, Status::Failure);
        return;
    }

    ChipLogProgress(Zcl,
                    "[SetWeekDaySchedule] Successfully created new schedule "
                    "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                    mPath.mEndpointId, weekDayIndex, userIndex, daysMask.Raw(), startHour, startMinute, endHour, endMinute);

    SendRemoteLockUserChange(LockDataTypeEnum::kWeekDaySchedule, DataOperationTypeEnum::kAdd, sourceNodeId, fabricIdx, userIndex,
                             static_cast<uint16_t>(weekDayIndex));

    handler->AddStatus(path, Status::Success);
}

void DoorLockCluster::HandleGetWeekDaySchedule(uint8_t weekDayIndex, uint16_t userIndex, CommandHandler * handler,
                                               const ConcreteCommandPath & path)
{
    if (!SupportsWeekDaySchedules())
    {
        ChipLogProgress(Zcl, "[GetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[GetWeekDaySchedule] Incoming command [endpointId=%d]", mPath.mEndpointId);

    if (!WeekDayIndexValid(weekDayIndex) || !UserIndexValid(userIndex))
    {
        ChipLogProgress(
            Zcl, "[GetWeekDaySchedule] Unable to get schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            mPath.mEndpointId, weekDayIndex, userIndex);
        SendGetWeekDayScheduleResponse(handler, path, weekDayIndex, userIndex, DlStatus::kInvalidField, DaysMaskMap(0), 0, 0, 0, 0);
        return;
    }

    if (!UserExists(userIndex))
    {
        ChipLogProgress(Zcl, "[GetWeekDaySchedule] User does not exist [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, weekDayIndex, userIndex);
        SendGetWeekDayScheduleResponse(handler, path, weekDayIndex, userIndex, DlStatus::kNotFound, DaysMaskMap(0), 0, 0, 0, 0);
        return;
    }

    EmberAfPluginDoorLockWeekDaySchedule scheduleInfo{ DaysMaskMap(0) };
    auto status = mDelegate.GetSchedule(weekDayIndex, userIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        SendGetWeekDayScheduleResponse(handler, path, weekDayIndex, userIndex, status, DaysMaskMap(0), 0, 0, 0, 0);
        return;
    }

    SendGetWeekDayScheduleResponse(handler, path, weekDayIndex, userIndex, DlStatus::kSuccess, scheduleInfo.daysMask,
                                   scheduleInfo.startHour, scheduleInfo.startMinute, scheduleInfo.endHour, scheduleInfo.endMinute);
}

void DoorLockCluster::HandleClearWeekDaySchedule(const DataModel::InvokeRequest & request, uint8_t weekDayIndex, uint16_t userIndex,
                                                 CommandHandler * handler, const ConcreteCommandPath & path)
{
    if (!SupportsWeekDaySchedules())
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Incoming command [endpointId=%d]", mPath.mEndpointId);

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearWeekDaySchedule] Unable to get the fabric IDX [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearWeekDaySchedule] Unable to get the source node index [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    if (!UserIndexValid(userIndex) || (!WeekDayIndexValid(weekDayIndex) && DOOR_LOCK_ALL_SCHEDULES_INDEX != weekDayIndex))
    {
        ChipLogProgress(Zcl,
                        "[ClearWeekDaySchedule] User or WeekDay index is out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, weekDayIndex, userIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (!UserExists(userIndex))
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] User does not exist [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, weekDayIndex, userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (DOOR_LOCK_ALL_SCHEDULES_INDEX == weekDayIndex)
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Clearing all schedules for a single user [endpointId=%d,userIndex=%d]",
                        mPath.mEndpointId, userIndex);
        clearStatus = ClearWeekDaySchedules(userIndex);
    }
    else
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Clearing a single schedule [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, weekDayIndex, userIndex);
        clearStatus = ClearWeekDaySchedule(userIndex, weekDayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        ChipLogProgress(
            Zcl, "[ClearWeekDaySchedule] Unable to clear the user schedules - app error [endpointId=%d,userIndex=%d,status=%u]",
            mPath.mEndpointId, userIndex, to_underlying(clearStatus));
        handler->AddStatus(path, Status::Failure);
        return;
    }

    SendRemoteLockUserChange(LockDataTypeEnum::kWeekDaySchedule, DataOperationTypeEnum::kClear, sourceNodeId, fabricIdx, userIndex,
                             static_cast<uint16_t>(weekDayIndex));

    handler->AddStatus(path, Status::Success);
}

void DoorLockCluster::HandleSetYearDaySchedule(const DataModel::InvokeRequest & request, uint8_t yearDayIndex, uint16_t userIndex,
                                               uint32_t localStartTime, uint32_t localEndTime, CommandHandler * handler,
                                               const ConcreteCommandPath & path)
{
    if (!SupportsYearDaySchedules())
    {
        ChipLogProgress(Zcl, "[SetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[SetYearDaySchedule] incoming command [endpointId=%d]", mPath.mEndpointId);

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetYearDaySchedule] Unable to get the fabric IDX [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetYearDaySchedule] Unable to get the source node index [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    if (!YearDayIndexValid(yearDayIndex) || !UserIndexValid(userIndex))
    {
        ChipLogProgress(
            Zcl, "[SetYearDaySchedule] Unable to add schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            mPath.mEndpointId, yearDayIndex, userIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (!UserExists(userIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetYearDaySchedule] Unable to add schedule - user does not exist "
                        "[endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, yearDayIndex, userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    if (localEndTime <= localStartTime)
    {
        ChipLogProgress(Zcl,
                        "[SetYearDaySchedule] Unable to add schedule - schedule ends earlier than starts"
                        "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStartTime=%" PRIu32 ",localEndTime=%" PRIu32 "]",
                        mPath.mEndpointId, yearDayIndex, userIndex, localStartTime, localEndTime);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    auto status = mDelegate.SetSchedule(yearDayIndex, userIndex, DlScheduleStatus::kOccupied, localStartTime, localEndTime);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[SetYearDaySchedule] Unable to add schedule - internal error "
                     "[endpointId=%d,yearDayIndex=%d,userIndex=%d,status=%u]",
                     mPath.mEndpointId, yearDayIndex, userIndex, to_underlying(status));
        handler->AddStatus(path, Status::Failure);
        return;
    }

    ChipLogProgress(Zcl,
                    "[SetYearDaySchedule] Successfully created new schedule "
                    "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStartTime=%" PRIu32 ",endTime=%" PRIu32 "]",
                    mPath.mEndpointId, yearDayIndex, userIndex, localStartTime, localEndTime);

    SendRemoteLockUserChange(LockDataTypeEnum::kYearDaySchedule, DataOperationTypeEnum::kAdd, sourceNodeId, fabricIdx, userIndex,
                             static_cast<uint16_t>(yearDayIndex));

    handler->AddStatus(path, Status::Success);
}

void DoorLockCluster::HandleGetYearDaySchedule(uint8_t yearDayIndex, uint16_t userIndex, CommandHandler * handler,
                                               const ConcreteCommandPath & path)
{
    if (!SupportsYearDaySchedules())
    {
        ChipLogProgress(Zcl, "[GetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[GetYearDaySchedule] incoming command [endpointId=%d]", mPath.mEndpointId);

    if (!YearDayIndexValid(yearDayIndex) || !UserIndexValid(userIndex))
    {
        ChipLogProgress(
            Zcl, "[GetYearDaySchedule] Unable to get schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            mPath.mEndpointId, yearDayIndex, userIndex);
        SendGetYearDayScheduleResponse(handler, path, yearDayIndex, userIndex, DlStatus::kInvalidField, 0, 0);
        return;
    }

    if (!UserExists(userIndex))
    {
        ChipLogProgress(Zcl, "[GetYearDaySchedule] User does not exist [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, yearDayIndex, userIndex);
        SendGetYearDayScheduleResponse(handler, path, yearDayIndex, userIndex, DlStatus::kNotFound, 0, 0);
        return;
    }

    EmberAfPluginDoorLockYearDaySchedule scheduleInfo{};
    auto status = mDelegate.GetSchedule(yearDayIndex, userIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        SendGetYearDayScheduleResponse(handler, path, yearDayIndex, userIndex, status, 0, 0);
        return;
    }

    SendGetYearDayScheduleResponse(handler, path, yearDayIndex, userIndex, DlStatus::kSuccess, scheduleInfo.localStartTime,
                                   scheduleInfo.localEndTime);
}

void DoorLockCluster::HandleClearYearDaySchedule(const DataModel::InvokeRequest & request, uint8_t yearDayIndex, uint16_t userIndex,
                                                 CommandHandler * handler, const ConcreteCommandPath & path)
{
    if (!SupportsYearDaySchedules())
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[ClearYearDaySchedule] incoming command [endpointId=%d]", mPath.mEndpointId);

    auto fabricIdx = CommandFabricIndex(request);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearYearDaySchedule] Unable to get the fabric IDX [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    auto sourceNodeId = CommandNodeId(request);
    if (kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearYearDaySchedule] Unable to get the source node index [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    if (!UserIndexValid(userIndex) || (!YearDayIndexValid(yearDayIndex) && DOOR_LOCK_ALL_SCHEDULES_INDEX != yearDayIndex))
    {
        ChipLogProgress(Zcl,
                        "[ClearYearDaySchedule] User or YearDay index is out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, yearDayIndex, userIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (!UserExists(userIndex))
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] User does not exist [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, yearDayIndex, userIndex);
        handler->AddStatus(path, Status::Failure);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (DOOR_LOCK_ALL_SCHEDULES_INDEX == yearDayIndex)
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] Clearing all schedules for a single user [endpointId=%d,userIndex=%d]",
                        mPath.mEndpointId, userIndex);
        clearStatus = ClearYearDaySchedules(userIndex);
    }
    else
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] Clearing a single schedule [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        mPath.mEndpointId, yearDayIndex, userIndex);
        clearStatus = ClearYearDaySchedule(userIndex, yearDayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        ChipLogProgress(
            Zcl, "[ClearYearDaySchedule] Unable to clear the user schedules - app error [endpointId=%d,userIndex=%d,status=%u]",
            mPath.mEndpointId, userIndex, to_underlying(clearStatus));
        handler->AddStatus(path, Status::Failure);
        return;
    }

    SendRemoteLockUserChange(LockDataTypeEnum::kYearDaySchedule, DataOperationTypeEnum::kClear, sourceNodeId, fabricIdx, userIndex,
                             static_cast<uint16_t>(yearDayIndex));

    handler->AddStatus(path, Status::Success);
}

void DoorLockCluster::HandleSetHolidaySchedule(uint8_t holidayIndex, uint32_t localStartTime, uint32_t localEndTime,
                                               OperatingModeEnum operatingMode, CommandHandler * handler,
                                               const ConcreteCommandPath & path)
{
    if (!SupportsHolidaySchedules())
    {
        ChipLogProgress(Zcl, "[SetHolidaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[SetHolidaySchedule] incoming command [endpointId=%d]", mPath.mEndpointId);

    if (!HolidayIndexValid(holidayIndex))
    {
        ChipLogProgress(Zcl, "[SetHolidaySchedule] Unable to add schedule - index out of range [endpointId=%d,scheduleIndex=%d]",
                        mPath.mEndpointId, holidayIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (localEndTime <= localStartTime)
    {
        ChipLogProgress(Zcl,
                        "[SetHolidaySchedule] Unable to add schedule - schedule ends earlier than starts"
                        "[endpointId=%d,scheduleIndex=%d,localStartTime=%" PRIu32 ",localEndTime=%" PRIu32 "]",
                        mPath.mEndpointId, holidayIndex, localStartTime, localEndTime);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    if (operatingMode == OperatingModeEnum::kUnknownEnumValue)
    {
        ChipLogProgress(Zcl,
                        "[SetHolidaySchedule] Unable to add schedule - operating mode is unknown"
                        "[endpointId=%d,scheduleIndex=%d,localStartTime=%" PRIu32 ",localEndTime=%" PRIu32 ", operatingMode=%d]",
                        mPath.mEndpointId, holidayIndex, localStartTime, localEndTime, to_underlying(operatingMode));
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    auto status = mDelegate.SetSchedule(holidayIndex, DlScheduleStatus::kOccupied, localStartTime, localEndTime, operatingMode);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl, "[SetHolidaySchedule] Unable to add schedule - internal error [endpointId=%d,scheduleIndex=%d,status=%u]",
                     mPath.mEndpointId, holidayIndex, to_underlying(status));
        handler->AddStatus(path, Status::Failure);
        return;
    }

    ChipLogProgress(Zcl,
                    "[SetHolidaySchedule] Successfully created new schedule "
                    "[endpointId=%d,scheduleIndex=%d,localStartTime=%" PRIu32 ",endTime=%" PRIu32 ",operatingMode=%d]",
                    mPath.mEndpointId, holidayIndex, localStartTime, localEndTime, to_underlying(operatingMode));

    handler->AddStatus(path, Status::Success);
}

void DoorLockCluster::HandleGetHolidaySchedule(uint8_t holidayIndex, CommandHandler * handler, const ConcreteCommandPath & path)
{
    if (!SupportsHolidaySchedules())
    {
        ChipLogProgress(Zcl, "[GetHolidaySchedule] Ignore command (not supported) [endpointId=%d,scheduleIndex=%d]",
                        mPath.mEndpointId, holidayIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[GetHolidaySchedule] incoming command [endpointId=%d,scheduleIndex=%d]", mPath.mEndpointId, holidayIndex);

    if (!HolidayIndexValid(holidayIndex))
    {
        ChipLogProgress(Zcl, "[GetYearDaySchedule] Unable to get schedule - index out of range [endpointId=%d,scheduleIndex=%d]",
                        mPath.mEndpointId, holidayIndex);
        SendGetHolidayScheduleResponse(handler, path, holidayIndex, DlStatus::kInvalidField, 0, 0, OperatingModeEnum::kNormal);
        return;
    }

    EmberAfPluginDoorLockHolidaySchedule scheduleInfo{};
    auto status = mDelegate.GetSchedule(holidayIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        SendGetHolidayScheduleResponse(handler, path, holidayIndex, status, 0, 0, OperatingModeEnum::kNormal);
        return;
    }
    SendGetHolidayScheduleResponse(handler, path, holidayIndex, DlStatus::kSuccess, scheduleInfo.localStartTime,
                                   scheduleInfo.localEndTime, scheduleInfo.operatingMode);
}

void DoorLockCluster::HandleClearHolidaySchedule(uint8_t holidayIndex, CommandHandler * handler, const ConcreteCommandPath & path)
{
    if (!SupportsHolidaySchedules())
    {
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Ignore command (not supported) [endpointId=%d]", mPath.mEndpointId);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[ClearHolidaySchedule] incoming command [endpointId=%d,scheduleIndex=%d]", mPath.mEndpointId,
                    holidayIndex);

    if (!HolidayIndexValid(holidayIndex) && DOOR_LOCK_ALL_SCHEDULES_INDEX != holidayIndex)
    {
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Holiday index is out of range [endpointId=%d,scheduleIndex=%d]",
                        mPath.mEndpointId, holidayIndex);
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (DOOR_LOCK_ALL_SCHEDULES_INDEX == holidayIndex)
    {
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Clearing all holiday schedules [endpointId=%d,scheduleIndex=%d]",
                        mPath.mEndpointId, holidayIndex);
        clearStatus = ClearHolidaySchedules();
    }
    else
    {
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Clearing a single schedule [endpointId=%d,scheduleIndex=%d]",
                        mPath.mEndpointId, holidayIndex);
        clearStatus = ClearHolidaySchedule(holidayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        ChipLogProgress(
            Zcl, "[ClearHolidaySchedule] Unable to clear the user schedules - app error [endpointId=%d,scheduleIndex=%d,status=%u]",
            mPath.mEndpointId, holidayIndex, to_underlying(clearStatus));
        handler->AddStatus(path, Status::Failure);
        return;
    }
    handler->AddStatus(path, Status::Success);
}

// =============================================================================
// Aliro provisionning command handlers
// =============================================================================

void DoorLockCluster::HandleSetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                                 const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey,
                                                 CommandHandler * handler, const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[SetAliroReaderConfig] Incoming command [endpointId=%d]", mPath.mEndpointId);

    // The GroupResolvingKey must be provided if and only if the Aliro BLE UWB
    // feature is supported. Otherwise, return INVALID_COMMAND
    const bool supportsAliroBLEUWB = SupportsAliroBLEUWB();
    if (supportsAliroBLEUWB != groupResolvingKey.HasValue())
    {
        if (supportsAliroBLEUWB)
        {
            ChipLogError(Zcl, "[SetAliroReaderConfig] Aliro BLE UWB supported but Group Resolving Key is not provided");
        }
        else
        {
            ChipLogError(Zcl, "[SetAliroReaderConfig] Aliro BLE UWB not supported but Group Resolving Key is provided");
        }
        handler->AddStatus(path, Status::InvalidCommand);
        return;
    }

    // Check if the size of the signingKey, verificationKey, groupIdentifier, groupResolvingKey parameters conform to the spec.
    // Return INVALID_COMMAND if not.
    if (signingKey.size() != kAliroSigningKeySize || verificationKey.size() != kAliroReaderVerificationKeySize ||
        groupIdentifier.size() != kAliroReaderGroupIdentifierSize ||
        (groupResolvingKey.HasValue() && groupResolvingKey.Value().size() != kAliroGroupResolvingKeySize))
    {
        ChipLogProgress(Zcl, "[SetAliroReaderConfig] One or more parameters in the command do not meet the size constraint");
        handler->AddStatus(path, Status::ConstraintError);
        return;
    }

    uint8_t buffer[kAliroReaderVerificationKeySize];
    MutableByteSpan readerVerificationKey(buffer);

    CHIP_ERROR err = mDelegate.GetAliroReaderVerificationKey(readerVerificationKey);

    // If Aliro reader verification key attribute was not read successfuly, return INVALID_IN_STATE. Or if the verification key
    // was read and is not null (i.e. not empty), we can't set a new reader config without clearing the previous one, return
    // INVALID_IN_STATE.
    if (err != CHIP_NO_ERROR || !readerVerificationKey.empty())
    {
        ChipLogError(Zcl, "[SetAliroReaderConfig] Aliro reader verification key was not read or is not null.");
        handler->AddStatus(path, Status::InvalidInState);
        return;
    }

    err = mDelegate.SetAliroReaderConfig(signingKey, verificationKey, groupIdentifier, groupResolvingKey);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "[SetAliroReaderConfig] Unable to set aliro reader config [endpointId=%d]", mPath.mEndpointId);
    }
    else
    {
        // Various attributes changed; mark them dirty.
        NotifyAttributeChanged(AliroReaderVerificationKey::Id);
        NotifyAttributeChanged(AliroReaderGroupIdentifier::Id);
        NotifyAttributeChanged(AliroGroupResolvingKey::Id);
    }
    handler->AddStatus(path, StatusIB(err).mStatus);
}

void DoorLockCluster::HandleClearAliroReaderConfig(CommandHandler * handler, const ConcreteCommandPath & path)
{
    ChipLogProgress(Zcl, "[ClearAliroReaderConfig] Incoming command [endpointId=%d]", mPath.mEndpointId);

    uint8_t buffer[kAliroReaderVerificationKeySize];
    MutableByteSpan readerVerificationKey(buffer);
    CHIP_ERROR err = mDelegate.GetAliroReaderVerificationKey(readerVerificationKey);
    if (err != CHIP_NO_ERROR && readerVerificationKey.empty())
    {
        // No reader config to start with. Just return without marking any attributes as dirty.
        handler->AddStatus(path, Status::Success);
        return;
    }

    err = mDelegate.ClearAliroReaderConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "[ClearAliroReaderConfig] Unable to clear aliro reader config [endpointId=%d]", mPath.mEndpointId);
    }
    else
    {
        // Various attributes changed; mark them dirty.
        NotifyAttributeChanged(AliroReaderVerificationKey::Id);
        NotifyAttributeChanged(AliroReaderGroupIdentifier::Id);
        NotifyAttributeChanged(AliroGroupResolvingKey::Id);
    }
    handler->AddStatus(path, StatusIB(err).mStatus);
}

// =============================================================================
// Fabric removal
// =============================================================================

bool DoorLockCluster::HandleFabricRemoved(FabricIndex fabricIndex)
{
    ChipLogProgress(Zcl, "[OnFabricRemoved] Handling a fabric removal from the door lock cluster [endpointId=%d,fabricIndex=%d]",
                    mPath.mEndpointId, fabricIndex);

    bool status{ true };
    // Iterate over all the users and clean up the deleted fabric
    if (!ClearFabricFromUsers(fabricIndex))
    {
        ChipLogError(Zcl, "[OnFabricRemoved] Unable to cleanup fabric from users - internal error [endpointId=%d,fabricIndex=%d]",
                     mPath.mEndpointId, fabricIndex);
        status = false;
    }

    // Iterate over all the credentials and clean up the fabrics
    if (!ClearFabricFromCredentials(fabricIndex))
    {
        ChipLogError(Zcl,
                     "[OnFabricRemoved] Unable to cleanup fabric from credentials - internal error [endpointId=%d,fabricIndex=%d]",
                     mPath.mEndpointId, fabricIndex);
        status = false;
    }

    mDelegate.OnFabricRemoved(fabricIndex);

    return status;
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
