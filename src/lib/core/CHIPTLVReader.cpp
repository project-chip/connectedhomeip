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

namespace chip {
namespace TLV {

using namespace chip::Encoding;

static const uint8_t sTagSizes[] = { 0, 1, 2, 4, 2, 4, 6, 8 };

void TLVReader::Init(const uint8_t * data, uint32_t dataLen)
{
    mBackingStore = nullptr;
    mReadPoint    = data;
    mBufEnd       = data + dataLen;
    mLenRead      = 0;
    mMaxLen       = dataLen;
    ClearElementState();
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);

    ImplicitProfileId = kProfileIdNotSpecified;
}

CHIP_ERROR TLVReader::Init(TLVBackingStore & backingStore, uint32_t maxLen)
{
    mBackingStore   = &backingStore;
    mReadPoint      = nullptr;
    uint32_t bufLen = 0;
    CHIP_ERROR err  = mBackingStore->OnInit(*this, mReadPoint, bufLen);
    if (err != CHIP_NO_ERROR)
        return err;

    mBufEnd  = mReadPoint + bufLen;
    mLenRead = 0;
    mMaxLen  = maxLen;
    ClearElementState();
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);

    ImplicitProfileId = kProfileIdNotSpecified;
    AppData           = nullptr;
    return CHIP_NO_ERROR;
}

void TLVReader::Init(const TLVReader & aReader)
{
    // Initialize private data members

    mElemTag       = aReader.mElemTag;
    mElemLenOrVal  = aReader.mElemLenOrVal;
    mBackingStore  = aReader.mBackingStore;
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
}

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

uint32_t TLVReader::GetLength() const
{
    if (TLVTypeHasLength(ElementType()))
        return static_cast<uint32_t>(mElemLenOrVal);
    return 0;
}

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

CHIP_ERROR TLVReader::Get(int8_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(static_cast<uint8_t>(v64));
    return err;
}

CHIP_ERROR TLVReader::Get(int16_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(static_cast<uint16_t>(v64));
    return err;
}

CHIP_ERROR TLVReader::Get(int32_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(static_cast<uint32_t>(v64));
    return err;
}

CHIP_ERROR TLVReader::Get(int64_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = CastToSigned(v64);
    return err;
}

CHIP_ERROR TLVReader::Get(uint8_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = static_cast<uint8_t>(v64);
    return err;
}

CHIP_ERROR TLVReader::Get(uint16_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = static_cast<uint16_t>(v64);
    return err;
}

CHIP_ERROR TLVReader::Get(uint32_t & v)
{
    uint64_t v64   = 0;
    CHIP_ERROR err = Get(v64);
    v              = static_cast<uint32_t>(v64);
    return err;
}

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

CHIP_ERROR TLVReader::GetString(char * buf, uint32_t bufSize)
{
    if (!TLVTypeIsString(ElementType()))
        return CHIP_ERROR_WRONG_TLV_TYPE;

    if ((mElemLenOrVal + 1) > bufSize)
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    buf[mElemLenOrVal] = 0;

    return GetBytes(reinterpret_cast<uint8_t *>(buf), bufSize - 1);
}

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

CHIP_ERROR TLVReader::OpenContainer(TLVReader & containerReader)
{
    TLVElementType elemType = ElementType();
    if (!TLVTypeIsContainer(elemType))
        return CHIP_ERROR_INCORRECT_STATE;

    containerReader.mBackingStore = mBackingStore;
    containerReader.mReadPoint    = mReadPoint;
    containerReader.mBufEnd       = mBufEnd;
    containerReader.mLenRead      = mLenRead;
    containerReader.mMaxLen       = mMaxLen;
    containerReader.ClearElementState();
    containerReader.mContainerType = static_cast<TLVType>(elemType);
    containerReader.SetContainerOpen(false);
    containerReader.ImplicitProfileId = ImplicitProfileId;
    containerReader.AppData           = AppData;

    SetContainerOpen(true);

    return CHIP_NO_ERROR;
}

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

    mBackingStore = containerReader.mBackingStore;
    mReadPoint    = containerReader.mReadPoint;
    mBufEnd       = containerReader.mBufEnd;
    mLenRead      = containerReader.mLenRead;
    mMaxLen       = containerReader.mMaxLen;
    ClearElementState();

    return CHIP_NO_ERROR;
}

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

CHIP_ERROR TLVReader::VerifyEndOfContainer()
{
    CHIP_ERROR err = Next();
    if (err == CHIP_END_OF_TLV)
        return CHIP_NO_ERROR;
    if (err == CHIP_NO_ERROR)
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    return err;
}

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
 *                                      TLVBackingStore.
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

    if (mReadPoint == nullptr)
    {
        return CHIP_ERROR_INVALID_TLV_ELEMENT;
    }
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
        case kTLVType_List:
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

        if (mBackingStore == nullptr)
            return noDataErr;

        uint32_t bufLen;
        err = mBackingStore->GetNextBuffer(*this, mReadPoint, bufLen);
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
    uint8_t tagBytes;
    uint8_t valOrLenBytes;
    TLVTagControl tagControl;
    TLVFieldSize lenOrValFieldSize;
    TLVElementType elemType = ElementType();

    // Verify element is of valid TLVType.
    VerifyOrReturnError(IsValidTLVType(elemType), CHIP_ERROR_INVALID_TLV_ELEMENT);

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
    VerifyOrReturnError(CanCastTo<uint8_t>(1 + tagBytes + valOrLenBytes), CHIP_ERROR_INTERNAL);
    elemHeadBytes = static_cast<uint8_t>(1 + tagBytes + valOrLenBytes);

    return CHIP_NO_ERROR;
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
