/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
 *      This file contains definitions for working with data encoded in CHIP TLV format.
 *
 *      CHIP TLV (Tag-Length-Value) is a generalized encoding method for simple structured data. It
 *      shares many properties with the commonly used JSON serialization format while being considerably
 *      more compact over the wire.
 */

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include "TLVCommon.h"

#include "TLVWriter.h"

/**
 * @namespace chip::TLV
 *
 * Definitions for working with data encoded in CHIP TLV format.
 *
 * CHIP TLV is a generalized encoding method for simple structured data. It shares many properties
 * with the commonly used JSON serialization format while being considerably more compact over the wire.
 */

namespace chip {
namespace TLV {

/**
 * Provides a memory efficient parser for data encoded in CHIP TLV format.
 *
 * TLVReader implements a forward-only, “pull-style” parser for CHIP TLV data.  The TLVReader
 * object operates as a cursor that can be used to iterate over a sequence of TLV elements
 * and interpret their contents.  When positioned on an element, applications can make calls
 * to the reader's Get() methods to query the current element’s type and tag, and to extract
 * any associated value.  The reader’s Next() method is used to advance from element to element.
 *
 * A TLVReader object is always positioned either before, on or after a TLV element.  When first
 * initialized, a TLVReader is positioned immediately before the first element of the encoding.
 * To begin reading, an application must make an initial call to the Next() method to position
 * the reader on the first element.  When a container element is encountered--either a structure,
 * an array or a path--the OpenContainer() or EnterContainer() methods can be used to iterate
 * through the contents of the container.
 *
 * When the reader reaches the end of a TLV encoding, or the last element within a container,
 * it signals the application by returning a CHIP_END_OF_TLV error from the Next() method.
 * The reader will continue to return CHIP_END_OF_TLV until it is reinitialized, or the current
 * container is exited (via CloseContainer() / ExitContainer()).
 *
 * A TLVReader object can parse data directly from a fixed input buffer, or from memory provided
 * by a TLVBackingStore.
 */
class DLL_EXPORT TLVReader
{
    friend class TLVWriter;
    friend class TLVUpdater;

public:
    /**
     * Initializes a TLVReader object from another TLVReader object.
     *
     * @param[in]   aReader  A read-only reference to the TLVReader to initialize
     *                       this from.
     *
     */
    void Init(const TLVReader & aReader);

    /**
     * Initializes a TLVReader object to read from a single input buffer.
     *
     * @param[in]   data    A pointer to a buffer containing the TLV data to be parsed.
     * @param[in]   dataLen The length of the TLV data to be parsed.
     *
     */
    void Init(const uint8_t * data, size_t dataLen);

    /**
     * Initializes a TLVReader object to read from a single input buffer
     * represented as a span.
     *
     * @param[in]   data    A byte span to read from
     *
     */
    void Init(const ByteSpan & data) { Init(data.data(), data.size()); }

    /**
     * Initializes a TLVReader object to read from a single input buffer
     * represented as byte array.
     *
     * @param[in]   data    A byte buffer to read from
     *
     */
    template <size_t N>
    void Init(const uint8_t (&data)[N])
    {
        Init(data, N);
    }

    /**
     * Initializes a TLVReader object to read from a TLVBackingStore.
     *
     * Parsing begins at the backing store's start position and continues until the
     * end of the data in the buffer, or maxLen bytes have been parsed.
     *
     * @param[in]   backingStore    A reference to a TLVBackingStore providing the TLV data to be parsed.
     * @param[in]   maxLen          The maximum number of bytes to parse. Defaults to the amount of data
     *                              in the input buffer.
     *
     * @retval #CHIP_NO_ERROR  If the method succeeded.
     * @retval other           Other error codes returned by TLVBackingStore::OnInit().
     */
    CHIP_ERROR Init(TLVBackingStore & backingStore, uint32_t maxLen = UINT32_MAX);

    /**
     * Advances the TLVReader object to the next TLV element to be read.
     *
     * The Next() method positions the reader object on the next element in a TLV encoding that resides
     * in the same containment context.  In particular, if the reader is positioned at the outer-most
     * level of a TLV encoding, calling Next() will advance the reader to the next, top-most element.
     * If the reader is positioned within a TLV container element (a structure, array or path), calling
     * Next() will advance the reader to the next member element of the container.
     *
     * Since Next() constrains reader motion to the current containment context, calling Next() when
     * the reader is positioned on a container element will advance @em over the container, skipping
     * its member elements (and the members of any nested containers) until it reaches the first element
     * after the container.
     *
     * When there are no further elements within a particular containment context the Next() method will
     * return a #CHIP_END_OF_TLV error and the position of the reader will remain unchanged.
     *
     * @retval #CHIP_NO_ERROR              If the reader was successfully positioned on a new element.
     * @retval #CHIP_END_OF_TLV            If no further elements are available.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV element
     *                                      type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval #CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG
     *                                      If the reader encountered a implicitly-encoded TLV tag for which
     *                                      the corresponding profile id is unknown.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR Next();

    /**
     * Advances the TLVReader object to the next TLV element to be read, asserting the tag of
     * the new element.
     *
     * The Next(Tag expectedTag) method is a convenience method that has the
     * same behavior as Next(), but also verifies that the tag of the new TLV element matches
     * the supplied argument.
     *
     * @param[in] expectedTag               The expected tag for the next element.
     *
     * @retval #CHIP_NO_ERROR              If the reader was successfully positioned on a new element.
     * @retval #CHIP_END_OF_TLV            If no further elements are available.
     * @retval #CHIP_ERROR_UNEXPECTED_TLV_ELEMENT
     *                                      If the tag associated with the new element does not match the
     *                                      value of the @p expectedTag argument.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV
     *                                      element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR Next(Tag expectedTag);

    /**
     * Advances the TLVReader object to the next TLV element to be read, asserting the type and tag of
     * the new element.
     *
     * The Next(TLVType expectedType, Tag expectedTag) method is a convenience method that has the
     * same behavior as Next(), but also verifies that the type and tag of the new TLV element match
     * the supplied arguments.
     *
     * @param[in] expectedType              The expected data type for the next element.
     * @param[in] expectedTag               The expected tag for the next element.
     *
     * @retval #CHIP_NO_ERROR              If the reader was successfully positioned on a new element.
     * @retval #CHIP_END_OF_TLV            If no further elements are available.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the type of the new element does not match the value
     *                                      of the @p expectedType argument.
     * @retval #CHIP_ERROR_UNEXPECTED_TLV_ELEMENT
     *                                      If the tag associated with the new element does not match the
     *                                      value of the @p expectedTag argument.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV
     *                                      element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR Next(TLVType expectedType, Tag expectedTag);

    /**
     * Returns the type of the current TLV element.
     *
     * @return      A TLVType value describing the data type of the current TLV element.  If the reader
     *              is not positioned on a TLV element, the return value will be kTLVType_NotSpecified.
     */
    TLVType GetType() const;

    /**
     * Returns the tag associated with current TLV element.
     *
     * The value returned by GetTag() can be used with the tag utility functions (IsProfileTag(),
     * IsContextTag(), ProfileIdFromTag(), etc.) to determine the type of tag and to extract various tag
     * field values.
     *
     * @note If the reader is not positioned on a TLV element when GetTag() is called, the return value
     * is undefined. Therefore whenever the position of the reader is uncertain applications should call
     * GetType() to determine if the reader is position on an element (GetType() != kTLVType_NotSpecified)
     * before calling GetTag().
     *
     * @return      An unsigned integer containing information about the tag associated with the current
     *              TLV element.
     */
    Tag GetTag() const { return mElemTag; }

    /**
     * Returns the length of data associated with current TLV element.
     *
     * Data length only applies to elements of type UTF8 string or byte string.  For UTF8 strings, the
     * value returned is the number of bytes in the string, not the number of characters.
     *
     * @return      The length (in bytes) of data associated with the current TLV element, or 0 if the
     *              current element is not a UTF8 string or byte string, or if the reader is not
     *              positioned on an element.
     */
    uint32_t GetLength() const;

    /**
     * Returns the control byte associated with current TLV element.
     *
     * Ideally, nobody ever needs to know about the control byte and only the
     * internal implementation of TLV should have access to it. But, nevertheless,
     * having access to the control byte is helpful for debugging purposes by the
     * TLV Debug Utilities (that try to decode the tag control byte when pretty
     * printing the TLV buffer contents).
     *
     * @note Unless you really know what you are doing, please refrain from using
     * this method and the associated control byte information.
     *
     * @return      An unsigned integer containing the control byte associated with
     *              the current TLV element. kTLVControlByte_NotSpecified is
     *              returned if the reader is not positioned @em on an element.
     */
    uint16_t GetControlByte() const { return mControlByte; }

    /**
     * Get the value of the current element as a bool type.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV boolean type, or the
     *                                      reader is not positioned on an element.
     */
    CHIP_ERROR Get(bool & v);

    /**
     * Get the value of the current element as an 8-bit signed integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(int8_t & v);

    /**
     * Get the value of the current element as a 16-bit signed integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(int16_t & v);

    /**
     * Get the value of the current element as a 32-bit signed integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(int32_t & v);

    /**
     * Get the value of the current element as a 64-bit signed integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(int64_t & v);

    /**
     * Get the value of the current element as an 8-bit unsigned integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.  Similarly, if the encoded integer value is negative, the value will be converted
     * to unsigned.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(uint8_t & v);

    /**
     * Get the value of the current element as a 16-bit unsigned integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.  Similarly, if the encoded integer value is negative, the value will be converted
     * to unsigned.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(uint16_t & v);

    /**
     * Get the value of the current element as a 32-bit unsigned integer.
     *
     * If the encoded integer value is larger than the output data type the resultant value will be
     * truncated.  Similarly, if the encoded integer value is negative, the value will be converted
     * to unsigned.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(uint32_t & v);

    /**
     * Get the value of the current element as a 64-bit unsigned integer.
     *
     * If the encoded integer value is negative, the value will be converted to unsigned.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV integer type (signed or
     *                                      unsigned), or the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(uint64_t & v);

    /**
     * Get the value of the current element as a double-precision floating point number.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV floating point type, or
     *                                      the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(double & v);

    /**
     * Get the value of the current element as a single-precision floating point number.
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV floating point type, or
     *                                      the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(float & v);

    /**
     * Get the value of the current element as a ByteSpan
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV bytes array, or
     *                                      the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(ByteSpan & v);

    /**
     * Get the value of the current element as a FixedByteSpan
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV bytes array, or
     *                                      the reader is not positioned on an element.
     *
     */
    template <size_t N>
    CHIP_ERROR Get(FixedByteSpan<N> & v)
    {
        const uint8_t * val;
        ReturnErrorOnFailure(GetDataPtr(val));
        VerifyOrReturnError(GetLength() == N, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
        v = FixedByteSpan<N>(val);
        return CHIP_NO_ERROR;
    }

    /**
     * Get the value of the current element as a CharSpan
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV character string, or
     *                                      the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(CharSpan & v);

    /**
     * Get the Localized String Identifier contained in the current element..
     *
     * The method takes what's after the first Information Separator 1 <IS1>, and until end of string
     * or second <IS1>, and return the hex-decoded string identifier, if one was there.
     *
     * @param[out]  lsid                       Optional Localized String Identifier. Returns empty
     *                                         if the value is not found or it was invalidly encoded.
     *
     * @retval #CHIP_NO_ERROR                  If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE      If the current element is not a TLV character string, or
     *                                         the reader is not positioned on an element.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT If the Localized String Identifier is malformed.
     */
    CHIP_ERROR Get(Optional<LocalizedStringIdentifier> & lsid);

    /**
     * Get the value of the current element as an enum value, if it's an integer
     * value that fits in the enum type.
     *
     * @param[out] v Receives the value associated with current TLV element.
     */
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    CHIP_ERROR Get(T & v)
    {
        std::underlying_type_t<T> val;
        ReturnErrorOnFailure(Get(val));
        v = static_cast<T>(val);
        return CHIP_NO_ERROR;
    }

    /**
     * Get the value of the current element as a BitFlags value, if it's an integer
     * value that fits in the BitFlags type.
     *
     * @param[out] v Receives the value associated with current TLV element.
     */
    template <typename T>
    CHIP_ERROR Get(BitFlags<T> & v)
    {
        std::underlying_type_t<T> val;
        ReturnErrorOnFailure(Get(val));
        v.SetRaw(val);
        return CHIP_NO_ERROR;
    }

    /**
     * Get the value of the current element as a BitMask value, if it's an integer
     * value that fits in the BitMask type.
     *
     * @param[out] v Receives the value associated with current TLV element.
     */
    template <typename T>
    CHIP_ERROR Get(BitMask<T> & v)
    {
        std::underlying_type_t<T> val;
        ReturnErrorOnFailure(Get(val));
        v.SetRaw(val);
        return CHIP_NO_ERROR;
    }

    /**
     * Get the value of the current byte or UTF8 string element.
     *
     * To determine the required input buffer size, call the GetLength() method before calling GetBytes().
     *
     * @note The data output by this method is NOT null-terminated.
     *
     * @param[in]  buf                      A pointer to a buffer to receive the string data.
     * @param[in]  bufSize                  The size in bytes of the buffer pointed to by @p buf.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV byte or UTF8 string, or
     *                                      the reader is not positioned on an element.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                                      If the supplied buffer is too small to hold the data associated
     *                                      with the current element.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR GetBytes(uint8_t * buf, size_t bufSize);

    /**
     * Allocates and returns a buffer containing the value of the current byte or UTF8 string.
     *
     * This method creates a buffer for and returns a copy of the data associated with the byte
     * or UTF-8 string element at the current position. Memory for the buffer is obtained with
     * Platform::MemoryAlloc() and should be freed with Platform::MemoryFree() by the caller when
     * it is no longer needed.
     *
     * @note The data returned by this method is NOT null-terminated.
     *
     * @param[out] buf                      A reference to a pointer to which a heap-allocated buffer of
     *                                      @p dataLen bytes will be assigned on success.
     * @param[out] dataLen                  A reference to storage for the size, in bytes, of @p buf on
     *                                      success.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV byte or UTF8 string, or
     *                                      the reader is not positioned on an element.
     * @retval #CHIP_ERROR_NO_MEMORY       If memory could not be allocated for the output buffer.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR DupBytes(uint8_t *& buf, uint32_t & dataLen);

    /**
     * Get the value of the current byte or UTF8 string element as a null terminated string.
     *
     * To determine the required input buffer size, call the GetLength() method before calling GetBytes().
     * The input buffer should be at least one byte bigger than the string length to accommodate the null
     * character.
     *
     * @param[in]  buf                      A pointer to a buffer to receive the byte string data.
     * @param[in]  bufSize                  The size in bytes of the buffer pointed to by @p buf.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV byte or UTF8 string, or
     *                                      the reader is not positioned on an element.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                                      If the supplied buffer is too small to hold the data associated
     *                                      with the current element.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR GetString(char * buf, size_t bufSize);

    /**
     * Allocates and returns a buffer containing the null-terminated value of the current byte or UTF8
     * string.
     *
     * This method creates a buffer for and returns a null-terminated copy of the data associated with
     * the byte or UTF-8 string element at the current position. Memory for the buffer is obtained with
     * Platform::MemoryAlloc() and should be freed with chip::Platform::MemoryFree() by the caller when
     * it is no longer needed.
     *
     * @param[out] buf                      A reference to a pointer to which a heap-allocated buffer of
     *                                      will be assigned on success.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV byte or UTF8 string, or
     *                                      the reader is not positioned on an element.
     * @retval #CHIP_ERROR_NO_MEMORY       If memory could not be allocated for the output buffer.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval other                       Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR DupString(char *& buf);

    /**
     * Get a pointer to the initial encoded byte of a TLV byte or UTF8 string element.
     *
     * This method returns a direct pointer to the encoded string value within the underlying input buffer
     * if a non-zero length string payload is present. To succeed, the method requires that the entirety of the
     * string value be present in a single buffer. Otherwise the method returns #CHIP_ERROR_TLV_UNDERRUN.
     * This makes the method of limited use when reading data from multiple discontiguous buffers.
     *
     * If no string data is present (i.e the length is zero), data shall be updated to point to null.
     *
     * @param[out] data                     A reference to a const pointer that will receive a pointer to
     *                                      the underlying string data.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV byte or UTF8 string, or the
     *                                      reader is not positioned on an element.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely or the value
     *                                      of the current string element is not contained within a single
     *                                      contiguous buffer.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR GetDataPtr(const uint8_t *& data);

    /**
     * Prepares a TLVReader object for reading the members of TLV container element.
     *
     * The EnterContainer() method prepares the current TLVReader object to begin reading the member
     * elements of a TLV container (a structure, array or path). For every call to EnterContainer()
     * applications must make a corresponding call to ExitContainer().
     *
     * When EnterContainer() is called the TLVReader object must be positioned on the container element
     * to be read.  The method takes as an argument a reference to a TLVType value which will be used
     * to save the context of the reader while it is reading the container.
     *
     * When the EnterContainer() method returns, the reader is positioned immediately @em before the
     * first member of the container. Repeatedly calling Next() will advance the reader through the members
     * of the collection until the end is reached, at which point the reader will return CHIP_END_OF_TLV.
     *
     * Once the application has finished reading a container it can continue reading the elements after
     * the container by calling the ExitContainer() method.
     *
     * @param[out] outerContainerType       A reference to a TLVType value that will receive the context
     *                                      of the reader.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE If the current element is not positioned on a container element.
     *
     */
    CHIP_ERROR EnterContainer(TLVType & outerContainerType);

    /**
     * Completes the reading of a TLV container and prepares a TLVReader object to read elements
     * after the container.
     *
     * The ExitContainer() method restores the state of a TLVReader object after a call to
     * EnterContainer().  For every call to EnterContainer() applications must make a corresponding
     * call to ExitContainer(), passing the context value returned by the EnterContainer() method.
     *
     * When ExitContainer() returns, the reader is positioned immediately before the first element that
     * follows the container.  From this point an application can use the Next() method to advance
     * through any remaining elements.
     *
     * Once EnterContainer() has been called, applications can call ExitContainer() on a reader at any
     * point in time, regardless of whether all elements in the underlying container have been read.
     *
     * @note Any changes made to the configuration of the reader between the calls to EnterContainer()
     * and ExitContainer() are NOT undone by the call to ExitContainer().  For example, a change to the
     * implicit profile id (@p ImplicitProfileId) will not be reversed when a container is exited.  Thus
     * it is the application's responsibility to adjust the configuration accordingly at the appropriate
     * times.
     *
     * @param[in] outerContainerType        The TLVType value that was returned by the EnterContainer() method.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE If OpenContainer() has not been called on the reader, or if
     *                                      the container reader does not match the one passed to the
     *                                      OpenContainer() method.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR ExitContainer(TLVType outerContainerType);

    /**
     * Initializes a new TLVReader object for reading the members of a TLV container element.
     *
     * The OpenContainer() method initializes a new TLVReader object for reading the member elements of a
     * TLV container (a structure, array or path).  When OpenContainer() is called, the current TLVReader
     * object must be positioned on the container element to be read.  The method takes as its sole argument
     * a reference to a new reader that will be initialized to read the container.  This reader is known as
     * the <em>container reader</em> while the reader on which OpenContainer() is called is known as the <em>parent
     * reader</em>.
     *
     * When the OpenContainer() method returns, the container reader is positioned immediately before the
     * first member of the container. Calling Next() on the container reader will advance through the members
     * of the collection until the end is reached, at which point the reader will return CHIP_END_OF_TLV.
     *
     * While the container reader is open, applications must not make calls on or otherwise alter the state
     * of the parent reader.  Once an application has finished using the container reader it must close it
     * by calling CloseContainer() on the parent reader, passing the container reader as an argument.
     * Applications may close the container reader at any point, with or without reading all elements
     * contained in the underlying container. After the container reader is closed, applications may
     * continue their use of the parent reader.
     *
     * The container reader inherits various configuration properties from the parent reader.  These are:
     *
     * @li The implicit profile id (ImplicitProfileId)
     * @li The application data pointer (AppData)
     * @li The GetNextBuffer function pointer
     *
     * @note The EnterContainer() method can be used as an alternative to OpenContainer() to read a
     * container element without initializing a new reader object.
     *
     * @param[out] containerReader          A reference to a TLVReader object that will be initialized for
     *                                      reading the members of the current container element. Any data
     *                                      associated with the supplied object is overwritten.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE If the current element is not positioned on a container element.
     *
     */
    CHIP_ERROR OpenContainer(TLVReader & containerReader);

    /**
     * Completes the reading of a TLV container after a call to OpenContainer().
     *
     * The CloseContainer() method restores the state of a parent TLVReader object after a call to
     * OpenContainer().  For every call to OpenContainer() applications must make a corresponding
     * call to CloseContainer(), passing a reference to the same container reader to both methods.
     *
     * When CloseContainer() returns, the parent reader is positioned immediately before the first
     * element that follows the container.  From this point an application can use the Next() method
     * to advance through any remaining elements.
     *
     * Applications can call close CloseContainer() on a parent reader at any point in time, regardless
     * of whether all elements in the underlying container have been read. After CloseContainer() has
     * been called, the application should consider the container reader 'de-initialized' and must not
     * use it further without re-initializing it.
     *
     * @param[in] containerReader           A reference to the TLVReader object that was supplied to the
     *                                      OpenContainer() method.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE If OpenContainer() has not been called on the reader, or if
     *                                      the container reader does not match the one passed to the
     *                                      OpenContainer() method.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV
     *                                      element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR CloseContainer(TLVReader & containerReader);

    /**
     * Returns the type of the container within which the TLVReader is currently reading.
     *
     * The GetContainerType() method returns the type of the TLV container within which the TLVReader
     * is reading.  If the TLVReader is positioned at the outer-most level of a TLV encoding (i.e. before,
     * on or after the outer-most TLV element), the method will return kTLVType_NotSpecified.
     *
     * @return  The TLVType of the current container, or kTLVType_NotSpecified if the TLVReader is not
     *          positioned within a container.
     */
    TLVType GetContainerType() const { return mContainerType; }

    /**
     * Verifies that the TLVReader object is at the end of a TLV container.
     *
     * The VerifyEndOfContainer() method verifies that there are no further TLV elements to be read
     * within the current TLV container.  This is a convenience method that is equivalent to calling
     * Next() and checking for a return value of CHIP_END_OF_TLV.
     *
     * @note When there are more TLV elements in the collection, this method will change the position
     * of the reader.
     *
     * @retval #CHIP_NO_ERROR              If there are no further TLV elements to be read.
     * @retval #CHIP_ERROR_UNEXPECTED_TLV_ELEMENT
     *                                      If another TLV element was found in the collection.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV element
     *                                      type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR VerifyEndOfContainer();

    /**
     * Returns the total number of bytes read since the reader was initialized.
     *
     * @return Total number of bytes read since the reader was initialized.
     */
    uint32_t GetLengthRead() const { return mLenRead; }

    /**
     * Returns the total number of bytes that can be read until the max read length is reached.
     *
     * @return Total number of bytes that can be read until the max read length is reached.
     */
    uint32_t GetRemainingLength() const { return mMaxLen - mLenRead; }

    /**
     * Return the total number of bytes for the TLV data
     * @return the total number of bytes for the TLV data
     */
    uint32_t GetTotalLength() const { return mMaxLen; }

    /**
     * Returns the stored backing store.
     *
     * @return the stored backing store.
     */
    TLVBackingStore * GetBackingStore() { return mBackingStore; }

    /**
     * Gets the point in the underlying input buffer that corresponds to the reader's current position.
     *
     * @note Depending on the type of the current element, GetReadPoint() will return a pointer that
     * is some number of bytes *after* the first byte of the element.  For string types (UTF8 and byte
     * strings), the pointer will point to the first byte of the string's value.  For container types
     * (structures, arrays and paths), the pointer will point to the first member element within the
     * container. For all other types, the pointer will point to the byte immediately after the element's
     * encoding.
     *
     * @return A pointer into underlying input buffer that corresponds to the reader's current position.
     */
    const uint8_t * GetReadPoint() const { return mReadPoint; }

    /**
     * Advances the TLVReader object to immediately after the current TLV element.
     *
     * The Skip() method positions the reader object immediately @em after the current TLV element, such
     * that a subsequent call to Next() will advance the reader to the following element.  Like Next(),
     * if the reader is positioned on a container element at the time of the call, the members of the
     * container will be skipped.  If the reader is not positioned on any element, its position remains
     * unchanged.
     *
     * @retval #CHIP_NO_ERROR              If the reader was successfully positioned on a new element.
     * @retval #CHIP_END_OF_TLV            If no further elements are available.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV
     *                                      element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      TLVBackingStore.
     *
     */
    CHIP_ERROR Skip();

    /**
     * Position the destination reader on the next element with the given tag within this reader's current container context
     *
     * @param[in] tagInApiForm             The destination context tag value
     * @param[in] destReader               The destination TLV reader value that was located by given tag
     *
     * @retval #CHIP_NO_ERROR              If the reader was successfully positioned at the given tag
     * @retval #CHIP_END_OF_TLV            If the given tag cannot be found
     * @retval other                       Other CHIP or platform error codes
     */
    CHIP_ERROR FindElementWithTag(Tag tagInApiForm, TLVReader & destReader) const;

    /**
     * Count how many elements remain in the currently-open container.  Will
     * fail with CHIP_ERROR_INCORRECT_STATE if not currently in a container.
     *
     * @param[out] size On success, set to the number of items following the
     *                  current reader position in the container.
     */
    CHIP_ERROR CountRemainingInContainer(size_t * size) const;

    /**
     * The profile id to be used for profile tags encoded in implicit form.
     *
     * When the reader encounters a profile-specific tag that has been encoded in implicit form, it
     * uses the value of the @p ImplicitProfileId property as the assumed profile id for the tag.
     *
     * By default, the @p ImplicitProfileId property is set to kProfileIdNotSpecified. When decoding
     * TLV that contains implicitly-encoded tags, applications must set @p ImplicitProfileId prior
     * to reading any TLV elements having such tags.  The appropriate profile id is usually dependent
     * on the context of the application or protocol being spoken.
     *
     * If an implicitly-encoded tag is encountered while @p ImplicitProfileId is set to
     * kProfileIdNotSpecified, the reader will return a #CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG error.
     */
    uint32_t ImplicitProfileId;

    /**
     * A pointer field that can be used for application-specific data.
     */
    void * AppData;

protected:
    Tag mElemTag;
    uint64_t mElemLenOrVal;
    TLVBackingStore * mBackingStore;
    const uint8_t * mReadPoint;
    const uint8_t * mBufEnd;
    uint32_t mLenRead;
    uint32_t mMaxLen;
    TLVType mContainerType;
    uint16_t mControlByte;

private:
    bool mContainerOpen;

protected:
    bool IsContainerOpen() const { return mContainerOpen; }
    void SetContainerOpen(bool aContainerOpen) { mContainerOpen = aContainerOpen; }

    CHIP_ERROR ReadElement();
    void ClearElementState();
    CHIP_ERROR SkipData();
    CHIP_ERROR SkipToEndOfContainer();
    CHIP_ERROR VerifyElement();
    Tag ReadTag(TLVTagControl tagControl, const uint8_t *& p) const;
    CHIP_ERROR EnsureData(CHIP_ERROR noDataErr);
    CHIP_ERROR ReadData(uint8_t * buf, uint32_t len);
    CHIP_ERROR GetElementHeadLength(uint8_t & elemHeadBytes) const;
    TLVElementType ElementType() const;
};

/*
 * A TLVReader that is backed by a scoped memory buffer that is owned by the reader
 */
class ScopedBufferTLVReader : public TLVReader
{
public:
    /*
     * Construct and initialize the reader by taking ownership of the provided scoped buffer.
     */
    ScopedBufferTLVReader(Platform::ScopedMemoryBuffer<uint8_t> && buffer, size_t dataLen) { Init(std::move(buffer), dataLen); }

    ScopedBufferTLVReader() {}

    /*
     * Initialize the reader by taking ownership of a passed in scoped buffer.
     */
    void Init(Platform::ScopedMemoryBuffer<uint8_t> && buffer, size_t dataLen)
    {
        mBuffer = std::move(buffer);
        TLVReader::Init(mBuffer.Get(), dataLen);
    }

    /*
     * Take back the buffer owned by the reader and transfer its ownership to
     * the provided buffer reference. This also re-initializes the reader with
     * a null buffer to prevent further use of the reader.
     */
    void TakeBuffer(Platform::ScopedMemoryBuffer<uint8_t> & buffer)
    {
        buffer = std::move(mBuffer);
        TLVReader::Init(nullptr, 0);
    }

private:
    Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};

/**
 * A TLVReader that is guaranteed to be backed by a single contiguous buffer.
 * This allows it to expose some additional methods that allow consumers to
 * directly access the data in that buffer in a safe way that is guaranteed to
 * work as long as the reader object stays in scope.
 */
class ContiguousBufferTLVReader : public TLVReader
{
public:
    ContiguousBufferTLVReader() : TLVReader() {}

    /**
     * Init with input buffer as ptr + length pair.
     */
    void Init(const uint8_t * data, size_t dataLen) { TLVReader::Init(data, dataLen); }

    /**
     * Init with input buffer as ByteSpan.
     */
    void Init(const ByteSpan & data) { Init(data.data(), data.size()); }

    /**
     * Init with input buffer as byte array.
     */
    template <size_t N>
    void Init(const uint8_t (&data)[N])
    {
        Init(data, N);
    }

    /**
     * Allow opening a container, with a new ContiguousBufferTLVReader reading
     * that container.  See TLVReader::OpenContainer for details.
     */
    CHIP_ERROR OpenContainer(ContiguousBufferTLVReader & containerReader);

    /**
     * Get the value of the current UTF8 string as a Span<const char> pointing
     * into the TLV data.  Consumers may need to copy the data elsewhere as
     * needed (e.g. before releasing the reader and its backing buffer if they
     * plan to use the data after that point).
     *
     * @param[out] data                     A Span<const char> representing the string data.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV UTF8 string, or
     *                                      the reader is not positioned on an element.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely (i.e. the string length was "too big").
     *
     */
    CHIP_ERROR GetStringView(Span<const char> & data);

    /**
     * Get the value of the current octet string as a ByteSpan pointing into the
     * TLV data.  Consumers may need to copy the data elsewhere as needed
     * (e.g. before releasing the reader and its backing buffer if they plan to
     * use the data after that point).
     *
     * @param[out] data                     A ByteSpan representing the string data.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV octet string, or
     *                                      the reader is not positioned on an element.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely (i.e. the string length was "too big").
     *
     */
    CHIP_ERROR GetByteView(ByteSpan & data);
};

} // namespace TLV
} // namespace chip
