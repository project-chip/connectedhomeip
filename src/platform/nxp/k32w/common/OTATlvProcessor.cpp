/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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
        if (mProcessedLength == mLength && block.size() > 0)
        {
            // If current block was processed fully and the block still contains data, it
            // means that the block contains another TLV's data and the current processor
            // should be changed by OTAImageProcessorImpl.
            return CHIP_OTA_CHANGE_PROCESSOR;
        }
    }

    return status;
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
