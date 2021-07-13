/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file defines error constants for the CHIP core
 *      subsystem.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate CHIP_CONFIG_* or _CHIP_CONFIG_*
 *      macros.
 */

#pragma once

#include <core/CHIPConfig.h>
#include <support/TypeTraits.h>

#include <inttypes.h>
#include <stdint.h>
#include <type_traits>

namespace chip {

/**
 * This is a helper class for managing `CHIP_ERROR` numbers.
 *
 * At the top level, an error belongs to a `Range` and has an integral Value whose meaning depends on the `Range`.
 * One, `Range::kSDK`, is used for the CHIP SDK's own errors; others encapsulate error codes from external sources
 * (e.g. libraries, OS) into a `CHIP_ERROR`.
 *
 * CHIP SDK errors inside `Range::kSDK` consist of a component identifier given by `SdkPart` and an arbitrary small
 * integer Code.
 */
class ChipError
{
public:
    using BaseType = uint32_t;

    /// `printf` format for error numbers. This is a C macro in order to allow for string literal concatenation.
#define CHIP_ERROR_FORMAT PRIx32

    /// Top-level error classification.
    enum class Range : uint8_t
    {
        kSDK        = 0x0, ///< CHIP SDK errors.
        kOS         = 0x1, ///< Encapsulated OS errors, other than POSIX errno.
        kPOSIX      = 0x2, ///< Encapsulated POSIX errno values.
        kLwIP       = 0x3, ///< Encapsulated LwIP errors.
        kOpenThread = 0x4, ///< Encapsulated OpenThread errors.
        kPlatform   = 0x5, ///< Platform-defined encapsulation.
    };

    /// Secondary classification of errors in `Range::kSDK`.
    enum class SdkPart : uint8_t
    {
        kCore        = 0, ///< SDK core errors.
        kInet        = 1, ///< Inet layer errors; see <inet/InetError.h>.
        kDevice      = 2, ///< Device layer errors; see <platform/CHIPDeviceError.h>.
        kASN1        = 3, ///< ASN1 errors; see <asn1/ASN1Error.h>.
        kBLE         = 4, ///< BLE layer errors; see <ble/BleError.h>.
        kApplication = 7, ///< Application-defined errors.
    };

    /// Test whether @a error belongs to @a range.
    static constexpr bool IsRange(Range range, BaseType error)
    {
        return (error & MakeMask(kRangeStart, kRangeLength)) == MakeField(kRangeStart, static_cast<BaseType>(range));
    }

    static constexpr Range GetRange(BaseType error) { return static_cast<Range>(GetField(kRangeStart, kRangeLength, error)); }
    static BaseType GetValue(BaseType error) { return GetField(kValueStart, kValueLength, error); }

    /// Test whether if @a value can be losslessly encapsulated in a `CHIP_ERROR`.
    static constexpr bool CanEncapsulate(BaseType value) { return FitsInField(kValueLength, value); }

    /// Construct a `CHIP_ERROR` encapsulating @a value inside the @a range.
    static BaseType Encapsulate(Range range, BaseType value) { return MakeInteger(range, (value & MakeMask(0, kValueLength))); }

    /// Test whether @a error is an SDK error belonging to @a part.
    static constexpr bool IsPart(SdkPart part, BaseType error)
    {
        return (error & (MakeMask(kRangeStart, kRangeLength) | MakeMask(kSdkPartStart, kSdkPartLength))) ==
            (MakeField(kRangeStart, static_cast<BaseType>(Range::kSDK)) | MakeField(kSdkPartStart, static_cast<BaseType>(part)));
    }

private:
    /*
     * The representation of a CHIP_ERROR is structured so that SDK error code constants are small, in order to improve code
     * density on embedded builds. Arm 32, Xtensa, and RISC-V can all handle 11-bit values in a move-immediate instruction.
     * Further, `SdkPart::kCore` is 0 so that the most common errors fit in 8 bits for additional density on some processors.
     *
     *  31    28      24      20      16      12       8       4       0    Bit
     *  |       |       |       |       |       |       |       |       |
     *  |     range     |                     value                     |
     *  |    kSdk==0    |       0               |0| part|    code       |   SDK error
     *  |    01 - FF    |          encapsulated error code              |   Encapsulated error
     */
    static constexpr int kRangeStart  = 24;
    static constexpr int kRangeLength = 8;
    static constexpr int kValueStart  = 0;
    static constexpr int kValueLength = 24;

    static constexpr int kSdkPartStart  = 8;
    static constexpr int kSdkPartLength = 3;
    static constexpr int kSdkCodeStart  = 0;
    static constexpr int kSdkCodeLength = 8;

    static constexpr BaseType GetField(unsigned int start, unsigned int length, BaseType value)
    {
        return (value >> start) & ((1u << length) - 1);
    }
    static constexpr BaseType MakeMask(unsigned int start, unsigned int length) { return ((1u << length) - 1) << start; }
    static constexpr BaseType MakeField(unsigned int start, BaseType value) { return value << start; }
    static constexpr bool FitsInField(unsigned int length, BaseType value) { return value < (1u << length); }

    static constexpr BaseType MakeInteger(Range range, BaseType value)
    {
        return MakeField(kRangeStart, to_underlying(range)) | MakeField(kValueStart, value);
    }
    static constexpr BaseType MakeInteger(SdkPart part, BaseType code)
    {
        return MakeInteger(Range::kSDK, MakeField(kSdkPartStart, to_underlying(part)) | MakeField(kSdkCodeStart, code));
    }

public:
    /*
     * Wrapper for constructing error constants. This is a C macro so that it can easily be augmented to track
     * error source line information on large platforms without touching users.
     *
     * The underlying template ensures that the numeric value is constant and well-formed.
     * (In C++20 this could be replaced by a consteval function.)
     */
#define CHIP_SDK_ERROR(part, code) (::chip::ChipError::MakeSdkErrorConstant<(part), (code)>::value)
    template <SdkPart part, BaseType code>
    struct MakeSdkErrorConstant
    {
        static_assert(FitsInField(kSdkPartLength, to_underlying(part)), "part is too large");
        static_assert(FitsInField(kSdkCodeLength, code), "code is too large");
        static_assert(MakeInteger(part, code) != 0, "value is zero");
        static constexpr BaseType value = MakeInteger(part, code);
    };
};

} // namespace chip

/**
 *  The basic type for all CHIP errors.
 */
using CHIP_ERROR = ::chip::ChipError::BaseType;

/**
 * Applications using the CHIP SDK can use this to define error codes in the `CHIP_ERROR` space for their own purposes.
 */
#define CHIP_APPLICATION_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kApplication, (e))

#define CHIP_CORE_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kCore, (e))

// clang-format off

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def CHIP_NO_ERROR
 *
 *  @brief
 *    This defines the CHIP error code for success or no error.
 *
 */
#define CHIP_NO_ERROR                                          (0)

/**
 *  @def CHIP_ERROR_SENDING_BLOCKED
 *
 *  @brief
 *    A message exceeds the sent limit.
 *
 */
#define CHIP_ERROR_SENDING_BLOCKED                             CHIP_CORE_ERROR(0x01)

/**
 *  @def CHIP_ERROR_CONNECTION_ABORTED
 *
 *  @brief
 *    A connection has been aborted.
 *
 */
#define CHIP_ERROR_CONNECTION_ABORTED                          CHIP_CORE_ERROR(0x02)

/**
 *  @def CHIP_ERROR_INCORRECT_STATE
 *
 *  @brief
 *    An unexpected state was encountered.
 *
 */
#define CHIP_ERROR_INCORRECT_STATE                             CHIP_CORE_ERROR(0x03)

/**
 *  @def CHIP_ERROR_MESSAGE_TOO_LONG
 *
 *  @brief
 *    A message is too long.
 *
 */
#define CHIP_ERROR_MESSAGE_TOO_LONG                            CHIP_CORE_ERROR(0x04)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_EXCHANGE_VERSION
 *
 *  @brief
 *    An exchange version is not supported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_EXCHANGE_VERSION                CHIP_CORE_ERROR(0x05)

/**
 *  @def CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS
 *
 *  @brief
 *    The attempt to register an unsolicited message handler failed because the
 *    unsolicited message handler pool is full.
 *
 */
#define CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS       CHIP_CORE_ERROR(0x06)

/**
 *  @def CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER
 *
 *  @brief
 *    The attempt to unregister an unsolicited message handler failed because
 *    the target handler was not found in the unsolicited message handler pool.
 *
 */
#define CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER              CHIP_CORE_ERROR(0x07)

/**
 *  @def CHIP_ERROR_NO_CONNECTION_HANDLER
 *
 *  @brief
 *    No callback has been registered for handling a connection.
 *
 */
#define CHIP_ERROR_NO_CONNECTION_HANDLER                       CHIP_CORE_ERROR(0x08)

/**
 *  @def CHIP_ERROR_TOO_MANY_PEER_NODES
 *
 *  @brief
 *    The number of peer nodes exceeds the maximum limit of a local node.
 *
 */
#define CHIP_ERROR_TOO_MANY_PEER_NODES                         CHIP_CORE_ERROR(0x09)

/**
 *  @def CHIP_ERROR_SENTINEL
 *
 *  @brief
 *    For use locally to mark conditions such as value found or end of iteration.
 *
 */
#define CHIP_ERROR_SENTINEL                                    CHIP_CORE_ERROR(0x0a)

/**
 *  @def CHIP_ERROR_NO_MEMORY
 *
 *  @brief
 *    The attempt to allocate a buffer or object failed due to a lack of memory.
 *
 */
#define CHIP_ERROR_NO_MEMORY                                   CHIP_CORE_ERROR(0x0b)

/**
 *  @def CHIP_ERROR_NO_MESSAGE_HANDLER
 *
 *  @brief
 *    No callback has been registered for handling a message.
 *
 */
#define CHIP_ERROR_NO_MESSAGE_HANDLER                          CHIP_CORE_ERROR(0x0c)

/**
 *  @def CHIP_ERROR_MESSAGE_INCOMPLETE
 *
 *  @brief
 *    A message is incomplete.
 *
 */
#define CHIP_ERROR_MESSAGE_INCOMPLETE                          CHIP_CORE_ERROR(0x0d)

/**
 *  @def CHIP_ERROR_DATA_NOT_ALIGNED
 *
 *  @brief
 *    The data is not aligned.
 *
 */
#define CHIP_ERROR_DATA_NOT_ALIGNED                            CHIP_CORE_ERROR(0x0e)

/**
 *  @def CHIP_ERROR_UNKNOWN_KEY_TYPE
 *
 *  @brief
 *    The encryption key type is unknown.
 *
 */
#define CHIP_ERROR_UNKNOWN_KEY_TYPE                            CHIP_CORE_ERROR(0x0f)

/**
 *  @def CHIP_ERROR_KEY_NOT_FOUND
 *
 *  @brief
 *    The encryption key is not found.
 *
 */
#define CHIP_ERROR_KEY_NOT_FOUND                               CHIP_CORE_ERROR(0x10)

/**
 *  @def CHIP_ERROR_WRONG_ENCRYPTION_TYPE
 *
 *  @brief
 *    The encryption type is incorrect for the specified key.
 *
 */
#define CHIP_ERROR_WRONG_ENCRYPTION_TYPE                       CHIP_CORE_ERROR(0x11)

/**
 *  @def CHIP_ERROR_TOO_MANY_KEYS
 *
 *  @brief
 *    The attempt to allocate a key failed because the number of active keys
 *    exceeds the maximum limit.
 *
 */
#define CHIP_ERROR_TOO_MANY_KEYS                               CHIP_CORE_ERROR(0x12)

/**
 *  @def CHIP_ERROR_INTEGRITY_CHECK_FAILED
 *
 *  @brief
 *    The integrity check in the message does not match the expected integrity
 *    check.
 *
 */
#define CHIP_ERROR_INTEGRITY_CHECK_FAILED                      CHIP_CORE_ERROR(0x13)

/**
 *  @def CHIP_ERROR_INVALID_SIGNATURE
 *
 *  @brief
 *    Invalid signature.
 *
 */
#define CHIP_ERROR_INVALID_SIGNATURE                           CHIP_CORE_ERROR(0x14)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION
 *
 *  @brief
 *    A message version is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION                 CHIP_CORE_ERROR(0x15)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE
 *
 *  @brief
 *    An encryption type is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE                 CHIP_CORE_ERROR(0x16)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE
 *
 *  @brief
 *    A signature type is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE                  CHIP_CORE_ERROR(0x17)

/**
 *  @def CHIP_ERROR_INVALID_MESSAGE_LENGTH
 *
 *  @brief
 *    A message length is invalid.
 *
 */
#define CHIP_ERROR_INVALID_MESSAGE_LENGTH                      CHIP_CORE_ERROR(0x18)

/**
 *  @def CHIP_ERROR_BUFFER_TOO_SMALL
 *
 *  @brief
 *    A buffer is too small.
 *
 */
#define CHIP_ERROR_BUFFER_TOO_SMALL                            CHIP_CORE_ERROR(0x19)

/**
 *  @def CHIP_ERROR_DUPLICATE_KEY_ID
 *
 *  @brief
 *    A key id is duplicate.
 *
 */
#define CHIP_ERROR_DUPLICATE_KEY_ID                            CHIP_CORE_ERROR(0x1a)

/**
 *  @def CHIP_ERROR_WRONG_KEY_TYPE
 *
 *  @brief
 *    A key type does not match the expected key type.
 *
 */
#define CHIP_ERROR_WRONG_KEY_TYPE                              CHIP_CORE_ERROR(0x1b)

/**
 *  @def CHIP_ERROR_WELL_UNINITIALIZED
 *
 *  @brief
 *    A requested object is uninitialized.
 *
 */
#define CHIP_ERROR_WELL_UNINITIALIZED                          CHIP_CORE_ERROR(0x1c)

/**
 *  @def CHIP_ERROR_WELL_EMPTY
 *
 *  @brief
 *    A requested object is empty.
 *
 */
#define CHIP_ERROR_WELL_EMPTY                                  CHIP_CORE_ERROR(0x1d)

/**
 *  @def CHIP_ERROR_INVALID_STRING_LENGTH
 *
 *  @brief
 *    A string length is invalid.
 *
 */
#define CHIP_ERROR_INVALID_STRING_LENGTH                       CHIP_CORE_ERROR(0x1e)

/**
 *  @def CHIP_ERROR_INVALID_LIST_LENGTH
 *
 *  @brief
 *    A list length is invalid.
 *
 */
#define CHIP_ERROR_INVALID_LIST_LENGTH                         CHIP_CORE_ERROR(0x1f)

/**
 *  @def CHIP_ERROR_INVALID_INTEGRITY_TYPE
 *
 *  @brief
 *    An integrity type is invalid.
 *
 */
#define CHIP_ERROR_INVALID_INTEGRITY_TYPE                      CHIP_CORE_ERROR(0x20)

/**
 *  @def CHIP_END_OF_TLV
 *
 *  @brief
 *    The end of a TLV encoding,
 *    or the end of a TLV container element has been reached.
 *
 */
#define CHIP_ERROR_END_OF_TLV                                  CHIP_CORE_ERROR(0x21)
#define CHIP_END_OF_TLV CHIP_ERROR_END_OF_TLV

/**
 *  @def CHIP_ERROR_TLV_UNDERRUN
 *
 *  @brief
 *    The TLV encoding ended prematurely.
 *
 */
#define CHIP_ERROR_TLV_UNDERRUN                                CHIP_CORE_ERROR(0x22)

/**
 *  @def CHIP_ERROR_INVALID_TLV_ELEMENT
 *
 *  @brief
 *    A TLV element is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TLV_ELEMENT                         CHIP_CORE_ERROR(0x23)

/**
 *  @def CHIP_ERROR_INVALID_TLV_TAG
 *
 *  @brief
 *    A TLV tag is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TLV_TAG                             CHIP_CORE_ERROR(0x24)

/**
 *  @def CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG
 *
 *  @brief
 *    An implicitly encoded TLV tag was encountered,
 *    but an implicit profile id has not been defined.
 *
 */
#define CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG                    CHIP_CORE_ERROR(0x25)

/**
 *  @def CHIP_ERROR_WRONG_TLV_TYPE
 *
 *  @brief
 *    A TLV type is wrong.
 *
 */
#define CHIP_ERROR_WRONG_TLV_TYPE                              CHIP_CORE_ERROR(0x26)

/**
 *  @def CHIP_ERROR_TLV_CONTAINER_OPEN
 *
 *  @brief
 *    A TLV container is unexpectedly open.
 *
 */
#define CHIP_ERROR_TLV_CONTAINER_OPEN                          CHIP_CORE_ERROR(0x27)

/**
 *  @def CHIP_ERROR_INVALID_TRANSFER_MODE
 *
 *  @brief
 *    A transfer mode is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TRANSFER_MODE                       CHIP_CORE_ERROR(0x28)

/**
 *  @def CHIP_ERROR_INVALID_PROFILE_ID
 *
 *  @brief
 *    A profile id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PROFILE_ID                          CHIP_CORE_ERROR(0x29)

/**
 *  @def CHIP_ERROR_INVALID_MESSAGE_TYPE
 *
 *  @brief
 *    A message type is invalid.
 *
 */
#define CHIP_ERROR_INVALID_MESSAGE_TYPE                        CHIP_CORE_ERROR(0x2a)

/**
 *  @def CHIP_ERROR_UNEXPECTED_TLV_ELEMENT
 *
 *  @brief
 *    An unexpected TLV element was encountered.
 *
 */
#define CHIP_ERROR_UNEXPECTED_TLV_ELEMENT                      CHIP_CORE_ERROR(0x2b)

/**
 *  @def CHIP_ERROR_STATUS_REPORT_RECEIVED
 *
 *  @brief
 *    A status report is received from a peer node.
 *
 */
#define CHIP_ERROR_STATUS_REPORT_RECEIVED                      CHIP_CORE_ERROR(0x2c)

/**
 *  @def CHIP_ERROR_NOT_IMPLEMENTED
 *
 *  @brief
 *    A requested function or feature is not implemented.
 *
 */
#define CHIP_ERROR_NOT_IMPLEMENTED                             CHIP_CORE_ERROR(0x2d)

/**
 *  @def CHIP_ERROR_INVALID_ADDRESS
 *
 *  @brief
 *    An address is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ADDRESS                             CHIP_CORE_ERROR(0x2e)

/**
 *  @def CHIP_ERROR_INVALID_ARGUMENT
 *
 *  @brief
 *    An argument is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ARGUMENT                            CHIP_CORE_ERROR(0x2f)

/**
 *  @def CHIP_ERROR_INVALID_PATH_LIST
 *
 *  @brief
 *    A TLV path list is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PATH_LIST                           CHIP_CORE_ERROR(0x30)

/**
 *  @def CHIP_ERROR_INVALID_DATA_LIST
 *
 *  @brief
 *    A TLV data list is invalid.
 *
 */
#define CHIP_ERROR_INVALID_DATA_LIST                           CHIP_CORE_ERROR(0x31)

/**
 *  @def CHIP_ERROR_TIMEOUT
 *
 *  @brief
 *    A request timed out.
 *
 */
#define CHIP_ERROR_TIMEOUT                                     CHIP_CORE_ERROR(0x32)

/**
 *  @def CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR
 *
 *  @brief
 *    A device descriptor is invalid.
 *
 */
#define CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR                   CHIP_CORE_ERROR(0x33)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION
 *
 *  @brief
 *    A device descriptor version is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION       CHIP_CORE_ERROR(0x34)

/**
 *  @def CHIP_END_OF_INPUT
 *
 *  @brief
 *    An input ended.
 *
 */
#define CHIP_ERROR_END_OF_INPUT                                CHIP_CORE_ERROR(0x35)
#define CHIP_END_OF_INPUT CHIP_ERROR_END_OF_INPUT

/**
 *  @def CHIP_ERROR_RATE_LIMIT_EXCEEDED
 *
 *  @brief
 *    A rate limit is exceeded.
 *
 */
#define CHIP_ERROR_RATE_LIMIT_EXCEEDED                         CHIP_CORE_ERROR(0x36)

/**
 *  @def CHIP_ERROR_SECURITY_MANAGER_BUSY
 *
 *  @brief
 *    A security manager is busy.
 *
 */
#define CHIP_ERROR_SECURITY_MANAGER_BUSY                       CHIP_CORE_ERROR(0x37)

/**
 *  @def CHIP_ERROR_INVALID_PASE_PARAMETER
 *
 *  @brief
 *    A PASE parameter is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PASE_PARAMETER                      CHIP_CORE_ERROR(0x38)

/**
 *  @def CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1
 *
 *  @brief
 *    PASE supports only config1.
 *
 */
#define CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1                  CHIP_CORE_ERROR(0x39)

/**
 *  @def CHIP_ERROR_KEY_CONFIRMATION_FAILED
 *
 *  @brief
 *    A key confirmation failed.
 *
 */
#define CHIP_ERROR_KEY_CONFIRMATION_FAILED                     CHIP_CORE_ERROR(0x3a)

/**
 *  @def CHIP_ERROR_INVALID_USE_OF_SESSION_KEY
 *
 *  @brief
 *    A use of session key is invalid.
 *
 */
#define CHIP_ERROR_INVALID_USE_OF_SESSION_KEY                  CHIP_CORE_ERROR(0x3b)

/**
 *  @def CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY
 *
 *  @brief
 *    A connection is closed unexpectedly.
 *
 */
#define CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY              CHIP_CORE_ERROR(0x3c)

/**
 *  @def CHIP_ERROR_MISSING_TLV_ELEMENT
 *
 *  @brief
 *    A TLV element is missing.
 *
 */
#define CHIP_ERROR_MISSING_TLV_ELEMENT                         CHIP_CORE_ERROR(0x3d)

/**
 *  @def CHIP_ERROR_RANDOM_DATA_UNAVAILABLE
 *
 *  @brief
 *    Secure random data is not available.
 *
 */
#define CHIP_ERROR_RANDOM_DATA_UNAVAILABLE                     CHIP_CORE_ERROR(0x3e)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT
 *
 *  @brief
 *    A type in host/port list is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT               CHIP_CORE_ERROR(0x3f)

/**
 *  @def CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX
 *
 *  @brief
 *    A suffix index in host/port list is invalid.
 *
 */
#define CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX                   CHIP_CORE_ERROR(0x40)

/**
 *  @def CHIP_ERROR_HOST_PORT_LIST_EMPTY
 *
 *  @brief
 *    A host/port list is empty.
 *
 */
#define CHIP_ERROR_HOST_PORT_LIST_EMPTY                        CHIP_CORE_ERROR(0x41)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_AUTH_MODE
 *
 *  @brief
 *    An authentication mode is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_AUTH_MODE                       CHIP_CORE_ERROR(0x42)

/**
 *  @def CHIP_ERROR_INVALID_SERVICE_EP
 *
 *  @brief
 *    A service endpoint is invalid.
 *
 */
#define CHIP_ERROR_INVALID_SERVICE_EP                          CHIP_CORE_ERROR(0x43)

/**
 *  @def CHIP_ERROR_INVALID_DIRECTORY_ENTRY_TYPE
 *
 *  @brief
 *    A directory entry type is unknown.
 *
 */
#define CHIP_ERROR_INVALID_DIRECTORY_ENTRY_TYPE                CHIP_CORE_ERROR(0x44)

/**
 *  @def CHIP_ERROR_FORCED_RESET
 *
 *  @brief
 *    A service manager is forced to reset.
 *
 */
#define CHIP_ERROR_FORCED_RESET                                CHIP_CORE_ERROR(0x45)

/**
 *  @def CHIP_ERROR_NO_ENDPOINT
 *
 *  @brief
 *    No endpoint is available.
 *
 */
#define CHIP_ERROR_NO_ENDPOINT                                 CHIP_CORE_ERROR(0x46)

/**
 *  @def CHIP_ERROR_INVALID_DESTINATION_NODE_ID
 *
 *  @brief
 *    A destination node id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_DESTINATION_NODE_ID                 CHIP_CORE_ERROR(0x47)

/**
 *  @def CHIP_ERROR_NOT_CONNECTED
 *
 *  @brief
 *    The operation cannot be performed because the underlying object is not
 *    connected.
 *
 */
#define CHIP_ERROR_NOT_CONNECTED                               CHIP_CORE_ERROR(0x48)

/**
 *  @def CHIP_ERROR_NO_SW_UPDATE_AVAILABLE
 *
 *  @brief
 *    No software update is available.
 *
 */
#define CHIP_ERROR_NO_SW_UPDATE_AVAILABLE                      CHIP_CORE_ERROR(0x49)

/**
 *  @def CHIP_ERROR_CA_CERT_NOT_FOUND
 *
 *  @brief
 *    CA certificate is not found.
 *
 */
#define CHIP_ERROR_CA_CERT_NOT_FOUND                           CHIP_CORE_ERROR(0x4a)

/**
 *  @def CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED
 *
 *  @brief
 *    A certificate path length exceeds the constraint.
 *
 */
#define CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED           CHIP_CORE_ERROR(0x4b)

/**
 *  @def CHIP_ERROR_CERT_PATH_TOO_LONG
 *
 *  @brief
 *    A certificate path is too long.
 *
 */
#define CHIP_ERROR_CERT_PATH_TOO_LONG                          CHIP_CORE_ERROR(0x4c)

/**
 *  @def CHIP_ERROR_CERT_USAGE_NOT_ALLOWED
 *
 *  @brief
 *    A requested certificate usage is not allowed.
 *
 */
#define CHIP_ERROR_CERT_USAGE_NOT_ALLOWED                      CHIP_CORE_ERROR(0x4d)

/**
 *  @def CHIP_ERROR_CERT_EXPIRED
 *
 *  @brief
 *    A certificate expired.
 *
 */
#define CHIP_ERROR_CERT_EXPIRED                                CHIP_CORE_ERROR(0x4e)

/**
 *  @def CHIP_ERROR_CERT_NOT_VALID_YET
 *
 *  @brief
 *    A certificate is not valid yet.
 *
 */
#define CHIP_ERROR_CERT_NOT_VALID_YET                          CHIP_CORE_ERROR(0x4f)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CERT_FORMAT
 *
 *  @brief
 *    A certificate format is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CERT_FORMAT                     CHIP_CORE_ERROR(0x50)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE
 *
 *  @brief
 *    An elliptic curve is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE                  CHIP_CORE_ERROR(0x51)

/**
 *  @def CHIP_CERT_NOT_USED
 *
 *  @brief
 *    A certificate was not used during the chain validation.
 *
 */
#define CHIP_ERROR_CERT_NOT_USED                               CHIP_CORE_ERROR(0x52)
#define CHIP_CERT_NOT_USED CHIP_ERROR_CERT_NOT_USED

/**
 *  @def CHIP_ERROR_CERT_NOT_FOUND
 *
 *  @brief
 *    A certificate is not found.
 *
 */
#define CHIP_ERROR_CERT_NOT_FOUND                              CHIP_CORE_ERROR(0x53)

/**
 *  @def CHIP_ERROR_INVALID_CASE_PARAMETER
 *
 *  @brief
 *    A CASE parameter is invalid.
 *
 */
#define CHIP_ERROR_INVALID_CASE_PARAMETER                      CHIP_CORE_ERROR(0x54)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION
 *
 *  @brief
 *    A CASE configuration is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION              CHIP_CORE_ERROR(0x55)

/**
 *  @def CHIP_ERROR_CERT_LOAD_FAILED
 *
 *  @brief
 *    A certificate load failed.
 *
 */
#define CHIP_ERROR_CERT_LOAD_FAILED                            CHIP_CORE_ERROR(0x56)

/**
 *  @def CHIP_ERROR_CERT_NOT_TRUSTED
 *
 *  @brief
 *    A certificate is not trusted.
 *
 */
#define CHIP_ERROR_CERT_NOT_TRUSTED                            CHIP_CORE_ERROR(0x57)

/**
 *  @def CHIP_ERROR_INVALID_ACCESS_TOKEN
 *
 *  @brief
 *    An access token is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ACCESS_TOKEN                        CHIP_CORE_ERROR(0x58)

/**
 *  @def CHIP_ERROR_WRONG_CERT_SUBJECT
 *
 *  @brief
 *    A certificate subject is wrong.
 *
 */
#define CHIP_ERROR_WRONG_CERT_SUBJECT                          CHIP_CORE_ERROR(0x59)

// deprecated alias
#define CHIP_ERROR_WRONG_CERTIFICATE_SUBJECT CHIP_ERROR_WRONG_CERT_SUBJECT

/**
 *  @def CHIP_ERROR_INVALID_PROVISIONING_BUNDLE
 *
 *  @brief
 *    A provisioning bundle is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PROVISIONING_BUNDLE                 CHIP_CORE_ERROR(0x5a)

/**
 *  @def CHIP_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR
 *
 *  @brief
 *    A provision bundle encountered a decryption error.
 *
 */
#define CHIP_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR        CHIP_CORE_ERROR(0x5b)

/**
 *  @def CHIP_ERROR_WRONG_NODE_ID
 *
 *  @brief
 *    A node id is wrong.
 *
 */
#define CHIP_ERROR_WRONG_NODE_ID                               CHIP_CORE_ERROR(0x5c)

/**
 *  @def CHIP_ERROR_CONN_ACCEPTED_ON_WRONG_PORT
 *
 *  @brief
 *    A connection is accepted on a wrong port.
 *
 */
#define CHIP_ERROR_CONN_ACCEPTED_ON_WRONG_PORT                 CHIP_CORE_ERROR(0x5d)

/**
 *  @def CHIP_ERROR_CALLBACK_REPLACED
 *
 *  @brief
 *    An application callback has been replaced.
 *
 */
#define CHIP_ERROR_CALLBACK_REPLACED                           CHIP_CORE_ERROR(0x5e)

/**
 *  @def CHIP_ERROR_NO_CASE_AUTH_DELEGATE
 *
 *  @brief
 *    No CASE authentication delegate is set.
 *
 */
#define CHIP_ERROR_NO_CASE_AUTH_DELEGATE                       CHIP_CORE_ERROR(0x5f)

/**
 *  @def CHIP_ERROR_DEVICE_LOCATE_TIMEOUT
 *
 *  @brief
 *    The attempt to locate device timed out.
 *
 */
#define CHIP_ERROR_DEVICE_LOCATE_TIMEOUT                       CHIP_CORE_ERROR(0x60)

/**
 *  @def CHIP_ERROR_DEVICE_CONNECT_TIMEOUT
 *
 *  @brief
 *    The attempt to connect device timed out.
 *
 */
#define CHIP_ERROR_DEVICE_CONNECT_TIMEOUT                      CHIP_CORE_ERROR(0x61)

/**
 *  @def CHIP_ERROR_DEVICE_AUTH_TIMEOUT
 *
 *  @brief
 *    The attempt to authenticate device timed out.
 *
 */
#define CHIP_ERROR_DEVICE_AUTH_TIMEOUT                         CHIP_CORE_ERROR(0x62)

/**
 *  @def CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED
 *
 *  @brief
 *    A message is not acknowledged after max retries.
 *
 */
#define CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED                    CHIP_CORE_ERROR(0x63)

/**
 *  @def CHIP_ERROR_RETRANS_TABLE_FULL
 *
 *  @brief
 *    A retransmission table is already full.
 *
 */
#define CHIP_ERROR_RETRANS_TABLE_FULL                          CHIP_CORE_ERROR(0x64)

/**
 *  @def CHIP_ERROR_INVALID_ACK_ID
 *
 *  @brief
 *    An acknowledgment id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ACK_ID                              CHIP_CORE_ERROR(0x65)

/**
 *  @def CHIP_ERROR_SEND_THROTTLED
 *
 *  @brief
 *    A send is throttled.
 *
 */
#define CHIP_ERROR_SEND_THROTTLED                              CHIP_CORE_ERROR(0x66)

/**
 *  @def CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE
 *
 *  @brief
 *    A message version is not supported by the current exchange context.
 *
 */
#define CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE              CHIP_CORE_ERROR(0x67)

/**
 *  @def CHIP_ERROR_TRANSACTION_CANCELED
 *
 *  @brief
 *    A transaction is cancelled.
 *
 */
#define CHIP_ERROR_TRANSACTION_CANCELED                        CHIP_CORE_ERROR(0x68)

/**
 *  @def CHIP_ERROR_LISTENER_ALREADY_STARTED
 *
 *  @brief
 *    A listener has already started.
 *
 */
#define CHIP_ERROR_LISTENER_ALREADY_STARTED                    CHIP_CORE_ERROR(0x69)

/**
 *  @def CHIP_ERROR_LISTENER_ALREADY_STOPPED
 *
 *  @brief
 *    A listener has already stopped.
 *
 */
#define CHIP_ERROR_LISTENER_ALREADY_STOPPED                    CHIP_CORE_ERROR(0x6a)

/**
 *  @def CHIP_ERROR_UNKNOWN_TOPIC
 *
 *  @brief
 *    A topic ID was unknown to the recipient.
 *
 */
#define CHIP_ERROR_UNKNOWN_TOPIC                               CHIP_CORE_ERROR(0x6b)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *
 *  @brief
 *    A CHIP feature is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE                    CHIP_CORE_ERROR(0x6c)

/**
 *  @def CHIP_ERROR_PASE_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    PASE is required to reconfigure.
 *
 */
#define CHIP_ERROR_PASE_RECONFIGURE_REQUIRED                   CHIP_CORE_ERROR(0x6d)

/**
 *  @def CHIP_ERROR_INVALID_PASE_CONFIGURATION
 *
 *  @brief
 *    A PASE configuration is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PASE_CONFIGURATION                  CHIP_CORE_ERROR(0x6e)

/**
 *  @def CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS
 *
 *  @brief
 *    No PASE configuration is in common.
 *
 */
#define CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS               CHIP_CORE_ERROR(0x6f)

/**
 *  @def CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR
 *
 *  @brief
 *    An unsolicited message with the originator bit clear.
 *
 */
#define CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR               CHIP_CORE_ERROR(0x70)

/**
 *  @def CHIP_ERROR_INVALID_FABRIC_ID
 *
 *  @brief
 *    A fabric id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_FABRIC_ID                           CHIP_CORE_ERROR(0x71)

/**
 *  @def CHIP_ERROR_TOO_MANY_CONNECTIONS
 *
 *  @brief
 *    The attempt to allocate a connection object failed because too many
 *    connections exist.
 *
 */
#define CHIP_ERROR_TOO_MANY_CONNECTIONS                        CHIP_CORE_ERROR(0x72)

// unused                                                      CHIP_CORE_ERROR(0x73)
// unused                                                      CHIP_CORE_ERROR(0x74)

/**
 *  @def CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED
 *
 *  @brief
 *    DRBG entropy source failed to generate entropy data.
 *
 */
#define CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED                  CHIP_CORE_ERROR(0x75)

/**
 *  @def CHIP_ERROR_TLV_TAG_NOT_FOUND
 *
 *  @brief
 *    A specified TLV tag was not found.
 *
 */
#define CHIP_ERROR_TLV_TAG_NOT_FOUND                           CHIP_CORE_ERROR(0x76)

/**
 *  @def CHIP_ERROR_INVALID_TOKENPAIRINGBUNDLE
 *
 *  @brief
 *    A token pairing bundle is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TOKENPAIRINGBUNDLE                  CHIP_CORE_ERROR(0x77)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION
 *
 *  @brief
 *    A token pairing bundle is invalid.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION      CHIP_CORE_ERROR(0x78)

/**
 *  @def CHIP_ERROR_NO_TAKE_AUTH_DELEGATE
 *
 *  @brief
 *    No TAKE authentication delegate is set.
 *
 */
#define CHIP_ERROR_NO_TAKE_AUTH_DELEGATE                       CHIP_CORE_ERROR(0x79)

/**
 *  @def CHIP_ERROR_TAKE_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    TAKE requires a reconfigure.
 *
 */
#define CHIP_ERROR_TAKE_RECONFIGURE_REQUIRED                   CHIP_CORE_ERROR(0x7a)

/**
 *  @def CHIP_ERROR_TAKE_REAUTH_POSSIBLE
 *
 *  @brief
 *    TAKE can do a reauthentication.
 *
 */
#define CHIP_ERROR_TAKE_REAUTH_POSSIBLE                        CHIP_CORE_ERROR(0x7b)

/**
 *  @def CHIP_ERROR_INVALID_TAKE_PARAMETER
 *
 *  @brief
 *    Received an invalid TAKE paramter.
 *
 */
#define CHIP_ERROR_INVALID_TAKE_PARAMETER                      CHIP_CORE_ERROR(0x7c)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_TAKE_CONFIGURATION
 *
 *  @brief
 *    This configuration is not supported by TAKE.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_TAKE_CONFIGURATION              CHIP_CORE_ERROR(0x7d)

/**
 *  @def CHIP_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED
 *
 *  @brief
 *    The TAKE Token Identification failed.
 *
 */
#define CHIP_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED            CHIP_CORE_ERROR(0x7e)

/**
 *  @def CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER
 *
 *  @brief
 *    The encryption key is not found error received from a peer node.
 *
 */
#define CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER                     CHIP_CORE_ERROR(0x7f)

/**
 *  @def CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    The wrong encryption type error received from a peer node.
 *
 */
#define CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER             CHIP_CORE_ERROR(0x80)

/**
 *  @def CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER
 *
 *  @brief
 *    The unknown key type error received from a peer node.
 *
 */
#define CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER                  CHIP_CORE_ERROR(0x81)

/**
 *  @def CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER
 *
 *  @brief
 *    The invalid use of session key error received from a peer node.
 *
 */
#define CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER        CHIP_CORE_ERROR(0x82)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    An unsupported encryption type error received from a peer node.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER       CHIP_CORE_ERROR(0x83)

/**
 *  @def CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER
 *
 *  @brief
 *    The internal key error received from a peer node.
 *
 */
#define CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER                CHIP_CORE_ERROR(0x84)

/**
 *  @def CHIP_ERROR_INVALID_KEY_ID
 *
 *  @brief
 *    A key id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_KEY_ID                              CHIP_CORE_ERROR(0x85)

/**
 *  @def CHIP_ERROR_INVALID_TIME
 *
 *  @brief
 *    Time has invalid value.
 *
 */
#define CHIP_ERROR_INVALID_TIME                                CHIP_CORE_ERROR(0x86)

/**
 *  @def CHIP_ERROR_LOCKING_FAILURE
 *
 *  @brief
 *    Failure to acquire or release an OS provided mutex.
 *
 */
#define CHIP_ERROR_LOCKING_FAILURE                             CHIP_CORE_ERROR(0x87)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_PASSCODE_CONFIG
 *
 *  @brief
 *    A passcode encryption configuration is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_PASSCODE_CONFIG                 CHIP_CORE_ERROR(0x88)

/**
 *  @def CHIP_ERROR_PASSCODE_AUTHENTICATION_FAILED
 *
 *  @brief
 *    The CHIP passcode authentication failed.
 *
 */
#define CHIP_ERROR_PASSCODE_AUTHENTICATION_FAILED              CHIP_CORE_ERROR(0x89)

/**
 *  @def CHIP_ERROR_PASSCODE_FINGERPRINT_FAILED
 *
 *  @brief
 *    The CHIP passcode fingerprint failed.
 *
 */
#define CHIP_ERROR_PASSCODE_FINGERPRINT_FAILED                 CHIP_CORE_ERROR(0x8a)

/**
 *  @def CHIP_ERROR_SERIALIZATION_ELEMENT_NULL
 *
 *  @brief
 *      The element of the struct is null.
 *
 */
#define CHIP_ERROR_SERIALIZATION_ELEMENT_NULL                  CHIP_CORE_ERROR(0x8b)

/**
 *  @def CHIP_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM
 *
 *  @brief
 *    The certificate was not signed using the required signature algorithm.
 *
 */
#define CHIP_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM              CHIP_CORE_ERROR(0x8c)

/**
 *  @def CHIP_ERROR_WRONG_CHIP_SIGNATURE_ALGORITHM
 *
 *  @brief
 *    The CHIP signature was not signed using the required signature algorithm.
 *
 */
#define CHIP_ERROR_WRONG_CHIP_SIGNATURE_ALGORITHM              CHIP_CORE_ERROR(0x8d)

/**
 *  @def CHIP_ERROR_SCHEMA_MISMATCH
 *
 *  @brief
 *    A mismatch in schema was encountered.
 *
 */
#define CHIP_ERROR_SCHEMA_MISMATCH                             CHIP_CORE_ERROR(0x8e)

/**
 *  @def CHIP_ERROR_INVALID_INTEGER_VALUE
 *
 *  @brief
 *    An integer does not have the kind of value we expect.
 *
 */
#define CHIP_ERROR_INVALID_INTEGER_VALUE                       CHIP_CORE_ERROR(0x8f)

/**
 *  @def CHIP_ERROR_CASE_RECONFIG_REQUIRED
 *
 *  @brief
 *    CASE is required to reconfigure.
 *
 */
#define CHIP_ERROR_CASE_RECONFIG_REQUIRED                      CHIP_CORE_ERROR(0x90)

/**
 *  @def CHIP_ERROR_TOO_MANY_CASE_RECONFIGURATIONS
 *
 *  @brief
 *    Too many CASE reconfigurations were received.
 *
 */
#define CHIP_ERROR_TOO_MANY_CASE_RECONFIGURATIONS              CHIP_CORE_ERROR(0x91)

/**
 *  @def CHIP_ERROR_BAD_REQUEST
 *
 *  @brief
 *    The request cannot be processed or fulfilled
 *
 */
#define CHIP_ERROR_BAD_REQUEST                                 CHIP_CORE_ERROR(0x92)

/**
 *  @def CHIP_ERROR_INVALID_MESSAGE_FLAG
 *
 *  @brief
 *    One or more message flags have invalid value.
 *
 */
#define CHIP_ERROR_INVALID_MESSAGE_FLAG                        CHIP_CORE_ERROR(0x93)

/**
 *  @def CHIP_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    Key export protocol required to reconfigure.
 *
 */
#define CHIP_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED             CHIP_CORE_ERROR(0x94)

/**
 *  @def CHIP_ERROR_INVALID_KEY_EXPORT_CONFIGURATION
 *
 *  @brief
 *    A key export protocol configuration is invalid.
 *
 */
#define CHIP_ERROR_INVALID_KEY_EXPORT_CONFIGURATION            CHIP_CORE_ERROR(0x95)

/**
 *  @def CHIP_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS
 *
 *  @brief
 *    No key export protocol configuration is in common.
 *
 */
#define CHIP_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS         CHIP_CORE_ERROR(0x96)

/**
 *  @def CHIP_ERROR_NO_KEY_EXPORT_DELEGATE
 *
 *  @brief
 *    No key export delegate is set.
 *
 */
#define CHIP_ERROR_NO_KEY_EXPORT_DELEGATE                      CHIP_CORE_ERROR(0x97)

/**
 *  @def CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST
 *
 *  @brief
 *    Unauthorized key export request.
 *
 */
#define CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST             CHIP_CORE_ERROR(0x98)

/**
 *  @def CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE
 *
 *  @brief
 *    Unauthorized key export response.
 *
 */
#define CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE            CHIP_CORE_ERROR(0x99)

/**
 *  @def CHIP_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED
 *
 *  @brief
 *    The CHIP exported encrypted key authentication failed.
 *
 */
#define CHIP_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED          CHIP_CORE_ERROR(0x9a)

/**
 *  @def CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES
 *
 *  @brief
 *    The number of shared secure sessions end nodes exceeds
 *    the maximum limit.
 *
 */
#define CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES           CHIP_CORE_ERROR(0x9b)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_ELEMENT
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_ELEMENT         CHIP_CORE_ERROR(0x9c)

/**
 * @def CHIP_ERROR_WRONG_CERT_TYPE
 *
 * @brief
 *   The presented certificate was of the wrong type.
 */
#define CHIP_ERROR_WRONG_CERT_TYPE                             CHIP_CORE_ERROR(0x9d)

/**
 * @def CHIP_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED
 *
 * @brief
 *   The application's event handler failed to call the default event handler function
 *   when presented with an unknown event.
 */
#define CHIP_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED            CHIP_CORE_ERROR(0x9e)

/**
 *  @def CHIP_ERROR_PERSISTED_STORAGE_FAILED
 *
 *  @brief
 *    Persisted storage memory read/write failure.
 *
 */
#define CHIP_ERROR_PERSISTED_STORAGE_FAILED                    CHIP_CORE_ERROR(0x9f)

/**
 *  @def CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND
 *
 *  @brief
 *    The specific value is not found in the persisted storage.
 *
 */
#define CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND           CHIP_CORE_ERROR(0xa0)

/**
 *  @def CHIP_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED
 *
 *  @brief
 *    The specified profile string support context is already registered.
 *
 */
#define CHIP_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED   CHIP_CORE_ERROR(0xa1)

/**
 *  @def CHIP_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED
 *
 *  @brief
 *    The specified profile string support context is not registered.
 *
 */
#define CHIP_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED       CHIP_CORE_ERROR(0xa2)

/**
 *  @def CHIP_ERROR_INCOMPATIBLE_SCHEMA_VERSION
 *
 *  @brief
 *    Encountered a mismatch in compatibility w.r.t to IDL schema version
 */
#define CHIP_ERROR_INCOMPATIBLE_SCHEMA_VERSION                 CHIP_CORE_ERROR(0xa3)

/**
 *  @def CHIP_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION
 *
 *  @brief
 *    Encountered a mismatch between update required version and current version
 */
#define CHIP_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION            CHIP_CORE_ERROR(0xa4)

/**
 *  @def CHIP_ERROR_ACCESS_DENIED
 *
 *  @brief
 *    The CHIP message is not granted access for further processing.
 */
#define CHIP_ERROR_ACCESS_DENIED                               CHIP_CORE_ERROR(0xa5)

/**
 *  @def CHIP_ERROR_UNKNOWN_RESOURCE_ID
 *
 *  @brief
 *    Unknown resource ID
 *
 */
#define CHIP_ERROR_UNKNOWN_RESOURCE_ID                         CHIP_CORE_ERROR(0xa6)

/**
 * @def CHIP_ERROR_VERSION_MISMATCH
 *
 * @brief
 *   The conditional update of a trait instance path has failed
 *   because the local changes are based on an obsolete version of the
 *   data.
 */
#define CHIP_ERROR_VERSION_MISMATCH                            CHIP_CORE_ERROR(0xa7)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE
 *
 *  @brief
 *    Device doesn't support standalone Thread network creation.
 *    On some legacy devices new Thread network can only be created
 *    together with CHIP Fabric using CrateFabric() message.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE           CHIP_CORE_ERROR(0xa8)

/**
 *  @def CHIP_ERROR_INCONSISTENT_CONDITIONALITY
 *
 *  @brief
 *    A TraitPath was declared updated with a conditionality that
 *    does not match that of other TraitPaths already updated in the
 *    same Trait Instance.
 *
 */
#define CHIP_ERROR_INCONSISTENT_CONDITIONALITY                 CHIP_CORE_ERROR(0xa9)

/**
 *  @def CHIP_ERROR_LOCAL_DATA_INCONSISTENT
 *
 *  @brief
 *    The local data does not match any known version of the
 *    Trait Instance and cannot support the operation requested.
 *
 */
#define CHIP_ERROR_LOCAL_DATA_INCONSISTENT                     CHIP_CORE_ERROR(0xaa)

/**
 * @def CHIP_EVENT_ID_FOUND
 *
 * @brief
 *   Event ID matching the criteria was found
 */
#define CHIP_ERROR_EVENT_ID_FOUND                              CHIP_CORE_ERROR(0xab)
#define CHIP_EVENT_ID_FOUND CHIP_ERROR_EVENT_ID_FOUND

/**
 * @def CHIP_ERROR_INTERNAL
 *
 * @brief
 *   Internal error
 */
#define CHIP_ERROR_INTERNAL                                    CHIP_CORE_ERROR(0xac)

/**
 * @def CHIP_ERROR_OPEN_FAILED
 *
 * @brief
 *   Open file failed
 */
#define CHIP_ERROR_OPEN_FAILED                                 CHIP_CORE_ERROR(0xad)

/**
 * @def CHIP_ERROR_READ_FAILED
 *
 * @brief
 *   Read from file failed
 */
#define CHIP_ERROR_READ_FAILED                                 CHIP_CORE_ERROR(0xae)

/**
 * @def CHIP_ERROR_WRITE_FAILED
 *
 * @brief
 *   Write to file failed
 */
#define CHIP_ERROR_WRITE_FAILED                                CHIP_CORE_ERROR(0xaf)

/**
 * @def CHIP_ERROR_DECODE_FAILED
 *
 * @brief
 *   Decoding failed
 */
#define CHIP_ERROR_DECODE_FAILED                               CHIP_CORE_ERROR(0xb0)

/**
 *  @def CHIP_ERROR_SESSION_KEY_SUSPENDED
 *
 *  @brief
 *    Use of the identified session key is suspended.
 *
 */
#define CHIP_ERROR_SESSION_KEY_SUSPENDED                       CHIP_CORE_ERROR(0xb1)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_WIRELESS_REGULATORY_DOMAIN
 *
 *  @brief
 *    The specified wireless regulatory domain is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_WIRELESS_REGULATORY_DOMAIN      CHIP_CORE_ERROR(0xb2)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_WIRELESS_OPERATING_LOCATION
 *
 *  @brief
 *    The specified wireless operating location is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_WIRELESS_OPERATING_LOCATION     CHIP_CORE_ERROR(0xb3)

/**
 *  @def CHIP_ERROR_MDNS_COLLISSION
 *
 *  @brief
 *    The registered service name has collision on the LAN.
 *
 */
#define CHIP_ERROR_MDNS_COLLISSION                             CHIP_CORE_ERROR(0xb4)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH
 *
 * @brief
 *   The Attribute path is malformed: it either does not contain
 *   the required path
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH                 CHIP_CORE_ERROR(0xb5)

/**
 * @def CHIP_ERROR_IM_MALFORMED_EVENT_PATH
 *
 * @brief
 *   The Attribute Path is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_EVENT_PATH                     CHIP_CORE_ERROR(0xb6)

/**
 * @def CHIP_ERROR_IM_MALFORMED_COMMAND_PATH
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_COMMAND_PATH                   CHIP_CORE_ERROR(0xb7)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT       CHIP_CORE_ERROR(0xb8)

/**
 * @def CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT           CHIP_CORE_ERROR(0xb9)

/**
 * @def CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT
 *
 * @brief
 *   The Event DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT             CHIP_CORE_ERROR(0xba)

/**
 * @def CHIP_ERROR_IM_MALFORMED_STATUS_CODE
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_STATUS_CODE                    CHIP_CORE_ERROR(0xbb)

/**
 * @def CHIP_ERROR_PEER_NODE_NOT_FOUND
 *
 * @brief
 *   Unable to find the peer node
 */
#define CHIP_ERROR_PEER_NODE_NOT_FOUND                         CHIP_CORE_ERROR(0xbc)

/**
 * @def CHIP_ERROR_HSM
 *
 * @brief
 *   Error in Hardware security module. Used for software fallback option.
 */
#define CHIP_ERROR_HSM                                         CHIP_CORE_ERROR(0xbd)

/**
 * @def CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED
 *
 * @brief
 *   The commissioner doesn't require an intermediate CA to sign the operational certificates.
 */
#define CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED                CHIP_CORE_ERROR(0xbe)

/**
 *  @def CHIP_ERROR_REAL_TIME_NOT_SYNCED
 *
 *  @brief
 *      The system's real time clock is not synchronized to an accurate time source.
 */
#define CHIP_ERROR_REAL_TIME_NOT_SYNCED                        CHIP_CORE_ERROR(0xbf)

/**
 *  @def CHIP_ERROR_UNEXPECTED_EVENT
 *
 *  @brief
 *      An unexpected event was encountered.
 */
#define CHIP_ERROR_UNEXPECTED_EVENT                            CHIP_CORE_ERROR(0xc0)

/**
 *  @def CHIP_ERROR_ENDPOINT_POOL_FULL
 *
 *  @brief
 *    No endpoint pool entry is available.
 *
 */
#define CHIP_ERROR_ENDPOINT_POOL_FULL                          CHIP_CORE_ERROR(0xc1)

/**
 *  @def CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG
 *
 *  @brief
 *    More inbound message data is pending than available buffer space available to copy it.
 *
 */
#define CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG                     CHIP_CORE_ERROR(0xc2)

/**
 *  @def CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG
 *
 *  @brief
 *    More outbound message data is pending than available buffer space available to copy it.
 *
 */
#define CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG                    CHIP_CORE_ERROR(0xc3)

/**
 * @def CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED
 *
 * @brief
 *   The received message is a duplicate of a previously received message.
 */
#define CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED                  CHIP_CORE_ERROR(0xc4)

/**
 *  @}
 */

// clang-format on

// !!!!! IMPORTANT !!!!!  If you add new CHIP errors, please update the translation
// of error codes to strings in CHIPError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

namespace chip {

extern void RegisterCHIPLayerErrorFormatter();
extern bool FormatCHIPError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} // namespace chip
