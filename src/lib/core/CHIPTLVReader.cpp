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
 *      This file implements a parser for the CHIP TLV (Tag-Length-Value) encoding format.
 *
 */

#include <stdlib.h>

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace TLV {

using namespace chip::Encoding;

static const uint8_t sTagSizes[] = { 0, 1, 2, 4, 2, 4, 6, 8 };

/**
 * @fn uint32_t TLVReader::GetLengthRead() const
 *
 * Returns the total number of bytes read since the reader was initialized.
 *
 * @return Total number of bytes read since the reader was initialized.
 */

/**
 * @fn uint32_t TLVReader::GetRemainingLength() const
 *
 * Returns the total number of bytes that can be read until the max read length is reached.
 *
 * @return Total number of bytes that can be read until the max read length is reached.
 */

/**
 * @fn const uint8_t *TLVReader::GetReadPoint() const
 *
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

/**
 *
 * @var uint32_t TLVReader::ImplicitProfileId
 *
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

/**
 * @var void *TLVReader::AppData
 *
 * A pointer field that can be used for application-specific data.
 */

/**
 * @typedef CHIP_ERROR (*TLVReader::GetNextBufferFunct)(TLVReader& reader, uintptr_t& bufHandle, const uint8_t *& bufStart,
 * uint32_t& bufLen)
 *
 * A function that can be used to retrieve additional TLV data to be parsed.
 *
 * Functions of this type are used to feed input data to a TLVReader. When called, the function is
 * expected to produce additional data for the reader to parse or signal the reader that no more
 * data is available.
 *
 * @param[in]       reader          A reference to the TLVReader object that is requesting input data.
 * @param[in,out]   bufHandle       A reference to a uintptr_t value that the function can use to store
 *                                  context data between calls.  This value is initialized to 0
 *                                  prior to the first call.
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
 * @retval #CHIP_NO_ERROR          If the function successfully produced more TLV data, or the end of
 *                                  the input data was reached (@p bufLen should be set to 0 in this case).
 * @retval other                    Other CHIP or platform-specific error codes indicating that an error
 *                                  occurred preventing the function from producing the requested data.
 *
 */

/**
 * @var GetNextBufferFunct TLVReader::GetNextBuffer
 *
 * A pointer to a function that will produce input data for the TLVReader object.  If set to NULL (the
 * default value), the reader will assume that no further input data is available.
 *
 * GetNextBuffer can be set by an application at any time, but is typically set when the reader
 * is initialized.
 *
 * See the GetNextBufferFunct type definition for additional information on implementing a
 * GetNextBuffer function.
 */

/**
 * Initializes a TLVReader object to read from a single input buffer.
 *
 * @param[in]   data    A pointer to a buffer containing the TLV data to be parsed.
 * @param[in]   dataLen The length of the TLV data to be parsed.
 *
 */
void TLVReader::Init(const uint8_t * data, uint32_t dataLen)
{
    mBufHandle = 0;
    mReadPoint = data;
    mBufEnd    = data + dataLen;
    mLenRead   = 0;
    mMaxLen    = dataLen;
    ClearElementState();
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);

    ImplicitProfileId = kProfileIdNotSpecified;
    AppData           = nullptr;
    GetNextBuffer     = nullptr;
}

/**
 * Initializes a TLVReader object to read from a single PacketBuffer.
 *
 * Parsing begins at the buffer's start position (buf->DataStart()) and continues until the
 * end of the data in the buffer (as denoted by buf->Datalen()), or maxLen bytes have been parsed.
 *
 * @param[in]   buf     A pointer to an PacketBuffer containing the TLV data to be parsed.
 * @param[in]   maxLen  The maximum of bytes to parse.  Defaults to the amount of data
 *                      in the input buffer.
 */
void TLVReader::Init(PacketBuffer * buf, uint32_t maxLen)
{
    mBufHandle = reinterpret_cast<uintptr_t>(buf);
    mReadPoint = buf->Start();
    mBufEnd    = mReadPoint + buf->DataLength();
    mLenRead   = 0;
    mMaxLen    = maxLen;
    ClearElementState();
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);

    ImplicitProfileId = kProfileIdNotSpecified;
    AppData           = nullptr;
    GetNextBuffer     = nullptr;
}

/**
 * Initializes a TLVReader object to read from a one or more PacketBuffers.
 *
 * Parsing begins at the initial buffer's start position (buf->DataStart()).  If
 * allowDiscontiguousBuffers is true, the reader will advance through the chain of buffers linked
 * by their Next() pointers. Parsing continues until all data in the buffer chain has been consumed
 * (as denoted by buf->Datalen()), or maxLen bytes have been parsed.
 *
 * @param[in]   buf    A pointer to an PacketBuffer containing the TLV data to be parsed.
 * @param[in]   maxLen  The maximum of bytes to parse.  Defaults to the total amount of data
 *                      in the input buffer chain.
 * @param[in]   allowDiscontiguousBuffers
 *                      If true, advance to the next buffer in the chain once all data in the
 *                      current buffer has been consumed.  If false, stop parsing at the end
 *                      of the initial buffer.
 */
void TLVReader::Init(PacketBuffer * buf, uint32_t maxLen, bool allowDiscontiguousBuffers)
{
    mBufHandle = reinterpret_cast<uintptr_t>(buf);
    mReadPoint = buf->Start();
    mBufEnd    = mReadPoint + buf->DataLength();
    mLenRead   = 0;
    mMaxLen    = maxLen;
    ClearElementState();
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);

    ImplicitProfileId = kProfileIdNotSpecified;
    AppData           = nullptr;

    if (allowDiscontiguousBuffers)
    {
        GetNextBuffer = GetNextPacketBuffer;
    }
    else
    {
        GetNextBuffer = nullptr;
    }
}

/**
 * Initializes a TLVReader object from another TLVReader object.
 *
 * @param[in]   aReader  A read-only reference to the TLVReader to initialize
 *                       this from.
 *
 */
void TLVReader::Init(const TLVReader & aReader)
{
    // Initialize private data members

    mElemTag       = aReader.mElemTag;
    mElemLenOrVal  = aReader.mElemLenOrVal;
    mBufHandle     = aReader.mBufHandle;
    mReadPoint     = aReader.mReadPoint;
    mBufEnd        = aReader.mBufEnd;
    mLenRead       = aReader.mLenRead;
    mMaxLen        = aReader.mMaxLen;
    mControlByte   = aReader.mControlByte;
    mContainerType = aReader.mContainerType;
    SetContainerOpen(aReader.IsContainerOpen());

    // Initialize public data members

    ImplicitProfileId = aReader.ImplicitProfileId;
    AppData           = aReader.AppData;
    GetNextBuffer     = aReader.GetNextBuffer;
}

/**
 * Returns the type of the current TLV element.
 *
 * @return      A TLVType value describing the data type of the current TLV element.  If the reader
 *              is not positioned on a TLV element, the return value will be kTLVType_NotSpecified.
 */
TLVType TLVReader::GetType() const
{
    TLVElementType elemType = ElementType();
    if (elemType == TLVElementType::EndOfContainer)
        return kTLVType_NotSpecified;
    if (elemType == TLVElementType::FloatingPointNumber32 || elemType == TLVElementType::FloatingPointNumber64)
        return kTLVType_FloatingPointNumber;
    if (elemType == TLVElementType::NotSpecified || elemType >= TLVElementType::Null)
        return static_cast<TLVType>(elemType);
    return static_cast<TLVType>(static_cast<uint8_t>(elemType) & ~kTLVTypeSizeMask);
}

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
uint16_t TLVReader::GetControlByte() const
{
    return mControlByte;
}

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
uint64_t TLVReader::GetTag() const
{
    return mElemTag;
}

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
uint32_t TLVReader::GetLength() const
{
    if (TLVTypeHasLength(ElementType()))
        return static_cast<uint32_t>(mElemLenOrVal);
    return 0;
}

/**
 * Get the value of the current element as a bool type.
 *
 * @param[out]  v                       Receives the value associated with current TLV element.
 *
 * @retval #CHIP_NO_ERROR              If the method succeeded.
 * @retval #CHIP_ERROR_WRONG_TLV_TYPE  If the current element is not a TLV boolean type, or the
 *                                      reader is not positioned on an element.
 *
 */
CHIP_ERROR TLVReader::Get(bool & v)
{
    TLVElementType elemType = ElementType();
    if (elemType == TLVElementType::BooleanFalse)
        v = false;
    else if (elemType == TLVElementType::BooleanTrue)
        v = true;
    else
        return CHIP_ERROR_WRONG_TLV_TYPE;
    return CHIP_NO_ERROR;
}

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
CHIP_ERROR TLVReader::Get(int8_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(static_cast<uint8_t>(v64));
    return err;
}

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
CHIP_ERROR TLVReader::Get(int16_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(static_cast<uint16_t>(v64));
    return err;
}

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
CHIP_ERROR TLVReader::Get(int32_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(static_cast<uint32_t>(v64));
    return err;
}

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
CHIP_ERROR TLVReader::Get(int64_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(v64);
    return err;
}

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
CHIP_ERROR TLVReader::Get(uint8_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = static_cast<uint8_t>(v64);
    return err;
}

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
CHIP_ERROR TLVReader::Get(uint16_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = static_cast<uint16_t>(v64);
    return err;
}

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
CHIP_ERROR TLVReader::Get(uint32_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = static_cast<uint32_t>(v64);
    return err;
}

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
CHIP_ERROR TLVReader::Get(uint64_t & v)
{
    switch (ElementType())
    {
    case TLVElementType::Int8:
        v = static_cast<uint64_t>(static_cast<int64_t>(CastToSigned(static_cast<uint8_t>(mElemLenOrVal))));
        break;
    case TLVElementType::Int16:
        v = static_cast<uint64_t>(static_cast<int64_t>(CastToSigned(static_cast<uint16_t>(mElemLenOrVal))));
        break;
    case TLVElementType::Int32:
        v = static_cast<uint64_t>(static_cast<int64_t>(CastToSigned(static_cast<uint32_t>(mElemLenOrVal))));
        break;
    case TLVElementType::Int64:
    case TLVElementType::UInt8:
    case TLVElementType::UInt16:
    case TLVElementType::UInt32:
    case TLVElementType::UInt64:
        v = mElemLenOrVal;
        break;
    default:
        return CHIP_ERROR_WRONG_TLV_TYPE;
    }
    return CHIP_NO_ERROR;
}

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
CHIP_ERROR TLVReader::Get(double & v)
{
    switch (ElementType())
    {
    case TLVElementType::FloatingPointNumber32: {
        union
        {
            uint32_t u32;
            float f;
        } cvt;
        cvt.u32 = static_cast<uint32_t>(mElemLenOrVal);
        v       = cvt.f;
        break;
    }
    case TLVElementType::FloatingPointNumber64: {
        union
        {
            uint64_t u64;
            double d;
        } cvt;
        cvt.u64 = mElemLenOrVal;
        v       = cvt.d;
        break;
    }
    default:
        return CHIP_ERROR_WRONG_TLV_TYPE;
    }
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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::GetBytes(uint8_t * buf, uint32_t bufSize)
{
    if (!TLVTypeIsString(ElementType()))
        return CHIP_ERROR_WRONG_TLV_TYPE;

    if (mElemLenOrVal > bufSize)
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    CHIP_ERROR err = ReadData(buf, static_cast<uint32_t>(mElemLenOrVal));
    if (err != CHIP_NO_ERROR)
        return err;

    mElemLenOrVal = 0;

    return CHIP_NO_ERROR;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::GetString(char * buf, uint32_t bufSize)
{
    if (!TLVTypeIsString(ElementType()))
        return CHIP_ERROR_WRONG_TLV_TYPE;

    if ((mElemLenOrVal + 1) > bufSize)
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    buf[mElemLenOrVal] = 0;

    return GetBytes(reinterpret_cast<uint8_t *>(buf), bufSize - 1);
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer
 *                                      is non-NULL.
 *
 */
CHIP_ERROR TLVReader::DupBytes(uint8_t *& buf, uint32_t & dataLen)
{
    if (!TLVTypeIsString(ElementType()))
        return CHIP_ERROR_WRONG_TLV_TYPE;

    buf = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(static_cast<uint32_t>(mElemLenOrVal)));
    if (buf == nullptr)
        return CHIP_ERROR_NO_MEMORY;

    CHIP_ERROR err = ReadData(buf, static_cast<uint32_t>(mElemLenOrVal));
    if (err != CHIP_NO_ERROR)
    {
        chip::Platform::MemoryFree(buf);
        buf = nullptr;
        return err;
    }

    dataLen       = static_cast<uint32_t>(mElemLenOrVal);
    mElemLenOrVal = 0;

    return CHIP_NO_ERROR;
}

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
 * @retval other                        Other CHIP or platform error codes returned by the configured
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer
 *                                      is non-NULL.
 *
 */
CHIP_ERROR TLVReader::DupString(char *& buf)
{
    if (!TLVTypeIsString(ElementType()))
        return CHIP_ERROR_WRONG_TLV_TYPE;

    if (mElemLenOrVal > UINT32_MAX - 1)
        return CHIP_ERROR_NO_MEMORY;

    buf = static_cast<char *>(chip::Platform::MemoryAlloc(static_cast<uint32_t>(mElemLenOrVal + 1)));
    if (buf == nullptr)
        return CHIP_ERROR_NO_MEMORY;

    CHIP_ERROR err = ReadData(reinterpret_cast<uint8_t *>(buf), static_cast<uint32_t>(mElemLenOrVal));
    if (err != CHIP_NO_ERROR)
    {
        chip::Platform::MemoryFree(buf);
        buf = nullptr;
        return err;
    }

    buf[mElemLenOrVal] = 0;
    mElemLenOrVal      = 0;

    return err;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::GetDataPtr(const uint8_t *& data)
{
    CHIP_ERROR err;

    if (!TLVTypeIsString(ElementType()))
        return CHIP_ERROR_WRONG_TLV_TYPE;

    err = EnsureData(CHIP_ERROR_TLV_UNDERRUN);
    if (err != CHIP_NO_ERROR)
        return err;

    uint32_t remainingLen = static_cast<decltype(mMaxLen)>(mBufEnd - mReadPoint);

    // Verify that the entirety of the data is available in the buffer.
    // Note that this may not be possible if the reader is reading from a chain of buffers.
    if (remainingLen < static_cast<uint32_t>(mElemLenOrVal))
        return CHIP_ERROR_TLV_UNDERRUN;

    data = mReadPoint;

    return CHIP_NO_ERROR;
}

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
CHIP_ERROR TLVReader::OpenContainer(TLVReader & containerReader)
{
    TLVElementType elemType = ElementType();
    if (!TLVTypeIsContainer(elemType))
        return CHIP_ERROR_INCORRECT_STATE;

    containerReader.mBufHandle = mBufHandle;
    containerReader.mReadPoint = mReadPoint;
    containerReader.mBufEnd    = mBufEnd;
    containerReader.mLenRead   = mLenRead;
    containerReader.mMaxLen    = mMaxLen;
    containerReader.ClearElementState();
    containerReader.mContainerType = static_cast<TLVType>(elemType);
    containerReader.SetContainerOpen(false);
    containerReader.ImplicitProfileId = ImplicitProfileId;
    containerReader.AppData           = AppData;
    containerReader.GetNextBuffer     = GetNextBuffer;

    SetContainerOpen(true);

    return CHIP_NO_ERROR;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::CloseContainer(TLVReader & containerReader)
{
    CHIP_ERROR err;

    if (!IsContainerOpen())
        return CHIP_ERROR_INCORRECT_STATE;

    if (static_cast<TLVElementType>(containerReader.mContainerType) != ElementType())
        return CHIP_ERROR_INCORRECT_STATE;

    err = containerReader.SkipToEndOfContainer();
    if (err != CHIP_NO_ERROR)
        return err;

    mBufHandle = containerReader.mBufHandle;
    mReadPoint = containerReader.mReadPoint;
    mBufEnd    = containerReader.mBufEnd;
    mLenRead   = containerReader.mLenRead;
    mMaxLen    = containerReader.mMaxLen;
    ClearElementState();

    return CHIP_NO_ERROR;
}

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
CHIP_ERROR TLVReader::EnterContainer(TLVType & outerContainerType)
{
    TLVElementType elemType = ElementType();
    if (!TLVTypeIsContainer(elemType))
        return CHIP_ERROR_INCORRECT_STATE;

    outerContainerType = mContainerType;
    mContainerType     = static_cast<TLVType>(elemType);

    ClearElementState();
    SetContainerOpen(false);

    return CHIP_NO_ERROR;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::ExitContainer(TLVType outerContainerType)
{
    CHIP_ERROR err;

    err = SkipToEndOfContainer();
    if (err != CHIP_NO_ERROR)
        return err;

    mContainerType = outerContainerType;
    ClearElementState();

    return CHIP_NO_ERROR;
}

/**
 * Verifies that the TVLReader object is at the end of a TLV container.
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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::VerifyEndOfContainer()
{
    CHIP_ERROR err = Next();
    if (err == CHIP_END_OF_TLV)
        return CHIP_NO_ERROR;
    if (err == CHIP_NO_ERROR)
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    return err;
}

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
TLVType TLVReader::GetContainerType() const
{
    return mContainerType;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::Next()
{
    CHIP_ERROR err;
    TLVElementType elemType = ElementType();

    err = Skip();
    if (err != CHIP_NO_ERROR)
        return err;

    err = ReadElement();
    if (err != CHIP_NO_ERROR)
        return err;

    elemType = ElementType();
    if (elemType == TLVElementType::EndOfContainer)
        return CHIP_END_OF_TLV;

    return CHIP_NO_ERROR;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::Next(TLVType expectedType, uint64_t expectedTag)
{
    CHIP_ERROR err = Next();
    if (err != CHIP_NO_ERROR)
        return err;
    if (GetType() != expectedType)
        return CHIP_ERROR_WRONG_TLV_TYPE;
    if (mElemTag != expectedTag)
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    return CHIP_NO_ERROR;
}

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
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 *
 */
CHIP_ERROR TLVReader::Skip()
{
    CHIP_ERROR err;
    TLVElementType elemType = ElementType();

    if (elemType == TLVElementType::EndOfContainer)
        return CHIP_END_OF_TLV;

    if (TLVTypeIsContainer(elemType))
    {
        TLVType outerContainerType;
        err = EnterContainer(outerContainerType);
        if (err != CHIP_NO_ERROR)
            return err;
        err = ExitContainer(outerContainerType);
        if (err != CHIP_NO_ERROR)
            return err;
    }

    else
    {
        err = SkipData();
        if (err != CHIP_NO_ERROR)
            return err;

        ClearElementState();
    }

    return CHIP_NO_ERROR;
}

/**
 * Clear the state of the TLVReader.
 * This method is used to position the reader before the first TLV,
 * between TLVs or after the last TLV.
 */
void TLVReader::ClearElementState()
{
    mElemTag      = AnonymousTag;
    mControlByte  = kTLVControlByte_NotSpecified;
    mElemLenOrVal = 0;
}

/**
 * Skip any data contained in the current TLV by reading over it without
 * a destination buffer.
 *
 * @retval #CHIP_NO_ERROR              If the reader was successfully positioned at the end of the
 *                                      data.
 * @retval other                        Other CHIP or platform error codes returned by the configured
 *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
 *                                      non-NULL.
 */
CHIP_ERROR TLVReader::SkipData()
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    TLVElementType elemType = ElementType();

    if (TLVTypeHasLength(elemType))
    {
        err = ReadData(nullptr, static_cast<uint32_t>(mElemLenOrVal));
        if (err != CHIP_NO_ERROR)
            return err;
    }

    return err;
}

CHIP_ERROR TLVReader::SkipToEndOfContainer()
{
    CHIP_ERROR err;
    TLVType outerContainerType = mContainerType;
    uint32_t nestLevel         = 0;

    // If the user calls Next() after having called OpenContainer() but before calling
    // CloseContainer() they're effectively doing a close container by skipping over
    // the container element.  So reset the 'container open' flag here to prevent them
    // from calling CloseContainer() with the now orphaned container reader.
    SetContainerOpen(false);

    while (true)
    {
        TLVElementType elemType = ElementType();

        if (elemType == TLVElementType::EndOfContainer)
        {
            if (nestLevel == 0)
                return CHIP_NO_ERROR;

            nestLevel--;
            mContainerType = (nestLevel == 0) ? outerContainerType : kTLVType_UnknownContainer;
        }

        else if (TLVTypeIsContainer(elemType))
        {
            nestLevel++;
            mContainerType = static_cast<TLVType>(elemType);
        }

        err = SkipData();
        if (err != CHIP_NO_ERROR)
            return err;

        err = ReadElement();
        if (err != CHIP_NO_ERROR)
            return err;
    }
}

CHIP_ERROR TLVReader::ReadElement()
{
    CHIP_ERROR err;
    uint8_t stagingBuf[17]; // 17 = 1 control byte + 8 tag bytes + 8 length/value bytes
    const uint8_t * p;
    TLVElementType elemType;

    // Make sure we have input data. Return CHIP_END_OF_TLV if no more data is available.
    err = EnsureData(CHIP_END_OF_TLV);
    if (err != CHIP_NO_ERROR)
        return err;

    // Get the element's control byte.
    mControlByte = *mReadPoint;

    // Extract the element type from the control byte. Fail if it's invalid.
    elemType = ElementType();
    if (!IsValidTLVType(elemType))
        return CHIP_ERROR_INVALID_TLV_ELEMENT;

    // Extract the tag control from the control byte.
    TLVTagControl tagControl = static_cast<TLVTagControl>(mControlByte & kTLVTagControlMask);

    // Determine the number of bytes in the element's tag, if any.
    uint8_t tagBytes = sTagSizes[tagControl >> kTLVTagControlShift];

    // Extract the size of length/value field from the control byte.
    TLVFieldSize lenOrValFieldSize = GetTLVFieldSize(elemType);

    // Determine the number of bytes in the length/value field.
    uint8_t valOrLenBytes = TLVFieldSizeToBytes(lenOrValFieldSize);

    // Determine the number of bytes in the element's 'head'. This includes: the control byte, the tag bytes (if present), the
    // length bytes (if present), and for elements that don't have a length (e.g. integers), the value bytes.
    uint8_t elemHeadBytes = static_cast<uint8_t>(1 + tagBytes + valOrLenBytes);

    // If the head of the element overlaps the end of the input buffer, read the bytes into the staging buffer
    // and arrange to parse them from there. Otherwise read them directly from the input buffer.
    if (elemHeadBytes > (mBufEnd - mReadPoint))
    {
        err = ReadData(stagingBuf, elemHeadBytes);
        if (err != CHIP_NO_ERROR)
            return err;
        p = stagingBuf;
    }
    else
    {
        p = mReadPoint;
        mReadPoint += elemHeadBytes;
        mLenRead += elemHeadBytes;
    }

    // Skip over the control byte.
    p++;

    // Read the tag field, if present.
    mElemTag = ReadTag(tagControl, p);

    // Read the length/value field, if present.
    switch (lenOrValFieldSize)
    {
    case kTLVFieldSize_0Byte:
        mElemLenOrVal = 0;
        break;
    case kTLVFieldSize_1Byte:
        mElemLenOrVal = Read8(p);
        break;
    case kTLVFieldSize_2Byte:
        mElemLenOrVal = LittleEndian::Read16(p);
        break;
    case kTLVFieldSize_4Byte:
        mElemLenOrVal = LittleEndian::Read32(p);
        break;
    case kTLVFieldSize_8Byte:
        mElemLenOrVal = LittleEndian::Read64(p);
        break;
    }

    return VerifyElement();
}

CHIP_ERROR TLVReader::VerifyElement()
{
    if (ElementType() == TLVElementType::EndOfContainer)
    {
        if (mContainerType == kTLVType_NotSpecified)
            return CHIP_ERROR_INVALID_TLV_ELEMENT;
        if (mElemTag != AnonymousTag)
            return CHIP_ERROR_INVALID_TLV_TAG;
    }
    else
    {
        if (mElemTag == UnknownImplicitTag)
            return CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG;
        switch (mContainerType)
        {
        case kTLVType_NotSpecified:
            if (IsContextTag(mElemTag))
                return CHIP_ERROR_INVALID_TLV_TAG;
            break;
        case kTLVType_Structure:
            if (mElemTag == AnonymousTag)
                return CHIP_ERROR_INVALID_TLV_TAG;
            break;
        case kTLVType_Array:
            if (mElemTag != AnonymousTag)
                return CHIP_ERROR_INVALID_TLV_TAG;
            break;
        case kTLVType_UnknownContainer:
        case kTLVType_Path:
            break;
        default:
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    // If the current element encodes a specific length (e.g. a UTF8 string or a byte string), verify
    // that the purported length fits within the remaining bytes of the encoding (as delineated by mMaxLen).
    //
    // Note that this check is not strictly necessary to prevent runtime errors, as any attempt to access
    // the data of an element with an invalid length will result in an error.  However checking the length
    // here catches the error earlier, and ensures that the application will never see the erroneous length
    // value.
    //
    if (TLVTypeHasLength(ElementType()))
    {
        uint32_t overallLenRemaining = mMaxLen - mLenRead;
        if (overallLenRemaining < static_cast<uint32_t>(mElemLenOrVal))
            return CHIP_ERROR_TLV_UNDERRUN;
    }

    return CHIP_NO_ERROR;
}

uint64_t TLVReader::ReadTag(TLVTagControl tagControl, const uint8_t *& p)
{
    uint16_t vendorId;
    uint16_t profileNum;

    switch (tagControl)
    {
    case TLVTagControl::ContextSpecific:
        return ContextTag(Read8(p));
    case TLVTagControl::CommonProfile_2Bytes:
        return CommonTag(LittleEndian::Read16(p));
    case TLVTagControl::CommonProfile_4Bytes:
        return CommonTag(LittleEndian::Read32(p));
    case TLVTagControl::ImplicitProfile_2Bytes:
        if (ImplicitProfileId == kProfileIdNotSpecified)
            return UnknownImplicitTag;
        return ProfileTag(ImplicitProfileId, LittleEndian::Read16(p));
    case TLVTagControl::ImplicitProfile_4Bytes:
        if (ImplicitProfileId == kProfileIdNotSpecified)
            return UnknownImplicitTag;
        return ProfileTag(ImplicitProfileId, LittleEndian::Read32(p));
    case TLVTagControl::FullyQualified_6Bytes:
        vendorId   = LittleEndian::Read16(p);
        profileNum = LittleEndian::Read16(p);
        return ProfileTag(vendorId, profileNum, LittleEndian::Read16(p));
    case TLVTagControl::FullyQualified_8Bytes:
        vendorId   = LittleEndian::Read16(p);
        profileNum = LittleEndian::Read16(p);
        return ProfileTag(vendorId, profileNum, LittleEndian::Read32(p));
    case TLVTagControl::Anonymous:
    default:
        return AnonymousTag;
    }
}

CHIP_ERROR TLVReader::ReadData(uint8_t * buf, uint32_t len)
{
    CHIP_ERROR err;

    while (len > 0)
    {
        err = EnsureData(CHIP_ERROR_TLV_UNDERRUN);
        if (err != CHIP_NO_ERROR)
            return err;

        uint32_t remainingLen = static_cast<decltype(mMaxLen)>(mBufEnd - mReadPoint);

        uint32_t readLen = len;
        if (readLen > remainingLen)
            readLen = remainingLen;

        if (buf != nullptr)
        {
            memcpy(buf, mReadPoint, readLen);
            buf += readLen;
        }
        mReadPoint += readLen;
        mLenRead += readLen;
        len -= readLen;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVReader::EnsureData(CHIP_ERROR noDataErr)
{
    CHIP_ERROR err;

    if (mReadPoint == mBufEnd)
    {
        if (mLenRead == mMaxLen)
            return noDataErr;

        if (GetNextBuffer == nullptr)
            return noDataErr;

        uint32_t bufLen;
        err = GetNextBuffer(*this, mBufHandle, mReadPoint, bufLen);
        if (err != CHIP_NO_ERROR)
            return err;
        if (bufLen == 0)
            return noDataErr;

        // Cap mBufEnd so that we don't read beyond the user's specified maximum length, even
        // if the underlying buffer is larger.
        uint32_t overallLenRemaining = mMaxLen - mLenRead;
        if (overallLenRemaining < bufLen)
            bufLen = overallLenRemaining;

        mBufEnd = mReadPoint + bufLen;
    }

    return CHIP_NO_ERROR;
}

/**
 * This is a private method used to compute the length of a TLV element head.
 */
CHIP_ERROR TLVReader::GetElementHeadLength(uint8_t & elemHeadBytes) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t tagBytes;
    uint8_t valOrLenBytes;
    TLVTagControl tagControl;
    TLVFieldSize lenOrValFieldSize;
    TLVElementType elemType = ElementType();

    // Verify element is of valid TLVType.
    VerifyOrExit(IsValidTLVType(elemType), err = CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Extract the tag control from the control byte.
    tagControl = static_cast<TLVTagControl>(mControlByte & kTLVTagControlMask);

    // Determine the number of bytes in the element's tag, if any.
    tagBytes = sTagSizes[tagControl >> kTLVTagControlShift];

    // Extract the size of length/value field from the control byte.
    lenOrValFieldSize = GetTLVFieldSize(elemType);

    // Determine the number of bytes in the length/value field.
    valOrLenBytes = TLVFieldSizeToBytes(lenOrValFieldSize);

    // Determine the number of bytes in the element's 'head'. This includes: the
    // control byte, the tag bytes (if present), the length bytes (if present),
    // and for elements that don't have a length (e.g. integers), the value
    // bytes.
    VerifyOrExit(CanCastTo<uint8_t>(1 + tagBytes + valOrLenBytes), err = CHIP_ERROR_INTERNAL);
    elemHeadBytes = static_cast<uint8_t>(1 + tagBytes + valOrLenBytes);

exit:
    return err;
}

/**
 * This is a private method that returns the TLVElementType from mControlByte
 */
TLVElementType TLVReader::ElementType() const
{
    if (mControlByte == static_cast<uint16_t>(kTLVControlByte_NotSpecified))
        return TLVElementType::NotSpecified;
    return static_cast<TLVElementType>(mControlByte & kTLVTypeMask);
}

CHIP_ERROR TLVReader::GetNextPacketBuffer(TLVReader & reader, uintptr_t & bufHandle, const uint8_t *& bufStart, uint32_t & bufLen)
{
    PacketBuffer *& buf = reinterpret_cast<PacketBuffer *&>(bufHandle);

    if (buf != nullptr)
        buf = buf->Next_ForNow();
    if (buf != nullptr)
    {
        bufStart = buf->Start();
        bufLen   = buf->DataLength();
    }
    else
    {
        bufStart = nullptr;
        bufLen   = 0;
    }

    return CHIP_NO_ERROR;
}

/**
 * Position the destination reader on the next element with the given tag within this reader's current container context
 *
 * @param[in] tag                      The destination context tag value
 * @param[in] destReader               The destination TLV reader value that was located by given tag
 *
 * @retval #CHIP_NO_ERROR              If the reader was successfully positioned at the given tag
 * @retval #CHIP_END_OF_TLV            If the given tag cannot be found
 * @retval other                       Other CHIP or platform error codes
 */
CHIP_ERROR TLVReader::FindElementWithTag(const uint64_t tag, TLVReader & destReader) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::TLV::TLVReader reader;
    reader.Init(*this);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::kTLVType_NotSpecified != reader.GetType(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);

        if (tag == reader.GetTag())
        {
            destReader.Init(reader);
            break;
        }
    }

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

} // namespace TLV
} // namespace chip
