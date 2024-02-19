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

#include <cstdio>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include <lib/core/CHIPError.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>
#include <system/SystemConfig.h>

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
    TLVWriter();

    // TODO(#30825): We do not cleanly handle copies for all backing stores, but we don't disallow copy...
#if 0
    // Disable copy (and move) semantics.
    TLVWriter(const TLVWriter&) = delete;
    TLVWriter& operator=(const TLVWriter&) = delete;
#endif

    // Initialization cookie that is set when properly initialized. Randomly-picked 16 bit value.
    static constexpr uint16_t kExpectedInitializationCookie = 0x52b1;

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
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              FinalizeBuffer() function.
     */
    CHIP_ERROR Finalize();

    /**
     * Reserve some buffer for encoding future fields.
     *
     * @retval #CHIP_NO_ERROR        Successfully reserved required buffer size.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
     * @retval #CHIP_ERROR_NO_MEMORY The reserved buffer size cannot fits into the remaining buffer size.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                               Uses TLVBackingStore and is in a state where it might allocate
     *                               additional non-contigious memory, thus making it difficult/impossible
     *                               to properly reserve space.
     */
    CHIP_ERROR ReserveBuffer(uint32_t aBufferSize);

    /**
     * Release previously reserved buffer.
     *
     * @retval #CHIP_NO_ERROR        Successfully released reserved buffer size.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
     * @retval #CHIP_ERROR_NO_MEMORY The released buffer is larger than previously reserved buffer size.
     */
    CHIP_ERROR UnreserveBuffer(uint32_t aBufferSize)
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mReservedSize >= aBufferSize, CHIP_ERROR_NO_MEMORY);
        mReservedSize -= aBufferSize;
        mRemainingLen += aBufferSize;
        return CHIP_NO_ERROR;
    }

    /**
     * Encodes a TLV signed integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR Put(Tag tag, int8_t v);

    /**
     * Encodes a TLV signed integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
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
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR Put(Tag tag, int8_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v)
     */
    CHIP_ERROR Put(Tag tag, int16_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(Tag tag, int16_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v)
     */
    CHIP_ERROR Put(Tag tag, int32_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(Tag tag, int32_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v)
     */
    CHIP_ERROR Put(Tag tag, int64_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(Tag tag, int64_t v, bool preserveSize);

    /**
     * Encodes a TLV unsigned integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag. Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR Put(Tag tag, uint8_t v);

    /**
     * Encodes a TLV unsigned integer value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
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
    CHIP_ERROR Put(Tag tag, uint8_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v)
     */
    CHIP_ERROR Put(Tag tag, uint16_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(Tag tag, uint16_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v)
     */
    CHIP_ERROR Put(Tag tag, uint32_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(Tag tag, uint32_t v, bool preserveSize);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v)
     */
    CHIP_ERROR Put(Tag tag, uint64_t v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v, bool preserveSize)
     */
    CHIP_ERROR Put(Tag tag, uint64_t v, bool preserveSize);

    /**
     * Encodes a TLV floating point value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR Put(Tag tag, double v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, double v)
     */
    CHIP_ERROR Put(Tag tag, float v);

    /**
     * Encodes a TLV byte string value using ByteSpan class.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   data            A ByteSpan object containing the bytes string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR Put(Tag tag, ByteSpan data);

    /**
     * static_cast to enumerations' underlying type when data is an enumeration.
     */
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    CHIP_ERROR Put(Tag tag, T data)
    {
        return Put(tag, to_underlying(data));
    }

    /**
     *
     * Encodes an unsigned integer with bits corresponding to the flags set when data is a BitFlags
     */
    template <typename T>
    CHIP_ERROR Put(Tag tag, BitFlags<T> data)
    {
        return Put(tag, data.Raw());
    }

    /**
     *
     * Encodes an unsigned integer with bits corresponding to the flags set when data is a BitMask
     */
    template <typename T>
    CHIP_ERROR Put(Tag tag, BitMask<T> data)
    {
        return Put(tag, data.Raw());
    }

    /**
     * Encodes a TLV boolean value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   v               The value to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutBoolean(Tag tag, bool v);

    /**
     * @overload CHIP_ERROR TLVWriter::Put(Tag tag, bool v)
     */
    CHIP_ERROR Put(Tag tag, bool v)
    {
        /*
         * In TLV, boolean values are encoded as standalone tags without actual values, so we have a separate
         * PutBoolean method.
         */
        return PutBoolean(tag, v);
    }

    /**
     * Encodes a TLV byte string value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   buf             A pointer to a buffer containing the bytes string to be encoded.
     * @param[in]   len             The number of bytes to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutBytes(Tag tag, const uint8_t * buf, uint32_t len);

    /**
     * Encodes a TLV UTF8 string value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   buf             A pointer to the null-terminated UTF-8 string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutString(Tag tag, const char * buf);

    /**
     * Encodes a TLV UTF8 string value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   buf             A pointer to the UTF-8 string to be encoded.
     * @param[in]   len             The length (in bytes) of the string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutString(Tag tag, const char * buf, uint32_t len);

    /**
     * Encodes a TLV UTF8 string value that's passed in as a Span.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   str             A Span containing a pointer and a length of the string to be encoded.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutString(Tag tag, CharSpan str);

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
     *                AnonymousTag() if the value should be encoded without
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
    // The ENFORCE_FORMAT args are "off by one" because this is a class method,
    // with an implicit "this" as first arg.
    CHIP_ERROR PutStringF(Tag tag, const char * fmt, ...) ENFORCE_FORMAT(3, 4);

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
     *                AnonymousTag() if the value should be encoded without
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
    // The ENFORCE_FORMAT args are "off by one" because this is a class method,
    // with an implicit "this" as first arg.
    CHIP_ERROR VPutStringF(Tag tag, const char * fmt, va_list ap) ENFORCE_FORMAT(3, 0);

    /**
     * Encodes a TLV null value.
     *
     * @param[in]   tag             The TLV tag to be encoded with the value, or @p AnonymousTag() if the
     *                              value should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutNull(Tag tag);

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
     *                              If the supplied reader is not positioned on an element or if the writer is not initialized.
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
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag() if
     *                              the container should be encoded without a tag.  Tag values should be
     *                              constructed with one of the tag definition functions ProfileTag(),
     *                              ContextTag() or CommonTag().
     * @param[in]   reader          A reference to a TLVReader object identifying a pre-encoded TLV
     *                              element whose type and value should be copied.
     *
     * @retval #CHIP_NO_ERROR      If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE
     *                              If the supplied reader is not positioned on an element or if the writer is not initialized.
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
    CHIP_ERROR CopyElement(Tag tag, TLVReader & reader);

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
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag() if
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
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR StartContainer(Tag tag, TLVType containerType, TLVType & outerContainerType);

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
     *                              If a corresponding StartContainer() call was not made or if the TLVWriter is not initialized.
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
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag() if
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
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR OpenContainer(Tag tag, TLVType containerType, TLVWriter & containerWriter);

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
     *                              If the supplied container writer is not in the correct state or if the TLVWriter is not
     * initialized.
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
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag() if
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
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR PutPreEncodedContainer(Tag tag, TLVType containerType, const uint8_t * data, uint32_t dataLen);

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
     *                              If the supplied reader is not positioned on a container element or if the TLVWriter was not
     * initialized.
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
     * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag() if
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
     *                              If the supplied reader is not positioned on a container element or of the TLVWriter was not
     * initialized.
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
    CHIP_ERROR CopyContainer(Tag tag, TLVReader & container);

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
     * @param[in] tag                   The TLV tag to be encoded with the container, or @p AnonymousTag() if
     *                                  the container should be encoded without a tag.  Tag values should be
     *                                  constructed with one of the tag definition functions ProfileTag(),
     *                                  ContextTag() or CommonTag().
     * @param[in] encodedContainer      A buffer containing a pre-encoded TLV container whose type and members
     *                                  should be copied.
     * @param[in] encodedContainerLen   The length in bytes of the pre-encoded container.
     *
     * @retval #CHIP_NO_ERROR          If the method succeeded.
     * @retval #CHIP_ERROR_INCORRECT_STATE  If the TLVWriter was not initialized.
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
    CHIP_ERROR CopyContainer(Tag tag, const uint8_t * encodedContainer, uint16_t encodedContainerLen);

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
     * @brief Returns true if this TLVWriter was properly initialized.
     */
    bool IsInitialized() const { return mInitializationCookie == kExpectedInitializationCookie; }

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
    uint32_t mReservedSize;
    TLVType mContainerType;
    uint16_t mInitializationCookie;

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
    static void TLVWriterPutcharCB(uint8_t c, void * appState);
#endif
    CHIP_ERROR WriteElementHead(TLVElementType elemType, Tag tag, uint64_t lenOrVal);
    CHIP_ERROR WriteElementWithData(TLVType type, Tag tag, const uint8_t * data, uint32_t dataLen);
    CHIP_ERROR WriteData(const uint8_t * p, uint32_t len);
};

/*
 * A TLVWriter that is backed by a scoped memory buffer that is owned by the writer.
 */
class ScopedBufferTLVWriter : public TLVWriter
{
public:
    /*
     * Construct and initialize the writer by taking ownership of the provided scoped buffer.
     */
    ScopedBufferTLVWriter(Platform::ScopedMemoryBuffer<uint8_t> && buffer, size_t dataLen)
    {
        mBuffer = std::move(buffer);
        Init(mBuffer.Get(), dataLen);
    }

    /*
     * Finalize the writer and take back the buffer owned by the writer. This transfers its
     * ownership to the provided buffer reference. This also re-initializes the writer with
     * a null buffer to prevent further inadvertent use of the writer.
     */
    CHIP_ERROR Finalize(Platform::ScopedMemoryBuffer<uint8_t> & buffer)
    {
        ReturnErrorOnFailure(TLVWriter::Finalize());
        buffer = std::move(mBuffer);
        Init(nullptr, 0);
        return CHIP_NO_ERROR;
    }

private:
    Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};

} // namespace TLV
} // namespace chip
