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
#include <app/util/af.h>

#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

#ifndef DOOR_LOCK_SERVER_ENABLE_DEFAULT_USERS_CREDENTIALS_IMPLEMENTATION
#define DOOR_LOCK_SERVER_ENABLE_DEFAULT_USERS_CREDENTIALS_IMPLEMENTATION 1
#endif

using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

static constexpr size_t DOOR_LOCK_MAX_USER_NAME_SIZE    = 10;
static constexpr size_t DOOR_LOCK_USER_NAME_BUFFER_SIZE = DOOR_LOCK_MAX_USER_NAME_SIZE + 1;

static constexpr size_t DOOR_LOCK_MAX_CREDENTIALS_PER_USER = 5;

/**
 * @brief Door Lock Server Plugin class.
 */
class DoorLockServer
{
public:
    static DoorLockServer & Instance();

    void InitServer(chip::EndpointId endpointId);

    bool SetLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState newLockState);
    bool SetActuatorEnabled(chip::EndpointId endpointId, bool newActuatorState);
    bool SetDoorState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlDoorState newDoorState);

    bool SetLanguage(chip::EndpointId endpointId, chip::CharSpan newLanguage);
    bool SetAutoRelockTime(chip::EndpointId endpointId, uint32_t newAutoRelockTimeSec);
    bool SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume);

    bool SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled);
    bool SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled);

    void SetUserCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData);

    void GetUserCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData);

    void ClearUserCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                 const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData);

    void SetCredentialCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                     const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData);

    void GetCredentialStatusCommandHandler(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData);

    void ClearCredentialCommandHandler(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                       const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData);

    EmberAfStatus CreateUser(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             uint16_t userIndex, const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                             const Nullable<DoorLock::DlUserStatus> & userStatus, const Nullable<DoorLock::DlUserType> & userType,
                             const Nullable<DoorLock::DlCredentialRule> & credentialRule,
                             const Nullable<DlCredential> & credentials = Nullable<DlCredential>());

    EmberAfStatus ModifyUser(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             uint16_t userIndex, const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                             const Nullable<DoorLock::DlUserStatus> & userStatus, const Nullable<DoorLock::DlUserType> & userType,
                             const Nullable<DoorLock::DlCredentialRule> & credentialRule);

    bool HasFeature(chip::EndpointId endpointId, DoorLock::DoorLockFeature feature);

    inline bool SupportsPIN(chip::EndpointId endpointId)
    {
        return HasFeature(endpointId, DoorLock::DoorLockFeature::kPINCredentials);
    }

    inline bool SupportsUSR(chip::EndpointId endpointId)
    {
        // appclusters, 5.2.2: USR feature has conformance [PIN | RID | FGP | FACE]
        // TODO: Add missing functions to check if RID, FGP or FACE are supported
        return HasFeature(endpointId, DoorLock::DoorLockFeature::kUsersManagement) && SupportsPIN(endpointId);
    }

private:
    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex);
    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUser);

    chip::FabricIndex getFabricIndex(const chip::app::CommandHandler * commandObj);
    EmberAfStatus clearUser(chip::EndpointId endpointId, uint16_t userIndex);

    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex);
    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex);

    DoorLock::DlStatus createNewCredentialAndUser(chip::EndpointId endpointId);
    DoorLock::DlStatus createNewCredentialAndAddItToUser(chip::EndpointId endpointId);

    CHIP_ERROR sendSetCredentialResponse(chip::app::CommandHandler * commandObj, DoorLock::DlStatus status,
                                         const Nullable<uint16_t> & userIndex, const Nullable<uint16_t> & nextCredentialIndex);

    DoorLock::DlStatus addCredentialToUser(chip::EndpointId endpointId, uint16_t userIndex, const DlCredential & credential);
    DoorLock::DlStatus modifyCredentialForUser(chip::EndpointId endpointId, uint16_t userIndex, const DlCredential & credential);

    static DoorLockServer instance;
};

enum class DlCredentialStatus : uint8_t
{
    kAvailable = 0x00,
    kOccupied  = 0x01,
};

struct EmberAfPluginDoorLockCredentialInfo
{
    DlCredentialStatus status;
    DoorLock::DlCredentialType credentialType;
    chip::ByteSpan credentialData;
};

struct EmberAfPluginDoorLockUserInfo
{
    char userName[DOOR_LOCK_USER_NAME_BUFFER_SIZE];
    DlCredential credentials[DOOR_LOCK_MAX_CREDENTIALS_PER_USER];
    size_t totalCredentials;
    uint32_t userUniqueId;
    DoorLock::DlUserStatus userStatus;
    DoorLock::DlUserType userType;
    DoorLock::DlCredentialRule credentialRule;
    chip::FabricIndex createdBy;
    chip::FabricIndex lastModifiedBy;
};

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode);
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode);
bool emberAfPluginDoorLockOnDoorUnlockWithTimeoutCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode,
                                                         uint16_t timeout);

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
 * @param  attrType        attribute that is going to be changed
 * @param  attrSize        attribute value storage size
 * @param  attrValue       attribute value to set
 *
 * @retval InteractionModel::Status::Success if attribute change is possible
 * @retval any other InteractionModel::Status value to forbid attribute change
 */
chip::Protocols::InteractionModel::Status emberAfPluginDoorLockOnUnhandledAttributeChange(chip::EndpointId EndpointId,
                                                                                          EmberAfAttributeType attrType,
                                                                                          uint16_t attrSize, uint8_t * attrValue);

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);
bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const char * userName, uint32_t uniqueId,
                                  DoorLock::DlUserStatus userStatus, DoorLock::DlUserType usertype,
                                  DoorLock::DlCredentialRule credentialRule, const DlCredential * credentials,
                                  size_t totalCredentials);

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                        EmberAfPluginDoorLockCredentialInfo & credential);
bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                        DoorLock::DlCredentialType credentialType, const chip::ByteSpan & credentialData);
