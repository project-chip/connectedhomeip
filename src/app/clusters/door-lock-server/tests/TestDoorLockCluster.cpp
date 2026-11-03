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

#include <app/clusters/door-lock-server/DoorLockCluster.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/DoorLock/Events.h>
#include <clusters/DoorLock/Metadata.h>
#include <lib/support/TimerDelegateMock.h>
#include <pw_unit_test/framework.h>

#include <lib/support/CHIPMem.h>

#include <cstring>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using chip::Testing::ClusterTester;

namespace {

constexpr EndpointId kTestEndpointId = 1;

/// Base for test delegates: neutral implementations of the Aliro virtuals
/// (the tests in this file do not exercise Aliro provisioning).
class TestDelegateBase : public Delegate
{
public:
    CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) override { return CHIP_ERROR_NOT_FOUND; }
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
    CHIP_ERROR ClearAliroReaderConfig() override { return CHIP_ERROR_NOT_FOUND; }
};

/// Delegate that accepts every hardware actuation request, so command tests
/// exercise the success paths.
class AcceptingDelegate : public TestDelegateBase
{
public:
    std::optional<OperationErrorEnum> HandleDoorLockCommand(const LockOperationRequest & request) override { return std::nullopt; }
    std::optional<OperationErrorEnum> HandleDoorUnlockCommand(const LockOperationRequest & request) override
    {
        return std::nullopt;
    }
    std::optional<OperationErrorEnum> HandleDoorUnboltCommand(const LockOperationRequest & request) override
    {
        return std::nullopt;
    }
};

/// Delegate with an unconfigured Aliro reader: the nullable key getters
/// succeed with an empty span (the cluster reports null for those).
class UnconfiguredAliroDelegate : public TestDelegateBase
{
public:
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) override
    {
        uint8_t subIdentifier[kAliroReaderGroupSubIdentifierSize] = { 0 };
        std::memcpy(groupSubIdentifier.data(), subIdentifier, sizeof(subIdentifier));
        groupSubIdentifier.reduce_size(sizeof(subIdentifier));
        return CHIP_NO_ERROR;
    }
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
    CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) override
    {
        groupResolvingKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey, const ByteSpan & groupIdentifier,
                                    const Optional<ByteSpan> & groupResolvingKey) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ClearAliroReaderConfig() override { return CHIP_NO_ERROR; }
};

/// Delegate that rejects every hardware actuation request (default behavior of
/// a delegate without a hardware integration).
class RejectingDelegate : public TestDelegateBase
{
public:
    int mLockAttempts = 0;
    std::optional<OperationErrorEnum> HandleDoorLockCommand(const LockOperationRequest & request) override
    {
        mLockAttempts++;
        return OperationErrorEnum::kUnspecified;
    }
};

Config BasicConfig(TimerDelegate & timerDelegate)
{
    Config config(timerDelegate);
    return config;
}

/// TimerDelegateMock recording CancelTimer/StartTimer calls, so tests can
/// verify the TimerDelegate contract (cancel before re-scheduling).
class TrackingTimerDelegate : public TimerDelegateMock
{
public:
    void CancelTimer(TimerContext * context) override
    {
        mCancelCount++;
        TimerDelegateMock::CancelTimer(context);
    }
    CriticalFailure StartTimer(TimerContext * context, chip::System::Clock::Timeout aTimeout) override
    {
        mStartCount++;
        return TimerDelegateMock::StartTimer(context, aTimeout);
    }

    int mCancelCount = 0;
    int mStartCount  = 0;
};

Config FeaturedConfig(TimerDelegate & timerDelegate)
{
    Config config(timerDelegate);
    config.features.Set(Feature::kPinCredential).Set(Feature::kUser).Set(Feature::kDoorPositionSensor).Set(Feature::kUnbolt);
    config.optionalAttributes.language                     = true;
    config.optionalAttributes.ledSettings                  = true;
    config.optionalAttributes.autoRelockTime               = true;
    config.optionalAttributes.soundVolume                  = true;
    config.optionalAttributes.defaultConfigurationRegister = true;
    config.optionalAttributes.doorOpenEvents               = true;
    config.optionalAttributes.doorClosedEvents             = true;
    config.optionalAttributes.openPeriod                   = true;
    config.optionalAttributes.expiringUserTimeout          = true;
    config.numberOfPINUsersSupported                       = 10;
    config.maxPINCodeLength                                = 8;
    config.minPINCodeLength                                = 4;
    config.wrongCodeEntryLimit                             = 5;
    config.userCodeTemporaryDisableTime                    = 10;
    return config;
}

/// PIN-only config without the USR feature, so SendPINOverTheAir exists
/// (it is only present when the User feature is NOT supported and PIN is,
/// and its optional attribute flag is set).
Config PinWithoutUserConfig(TimerDelegate & timerDelegate)
{
    Config config(timerDelegate);
    config.features.Set(Feature::kPinCredential);
    config.optionalAttributes.sendPINOverTheAir = true;
    config.numberOfPINUsersSupported            = 10;
    config.maxPINCodeLength                     = 8;
    config.minPINCodeLength                     = 4;
    config.wrongCodeEntryLimit                  = 5;
    config.userCodeTemporaryDisableTime         = 10;
    return config;
}

/// Aliro config: ALIRO provisioning with BLE UWB.
Config AliroConfig(TimerDelegate & timerDelegate)
{
    Config config(timerDelegate);
    config.features.Set(Feature::kAliroProvisioning).Set(Feature::kAliroBLEUWB);
    return config;
}

/// Fixture with a feature-less cluster (mandatory attributes + lock commands only).
class TestDoorLockClusterBasic : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestDoorLockClusterBasic() : mCluster(kTestEndpointId, mDelegate, BasicConfig(mTimerDelegate)) {}

    void SetUp() override { ASSERT_EQ(mCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }

    AcceptingDelegate mDelegate;
    TimerDelegateMock mTimerDelegate;
    DoorLockCluster mCluster;
    ClusterTester tester{ mCluster };
};

/// Fixture with PIN + USR + DPS + UBOLT features and some optional attributes enabled.
class TestDoorLockClusterFeatured : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestDoorLockClusterFeatured() : mCluster(kTestEndpointId, mDelegate, FeaturedConfig(mTimerDelegate)) {}

    void SetUp() override { ASSERT_EQ(mCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }

    AcceptingDelegate mDelegate;
    TimerDelegateMock mTimerDelegate;
    DoorLockCluster mCluster;
    ClusterTester tester{ mCluster };
};

TEST_F(TestDoorLockClusterBasic, StartupReadsMandatoryAttributes)
{
    // TC-DRLK 2.1: ClusterRevision is 10 for this cluster revision.
    uint16_t revision = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, 10);

    uint32_t featureMap = 0xFFFF;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    DataModel::Nullable<DlLockState> lockState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::LockState::Id, lockState), CHIP_NO_ERROR);
    EXPECT_TRUE(lockState.IsNull());

    DlLockType lockType = DlLockType::kDeadBolt;
    ASSERT_EQ(tester.ReadAttribute(Attributes::LockType::Id, lockType), CHIP_NO_ERROR);
    EXPECT_EQ(lockType, DlLockType::kDeadBolt);

    bool actuatorEnabled = false;
    ASSERT_EQ(tester.ReadAttribute(Attributes::ActuatorEnabled::Id, actuatorEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(actuatorEnabled);

    OperatingModeEnum mode = OperatingModeEnum::kNormal;
    ASSERT_EQ(tester.ReadAttribute(Attributes::OperatingMode::Id, mode), CHIP_NO_ERROR);
    EXPECT_EQ(mode, OperatingModeEnum::kNormal);
}

TEST_F(TestDoorLockClusterBasic, FeatureGatedAttributesAreAbsent)
{
    // DPS attributes must be absent without the DPS feature.
    DataModel::Nullable<DoorStateEnum> doorState;
    EXPECT_EQ(tester.ReadAttribute(Attributes::DoorState::Id, doorState),
              Protocols::InteractionModel::Status::UnsupportedAttribute);

    // USR attributes must be absent without the USR feature.
    uint16_t users = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::NumberOfTotalUsersSupported::Id, users),
              Protocols::InteractionModel::Status::UnsupportedAttribute);

    // Plain optional attributes must be absent when not configured.
    uint32_t autoRelock = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::AutoRelockTime::Id, autoRelock),
              Protocols::InteractionModel::Status::UnsupportedAttribute);
}

TEST_F(TestDoorLockClusterBasic, FeatureGatedCommandsAreAbsent)
{
    // UnboltDoor requires the UBOLT feature.
    Commands::UnboltDoor::Type unboltRequest;
    auto result = tester.Invoke(Commands::UnboltDoor::Id, unboltRequest);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedCommand);
    }
}

TEST_F(TestDoorLockClusterBasic, LockDoorWithoutHardwareSupportFails)
{
    // A delegate without a hardware integration rejects the actuation, so the
    // command reports Failure and a LockOperationError event is emitted.
    RejectingDelegate rejectingDelegate;
    TimerDelegateMock timerDelegate;
    DoorLockCluster cluster(kTestEndpointId, rejectingDelegate, BasicConfig(timerDelegate));
    ClusterTester rejectingTester(cluster);
    ASSERT_EQ(cluster.Startup(rejectingTester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::LockDoor::Type request;
    auto result = rejectingTester.Invoke(Commands::LockDoor::Id, request);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Failure);
    }
    EXPECT_EQ(rejectingDelegate.mLockAttempts, 1);

    auto event = rejectingTester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    if (event.has_value())
    {
        Events::LockOperationError::DecodableType errorData;
        EXPECT_EQ(event.value().GetEventData(errorData), CHIP_NO_ERROR);
        EXPECT_EQ(errorData.lockOperationType, LockOperationTypeEnum::kLock);
        EXPECT_EQ(errorData.operationSource, OperationSourceEnum::kRemote);
    }
}

TEST_F(TestDoorLockClusterFeatured, UnboltDoorAvailableWithFeature)
{
    Commands::UnboltDoor::Type unboltRequest;
    auto result = tester.Invoke(Commands::UnboltDoor::Id, unboltRequest);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
    }
}

TEST_F(TestDoorLockClusterFeatured, AutoRelockTimeIsWritableWhenEnabled)
{
    uint32_t autoRelock = 0;
    ASSERT_EQ(tester.WriteAttribute(Attributes::AutoRelockTime::Id, static_cast<uint32_t>(10)), CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(Attributes::AutoRelockTime::Id, autoRelock), CHIP_NO_ERROR);
    EXPECT_EQ(autoRelock, 10u);
}

TEST_F(TestDoorLockClusterFeatured, ExpiringUserTimeoutConstraints)
{
    // Spec: ExpiringUserTimeout range is 1-2880 seconds.
    EXPECT_EQ(tester.WriteAttribute(Attributes::ExpiringUserTimeout::Id, static_cast<uint16_t>(0)),
              Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::ExpiringUserTimeout::Id, static_cast<uint16_t>(2881)),
              Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::ExpiringUserTimeout::Id, static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(Attributes::ExpiringUserTimeout::Id, static_cast<uint16_t>(2880)), CHIP_NO_ERROR);
}

TEST_F(TestDoorLockClusterFeatured, OperatingModeMustBeSupported)
{
    // Spec: OperatingModesBitmap uses inverted polarity (a `0` bit marks a
    // supported mode); the 0xFFF6 default supports Normal and NoRemoteLockUnlock.
    uint16_t supportedModes = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::SupportedOperatingModes::Id, supportedModes), CHIP_NO_ERROR);
    EXPECT_EQ(supportedModes, 0xFFF6u);

    // Privacy mode is not supported -> rejected.
    EXPECT_EQ(tester.WriteAttribute(Attributes::OperatingMode::Id, OperatingModeEnum::kPrivacy),
              Protocols::InteractionModel::Status::ConstraintError);

    // Unknown enum values are rejected.
    EXPECT_EQ(tester.WriteAttribute(Attributes::OperatingMode::Id, OperatingModeEnum::kUnknownEnumValue),
              Protocols::InteractionModel::Status::ConstraintError);

    // kNoRemoteLockUnlock is supported -> accepted.
    EXPECT_EQ(tester.WriteAttribute(Attributes::OperatingMode::Id, OperatingModeEnum::kNoRemoteLockUnlock), CHIP_NO_ERROR);

    OperatingModeEnum mode = OperatingModeEnum::kNormal;
    ASSERT_EQ(tester.ReadAttribute(Attributes::OperatingMode::Id, mode), CHIP_NO_ERROR);
    EXPECT_EQ(mode, OperatingModeEnum::kNoRemoteLockUnlock);
}

TEST_F(TestDoorLockClusterFeatured, LanguageWriteIsSuppressedWhenUnchanged)
{
    ASSERT_EQ(tester.WriteAttribute(Attributes::Language::Id, CharSpan::fromCharString("en")), CHIP_NO_ERROR);
    const size_t dirtyAfterSet = tester.GetDirtyList().size();

    // Writing the current value again does not mark the attribute dirty.
    EXPECT_EQ(tester.WriteAttribute(Attributes::Language::Id, CharSpan::fromCharString("en")), CHIP_NO_ERROR);
    EXPECT_EQ(tester.GetDirtyList().size(), dirtyAfterSet);

    // A different value marks it dirty again.
    ASSERT_EQ(tester.WriteAttribute(Attributes::Language::Id, CharSpan::fromCharString("fr")), CHIP_NO_ERROR);
    EXPECT_EQ(tester.GetDirtyList().size(), dirtyAfterSet + 1);
}

TEST_F(TestDoorLockClusterFeatured, LEDSettingsAndSoundVolumeBounds)
{
    // Spec: LEDSettings is 0-2 and SoundVolume is 0-3.
    EXPECT_EQ(tester.WriteAttribute(Attributes::LEDSettings::Id, static_cast<uint8_t>(3)),
              Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::LEDSettings::Id, static_cast<uint8_t>(2)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(Attributes::SoundVolume::Id, static_cast<uint8_t>(4)),
              Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::SoundVolume::Id, static_cast<uint8_t>(3)), CHIP_NO_ERROR);
}

TEST_F(TestDoorLockClusterFeatured, LanguageLengthConstraint)
{
    // Spec: Language is a char string with max length 3.
    EXPECT_EQ(tester.WriteAttribute(Attributes::Language::Id, CharSpan::fromCharString("ende")),
              Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::Language::Id, CharSpan::fromCharString("en")), CHIP_NO_ERROR);
}

TEST_F(TestDoorLockClusterFeatured, LockoutEngagesAfterWrongCodeLimit)
{
    // Wrong-code entries count up to WrongCodeEntryLimit, then the lockout
    // window (UserCodeTemporaryDisableTime) engages and an alarm is emitted.
    for (int i = 0; i < 4; i++)
    {
        ASSERT_EQ(mCluster.HandleWrongCodeEntry(), CHIP_NO_ERROR);
        EXPECT_FALSE(mCluster.IsLockoutEngaged());
    }

    // Fifth wrong entry (limit is 5) engages the lockout.
    ASSERT_EQ(mCluster.HandleWrongCodeEntry(), CHIP_NO_ERROR);
    EXPECT_TRUE(mCluster.IsLockoutEngaged());

    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    if (event.has_value())
    {
        Events::DoorLockAlarm::DecodableType alarmData;
        EXPECT_EQ(event.value().GetEventData(alarmData), CHIP_NO_ERROR);
        EXPECT_EQ(alarmData.alarmCode, AlarmCodeEnum::kWrongCodeEntryLimit);
    }
}

TEST_F(TestDoorLockClusterFeatured, RemoteOperationsIgnoredDuringLockout)
{
    // Engage the lockout.
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(mCluster.HandleWrongCodeEntry(), CHIP_NO_ERROR);
    }
    EXPECT_TRUE(mCluster.IsLockoutEngaged());

    // Consume the DoorLockAlarm event emitted by the lockout engagement.
    ASSERT_TRUE(tester.GetNextGeneratedEvent().has_value());

    // While locked out, remote lock/unlock attempts are ignored (no event).
    Commands::LockDoor::Type request;
    auto result = tester.Invoke(Commands::LockDoor::Id, request);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Failure);
    }
    EXPECT_FALSE(tester.GetNextGeneratedEvent().has_value());
}

TEST_F(TestDoorLockClusterFeatured, AutoRelockAfterUnlock)
{
    // Configure AutoRelockTime = 10 seconds.
    ASSERT_EQ(tester.WriteAttribute(Attributes::AutoRelockTime::Id, static_cast<uint32_t>(10)), CHIP_NO_ERROR);

    // Unlock: the lock state changes and the auto-relock timer is scheduled.
    DataModel::Nullable<uint16_t> noUser;
    ASSERT_EQ(mCluster.SetLockState(DlLockState::kUnlocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                    DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);

    // Advance past the timeout: the lock relocks itself with an auto source.
    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(11));
    DataModel::Nullable<DlLockState> lockState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::LockState::Id, lockState), CHIP_NO_ERROR);
    ASSERT_FALSE(lockState.IsNull());
    EXPECT_EQ(lockState.Value(), DlLockState::kLocked);
}

TEST_F(TestDoorLockClusterFeatured, NoAutoRelockWhenDisabled)
{
    // AutoRelockTime is 0 by default: unlocking does not schedule a relock.
    DataModel::Nullable<uint16_t> noUser;
    ASSERT_EQ(mCluster.SetLockState(DlLockState::kUnlocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                    DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(1000));
    DataModel::Nullable<DlLockState> lockState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::LockState::Id, lockState), CHIP_NO_ERROR);
    ASSERT_FALSE(lockState.IsNull());
    EXPECT_EQ(lockState.Value(), DlLockState::kUnlocked);
}

TEST_F(TestDoorLockClusterFeatured, RescheduledAutoRelockCancelsPendingTimer)
{
    // TimerDelegate::StartTimer requires cancelling a pending timer for the
    // same context first: a re-schedule must replace the previous auto-relock.
    TrackingTimerDelegate timerDelegate;
    DoorLockCluster cluster(kTestEndpointId, mDelegate, FeaturedConfig(timerDelegate));
    ClusterTester relockTester(cluster);
    ASSERT_EQ(cluster.Startup(relockTester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(relockTester.WriteAttribute(Attributes::AutoRelockTime::Id, static_cast<uint32_t>(10)), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> noUser;
    ASSERT_EQ(cluster.SetLockState(DlLockState::kUnlocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                   DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);
    // Snapshot after the first schedule (which itself cancelled the previous
    // relock timer, if any, under the cancel-before-start contract).
    const int cancelAfterFirstSchedule = timerDelegate.mCancelCount;

    // A second unlock re-schedules: the pending timer is cancelled first.
    ASSERT_EQ(cluster.SetLockState(DlLockState::kUnlocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                   DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);
    EXPECT_EQ(timerDelegate.mCancelCount, cancelAfterFirstSchedule + 1);

    // The re-scheduled relock still fires once.
    timerDelegate.AdvanceClock(System::Clock::Seconds32(10));
    DataModel::Nullable<DlLockState> lockState;
    ASSERT_EQ(relockTester.ReadAttribute(Attributes::LockState::Id, lockState), CHIP_NO_ERROR);
    ASSERT_FALSE(lockState.IsNull());
    EXPECT_EQ(lockState.Value(), DlLockState::kLocked);
}

TEST_F(TestDoorLockClusterFeatured, AutoRelockAfterUnlockCommand)
{
    // A successful UnlockDoor command starts the auto-relock countdown using
    // AutoRelockTime, even when the lock operation reports Unlatch (UBOLT).
    ASSERT_EQ(tester.WriteAttribute(Attributes::AutoRelockTime::Id, static_cast<uint32_t>(10)), CHIP_NO_ERROR);

    Commands::UnlockDoor::Type request;
    auto result = tester.Invoke(Commands::UnlockDoor::Id, request);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
    }

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(11));
    DataModel::Nullable<DlLockState> lockState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::LockState::Id, lockState), CHIP_NO_ERROR);
    ASSERT_FALSE(lockState.IsNull());
    EXPECT_EQ(lockState.Value(), DlLockState::kLocked);
}

TEST_F(TestDoorLockClusterFeatured, LockCancelsPendingAutoRelockTimer)
{
    // A lock operation while an auto-relock timer is pending makes the timer
    // obsolete: the stale timer must not relock a door that is later unlocked
    // again with AutoRelockTime == 0.
    ASSERT_EQ(tester.WriteAttribute(Attributes::AutoRelockTime::Id, static_cast<uint32_t>(10)), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> noUser;
    ASSERT_EQ(mCluster.SetLockState(DlLockState::kUnlocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                    DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);
    ASSERT_EQ(mCluster.SetLockState(DlLockState::kLocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                    DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);

    // Unlock again with AutoRelockTime == 0: nothing re-schedules the relock.
    ASSERT_EQ(tester.WriteAttribute(Attributes::AutoRelockTime::Id, static_cast<uint32_t>(0)), CHIP_NO_ERROR);
    ASSERT_EQ(mCluster.SetLockState(DlLockState::kUnlocked, OperationSourceEnum::kManual, noUser, Span<const CredentialStruct>(),
                                    DataModel::Nullable<FabricIndex>(), DataModel::Nullable<NodeId>()),
              CHIP_NO_ERROR);

    // The stale timer from the first unlock must not fire.
    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(11));
    DataModel::Nullable<DlLockState> lockState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::LockState::Id, lockState), CHIP_NO_ERROR);
    ASSERT_FALSE(lockState.IsNull());
    EXPECT_EQ(lockState.Value(), DlLockState::kUnlocked);
}

/// Fixture with the Aliro provisioning + BLE UWB features.
class TestDoorLockClusterAliro : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestDoorLockClusterAliro() : mCluster(kTestEndpointId, mDelegate, AliroConfig(mTimerDelegate)) {}

    void SetUp() override { ASSERT_EQ(mCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }

    UnconfiguredAliroDelegate mDelegate;
    TimerDelegateMock mTimerDelegate;
    DoorLockCluster mCluster;
    ClusterTester tester{ mCluster };
};

TEST_F(TestDoorLockClusterFeatured, SendPINOverTheAirAbsentWithUserFeature)
{
    // Spec conformance: SendPINOverTheAir exists only when USR is NOT
    // supported (with USR the credentials belong to user records).
    bool sendPin = false;
    EXPECT_EQ(tester.ReadAttribute(Attributes::SendPINOverTheAir::Id, sendPin),
              Protocols::InteractionModel::Status::UnsupportedAttribute);
}

TEST_F(TestDoorLockClusterFeatured, SendPINOverTheAirPresentWithoutUserFeature)
{
    // With PIN but without USR, SendPINOverTheAir is present and writable.
    TrackingTimerDelegate timerDelegate;
    DoorLockCluster cluster(kTestEndpointId, mDelegate, PinWithoutUserConfig(timerDelegate));
    ClusterTester pinTester(cluster);
    ASSERT_EQ(cluster.Startup(pinTester.GetServerClusterContext()), CHIP_NO_ERROR);

    bool sendPin = true;
    ASSERT_EQ(pinTester.ReadAttribute(Attributes::SendPINOverTheAir::Id, sendPin), CHIP_NO_ERROR);
    EXPECT_FALSE(sendPin);

    ASSERT_EQ(pinTester.WriteAttribute(Attributes::SendPINOverTheAir::Id, true), CHIP_NO_ERROR);
    ASSERT_EQ(pinTester.ReadAttribute(Attributes::SendPINOverTheAir::Id, sendPin), CHIP_NO_ERROR);
    EXPECT_TRUE(sendPin);
}

TEST_F(TestDoorLockClusterFeatured, DoorPositionCountersAreWritable)
{
    // Spec 9.2.3.5-9.2.3.7: DoorOpenEvents, DoorClosedEvents and OpenPeriod
    // are RW VM attributes.
    ASSERT_EQ(tester.WriteAttribute(Attributes::DoorOpenEvents::Id, static_cast<uint32_t>(10)), CHIP_NO_ERROR);
    ASSERT_EQ(tester.WriteAttribute(Attributes::DoorClosedEvents::Id, static_cast<uint32_t>(11)), CHIP_NO_ERROR);
    ASSERT_EQ(tester.WriteAttribute(Attributes::OpenPeriod::Id, static_cast<uint16_t>(2)), CHIP_NO_ERROR);

    uint32_t openEvents = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::DoorOpenEvents::Id, openEvents), CHIP_NO_ERROR);
    EXPECT_EQ(openEvents, 10u);
    uint32_t closedEvents = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::DoorClosedEvents::Id, closedEvents), CHIP_NO_ERROR);
    EXPECT_EQ(closedEvents, 11u);
    uint16_t openPeriod = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::OpenPeriod::Id, openPeriod), CHIP_NO_ERROR);
    EXPECT_EQ(openPeriod, 2u);
}

TEST_F(TestDoorLockClusterFeatured, DefaultConfigurationRegisterIsReadOnly)
{
    // Spec 9.2.9.28: DefaultConfigurationRegister is read-only (R V): the
    // attribute is readable, but any write to it fails.
    BitMask<DlDefaultConfigurationRegister> value;
    ASSERT_EQ(tester.ReadAttribute(Attributes::DefaultConfigurationRegister::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(Attributes::DefaultConfigurationRegister::Id, value), CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
}

TEST_F(TestDoorLockClusterAliro, UnconfiguredAliroKeysReadAsNull)
{
    // Spec 9.2.9.x: the Aliro reader attributes SHALL be null when no reader
    // configuration is provisioned.
    DataModel::Nullable<ByteSpan> verificationKey;
    ASSERT_EQ(tester.ReadAttribute(Attributes::AliroReaderVerificationKey::Id, verificationKey), CHIP_NO_ERROR);
    EXPECT_TRUE(verificationKey.IsNull());

    DataModel::Nullable<ByteSpan> groupIdentifier;
    ASSERT_EQ(tester.ReadAttribute(Attributes::AliroReaderGroupIdentifier::Id, groupIdentifier), CHIP_NO_ERROR);
    EXPECT_TRUE(groupIdentifier.IsNull());

    DataModel::Nullable<ByteSpan> groupResolvingKey;
    ASSERT_EQ(tester.ReadAttribute(Attributes::AliroGroupResolvingKey::Id, groupResolvingKey), CHIP_NO_ERROR);
    EXPECT_TRUE(groupResolvingKey.IsNull());
}

TEST_F(TestDoorLockClusterAliro, ReaderGroupSubIdentifierIsNotNullable)
{
    // The group subidentifier is mandatory and NOT nullable: reading it with
    // a plain (non-nullable) type must succeed.
    ByteSpan groupSubIdentifier;
    ASSERT_EQ(tester.ReadAttribute(Attributes::AliroReaderGroupSubIdentifier::Id, groupSubIdentifier), CHIP_NO_ERROR);
}

} // namespace
