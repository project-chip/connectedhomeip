/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief APIs and defines for the Door Lock Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "door-lock-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>

#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

using chip::Optional;
using chip::app::Clusters::DoorLock::AlarmCodeEnum;
using chip::app::Clusters::DoorLock::CredentialRuleEnum;
using chip::app::Clusters::DoorLock::CredentialTypeEnum;
using chip::app::Clusters::DoorLock::DataOperationTypeEnum;
using chip::app::Clusters::DoorLock::DaysMaskMap;
using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::DlStatus;
using chip::app::Clusters::DoorLock::DoorStateEnum;
using chip::app::Clusters::DoorLock::LockDataTypeEnum;
using chip::app::Clusters::DoorLock::LockOperationTypeEnum;
using chip::app::Clusters::DoorLock::OperatingModeEnum;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;
using chip::app::Clusters::DoorLock::UserStatusEnum;
using chip::app::Clusters::DoorLock::UserTypeEnum;
using chip::app::DataModel::List;
using chip::app::DataModel::Nullable;
using chip::app::DataModel::NullNullable;

using CredentialStruct  = chip::app::Clusters::DoorLock::Structs::CredentialStruct::Type;
using LockOpCredentials = CredentialStruct;

/**
 * Handler for executing remote lock operations.
 *
 * @param endpointId endpoint for which remote lock command is called
 * @param fabricIdx fabric index responsible for operating the lock
 * @param nodeId node id responsible for operating the lock
 * @param pinCode PIN code (optional)
 * @param err error code if door locking failed (set only if retval==false)
 */
typedef bool (*RemoteLockOpHandler)(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                    const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                    OperationErrorEnum & err);

static constexpr size_t DOOR_LOCK_MAX_USER_NAME_SIZE = 10; /**< Maximum size of the user name (in characters). */
static constexpr size_t DOOR_LOCK_USER_NAME_BUFFER_SIZE =
    DOOR_LOCK_MAX_USER_NAME_SIZE + 1; /**< Maximum size of the user name string (in bytes). */

struct EmberAfPluginDoorLockCredentialInfo;
struct EmberAfPluginDoorLockUserInfo;

struct EmberAfDoorLockEndpointContext
{
    chip::System::Clock::Timestamp lockoutEndTimestamp;
    int wrongCodeEntryAttempts;
    chip::app::Clusters::DoorLock::Delegate * delegate = nullptr;
};

/**
 * @brief Door Lock Server Plugin class.
 */
class DoorLockServer : public chip::app::AttributeAccessInterface
{
public:
    DoorLockServer() : AttributeAccessInterface(chip::Optional<chip::EndpointId>::Missing(), chip::app::Clusters::DoorLock::Id) {}
    static DoorLockServer & Instance();

    using Feature                       = chip::app::Clusters::DoorLock::Feature;
    using OnFabricRemovedCustomCallback = void (*)(chip::EndpointId endpointId, chip::FabricIndex fabricIndex);

    /**
     * Multiple InitEndpoint calls can happen for different endpoints.  Calling
     * InitEndpoint twice for the same endpoint requires a ShutdownEndpoint call
     * for that endpoint in between.
     *
     * A DoorLock::Delegate is optional, but needs to be provided in either
     * InitEndpoint or in a separate SetDelegate call for Aliro features, and
     * possibly other new features, to work.
     */
    CHIP_ERROR InitEndpoint(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::Delegate * delegate = nullptr);

    void ShutdownEndpoint(chip::EndpointId endpointId);

    // InitServer is a deprecated alias for InitEndpoint with no delegate.
    void InitServer(chip::EndpointId endpointid);

    /**
     * Delegate is not supposed to be null. Removing a delegate
     * should only happen when shutting down the door lock cluster on the
     * endpoint, via ShutdownEndpoint.
     */
    CHIP_ERROR SetDelegate(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::Delegate * delegate);

    /**
     * Updates the LockState attribute with new value and sends LockOperation event.
     *
     * @note Does not send an event of opSource is kRemote.
     *
     * @param endpointId ID of the endpoint to the lock state
     * @param newLockState new lock state
     * @param opSource source of the operation (will be used in the event).
     * @param fabricIdx fabric index responsible for operating the lock
     * @param nodeId node id responsible for operating the lock
     *
     * @return true on success, false on failure.
     */
    bool SetLockState(chip::EndpointId endpointId, DlLockState newLockState, OperationSourceEnum opSource,
                      const Nullable<uint16_t> & userIndex                        = NullNullable,
                      const Nullable<List<const LockOpCredentials>> & credentials = NullNullable,
                      const Nullable<chip::FabricIndex> & fabricIdx               = NullNullable,
                      const Nullable<chip::NodeId> & nodeId                       = NullNullable);

    /**
     * Updates the LockState attribute with new value.
     *
     * @note Does not generate Lock Operation event
     *
     * @param endpointId ID of the endpoint to the lock state
     * @param newLockState new lock state
     *
     * @return true on success, false on failure.
     */
    bool SetLockState(chip::EndpointId endpointId, DlLockState newLockState);
    bool SetActuatorEnabled(chip::EndpointId endpointId, bool newActuatorState);
    bool SetDoorState(chip::EndpointId endpointId, DoorStateEnum newDoorState);

    bool SetLanguage(chip::EndpointId endpointId, chip::CharSpan newLanguage);
    bool SetAutoRelockTime(chip::EndpointId endpointId, uint32_t newAutoRelockTimeSec);
    bool SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume);

    bool SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled);
    bool SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled);

    /**
     * @brief Handles a wrong code entry attempt for the endpoint. If the number of wrong entry attempts exceeds the max limit,
     *        engage lockout. Otherwise increment the number of incorrect attempts by 1. This is handled automatically for
     *        remote operations - lock and unlock.  Applications are responsible for calling it for non-remote incorrect credential
     * presentation attempts.
     *
     * @param endpointId
     */
    bool HandleWrongCodeEntry(chip::EndpointId endpointId);

    bool GetAutoRelockTime(chip::EndpointId endpointId, uint32_t & autoRelockTime);
    bool GetNumberOfUserSupported(chip::EndpointId endpointId, uint16_t & numberOfUsersSupported);
    bool GetNumberOfPINCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfPINCredentials);
    bool GetNumberOfRFIDCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfRFIDCredentials);
    bool GetNumberOfWeekDaySchedulesPerUserSupported(chip::EndpointId endpointId, uint8_t & numberOfWeekDaySchedulesPerUser);
    bool GetNumberOfYearDaySchedulesPerUserSupported(chip::EndpointId endpointId, uint8_t & numberOfYearDaySchedulesPerUser);
    bool GetNumberOfCredentialsSupportedPerUser(chip::EndpointId endpointId, uint8_t & numberOfCredentialsSupportedPerUser);
    bool GetNumberOfHolidaySchedulesSupported(chip::EndpointId endpointId, uint8_t & numberOfHolidaySchedules);

    bool SendLockAlarmEvent(chip::EndpointId endpointId, AlarmCodeEnum alarmCode);

    static chip::BitFlags<Feature> GetFeatures(chip::EndpointId endpointId);

    static inline bool SupportsPIN(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(Feature::kPinCredential); }

    static inline bool SupportsRFID(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(Feature::kRfidCredential); }

    static inline bool SupportsFingers(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kFingerCredentials);
    }

    static inline bool SupportsFace(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(Feature::kFaceCredentials); }

    static inline bool SupportsWeekDaySchedules(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kWeekDayAccessSchedules);
    }

    static inline bool SupportsYearDaySchedules(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kYearDayAccessSchedules);
    }

    static inline bool SupportsHolidaySchedules(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kHolidaySchedules);
    }

    static inline bool SupportsAnyCredential(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId)
            .HasAny(Feature::kPinCredential, Feature::kRfidCredential, Feature::kFingerCredentials, Feature::kFaceCredentials,
                    Feature::kAliroProvisioning);
    }

    static inline bool SupportsCredentialsOTA(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kCredentialsOverTheAirAccess);
    }

    static inline bool SupportsUSR(chip::EndpointId endpointId)
    {
        // appclusters, 5.2.2: USR feature has conformance [PIN | RID | FGP | FACE]
        return GetFeatures(endpointId).Has(Feature::kUser) && SupportsAnyCredential(endpointId);
    }

    static bool SupportsUnbolt(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(Feature::kUnbolt); }

    /**
     * @brief Checks if Aliro Provisioning feature is supported on the given endpoint
     *
     * @param endpointId endpointId ID of the endpoint which contains the lock.
     */
    static inline bool SupportsAliroProvisioning(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kAliroProvisioning);
    }

    /**
     * @brief Checks if Aliro BLE UWB feature is supported on the given endpoint
     *
     * @param endpointId endpointId ID of the endpoint which contains the lock.
     */
    static inline bool SupportsAliroBLEUWB(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(Feature::kAliroBLEUWB);
    }

    /**
     * @brief Allows the application to register a custom callback which will be called after the default DoorLock
     *        OnFabricRemoved implementation. At that point the door lock cluster has done any
     *        spec-required clearing of state for fabric removal.
     *
     * @param callback callback to be registered
     */
    inline void SetOnFabricRemovedCustomCallback(OnFabricRemovedCustomCallback callback)
    {
        mOnFabricRemovedCustomCallback = callback;
    }

    bool OnFabricRemoved(chip::EndpointId endpointId, chip::FabricIndex fabricIndex);

    static void DoorLockOnAutoRelockCallback(chip::System::Layer *, void * callbackContext);
    /**
     * @brief Resets the wrong code entry attempts to 0 for the endpoint. This is done automatically when a
     *        remote lock operation with credentials succeeds, or when SetLockState is called with a non-empty credentials list.
     *        Applications that call the two-argument version of SetLockState and handle sending the relevant operation events
     *        themselves or via SendLockOperationEvent are responsible for calling this API when a valid credential is presented.
     *
     * @param endpointId
     */
    void ResetWrongCodeEntryAttempts(chip::EndpointId endpointId);

    /**
     * @brief Handles a local lock operation error. This method allows handling a wrong attempt of providing
     *        user credential entry that has been provided locally by the user. The method will emit the LockOperationEvent
     *        to inform the controller that a local wrong attempt occurred, and also call HandleWrongEntry method to
     *        increment wrong entry counter.
     *
     * @param endpointId
     * @param opType Operation source to be registered in the LockOperationEvent.
     * @param opSource source of the operation to be registered in the LockOperationEvent.
     * @param userId Optional user id to be registered in the LockOperationEvent
     */
    void HandleLocalLockOperationError(chip::EndpointId endpointId, LockOperationTypeEnum opType, OperationSourceEnum opSource,
                                       Nullable<uint16_t> userId);

private:
    chip::FabricIndex getFabricIndex(const chip::app::CommandHandler * commandObj);
    chip::NodeId getNodeId(const chip::app::CommandHandler * commandObj);

    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex);
    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUser);
    bool userExists(chip::EndpointId endpointId, uint16_t userIndex);

    bool credentialIndexValid(chip::EndpointId endpointId, CredentialTypeEnum type, uint16_t credentialIndex);
    bool credentialIndexValid(chip::EndpointId endpointId, CredentialTypeEnum type, uint16_t credentialIndex,
                              uint16_t & maxNumberOfCredentials);
    DlStatus credentialLengthWithinRange(chip::EndpointId endpointId, CredentialTypeEnum type,
                                         const chip::ByteSpan & credentialData);
    bool getMaxNumberOfCredentials(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
                                   uint16_t & maxNumberOfCredentials);

    bool findOccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex);

    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex);
    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex);

    bool findOccupiedCredentialSlot(chip::EndpointId endpointId, CredentialTypeEnum credentialType, uint16_t startIndex,
                                    uint16_t & credentialIndex);
    bool findUnoccupiedCredentialSlot(chip::EndpointId endpointId, CredentialTypeEnum credentialType, uint16_t startIndex,
                                      uint16_t & credentialIndex);

    bool findUserIndexByCredential(chip::EndpointId endpointId, CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                   uint16_t & userIndex);

    bool findUserIndexByCredential(chip::EndpointId endpointId, CredentialTypeEnum credentialType, chip::ByteSpan credentialData,
                                   uint16_t & userIndex, uint16_t & credentialIndex, EmberAfPluginDoorLockUserInfo & userInfo);

    chip::Protocols::InteractionModel::ClusterStatusCode
    createUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId, uint16_t userIndex,
               const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
               const Nullable<UserStatusEnum> & userStatus, const Nullable<UserTypeEnum> & userType,
               const Nullable<CredentialRuleEnum> & credentialRule,
               const Nullable<CredentialStruct> & credential = Nullable<CredentialStruct>());
    chip::Protocols::InteractionModel::Status
    modifyUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId, uint16_t userIndex,
               const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
               const Nullable<UserStatusEnum> & userStatus, const Nullable<UserTypeEnum> & userType,
               const Nullable<CredentialRuleEnum> & credentialRule);
    chip::Protocols::InteractionModel::Status clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId,
                                                        chip::NodeId sourceNodeId, uint16_t userIndex, bool sendUserChangeEvent);
    chip::Protocols::InteractionModel::Status clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId,
                                                        chip::NodeId sourceNodeId, uint16_t userIndex,
                                                        const EmberAfPluginDoorLockUserInfo & user, bool sendUserChangeEvent);

    bool clearFabricFromUsers(chip::EndpointId endpointId, chip::FabricIndex fabricIndex);

    DlStatus createNewCredentialAndUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId,
                                        const Nullable<UserStatusEnum> & userStatus, const Nullable<UserTypeEnum> & userType,
                                        const CredentialStruct & credential, const chip::ByteSpan & credentialData,
                                        uint16_t & createdUserIndex);
    DlStatus createNewCredentialAndAddItToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                               const CredentialStruct & credential, const chip::ByteSpan & credentialData);

    DlStatus addCredentialToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                 const CredentialStruct & credential);
    DlStatus modifyCredentialForUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                     const CredentialStruct & credential);

    DlStatus createCredential(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId,
                              uint16_t credentialIndex, CredentialTypeEnum credentialType,
                              const EmberAfPluginDoorLockCredentialInfo & existingCredential, const chip::ByteSpan & credentialData,
                              Nullable<uint16_t> userIndex, const Nullable<UserStatusEnum> & userStatus,
                              Nullable<UserTypeEnum> userType, uint16_t & createdUserIndex);
    /**
     * countOccupiedCredentials counts the number of occupied credentials of the
     * given type.  Returns false on application-side errors (i.e. if the count
     * cannot be determined).
     */
    bool countOccupiedCredentials(chip::EndpointId endpointId, CredentialTypeEnum credentialType, size_t & occupiedCount);
    DlStatus modifyProgrammingPIN(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId,
                                  uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                  const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                  const chip::ByteSpan & credentialData);
    DlStatus modifyCredential(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId,
                              uint16_t credentialIndex, CredentialTypeEnum credentialType,
                              const EmberAfPluginDoorLockCredentialInfo & existingCredential, const chip::ByteSpan & credentialData,
                              uint16_t userIndex, const Nullable<UserStatusEnum> & userStatus, Nullable<UserTypeEnum> userType);

    chip::Protocols::InteractionModel::Status clearCredential(chip::EndpointId endpointId, chip::FabricIndex modifier,
                                                              chip::NodeId sourceNodeId, CredentialTypeEnum credentialType,
                                                              uint16_t credentialIndex, bool sendUserChangeEvent);
    chip::Protocols::InteractionModel::Status clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier,
                                                               chip::NodeId sourceNodeId);
    chip::Protocols::InteractionModel::Status clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier,
                                                               chip::NodeId sourceNodeId, CredentialTypeEnum credentialType);

    bool clearFabricFromCredentials(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
                                    chip::FabricIndex fabricToRemove);
    bool clearFabricFromCredentials(chip::EndpointId endpointId, chip::FabricIndex fabricToRemove);

    void sendSetCredentialResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                   DlStatus status, uint16_t userIndex, uint16_t nextCredentialIndex);

    // TODO: Maybe use CHIP_APPLICATION_ERROR instead of boolean in class methods?
    // OPTIMIZE: there are a lot of methods such as this that could be made static which could help reduce the stack footprint
    // in case of multiple lock endpoints
    bool credentialTypeSupported(chip::EndpointId endpointId, CredentialTypeEnum type);

    bool weekDayIndexValid(chip::EndpointId endpointId, uint8_t weekDayIndex);

    DlStatus clearWeekDaySchedule(chip::EndpointId endpointId, uint16_t userIndex, uint8_t weekDayIndex);
    DlStatus clearWeekDaySchedules(chip::EndpointId endpointId, uint16_t userIndex);
    DlStatus clearSchedules(chip::EndpointId endpointId, uint16_t userIndex);

    void sendGetWeekDayScheduleResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                        uint8_t weekdayIndex, uint16_t userIndex, DlStatus status,
                                        DaysMaskMap daysMask = DaysMaskMap(0), uint8_t startHour = 0, uint8_t startMinute = 0,
                                        uint8_t endHour = 0, uint8_t endMinute = 0);

    bool yearDayIndexValid(chip::EndpointId endpointId, uint8_t yearDayIndex);

    DlStatus clearYearDaySchedule(chip::EndpointId endpointId, uint16_t userIndex, uint8_t weekDayIndex);
    DlStatus clearYearDaySchedules(chip::EndpointId endpointId, uint16_t userIndex);

    void sendGetYearDayScheduleResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                        uint8_t yearDayIndex, uint16_t userIndex, DlStatus status, uint32_t localStartTime = 0,
                                        uint32_t localEndTime = 0);

    bool holidayIndexValid(chip::EndpointId endpointId, uint8_t holidayIndex);

    DlStatus clearHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex);
    DlStatus clearHolidaySchedules(chip::EndpointId endpointId);

    void sendHolidayScheduleResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                     uint8_t holidayIndex, DlStatus status, uint32_t localStartTime = 0, uint32_t localEndTime = 0,
                                     OperatingModeEnum operatingMode = OperatingModeEnum::kNormal);

    bool sendRemoteLockUserChange(chip::EndpointId endpointId, LockDataTypeEnum dataType, DataOperationTypeEnum operation,
                                  chip::NodeId nodeId, chip::FabricIndex fabricIndex, uint16_t userIndex = 0,
                                  uint16_t dataIndex = 0);

    LockDataTypeEnum credentialTypeToLockDataType(CredentialTypeEnum credentialType);

    bool isUserScheduleRestricted(chip::EndpointId endpointId, const EmberAfPluginDoorLockUserInfo & user);

    void setUserCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData);

    void getUserCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               uint16_t userIndex);

    void clearUserCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                 uint16_t userIndex);

    void setCredentialCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                     const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData);

    void getCredentialStatusCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath, CredentialTypeEnum credentialType,
                                           uint16_t credentialIndex);

    void sendGetCredentialResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                   CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                   chip::app::DataModel::Nullable<uint16_t> nextCredentialIndex, uint16_t userIndexWithCredential,
                                   EmberAfPluginDoorLockCredentialInfo * credentialInfo, bool credentialExists);

    void clearCredentialCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                       const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData);

    void setWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                          uint16_t userIndex, const chip::BitMask<DaysMaskMap> & daysMask, uint8_t startHour,
                                          uint8_t startMinute, uint8_t endHour, uint8_t endMinute);

    void getWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                          uint16_t userIndex);

    void clearWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                            uint16_t userIndex);

    void setYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                          uint16_t userIndex, uint32_t localStartTime, uint32_t localEndTime);
    void getYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                          uint16_t userIndex);
    void clearYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                            uint16_t userIndex);

    void setHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode);

    void getHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex);

    void clearHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex);

    void setAliroReaderConfigCommandHandler(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath, const chip::ByteSpan & signingKey,
                                            const chip::ByteSpan & verificationKey, const chip::ByteSpan & groupIdentifier,
                                            const Optional<chip::ByteSpan> & groupResolvingKey);
    void clearAliroReaderConfigCommandHandler(chip::app::CommandHandler * commandObj,
                                              const chip::app::ConcreteCommandPath & commandPath);

    bool RemoteOperationEnabled(chip::EndpointId endpointId) const;

    EmberAfDoorLockEndpointContext * getContext(chip::EndpointId endpointId);

    bool engageLockout(chip::EndpointId endpointId);

    static void sendClusterResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                    chip::Protocols::InteractionModel::ClusterStatusCode status);

    /**
     * Get the DoorLock::Delegate for the given endpoint, if any. Will return
     * null if there is no door lock server initialized on that endpoint or if
     * there is no delegate associated with the initialized server.
     */
    chip::app::Clusters::DoorLock::Delegate * GetDelegate(chip::EndpointId endpointId);

    /**
     * @brief Common handler for LockDoor, UnlockDoor, UnlockWithTimeout commands
     *
     * @param commandObj    original command context
     * @param commandPath   original command path
     * @param opType        remote operation type (lock, unlock)
     * @param opHandler     plugin handler for specified command
     * @param pinCode       pin code passed by client
     * @return true         if locking/unlocking was successful
     * @return false        if error happened during lock/unlock
     */
    bool HandleRemoteLockOperation(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                   LockOperationTypeEnum opType, RemoteLockOpHandler opHandler,
                                   const chip::Optional<chip::ByteSpan> & pinCode);

    /**
     * @brief Send LockOperation event if opSuccess is true, otherwise send LockOperationError with given opErr code
     *
     * @param endpointId    endpoint where DoorLockServer is running
     * @param opType        lock operation type (lock, unlock, etc)
     * @param opSource      operation source (remote, keypad, auto, etc)
     * @param opErr         operation error code (if opSuccess == false)
     * @param userId        user id
     * @param fabricIdx     fabric index responsible for operating the lock
     * @param nodeId        node id responsible for operating the lock
     * @param credList      list of credentials used in lock operation (can be NULL if no credentials were used)
     * @param credListSize  size of credentials list (if 0, then no credentials were used)
     * @param opSuccess     flags if operation was successful or not
     */
    void SendLockOperationEvent(chip::EndpointId endpointId, LockOperationTypeEnum opType, OperationSourceEnum opSource,
                                OperationErrorEnum opErr, const Nullable<uint16_t> & userId,
                                const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                                const Nullable<List<const LockOpCredentials>> & credentials = NullNullable, bool opSuccess = true);

    /**
     * @brief Schedule auto relocking with a given timeout
     *
     * @param endpointId    endpoint where DoorLockServer is running
     * @param timeoutSec    timeout in seconds
     */
    void ScheduleAutoRelock(chip::EndpointId endpointId, uint32_t timeoutSec);

    /**
     * @brief Send generic event
     *
     * @tparam T            Any event type supported by Matter
     * @param endpointId    endpoint where DoorLockServer is running
     * @param event         event object built by caller
     */
    template <typename T>
    void SendEvent(chip::EndpointId endpointId, T & event);

    /**
     * @brief Get generic attribute value
     *
     * @tparam T            attribute value type
     * @param endpointId    endpoint where DoorLockServer is running
     * @param attributeId   attribute Id (used for logging only)
     * @param getFn         attribute getter function as defined in <Accessors.h>
     * @param value         actual attribute value on success
     * @return true         on success (value is set to the actual attribute value)
     * @return false        if attribute reading failed (value is kept unchanged)
     */
    template <typename T>
    static bool GetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                             chip::Protocols::InteractionModel::Status (*getFn)(chip::EndpointId endpointId, T * value), T & value);

    /**
     * @brief Set generic attribute value
     *
     * @tparam T            attribute value type
     * @param endpointId    endpoint where DoorLockServer is running
     * @param attributeId   attribute Id (used for logging only)
     * @param setFn         attribute setter function as defined in <Accessors.h>
     * @param value         new attribute value
     * @return true         on success
     * @return false        if attribute writing failed
     */
    template <typename T>
    bool SetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                      chip::Protocols::InteractionModel::Status (*setFn)(chip::EndpointId endpointId, T value), T value);

    // AttributeAccessInterface's Read API
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;

    /**
     * @brief Reads AliroExpeditedTransactionSupportedProtocolVersions attribute for door lock
     *
     * @param aEncoder      attribute value encoder.
     * @param delegate      door lock cluster delegate that will provide the value
     *
     * @return CHIP_NO_ERROR  on success
     * @return CHIP_ERROR     if attribute read failed
     */
    CHIP_ERROR ReadAliroExpeditedTransactionSupportedProtocolVersions(chip::app::AttributeValueEncoder & aEncoder,
                                                                      chip::app::Clusters::DoorLock::Delegate * delegate);

    /**
     * @brief Reads AliroSupportedBLEUWBProtocolVersions attribute for door lock
     *
     * @param aEncoder      attribute value encoder.
     * @param delegate      door lock cluster delegate that will provide the value
     *
     * @return CHIP_NO_ERROR  on success
     * @return CHIP_ERROR     if attribute read failed
     */
    CHIP_ERROR ReadAliroSupportedBLEUWBProtocolVersions(chip::app::AttributeValueEncoder & aEncoder,
                                                        chip::app::Clusters::DoorLock::Delegate * delegate);

    /**
     * @brief Indicates whether an attribute can be nullable or not.
     */
    enum class AttributeNullabilityType : uint8_t
    {
        kNullable    = 0, /**< Indicates if an attribute is nullable */
        kNotNullable = 1, /**< Indicates if an attribute is not nullable */
    };

    /**
     * @brief Utility to read aliro attributes of type ByteSpan
     *
     * @param func          getter function for the attribute.
     * @param data          buffer for the data.
     * @param delegate      door lock cluster delegate that will provide the value
     * @param aEncoder      attribute value encoder.
     * @param nullabilityType enum value indicating whether the attribute is nullable or not.
     *
     * @return CHIP_NO_ERROR  on success
     * @return CHIP_ERROR     if attribute read failed
     */
    CHIP_ERROR ReadAliroByteSpanAttribute(CHIP_ERROR (chip::app::Clusters::DoorLock::Delegate::*func)(chip::MutableByteSpan & data),
                                          chip::MutableByteSpan & data, chip::app::Clusters::DoorLock::Delegate * delegate,
                                          chip::app::AttributeValueEncoder & aEncoder, AttributeNullabilityType nullabilityType);

    friend bool
    emberAfDoorLockClusterLockDoorCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::LockDoor::DecodableType & commandData);

    friend bool emberAfDoorLockClusterUnlockDoorCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::UnlockDoor::DecodableType & commandData);

    friend bool emberAfDoorLockClusterUnlockWithTimeoutCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::UnlockWithTimeout::DecodableType & commandData);

    friend bool emberAfDoorLockClusterUnboltDoorCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::UnboltDoor::DecodableType & commandData);

    friend bool emberAfDoorLockClusterSetHolidayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::SetHolidaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterGetHolidayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::GetHolidaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterClearHolidayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::ClearHolidaySchedule::DecodableType & commandData);

    friend bool
    emberAfDoorLockClusterSetUserCallback(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath,
                                          const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData);

    friend bool
    emberAfDoorLockClusterGetUserCallback(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath,
                                          const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData);

    friend bool
    emberAfDoorLockClusterClearUserCallback(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath,
                                            const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData);

    friend bool emberAfDoorLockClusterSetCredentialCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData);

    friend bool emberAfDoorLockClusterGetCredentialStatusCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData);

    friend bool emberAfDoorLockClusterClearCredentialCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData);

    friend bool emberAfDoorLockClusterSetWeekDayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::SetWeekDaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterGetWeekDayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::GetWeekDaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterClearWeekDayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::ClearWeekDaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterSetYearDayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::SetYearDaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterGetYearDayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::GetYearDaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterClearYearDayScheduleCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::ClearYearDaySchedule::DecodableType & commandData);

    friend bool emberAfDoorLockClusterSetAliroReaderConfigCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::SetAliroReaderConfig::DecodableType & commandData);

    friend bool emberAfDoorLockClusterClearAliroReaderConfigCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::ClearAliroReaderConfig::DecodableType & commandData);

    static constexpr size_t kDoorLockClusterServerMaxEndpointCount =
        MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
    static_assert(kDoorLockClusterServerMaxEndpointCount <= kEmberInvalidEndpointIndex, "DoorLock Endpoint count error");

    std::array<EmberAfDoorLockEndpointContext, kDoorLockClusterServerMaxEndpointCount> mEndpointCtx;

    OnFabricRemovedCustomCallback mOnFabricRemovedCustomCallback{ nullptr };

    static DoorLockServer instance;
};

/**
 * @brief Status of the credential slot in the credentials database.
 */
enum class DlCredentialStatus : uint8_t
{
    kAvailable = 0x00, /**< Indicates if credential slot is available. */
    kOccupied  = 0x01, /**< Indicates if credential slot is already occupied. */
};

enum class DlAssetSource : uint8_t
{
    kUnspecified = 0x00,
    kMatterIM    = 0x01,
};

/**
 * @brief Structure that holds the credential information.
 */
struct EmberAfPluginDoorLockCredentialInfo
{
    DlCredentialStatus status = DlCredentialStatus::kAvailable; /**< Indicates if credential slot is occupied or not. */
    CredentialTypeEnum credentialType;                          /**< Specifies the type of the credential (PIN, RFID, etc.). */
    chip::ByteSpan credentialData;                              /**< Credential data bytes. */

    DlAssetSource creationSource;
    chip::FabricIndex createdBy; /**< Index of the fabric that created the user. */

    DlAssetSource modificationSource;
    chip::FabricIndex lastModifiedBy; /**< ID of the fabric that modified the user. */
};

/**
 * @brief Structure that holds user information.
 */
struct EmberAfPluginDoorLockUserInfo
{
    chip::CharSpan userName;                                /**< Name of the user. */
    chip::Span<const CredentialStruct> credentials;         /**< Credentials that are associated with user (without data).*/
    uint32_t userUniqueId;                                  /**< Unique user identifier. */
    UserStatusEnum userStatus = UserStatusEnum::kAvailable; /**< Status of the user slot (available/occupied). */
    UserTypeEnum userType;                                  /**< Type of the user. */
    CredentialRuleEnum credentialRule;                      /**< Number of supported credentials. */

    DlAssetSource creationSource;
    chip::FabricIndex createdBy; /**< ID of the fabric that created the user. */

    DlAssetSource modificationSource;
    chip::FabricIndex lastModifiedBy; /**< ID of the fabric that modified the user. */
};

/**
 * @brief Status of the schedule slot in the schedule database.
 */
enum class DlScheduleStatus : uint8_t
{
    kAvailable = 0x00, /**< Indicates if schedule slot is available. */
    kOccupied  = 0x01, /**< Indicates if schedule slot is already occupied. */
};

/**
 * @brief Structure that holds week day schedule information.
 */
struct EmberAfPluginDoorLockWeekDaySchedule
{
    DaysMaskMap daysMask; /** Indicates the days of the week the Week Day schedule applies for. */
    uint8_t startHour;    /** Starting hour for the Week Day schedule. */
    uint8_t startMinute;  /** Starting minute for the Week Day schedule. */
    uint8_t endHour;      /** Ending hour for the Week Day schedule. */
    uint8_t endMinute;    /** Ending minute for the Week Day schedule. */
};

/**
 * @brief Structure that holds year day schedule information.
 */
struct EmberAfPluginDoorLockYearDaySchedule
{
    uint32_t localStartTime; /** The starting time for the Year Day schedule in Epoch Time in Seconds with local time offset based
                                on the local timezone and DST offset on the day represented by the value. */
    uint32_t localEndTime;   /** The ending time for the Year Day schedule in Epoch Time in Seconds with local time offset based on
                              * the local timezone and DST offset on the day represented by the value. */
};

/**
 * @brief Structure that holds holiday schedule information.
 */
struct EmberAfPluginDoorLockHolidaySchedule
{
    uint32_t localStartTime; /** The starting time for the Holiday schedule in Epoch Time in Seconds with local time offset based
                            on the local timezone and DST offset on the day represented by the value. */
    uint32_t localEndTime;   /** The ending time for the Holiday schedule in Epoch Time in Seconds with local time offset based on
                              * the local timezone and DST offset on the day represented by the value. */
    OperatingModeEnum operatingMode; /** Operating mode during the schedule. */
};

/**
 * @brief This callback is called when Door Lock cluster needs to access the Week Day schedule in the schedules database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param weekdayIndex Index of the week day schedule to access. It is guaranteed to be within limits declared in the spec for
 *                     week day schedule (from 1 up to NumberOfWeekDaySchedulesSupportedPerUser).
 * @param userIndex Index of the user to get week day schedule. It is guaranteed to be within limits declared in the spec (from 1 up
 *                  to the value of NumberOfUsersSupported attribute).
 * @param[out] schedule Resulting week day schedule.
 *
 * @retval DlStatus::kSuccess if schedule was retrieved successfully
 * @retval DlStatus::kNotFound if the schedule or user does not exist
 * @retval DlStatus::kFailure in case of any other failure
 */
DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule);
/**
 * @brief This callback is called when Door Lock cluster needs to access the Year Day schedule in the schedules database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param yearDayIndex Index of the year day schedule to access. It is guaranteed to be within limits declared in the spec for
 *                     year day schedule (from 1 up to NumberOfYearDaySchedulesSupportedPerUser)
 * @param userIndex Index of the user to get year day schedule. It is guaranteed to be within limits declared in the spec (from 1 up
 *                  to the value of NumberOfUsersSupported attribute).
 * @param[out] schedule Resulting year day schedule.
 *
 * @retval DlStatus::kSuccess if schedule was retrieved successfully
 * @retval DlStatus::kNotFound if the schedule or user does not exist
 * @retval DlStatus::kFailure in case of any other failure
 */
DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule);

/**
 * @brief This callback is called when Door Lock cluster needs to access the Holiday schedule in the schedules database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param holidayIndex Index of the holiday schedule to access. It is guaranteed to be within limits declared in the spec for
 *                     holiday schedule (from 1 up to NumberOfHolidaySchedulesSupported)
 * @param[out] schedule Resulting holiday schedule.
 *
 * @retval DlStatus::kSuccess if schedule was retrieved successfully
 * @retval DlStatus::kNotFound if the schedule or user does not exist
 * @retval DlStatus::kFailure in case of any other failure
 */
DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & schedule);

/**
 * @brief This callback is called when Door Lock cluster needs to create, modify or clear the week day schedule in schedules
 * database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param weekdayIndex Index of the week day schedule to access. It is guaranteed to be within limits declared in the spec for
 *                     week day schedule (from 1 up to NumberOfWeekDaySchedulesSupportedPerUser).
 * @param userIndex Index of the user to get year day schedule. It is guaranteed to be within limits declared in the spec (from 1 up
 *                  to the value of NumberOfUsersSupported attribute).
 * @param status New status of the schedule slot (occupied/available). DlScheduleStatus::kAvailable means that the
 *               schedules must be deleted.
 * @param daysMask Indicates the days of the week the Week Day schedule applies for.
 * @param startHour Starting hour for the Week Day schedule.
 * @param startMinute Starting minute for the Week Day schedule
 * @param endHour Ending hour for the Week Day schedule. Guaranteed to be greater or equal to \p startHour.
 * @param endMinute Ending minute for the Week Day schedule. If \p endHour is equal to \p startHour then EndMinute
 *                  is guaranteed to be greater than \p startMinute.
 *
 * @retval DlStatus::kSuccess if schedule was successfully modified
 * @retval DlStatus::kNotFound if the schedule or user does not exist
 * @retval DlStatus::kFailure in case of any other failure
 */
DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute);
/**
 * @brief This callback is called when Door Lock cluster needs to create, modify or clear the year day schedule in schedules
 * database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param yearDayIndex Index of the year day schedule to access. It is guaranteed to be within limits declared in the spec for
 *                     year day schedule (from 1 up to NumberOfYearDaySchedulesSupportedPerUser).
 * @param userIndex Index of the user to get year day schedule. It is guaranteed to be within limits declared in the spec (from 1 up
 *                  to the value of NumberOfUsersSupported attribute).
 * @param status New status of the schedule slot (occupied/available). DlScheduleStatus::kAvailable means that the
 *               schedules must be deleted.
 * @param localStartTime The starting time for the Year Day schedule in Epoch Time in Seconds with local time offset based on the
 *                       local timezone and DST offset on the day represented by the value.
 * @param localEndTime The ending time for the Year Day schedule in Epoch Time in Seconds with local time offset based on the local
 *                     timezone and DST offset on the day represented by the value. \p localEndTime is guaranteed to be greater than
 *                     \p localStartTime.
 *
 * @retval DlStatus::kSuccess if schedule was successfully modified
 * @retval DlStatus::kNotFound if the schedule or user does not exist
 * @retval DlStatus::kFailure in case of any other failure
 */
DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime);

/**
 * @brief This callback is called when Door Lock cluster needs to create, modify or clear the holiday schedule in schedules
 * database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param holidayIndex Index of the holiday schedule to access. It is guaranteed to be within limits declared in the spec for
 *                     holiday schedule (from 1 up to NumberOfHolidaySchedulesSupported).
 * @param status New status of the schedule slot (occupied/available). DlScheduleStatus::kAvailable means that the
 *               schedules must be deleted.
 * @param localStartTime The starting time for the Year Day schedule in Epoch Time in Seconds with local time offset based on the
 *                       local timezone and DST offset on the day represented by the value.
 * @param localEndTime The ending time for the Year Day schedule in Epoch Time in Seconds with local time offset based on the local
 *                     timezone and DST offset on the day represented by the value. \p localEndTime is guaranteed to be greater than
 *                     \p localStartTime.
 * @param operatingMode The operating mode to use during this Holiday schedule start/end time.
 *
 * @retval DlStatus::kSuccess if schedule was successfully modified
 * @retval DlStatus::kNotFound if the schedule or user does not exist
 * @retval DlStatus::kFailure in case of any other failure
 */
DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode);

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

/** @brief 'Language' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  newLanguage     language to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnLanguageChange(chip::EndpointId EndpointId,
                                                                                chip::CharSpan newLanguage);

/** @brief 'AutoRelockTime' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  newTime         relock time value to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnAutoRelockTimeChange(chip::EndpointId EndpointId,
                                                                                      uint32_t newTime);

/** @brief 'SoundVolume' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  newVolume       volume level to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnSoundVolumeChange(chip::EndpointId EndpointId, uint8_t newVolume);

/** @brief 'OperatingMode' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  newMode         operating mode to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnOperatingModeChange(chip::EndpointId EndpointId, uint8_t newMode);

/** @brief 'EnableOneTouchLocking' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  enable          true to enable one touch locking, false otherwise
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnEnableOneTouchLockingChange(chip::EndpointId EndpointId,
                                                                                             bool enable);

/** @brief 'EnablePrivacyModeButton' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  enable          true to enable privacy mode button, false otherwise
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnEnablePrivacyModeButtonChange(chip::EndpointId EndpointId,
                                                                                               bool enable);

/** @brief 'WrongCodeEntryLimit' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  newLimit        new limit for the number of incorrect PIN attempts to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnWrongCodeEntryLimitChange(chip::EndpointId EndpointId,
                                                                                           uint8_t newLimit);

/** @brief 'UserCodeTemporaryDisableTime' attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  newTime         new number of seconds for which lock will be shut down due to wrong code entry
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnUserCodeTemporaryDisableTimeChange(chip::EndpointId EndpointId,
                                                                                                    uint8_t newTime);

/** @note This callback is called for any cluster attribute that has no predefined callback above
 *
 * @brief Cluster attribute pre-change callback
 *
 * @param  EndpointId      endpoint for which attribute is changing
 * @param  attributePath   concrete attribute path that is changing
 * @param  attrType        attribute that is going to be changed
 * @param  attrSize        attribute value storage size
 * @param  attrValue       attribute value to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status
emberAfPluginDoorLockOnUnhandledAttributeChange(chip::EndpointId EndpointId, const chip::app::ConcreteAttributePath & attributePath,
                                                EmberAfAttributeType attrType, uint16_t attrSize, uint8_t * attrValue);

// =============================================================================
// Plugin callbacks that are called by cluster server and should be implemented
// by the server app
// =============================================================================

/**
 * @brief User handler for LockDoor command (server)
 *
 * @param   endpointId      endpoint for which LockDoor command is called
 * @param   fabricIdx       fabric index responsible for operating the lock
 * @param   nodeId          node id responsible for operating the lock
 * @param   pinCode         PIN code (optional)
 * @param   err             error code if door locking failed (set only if retval==false)
 *
 * @retval true on success
 * @retval false if error happenned (err should be set to appropriate error code)
 */
bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                            OperationErrorEnum & err);

/**
 * @brief User handler for UnlockDoor command (server)
 *
 * @param   endpointId      endpoint for which UnlockDoor command is called
 * @param   fabricIdx       fabric index responsible for operating the lock
 * @param   nodeId          node id responsible for operating the lock
 * @param   pinCode         PIN code (optional)
 * @param   err             error code if door unlocking failed (set only if retval==false)
 *
 * @retval true on success
 * @retval false if error happenned (err should be set to appropriate error code)
 */
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                              OperationErrorEnum & err);

/**
 * @brief User handler for UnboltDoor command (server)
 *
 * @param   endpointId      endpoint for which UnboltDoor command is called
 * @param   fabricIdx       fabric index responsible for operating the lock
 * @param   nodeId          node id responsible for operating the lock
 * @param   pinCode         PIN code (optional)
 * @param   err             error code if door unbolting failed (set only if retval==false)
 *
 * @retval true on success
 * @retval false if error happenned (err should be set to appropriate error code)
 */
bool emberAfPluginDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                              OperationErrorEnum & err);

/**
 * @brief This callback is called when the AutoRelock timer is expired.
 *
 * @param endpointId ID of the endpoint that contains the door lock to be relocked.
 */
void emberAfPluginDoorLockOnAutoRelock(chip::EndpointId endpointId);

/**
 * @brief This callback is called when Door Lock cluster needs to access the users database.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param userIndex Index of the user to access. It is guaranteed to be within limits declared in the spec (from 1 up to the value
 *                  of NumberOfUsersSupported attribute).
 * @param[out] user Reference to the user information variable which will hold user info upon successful function call.
 *
 * @retval true, if \p userIndex was found in the database and \p user parameter was written with valid data.
 * @retval false, if error occurred.
 */
bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);

/**
 * @brief This callback is called when Door Lock cluster needs to create, modify or clear the user in users database.
 *
 * @note This function is used for creating, modifying and clearing users. It is not guaranteed that the parameters always differ
 *       from current user state. For example, when modifying a single field (say, uniqueId) the user information that is passed
 *       to a function will be the same as the user record except this field.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param userIndex Index of the user to create/modify. It is guaranteed to be within limits declared in the spec (from 1 up to the
 *                  value of NumberOfUsersSupported attribute).
 * @param creator Fabric ID that created the user. Could be kUndefinedFabricIndex (0).
 * @param modifier Fabric ID that was last to modify the user. Could be kUndefinedFabricIndex (0).
 * @param[in] userName Pointer to the user name. Could be an empty string, data is guaranteed not to be a nullptr.
 * @param uniqueId New Unique ID of the user.
 * @param userStatus New status of the user.
 * @param usertype New type of the user.
 * @param credentialRule New credential rule (how many credentials are allowed for user).
 * @param[in] credentials Array of credentials. Size of this array is determined by totalCredentials variable. Could be nullptr
 *            which means that the credentials should be deleted.
 * @param totalCredentials Size of \p credentials array. Could be 0.
 *
 * @retval true, if user pointed by \p userIndex was successfully changed in the database.
 * @retval false, if error occurred while changing the user.
 */
bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  UserStatusEnum userStatus, UserTypeEnum usertype, CredentialRuleEnum credentialRule,
                                  const CredentialStruct * credentials, size_t totalCredentials);

/**
 * @brief This callback is called when Door Lock cluster needs to access the credential in credentials database.
 *
 * @note The door lock cluster does not assume in any way underlying implementation of the database. Different credential types
 *       may be stored in the single data structure with shared index or separately. Door lock cluster guarantees that the
 *       credentialIndex will always be within the range for a particular credential type.
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param credentialIndex Index of the credential to access. It is guaranteed to be within limits declared in the spec for
 *                        particular credential type. Starts from 1 for all credential types except Programming PIN -- in that case
 *                         it could only be equal to 0.
 * @param credentialType Type of the accessing credential.
 * @param[out] credential Reference to the credential information which will be filled upon successful function call.
 *
 * @retval true, if the credential pointed by \p credentialIndex was found and \p credential parameter was written with valid data.
 * @retval false, if error occurred.
 */
bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential);

/**
 * @brief This callback is called when Door Lock cluster needs to create, modify or clear the credential in credentials database.
 *
 * @note It is guaranteed that the call to this function will not cause any duplicated entries in the database (e.g. entries that
 *       share the same \p credentialType and \p credentialData).
 *
 * @param endpointId ID of the endpoint which contains the lock.
 * @param credentialIndex Index of the credential to access. It is guaranteed to be within limits declared in the spec for
 *                         particular credential type. Starts from 1 for all credential types except Programming PIN -- in that case
 *                         it could only be equal to 0.
 * @param creator Fabric ID that created the user. Could be kUndefinedFabricIndex (0).
 * @param modifier Fabric ID that was last to modify the user. Could be kUndefinedFabricIndex (0).
 * @param credentialStatus New status of the credential slot (occupied/available). DlCredentialStatus::kAvailable means that the
 *                         credential must be deleted.
 * @param credentialType Type of the credential (PIN, RFID, etc.).
 * @param[in] credentialData Data attached to a credential. Can contain nullptr as data which indicates that the data for credential
 *                           should be removed.
 *
 * @retval true, if credential pointed by \p credentialIndex of type \p credentialType was successfully changed in the database.
 * @retval false, if error occurred.
 */
bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                        chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                        CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData);

/**
 * @brief This callback is called when the Door Lock server starts the lockout so the app could be notified about it.
 *
 * @param endpointId ID of the endpoint that contains the door lock to be locked out.
 * @param lockoutEndTime Monotonic time of when lockout ends.
 */
void emberAfPluginDoorLockLockoutStarted(chip::EndpointId endpointId, chip::System::Clock::Timestamp lockoutEndTime);

/**
 * @brief This callback is called when the Door Lock server needs to find out
 * the number of Fingerprint credentials supported, since there is no attribute
 * that represents that value.
 *
 * @param[in] endpointId ID of the endpoint that contains the door lock.
 * @param[out] maxNumberOfCredentials the number of Fingerprint credentials supported by the lock.
 *
 * @return false on failure, true on success.  On failure, the cluster
 * implementation will assume that 0 Fingerprint credentials are supported.
 */
bool emberAfPluginDoorLockGetNumberOfFingerprintCredentialsSupported(chip::EndpointId endpointId,
                                                                     uint16_t & maxNumberOfCredentials);

/**
 * @brief This callback is called when the Door Lock server needs to find out
 * the number of FingerVein credentials supported, since there is no attribute
 * that represents that value.
 *
 * @param[in] endpointId ID of the endpoint that contains the door lock.
 * @param[out] maxNumberOfCredentials the number of FingerVein credentials supported by the lock.
 *
 * @return false on failure, true on success.  On failure, the cluster
 * implementation will assume that 0 FingerVein credentials are supported.
 */
bool emberAfPluginDoorLockGetNumberOfFingerVeinCredentialsSupported(chip::EndpointId endpointId, uint16_t & maxNumberOfCredentials);

/**
 * @brief This callback is called when the Door Lock server needs to find out
 * the number of Face credentials supported, since there is no attribute
 * that represents that value.
 *
 * @param[in] endpointId ID of the endpoint that contains the door lock.
 * @param[out] maxNumberOfCredentials the number of Face credentials supported by the lock.
 *
 * @return false on failure, true on success.  On failure, the cluster
 * implementation will assume that 0 Face credentials are supported.
 */
bool emberAfPluginDoorLockGetNumberOfFaceCredentialsSupported(chip::EndpointId endpointId, uint16_t & maxNumberOfCredentials);

/**
 * @brief This callback is called when the Door Lock server needs to find out
 * the min and max lengths of Fingerprint credentials supported, since there are no
 * attributes that represents those values.
 *
 * @param[in] endpointId ID of the endpoint that contains the door lock.
 * @param[out] minLen the minimal length, in bytes, of a Fingerprint credential supported by the lock.
 * @param[out] maxLen the minimal length, in bytes, of a Fingerprint credential supported by the lock.
 *
 * @return false on failure, true on success.
 */
bool emberAfPluginDoorLockGetFingerprintCredentialLengthConstraints(chip::EndpointId endpointId, uint8_t & minLen,
                                                                    uint8_t & maxLen);

/**
 * @brief This callback is called when the Door Lock server needs to find out
 * the min and max lengths of FingerVein credentials supported, since there are no
 * attributes that represents those values.
 *
 * @param[in] endpointId ID of the endpoint that contains the door lock.
 * @param[out] minLen the minimal length, in bytes, of a FingerVein credential supported by the lock.
 * @param[out] maxLen the minimal length, in bytes, of a FingerVein credential supported by the lock.
 *
 * @return false on failure, true on success.
 */
bool emberAfPluginDoorLockGetFingerVeinCredentialLengthConstraints(chip::EndpointId endpointId, uint8_t & minLen, uint8_t & maxLen);

/**
 * @brief This callback is called when the Door Lock server needs to find out
 * the min and max lengths of Face credentials supported, since there are no
 * attributes that represents those values.
 *
 * @param[in] endpointId ID of the endpoint that contains the door lock.
 * @param[out] minLen the minimal length, in bytes, of a Face credential supported by the lock.
 * @param[out] maxLen the minimal length, in bytes, of a Face credential supported by the lock.
 *
 * @return false on failure, true on success.
 */
bool emberAfPluginDoorLockGetFaceCredentialLengthConstraints(chip::EndpointId endpointId, uint8_t & minLen, uint8_t & maxLen);
