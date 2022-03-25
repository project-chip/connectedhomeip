/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file implements an updating encoder for the CHIP TLV
 *      (Tag-Length-Value) encoding format.
 *
 */

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace TLV {

using namespace chip::Encoding;

CHIP_ERROR TLVUpdater::Init(uint8_t * buf, uint32_t dataLen, uint32_t maxLen)
{
    uint32_t freeLen;

    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(maxLen >= dataLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    // memmove the buffer data to end of the buffer
    freeLen = maxLen - dataLen;
    memmove(buf + freeLen, buf, dataLen);

    // Init reader
    mUpdaterReader.Init(buf + freeLen, dataLen);

    // Init writer
    mUpdaterWriter.Init(buf, freeLen);
    mUpdaterWriter.SetCloseContainerReserved(false);
    mElementStartAddr = buf + freeLen;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVUpdater::Init(TLVReader & aReader, uint32_t freeLen)
{
    uint8_t * buf             = const_cast<uint8_t *>(aReader.GetReadPoint());
    uint32_t remainingDataLen = aReader.GetRemainingLength();
    uint32_t readDataLen      = aReader.GetLengthRead();

    // TLVUpdater does not support backing stores yet
    VerifyOrReturnError(aReader.mBackingStore == nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    // TLVReader should point to a non-NULL buffer
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // If reader is already on an element, reset it to start of element
    if (aReader.ElementType() != TLVElementType::NotSpecified)
    {
        uint8_t elemHeadLen;

        ReturnErrorOnFailure(aReader.GetElementHeadLength(elemHeadLen));

        buf -= elemHeadLen;
        remainingDataLen += elemHeadLen;
        readDataLen -= elemHeadLen;
    }

    // memmove the buffer data to end of the buffer
    memmove(buf + freeLen, buf, remainingDataLen);

    // Initialize the internal reader object
    mUpdaterReader.mBackingStore  = nullptr;
    mUpdaterReader.mReadPoint     = buf + freeLen;
    mUpdaterReader.mBufEnd        = buf + freeLen + remainingDataLen;
    mUpdaterReader.mLenRead       = readDataLen;
    mUpdaterReader.mMaxLen        = aReader.mMaxLen;
    mUpdaterReader.mControlByte   = kTLVControlByte_NotSpecified;
    mUpdaterReader.mElemTag       = AnonymousTag();
    mUpdaterReader.mElemLenOrVal  = 0;
    mUpdaterReader.mContainerType = aReader.mContainerType;
    mUpdaterReader.SetContainerOpen(false);

    mUpdaterReader.ImplicitProfileId = aReader.ImplicitProfileId;
    mUpdaterReader.AppData           = aReader.AppData;

    // Initialize the internal writer object
    mUpdaterWriter.mBackingStore  = nullptr;
    mUpdaterWriter.mBufStart      = buf - readDataLen;
    mUpdaterWriter.mWritePoint    = buf;
    mUpdaterWriter.mRemainingLen  = freeLen;
    mUpdaterWriter.mLenWritten    = readDataLen;
    mUpdaterWriter.mMaxLen        = readDataLen + freeLen;
    mUpdaterWriter.mContainerType = aReader.mContainerType;
    mUpdaterWriter.SetContainerOpen(false);
    mUpdaterWriter.SetCloseContainerReserved(false);

    mUpdaterWriter.ImplicitProfileId = aReader.ImplicitProfileId;

    // Cache element start address for internal use
    mElementStartAddr = buf + freeLen;

    // Clear the input reader object before returning. The user can no longer
    // use the original TLVReader object anymore.
    aReader.Init(static_cast<const uint8_t *>(nullptr), 0);

    return CHIP_NO_ERROR;
}

void TLVUpdater::SetImplicitProfileId(uint32_t profileId)
{
    mUpdaterReader.ImplicitProfileId = profileId;
    mUpdaterWriter.ImplicitProfileId = profileId;
}

CHIP_ERROR TLVUpdater::Next()
{
    // Skip current element if the reader is already positioned on an element
    ReturnErrorOnFailure(mUpdaterReader.Skip());

    AdjustInternalWriterFreeSpace();

    // Move the reader to next element
    ReturnErrorOnFailure(mUpdaterReader.Next());

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVUpdater::Move()
{
    const uint8_t * elementEnd;
    uint32_t copyLen;

    VerifyOrReturnError(static_cast<TLVElementType>((mUpdaterReader.mControlByte & kTLVTypeMask)) != TLVElementType::EndOfContainer,
                        CHIP_END_OF_TLV);

    VerifyOrReturnError(mUpdaterReader.GetType() != kTLVType_NotSpecified, CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Skip to the end of the element
    ReturnErrorOnFailure(mUpdaterReader.Skip());

    elementEnd = mUpdaterReader.mReadPoint;

    copyLen = static_cast<uint32_t>(elementEnd - mElementStartAddr);

    // Move the element to output TLV
    memmove(mUpdaterWriter.mWritePoint, mElementStartAddr, copyLen);

    // Adjust the updater state
    mElementStartAddr += copyLen;
    mUpdaterWriter.mWritePoint += copyLen;
    mUpdaterWriter.mLenWritten += copyLen;
    mUpdaterWriter.mMaxLen += copyLen;

    return CHIP_NO_ERROR;
}

void TLVUpdater::MoveUntilEnd()
{
    const uint8_t * buffEnd = mUpdaterReader.GetReadPoint() + mUpdaterReader.GetRemainingLength();

    uint32_t copyLen = static_cast<uint32_t>(buffEnd - mElementStartAddr);

    // Move all elements till end to output TLV
    memmove(mUpdaterWriter.mWritePoint, mElementStartAddr, copyLen);

    // Adjust the updater state
    mElementStartAddr += copyLen;
    mUpdaterWriter.mWritePoint += copyLen;
    mUpdaterWriter.mLenWritten += copyLen;
    mUpdaterWriter.mMaxLen += copyLen;
    mUpdaterWriter.mContainerType = kTLVType_NotSpecified;
    mUpdaterWriter.SetContainerOpen(false);
    mUpdaterWriter.SetCloseContainerReserved(false);
    mUpdaterReader.mReadPoint += copyLen;
    mUpdaterReader.mLenRead += copyLen;
    mUpdaterReader.mControlByte   = kTLVControlByte_NotSpecified;
    mUpdaterReader.mElemTag       = AnonymousTag();
    mUpdaterReader.mElemLenOrVal  = 0;
    mUpdaterReader.mContainerType = kTLVType_NotSpecified;
    mUpdaterReader.SetContainerOpen(false);
}

CHIP_ERROR TLVUpdater::EnterContainer(TLVType & outerContainerType)
{
    TLVType containerType;

    VerifyOrReturnError(TLVTypeIsContainer(static_cast<TLVType>(mUpdaterReader.mControlByte & kTLVTypeMask)),
                        CHIP_ERROR_INCORRECT_STATE);

    // Change the updater state
    AdjustInternalWriterFreeSpace();

    ReturnErrorOnFailure(mUpdaterWriter.StartContainer(mUpdaterReader.GetTag(), mUpdaterReader.GetType(), containerType));

    ReturnErrorOnFailure(mUpdaterReader.EnterContainer(containerType));

    outerContainerType = containerType;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVUpdater::ExitContainer(TLVType outerContainerType)
{
    ReturnErrorOnFailure(mUpdaterReader.ExitContainer(outerContainerType));

    // Change the updater's state
    AdjustInternalWriterFreeSpace();

    ReturnErrorOnFailure(mUpdaterWriter.EndContainer(outerContainerType));

    return CHIP_NO_ERROR;
}

/**
 * This is a private method that adjusts the TLVUpdater's free space count by
 * accounting for the freespace from mElementStartAddr to current read point.
 */
void TLVUpdater::AdjustInternalWriterFreeSpace()
{
    const uint8_t * nextElementStart = mUpdaterReader.mReadPoint;

    if (nextElementStart != mElementStartAddr)
    {
        // Increase the internal writer's free space state variables
        uint32_t spaceIncrease = static_cast<uint32_t>(nextElementStart - mElementStartAddr);
        mUpdaterWriter.mRemainingLen += spaceIncrease;
        mUpdaterWriter.mMaxLen += spaceIncrease;

        // Cache the start address of the next element
        mElementStartAddr = nextElementStart;
    }
}

} // namespace TLV
} // namespace chip
