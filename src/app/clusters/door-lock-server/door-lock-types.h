/*
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

#include <clusters/DoorLock/Enums.h>
#include <clusters/DoorLock/Structs.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <algorithm>

// TODO: The local-buffer build variant couples these library data types to the
// CHIPProjectConfig.h compile arguments of a single application, which means
// every such build has its own struct layout. This is inherited from the
// legacy design; revisit so buffered and unbuffered variants can be separated
// from the "single libCHIP for everyone" types.
/**
 * Structs can be built to only use local (stack / member) buffers instead of
 * external ones; in that configuration the spans inside the structs re-bind on
 * copy (deep-copy semantics, see below). Normally disabled.
 */
#ifndef DOOR_LOCK_USE_LOCAL_BUFFER
#define DOOR_LOCK_USE_LOCAL_BUFFER 0
#endif

// Local-buffer sizes, required when DOOR_LOCK_USE_LOCAL_BUFFER is enabled;
// applications providing their own local buffer configuration (e.g. the silabs
// lock app) define these in their CHIPProjectConfig.h. No defaults on purpose:
// a build that forgets to size its buffers fails at compile time instead of
// aborting at the copy that overflows.

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

/// Maximum size of the user name (in characters).
inline constexpr size_t kDoorLockMaxUsernameSize = 10;
/// Maximum size of the user name string (in bytes).
inline constexpr size_t kDoorLockUsernameBufferSize = kDoorLockMaxUsernameSize + 1;

using CredentialStruct = Structs::CredentialStruct::Type;

/**
 * @brief Status of the credential slot in the credentials database.
 */
enum class DlCredentialStatus : uint8_t
{
    kAvailable = 0x00, /**< Indicates if credential slot is available. */
    kOccupied  = 0x01, /**< Indicates if credential slot is already occupied. */
};

/**
 * @brief Source that created/modified a user or a credential.
 */
enum class DlAssetSource : uint8_t
{
    kUnspecified = 0x00,
    kMatterIM    = 0x01,
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
 * @brief Structure that holds the credential information.
 */
struct CredentialInfo
{
    DlCredentialStatus status = DlCredentialStatus::kAvailable; /**< Indicates if credential slot is occupied or not. */
    CredentialTypeEnum credentialType;                          /**< Specifies the type of the credential (PIN, RFID, etc.). */
#if DOOR_LOCK_USE_LOCAL_BUFFER
    MutableByteSpan credentialData; /**< Credential data bytes. */
#else
    ByteSpan credentialData;                  /**< Credential data bytes. */
#endif
    DlAssetSource creationSource;
    FabricIndex createdBy; /**< Index of the fabric that created the user. */

    DlAssetSource modificationSource;
    FabricIndex lastModifiedBy; /**< ID of the fabric that modified the user. */

#if DOOR_LOCK_USE_LOCAL_BUFFER
    uint8_t credentialDataBuffer[DOOR_LOCK_CREDENTIAL_BUFFER_LENGTH];
    CredentialInfo() { credentialData = MutableByteSpan(credentialDataBuffer); }

    // credentialData is a span into credentialDataBuffer, so the implicitly-defined copy would leave a
    // copy's span pointing into the source's buffer (dangling once the source is gone). Deep-copy the
    // bytes and re-bind the span to this object's own buffer.
    CredentialInfo(const CredentialInfo & other) { *this = other; }

    CredentialInfo & operator=(const CredentialInfo & other)
    {
        if (this == &other)
        {
            return *this;
        }

        status             = other.status;
        credentialType     = other.credentialType;
        creationSource     = other.creationSource;
        createdBy          = other.createdBy;
        modificationSource = other.modificationSource;
        lastModifiedBy     = other.lastModifiedBy;

        credentialData = MutableByteSpan(credentialDataBuffer);
        SuccessOrDie(CopySpanToMutableSpan(other.credentialData, credentialData));
        return *this;
    }
#endif
};

/**
 * @brief Structure that holds user information.
 */
struct UserInfo
{
#if DOOR_LOCK_USE_LOCAL_BUFFER
    MutableCharSpan userName;           /**< Name of the user. */
    Span<CredentialStruct> credentials; /**< Credentials that are associated with user (without data).*/
#else
    CharSpan userName;                        /**< Name of the user. */
    Span<const CredentialStruct> credentials; /**< Credentials that are associated with user (without data).*/
#endif
    uint32_t userUniqueId;                                  /**< Unique user identifier. */
    UserStatusEnum userStatus = UserStatusEnum::kAvailable; /**< Status of the user slot (available/occupied). */
    UserTypeEnum userType;                                  /**< Type of the user. */
    CredentialRuleEnum credentialRule;                      /**< Number of supported credentials. */

    DlAssetSource creationSource;
    FabricIndex createdBy; /**< ID of the fabric that created the user. */
    DlAssetSource modificationSource;
    FabricIndex lastModifiedBy; /**< ID of the fabric that modified the user. */

#if DOOR_LOCK_USE_LOCAL_BUFFER
    char nameBuffer[kDoorLockMaxUsernameSize];
    CredentialStruct credentialsBuffer[DOOR_LOCK_USER_CREDENTIALS_BUFFER_LENGTH];
    UserInfo()
    {
        userName    = MutableCharSpan(nameBuffer);
        credentials = Span<CredentialStruct>(credentialsBuffer);
    }

    // userName and credentials are spans into nameBuffer/credentialsBuffer, so the implicitly-defined
    // copy would leave a copy's spans pointing into the source's buffers (dangling once the source is
    // gone). Deep-copy the contents and re-bind the spans to this object's own buffers.
    UserInfo(const UserInfo & other) { *this = other; }

    UserInfo & operator=(const UserInfo & other)
    {
        if (this == &other)
        {
            return *this;
        }

        userUniqueId       = other.userUniqueId;
        userStatus         = other.userStatus;
        userType           = other.userType;
        credentialRule     = other.credentialRule;
        creationSource     = other.creationSource;
        createdBy          = other.createdBy;
        modificationSource = other.modificationSource;
        lastModifiedBy     = other.lastModifiedBy;

        userName = MutableCharSpan(nameBuffer);
        SuccessOrDie(CopyCharSpanToMutableCharSpan(other.userName, userName));

        size_t credentialCount = std::min(other.credentials.size(), sizeof(credentialsBuffer) / sizeof(credentialsBuffer[0]));
        for (size_t i = 0; i < credentialCount; i++)
        {
            credentialsBuffer[i] = other.credentials[i];
        }
        credentials = Span<CredentialStruct>(credentialsBuffer, credentialCount);
        return *this;
    }
#endif
};

/**
 * @brief Structure that holds week day schedule information.
 */
struct WeekDaySchedule
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
struct YearDaySchedule
{
    uint32_t localStartTime; /** The starting time for the Year Day schedule in Epoch Time in Seconds with local time offset based
                                on the local timezone and DST offset on the day represented by the value. */
    uint32_t localEndTime;   /** The ending time for the Year Day schedule in Epoch Time in Seconds with local time offset based on
                              * the local timezone and DST offset on the day represented by the value. */
};

/**
 * @brief Structure that holds holiday schedule information.
 */
struct HolidaySchedule
{
    uint32_t localStartTime; /** The starting time for the Holiday schedule in Epoch Time in Seconds with local time offset based
                            on the local timezone and DST offset on the day represented by the value. */
    uint32_t localEndTime;   /** The ending time for the Holiday schedule in Epoch Time in Seconds with local time offset based on
                            the local timezone and DST offset on the day represented by the value. */
    OperatingModeEnum operatingMode; /** Operating mode during the schedule. */
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
