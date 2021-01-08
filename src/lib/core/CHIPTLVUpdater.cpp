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

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>

namespace chip {
namespace TLV {

using namespace chip::Encoding;

CHIP_ERROR TLVUpdater::Init(uint8_t * buf, uint32_t dataLen, uint32_t maxLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t freeLen;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(maxLen >= dataLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // memmove the buffer data to end of the buffer
    freeLen = maxLen - dataLen;
    memmove(buf + freeLen, buf, dataLen);

    // Init reader
    mUpdaterReader.Init(buf + freeLen, dataLen);

    // Init writer
    mUpdaterWriter.Init(buf, freeLen);
    mUpdaterWriter.SetCloseContainerReserved(false);
    mElementStartAddr = buf + freeLen;

exit:
    return err;
}

CHIP_ERROR TLVUpdater::Init(TLVReader & aReader, uint32_t freeLen)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    uint8_t * buf             = const_cast<uint8_t *>(aReader.GetReadPoint());
    uint32_t remainingDataLen = aReader.GetRemainingLength();
    uint32_t readDataLen      = aReader.GetLengthRead();

    // TLVUpdater does not support backing stores yet
    VerifyOrExit(aReader.mBackingStore == 0, err = CHIP_ERROR_NOT_IMPLEMENTED);

    // TLVReader should point to a non-NULL buffer
    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    // If reader is already on an element, reset it to start of element
    if (aReader.ElementType() != TLVElementType::NotSpecified)
    {
        uint8_t elemHeadLen;

        err = aReader.GetElementHeadLength(elemHeadLen);
        SuccessOrExit(err);

        buf -= elemHeadLen;
        remainingDataLen += elemHeadLen;
        readDataLen -= elemHeadLen;
    }

    // memmove the buffer data to end of the buffer
    memmove(buf + freeLen, buf, remainingDataLen);

    // Initialize the internal reader object
    mUpdaterReader.mBackingStore  = 0;
    mUpdaterReader.mReadPoint     = buf + freeLen;
    mUpdaterReader.mBufEnd        = buf + freeLen + remainingDataLen;
    mUpdaterReader.mLenRead       = readDataLen;
    mUpdaterReader.mMaxLen        = aReader.mMaxLen;
    mUpdaterReader.mControlByte   = kTLVControlByte_NotSpecified;
    mUpdaterReader.mElemTag       = AnonymousTag;
    mUpdaterReader.mElemLenOrVal  = 0;
    mUpdaterReader.mContainerType = aReader.mContainerType;
    mUpdaterReader.SetContainerOpen(false);

    mUpdaterReader.ImplicitProfileId = aReader.ImplicitProfileId;
    mUpdaterReader.AppData           = aReader.AppData;

    // Initialize the internal writer object
    mUpdaterWriter.mBackingStore  = 0;
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

exit:
    return err;
}

void TLVUpdater::SetImplicitProfileId(uint32_t profileId)
{
    mUpdaterReader.ImplicitProfileId = profileId;
    mUpdaterWriter.ImplicitProfileId = profileId;
}

CHIP_ERROR TLVUpdater::Next()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Skip current element if the reader is already positioned on an element
    err = mUpdaterReader.Skip();
    SuccessOrExit(err);

    AdjustInternalWriterFreeSpace();

    // Move the reader to next element
    err = mUpdaterReader.Next();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR TLVUpdater::Move()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const uint8_t * elementEnd;
    uint32_t copyLen;

    VerifyOrExit(static_cast<TLVElementType>((mUpdaterReader.mControlByte & kTLVTypeMask)) != TLVElementType::EndOfContainer,
                 err = CHIP_END_OF_TLV);

    VerifyOrExit(mUpdaterReader.GetType() != kTLVType_NotSpecified, err = CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Skip to the end of the element
    err = mUpdaterReader.Skip();
    SuccessOrExit(err);

    elementEnd = mUpdaterReader.mReadPoint;

    copyLen = static_cast<uint32_t>(elementEnd - mElementStartAddr);

    // Move the element to output TLV
    memmove(mUpdaterWriter.mWritePoint, mElementStartAddr, copyLen);

    // Adjust the updater state
    mElementStartAddr += copyLen;
    mUpdaterWriter.mWritePoint += copyLen;
    mUpdaterWriter.mLenWritten += copyLen;
    mUpdaterWriter.mMaxLen += copyLen;

exit:
    return err;
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
    mUpdaterReader.mElemTag       = AnonymousTag;
    mUpdaterReader.mElemLenOrVal  = 0;
    mUpdaterReader.mContainerType = kTLVType_NotSpecified;
    mUpdaterReader.SetContainerOpen(false);
}

CHIP_ERROR TLVUpdater::EnterContainer(TLVType & outerContainerType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVType containerType;

    VerifyOrExit(TLVTypeIsContainer(static_cast<TLVType>(mUpdaterReader.mControlByte & kTLVTypeMask)),
                 err = CHIP_ERROR_INCORRECT_STATE);

    // Change the updater state
    AdjustInternalWriterFreeSpace();

    err = mUpdaterWriter.StartContainer(mUpdaterReader.GetTag(), mUpdaterReader.GetType(), containerType);
    SuccessOrExit(err);

    err = mUpdaterReader.EnterContainer(containerType);
    SuccessOrExit(err);

    outerContainerType = containerType;

exit:
    return err;
}

CHIP_ERROR TLVUpdater::ExitContainer(TLVType outerContainerType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mUpdaterReader.ExitContainer(outerContainerType);
    SuccessOrExit(err);

    // Change the updater's state
    AdjustInternalWriterFreeSpace();

    err = mUpdaterWriter.EndContainer(outerContainerType);
    SuccessOrExit(err);

exit:
    return err;
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
