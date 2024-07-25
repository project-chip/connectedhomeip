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

#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>
#include <platform/silabs/multi-ota/OTATlvProcessor.h>
#if OTA_ENCRYPTION_ENABLE
#include "OtaUtils.h"
#include "rom_aes.h"
#endif
namespace chip {

#if OTA_ENCRYPTION_ENABLE
constexpr uint8_t au8Iv[] = { 0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x00, 0x00, 0x00, 0x00 };
#endif
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
            if (!IsError(status))
            {
                // If current block was processed fully and the block still contains data, it
                // means that the block contains another TLV's data and the current processor
                // should be changed by OTAMultiImageProcessorImpl.
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
#if OTA_ENCRYPTION_ENABLE
    mIVOffset = 0;
#endif
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

#if OTA_ENCRYPTION_ENABLE
CHIP_ERROR OTATlvProcessor::vOtaProcessInternalEncryption(MutableByteSpan & block)
{
    uint8_t iv[16];
    uint8_t key[kOTAEncryptionKeyLength];
    uint8_t dataOut[16] = { 0 };
    uint32_t u32IVCount;
    uint32_t Offset = 0;
    uint8_t data;
    tsReg128 sKey;
    aesContext_t Context;

    memcpy(iv, au8Iv, sizeof(au8Iv));

    u32IVCount = (((uint32_t) iv[12]) << 24) | (((uint32_t) iv[13]) << 16) | (((uint32_t) iv[14]) << 8) | (iv[15]);
    u32IVCount += (mIVOffset >> 4);

    iv[12] = (uint8_t) ((u32IVCount >> 24) & 0xff);
    iv[13] = (uint8_t) ((u32IVCount >> 16) & 0xff);
    iv[14] = (uint8_t) ((u32IVCount >> 8) & 0xff);
    iv[15] = (uint8_t) (u32IVCount & 0xff);

    if (Encoding::HexToBytes(OTA_ENCRYPTION_KEY, strlen(OTA_ENCRYPTION_KEY), key, kOTAEncryptionKeyLength) !=
        kOTAEncryptionKeyLength)
    {
        // Failed to convert the OTAEncryptionKey string to octstr type value
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    ByteSpan KEY = ByteSpan(key);
    Encoding::LittleEndian::Reader reader_key(KEY.data(), KEY.size());
    ReturnErrorOnFailure(reader_key.Read32(&sKey.u32register0)
                             .Read32(&sKey.u32register1)
                             .Read32(&sKey.u32register2)
                             .Read32(&sKey.u32register3)
                             .StatusCode());

    while (Offset + 16 <= block.size())
    {
        /*Encrypt the IV*/
        Context.mode         = AES_MODE_ECB_ENCRYPT;
        Context.pSoftwareKey = (uint32_t *) &sKey;
        AES_128_ProcessBlocks(&Context, (uint32_t *) &iv[0], (uint32_t *) &dataOut[0], 1);

        /* Decrypt a block of the buffer */
        for (uint8_t i = 0; i < 16; i++)
        {
            data = block[Offset + i] ^ dataOut[i];
            memcpy(&block[Offset + i], &data, sizeof(uint8_t));
        }

        /* increment the IV for the next block  */
        u32IVCount++;

        iv[12] = (uint8_t) ((u32IVCount >> 24) & 0xff);
        iv[13] = (uint8_t) ((u32IVCount >> 16) & 0xff);
        iv[14] = (uint8_t) ((u32IVCount >> 8) & 0xff);
        iv[15] = (uint8_t) (u32IVCount & 0xff);

        Offset += 16; /* increment the buffer offset */
        mIVOffset += 16;
    }

    return CHIP_NO_ERROR;
}
#endif
} // namespace chip
