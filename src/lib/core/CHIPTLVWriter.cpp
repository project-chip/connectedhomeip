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
 *      This file implements an encoder for the CHIP TLV (Tag-Length-Value) encoding format.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <core/CHIPTLV.h>

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>

#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <system/SystemPacketBuffer.h>

#include <stdarg.h>
#include <stdint.h>

// Doxygen is confused by the __attribute__ annotation
#ifndef DOXYGEN
#define NO_INLINE __attribute__((noinline))
#endif // DOXYGEN

namespace chip {
namespace TLV {

using namespace chip::Encoding;

/**
 * @var uint32_t TLVWriter::ImplicitProfileId
 *
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

/**
 * @var void *TLVWriter::AppData
 *
 * A pointer field that can be used for application-specific data.
 */

/**
 * @typedef CHIP_ERROR (*TLVWriter::GetNewBufferFunct)(TLVWriter& writer, uintptr_t& bufHandle, uint8_t *& bufStart, uint32_t&
 * bufLen)
 *
 * A function that supplies new output buffer space to a TLVWriter.
 *
 * Functions of this type are used to prepare new buffer space for a TLVWriter to write to. When called,
 * the function is expected to return a pointer to a memory location where new data should be written,
 * along with an associated maximum length. The function can supply write space either by allocating
 * a new buffer to hold the data or by clearing out previously written data from an existing buffer.
 *
 * @param[in]       writer          A reference to the TLVWriter object that is requesting new buffer
 *                                  space.
 * @param[in,out]   bufHandle       A reference to a uintptr_t value that the function can use to store
 *                                  context data between calls.  This value is initialized to 0
 *                                  prior to the first call.
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
 *
 */

/**
 * @var GetNewBufferFunct TLVWriter::GetNewBuffer
 *
 * A pointer to a function that will supply new output buffer space to a TLVWriter.
 *
 * A TLVWriter object will call the GetNewBuffer function whenever an attempt is made to write data
 * that exceeds the size of the current output buffer.  If set to NULL (the default value), the
 * writer will return a CHIP_ERROR_NO_MEMORY if the output data overflows the current buffer.
 *
 * GetNewBuffer can be set by an application at any time, but is typically set when the writer
 * is initialized.
 *
 * See the GetNewBufferFunct type definition for additional information on implementing a
 * GetNewBuffer function.
 */

/**
 * @typedef CHIP_ERROR (*TLVWriter::FinalizeBufferFunct)(TLVWriter& writer, uintptr_t bufHandle, uint8_t *bufStart, uint32_t bufLen)
 *
 * A function used to perform finalization of the output from a TLVWriter object.
 *
 * Functions of this type are called when a TLVWriter's Finalize() method is called. The function is
 * expected to perform any necessary clean-up or finalization related to consuming the output of the
 * writer object. Examples of this include such things as recording the final length of the encoding,
 * or closing a file descriptor.
 *
 * @param[in]       writer          A reference to the TLVWriter object that is being finalized.
 * @param[in,out]   bufHandle       A uintptr_t context value that was set by previous calls to the
 *                                  @p GetNewBuffer function.
 * @param[in,out]   bufStart        A pointer to the beginning of the current (and final) output buffer.
 * @param[in,out]   bufLen          The number of bytes contained in the buffer pointed to by @p bufStart.
 *
 * @retval #CHIP_NO_ERROR          If finalization was successful.
 * @retval other                    Other CHIP or platform-specific error codes indicating that an error
 *                                  occurred during finalization.
 *
 */

/**
 * @var FinalizeBufferFunct TLVWriter::FinalizeBuffer
 *
 * A pointer to a function that will be called when the TLVWriter is finalized.
 *
 * A TLVWriter object will call the FinalizeBuffer function whenever its Finalize() method is
 * called. Applications can set the function pointer at any point prior to calling Finalize().
 * By default the pointer is set to NULL, which causes the Finalize() method to forego calling
 * the function.
 *
 * See the FinalizeBufferFunct type definition for additional information on implementing a
 * FinalizeBuffer function.
 */

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
NO_INLINE void TLVWriter::Init(uint8_t * buf, uint32_t maxLen)
{
    mBufHandle = 0;
    mBufStart = mWritePoint = buf;
    mRemainingLen           = maxLen;
    mLenWritten             = 0;
    mMaxLen                 = maxLen;
    mContainerType          = kTLVType_NotSpecified;
    SetContainerOpen(false);
    SetCloseContainerReserved(true);

    ImplicitProfileId = kProfileIdNotSpecified;
    GetNewBuffer      = nullptr;
    FinalizeBuffer    = nullptr;
}

/**
 * Initializes a TLVWriter object to write into a single PacketBuffer.
 *
 * Writing begins immediately after the last byte of existing data in the supplied buffer.
 *
 * @note If a chain of buffers is given, data will only be written to the first buffer.
 *
 * @note Applications must call Finalize() on the writer before using the contents of the buffer.
 *
 * @param[in]   buf     A pointer to an PacketBuffer into which TLV should be written.
 * @param[in]   maxLen  The maximum number of bytes that should be written to the output buffer.
 *
 */
void TLVWriter::Init(PacketBuffer * buf, uint32_t maxLen)
{
    mBufHandle = reinterpret_cast<uintptr_t>(buf);
    mBufStart = mWritePoint = buf->Start() + buf->DataLength();
    mRemainingLen           = buf->AvailableDataLength();
    if (mRemainingLen > maxLen)
        mRemainingLen = maxLen;
    mLenWritten    = 0;
    mMaxLen        = maxLen;
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);
    SetCloseContainerReserved(true);

    ImplicitProfileId = kProfileIdNotSpecified;
    GetNewBuffer      = nullptr;
    FinalizeBuffer    = FinalizePacketBuffer;
}

/**
 * Initializes a TLVWriter object to write into one or more PacketBuffers
 *
 * Writing begins immediately after the last byte of existing data in the specified buffer. If
 * @p allowDiscontiguousBuffers is true, additional PacketBuffers will be allocated and chained to
 * the supplied buffer as needed to accommodate the amount of data written.  If the specified output
 * buffer is already the head of a chain of buffers, output will be written to the subsequent buffers
 * in the chain before any new buffers are allocated.
 *
 * @note Applications must call Finalize() on the writer before using the contents of the output
 * buffer(s).
 *
 * @param[in]   buf     A pointer to an PacketBuffer into which TLV data should be written.
 * @param[in]   maxLen  The maximum number of bytes that should be written to the output buffer(s).
 * @param[in]   allowDiscontiguousBuffers
 *                      If true, write data to a chain of PacketBuffers, allocating new buffers as
 *                      needed to store the data written.  If false, writing will fail with
 *                      CHIP_ERROR_BUFFER_TOO_SMALL if the written data exceeds the space available
 *                      in the initial output buffer.
 *
 */
void TLVWriter::Init(PacketBuffer * buf, uint32_t maxLen, bool allowDiscontiguousBuffers)
{
    Init(buf, maxLen);

    if (allowDiscontiguousBuffers)
    {
        GetNewBuffer = GetNewPacketBuffer;
    }
    else
    {
        GetNewBuffer = nullptr;
    }
}

/**
 * Returns the total number of bytes written since the writer was initialized.
 *
 * @return Total number of bytes written since the writer was initialized.
 */
uint32_t TLVWriter::GetLengthWritten()
{
    return mLenWritten;
}

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
CHIP_ERROR TLVWriter::Finalize()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;
    if (FinalizeBuffer != nullptr)
        err = FinalizeBuffer(*this, mBufHandle, mBufStart, static_cast<uint32_t>(mWritePoint - mBufStart));
    return err;
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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::PutBoolean(uint64_t tag, bool v)
{
    return WriteElementHead((v) ? TLVElementType::BooleanTrue : TLVElementType::BooleanFalse, tag, 0);
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
{
    return Put(tag, static_cast<uint64_t>(v));
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt8, tag, v);
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint16_t v)
{
    return Put(tag, static_cast<uint64_t>(v));
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint16_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt16, tag, v);
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint32_t v)
{
    return Put(tag, static_cast<uint64_t>(v));
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint32_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt32, tag, v);
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint64_t v)
{
    TLVElementType elemType;
    if (v <= UINT8_MAX)
        elemType = TLVElementType::UInt8;
    else if (v <= UINT16_MAX)
        elemType = TLVElementType::UInt16;
    else if (v <= UINT32_MAX)
        elemType = TLVElementType::UInt32;
    else
        elemType = TLVElementType::UInt64;
    return WriteElementHead(elemType, tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, uint8_t v, bool preserveSize)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, uint64_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt64, tag, v);
    return Put(tag, v);
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
{
    return Put(tag, static_cast<int64_t>(v));
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int8, tag, static_cast<uint8_t>(v));
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int16_t v)
{
    return Put(tag, static_cast<int64_t>(v));
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int16_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int16, tag, static_cast<uint16_t>(v));
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int32_t v)
{
    return Put(tag, static_cast<int64_t>(v));
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int32_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int32, tag, static_cast<uint32_t>(v));
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int64_t v)
{
    TLVElementType elemType;
    if (v >= INT8_MIN && v <= INT8_MAX)
        elemType = TLVElementType::Int8;
    else if (v >= INT16_MIN && v <= INT16_MAX)
        elemType = TLVElementType::Int16;
    else if (v >= INT32_MIN && v <= INT32_MAX)
        elemType = TLVElementType::Int32;
    else
        elemType = TLVElementType::Int64;
    return WriteElementHead(elemType, tag, static_cast<uint64_t>(v));
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, int8_t v, bool preserveSize)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, int64_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int64, tag, static_cast<uint64_t>(v));
    return Put(tag, v);
}

/**
 * @overload CHIP_ERROR TLVWriter::Put(uint64_t tag, double v)
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, float v)
{
    union
    {
        float f;
        uint32_t u32;
    } cvt;
    cvt.f = v;
    return WriteElementHead(TLVElementType::FloatingPointNumber32, tag, cvt.u32);
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::Put(uint64_t tag, double v)
{
    union
    {
        double d;
        uint64_t u64;
    } cvt;
    cvt.d = v;
    return WriteElementHead(TLVElementType::FloatingPointNumber64, tag, cvt.u64);
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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::PutBytes(uint64_t tag, const uint8_t * buf, uint32_t len)
{
    return WriteElementWithData(kTLVType_ByteString, tag, buf, len);
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::PutString(uint64_t tag, const char * buf)
{
    size_t len = strlen(buf);
    if (!CanCastTo<uint32_t>(len))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return PutString(tag, buf, static_cast<uint32_t>(len));
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::PutString(uint64_t tag, const char * buf, uint32_t len)
{
    return WriteElementWithData(kTLVType_UTF8String, tag, reinterpret_cast<const uint8_t *>(buf), len);
}

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
CHIP_ERROR TLVWriter::PutStringF(uint64_t tag, const char * fmt, ...)
{
    CHIP_ERROR err;
    va_list ap;

    va_start(ap, fmt);

    err = VPutStringF(tag, fmt, ap);

    va_end(ap);

    return err;
}

#if CONFIG_HAVE_VCBPRINTF
// We have a variant of the printf function that takes a callback that
// emits a single character.  The callback performs a function
// identical to putchar.

void TLVWriter::CHIPTLVWriterPutcharCB(uint8_t c, void * appState)
{
    TLVWriter * w = static_cast<TLVWriter *>(appState);
    w->WriteData(&c, sizeof(c));
}
#endif

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
CHIP_ERROR TLVWriter::VPutStringF(uint64_t tag, const char * fmt, va_list ap)
{
    va_list aq;
    size_t dataLen;
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVFieldSize lenFieldSize;
#if CONFIG_HAVE_VSNPRINTF_EX
    size_t skipLen;
    size_t writtenBytes;
#elif CONFIG_HAVE_VCBPRINTF
#elif CONFIG_TLV_TRUNCATE
    size_t maxLen;
#else
    char * tmpBuf;
#endif
    va_copy(aq, ap);

    dataLen = static_cast<size_t>(vsnprintf(nullptr, 0, fmt, aq));
    if (!CanCastTo<uint32_t>(dataLen))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    va_end(aq);

    if (dataLen <= UINT8_MAX)
        lenFieldSize = kTLVFieldSize_1Byte;
    else if (dataLen <= UINT16_MAX)
        lenFieldSize = kTLVFieldSize_2Byte;
    else
        lenFieldSize = kTLVFieldSize_4Byte;

#if !(CONFIG_HAVE_VCBPRINTF) && !(CONFIG_HAVE_VSNPRINTF_EX) && CONFIG_TLV_TRUNCATE
    // no facilities for splitting the stream across multiple buffers,
    // just write however much fits in the current buffer.
    // assume conservative tag length at this time (8 bytes)
    maxLen = mRemainingLen -
        (1 + 8 + (1 << static_cast<uint8_t>(lenFieldSize)) +
         1); // 1 : control byte, 8 : tag length, stringLen + 1 for null termination
    if (maxLen < dataLen)
        dataLen = maxLen;
#endif

    // write length.
    err = WriteElementHead(
        static_cast<TLVElementType>(static_cast<uint8_t>(kTLVType_UTF8String) | static_cast<uint8_t>(lenFieldSize)), tag, dataLen);
    SuccessOrExit(err);

    VerifyOrExit((mLenWritten + dataLen) <= mMaxLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // write data
#if CONFIG_HAVE_VSNPRINTF_EX

    skipLen = 0;

    do
    {
        va_copy(aq, ap);

        vsnprintf_ex(reinterpret_cast<char *>(mWritePoint), mRemainingLen, skipLen, fmt, aq);

        va_end(aq);

        writtenBytes = (mRemainingLen >= (dataLen - skipLen)) ? dataLen - skipLen : mRemainingLen;
        skipLen += writtenBytes;
        mWritePoint += writtenBytes;
        mRemainingLen -= writtenBytes;
        mLenWritten += writtenBytes;
        if (skipLen < dataLen)
        {
            VerifyOrExit(GetNewBuffer != NULL, err = CHIP_ERROR_NO_MEMORY);

            if (FinalizeBuffer != NULL)
            {
                err = FinalizeBuffer(*this, mBufHandle, mBufStart, mWritePoint - mBufStart);
                SuccessOrExit(err);
            }

            err = GetNewBuffer(*this, mBufHandle, mBufStart, mRemainingLen);
            SuccessOrExit(err);

            mWritePoint = mBufStart;
        }

    } while (skipLen < dataLen);

#elif CONFIG_HAVE_VCBPRINTF

    va_copy(aq, ap);

    vcbprintf(CHIPTLVWriterPutcharCB, this, dataLen, fmt, aq);

    va_end(aq);

#else // CONFIG_HAVE_VSNPRINTF_EX

    tmpBuf = static_cast<char *>(chip::Platform::MemoryAlloc(dataLen + 1));
    VerifyOrExit(tmpBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    va_copy(aq, ap);

    vsnprintf(tmpBuf, dataLen + 1, fmt, aq);

    va_end(aq);

    err = WriteData(reinterpret_cast<uint8_t *>(tmpBuf), static_cast<uint32_t>(dataLen));
    chip::Platform::MemoryFree(tmpBuf);

#endif // CONFIG_HAVE_VSNPRINTF_EX

exit:

    return err;
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::PutNull(uint64_t tag)
{
    return WriteElementHead(TLVElementType::Null, tag, 0);
}

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
CHIP_ERROR TLVWriter::CopyElement(TLVReader & reader)
{
    return CopyElement(reader.GetTag(), reader);
}

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

const size_t kCHIPTLVCopyChunkSize = 16;

CHIP_ERROR TLVWriter::CopyElement(uint64_t tag, TLVReader & reader)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    TLVElementType elemType = reader.ElementType();
    uint64_t elemLenOrVal   = reader.mElemLenOrVal;
    TLVReader readerHelper; // used to figure out the length of the element and read data of the element
    uint32_t copyDataLen;
    uint8_t chunk[kCHIPTLVCopyChunkSize];

    VerifyOrExit(elemType != TLVElementType::NotSpecified && elemType != TLVElementType::EndOfContainer,
                 err = CHIP_ERROR_INCORRECT_STATE);

    // Initialize the helper
    readerHelper.Init(reader);

    // Skip to the end of the element.
    err = reader.Skip();
    SuccessOrExit(err);

    // Compute the amount of value data to copy from the reader.
    copyDataLen = reader.GetLengthRead() - readerHelper.GetLengthRead();

    // Write the head of the new element with the same type and length/value, but using the
    // specified tag.
    err = WriteElementHead(elemType, tag, elemLenOrVal);
    SuccessOrExit(err);

    while (copyDataLen > 0)
    {
        uint32_t chunkSize = copyDataLen > kCHIPTLVCopyChunkSize ? kCHIPTLVCopyChunkSize : copyDataLen;
        err                = readerHelper.ReadData(chunk, chunkSize);
        SuccessOrExit(err);

        err = WriteData(chunk, chunkSize);
        SuccessOrExit(err);

        copyDataLen -= chunkSize;
    }

exit:
    return err;
}

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
 *                              @p kTLVType_Array or @p kTLVType_Path.
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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::OpenContainer(uint64_t tag, TLVType containerType, TLVWriter & containerWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(TLVTypeIsContainer(containerType), err = CHIP_ERROR_WRONG_TLV_TYPE);

    if (IsCloseContainerReserved())
    {
        VerifyOrExit(mMaxLen >= kEndOfContainerMarkerSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        mMaxLen -= kEndOfContainerMarkerSize;
    }
    err = WriteElementHead(static_cast<TLVElementType>(containerType), tag, 0);

    if (err != CHIP_NO_ERROR)
    {
        // undo the space reservation, as the container is not actually open
        if (IsCloseContainerReserved())
            mMaxLen += kEndOfContainerMarkerSize;

        ExitNow();
    }

    containerWriter.mBufHandle     = mBufHandle;
    containerWriter.mBufStart      = mBufStart;
    containerWriter.mWritePoint    = mWritePoint;
    containerWriter.mRemainingLen  = mRemainingLen;
    containerWriter.mLenWritten    = 0;
    containerWriter.mMaxLen        = mMaxLen - mLenWritten;
    containerWriter.mContainerType = containerType;
    containerWriter.SetContainerOpen(false);
    containerWriter.SetCloseContainerReserved(IsCloseContainerReserved());
    containerWriter.ImplicitProfileId = ImplicitProfileId;
    containerWriter.GetNewBuffer      = GetNewBuffer;
    containerWriter.FinalizeBuffer    = FinalizeBuffer;

    SetContainerOpen(true);

exit:
    return err;
}

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
 *                              configured GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::CloseContainer(TLVWriter & containerWriter)
{
    if (!TLVTypeIsContainer(containerWriter.mContainerType))
        return CHIP_ERROR_INCORRECT_STATE;

    if (containerWriter.IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;

    mBufHandle    = containerWriter.mBufHandle;
    mBufStart     = containerWriter.mBufStart;
    mWritePoint   = containerWriter.mWritePoint;
    mRemainingLen = containerWriter.mRemainingLen;
    mLenWritten += containerWriter.mLenWritten;

    if (IsCloseContainerReserved())
        mMaxLen += kEndOfContainerMarkerSize;

    SetContainerOpen(false);

    // Reset the container writer so that it can't accidentally be used again.
    containerWriter.Init(static_cast<uint8_t *>(nullptr), 0);

    return WriteElementHead(TLVElementType::EndOfContainer, AnonymousTag, 0);
}

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
 *                              @p kTLVType_Array or @p kTLVType_Path.
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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::StartContainer(uint64_t tag, TLVType containerType, TLVType & outerContainerType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(TLVTypeIsContainer(containerType), err = CHIP_ERROR_WRONG_TLV_TYPE);

    if (IsCloseContainerReserved())
    {
        VerifyOrExit(mMaxLen >= kEndOfContainerMarkerSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        mMaxLen -= kEndOfContainerMarkerSize;
    }

    err = WriteElementHead(static_cast<TLVElementType>(containerType), tag, 0);
    if (err != CHIP_NO_ERROR)
    {
        // undo the space reservation, as the container is not actually open
        if (IsCloseContainerReserved())
            mMaxLen += kEndOfContainerMarkerSize;

        ExitNow();
    }
    outerContainerType = mContainerType;
    mContainerType     = containerType;

    SetContainerOpen(false);

exit:
    return err;
}

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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::EndContainer(TLVType outerContainerType)
{
    if (!TLVTypeIsContainer(mContainerType))
        return CHIP_ERROR_INCORRECT_STATE;

    mContainerType = outerContainerType;

    if (IsCloseContainerReserved())
        mMaxLen += kEndOfContainerMarkerSize;

    return WriteElementHead(TLVElementType::EndOfContainer, AnonymousTag, 0);
}

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
 *                              @p kTLVType_Array or @p kTLVType_Path.
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
 *                              GetNewBuffer() or FinalizeBuffer() functions.
 *
 */
CHIP_ERROR TLVWriter::PutPreEncodedContainer(uint64_t tag, TLVType containerType, const uint8_t * data, uint32_t dataLen)
{
    if (!TLVTypeIsContainer(containerType))
        return CHIP_ERROR_INVALID_ARGUMENT;

    CHIP_ERROR err = WriteElementHead(static_cast<TLVElementType>(containerType), tag, 0);
    if (err != CHIP_NO_ERROR)
        return err;

    return WriteData(data, dataLen);
}

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
 * input buffer that contains the entirety of the underlying TLV encoding. Supplying a reader in any
 * other mode has undefined behavior.
 *
 * @param[in]   container       A reference to a TLVReader object identifying the pre-encoded TLV
 *                              container to be copied.
 *
 * @retval #CHIP_NO_ERROR      If the method succeeded.
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
 *                              GetNewBuffer() or FinalizeBuffer() functions, or by the GetNextBuffer()
 *                              function associated with the reader object.
 *
 */
CHIP_ERROR TLVWriter::CopyContainer(TLVReader & container)
{
    return CopyContainer(container.GetTag(), container);
}

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
 * input buffer that contains the entirety of the underlying TLV encoding. Supplying a reader in any
 * other mode has undefined behavior.
 *
 * @param[in]   tag             The TLV tag to be encoded with the container, or @p AnonymousTag if
 *                              the container should be encoded without a tag.  Tag values should be
 *                              constructed with one of the tag definition functions ProfileTag(),
 *                              ContextTag() or CommonTag().
 * @param[in]   container       A reference to a TLVReader object identifying a pre-encoded TLV
 *                              container whose type and members should be copied.
 *
 * @retval #CHIP_NO_ERROR      If the method succeeded.
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
 *                              GetNewBuffer() or FinalizeBuffer() functions, or by the GetNextBuffer()
 *                              function associated with the reader object.
 *
 */
CHIP_ERROR TLVWriter::CopyContainer(uint64_t tag, TLVReader & container)
{
    // NOTE: This function MUST be used with a TVLReader that is reading from a contiguous buffer.
    CHIP_ERROR err;
    TLVType containerType, outerContainerType;
    const uint8_t * containerStart;

    containerType = container.GetType();

    err = container.EnterContainer(outerContainerType);
    if (err != CHIP_NO_ERROR)
        return err;

    containerStart = container.GetReadPoint();

    err = container.ExitContainer(outerContainerType);
    if (err != CHIP_NO_ERROR)
        return err;

    return PutPreEncodedContainer(tag, containerType, containerStart,
                                  static_cast<uint32_t>(container.GetReadPoint() - containerStart));
}

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
 *                                  GetNewBuffer() or FinalizeBuffer() functions, or by the GetNextBuffer()
 *                                  function associated with the reader object.
 *
 */
CHIP_ERROR TLVWriter::CopyContainer(uint64_t tag, const uint8_t * encodedContainer, uint16_t encodedContainerLen)
{
    CHIP_ERROR err;
    TLVReader reader;

    reader.Init(encodedContainer, encodedContainerLen);

    err = reader.Next();
    SuccessOrExit(err);

    err = PutPreEncodedContainer(tag, reader.GetType(), reader.GetReadPoint(), reader.GetRemainingLength());
    SuccessOrExit(err);

exit:
    return err;
}

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
TLVType TLVWriter::GetContainerType() const
{
    return mContainerType;
}

CHIP_ERROR TLVWriter::WriteElementHead(TLVElementType elemType, uint64_t tag, uint64_t lenOrVal)
{
    uint8_t * p;
    uint8_t stagingBuf[17]; // 17 = 1 control byte + 8 tag bytes + 8 length/value bytes

    if (IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;

    uint32_t tagNum = TagNumFromTag(tag);

    if ((mRemainingLen >= sizeof(stagingBuf)) && (mMaxLen >= sizeof(stagingBuf)))
        p = mWritePoint;
    else
        p = stagingBuf;

    if (IsSpecialTag(tag))
    {
        if (tagNum < 256)
        {
            if (mContainerType != kTLVType_Structure && mContainerType != kTLVType_Path)
                return CHIP_ERROR_INVALID_TLV_TAG;

            Write8(p, TLVTagControl::ContextSpecific | elemType);
            Write8(p, static_cast<uint8_t>(tagNum));
        }
        else
        {
            if (elemType != TLVElementType::EndOfContainer && mContainerType != kTLVType_NotSpecified &&
                mContainerType != kTLVType_Array && mContainerType != kTLVType_Path)
                return CHIP_ERROR_INVALID_TLV_TAG;

            Write8(p, TLVTagControl::Anonymous | elemType);
        }
    }
    else
    {
        uint32_t profileId = ProfileIdFromTag(tag);

        if (mContainerType != kTLVType_NotSpecified && mContainerType != kTLVType_Structure && mContainerType != kTLVType_Path)
            return CHIP_ERROR_INVALID_TLV_TAG;

        if (profileId == kCommonProfileId)
        {
            if (tagNum < 65536)
            {
                Write8(p, TLVTagControl::CommonProfile_2Bytes | elemType);
                LittleEndian::Write16(p, static_cast<uint16_t>(tagNum));
            }
            else
            {
                Write8(p, TLVTagControl::CommonProfile_4Bytes | elemType);
                LittleEndian::Write32(p, tagNum);
            }
        }
        else if (profileId == ImplicitProfileId)
        {
            if (tagNum < 65536)
            {
                Write8(p, TLVTagControl::ImplicitProfile_2Bytes | elemType);
                LittleEndian::Write16(p, static_cast<uint16_t>(tagNum));
            }
            else
            {
                Write8(p, TLVTagControl::ImplicitProfile_4Bytes | elemType);
                LittleEndian::Write32(p, tagNum);
            }
        }
        else
        {
            uint16_t vendorId   = static_cast<uint16_t>(profileId >> 16);
            uint16_t profileNum = static_cast<uint16_t>(profileId);

            if (tagNum < 65536)
            {
                Write8(p, TLVTagControl::FullyQualified_6Bytes | elemType);
                LittleEndian::Write16(p, vendorId);
                LittleEndian::Write16(p, profileNum);
                LittleEndian::Write16(p, static_cast<uint16_t>(tagNum));
            }
            else
            {
                Write8(p, TLVTagControl::FullyQualified_8Bytes | elemType);
                LittleEndian::Write16(p, vendorId);
                LittleEndian::Write16(p, profileNum);
                LittleEndian::Write32(p, tagNum);
            }
        }
    }

    switch (GetTLVFieldSize(elemType))
    {
    case kTLVFieldSize_0Byte:
        break;
    case kTLVFieldSize_1Byte:
        Write8(p, static_cast<uint8_t>(lenOrVal));
        break;
    case kTLVFieldSize_2Byte:
        LittleEndian::Write16(p, static_cast<uint16_t>(lenOrVal));
        break;
    case kTLVFieldSize_4Byte:
        LittleEndian::Write32(p, static_cast<uint32_t>(lenOrVal));
        break;
    case kTLVFieldSize_8Byte:
        LittleEndian::Write64(p, lenOrVal);
        break;
    }

    if ((mRemainingLen >= sizeof(stagingBuf)) && (mMaxLen >= sizeof(stagingBuf)))
    {
        uint32_t len = static_cast<uint32_t>(p - mWritePoint);
        mWritePoint  = p;
        mRemainingLen -= len;
        mLenWritten += len;
        return CHIP_NO_ERROR;
    }
    return WriteData(stagingBuf, static_cast<uint32_t>(p - stagingBuf));
}

CHIP_ERROR TLVWriter::WriteElementWithData(TLVType type, uint64_t tag, const uint8_t * data, uint32_t dataLen)
{
    if (static_cast<uint64_t>(type) & kTLVTypeSizeMask)
    {
        // We won't be able to recover this type properly!
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLVFieldSize lenFieldSize;

    if (dataLen <= UINT8_MAX)
        lenFieldSize = kTLVFieldSize_1Byte;
    else if (dataLen <= UINT16_MAX)
        lenFieldSize = kTLVFieldSize_2Byte;
    else
        lenFieldSize = kTLVFieldSize_4Byte;

    CHIP_ERROR err = WriteElementHead(static_cast<TLVElementType>(static_cast<uint8_t>(type) | static_cast<uint8_t>(lenFieldSize)),
                                      tag, dataLen);
    if (err != CHIP_NO_ERROR)
        return err;

    return WriteData(data, dataLen);
}

CHIP_ERROR TLVWriter::WriteData(const uint8_t * p, uint32_t len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit((mLenWritten + len) <= mMaxLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    while (len > 0)
    {
        if (mRemainingLen == 0)
        {
            VerifyOrExit(GetNewBuffer != nullptr, err = CHIP_ERROR_NO_MEMORY);

            if (FinalizeBuffer != nullptr)
            {
                VerifyOrExit(CanCastTo<uint32_t>(mWritePoint - mBufStart), err = CHIP_ERROR_INCORRECT_STATE);
                err = FinalizeBuffer(*this, mBufHandle, mBufStart, static_cast<uint32_t>(mWritePoint - mBufStart));
                SuccessOrExit(err);
            }

            err = GetNewBuffer(*this, mBufHandle, mBufStart, mRemainingLen);
            SuccessOrExit(err);

            mWritePoint = mBufStart;

            if (mRemainingLen > (mMaxLen - mLenWritten))
                mRemainingLen = (mMaxLen - mLenWritten);
        }

        uint32_t writeLen = len;
        if (writeLen > mRemainingLen)
            writeLen = mRemainingLen;

        memmove(mWritePoint, p, writeLen);
        mWritePoint += writeLen;
        mRemainingLen -= writeLen;
        mLenWritten += writeLen;
        p += writeLen;
        len -= writeLen;
    }

exit:
    return err;
}

/**
 * An implementation of a TLVWriter GetNewBuffer function for writing to a chain of PacketBuffers.
 *
 * The GetNewPacketBuffer() function supplies new output space to a TLVWriter by allocating a chain
 * of one or more PacketBuffers as needed to store the encoding.  The function is designed to be
 * assigned to the TLVWriter GetNewBuffer function pointer.
 *
 * Note that when using the GetNewPacketBuffer with a TLVWriter, the corresponding FinalizePacketBuffer()
 * function (or an equivalent) should also be used to finalize the buffer chain.
 *
 * See the GetNewBufferFunct type definition for additional information on the API of the
 * GetNewPacketBuffer() function.
 */
CHIP_ERROR TLVWriter::GetNewPacketBuffer(TLVWriter & writer, uintptr_t & bufHandle, uint8_t *& bufStart, uint32_t & bufLen)
{
    PacketBuffer * buf = reinterpret_cast<PacketBuffer *>(bufHandle);

    PacketBuffer * newBuf = buf->Next_ForNow();
    if (newBuf == nullptr)
    {
        System::PacketBufferHandle newBufHandle = PacketBuffer::New(0);
        if (!newBufHandle.IsNull())
        {
            newBuf = newBufHandle.Get_ForNow();
            buf->AddToEnd(std::move(newBufHandle));
        }
    }

    if (newBuf != nullptr)
    {
        bufHandle = reinterpret_cast<uintptr_t>(newBuf);
        bufStart  = newBuf->Start();
        bufLen    = newBuf->MaxDataLength();
    }
    else
    {
        bufStart = nullptr;
        bufLen   = 0;
    }

    return CHIP_NO_ERROR;
}

/**
 * An implementation of a TLVWriter FinalizeBuffer function for writing to a chain of PacketBuffers.
 *
 * The FinalizePacketBuffer() function performs the necessary finalization required when using a
 * TLVWriter to write to a chain of PacketBuffers.  The function is designed to be used in conjunction
 * with the GetNewPacketBuffer() function.
 *
 * See the FinalizeBufferFunct type definition for additional information on the API of the
 * FinalizePacketBuffer() function.
 */
CHIP_ERROR TLVWriter::FinalizePacketBuffer(TLVWriter & writer, uintptr_t bufHandle, uint8_t * bufStart, uint32_t dataLen)
{
    PacketBuffer * buf = reinterpret_cast<PacketBuffer *>(bufHandle);
    uint8_t * endPtr   = bufStart + dataLen;

    intptr_t length = endPtr - buf->Start();
    if (!CanCastTo<uint16_t>(length))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    buf->SetDataLength(static_cast<uint16_t>(length));

    return CHIP_NO_ERROR;
}

} // namespace TLV
} // namespace chip
