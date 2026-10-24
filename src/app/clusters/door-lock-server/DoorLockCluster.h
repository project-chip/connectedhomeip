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
#include <app/clusters/door-lock-server/door-lock-types.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/DoorLock/Attributes.h>
#include <clusters/DoorLock/Commands.h>
#include <clusters/DoorLock/Enums.h>
#include <clusters/DoorLock/Events.h>
#include <clusters/DoorLock/Metadata.h>
#include <clusters/DoorLock/Structs.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>
#include <protocols/interaction_model/StatusCode.h>

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

/// Optional attribute enablement of the cluster.
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
    // Feature-gated optional attributes ([DPS], [!USR & PIN] and [USR]
    // conformance): present only when the flag is set AND the feature condition
    // is satisfied.
    bool doorOpenEvents      = false;
    bool doorClosedEvents    = false;
    bool openPeriod          = false;
    bool sendPINOverTheAir   = false;
    bool expiringUserTimeout = false;
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
    // The OperatingModesBitmap uses inverted polarity: a `0` bit marks a
    // supported mode and undefined bits are `1`. The 0xFFF6 default supports
    // Normal and NoRemoteLockUnlock only.
    BitMask<DlSupportedOperatingModes> supportedOperatingModes = BitMask<DlSupportedOperatingModes>(0xFFF6);

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
    uint8_t wrongCodeEntryLimit          = 3;
    uint8_t userCodeTemporaryDisableTime = 10;
    bool sendPINOverTheAir               = false;
    bool requirePINforRemoteOperation    = false;
    uint16_t expiringUserTimeout         = 10;

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

    // ---- Feature access (application-facing API) ----------------

    BitFlags<Feature> Features() const { return mFeatures; }

    /// True when the current OperatingMode allows remote lock/unlock operations.
    bool RemoteOperationEnabled() const
    {
        return (mOperatingMode != OperatingModeEnum::kPrivacy) && (mOperatingMode != OperatingModeEnum::kNoRemoteLockUnlock);
    }

    DlLockState GetLockState() const { return mLockState.ValueOr(DlLockState::kNotFullyLocked); }
    const DataModel::Nullable<DlLockState> & GetLockStateNullable() const { return mLockState; }
    uint32_t GetAutoRelockTime() const { return mAutoRelockTime; }
    bool GetRequirePINforRemoteOperation() const { return mRequirePINforRemoteOperation; }

    /// Updates the ActuatorEnabled read-only state (application reported).
    CHIP_ERROR SetActuatorEnabled(bool actuatorEnabled)
    {
        SetAttributeValue(mActuatorEnabled, actuatorEnabled, Attributes::ActuatorEnabled::Id);
        return CHIP_NO_ERROR;
    }

    /// Updates the AutoRelockTime attribute without changing any pending relock.
    CHIP_ERROR SetAutoRelockTime(uint32_t autoRelockTime)
    {
        SetAttributeValue(mAutoRelockTime, autoRelockTime, Attributes::AutoRelockTime::Id);
        return CHIP_NO_ERROR;
    }

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

    using DelegateLockOpHandler = std::optional<OperationErrorEnum> (Delegate::*)(const LockOperationRequest &);

    // ---- User / credential / schedule command handlers --------------------------
    // Each handler mirrors the ember implementation behavior 1:1 (including
    // response commands, cluster-specific failure codes and LockUserChange
    // events); the response is always sent to the CommandHandler here, so the
    // InvokeCommand dispatch returns std::nullopt after calling them.

    void HandleSetUser(const DataModel::InvokeRequest & request, const Commands::SetUser::DecodableType & commandData,
                       CommandHandler * handler, const ConcreteCommandPath & path);
    void HandleGetUser(const DataModel::InvokeRequest & request, uint16_t userIndex, CommandHandler * handler,
                       const ConcreteCommandPath & path);
    void HandleClearUser(const DataModel::InvokeRequest & request, uint16_t userIndex, CommandHandler * handler,
                         const ConcreteCommandPath & path);
    void HandleSetCredential(const DataModel::InvokeRequest & request, const Commands::SetCredential::DecodableType & commandData,
                             CommandHandler * handler, const ConcreteCommandPath & path);
    void HandleGetCredentialStatus(CredentialTypeEnum credentialType, uint16_t credentialIndex, CommandHandler * handler,
                                   const ConcreteCommandPath & path);
    void HandleClearCredential(const DataModel::InvokeRequest & request,
                               const Commands::ClearCredential::DecodableType & commandData, CommandHandler * handler,
                               const ConcreteCommandPath & path);

    void HandleSetWeekDaySchedule(const DataModel::InvokeRequest & request, uint8_t weekDayIndex, uint16_t userIndex,
                                  const BitMask<DaysMaskMap> & daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                  uint8_t endMinute, CommandHandler * handler, const ConcreteCommandPath & path);
    void HandleGetWeekDaySchedule(uint8_t weekDayIndex, uint16_t userIndex, CommandHandler * handler,
                                  const ConcreteCommandPath & path);
    void HandleClearWeekDaySchedule(const DataModel::InvokeRequest & request, uint8_t weekDayIndex, uint16_t userIndex,
                                    CommandHandler * handler, const ConcreteCommandPath & path);

    void HandleSetYearDaySchedule(const DataModel::InvokeRequest & request, uint8_t yearDayIndex, uint16_t userIndex,
                                  uint32_t localStartTime, uint32_t localEndTime, CommandHandler * handler,
                                  const ConcreteCommandPath & path);
    void HandleGetYearDaySchedule(uint8_t yearDayIndex, uint16_t userIndex, CommandHandler * handler,
                                  const ConcreteCommandPath & path);
    void HandleClearYearDaySchedule(const DataModel::InvokeRequest & request, uint8_t yearDayIndex, uint16_t userIndex,
                                    CommandHandler * handler, const ConcreteCommandPath & path);

    void HandleSetHolidaySchedule(uint8_t holidayIndex, uint32_t localStartTime, uint32_t localEndTime,
                                  OperatingModeEnum operatingMode, CommandHandler * handler, const ConcreteCommandPath & path);
    void HandleGetHolidaySchedule(uint8_t holidayIndex, CommandHandler * handler, const ConcreteCommandPath & path);
    void HandleClearHolidaySchedule(uint8_t holidayIndex, CommandHandler * handler, const ConcreteCommandPath & path);

    void HandleSetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey, const ByteSpan & groupIdentifier,
                                    const Optional<ByteSpan> & groupResolvingKey, CommandHandler * handler,
                                    const ConcreteCommandPath & path);
    void HandleClearAliroReaderConfig(CommandHandler * handler, const ConcreteCommandPath & path);

    // ---- Command helpers (ported from the ember implementation) -----------------

    /// True when the user management feature is enabled (also required for
    /// per-user schedules and user-scoped credentials).
    bool SupportsUSR() const { return mFeatures.Has(Feature::kUser); }
    bool SupportsWeekDaySchedules() const { return mFeatures.Has(Feature::kWeekDayAccessSchedules); }
    bool SupportsYearDaySchedules() const { return mFeatures.Has(Feature::kYearDayAccessSchedules); }
    bool SupportsHolidaySchedules() const { return mFeatures.Has(Feature::kHolidaySchedules); }
    bool SupportsPIN() const { return mFeatures.Has(Feature::kPinCredential); }
    bool SupportsRFID() const { return mFeatures.Has(Feature::kRfidCredential); }
    bool SupportsFingers() const { return mFeatures.Has(Feature::kFingerCredentials); }
    bool SupportsFace() const { return mFeatures.Has(Feature::kFaceCredentials); }
    bool SupportsAliroProvisioning() const { return mFeatures.Has(Feature::kAliroProvisioning); }
    bool SupportsAliroBLEUWB() const { return mFeatures.Has(Feature::kAliroBLEUWB); }

    bool UserIndexValid(uint16_t userIndex) const;
    bool UserIndexValid(uint16_t userIndex, uint16_t & maxNumberOfUsers) const;
    bool UserExists(uint16_t userIndex) const;

    bool CredentialIndexValid(CredentialTypeEnum type, uint16_t credentialIndex, uint16_t & maxNumberOfCredentials) const;
    chip::app::Clusters::DoorLock::DlStatus CredentialLengthWithinRange(CredentialTypeEnum type,
                                                                        const ByteSpan & credentialData) const;
    chip::app::Clusters::DoorLock::DlStatus AliroCredentialDataValid(CredentialTypeEnum type,
                                                                     const ByteSpan & credentialData) const;
    bool GetMaxNumberOfCredentials(CredentialTypeEnum credentialType, uint16_t & maxNumberOfCredentials) const;
    uint16_t NumberOfCredentialsSupportedPerUserValue() const { return mNumberOfCredentialsSupportedPerUser; }

    bool FindOccupiedUserSlot(uint16_t startIndex, uint16_t & userIndex) const;
    bool FindUnoccupiedUserSlot(uint16_t & userIndex) const;
    bool FindUnoccupiedUserSlot(uint16_t startIndex, uint16_t & userIndex) const;
    bool FindOccupiedCredentialSlot(CredentialTypeEnum credentialType, uint16_t startIndex, uint16_t & credentialIndex) const;
    bool FindUnoccupiedCredentialSlot(CredentialTypeEnum credentialType, uint16_t startIndex, uint16_t & credentialIndex) const;
    bool FindUserIndexByCredential(CredentialTypeEnum credentialType, uint16_t credentialIndex, uint16_t & userIndex) const;
    bool FindUserIndexByCredential(CredentialTypeEnum credentialType, ByteSpan credentialData, uint16_t & userIndex,
                                   uint16_t & credentialIndex, EmberAfPluginDoorLockUserInfo & userInfo) const;

    bool CredentialTypeSupported(CredentialTypeEnum type) const;
    bool WeekDayIndexValid(uint8_t weekDayIndex) const;
    bool YearDayIndexValid(uint8_t yearDayIndex) const;
    bool HolidayIndexValid(uint8_t holidayIndex) const;
    bool CountOccupiedCredentials(CredentialTypeEnum credentialType, size_t & occupiedCount) const;
    LockDataTypeEnum CredentialTypeToLockDataType(CredentialTypeEnum credentialType) const;
    bool IsUserScheduleRestricted(const EmberAfPluginDoorLockUserInfo & user) const;

    chip::Protocols::InteractionModel::ClusterStatusCode
    CreateUser(FabricIndex creatorFabricIdx, NodeId sourceNodeId, uint16_t userIndex,
               const DataModel::Nullable<CharSpan> & userName, const DataModel::Nullable<uint32_t> & userUniqueId,
               const DataModel::Nullable<UserStatusEnum> & userStatus, const DataModel::Nullable<UserTypeEnum> & userType,
               const DataModel::Nullable<CredentialRuleEnum> & credentialRule,
               const DataModel::Nullable<CredentialStruct> & credential = DataModel::Nullable<CredentialStruct>());
    chip::Protocols::InteractionModel::Status ModifyUser(FabricIndex modifierFabricIndex, NodeId sourceNodeId, uint16_t userIndex,
                                                         const DataModel::Nullable<CharSpan> & userName,
                                                         const DataModel::Nullable<uint32_t> & userUniqueId,
                                                         const DataModel::Nullable<UserStatusEnum> & userStatus,
                                                         const DataModel::Nullable<UserTypeEnum> & userType,
                                                         const DataModel::Nullable<CredentialRuleEnum> & credentialRule);
    chip::Protocols::InteractionModel::Status ClearUser(FabricIndex modifierFabricId, NodeId sourceNodeId, uint16_t userIndex,
                                                        const EmberAfPluginDoorLockUserInfo & user, bool sendUserChangeEvent);
    chip::Protocols::InteractionModel::Status ClearUser(FabricIndex modifierFabricId, NodeId sourceNodeId, uint16_t userIndex,
                                                        bool sendUserChangeEvent);

    DlStatus CreateNewCredentialAndUser(FabricIndex creatorFabricIdx, NodeId sourceNodeId,
                                        const DataModel::Nullable<UserStatusEnum> & userStatus,
                                        const DataModel::Nullable<UserTypeEnum> & userType, const CredentialStruct & credential,
                                        const ByteSpan & credentialData, uint16_t & createdUserIndex);
    DlStatus CreateNewCredentialAndAddItToUser(FabricIndex modifierFabricIdx, uint16_t userIndex,
                                               const CredentialStruct & credential, const ByteSpan & credentialData);
    DlStatus AddCredentialToUser(FabricIndex modifierFabricIdx, uint16_t userIndex, const CredentialStruct & credential);
    DlStatus ModifyCredentialForUser(FabricIndex modifierFabricIdx, uint16_t userIndex, const CredentialStruct & credential);
    DlStatus CreateCredential(FabricIndex creatorFabricIdx, NodeId sourceNodeId, uint16_t credentialIndex,
                              CredentialTypeEnum credentialType, const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                              const ByteSpan & credentialData, const DataModel::Nullable<uint16_t> & userIndex,
                              const DataModel::Nullable<UserStatusEnum> & userStatus,
                              const DataModel::Nullable<UserTypeEnum> & userType, uint16_t & createdUserIndex);
    DlStatus ModifyProgrammingPIN(FabricIndex modifierFabricIndex, NodeId sourceNodeId, uint16_t credentialIndex,
                                  CredentialTypeEnum credentialType, const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                  const ByteSpan & credentialData);
    DlStatus ModifyCredential(FabricIndex modifierFabricIndex, NodeId sourceNodeId, uint16_t credentialIndex,
                              CredentialTypeEnum credentialType, const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                              const ByteSpan & credentialData, uint16_t userIndex,
                              const DataModel::Nullable<UserStatusEnum> & userStatus,
                              const DataModel::Nullable<UserTypeEnum> & userType);

    chip::Protocols::InteractionModel::Status ClearCredential(FabricIndex modifier, NodeId sourceNodeId,
                                                              CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                                              bool sendUserChangeEvent);
    chip::Protocols::InteractionModel::Status ClearCredentials(FabricIndex modifier, NodeId sourceNodeId);
    chip::Protocols::InteractionModel::Status ClearCredentials(FabricIndex modifier, NodeId sourceNodeId,
                                                               CredentialTypeEnum credentialType);
    bool ClearFabricFromUsers(FabricIndex fabricIndex) const;
    bool ClearFabricFromCredentials(FabricIndex fabricIndex) const;
    bool ClearFabricFromCredentials(CredentialTypeEnum credentialType, FabricIndex fabricToRemove) const;

    chip::app::Clusters::DoorLock::DlStatus ClearWeekDaySchedule(uint16_t userIndex, uint8_t weekDayIndex) const;
    DlStatus ClearWeekDaySchedules(uint16_t userIndex) const;
    DlStatus ClearSchedules(uint16_t userIndex) const;
    chip::app::Clusters::DoorLock::DlStatus ClearYearDaySchedule(uint16_t userIndex, uint8_t yearDayIndex) const;
    DlStatus ClearYearDaySchedules(uint16_t userIndex) const;
    DlStatus ClearHolidaySchedule(uint8_t holidayIndex) const;
    DlStatus ClearHolidaySchedules() const;

    void SendRemoteLockUserChange(LockDataTypeEnum dataType, DataOperationTypeEnum operation, NodeId nodeId,
                                  FabricIndex fabricIndex, uint16_t userIndex, uint16_t dataIndex) const;

    /// Handles a fabric removal: unstamps the fabric from users and credentials
    /// (so the records survive owned by "no fabric") and notifies the delegate.
    bool HandleFabricRemoved(FabricIndex fabricIndex);

    /// Loads persistent attribute values from the attribute storage.
    void LoadPersistentAttributes(ServerClusterContext & context);

    /// Stores LockState into the attribute persistence storage.
    void StoreLockState() const;

    // ---- Response encoding helpers ------------------------------------------------

    void SendSetCredentialResponse(CommandHandler * handler, const ConcreteCommandPath & path, DlStatus status, uint16_t userIndex,
                                   uint16_t nextCredentialIndex) const;
    void SendGetCredentialResponse(CommandHandler * handler, const ConcreteCommandPath & path, CredentialTypeEnum credentialType,
                                   uint16_t credentialIndex, const DataModel::Nullable<uint16_t> & nextCredentialIndex,
                                   uint16_t userIndexWithCredential, const EmberAfPluginDoorLockCredentialInfo * credentialInfo,
                                   bool credentialExists) const;
    void SendGetWeekDayScheduleResponse(CommandHandler * handler, const ConcreteCommandPath & path, uint8_t weekDayIndex,
                                        uint16_t userIndex, DlStatus status, DaysMaskMap daysMask, uint8_t startHour,
                                        uint8_t startMinute, uint8_t endHour, uint8_t endMinute) const;
    void SendGetYearDayScheduleResponse(CommandHandler * handler, const ConcreteCommandPath & path, uint8_t yearDayIndex,
                                        uint16_t userIndex, DlStatus status, uint32_t localStartTime, uint32_t localEndTime) const;
    void SendGetHolidayScheduleResponse(CommandHandler * handler, const ConcreteCommandPath & path, uint8_t holidayIndex,
                                        DlStatus status, uint32_t localStartTime, uint32_t localEndTime,
                                        OperatingModeEnum operatingMode) const;

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
    void SendEvent(const T & event) const
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
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
