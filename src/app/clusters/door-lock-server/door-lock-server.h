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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/config.h>

#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

using chip::Optional;
using chip::app::Clusters::DoorLock::DlAlarmCode;
using chip::app::Clusters::DoorLock::DlCredentialRule;
using chip::app::Clusters::DoorLock::DlCredentialType;
using chip::app::Clusters::DoorLock::DlDataOperationType;
using chip::app::Clusters::DoorLock::DlDaysMaskMap;
using chip::app::Clusters::DoorLock::DlDoorState;
using chip::app::Clusters::DoorLock::DlLockDataType;
using chip::app::Clusters::DoorLock::DlLockOperationType;
using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::DlOperatingMode;
using chip::app::Clusters::DoorLock::DlOperationError;
using chip::app::Clusters::DoorLock::DlOperationSource;
using chip::app::Clusters::DoorLock::DlStatus;
using chip::app::Clusters::DoorLock::DlUserStatus;
using chip::app::Clusters::DoorLock::DlUserType;
using chip::app::Clusters::DoorLock::DoorLockFeature;
using chip::app::DataModel::List;
using chip::app::DataModel::Nullable;

using LockOpCredentials = chip::app::Clusters::DoorLock::Structs::DlCredential::Type;

typedef bool (*RemoteLockOpHandler)(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pinCode, DlOperationError & err);

static constexpr size_t DOOR_LOCK_MAX_USER_NAME_SIZE = 10; /**< Maximum size of the user name (in characters). */
static constexpr size_t DOOR_LOCK_USER_NAME_BUFFER_SIZE =
    DOOR_LOCK_MAX_USER_NAME_SIZE + 1; /**< Maximum size of the user name string (in bytes). */

struct EmberAfPluginDoorLockCredentialInfo;
struct EmberAfPluginDoorLockUserInfo;

struct EmberAfDoorLockEndpointContext
{
    chip::System::Clock::Timestamp lockoutEndTimestamp;
    int wrongCodeEntryAttempts;
};

/**
 * @brief Door Lock Server Plugin class.
 */
class DoorLockServer
{
public:
    static DoorLockServer & Instance();

    void InitServer(chip::EndpointId endpointId);

    /**
     * Updates the LockState attribute with new value and sends LockOperation event.
     *
     * @note Does not send an event of opSource is kRemote.
     *
     * @param endpointId ID of the endpoint to the lock state
     * @param newLockState new lock state
     * @param opSource source of the operation (will be used in the event).
     *
     * @return true on success, false on failure.
     */
    bool SetLockState(chip::EndpointId endpointId, DlLockState newLockState, DlOperationSource opSource);

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
    bool SetDoorState(chip::EndpointId endpointId, DlDoorState newDoorState);

    bool SetLanguage(chip::EndpointId endpointId, chip::CharSpan newLanguage);
    bool SetAutoRelockTime(chip::EndpointId endpointId, uint32_t newAutoRelockTimeSec);
    bool SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume);

    bool SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled);
    bool SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled);

    bool TrackWrongCodeEntry(chip::EndpointId endpointId);

    bool GetAutoRelockTime(chip::EndpointId endpointId, uint32_t & autoRelockTime);
    bool GetNumberOfUserSupported(chip::EndpointId endpointId, uint16_t & numberOfUsersSupported);
    bool GetNumberOfPINCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfPINCredentials);
    bool GetNumberOfRFIDCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfRFIDCredentials);
    bool GetNumberOfWeekDaySchedulesPerUserSupported(chip::EndpointId endpointId, uint8_t & numberOfWeekDaySchedulesPerUser);
    bool GetNumberOfYearDaySchedulesPerUserSupported(chip::EndpointId endpointId, uint8_t & numberOfYearDaySchedulesPerUser);
    bool GetNumberOfCredentialsSupportedPerUser(chip::EndpointId endpointId, uint8_t & numberOfCredentialsSupportedPerUser);
    bool GetNumberOfHolidaySchedulesSupported(chip::EndpointId endpointId, uint8_t & numberOfHolidaySchedules);

    bool SendLockAlarmEvent(chip::EndpointId endpointId, DlAlarmCode alarmCode);

    chip::BitFlags<DoorLockFeature> GetFeatures(chip::EndpointId endpointId);

    inline bool SupportsPIN(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(DoorLockFeature::kPINCredentials); }

    inline bool SupportsRFID(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(DoorLockFeature::kRFIDCredentials); }

    inline bool SupportsFingers(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(DoorLockFeature::kFingerCredentials);
    }

    inline bool SupportsFace(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(DoorLockFeature::kFaceCredentials); }

    inline bool SupportsWeekDaySchedules(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(DoorLockFeature::kWeekDaySchedules);
    }

    inline bool SupportsYearDaySchedules(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(DoorLockFeature::kYearDaySchedules);
    }

    inline bool SupportsHolidaySchedules(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(DoorLockFeature::kHolidaySchedules);
    }

    inline bool SupportsAnyCredential(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId)
            .HasAny(DoorLockFeature::kPINCredentials, DoorLockFeature::kRFIDCredentials, DoorLockFeature::kFingerCredentials,
                    DoorLockFeature::kFaceCredentials);
    }

    inline bool SupportsCredentialsOTA(chip::EndpointId endpointId)
    {
        return GetFeatures(endpointId).Has(DoorLockFeature::kCredentialsOTA);
    }

    inline bool SupportsUSR(chip::EndpointId endpointId)
    {
        // appclusters, 5.2.2: USR feature has conformance [PIN | RID | FGP | FACE]
        return GetFeatures(endpointId).Has(DoorLockFeature::kUsersManagement) && SupportsAnyCredential(endpointId);
    }

    bool OnFabricRemoved(chip::EndpointId endpointId, chip::FabricIndex fabricIndex);

private:
    chip::FabricIndex getFabricIndex(const chip::app::CommandHandler * commandObj);
    chip::NodeId getNodeId(const chip::app::CommandHandler * commandObj);

    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex);
    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUser);
    bool userExists(chip::EndpointId endpointId, uint16_t userIndex);

    bool credentialIndexValid(chip::EndpointId endpointId, DlCredentialType type, uint16_t credentialIndex);
    bool credentialIndexValid(chip::EndpointId endpointId, DlCredentialType type, uint16_t credentialIndex,
                              uint16_t & maxNumberOfCredentials);
    DlStatus credentialLengthWithinRange(chip::EndpointId endpointId, DlCredentialType type, const chip::ByteSpan & credentialData);
    bool getMaxNumberOfCredentials(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t & maxNumberOfCredentials);

    bool findOccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex);

    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex);
    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex);

    bool findOccupiedCredentialSlot(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t startIndex,
                                    uint16_t & credentialIndex);
    bool findUnoccupiedCredentialSlot(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t startIndex,
                                      uint16_t & credentialIndex);

    bool findUserIndexByCredential(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t credentialIndex,
                                   uint16_t & userIndex);

    bool findUserIndexByCredential(chip::EndpointId endpointId, DlCredentialType credentialType, chip::ByteSpan credentialData,
                                   uint16_t & userIndex, uint16_t & credentialIndex, EmberAfPluginDoorLockUserInfo & userInfo);

    EmberAfStatus createUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId,
                             uint16_t userIndex, const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                             const Nullable<DlUserStatus> & userStatus, const Nullable<DlUserType> & userType,
                             const Nullable<DlCredentialRule> & credentialRule,
                             const Nullable<DlCredential> & credential = Nullable<DlCredential>());
    EmberAfStatus modifyUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId,
                             uint16_t userIndex, const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                             const Nullable<DlUserStatus> & userStatus, const Nullable<DlUserType> & userType,
                             const Nullable<DlCredentialRule> & credentialRule);
    EmberAfStatus clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId, chip::NodeId sourceNodeId,
                            uint16_t userIndex, bool sendUserChangeEvent);
    EmberAfStatus clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId, chip::NodeId sourceNodeId,
                            uint16_t userIndex, const EmberAfPluginDoorLockUserInfo & user, bool sendUserChangeEvent);

    bool clearFabricFromUsers(chip::EndpointId endpointId, chip::FabricIndex fabricIndex);

    DlStatus createNewCredentialAndUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId,
                                        const Nullable<DlUserStatus> & userStatus, const Nullable<DlUserType> & userType,
                                        const DlCredential & credential, const chip::ByteSpan & credentialData,
                                        uint16_t & createdUserIndex);
    DlStatus createNewCredentialAndAddItToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                               const DlCredential & credential, const chip::ByteSpan & credentialData);

    DlStatus addCredentialToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                 const DlCredential & credential);
    DlStatus modifyCredentialForUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                     const DlCredential & credential);

    DlStatus createCredential(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId,
                              uint16_t credentialIndex, DlCredentialType credentialType,
                              const EmberAfPluginDoorLockCredentialInfo & existingCredential, const chip::ByteSpan & credentialData,
                              Nullable<uint16_t> userIndex, const Nullable<DlUserStatus> & userStatus,
                              Nullable<DlUserType> userType, uint16_t & createdUserIndex);
    DlStatus modifyProgrammingPIN(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId,
                                  uint16_t credentialIndex, DlCredentialType credentialType,
                                  const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                  const chip::ByteSpan & credentialData);
    DlStatus modifyCredential(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId,
                              uint16_t credentialIndex, DlCredentialType credentialType,
                              const EmberAfPluginDoorLockCredentialInfo & existingCredential, const chip::ByteSpan & credentialData,
                              uint16_t userIndex, const Nullable<DlUserStatus> & userStatus, Nullable<DlUserType> userType);

    EmberAfStatus clearCredential(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                  DlCredentialType credentialType, uint16_t credentialIndex, bool sendUserChangeEvent);
    EmberAfStatus clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId);
    EmberAfStatus clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                   DlCredentialType credentialType);

    bool clearFabricFromCredentials(chip::EndpointId endpointId, DlCredentialType credentialType, chip::FabricIndex fabricToRemove);
    bool clearFabricFromCredentials(chip::EndpointId endpointId, chip::FabricIndex fabricToRemove);

    void sendSetCredentialResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                   DlStatus status, uint16_t userIndex, uint16_t nextCredentialIndex);

    // TODO: Maybe use CHIP_APPLICATION_ERROR instead of boolean in class methods?
    // OPTIMIZE: there are a lot of methods such as this that could be made static which could help reduce the stack footprint
    // in case of multiple lock endpoints
    bool credentialTypeSupported(chip::EndpointId endpointId, DlCredentialType type);

    bool weekDayIndexValid(chip::EndpointId endpointId, uint8_t weekDayIndex);

    DlStatus clearWeekDaySchedule(chip::EndpointId endpointId, uint16_t userIndex, uint8_t weekDayIndex);
    DlStatus clearWeekDaySchedules(chip::EndpointId endpointId, uint16_t userIndex);
    DlStatus clearSchedules(chip::EndpointId endpointId, uint16_t userIndex);

    void sendGetWeekDayScheduleResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                        uint8_t weekdayIndex, uint16_t userIndex, DlStatus status,
                                        DlDaysMaskMap daysMask = DlDaysMaskMap(0), uint8_t startHour = 0, uint8_t startMinute = 0,
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
                                     DlOperatingMode operatingMode = DlOperatingMode::kNormal);

    bool sendRemoteLockUserChange(chip::EndpointId endpointId, DlLockDataType dataType, DlDataOperationType operation,
                                  chip::NodeId nodeId, chip::FabricIndex fabricIndex, uint16_t userIndex = 0,
                                  uint16_t dataIndex = 0);

    DlLockDataType credentialTypeToLockDataType(DlCredentialType credentialType);

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
                                           const chip::app::ConcreteCommandPath & commandPath, DlCredentialType credentialType,
                                           uint16_t credentialIndex);

    void sendGetCredentialResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                   DlCredentialType credentialType, uint16_t credentialIndex, uint16_t userIndexWithCredential,
                                   EmberAfPluginDoorLockCredentialInfo * credentialInfo, bool credentialExists);

    void clearCredentialCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                       const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData);

    void setWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                          uint16_t userIndex, const chip::BitMask<DlDaysMaskMap> & daysMask, uint8_t startHour,
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
                                          uint32_t localStartTime, uint32_t localEndTime, DlOperatingMode operatingMode);

    void getHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex);

    void clearHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex);

    bool RemoteOperationEnabled(chip::EndpointId endpointId) const;

    EmberAfDoorLockEndpointContext * getContext(chip::EndpointId endpointId);

    bool engageLockout(chip::EndpointId endpointId);

    static CHIP_ERROR sendClusterResponse(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath, EmberAfStatus status);

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
                                   DlLockOperationType opType, RemoteLockOpHandler opHandler,
                                   const chip::Optional<chip::ByteSpan> & pinCode);

    /**
     * @brief Send LockOperation event if opSuccess is true, otherwise send LockOperationError with given opErr code
     *
     * @param endpointId    endpoint where DoorLockServer is running
     * @param opType        lock operation type (lock, unlock, etc)
     * @param opSource      operation source (remote, keypad, auto, etc)
     * @param opErr         operation error code (if opSuccess == false)
     * @param userId        user id
     * @param fabricIdx     fabric index
     * @param nodeId        node id
     * @param credList      list of credentials used in lock operation (can be NULL if no credentials were used)
     * @param credListSize  size of credentials list (if 0, then no credentials were used)
     * @param opSuccess     flags if operation was successful or not
     */
    void SendLockOperationEvent(chip::EndpointId endpointId, DlLockOperationType opType, DlOperationSource opSource,
                                DlOperationError opErr, const Nullable<uint16_t> & userId,
                                const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                                LockOpCredentials * credList, size_t credListSize, bool opSuccess = true);

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
    bool GetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                      EmberAfStatus (*getFn)(chip::EndpointId endpointId, T * value), T & value) const;

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
                      EmberAfStatus (*setFn)(chip::EndpointId endpointId, T value), T value);

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

    friend void emberAfPluginDoorLockOnAutoRelock(chip::EndpointId endpointId);

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

    EmberEventControl AutolockEvent; /**< for automatic relock scheduling */

    std::array<EmberAfDoorLockEndpointContext, EMBER_AF_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT> mEndpointCtx;

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
    DlCredentialStatus status;       /**< Indicates if credential slot is occupied or not. */
    DlCredentialType credentialType; /**< Specifies the type of the credential (PIN, RFID, etc.). */
    chip::ByteSpan credentialData;   /**< Credential data bytes. */

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
    chip::CharSpan userName;                    /**< Name of the user. */
    chip::Span<const DlCredential> credentials; /**< Credentials that are associated with user (without data).*/
    uint32_t userUniqueId;                      /**< Unique user identifier. */
    DlUserStatus userStatus;                    /**< Status of the user slot (available/occupied). */
    DlUserType userType;                        /**< Type of the user. */
    DlCredentialRule credentialRule;            /**< Number of supported credentials. */

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
    DlDaysMaskMap daysMask; /** Indicates the days of the week the Week Day schedule applies for. */
    uint8_t startHour;      /** Starting hour for the Week Day schedule. */
    uint8_t startMinute;    /** Starting minute for the Week Day schedule. */
    uint8_t endHour;        /** Ending hour for the Week Day schedule. */
    uint8_t endMinute;      /** Ending minute for the Week Day schedule. */
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
    DlOperatingMode operatingMode; /** Operating mode during the schedule. */
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
                                          DlScheduleStatus status, DlDaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
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
                                          uint32_t localStartTime, uint32_t localEndTime, DlOperatingMode operatingMode);

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
 * @param   pinCode         PIN code (optional)
 * @param   err             error code if door locking failed (set only if retval==false)
 *
 * @retval true on success
 * @retval false if error happenned (err should be set to appropriate error code)
 */
bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pinCode,
                                            DlOperationError & err);

/**
 * @brief User handler for UnlockDoor command (server)
 *
 * @param   endpointId      endpoint for which UnlockDoor command is called
 * @param   pinCode         PIN code (optional)
 * @param   err             error code if door unlocking failed (set only if retval==false)
 *
 * @retval true on success
 * @retval false if error happenned (err should be set to appropriate error code)
 */
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pinCode,
                                              DlOperationError & err);

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
                                  DlUserStatus userStatus, DlUserType usertype, DlCredentialRule credentialRule,
                                  const DlCredential * credentials, size_t totalCredentials);

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
bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
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
                                        DlCredentialType credentialType, const chip::ByteSpan & credentialData);

/**
 * @brief This callback is called when the Door Lock server starts the lockout so the app could be notified about it.
 *
 * @param endpointId ID of the endpoint that contains the door lock to be locked out.
 * @param lockoutEndTime Monotonic time of when lockout ends.
 */
void emberAfPluginDoorLockLockoutStarted(chip::EndpointId endpointId, chip::System::Clock::Timestamp lockoutEndTime);
