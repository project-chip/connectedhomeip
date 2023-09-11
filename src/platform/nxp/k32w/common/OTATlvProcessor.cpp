/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/core/TLV.h>
#include <lib/support/BufferReader.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/k32w/common/OTAImageProcessorImpl.h>
#include <platform/nxp/k32w/common/OTATlvProcessor.h>

namespace chip {

CHIP_ERROR OTATlvProcessor::Process(ByteSpan & block)
{
    CHIP_ERROR status     = CHIP_NO_ERROR;
    uint32_t bytes        = chip::min(mLength - mProcessedLength, static_cast<uint32_t>(block.size()));
    ByteSpan relevantData = block.SubSpan(0, bytes);

    status = ProcessInternal(relevantData);
    if (!IsError(status))
    {
        mProcessedLength += bytes;
        block = block.SubSpan(bytes);
        if (mProcessedLength == mLength)
        {
            status = ExitAction();
            if (!IsError(status) && (block.size() > 0))
            {
                // If current block was processed fully and the block still contains data, it
                // means that the block contains another TLV's data and the current processor
                // should be changed by OTAImageProcessorImpl.
                return CHIP_OTA_CHANGE_PROCESSOR;
            }
        }
    }

    return status;
}

void OTATlvProcessor::ClearInternal()
{
    mLength          = 0;
    mProcessedLength = 0;
    mWasSelected     = false;
}

bool OTATlvProcessor::IsError(CHIP_ERROR & status)
{
    return status != CHIP_NO_ERROR && status != CHIP_ERROR_BUFFER_TOO_SMALL && status != CHIP_OTA_FETCH_ALREADY_SCHEDULED;
}

void OTADataAccumulator::Init(uint32_t threshold)
{
    mThreshold    = threshold;
    mBufferOffset = 0;
    mBuffer.Alloc(mThreshold);
}

void OTADataAccumulator::Clear()
{
    mThreshold    = 0;
    mBufferOffset = 0;
    mBuffer.Free();
}

CHIP_ERROR OTADataAccumulator::Accumulate(ByteSpan & block)
{
    uint32_t numBytes = chip::min(mThreshold - mBufferOffset, static_cast<uint32_t>(block.size()));
    memcpy(&mBuffer[mBufferOffset], block.data(), numBytes);
    mBufferOffset += numBytes;
    block = block.SubSpan(numBytes);

    if (mBufferOffset < mThreshold)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
