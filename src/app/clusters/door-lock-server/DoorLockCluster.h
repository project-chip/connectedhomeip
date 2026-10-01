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

#pragma once

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/door-lock-server/door-lock-delegate.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/DoorLock/Attributes.h>
#include <clusters/DoorLock/Enums.h>
#include <clusters/DoorLock/Events.h>
#include <clusters/DoorLock/Metadata.h>
#include <clusters/DoorLock/Structs.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>

#include <array>
#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

namespace Internal {
// Spec: Language attribute is constrained to a maximum of 3 characters.
inline constexpr size_t kMaxLanguageLength = 3;
} // namespace Internal

using CredentialStruct = Structs::CredentialStruct::Type;

/// Plain (non-feature) optional attributes of the cluster.
///
/// A struct of flags is used instead of `OptionalAttributeSet` because Door Lock
/// attribute ids exceed the 32-bit mask range supported by `OptionalAttributeSet`.
struct OptionalAttributes
{
    bool language                     = false;
    bool ledSettings                  = false;
    bool autoRelockTime               = false;
    bool soundVolume                  = false;
    bool defaultConfigurationRegister = false;
    bool enableLocalProgramming       = false;
    bool enableOneTouchLocking        = false;
    bool enableInsideStatusLED        = false;
    bool enablePrivacyModeButton      = false;
    bool localProgrammingFeatures     = false;
};

/// Values that configure a single DoorLock cluster instance.
///
/// This is a passive configuration bundle: all fields have spec-compliant
/// defaults and are fixed after construction. Fields describing "F" (fixed)
/// attributes describe the physical lock and must match the actual hardware.
struct Config
{
    explicit Config(TimerDelegate & timerDelegateIn) : timerDelegate(timerDelegateIn) {}

    // ---- Feature configuration ----------------------------------------------
    BitFlags<Feature> features;
    OptionalAttributes optionalAttributes;

    // ---- Fixed attributes ----------------------------------------------------
    DlLockType lockType = DlLockType::kDeadBolt;
    BitMask<DlSupportedOperatingModes> supportedOperatingModes =
        BitMask<DlSupportedOperatingModes>(DlSupportedOperatingModes::kNormal, DlSupportedOperatingModes::kNoRemoteLockUnlock);

    // Capacities, only used when the corresponding feature is enabled.
    uint16_t numberOfTotalUsersSupported        = 0;
    uint16_t numberOfPINUsersSupported          = 0;
    uint16_t numberOfRFIDUsersSupported         = 0;
    uint8_t numberOfWeekDaySchedulesPerUser     = 0;
    uint8_t numberOfYearDaySchedulesPerUser     = 0;
    uint8_t numberOfHolidaySchedulesSupported   = 0;
    uint8_t numberOfCredentialsSupportedPerUser = 0;
    BitMask<DlCredentialRuleMask> credentialRulesSupport;

    uint8_t maxPINCodeLength  = 0;
    uint8_t minPINCodeLength  = 0;
    uint8_t maxRFIDCodeLength = 0;
    uint8_t minRFIDCodeLength = 0;

    // ---- Initial values for mutable attributes --------------------------------
    DataModel::Nullable<DlLockState> lockState;
    DataModel::Nullable<DoorStateEnum> doorState;
    uint32_t doorOpenEvents         = 0;
    uint32_t doorClosedEvents       = 0;
    uint16_t openPeriod             = 0;
    bool actuatorEnabled            = true;
    OperatingModeEnum operatingMode = OperatingModeEnum::kNormal;
    CharSpan language;
    uint8_t ledSettings     = 0;
    uint32_t autoRelockTime = 0;
    uint8_t soundVolume     = 0;
    BitMask<DlDefaultConfigurationRegister> defaultConfigurationRegister;
    bool enableLocalProgramming  = true;
    bool enableOneTouchLocking   = false;
    bool enableInsideStatusLED   = false;
    bool enablePrivacyModeButton = false;
    BitMask<DlLocalProgrammingFeatures> localProgrammingFeatures;
    uint8_t wrongCodeEntryLimit          = 0;
    uint8_t userCodeTemporaryDisableTime = 0;
    bool sendPINOverTheAir               = false;
    bool requirePINforRemoteOperation    = false;
    uint16_t expiringUserTimeout         = 0;

    /// Timer delegate used for auto-relock scheduling and monotonic timestamps.
    TimerDelegate & timerDelegate;
};

/**
 * @brief Code-driven implementation of the Matter Door Lock cluster (0x0101).
 *
 * One instance per endpoint. All attribute state is stored in class members
 * (or provided by the delegate for Aliro attributes); the ember attribute
 * store is not used.
 *
 * Hardware actuation is performed by the DoorLock::Delegate; user/credential
 * and schedule storage integration is layered on top of the same delegate.
 */
class DoorLockCluster : public DefaultServerCluster
{
public:
    DoorLockCluster(EndpointId endpointId, Delegate & delegate, const Config & config);

    ~DoorLockCluster() override = default;

    // ---- ServerClusterInterface implementation --------------------------------

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    // ---- Application-facing API ------------------------------------------------

    /// Applies a lock-state change originating from a local operation and emits the
    /// corresponding LockOperation / LockOperationError events.
    CHIP_ERROR SetLockState(DlLockState newState, OperationSourceEnum opSource, const DataModel::Nullable<uint16_t> & userIndex,
                            const Span<const CredentialStruct> & credentials, const DataModel::Nullable<FabricIndex> & fabricIdx,
                            const DataModel::Nullable<NodeId> & nodeId);

    /// Updates the door position state and emits a DoorStateChange event.
    CHIP_ERROR SetDoorState(DataModel::Nullable<DoorStateEnum> doorState);

    /// Sends a DoorLockAlarm event.
    CHIP_ERROR SendLockAlarmEvent(AlarmCodeEnum alarmCode);

    /// Counts a wrong credential entry against WrongCodeEntryLimit, engaging the
    /// temporary lockout when the limit is reached.
    CHIP_ERROR HandleWrongCodeEntry();

    /// Clears the wrong-code entry attempts counter.
    void ResetWrongCodeEntryAttempts();

    /// True while the UserCodeTemporaryDisableTime lockout window is active.
    bool IsLockoutEngaged() const;

    // ---- Capacity access (applications size their local storage from these) ---
    uint16_t GetNumberOfUserSupported() const { return mNumberOfTotalUsersSupported; }
    uint16_t GetNumberOfPINCredentialsSupported() const { return mNumberOfPINUsersSupported; }
    uint16_t GetNumberOfRFIDCredentialsSupported() const { return mNumberOfRFIDUsersSupported; }
    uint8_t GetNumberOfWeekDaySchedulesPerUserSupported() const { return mNumberOfWeekDaySchedulesPerUser; }
    uint8_t GetNumberOfYearDaySchedulesPerUserSupported() const { return mNumberOfYearDaySchedulesPerUser; }
    uint8_t GetNumberOfHolidaySchedulesSupported() const { return mNumberOfHolidaySchedulesSupported; }
    uint8_t GetNumberOfCredentialsSupportedPerUser() const { return mNumberOfCredentialsSupportedPerUser; }
    bool RequirePINforRemoteOperation() const { return mRequirePINforRemoteOperation; }

    // ---- Feature access (mirrors the legacy DoorLockServer API) ----------------

    BitFlags<Feature> Features() const { return mFeatures; }

    /// True when the current OperatingMode allows remote lock/unlock operations.
    bool RemoteOperationEnabled() const
    {
        return (mOperatingMode != OperatingModeEnum::kPrivacy) && (mOperatingMode != OperatingModeEnum::kNoRemoteLockUnlock);
    }

    DlLockState GetLockState() const { return mLockState.ValueOr(DlLockState::kNotFullyLocked); }
    uint32_t GetAutoRelockTime() const { return mAutoRelockTime; }

private:
    Delegate & mDelegate;
    TimerDelegate & mTimerDelegate;

    // ---- Fixed configuration (const after construction) ------------------------
    const BitFlags<Feature> mFeatures;
    const OptionalAttributes mOptionalAttributes;
    const DlLockType mLockType;
    const BitMask<DlSupportedOperatingModes> mSupportedOperatingModes;
    const uint16_t mNumberOfTotalUsersSupported;
    const uint16_t mNumberOfPINUsersSupported;
    const uint16_t mNumberOfRFIDUsersSupported;
    const uint8_t mNumberOfWeekDaySchedulesPerUser;
    const uint8_t mNumberOfYearDaySchedulesPerUser;
    const uint8_t mNumberOfHolidaySchedulesSupported;
    const uint8_t mNumberOfCredentialsSupportedPerUser;
    const BitMask<DlCredentialRuleMask> mCredentialRulesSupport;
    const uint8_t mMaxPINCodeLength;
    const uint8_t mMinPINCodeLength;
    const uint8_t mMaxRFIDCodeLength;
    const uint8_t mMinRFIDCodeLength;

    // ---- Mutable attribute storage ----------------------------------------------
    DataModel::Nullable<DlLockState> mLockState;
    DataModel::Nullable<DoorStateEnum> mDoorState;
    uint32_t mDoorOpenEvents;
    uint32_t mDoorClosedEvents;
    uint16_t mOpenPeriod;
    bool mActuatorEnabled;
    OperatingModeEnum mOperatingMode;
    std::array<char, Internal::kMaxLanguageLength> mLanguage;
    size_t mLanguageLength = 0;
    uint8_t mLEDSettings;
    uint32_t mAutoRelockTime;
    uint8_t mSoundVolume;
    BitMask<DlDefaultConfigurationRegister> mDefaultConfigurationRegister;
    bool mEnableLocalProgramming;
    bool mEnableOneTouchLocking;
    bool mEnableInsideStatusLED;
    bool mEnablePrivacyModeButton;
    BitMask<DlLocalProgrammingFeatures> mLocalProgrammingFeatures;
    uint8_t mWrongCodeEntryLimit;
    uint8_t mUserCodeTemporaryDisableTime;
    bool mSendPINOverTheAir;
    bool mRequirePINforRemoteOperation;
    uint16_t mExpiringUserTimeout;

    // ---- Wrong-code lockout state -------------------------------------------------
    System::Clock::Timestamp mLockoutEndTimestamp{ 0 };
    uint16_t mWrongCodeEntryAttempts = 0;

    /// TimerContext adapter so the cluster itself receives the auto-relock firing.
    struct AutoRelockTimerContext : public TimerContext
    {
        DoorLockCluster * cluster = nullptr;
        void TimerFired() override;
    };
    AutoRelockTimerContext mAutoRelockTimerContext;

    // ---- Internal helpers -----------------------------------------------------------

    using DelegateLockOpHandler = bool (Delegate::*)(chip::EndpointId, const DataModel::Nullable<FabricIndex> &,
                                                     const DataModel::Nullable<NodeId> &, const Optional<chip::ByteSpan> &,
                                                     OperationErrorEnum &);

    /// Shared implementation of LockDoor/UnlockDoor/UnlockWithTimeout/UnboltDoor:
    /// operating-mode gate, lockout window, credential checks, hardware actuation,
    /// LockOperation/LockOperationError events and auto-relock scheduling.
    std::optional<DataModel::ActionReturnStatus> HandleRemoteLockOperation(const DataModel::InvokeRequest & request,
                                                                           LockOperationTypeEnum opType,
                                                                           DelegateLockOpHandler handler,
                                                                           const Optional<chip::ByteSpan> & pinCode);

    /// Engages the wrong-code lockout window and emits the corresponding alarm.
    CHIP_ERROR EngageLockout();

    /// Schedules the auto-relock timer (timeout in seconds).
    void ScheduleAutoRelock(uint32_t timeoutSec);

    /// Auto-relock timer callback: relocks the door with an "auto" operation source.
    void OnAutoRelockTimerExpired();

    /// Emits any typed cluster event through the interaction model context.
    /// Nothing actionable can be done on failure: GenerateEvent already logs it.
    template <typename T>
    void SendEvent(const T & event)
    {
        if (IsStarted())
        {
            mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        }
    }

    /// Emits a LockOperation or LockOperationError event.
    void SendLockOperationEvent(LockOperationTypeEnum opType, OperationSourceEnum opSource, OperationErrorEnum opErr,
                                const DataModel::Nullable<uint16_t> & userIndex, const Span<const CredentialStruct> & credentials,
                                const DataModel::Nullable<FabricIndex> & fabricIdx, const DataModel::Nullable<NodeId> & nodeId,
                                bool opSuccess);

    /// Applies a lock-state change, emitting events and scheduling auto-relock.
    CHIP_ERROR ApplyLockStateChange(DlLockState newState, OperationSourceEnum opSource,
                                    const DataModel::Nullable<uint16_t> & userIndex,
                                    const Span<const CredentialStruct> & credentials,
                                    const DataModel::Nullable<FabricIndex> & fabricIdx, const DataModel::Nullable<NodeId> & nodeId,
                                    bool sendLockOperationEvent);
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
