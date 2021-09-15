/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLVTags.h>
#include <lib/core/CHIPTLVTypes.h>

#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>

#include <stdarg.h>
#include <stdlib.h>
#include <type_traits>

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

inline uint8_t operator|(TLVElementType lhs, TLVTagControl rhs)
{
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

inline uint8_t operator|(TLVTagControl lhs, TLVElementType rhs)
{
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

enum
{
    kTLVControlByte_NotSpecified = 0xFFFF
};

class TLVBackingStore;

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
     * Advances the TLVReader object to the next TLV element to be read, asserting the type and tag of
     * the new element.
     *
     * The Next(TLVType expectedType, uint64_t expectedTag) method is a convenience method that has the
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
    CHIP_ERROR Next(TLVType expectedType, uint64_t expectedTag);

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
    uint64_t GetTag() const { return mElemTag; }

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
     * Get the value of the current element as a chip::ByteSpan
     *
     * @param[out]  v                       Receives the value associated with current TLV element.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV bytes array, or
     *                                      the reader is not positioned on an element.
     *
     */
    CHIP_ERROR Get(chip::ByteSpan & v);

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
     * This method returns a direct pointer the encoded string value within the underlying input buffer.
     * To succeed, the method requires that the entirety of the string value be present in a single buffer.
     * Otherwise the method returns #CHIP_ERROR_TLV_UNDERRUN.  This makes the method of limited use when
     * reading data from multiple discontiguous buffers.
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
    CHIP_ERROR FindElementWithTag(const uint64_t tagInApiForm, TLVReader & destReader) const;

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
    uint64_t mElemTag;
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
    uint64_t ReadTag(TLVTagControl tagControl, const uint8_t *& p);
    CHIP_ERROR EnsureData(CHIP_ERROR noDataErr);
    CHIP_ERROR ReadData(uint8_t * buf, uint32_t len);
    CHIP_ERROR GetElementHeadLength(uint8_t & elemHeadBytes) const;
    TLVElementType ElementType() const;
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

/**
 * Provides a memory efficient encoder for writing data in CHIP TLV format.
 *
 * TLVWriter implements a forward-only, stream-style encoder for CHIP TLV data.  Applications
 * write data to an encoding by calling one of the writer's Put() methods, passing associated
 * tag and value information as necessary.  Similarly applications can encode TLV container types
 * (structures, arrays or paths) by calling the writer's OpenContainer() or EnterContainer()
 * methods.
 *
 * A TLVWriter object can write data directly to a fixed output buffer, or to memory provided by
 * a TLVBackingStore.
 */
class DLL_EXPORT TLVWriter
{
    friend class TLVUpdater;

public:
    /**
     * Initializes a TLVWriter object to write into a single output buffer.
     *
     * @note Applications must call Finalize() on the writer before using the contents of the output
     * buffer.
     *
     * @param[in]   buf     A pointer to the buffer into which TLV should be written.
     * @param[in]   maxLen  The maximum number of bytes that should be written to the output buffer.
     *
     */
    void Init(uint8_t * buf, size_t maxLen);

    /**
     * Initializes a TLVWriter object to write into a single output buffer
     * represented by a MutableSpan.  See documentation for the two-arg Init()
     * form for details.
     *
     */
    void Init(const MutableByteSpan & data) { Init(data.data(), data.size()); }

    /**
     * Initializes a TLVWriter object to write into a single output buffer
     * represented by a fixed-size byte array.  See documentation for the
     * two-arg Init() form for details.
     *
     */
    template <size_t N>
    void Init(uint8_t (&data)[N])
    {
        Init(data, N);
    }

    /**
     * Initializes a TLVWriter object to write into memory provided by a TLVBackingStore.
     *
     * @note Applications must call Finalize() on the writer before using the contents of the buffer.
     *
     * @param[in]   backingStore    A TLVBackingStore providing memory, which must outlive the TVLWriter.
     * @param[in]   maxLen          The maximum number of bytes that should be written to the output buffer.
     *
     * @retval #CHIP_NO_ERROR  If the method succeeded.
     * @retval other           Other error codes returned by TLVBackingStore::OnInit().
     */
    CHIP_ERROR Init(TLVBackingStore & backingStore, uint32_t maxLen = UINT32_MAX);

    /**
     * Finish the writing of a TLV encoding.
     *
     * The Finalize() method completes the process of writing a TLV encoding to the underlying output
     * buffer.  The method must be called by the application before it uses the contents of the buffer.
     * Finalize() can only be called when there are no container writers open for the current writer.
     * (See @p OpenContainer()).
     *
     * @retval #CHIP_NO_ERROR      If the encoding was finalized successfully.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              FinalizeBuffer() function.
     */
    CHIP_ERROR Finalize();

    /**
     * Encodes a TLV signed integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR Put(uint64_t tag, int8_t v);

    /**
     * Encodes a TLV signed integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     * @param[in]   preserveSize    True if the value should be encoded in the same number of bytes as
     *                              at the input type.  False if value should be encoded in the minimum
     *                              number of bytes necessary to represent the value.  Note: Applications
     *                              are strongly encouraged to set this parameter to false.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR Put(uint64_t tag, int8_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
     */
    CHIP_ERROR Put(uint64_t tag, int16_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(uint64_t tag, int16_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
     */
    CHIP_ERROR Put(uint64_t tag, int32_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(uint64_t tag, int32_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
     */
    CHIP_ERROR Put(uint64_t tag, int64_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(uint64_t tag, int64_t v, bool preserveSize);

    /**
     * Encodes a TLV unsigned integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag. Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR Put(uint64_t tag, uint8_t v);

    /**
     * Encodes a TLV unsigned integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     * @param[in]   preserveSize    True if the value should be encoded in the same number of bytes as
     *                              at the input type.  False if value should be encoded in the minimum
     *                              number of bytes necessary to represent the value.  Note: Applications
     *                              are strongly encouraged to set this parameter to false.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR Put(uint64_t tag, uint8_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
     */
    CHIP_ERROR Put(uint64_t tag, uint16_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(uint64_t tag, uint16_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
     */
    CHIP_ERROR Put(uint64_t tag, uint32_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(uint64_t tag, uint32_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
     */
    CHIP_ERROR Put(uint64_t tag, uint64_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(uint64_t tag, uint64_t v, bool preserveSize);

    /**
     * Encodes a TLV floating point value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR Put(uint64_t tag, double v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, double v)
     */
    CHIP_ERROR Put(uint64_t tag, float v);

    /**
     * Encodes a TLV byte string value using ByteSpan class.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   data            A ByteSpan object containing the bytes string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR Put(uint64_t tag, ByteSpan data);

    /**
     * static_cast to enumerations' underlying type when data is an enumeration.
     */
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    CHIP_ERROR Put(uint64_t tag, T data)
    {
        return Put(tag, to_underlying(data));
    }

    /**
     * Encodes a TLV boolean value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR PutBoolean(uint64_t tag, bool v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, bool v)
     */
    CHIP_ERROR Put(uint64_t tag, bool v)
    {
        /*
         * In TLV, boolean values are encoded as standalone tags without actual values, so we have a seperate
         * PutBoolean method.
         */
        return PutBoolean(tag, v);
    }

    /**
     * Encodes a TLV byte string value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   buf             A pointer to a buffer containing the bytes string to be encoded.
     * @param[in]   len             The number of bytes to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR PutBytes(uint64_t tag, const uint8_t * buf, uint32_t len);

    /**
     * Encodes a TLV UTF8 string value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   buf             A pointer to the null-terminated UTF-8 string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR PutString(uint64_t tag, const char * buf);

    /**
     * Encodes a TLV UTF8 string value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   buf             A pointer to the UTF-8 string to be encoded.
     * @param[in]   len             The length (in bytes) of the string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR PutString(uint64_t tag, const char * buf, uint32_t len);

    /**
     * @brief
     *   Encode the string output formatted according to the format in the TLV element.
     *
     * PutStringF is an analog of a sprintf where the output is stored in
     * a TLV element as opposed to a character buffer.  When extended
     * printf functionality is available, the function is able to output
     * the result string into a discontinuous underlying storage.  The
     * implementation supports the following printf enhancements:
     *
     * -- The platform supplies a callback-based `vcbprintf` that provides
     *    the ability to call a custom callback in place of putchar.
     *
     * -- The platform supplies a variant of `vsnprintf` called
     *    `vsnprintf_ex`, that behaves exactly like vsnprintf except it
     *    has provisions for omitting the first `n` characters of the
     *    output.
     *
     * Note that while the callback-based function may be the simplest and
     * use the least amount of code, the `vsprintf_ex` variety of
     * functions will consume less stack.
     *
     * If neither of the above is available, the function will allocate a
     * temporary buffer to hold the output, using Platform::MemoryAlloc().
     *
     * @param[in] tag The TLV tag to be encoded with the value, or @p
     *                AnonymousTag if the value should be encoded without
     *                a tag.  Tag values should be constructed with one of
     *                the tag definition functions ProfileTag(),
     *                ContextTag() or CommonTag().
     *
     * @param[in] fmt The format string used to format the argument list.
     *                Follows the same syntax and rules as the format
     *                string for `printf` family of functions.
     *
     * @param[in] ... A list of arguments to be formatted in the output value
     *                according to fmt.
     *
     * @retval #CHIP_NO_ERROR  If the method succeeded.
     *
     * @retval other If underlying calls to TLVWriter methods --
     *               `WriteElementHead` or `GetNewBuffer` -- failed, their
     *               error is immediately forwarded up the call stack.
     */
    CHIP_ERROR PutStringF(uint64_t tag, const char * fmt, ...);

    /**
     * @brief
     *   Encode the string output formatted according to the format in the TLV element.
     *
     * PutStringF is an analog of a sprintf where the output is stored in
     * a TLV element as opposed to a character buffer.  When extended
     * printf functionality is available, the function is able to output
     * the result string into a discontinuous underlying storage.  The
     * implementation supports the following printf enhancements:
     *
     * -- The platform supplies a callback-based `vcbprintf` that provides
     *    the ability to call a custom callback in place of putchar.
     *
     * -- The platform supplies a variant of `vsnprintf` called
     *    `vsnprintf_ex`, that behaves exactly like vsnprintf except it
     *    has provisions for omitting the first `n` characters of the
     *    output.
     *
     * Note that while the callback-based function may be the simplest and
     * use the least amount of code, the `vsprintf_ex` variety of
     * functions will consume less stack.
     *
     * If neither of the above is available, the function will allocate a
     * temporary buffer to hold the output, using Platform::MemoryAlloc().
     *
     * @param[in] tag The TLV tag to be encoded with the value, or @p
     *                AnonymousTag if the value should be encoded without
     *                a tag.  Tag values should be constructed with one of
     *                the tag definition functions ProfileTag(),
     *                ContextTag() or CommonTag().
     *
     * @param[in] fmt The format string used to format the argument list.
     *                Follows the same syntax and rules as the format
     *                string for `printf` family of functions.
     *
     * @param[in] ap A list of arguments to be formatted in the output value
     *                according to fmt.
     *
     * @retval #CHIP_NO_ERROR  If the method succeeded.
     *
     * @retval other If underlying calls to TLVWriter methods --
     *               `WriteElementHead` or `GetNewBuffer` -- failed, their
     *               error is immediately forwarded up the call stack.
     */
    CHIP_ERROR VPutStringF(uint64_t tag, const char * fmt, va_list ap);

    /**
     * Encodes a TLV null value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR PutNull(uint64_t tag);

    /**
     * Copies a TLV element from a reader object into the writer.
     *
     * The CopyElement() method encodes a new TLV element whose type, tag and value are taken from a TLVReader
     * object. When the method is called, the supplied reader object is expected to be positioned on the
     * source TLV element. The newly encoded element will have the same type, tag and contents as the input
     * container.  If the supplied element is a TLV container (structure, array or path), the entire contents
     * of the container will be copied.
     *
     * @note This method requires the supplied TVLReader object to be reading from a single, contiguous
     * input buffer that contains the entirety of the underlying TLV encoding. Supplying a reader in any
     * other mode has undefined behavior.
     *
     * @param[in]   reader          A reference to a TLVReader object identifying a pre-encoded TLV
     *                              element that should be copied.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If the supplied reader is not positioned on an element.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_TLV_UNDERRUN
     *                              If the underlying TLV encoding associated with the supplied reader ended
     *                              prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                              If the supplied reader encountered an invalid or unsupported TLV element
     *                              type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the supplied reader encountered a TLV tag in an invalid context,
     *                              or if the supplied tag is invalid or inappropriate in the context in
     *                              which the new container is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              GetNewBuffer() or FinalizeBuffer() functions, or by the GetNextBuffer()
     *                              function associated with the reader object.
     *
     */
    CHIP_ERROR CopyElement(TLVReader & reader);

    /**
     * Copies a TLV element from a reader object into the writer.
     *
     * The CopyElement() method encodes a new TLV element whose type and value are taken from a TLVReader
     * object. When the method is called, the supplied reader object is expected to be positioned on the
     * source TLV element. The newly encoded element will have the same type and contents as the input
     * container, however the tag will be set to the specified argument.  If the supplied element is a
     * TLV container (structure, array or path), the entire contents of the container will be copied.
     *
     * @note This method requires the supplied TVLReader object to be reading from a single, contiguous
     * input buffer that contains the entirety of the underlying TLV encoding. Supplying a reader in any
     * other mode has undefined behavior.
     *
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag if
     *                              the container should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   reader          A reference to a TLVReader object identifying a pre-encoded TLV
     *                              element whose type and value should be copied.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If the supplied reader is not positioned on an element.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_TLV_UNDERRUN
     *                              If the underlying TLV encoding associated with the supplied reader ended
     *                              prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                              If the supplied reader encountered an invalid or unsupported TLV element
     *                              type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the supplied reader encountered a TLV tag in an invalid context,
     *                              or if the supplied tag is invalid or inappropriate in the context in
     *                              which the new container is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              GetNewBuffer() or FinalizeBuffer() functions, or by the GetNextBuffer()
     *                              function associated with the reader object.
     *
     */
    CHIP_ERROR CopyElement(uint64_t tag, TLVReader & reader);

    /**
     * Begins encoding a new TLV container element.
     *
     * The StartContainer() method is used to write TLV container elements (structure, arrays or paths)
     * to an encoding.  The method takes the type and tag (if any) of the new container, and a reference
     * to a TLVType value which will be used to save the current context of the writer while it is being
     * used to write the container.
     *
     * Once the StartContainer() method returns, the application should use the current TLVWriter object to
     * write the elements of the container.  When finish, the application must call the EndContainer()
     * method to finish the encoding of the container.
     *
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag if
     *                              the container should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   containerType   The type of container to encode.  Must be one of @p kTLVType_Structure,
     *                              @p kTLVType_Array or @p kTLVType_List.
     * @param[out]  outerContainerType
     *                              A reference to a TLVType value that will receive the context of the
     *                              writer.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE
     *                              If the value specified for containerType is incorrect.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR StartContainer(uint64_t tag, TLVType containerType, TLVType & outerContainerType);

    /**
     * Completes the encoding of a TLV container element.
     *
     * The EndContainer() method completes the encoding of a TLV container element and restores the state
     * of a TLVWrite object after an earlier call to StartContainer().  For every call to StartContainer()
     * applications must make a corresponding call to EndContainer(), passing the TLVType value returned
     * by the StartContainer() call.  When EndContainer() returns, the writer object can be used to write
     * additional TLV elements that follow the container element.
     *
     * @note Any changes made to the configuration of the writer between the calls to StartContainer()
     * and EndContainer() are NOT undone by the call to EndContainer().  For example, a change to the
     * implicit profile id (@p ImplicitProfileId) will not be reversed when a container is ended.  Thus
     * it is the application's responsibility to adjust the configuration accordingly at the appropriate
     * times.
     *
     * @param[in] outerContainerType
     *                              The TLVType value that was returned by the StartContainer() method.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If a corresponding StartContainer() call was not made.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR EndContainer(TLVType outerContainerType);

    /**
     * Initializes a new TLVWriter object for writing the members of a TLV container element.
     *
     * The OpenContainer() method is used to write TLV container elements (structure, arrays or paths)
     * to an encoding.  The method takes the type and tag (if any) of the new container, and a reference
     * to a new writer object (the <em>container writer</em>) that will be initialized for the purpose
     * of writing the container's elements.  Applications write the members of the new container using
     * the container writer and then call CloseContainer() to complete the container encoding.
     *
     * While the container writer is open, applications must not make calls on or otherwise alter the state
     * of the parent writer.
     *
     * The container writer inherits various configuration properties from the parent writer.  These are:
     *
     * @li The implicit profile id (ImplicitProfileId)
     * @li The application data pointer (AppData)
     * @li The GetNewBuffer and FinalizeBuffer function pointers
     *
     * @note The StartContainer() method can be used as an alternative to OpenContainer() to write a
     * container element without initializing a new writer object.
     *
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag if
     *                              the container should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   containerType   The type of container to encode.  Must be one of @p kTLVType_Structure,
     *                              @p kTLVType_Array or @p kTLVType_List.
     * @param[out]  containerWriter A reference to a TLVWriter object that will be initialized for
     *                              writing the members of the new container element. Any data
     *                              associated with the supplied object is overwritten.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE
     *                              If the value specified for containerType is incorrect.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR OpenContainer(uint64_t tag, TLVType containerType, TLVWriter & containerWriter);

    /**
     * Completes the writing of a TLV container after a call to OpenContainer().
     *
     * The CloseContainer() method restores the state of a parent TLVWriter object after a call to
     * OpenContainer().  For every call to OpenContainer() applications must make a corresponding
     * call to CloseContainer(), passing a reference to the same container writer to both methods.
     *
     * When CloseContainer() returns, applications may continue to use the parent writer to write
     * additional TLV elements that appear after the container element.  At this point the supplied
     * container writer should be considered 'de-initialized' and must not be used without
     * re-initialization.
     *
     * @param[in] containerWriter   A reference to the TLVWriter object that was supplied to the
     *                              OpenContainer() method.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If the supplied container writer is not in the correct state.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If another container writer has been opened on the supplied
     *                              container writer and not yet closed.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If completing the encoding of the container would exceed the
     *                              limit on the maximum number of bytes specified when the writer
     *                              was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack
     *                              of memory.
     * @retval other                Other CHIP or platform-specific errors returned by the
     *                              configured TLVBackingStore.
     *
     */
    CHIP_ERROR CloseContainer(TLVWriter & containerWriter);

    /**
     * Encodes a TLV container element from a pre-encoded set of member elements
     *
     * The PutPreEncodedContainer() method encodes a new TLV container element (a structure, array or path)
     * containing a set of member elements taken from a pre-encoded buffer.  The input buffer is expected to
     * contain zero or more full-encoded TLV elements, with tags that conform to the rules associated with
     * the specified container type (e.g. structure members must have tags, while array members must not).
     *
     * The method encodes the entirety of the container element in one call.  When PutPreEncodedContainer()
     * returns, the writer object can be used to write additional TLV elements following the container element.
     *
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag if
     *                              the container should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   containerType   The type of container to encode.  Must be one of @p kTLVType_Structure,
     *                              @p kTLVType_Array or @p kTLVType_List.
     * @param[in]   data            A pointer to a buffer containing zero of more encoded TLV elements that
     *                              will become the members of the new container.
     * @param[in]   dataLen         The number of bytes in the @p data buffer.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_WRONG_TLV_TYPE
     *                              If the value specified for containerType is incorrect.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR PutPreEncodedContainer(uint64_t tag, TLVType containerType, const uint8_t * data, uint32_t dataLen);

    /**
     * Copies a TLV container element from TLVReader object
     *
     * The CopyContainer() encodes a new TLV container element by copying a pre-encoded container element
     * located at the current position of a TLVReader object. The method writes the entirety of the new
     * container element in one call, copying the container's type, tag and elements from the source
     * encoding. When the method returns, the writer object can be used to write additional TLV elements
     * following the container element.
     *
     * @note This method requires the supplied TVLReader object to be reading from a single, contiguous
     * input buffer that contains the entirety of the underlying TLV encoding.
     *
     * @param[in]   container       A reference to a TLVReader object identifying the pre-encoded TLV
     *                              container to be copied.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT
     *                              If the supplied reader uses a TLVBackingStore rather than a simple buffer.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If the supplied reader is not positioned on a container element.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_TLV_UNDERRUN
     *                              If the underlying TLV encoding associated with the supplied reader ended
     *                              prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                              If the supplied reader encountered an invalid or unsupported TLV element
     *                              type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the supplied reader encountered a TLV tag in an invalid context,
     *                              or if the tag associated with the source container is invalid or
     *                              inappropriate in the context in which the new container is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR CopyContainer(TLVReader & container);

    /**
     * Encodes a TLV container element from a pre-encoded set of member elements
     *
     * The CopyContainer() method encodes a new TLV container element (a structure, array or path)
     * containing a set of member elements taken from a TLVReader object. When the method is called, the
     * supplied reader object is expected to be positioned on a TLV container element. The newly encoded
     * container will have the same type and members as the input container.  The tag for the new
     * container is specified as an input parameter.
     *
     * When the method returns, the writer object can be used to write additional TLV elements following
     * the container element.
     *
     * @note This method requires the supplied TVLReader object to be reading from a single, contiguous
     * input buffer that contains the entirety of the underlying TLV encoding.
     *
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag if
     *                              the container should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   container       A reference to a TLVReader object identifying a pre-encoded TLV
     *                              container whose type and members should be copied.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT
     *                              If the supplied reader uses a TLVBackingStore rather than a simple buffer.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If the supplied reader is not positioned on a container element.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_TLV_UNDERRUN
     *                              If the underlying TLV encoding associated with the supplied reader ended
     *                              prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                              If the supplied reader encountered an invalid or unsupported TLV element
     *                              type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the supplied reader encountered a TLV tag in an invalid context,
     *                              or if the supplied tag is invalid or inappropriate in the context in
     *                              which the new container is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore.
     *
     */
    CHIP_ERROR CopyContainer(uint64_t tag, TLVReader & container);

    /**
     * Encodes a TLV container element that contains member elements from a pre-encoded container
     *
     * The CopyContainer() method encodes a new TLV container element (a structure, array or path)
     * containing a set of member elements taken from the contents of a supplied pre-encoded container.
     * When the method is called, data in the supplied input buffer is parsed as a TLV container element
     * an a new container is written that has the same type and members as the input container.  The tag
     * for the new container is specified as an input parameter.
     *
     * When the method returns, the writer object can be used to write additional TLV elements following
     * the container element.
     *
     * @param[in] tag                   The TLV tag to be encoded with the container, or @p AnonymousTag if
     *                                  the container should be encoded without a tag.  Tag values should be
     *                                  constructed with one of the tag definition functions ProfileTag(),
     *                                  ContextTag() or CommonTag().
     * @param[in] encodedContainer      A buffer containing a pre-encoded TLV container whose type and members
     *                                  should be copied.
     * @param[in] encodedContainerLen   The length in bytes of the pre-encoded container.
     *
     * @retval #CHIP_NO_ERROR          If the method succeeded.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                                  If a container writer has been opened on the current writer and not
     *                                  yet closed.
     * @retval #CHIP_ERROR_TLV_UNDERRUN
     *                                  If the encoded container ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                  If the encoded container contained an invalid or unsupported TLV element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                                  If the encoded container contained a TLV tag in an invalid context,
     *                                  or if the supplied tag is invalid or inappropriate in the context in
     *                                  which the new container is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                                  If writing the value would exceed the limit on the maximum number of
     *                                  bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                                  If an attempt to allocate an output buffer failed due to lack of
     *                                  memory.
     * @retval other                    Other CHIP or platform-specific errors returned by the configured
     *                                  TLVBackingStore.
     *
     */
    CHIP_ERROR CopyContainer(uint64_t tag, const uint8_t * encodedContainer, uint16_t encodedContainerLen);

    /**
     * Returns the type of container within which the TLVWriter is currently writing.
     *
     * The GetContainerType() method returns the type of the TLV container within which the TLVWriter
     * is currently writing.  If the TLVWriter is not writing elements within a container (i.e. if writing
     * at the outer-most level of an encoding) the method returns kTLVType_NotSpecified.
     *
     * @return  The TLVType of the current container, or kTLVType_NotSpecified if the TLVWriter is not
     *          writing elements within a container.
     */
    TLVType GetContainerType() const { return mContainerType; }

    /**
     * Returns the total number of bytes written since the writer was initialized.
     *
     * @return Total number of bytes written since the writer was initialized.
     */
    uint32_t GetLengthWritten() const { return mLenWritten; }

    /**
     * Returns the total remaining number of bytes for current tlv writer
     *
     * @return the total remaining number of bytes.
     */
    uint32_t GetRemainingFreeLength() const { return mRemainingLen; }
    /**
     * The profile id of tags that should be encoded in implicit form.
     *
     * When a writer is asked to encode a new element, if the profile id of the tag associated with the
     * new element matches the value of the @p ImplicitProfileId member, the writer will encode the tag
     * in implicit form, omitting the profile id in the process.
     *
     * By default, the @p ImplicitProfileId property is set to kProfileIdNotSpecified, which instructs
     * the writer not to emit implicitly encoded tags.  Applications can set @p ImplicitProfileId at any
     * time to enable encoding tags in implicit form starting at the current point in the encoding.  The
     * appropriate profile id to set is usually dependent on the context of the application or protocol
     * being spoken.
     *
     * @note The value of the @p ImplicitProfileId member affects the encoding of profile-specific
     * tags only; the encoding of context-specific tags is unchanged.
     */
    uint32_t ImplicitProfileId;

    /**
     * A pointer field that can be used for application-specific data.
     */
    void * AppData;

protected:
    TLVBackingStore * mBackingStore;
    uint8_t * mBufStart;
    uint8_t * mWritePoint;
    uint32_t mRemainingLen;
    uint32_t mLenWritten;
    uint32_t mMaxLen;
    TLVType mContainerType;

private:
    bool mContainerOpen;
    bool mCloseContainerReserved;

protected:
    bool IsContainerOpen() const { return mContainerOpen; }
    void SetContainerOpen(bool aContainerOpen) { mContainerOpen = aContainerOpen; }

    enum
    {
        kEndOfContainerMarkerSize = 1, /**< Size of the EndOfContainer marker, used in reserving space. */
    };

    /**
     * @brief
     *   Determine whether the container should reserve space for the
     *   CloseContainer symbol at the point of starting / opening the
     *   container.
     */
    bool IsCloseContainerReserved() const { return mCloseContainerReserved; }

    /**
     * @brief
     * Set whether the container should reserve the space for the
     * CloseContainer symbol at the point of starting / opening the
     * container.
     */
    void SetCloseContainerReserved(bool aCloseContainerReserved) { mCloseContainerReserved = aCloseContainerReserved; }

#if CONFIG_HAVE_VCBPRINTF
    static void CHIPTLVWriterPutcharCB(uint8_t c, void * appState);
#endif
    CHIP_ERROR WriteElementHead(TLVElementType elemType, uint64_t tag, uint64_t lenOrVal);
    CHIP_ERROR WriteElementWithData(TLVType type, uint64_t tag, const uint8_t * data, uint32_t dataLen);
    CHIP_ERROR WriteData(const uint8_t * p, uint32_t len);
};

/**
 * Provides a unified Reader/Writer interface for editing/adding/deleting elements in TLV encoding.
 *
 * The TLVUpdater is a union of the TLVReader and TLVWriter objects and provides interface methods
 * for editing/deleting data in an encoding as well as adding new elements to the TLV encoding. The
 * TLVUpdater object essentially acts like two cursors, one for reading existing encoding and
 * another for writing (either for copying over existing data or writing new data).
 *
 * Semantically, the TLVUpdater object functions like a union of the TLVReader and TLVWriter. The
 * TLVUpdater methods have more or less similar meanings as similarly named counterparts in
 * TLVReader/TLVWriter. Where there are differences in the semantics, the differences are clearly
 * documented in the function's comment section in CHIPTLVUpdater.cpp.
 *
 * One particularly important note about the TLVUpdater's PutBytes() and PutString() methods is that
 * it can leave the encoding in a corrupt state with only the element header written when an
 * overflow occurs. Applications can call GetRemainingFreeLength() to make sure there is
 * @em approximately enough free space to write the encoding. Note that GetRemainingFreeLength()
 * only tells you the available free bytes and there is @em no way for the application to know the
 * length of encoded data that gets written. In the event of an overflow, both PutBytes() and
 * PutString() will return CHIP_ERROR_BUFFER_TOO_SMALL to the caller.
 *
 * Also, note that Next() method is overloaded to both skip the current element and also advance the
 * internal reader to the next element. Because skipping already encoded elements requires changing
 * the internal writer's free space state variables to account for the new freed space (made
 * available by skipping), the application is expected to call Next() on the updater after a Get()
 * method whose value it doesn't wish to write back (which is equivalent to skipping the current
 * element).
 *
 * @note The application is expected to use the TLVUpdater object atomically from the time it calls
 * Init() till it calls Finalize(). The same buffer should NOT be used with other TLVWriter objects.
 *
 * @note The TLVUpdater currently only supports single static buffers. TLVBackingStore is NOT supported.
 */
class DLL_EXPORT TLVUpdater
{
public:
    /**
     * Initialize a TLVUpdater object to edit a single input buffer.
     *
     * On calling this method, the TLV data in the buffer is moved to the end of the
     * buffer and a private TLVReader object is initialized on this relocated
     * buffer. A private TLVWriter object is also initialized on the free space that
     * is now available at the beginning. Applications can use the TLVUpdater object
     * to parse the TLV data and modify/delete existing elements or add new elements
     * to the encoding.
     *
     * @param[in]   buf     A pointer to a buffer containing the TLV data to be edited.
     * @param[in]   dataLen The length of the TLV data in the buffer.
     * @param[in]   maxLen  The total length of the buffer.
     *
     * @retval #CHIP_NO_ERROR                  If the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT    If the buffer address is invalid.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL    If the buffer is too small.
     *
     */
    CHIP_ERROR Init(uint8_t * buf, uint32_t dataLen, uint32_t maxLen);

    /**
     * Initialize a TLVUpdater object using a TLVReader.
     *
     * On calling this method, TLV data in the buffer pointed to by the TLVReader
     * is moved from the current read point to the end of the buffer. A new
     * private TLVReader object is initialized to read from this new location, while
     * a new private TLVWriter object is initialized to write to the freed up buffer
     * space.
     *
     * Note that if the TLVReader is already positioned "on" an element, it is first
     * backed-off to the start of that element. Also note that this backing off
     * works well with container elements, i.e., if the TLVReader was already  used
     * to call EnterContainer(), then there is nothing to back-off. But if the
     * TLVReader was positioned on the container element and EnterContainer() was
     * not yet called, then the TLVReader object is backed-off to the start of the
     * container head.
     *
     * The input TLVReader object will be destroyed before returning and the
     * application must not make use of the same on return.
     *
     * @param[in,out]   aReader     Reference to a TLVReader object that will be
     *                              destroyed before returning.
     * @param[in]       freeLen     The length of free space (in bytes) available
     *                              in the pre-encoded data buffer.
     *
     * @retval #CHIP_NO_ERROR                  If the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT    If the buffer address is invalid.
     * @retval #CHIP_ERROR_NOT_IMPLEMENTED     If reader was initialized on a chain
     *                                          of buffers.
     */
    CHIP_ERROR Init(TLVReader & aReader, uint32_t freeLen);

    CHIP_ERROR Finalize() { return mUpdaterWriter.Finalize(); }

    // Common methods

    /**
     * Set the Implicit Profile ID for the TLVUpdater object.
     *
     * This method sets the implicit profile ID for the TLVUpdater object. When the
     * updater is asked to encode a new element, if the profile ID of the tag
     * associated with the new element matches the value of the @p profileId, the
     * updater will encode the tag in implicit form, thereby omitting the profile ID
     * in the process.
     *
     * @param[in]   profileId   The profile id of tags that should be encoded in
     *                          implicit form.
     */
    void SetImplicitProfileId(uint32_t profileId);
    uint32_t GetImplicitProfileId() { return mUpdaterReader.ImplicitProfileId; }

    /**
     * Copies the current element from input TLV to output TLV.
     *
     * The Move() method copies the current element on which the TLVUpdater's reader
     * is positioned on, to the TLVUpdater's writer. The application should call
     * Next() and position the TLVUpdater's reader on an element before calling this
     * method. Just like the TLVReader::Next() method, if the reader is positioned
     * on a container element at the time of the call, all the members of the
     * container will be copied. If the reader is not positioned on any element,
     * nothing changes on calling this method.
     *
     * @retval #CHIP_NO_ERROR              If the TLVUpdater reader was
     *                                      successfully positioned on a new
     *                                      element.
     * @retval #CHIP_END_OF_TLV            If the TLVUpdater's reader is pointing
     *                                      to end of container.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the TLVIpdater's reader is not
     *                                      positioned on a valid TLV element.
     * @retval other                        Returns other error codes returned by
     *                                      TLVReader::Skip() method.
     *
     */
    CHIP_ERROR Move();

    /**
     * Move everything from the TLVUpdater's current read point till end of input
     * TLV buffer over to output.
     *
     * This method supports moving everything from the TLVUpdater's current read
     * point till the end of the reader buffer over to the TLVUpdater's writer.
     *
     * @note This method can be called with the TLVUpdater's reader positioned
     * anywhere within the input TLV. The reader can also be positioned under
     * multiple levels of nested containers and this method will still work.
     *
     * @note This method also changes the state of the TLVUpdater object to a state
     * it would be in if the application had painstakingly parsed each element from
     * the current read point till the end of the input encoding and copied them to
     * the output TLV.
     */
    void MoveUntilEnd();

    /**
     * Prepares a TLVUpdater object for reading elements of a container. It also
     * encodes a start of container object in the output TLV.
     *
     * The EnterContainer() method prepares the current TLVUpdater object to begin
     * reading the member elements of a TLV container (a structure, array or path).
     * For every call to EnterContainer() applications must make a corresponding
     * call to ExitContainer().
     *
     * When EnterContainer() is called the TLVUpdater's reader must be positioned on
     * the container element. The method takes as an argument a reference to a
     * TLVType value which will be used to save the context of the updater while it
     * is reading the container.
     *
     * When the EnterContainer() method returns, the updater is positioned
     * immediately @em before the first member of the container. Repeatedly calling
     * Next() will advance the updater through the members of the collection until
     * the end is reached, at which point the updater will return CHIP_END_OF_TLV.
     *
     * Once the application has finished reading a container it can continue reading
     * the elements after the container by calling the ExitContainer() method.
     *
     * @note This method implicitly encodes a start of container element in the
     * output TLV buffer.
     *
     * @param[out] outerContainerType       A reference to a TLVType value that will
     *                                      receive the context of the updater.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE If the TLVUpdater reader is not
     *                                      positioned on a container element.
     * @retval other                        Any other CHIP or platform error code
     *                                      returned by TLVWriter::StartContainer()
     *                                      or TLVReader::EnterContainer().
     *
     */
    CHIP_ERROR EnterContainer(TLVType & outerContainerType);

    /**
     * Completes the reading of a TLV container element and encodes an end of TLV
     * element in the output TLV.
     *
     * The ExitContainer() method restores the state of a TLVUpdater object after a
     * call to EnterContainer(). For every call to EnterContainer() applications
     * must make a corresponding call to ExitContainer(), passing the context value
     * returned by the EnterContainer() method.
     *
     * When ExitContainer() returns, the TLVUpdater reader is positioned immediately
     * before the first element that follows the container in the input TLV. From
     * this point applications can call Next() to advance through any remaining
     * elements.
     *
     * Once EnterContainer() has been called, applications can call ExitContainer()
     * on the updater at any point in time, regardless of whether all elements in
     * the underlying container have been read. Also, note that calling
     * ExitContainer() before reading all the elements in the container, will result
     * in the updated container getting truncated in the output TLV.
     *
     * @note Any changes made to the configuration of the updater between the calls
     * to EnterContainer() and ExitContainer() are NOT undone by the call to
     * ExitContainer(). For example, a change to the implicit profile id
     * (@p ImplicitProfileId) will not be reversed when a container is exited. Thus
     * it is the application's responsibility to adjust the configuration
     * accordingly at the appropriate times.
     *
     * @param[in] outerContainerType        The TLVType value that was returned by
     *                                      the EnterContainer() method.
     *
     * @retval #CHIP_NO_ERROR              If the method succeeded.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended
     *                                      prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the updater encountered an invalid or
     *                                      unsupported TLV element type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the updater encountered a TLV tag in
     *                                      an invalid context.
     * @retval other                        Any other CHIP or platform error code
     *                                      returned by TLVWriter::EndContainer() or
     *                                      TLVReader::ExitContainer().
     *
     */
    CHIP_ERROR ExitContainer(TLVType outerContainerType);

    void GetReader(TLVReader & containerReader) { containerReader = mUpdaterReader; }

    // Reader methods

    /**
     * Skip the current element and advance the TLVUpdater object to the next
     * element in the input TLV.
     *
     * The Next() method skips the current element in the input TLV and advances the
     * TLVUpdater's reader to the next element that resides in the same containment
     * context. In particular, if the reader is positioned at the outer most level
     * of a TLV encoding, calling Next() will advance it to the next, top most
     * element. If the reader is positioned within a TLV container element (a
     * structure, array or path), calling Next() will advance it to the next member
     * element of the container.
     *
     * Since Next() constrains reader motion to the current containment context,
     * calling Next() when the reader is positioned on a container element will
     * advance @em over the container, skipping its member elements (and the members
     * of any nested containers) until it reaches the first element after the
     * container.
     *
     * When there are no further elements within a particular containment context
     * the Next() method will return a #CHIP_END_OF_TLV error and the position of
     * the reader will remain unchanged.
     *
     * @note The Next() method implicitly skips the current element. Hence, the
     * TLVUpdater's private writer state variables will be adjusted to account for
     * the new freed space (made available by skipping). This means that the
     * application is expected to call Next() on the TLVUpdater object after a Get()
     * whose value the application does @em not write back (which from the
     * TLVUpdater's view is equivalent to skipping that element).
     *
     * @note Applications are also expected to call Next() when they are at the end
     * of a container, and want to add new elements there. This is particularly
     * important in situations where there is a fixed schema. Applications that have
     * fixed schemas and know where the container end is cannot just add new
     * elements at the end, because the TLVUpdater writer's state will not reflect
     * the correct free space available for the Put() operation. Hence, applications
     * must call Next() (and possibly also test for CHIP_END_OF_TLV) before adding
     * elements at the end of a container.
     *
     * @retval #CHIP_NO_ERROR              If the TLVUpdater reader was
     *                                      successfully positioned on a new
     *                                      element.
     * @retval other                        Returns the CHIP or platform error
     *                                      codes returned by the TLVReader::Skip()
     *                                      and TLVReader::Next() method.
     *
     */
    CHIP_ERROR Next();

    CHIP_ERROR Get(bool & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int8_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int16_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int32_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int64_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint8_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint16_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint32_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint64_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(float & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(double & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(chip::ByteSpan & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR GetBytes(uint8_t * buf, uint32_t bufSize) { return mUpdaterReader.GetBytes(buf, bufSize); }
    CHIP_ERROR DupBytes(uint8_t *& buf, uint32_t & dataLen) { return mUpdaterReader.DupBytes(buf, dataLen); }
    CHIP_ERROR GetString(char * buf, uint32_t bufSize) { return mUpdaterReader.GetString(buf, bufSize); }
    CHIP_ERROR DupString(char *& buf) { return mUpdaterReader.DupString(buf); }

    TLVType GetType() const { return mUpdaterReader.GetType(); }
    uint64_t GetTag() const { return mUpdaterReader.GetTag(); }
    uint32_t GetLength() const { return mUpdaterReader.GetLength(); }
    CHIP_ERROR GetDataPtr(const uint8_t *& data) { return mUpdaterReader.GetDataPtr(data); }
    CHIP_ERROR VerifyEndOfContainer() { return mUpdaterReader.VerifyEndOfContainer(); }
    TLVType GetContainerType() const { return mUpdaterReader.GetContainerType(); }
    uint32_t GetLengthRead() const { return mUpdaterReader.GetLengthRead(); }
    uint32_t GetRemainingLength() const { return mUpdaterReader.GetRemainingLength(); }

    // Writer methods
    CHIP_ERROR Put(uint64_t tag, int8_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int16_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int32_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int64_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint8_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint16_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint32_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint64_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int8_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, int16_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, int32_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, int64_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint8_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint16_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint32_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint64_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, float v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, double v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR PutBoolean(uint64_t tag, bool v) { return mUpdaterWriter.PutBoolean(tag, v); }
    CHIP_ERROR PutNull(uint64_t tag) { return mUpdaterWriter.PutNull(tag); }
    CHIP_ERROR PutBytes(uint64_t tag, const uint8_t * buf, uint32_t len) { return mUpdaterWriter.PutBytes(tag, buf, len); }
    CHIP_ERROR PutString(uint64_t tag, const char * buf) { return mUpdaterWriter.PutString(tag, buf); }
    CHIP_ERROR PutString(uint64_t tag, const char * buf, uint32_t len) { return mUpdaterWriter.PutString(tag, buf, len); }
    CHIP_ERROR CopyElement(TLVReader & reader) { return mUpdaterWriter.CopyElement(reader); }
    CHIP_ERROR CopyElement(uint64_t tag, TLVReader & reader) { return mUpdaterWriter.CopyElement(tag, reader); }
    CHIP_ERROR StartContainer(uint64_t tag, TLVType containerType, TLVType & outerContainerType)
    {
        return mUpdaterWriter.StartContainer(tag, containerType, outerContainerType);
    }
    CHIP_ERROR EndContainer(TLVType outerContainerType) { return mUpdaterWriter.EndContainer(outerContainerType); }
    uint32_t GetLengthWritten() { return mUpdaterWriter.GetLengthWritten(); }
    uint32_t GetRemainingFreeLength() { return mUpdaterWriter.mRemainingLen; }

private:
    void AdjustInternalWriterFreeSpace();

    TLVWriter mUpdaterWriter;
    TLVReader mUpdaterReader;
    const uint8_t * mElementStartAddr;
};

/**
 * Provides an interface for TLVReader or TLVWriter to use memory other than a simple contiguous buffer.
 */
class DLL_EXPORT TLVBackingStore
{
public:
    virtual ~TLVBackingStore() {}
    /**
     * A function to provide a backing store's initial start position and data length to a reader.
     *
     * @param[in]       reader          A reference to the TLVReader object that is requesting input data.
     * @param[out]      bufStart        A reference to a data pointer. On exit, bufStart is expected to point
     *                                  to the first byte of TLV data to be parsed.
     * @param[out]      bufLen          A reference to an unsigned integer that the function must set to
     *                                  the number of TLV data bytes being returned.  If the end of the
     *                                  input TLV data has been reached, the function should set this value
     *                                  to 0.
     *
     * @retval #CHIP_NO_ERROR           If the function successfully produced TLV data.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing the requested data.
     */
    virtual CHIP_ERROR OnInit(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function that can be used to retrieve additional TLV data to be parsed.
     *
     * When called, the function is expected to produce additional data for the reader to parse or signal
     * the reader that no more data is available.
     *
     * @param[in]       reader          A reference to the TLVReader object that is requesting input data.
     * @param[in,out]   bufStart        A reference to a data pointer. On entry to the function, @p bufStart
     *                                  points to one byte beyond the last TLV data byte consumed by the
     *                                  reader.  On exit, bufStart is expected to point to the first byte
     *                                  of new TLV data to be parsed.  The new pointer value can be within
     *                                  the same buffer as the previously consumed data, or it can point
     *                                  to an entirely new buffer.
     * @param[out]      bufLen          A reference to an unsigned integer that the function must set to
     *                                  the number of TLV data bytes being returned.  If the end of the
     *                                  input TLV data has been reached, the function should set this value
     *                                  to 0.
     *
     * @retval #CHIP_NO_ERROR           If the function successfully produced more TLV data, or the end of
     *                                  the input data was reached (@p bufLen should be set to 0 in this case).
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing the requested data.
     */
    virtual CHIP_ERROR GetNextBuffer(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function to provide a backing store's initial start position and data length to a writer.
     *
     * @param[in]       writer          A reference to the TLVWriter object that is requesting new buffer
     *                                  space.
     * @param[out]      bufStart        A reference to a data pointer. On exit, @p bufStart is expected to
     *                                  point to the beginning of the new output buffer.
     * @param[out]      bufLen          A reference to an unsigned integer. On exit, @p bufLen is expected
     *                                  to contain the maximum number of bytes that can be written to the
     *                                  new output buffer.
     *
     * @retval #CHIP_NO_ERROR           If the function was able to supply buffer space for the writer.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing buffer space.
     */
    virtual CHIP_ERROR OnInit(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function that supplies new output buffer space to a TLVWriter.
     *
     * The function is expected to return a pointer to a memory location where new data should be written,
     * along with an associated maximum length. The function can supply write space either by allocating
     * a new buffer to hold the data or by clearing out previously written data from an existing buffer.
     *
     * @param[in]       writer          A reference to the TLVWriter object that is requesting new buffer
     *                                  space.
     * @param[in,out]   bufStart        A reference to a data pointer. On entry to the function, @p bufStart
     *                                  points the beginning of the current output buffer.  On exit, @p bufStart
     *                                  is expected to point to the beginning of the new output buffer.
     *                                  The new pointer value can be the same as the previous value (e.g.
     *                                  if the function copied the existing data elsewhere), or it can point
     *                                  to an entirely new location.
     * @param[in,out]   bufLen          A reference to an unsigned integer. On entry to the function,
     *                                  @p bufLen contains the number of byte of @em unused space in the
     *                                  current buffer.  On exit, @p bufLen is expected to contain the maximum
     *                                  number of bytes that can be written to the new output buffer.
     *
     * @retval #CHIP_NO_ERROR          If the function was able to supply more buffer space for the writer.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing additional buffer
     *                                  space.
     */
    virtual CHIP_ERROR GetNewBuffer(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function used to perform finalization of the output from a TLVWriter object.
     *
     * Functions of this type are called when a TLVWriter's Finalize() method is called. The function is
     * expected to perform any necessary clean-up or finalization related to consuming the output of the
     * writer object. Examples of this include such things as recording the final length of the encoding,
     * or closing a file descriptor.
     *
     * @param[in]       writer          A reference to the TLVWriter object that is being finalized.
     * @param[in,out]   bufStart        A pointer to the beginning of the current (and final) output buffer.
     * @param[in,out]   bufLen          The number of bytes contained in the buffer pointed to by @p bufStart.
     *
     * @retval #CHIP_NO_ERROR           If finalization was successful.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred during finalization.
     *
     */
    virtual CHIP_ERROR FinalizeBuffer(TLVWriter & writer, uint8_t * bufStart, uint32_t bufLen) = 0;
};

} // namespace TLV
} // namespace chip
