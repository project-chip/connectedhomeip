/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <lib/support/TypeTraits.h>

#include <inttypes.h>
#include <limits>
#include <stdint.h>
#include <type_traits>

namespace chip {

/**
 * This class represents CHIP errors.
 *
 * At the top level, an error belongs to a Range and has an integral Value whose meaning depends on the Range.
 * One, Range::kSDK, is used for the CHIP SDK's own errors; others encapsulate error codes from external sources
 * (e.g. libraries, OS) into a CHIP_ERROR.
 *
 * CHIP SDK errors inside Range::kSDK consist of a component identifier given by SdkPart and an arbitrary small
 * integer Code.
 */
class ChipError
{
public:
    /// Internal representation of an error.
    using StorageType = uint32_t;

    /// Type for encapsulated error values.
    using ValueType = StorageType;

    /// Integer `printf` format for errors. This is a C macro in order to allow for string literal concatenation.
#define CHIP_ERROR_INTEGER_FORMAT PRIx32

#if CHIP_CONFIG_ERROR_FORMAT_AS_STRING

    /// Type returned by Format().
    using FormatType = const char *;
    /// `printf` format for Format(). This is a C macro in order to allow for string literal concatenation.
#define CHIP_ERROR_FORMAT "s"

#else // CHIP_CONFIG_ERROR_FORMAT_AS_STRING

    /// Type returned by Format().
    using FormatType = StorageType;
    /// `printf` format for Format(). This is a C macro in order to allow for string literal concatenation.
#define CHIP_ERROR_FORMAT CHIP_ERROR_INTEGER_FORMAT

#endif // CHIP_CONFIG_ERROR_FORMAT_AS_STRING

    /**
     * Top-level error classification.
     *
     * Every error belongs to a Range and has an integral Value whose meaning depends on the Range.
     * All native CHIP SDK errors belong to the kSDK range. Other ranges are used to encapsulate error
     * codes from other subsystems (e.g. platform or library) used by the CHIP SDK.
     */
    enum class Range : uint8_t
    {
        kSDK        = 0x0, ///< CHIP SDK errors.
        kOS         = 0x1, ///< Encapsulated OS errors, other than POSIX errno.
        kPOSIX      = 0x2, ///< Encapsulated POSIX errno values.
        kLwIP       = 0x3, ///< Encapsulated LwIP errors.
        kOpenThread = 0x4, ///< Encapsulated OpenThread errors.
        kPlatform   = 0x5, ///< Platform-defined encapsulation.
        kLastRange  = kPlatform,
    };

    /**
     * Secondary classification of CHIP SDK errors (Range::kSDK).
     */
    enum class SdkPart : uint8_t
    {
        kCore            = 0, ///< SDK core errors.
        kInet            = 1, ///< Inet layer errors; see <inet/InetError.h>.
        kDevice          = 2, ///< Device layer errors; see <platform/CHIPDeviceError.h>.
        kASN1            = 3, ///< ASN1 errors; see <asn1/ASN1Error.h>.
        kBLE             = 4, ///< BLE layer errors; see <ble/BleError.h>.
        kIMGlobalStatus  = 5, ///< Interaction Model global status code.
        kIMClusterStatus = 6, ///< Interaction Model cluster-specific status code.
        kApplication     = 7, ///< Application-defined errors; see CHIP_APPLICATION_ERROR
    };

    ChipError() = default;

    // Helper for declaring constructors without too much repetition.
#if CHIP_CONFIG_ERROR_SOURCE
#define CHIP_INITIALIZE_ERROR_SOURCE(f, l) , mFile((f)), mLine((l))
#else // CHIP_CONFIG_ERROR_SOURCE
#define CHIP_INITIALIZE_ERROR_SOURCE(f, l)
#endif // CHIP_CONFIG_ERROR_SOURCE

    /**
     * Construct a CHIP_ERROR encapsulating @a value inside the Range @a range.
     *
     * @note
     *  The result is valid only if CanEncapsulate() is true.
     */
    constexpr ChipError(Range range, ValueType value) :
        mError(MakeInteger(range, (value & MakeMask(0, kValueLength)))) CHIP_INITIALIZE_ERROR_SOURCE(nullptr, 0)
    {}
    constexpr ChipError(Range range, ValueType value, const char * file, unsigned int line) :
        mError(MakeInteger(range, (value & MakeMask(0, kValueLength)))) CHIP_INITIALIZE_ERROR_SOURCE(file, line)
    {}

    /**
     * Construct a CHIP_ERROR for SdkPart @a part with @a code.
     *
     * @note
     *  The macro version CHIP_SDK_ERROR checks that the numeric value is constant and well-formed.
     */
    constexpr ChipError(SdkPart part, uint8_t code) : mError(MakeInteger(part, code)) CHIP_INITIALIZE_ERROR_SOURCE(nullptr, 0) {}
    constexpr ChipError(SdkPart part, uint8_t code, const char * file, unsigned int line) :
        mError(MakeInteger(part, code)) CHIP_INITIALIZE_ERROR_SOURCE(file, line)
    {}

    /**
     * Construct a CHIP_ERROR constant for SdkPart @a part with @a code at the current source line.
     * This checks that the numeric value is constant and well-formed.
     * (In C++20 this could be replaced by a consteval constructor.)
     */
#if CHIP_CONFIG_ERROR_SOURCE
#define CHIP_SDK_ERROR(part, code)                                                                                                 \
    (::chip::ChipError(::chip::ChipError::SdkErrorConstant<(part), (code)>::value, __FILE__, __LINE__))
#else // CHIP_CONFIG_ERROR_SOURCE
#define CHIP_SDK_ERROR(part, code) (::chip::ChipError(::chip::ChipError::SdkErrorConstant<(part), (code)>::value))
#endif // CHIP_CONFIG_ERROR_SOURCE

    /**
     * Construct a CHIP_ERROR from the underlying storage type.
     *
     * @note
     *  This is intended to be used only in foreign function interfaces.
     */
    explicit constexpr ChipError(StorageType error) : mError(error) CHIP_INITIALIZE_ERROR_SOURCE(nullptr, 0) {}
    explicit constexpr ChipError(StorageType error, const char * file, unsigned int line) :
        mError(error) CHIP_INITIALIZE_ERROR_SOURCE(file, line)
    {}

#undef CHIP_INITIALIZE_ERROR_SOURCE

    /**
     * Compare errors for equality.
     *
     * @note
     *  This only compares the error code. Under the CHIP_CONFIG_ERROR_SOURCE configuration, errors compare equal
     *  if they have the same error code, even if they have different source locations.
     */
    bool operator==(const ChipError & other) const { return mError == other.mError; }
    bool operator!=(const ChipError & other) const { return mError != other.mError; }

    /**
     * Return an integer code for the error.
     */
    constexpr StorageType AsInteger() const { return mError; }

    /*
     * IsSuccess() is intended to support macros that can take either a ChipError or an integer error code.
     * The latter follows the C convention that a non-zero integer indicates an error.
     *
     * @note
     *  Normal code should use `status == CHIP_NO_ERROR` rather than `IsSuccess(status)`.
     */
    static constexpr bool IsSuccess(ChipError error) { return error.mError == 0; }
    static constexpr bool IsSuccess(StorageType error) { return error == 0; }

    /**
     * Format an @a error for printing.
     *
     * Normally, this is used with the `printf()`-style macro CHIP_ERROR_FORMAT.
     * For example,
     *  @code
     *      ChipLogError(subsystem, "A bad thing happened! %" CHIP_ERROR_FORMAT, status.Format());
     *  @endcode
     */
#if CHIP_CONFIG_ERROR_FORMAT_AS_STRING
    FormatType Format() const { return AsString(); }
#else  // CHIP_CONFIG_ERROR_FORMAT_AS_STRING
    FormatType Format() const { return mError; }
#endif // CHIP_CONFIG_ERROR_FORMAT_AS_STRING

    /**
     * Format an @a error as a string for printing.
     *
     * @note
     *  Normally, prefer to use Format()
     */
    const char * AsString() const
    {
        extern const char * ErrorStr(ChipError);
        return ErrorStr(*this);
    }

    /**
     * Test whether @a error belongs to the Range @a range.
     */
    constexpr bool IsRange(Range range) const
    {
        return (mError & MakeMask(kRangeStart, kRangeLength)) == MakeField(kRangeStart, static_cast<StorageType>(range));
    }

    /**
     * Get the Range to which the @a error belongs.
     */
    constexpr Range GetRange() const { return static_cast<Range>(GetField(kRangeStart, kRangeLength, mError)); }

    /**
     * Get the encapsulated value of an @a error.
     */
    constexpr ValueType GetValue() const { return GetField(kValueStart, kValueLength, mError); }

    /**
     * Test whether type @a T can always be losslessly encapsulated in a CHIP_ERROR.
     */
    template <typename T>
    static constexpr bool CanEncapsulate()
    {
        return std::numeric_limits<typename std::make_unsigned_t<T>>::digits <= kValueLength;
    }

    /**
     * Test whether if @a value can be losslessly encapsulated in a CHIP_ERROR.
     */
    template <typename T>
    static constexpr bool CanEncapsulate(T value)
    {
        return CanEncapsulate<T>() || FitsInField(kValueLength, static_cast<ValueType>(value));
    }

    /**
     * Test whether @a error is an SDK error belonging to the SdkPart @a part.
     */
    constexpr bool IsPart(SdkPart part) const
    {
        return (mError & (MakeMask(kRangeStart, kRangeLength) | MakeMask(kSdkPartStart, kSdkPartLength))) ==
            (MakeField(kRangeStart, static_cast<StorageType>(Range::kSDK)) |
             MakeField(kSdkPartStart, static_cast<StorageType>(part)));
    }

    /**
     * Get the SDK code for an SDK error.
     */
    constexpr uint8_t GetSdkCode() const { return static_cast<uint8_t>(GetField(kSdkCodeStart, kSdkCodeLength, mError)); }

    /**
     * Test whether @a error is an SDK error representing an Interaction Model
     * status.  If it is, it can be converted to/from an interaction model
     * StatusIB struct.
     */
    constexpr bool IsIMStatus() const
    {
        // Open question: should CHIP_NO_ERROR be treated as an IM status for
        // purposes of this test?
        return IsPart(SdkPart::kIMGlobalStatus) || IsPart(SdkPart::kIMClusterStatus);
    }

#if CHIP_CONFIG_ERROR_SOURCE

    /**
     * Get the source file name of the point where the error occurred.
     *
     * @note
     *  This will be `nullptr` if the error was not created with a file name.
     */
    const char * GetFile() const { return mFile; }

    /**
     * Get the source line number of the point where the error occurred.
     *
     * @note
     *  This will be 0 if the error was not created with a file name.
     */
    unsigned int GetLine() const { return mLine; }

#endif // CHIP_CONFIG_ERROR_SOURCE

private:
    /*
     * The representation of a CHIP_ERROR is structured so that SDK error code constants are small, in order to improve code
     * density on embedded builds. Arm 32, Xtensa, and RISC-V can all handle 11-bit values in a move-immediate instruction.
     * Further, SdkPart::kCore is 0 so that the most common errors fit in 8 bits for additional density on some processors.
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

    static constexpr StorageType GetField(unsigned int start, unsigned int length, StorageType value)
    {
        return (value >> start) & ((1u << length) - 1);
    }
    static constexpr StorageType MakeMask(unsigned int start, unsigned int length) { return ((1u << length) - 1) << start; }
    static constexpr StorageType MakeField(unsigned int start, StorageType value) { return value << start; }
    static constexpr bool FitsInField(unsigned int length, StorageType value) { return value < (1u << length); }

    static constexpr StorageType MakeInteger(Range range, StorageType value)
    {
        return MakeField(kRangeStart, to_underlying(range)) | MakeField(kValueStart, value);
    }
    static constexpr StorageType MakeInteger(SdkPart part, uint8_t code)
    {
        return MakeInteger(Range::kSDK, MakeField(kSdkPartStart, to_underlying(part)) | MakeField(kSdkCodeStart, code));
    }
    template <unsigned int START, unsigned int LENGTH>
    struct MaskConstant
    {
        static constexpr StorageType value = ((1u << LENGTH) - 1) << START;
    };

    // Assert that Range and Value fields fit in StorageType and don't overlap.
    static_assert(kRangeStart + kRangeLength <= std::numeric_limits<StorageType>::digits, "Range does not fit in StorageType");
    static_assert(kValueStart + kValueLength <= std::numeric_limits<StorageType>::digits, "Value does not fit in StorageType");
    static_assert((MaskConstant<kRangeStart, kRangeLength>::value & MaskConstant<kValueStart, kValueLength>::value) == 0,
                  "Range and Value overlap");

    // Assert that SDK Part and Code fields fit in SdkCode field and don't overlap.
    static_assert(kSdkPartStart + kSdkPartLength <= kValueLength, "SdkPart does not fit in Value");
    static_assert(kSdkCodeStart + kSdkCodeLength <= kValueLength, "SdkCode does not fit in Value");
    static_assert((MaskConstant<kSdkPartStart, kSdkPartLength>::value & MaskConstant<kSdkCodeStart, kSdkCodeLength>::value) == 0,
                  "SdkPart and SdkCode overlap");

    // Assert that Value fits in ValueType.
    static_assert(kValueStart + kValueLength <= std::numeric_limits<ValueType>::digits, "Value does not fit in ValueType");

    StorageType mError;

#if CHIP_CONFIG_ERROR_SOURCE
    const char * mFile;
    unsigned int mLine;
#endif // CHIP_CONFIG_ERROR_SOURCE

public:
    /**
     * Helper for constructing error constants.
     *
     * This template ensures that the numeric value is constant and well-formed.
     */
    template <SdkPart PART, StorageType CODE>
    struct SdkErrorConstant
    {
        static_assert(FitsInField(kSdkPartLength, to_underlying(PART)), "part is too large");
        static_assert(FitsInField(kSdkCodeLength, CODE), "code is too large");
        static_assert(MakeInteger(PART, CODE) != 0, "value is zero");
        static constexpr StorageType value = MakeInteger(PART, CODE);
    };
};

} // namespace chip

/**
 *  The basic type for all CHIP errors.
 */
using CHIP_ERROR = ::chip::ChipError;

/**
 * Applications using the CHIP SDK can use this to define error codes in the CHIP_ERROR space for their own purposes.
 * This is suitable for a small fixed set of errors, similar to `CHIP_ERROR_…` constants. For embedding arbitrary or
 * larger values, use a custom Range offset from Range::kLastRange.
 */
#define CHIP_APPLICATION_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kApplication, (e))

#define CHIP_CORE_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kCore, (e))

#define CHIP_IM_GLOBAL_STATUS(type)                                                                                                \
    CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kIMGlobalStatus,                                                                    \
                   ::chip::to_underlying(::chip::Protocols::InteractionModel::Status::type))

//
// type must be a compile-time constant as mandated by CHIP_SDK_ERROR.
//
#define CHIP_IM_CLUSTER_STATUS(type) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kIMClusterStatus, type)

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
#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_SOURCE_NO_ERROR
#define CHIP_NO_ERROR                                          CHIP_ERROR(0, __FILE__, __LINE__)
#else // CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_SOURCE_NO_ERROR
#define CHIP_NO_ERROR                                          CHIP_ERROR(0)
#endif // CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_SOURCE_NO_ERROR

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
 *    Recursion depth overflow
 */
#define CHIP_ERROR_RECURSION_DEPTH_LIMIT                       CHIP_CORE_ERROR(0x05)

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

// AVAILABLE: 0x12

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

// AVAILABLE: 0x15
// AVAILABLE: 0x16

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

// AVAILABLE: 0x20

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

// AVAILABLE: 0x28
// AVAILABLE: 0x29

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

// AVAILABLE: 0x2c

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

// AVAILABLE: 0x34
// AVAILABLE: 0x35
// AVAILABLE: 0x36
// AVAILABLE: 0x37

/**
 *  @def CHIP_ERROR_INVALID_PASE_PARAMETER
 *
 *  @brief
 *    A PASE parameter is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PASE_PARAMETER                      CHIP_CORE_ERROR(0x38)

// AVAILABLE: 0x39
// AVAILABLE: 0x3a

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

// AVAILABLE: 0x3f
// AVAILABLE: 0x40

/**
 *  @def CHIP_ERROR_HOST_PORT_LIST_EMPTY
 *
 *  @brief
 *    A host/port list is empty.
 *
 */
#define CHIP_ERROR_HOST_PORT_LIST_EMPTY                        CHIP_CORE_ERROR(0x41)

// AVAILABLE: 0x42
// AVAILABLE: 0x43
// AVAILABLE: 0x44

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

// AVAILABLE: 0x49

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

// AVAILABLE: 0x52

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

// AVAILABLE: 0x55

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

// AVAILABLE: 0x58

/**
 *  @def CHIP_ERROR_WRONG_CERT_DN
 *
 *  @brief
 *    A certificate subject/issuer distinguished name is wrong.
 *
 */
#define CHIP_ERROR_WRONG_CERT_DN                               CHIP_CORE_ERROR(0x59)

// AVAILABLE: 0x5a
// AVAILABLE: 0x5b

/**
 *  @def CHIP_ERROR_WRONG_NODE_ID
 *
 *  @brief
 *    A node id is wrong.
 *
 */
#define CHIP_ERROR_WRONG_NODE_ID                               CHIP_CORE_ERROR(0x5c)

// AVAILABLE: 0x5d
// AVAILABLE: 0x5e
// AVAILABLE: 0x5f
// AVAILABLE: 0x60
// AVAILABLE: 0x61
// AVAILABLE: 0x62
// AVAILABLE: 0x63

/**
 *  @def CHIP_ERROR_RETRANS_TABLE_FULL
 *
 *  @brief
 *    A retransmission table is already full.
 *
 */
#define CHIP_ERROR_RETRANS_TABLE_FULL                          CHIP_CORE_ERROR(0x64)

// AVAILABLE: 0x65
// AVAILABLE: 0x66
// AVAILABLE: 0x67

/**
 *  @def CHIP_ERROR_TRANSACTION_CANCELED
 *
 *  @brief
 *    A transaction is cancelled.
 *
 */
#define CHIP_ERROR_TRANSACTION_CANCELED                        CHIP_CORE_ERROR(0x68)

// AVAILABLE: 0x69
// AVAILABLE: 0x6a

/**
 *  @def CHIP_ERROR_INVALID_SUBSCRIPTION
 *
 *  @brief
 *    A message was received as part of a subscription exchange that has a mis-matching subscription id.
 *
 */
#define CHIP_ERROR_INVALID_SUBSCRIPTION                               CHIP_CORE_ERROR(0x6b)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *
 *  @brief
 *    A CHIP feature is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE                    CHIP_CORE_ERROR(0x6c)

/**
 *  @def CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR
 *
 *  @brief
 *    An unsolicited message with the originator bit clear.
 *
 */
#define CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR               CHIP_CORE_ERROR(0x70)

/**
 *  @def CHIP_ERROR_INVALID_FABRIC_INDEX
 *
 *  @brief
 *    A fabric index is invalid.
 *
 */
#define CHIP_ERROR_INVALID_FABRIC_INDEX                        CHIP_CORE_ERROR(0x71)

/**
 *  @def CHIP_ERROR_TOO_MANY_CONNECTIONS
 *
 *  @brief
 *    The attempt to allocate a connection object failed because too many
 *    connections exist.
 *
 */
#define CHIP_ERROR_TOO_MANY_CONNECTIONS                        CHIP_CORE_ERROR(0x72)

/**
 * @def CHIP_ERROR_SHUT_DOWN
 *
 * @brief
 *   The operation cancelled because a shut down was initiated
 */
#define CHIP_ERROR_SHUT_DOWN                                   CHIP_CORE_ERROR(0x73)

/**
 * @def CHIP_ERROR_CANCELLED
 *
 * @brief
 *   The operation has been cancelled, generally by calling a cancel/abort request.
 */
#define CHIP_ERROR_CANCELLED                                   CHIP_CORE_ERROR(0x74)

// AVAILABLE: 0x75

/**
 *  @def CHIP_ERROR_TLV_TAG_NOT_FOUND
 *
 *  @brief
 *    A specified TLV tag was not found.
 *
 */
#define CHIP_ERROR_TLV_TAG_NOT_FOUND                           CHIP_CORE_ERROR(0x76)

/**
 * @def CHIP_ERROR_MISSING_SECURE_SESSION
 *
 * @brief
 *
 *  A secure session is needed to do work, but is missing/is not present.
 */
#define CHIP_ERROR_MISSING_SECURE_SESSION                      CHIP_CORE_ERROR(0x77)

/**
 * @def CHIP_ERROR_INVALID_ADMIN_SUBJECT
 *
 * @brief
 *   The CaseAdminSubject field is not valid in AddNOC command.
 *
 */
#define CHIP_ERROR_INVALID_ADMIN_SUBJECT                      CHIP_CORE_ERROR(0x78)

/**
 * @def CHIP_ERROR_INSUFFICIENT_PRIVILEGE
 *
 * @brief
 *   Required privilege was insufficient during an operation.
 *
 */
#define CHIP_ERROR_INSUFFICIENT_PRIVILEGE                          CHIP_CORE_ERROR(0x79)

// AVAILABLE: 0x7a
// AVAILABLE: 0x7b
// AVAILABLE: 0x7c

/**
 * @def CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED
 *
 * @brief
 *   The message counter of the session is exhausted, the session should be closed.
 */
#define CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED                 CHIP_CORE_ERROR(0x7d)

/**
 *  @def CHIP_ERROR_FABRIC_EXISTS
 *
 *  @brief
 *    The fabric with the given fabric id and root public key already exists.
 *
 */
#define CHIP_ERROR_FABRIC_EXISTS                               CHIP_CORE_ERROR(0x7e)

// AVAILABLE: 0x7f

/**
 *  @def CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    The wrong encryption type error received from a peer node.
 *
 */
#define CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER             CHIP_CORE_ERROR(0x80)

// AVAILABLE: 0x81
// AVAILABLE: 0x82
// AVAILABLE: 0x83
// AVAILABLE: 0x84

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

// AVAILABLE: 0x87
// AVAILABLE: 0x88
// AVAILABLE: 0x89
// AVAILABLE: 0x8a
// AVAILABLE: 0x8b
// AVAILABLE: 0x8c
// AVAILABLE: 0x8d

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

// AVAILABLE: 0x90
// AVAILABLE: 0x91

/**
 *  @def CHIP_ERROR_BAD_REQUEST
 *
 *  @brief
 *    The request cannot be processed or fulfilled
 *
 */
#define CHIP_ERROR_BAD_REQUEST                                 CHIP_CORE_ERROR(0x92)

// AVAILABLE: 0x93
// AVAILABLE: 0x94
// AVAILABLE: 0x95
// AVAILABLE: 0x96
// AVAILABLE: 0x97
// AVAILABLE: 0x98
// AVAILABLE: 0x99
// AVAILABLE: 0x9a
// AVAILABLE: 0x9b
// AVAILABLE: 0x9c

/**
 * @def CHIP_ERROR_WRONG_CERT_TYPE
 *
 * @brief
 *   The presented certificate was of the wrong type.
 */
#define CHIP_ERROR_WRONG_CERT_TYPE                             CHIP_CORE_ERROR(0x9d)

// AVAILABLE: 0x9e

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
 * @def CHIP_ERROR_IM_FABRIC_DELETED
 *
 * @brief
 *   The fabric is deleted, and the corresponding IM resources are released
 */
#define CHIP_ERROR_IM_FABRIC_DELETED                            CHIP_CORE_ERROR(0xa1)

// AVAILABLE: 0xa2
// AVAILABLE: 0xa3

/**
 * @def CHIP_ERROR_IN_PROGRESS
 *
 * @brief
 *   The operation is still in progress
 */
#define CHIP_ERROR_IN_PROGRESS                                 CHIP_CORE_ERROR(0xa4)

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

// AVAILABLE: 0xa8
// AVAILABLE: 0xa9
// AVAILABLE: 0xaa

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

// AVAILABLE: 0xb1
// AVAILABLE: 0xb2
// AVAILABLE: 0xb3

/**
 *  @def CHIP_ERROR_MDNS_COLLISION
 *
 *  @brief
 *    The registered service name has collision on the LAN.
 *
 */
#define CHIP_ERROR_MDNS_COLLISION                             CHIP_CORE_ERROR(0xb4)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB
 *
 * @brief
 *   The Attribute path IB is malformed: it does not contain
 *   the required path
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB                CHIP_CORE_ERROR(0xb5)

/**
 * @def CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB
 *
 * @brief
 *   The Event Path IB is malformed: it does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB                     CHIP_CORE_ERROR(0xb6)

// AVAILABLE: 0xb7
// AVAILABLE: 0xb8

/**
 * @def CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB
 *
 * @brief
 *   The Command Data IB is malformed: it does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB           CHIP_CORE_ERROR(0xb9)

/**
 * @def CHIP_ERROR_IM_MALFORMED_EVENT_DATA_IB
 *
 * @brief
 *   The Event Data IB is malformed: it does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_EVENT_DATA_IB             CHIP_CORE_ERROR(0xba)

// AVAILABLE: 0xbb

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

// AVAILABLE: 0xbe

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
 * @def CHIP_ERROR_INVALID_PUBLIC_KEY
 *
 * @brief
 *   The received public key doesn't match locally generated key.
 */
#define CHIP_ERROR_INVALID_PUBLIC_KEY                          CHIP_CORE_ERROR(0xc5)

/**
 * @def CHIP_ERROR_FABRIC_MISMATCH_ON_ICA
 *
 * @brief
 *   The fabric ID in ICA certificate doesn't match the one in NOC.
 */
#define CHIP_ERROR_FABRIC_MISMATCH_ON_ICA                      CHIP_CORE_ERROR(0xc6)

// AVAILABLE: 0xc7
// AVAILABLE: 0xc8

/**
 * @def CHIP_ERROR_NO_SHARED_TRUSTED_ROOT
 *
 * @brief
 *   The CASE session could not be established as peer's credentials do not have
 *   a common root of trust.
 */
#define CHIP_ERROR_NO_SHARED_TRUSTED_ROOT                      CHIP_CORE_ERROR(0xc9)

/*
 * @def CHIP_ERROR_IM_STATUS_CODE_RECEIVED
 *
 * @brief
 *   Indicates an IM status code was received. Usually accompanied with
 *   the actual IM status code.
 */
#define CHIP_ERROR_IM_STATUS_CODE_RECEIVED                     CHIP_CORE_ERROR(0xca)

// AVAILABLEL 0xcb
// AVAILABLEL 0xcc
// AVAILABLEL 0xcd
// AVAILABLEL 0xce
// AVAILABLEL 0xcf
// AVAILABLEL 0xd0
// AVAILABLEL 0xd1
// AVAILABLEL 0xd2
// AVAILABLEL 0xd3
// AVAILABLEL 0xd4
// AVAILABLEL 0xd5
// AVAILABLEL 0xd6

/**
 * @def CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB
 *
 * @brief
 *   The Data Version Filter IB is malformed: it does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB             CHIP_CORE_ERROR(0xd7)

/**
 * @def CHIP_ERROR_NOT_FOUND
 *
 * @brief
 *   The item referenced in the function call was not found
 */
#define CHIP_ERROR_NOT_FOUND                                       CHIP_CORE_ERROR(0xd8)

// AVAILABLE: 0xd9

/**
 * @def CHIP_ERROR_INVALID_FILE_IDENTIFIER
 *
 * @brief
 *   The file identifier, encoded in the first few bytes of a processed file,
 *   has unexpected value.
 */
#define CHIP_ERROR_INVALID_FILE_IDENTIFIER                     CHIP_CORE_ERROR(0xda)

/**
 * @def CHIP_ERROR_BUSY
 *
 * @brief
 *   The Resource is busy and cannot process the request. Trying again might work.
 */
#define CHIP_ERROR_BUSY                                        CHIP_CORE_ERROR(0xdb)

/**
  * @def CHIP_ERROR_MAX_RETRY_EXCEEDED
  *
  * @brief
  *   The maximum retry limit has been exceeded.
  */
 #define CHIP_ERROR_MAX_RETRY_EXCEEDED                         CHIP_CORE_ERROR(0xdc)

 /**
  * @def CHIP_ERROR_PROVIDER_LIST_EXHAUSTED
  *
  * @brief
  *   The provider list has been exhausted.
  */
 #define CHIP_ERROR_PROVIDER_LIST_EXHAUSTED                    CHIP_CORE_ERROR(0xdd)

// AVAILABLE: 0xde

/**
 * @def CHIP_ERROR_INVALID_SCHEME_PREFIX
 *
 * @brief
 *   The scheme field contains an invalid prefix
 */
#define CHIP_ERROR_INVALID_SCHEME_PREFIX                      CHIP_CORE_ERROR(0xdf)

/**
 * @def CHIP_ERROR_MISSING_URI_SEPARATOR
 *
 * @brief
 *   The URI separator is missing
 */
#define CHIP_ERROR_MISSING_URI_SEPARATOR                      CHIP_CORE_ERROR(0xe0)

/**
 * @def CHIP_ERROR_HANDLER_NOT_SET
 *
 * @brief
 *   Callback function or callable object is not set
 */
#define CHIP_ERROR_HANDLER_NOT_SET                            CHIP_CORE_ERROR(0xe1)

// WHEN ADDING A NEW ERROR CODE: Look for "AVAILABLE" comments above and fill in gaps.

// clang-format on

// !!!!! IMPORTANT !!!!!  If you add new CHIP errors, please update the translation
// of error codes to strings in CHIPError.cpp, and add them to kTestElements[]
// in core/tests/TestCHIPErrorStr.cpp

namespace chip {

extern void RegisterCHIPLayerErrorFormatter();
extern bool FormatCHIPError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} // namespace chip
