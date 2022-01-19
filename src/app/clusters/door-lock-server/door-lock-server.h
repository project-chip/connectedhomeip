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

using chip::app::Clusters::DoorLock::DlCredentialRule;
using chip::app::Clusters::DoorLock::DlCredentialType;
using chip::app::Clusters::DoorLock::DlDataOperationType;
using chip::app::Clusters::DoorLock::DlDoorState;
using chip::app::Clusters::DoorLock::DlLockDataType;
using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::DlOperationSource;
using chip::app::Clusters::DoorLock::DlStatus;
using chip::app::Clusters::DoorLock::DlUserStatus;
using chip::app::Clusters::DoorLock::DlUserType;
using chip::app::Clusters::DoorLock::DoorLockFeature;
using chip::app::DataModel::Nullable;

static constexpr size_t DOOR_LOCK_MAX_USER_NAME_SIZE = 10; /**< Maximum size of the user name (in characters). */
static constexpr size_t DOOR_LOCK_USER_NAME_BUFFER_SIZE =
    DOOR_LOCK_MAX_USER_NAME_SIZE + 1; /**< Maximum size of the user name string (in bytes). */

static constexpr size_t DOOR_LOCK_MAX_CREDENTIALS_PER_USER = 5; /**< Maximum number of supported credentials by a single user. */

/**
 * @brief Door Lock Server Plugin class.
 */
class DoorLockServer
{
public:
    static DoorLockServer & Instance();

    void InitServer(chip::EndpointId endpointId);

    bool SetLockState(chip::EndpointId endpointId, DlLockState newLockState);
    bool SetActuatorEnabled(chip::EndpointId endpointId, bool newActuatorState);
    bool SetDoorState(chip::EndpointId endpointId, DlDoorState newDoorState);

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

    bool HasFeature(chip::EndpointId endpointId, DoorLockFeature feature);

    inline bool SupportsPIN(chip::EndpointId endpointId) { return HasFeature(endpointId, DoorLockFeature::kPINCredentials); }

    inline bool SupportsPFID(chip::EndpointId endpointId) { return HasFeature(endpointId, DoorLockFeature::kRFIDCredentials); }

    inline bool SupportsFingers(chip::EndpointId endpointId) { return HasFeature(endpointId, DoorLockFeature::kFingerCredentials); }

    inline bool SupportsFace(chip::EndpointId endpointId) { return HasFeature(endpointId, DoorLockFeature::kFaceCredentials); }

    inline bool SupportsUSR(chip::EndpointId endpointId)
    {
        // appclusters, 5.2.2: USR feature has conformance [PIN | RID | FGP | FACE]
        // TODO: Add missing functions to check if RID, FGP or FACE are supported
        return HasFeature(endpointId, DoorLockFeature::kUsersManagement) && SupportsPIN(endpointId);
    }

private:
    chip::FabricIndex getFabricIndex(const chip::app::CommandHandler * commandObj);
    chip::NodeId getNodeId(const chip::app::CommandHandler * commandObj);

    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex);
    bool userIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUser);

    bool credentialIndexValid(chip::EndpointId endpointId, DlCredentialType type, uint16_t credentialIndex);
    bool credentialIndexValid(chip::EndpointId endpointId, DlCredentialType type, uint16_t credentialIndex,
                              uint16_t & maxNumberOfCredentials);
    bool getCredentialRange(chip::EndpointId endpointId, DlCredentialType type, size_t & minSize, size_t & maxSize);
    bool getMaxNumberOfCredentials(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t & maxNumberOfCredentials);

    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex);
    bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex);

    bool findUnoccupiedCredentialSlot(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t startIndex,
                                      uint16_t & credentialIndex);

    bool findUserIndexByCredential(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t credentialIndex,
                                   uint16_t & userIndex);

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

    EmberAfStatus clearCredential(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                  DlCredentialType credentialType, uint16_t credentialIndex, bool sendUserChangeEvent);
    EmberAfStatus clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId);
    EmberAfStatus clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                   DlCredentialType credentialType);

    CHIP_ERROR sendSetCredentialResponse(chip::app::CommandHandler * commandObj, DlStatus status, uint16_t userIndex,
                                         uint16_t nextCredentialIndex);

    // TODO: Maybe use CHIP_APPLICATION_ERROR instead of boolean in class methods?
    bool credentialTypeSupported(chip::EndpointId endpointId, DlCredentialType type);

    bool sendRemoteLockUserChange(chip::EndpointId endpointId, DlLockDataType dataType, DlDataOperationType operation,
                                  chip::NodeId nodeId, chip::FabricIndex fabricIndex, uint16_t userIndex = 0,
                                  uint16_t dataIndex = 0);

    DlLockDataType credentialTypeToLockDataType(DlCredentialType credentialType);

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

/**
 * @brief Structure that holds the credential information.
 */
struct EmberAfPluginDoorLockCredentialInfo
{
    DlCredentialStatus status;       /**< Indicates if credential slot is occupied or not. */
    DlCredentialType credentialType; /**< Specifies the type of the credential (PIN, RFID, etc.). */
    chip::ByteSpan credentialData;   /**< Credential data bytes. */
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
    chip::FabricIndex createdBy;                /**< ID of the fabric that created the user. */
    chip::FabricIndex lastModifiedBy;           /**< ID of the fabric that modified the user. */
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
 *                        particular credential type. Starts from 1.
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
 *                         particular credential type. Starts from 1.
 * @param credentialStatus New status of the credential slot (occupied/available). DlCredentialStatus::kAvailable means that the
 *                         credential must be deleted.
 * @param credentialType Type of the credential (PIN, RFID, etc.).
 * @param[in] credentialData Data attached to a credential. Can contain nullptr as data which indicates that the data for credential
 *                           should be removed.
 *
 * @retval true, if credential pointed by \p credentialIndex of type \p credentialType was successfully changed in the database.
 * @retval false, if error occurred.
 */
bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                        DlCredentialType credentialType, const chip::ByteSpan & credentialData);
