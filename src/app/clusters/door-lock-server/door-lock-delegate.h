/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "door-lock-types.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

static constexpr size_t kAliroReaderVerificationKeySize = 65;

static constexpr size_t kAliroReaderGroupIdentifierSize = 16;

static constexpr size_t kAliroReaderGroupSubIdentifierSize = 16;

static constexpr size_t kAliroGroupResolvingKeySize = 16;

static constexpr size_t kAliroProtocolVersionSize = 2;

static constexpr size_t kAliroSigningKeySize = 32;

/** @brief
 *  Defines methods for implementing application-specific logic for the door lock cluster.
 *  It defines the interfaces that a door lock should implement to support Aliro provisioning attributes.
 */

/// Payload of a lock / unlock / unbolt actuation request.
///
/// Bundles the operation initiator (fabric index and node id, both null for
/// local actuations) with the optional PIN credential carried by the command,
/// so the actuation hooks below take a single request argument.
struct LockOperationRequest
{
    DataModel::Nullable<FabricIndex> fabricIdx;
    DataModel::Nullable<NodeId> nodeId;
    Optional<ByteSpan> pinCode;
};

class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * @brief Get the Aliro verification key component of the Reader's key pair.
     *
     * @param[out] verificationKey The MutableByteSpan to copy the verification key into. On success,
     *             the callee must update the length to the length of the copied data. If the value of
     *             the attribute is null (e.g. no reader configuration is provisioned), the callee
     *             must leave the MutableByteSpan empty: the cluster reports an empty span as a null
     *             attribute value.
     */
    virtual CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) = 0;

    /**
     * @brief Get the Aliro Reader's group identifier
     *
     * @param[out] groupIdentifier The MutableByteSpan to copy the group identifier into. On success,
     *             the callee must update the length to the length of the copied data. If the value of
     *             the attribute is null (e.g. no reader configuration is provisioned), the callee
     *             must leave the MutableByteSpan empty: the cluster reports an empty span as a null
     *             attribute value.
     */
    virtual CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) = 0;

    /**
     * @brief Get the Aliro Reader's group subidentifier
     *
     * @param[out] groupSubIdentifier  The MutableByteSpan to copy the group subidentifier into. On success,
     *             the callee must update the length to the length of the copied data. The MutableByteSpan
     *             must not be empty since the attribute is not nullable.
     */
    virtual CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) = 0;

    /**
     * @brief Get the Aliro expedited transaction supported protocol version at the given index.
     *
     * @param[in] index The index of the protocol version in the list.
     * @param[out] protocolVersion The MutableByteSpan to copy the expedited transaction supported protocol version at the given
     *             index into. On success, the callee must update the length to the length of the copied data.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the attribute list.
     */
    virtual CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                                   MutableByteSpan & protocolVersion) = 0;

    /**
     * @brief Get the Reader's group resolving key.
     *
     * @param[out] groupResolvingKey The MutableByteSpan to copy the group resolving key into. On success,
     *             the callee must update the length to the length of the copied data. If the value of
     *             the attribute is null (e.g. no reader configuration is provisioned), the callee
     *             must leave the MutableByteSpan empty: the cluster reports an empty span as a null
     *             attribute value.
     */
    virtual CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) = 0;

    /**
     * @brief Get the Aliro supported BLE UWB protocol version at the given index.
     *
     * @param[in] index The index of the protocol version in the list.
     * @param[out] protocolVersion  The MutableByteSpan to copy the supported BLE UWB protocol version at the given index into.
     *             On success, the callee must update the length to the length of the copied data.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the attribute list.
     */
    virtual CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) = 0;

    /**
     * @brief Get the Aliro BLE Advertising Version.
     *
     * @return The BLE Advertising Version.
     */
    virtual uint8_t GetAliroBLEAdvertisingVersion() = 0;

    /**
     * @brief Get the maximum number of Aliro credential issuer keys supported.
     *
     * @return The max number of Aliro credential issuer keys supported.
     */
    virtual uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() = 0;

    /**
     * @brief Get the maximum number of Aliro endpoint keys supported.
     *
     * @return The max number of Aliro endpoint keys supported.
     */
    virtual uint16_t GetNumberOfAliroEndpointKeysSupported() = 0;

    /**
     * Set the Aliro reader configuration for the lock.  The various arguments
     * have already been checked for constraints and consistency with the
     * FeatureMap.
     *
     * @param[in] signingKey Signing key component of the Reader's key pair.
     * @param[in] verificationKey Verification key component of the Reader's key pair.
     * @param[in] groupIdentifier Reader group identifier for the lock.
     * @param[in] groupResolvingKey Group resolving key for the lock if Aliro BLE UWB feature is supported
     */
    virtual CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                            const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey) = 0;

    /**
     * Clear the Aliro reader configuration for the lock.
     */
    virtual CHIP_ERROR ClearAliroReaderConfig() = 0;

    /**
     * @brief Hardware actuation hook for the LockDoor command.
     *
     * @param[in] request the actuation request: initiator identity and PIN credential
     * @return nullopt on success, otherwise the operation error to report in
     *         the LockOperationError event
     */
    virtual std::optional<OperationErrorEnum> HandleDoorLockCommand(const LockOperationRequest & request)
    {
        return OperationErrorEnum::kUnspecified;
    }

    /**
     * @brief Hardware actuation hook for the UnlockDoor / UnlockWithTimeout commands.
     *
     * @param[in] request the actuation request: initiator identity and PIN credential
     * @return nullopt on success, otherwise the operation error to report in
     *         the LockOperationError event
     */
    virtual std::optional<OperationErrorEnum> HandleDoorUnlockCommand(const LockOperationRequest & request)
    {
        return OperationErrorEnum::kUnspecified;
    }

    /**
     * @brief Hardware actuation hook for the UnboltDoor command.
     *
     * @param[in] request the actuation request: initiator identity and PIN credential
     * @return nullopt on success, otherwise the operation error to report in
     *         the LockOperationError event
     */
    virtual std::optional<OperationErrorEnum> HandleDoorUnboltCommand(const LockOperationRequest & request)
    {
        return OperationErrorEnum::kUnspecified;
    }

    /**
     * @brief Called after the AutoRelock timer expires and the cluster reports
     * the door as locked (legacy `emberAfPluginDoorLockOnAutoRelock`
     * equivalent).
     *
     * The cluster has already updated the LockState attribute (and emitted the
     * lock operation event) when this is called. Matching the legacy contract,
     * the delegate performs the physical relock actuation here.
     */
    virtual void OnAutoRelock() {}

    /**
     * @brief Called when the UserCodeTemporaryDisableTime lockout window starts.
     *
     * @param[in] lockoutEndTimestamp timestamp at which the lockout window ends
     */
    virtual void OnLockoutStarted(chip::System::Clock::Timestamp lockoutEndTimestamp) {}

    // ---- User management ([USR] feature) ---------------------------------------

    /**
     * @brief Read a user record from the application's user database.
     *
     * Must succeed even for unoccupied slots (userStatus == kAvailable), the
     * slot content is only populated for occupied ones.
     *
     * @param[in] userIndex 1-based user index, guaranteed within
     *            NumberOfTotalUsersSupported
     * @param[out] user the user record
     * @retval true if the user index is within the application database
     */
    virtual bool GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) { return false; }

    /**
     * @brief Create, modify or clear a user record.
     *
     * A clear request passes kUndefinedFabricIndex for creator and modifier,
     * an empty name, 0 uniqueId, kAvailable status, kUnrestrictedUser type,
     * kSingle credential rule and no credentials.
     */
    virtual bool SetUser(uint16_t userIndex, FabricIndex creator, FabricIndex modifier, const CharSpan & userName,
                         uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType, CredentialRuleEnum credentialRule,
                         const CredentialStruct * credentials, size_t totalCredentials)
    {
        return false;
    }

    // ---- Credential storage ------------------------------------------------------

    /**
     * @brief Read a credential record. Must succeed even for unoccupied slots;
     * only populated for occupied ones.
     *
     * @param[in] credentialIndex 1-based credential index for PIN/RFID/finger
     *            kinds; 0 for the Programming PIN.
     */
    virtual bool GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
                               EmberAfPluginDoorLockCredentialInfo & credential)
    {
        return false;
    }

    /**
     * @brief Create/modify/clear a credential. A status of kAvailable deletes
     * the credential (credential data may be an empty span then).
     */
    virtual bool SetCredential(uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                               DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                               const ByteSpan & credentialData)
    {
        return false;
    }

    // ---- Capacities and constraints for non-PIN/RFID credential kinds ------------

    virtual bool GetNumberOfFingerprintCredentialsSupported(uint16_t & maxNumberOfCredentials) { return false; }
    virtual bool GetNumberOfFingerVeinCredentialsSupported(uint16_t & maxNumberOfCredentials) { return false; }
    virtual bool GetNumberOfFaceCredentialsSupported(uint16_t & maxNumberOfCredentials) { return false; }

    virtual bool GetFingerprintCredentialLengthConstraints(uint8_t & minLen, uint8_t & maxLen) { return false; }
    virtual bool GetFingerVeinCredentialLengthConstraints(uint8_t & minLen, uint8_t & maxLen) { return false; }
    virtual bool GetFaceCredentialLengthConstraints(uint8_t & minLen, uint8_t & maxLen) { return false; }

    // ---- Schedules -----------------------------------------------------------------

    virtual DlStatus GetSchedule(uint8_t weekDayIndex, uint16_t userIndex, EmberAfPluginDoorLockWeekDaySchedule & schedule)
    {
        return DlStatus::kFailure;
    }
    virtual DlStatus SetSchedule(uint8_t weekDayIndex, uint16_t userIndex, DlScheduleStatus status, DaysMaskMap daysMask,
                                 uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
    {
        return DlStatus::kFailure;
    }
    virtual DlStatus GetSchedule(uint8_t yearDayIndex, uint16_t userIndex, EmberAfPluginDoorLockYearDaySchedule & schedule)
    {
        return DlStatus::kFailure;
    }
    virtual DlStatus SetSchedule(uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status, uint32_t localStartTime,
                                 uint32_t localEndTime)
    {
        return DlStatus::kFailure;
    }
    virtual DlStatus GetSchedule(uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule & schedule)
    {
        return DlStatus::kFailure;
    }
    virtual DlStatus SetSchedule(uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime,
                                 OperatingModeEnum operatingMode)
    {
        return DlStatus::kFailure;
    }

    /**
     * @brief Called after the cluster has unstamped the fabric from all users
     * and credentials of this endpoint, so the application can purge
     * app-local per-fabric state.
     */
    virtual void OnFabricRemoved(FabricIndex fabricIndex) {}
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
