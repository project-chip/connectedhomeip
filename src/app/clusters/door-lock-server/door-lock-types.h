/****************************************************************************
 * @file
 * @brief Door Lock server types shared with the application data model.
 *
 * Types used by the DoorLock::Delegate for user / credential / schedule
 * storage. These were previously part of the ember door-lock-server.h; they
 * carry no ember dependency on their own.
 *******************************************************************************/

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <clusters/DoorLock/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <algorithm>

/**
 * Endpoint id used by the lock-app command delegates / hardware integrations.
 */
#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

/**
 * Structs can be built to only use local (stack / member) buffers instead of
 * external ones; in that configuration the spans inside the structs re-bind on
 * copy (deep-copy semantics see below). Normally disabled.
 */
#ifndef DOOR_LOCK_USE_LOCAL_BUFFER
#define DOOR_LOCK_USE_LOCAL_BUFFER 0
#endif

// Local-buffer sizes, only used when DOOR_LOCK_USE_LOCAL_BUFFER is enabled;
// applications providing their own local buffer configuration (e.g. the silabs
// lock app) define these in their CHIPProjectConfig.h.
#ifndef DOOR_LOCK_CREDENTIAL_BUFFER_LENGTH
#define DOOR_LOCK_CREDENTIAL_BUFFER_LENGTH 20
#endif

#ifndef DOOR_LOCK_USER_CREDENTIALS_BUFFER_LENGTH
#define DOOR_LOCK_USER_CREDENTIALS_BUFFER_LENGTH 5
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

static constexpr size_t DOOR_LOCK_MAX_USER_NAME_SIZE = 10; /**< Maximum size of the user name (in characters). */
static constexpr size_t DOOR_LOCK_USER_NAME_BUFFER_SIZE =
    DOOR_LOCK_MAX_USER_NAME_SIZE + 1; /**< Maximum size of the user name string (in bytes). */

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
struct EmberAfPluginDoorLockCredentialInfo
{
    DlCredentialStatus status = DlCredentialStatus::kAvailable; /**< Indicates if credential slot is occupied or not. */
    CredentialTypeEnum credentialType;                          /**< Specifies the type of the credential (PIN, RFID, etc.). */
#if DOOR_LOCK_USE_LOCAL_BUFFER
    chip::MutableByteSpan credentialData; /**< Credential data bytes. */
#else
    chip::ByteSpan credentialData;                  /**< Credential data bytes. */
#endif
    DlAssetSource creationSource;
    chip::FabricIndex createdBy; /**< Index of the fabric that created the user. */

    DlAssetSource modificationSource;
    chip::FabricIndex lastModifiedBy; /**< ID of the fabric that modified the user. */

#if DOOR_LOCK_USE_LOCAL_BUFFER
    uint8_t credentialDataBuffer[DOOR_LOCK_CREDENTIAL_BUFFER_LENGTH];
    EmberAfPluginDoorLockCredentialInfo() { credentialData = chip::MutableByteSpan(credentialDataBuffer); }

    // credentialData is a span into credentialDataBuffer, so the implicitly-defined copy would leave a
    // copy's span pointing into the source's buffer (dangling once the source is gone). Deep-copy the
    // bytes and re-bind the span to this object's own buffer.
    EmberAfPluginDoorLockCredentialInfo(const EmberAfPluginDoorLockCredentialInfo & other) { *this = other; }

    EmberAfPluginDoorLockCredentialInfo & operator=(const EmberAfPluginDoorLockCredentialInfo & other)
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

        credentialData = chip::MutableByteSpan(credentialDataBuffer);
        SuccessOrDie(CopySpanToMutableSpan(other.credentialData, credentialData));
        return *this;
    }
#endif
};

/**
 * @brief Structure that holds user information.
 */
struct EmberAfPluginDoorLockUserInfo
{
#if DOOR_LOCK_USE_LOCAL_BUFFER
    chip::MutableCharSpan userName;           /**< Name of the user. */
    chip::Span<CredentialStruct> credentials; /**< Credentials that are associated with user (without data).*/
#else
    chip::CharSpan userName;                        /**< Name of the user. */
    chip::Span<const CredentialStruct> credentials; /**< Credentials that are associated with user (without data).*/
#endif
    uint32_t userUniqueId;                                  /**< Unique user identifier. */
    UserStatusEnum userStatus = UserStatusEnum::kAvailable; /**< Status of the user slot (available/occupied). */
    UserTypeEnum userType;                                  /**< Type of the user. */
    CredentialRuleEnum credentialRule;                      /**< Number of supported credentials. */

    DlAssetSource creationSource;
    chip::FabricIndex createdBy; /**< ID of the fabric that created the user. */
    DlAssetSource modificationSource;
    chip::FabricIndex lastModifiedBy; /**< ID of the fabric that modified the user. */

#if DOOR_LOCK_USE_LOCAL_BUFFER
    char nameBuffer[DOOR_LOCK_MAX_USER_NAME_SIZE];
    CredentialStruct credentialsBuffer[DOOR_LOCK_USER_CREDENTIALS_BUFFER_LENGTH];
    EmberAfPluginDoorLockUserInfo()
    {
        userName    = chip::MutableCharSpan(nameBuffer);
        credentials = chip::Span<CredentialStruct>(credentialsBuffer);
    }

    // userName and credentials are spans into nameBuffer/credentialsBuffer, so the implicitly-defined
    // copy would leave a copy's spans pointing into the source's buffers (dangling once the source is
    // gone). Deep-copy the contents and re-bind the spans to this object's own buffers.
    EmberAfPluginDoorLockUserInfo(const EmberAfPluginDoorLockUserInfo & other) { *this = other; }

    EmberAfPluginDoorLockUserInfo & operator=(const EmberAfPluginDoorLockUserInfo & other)
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

        userName = chip::MutableCharSpan(nameBuffer);
        SuccessOrDie(CopyCharSpanToMutableCharSpan(other.userName, userName));

        size_t credentialCount = std::min(other.credentials.size(), sizeof(credentialsBuffer) / sizeof(credentialsBuffer[0]));
        for (size_t i = 0; i < credentialCount; i++)
        {
            credentialsBuffer[i] = other.credentials[i];
        }
        credentials = chip::Span<CredentialStruct>(credentialsBuffer, credentialCount);
        return *this;
    }
#endif
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
