/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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
#include <lib/support/TypeTraits.h>

#include <headers/ProvisionManager.h>
#include <headers/ProvisionStorage.h>
#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>
#include <platform/silabs/multi-ota/OTATlvProcessor.h>
#if SL_MATTER_ENABLE_OTA_ENCRYPTION
#include <platform/silabs/multi-ota/OtaTlvEncryptionKey.h>
#endif

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::Silabs;

namespace chip {

CHIP_ERROR OTATlvProcessor::Init()
{
    VerifyOrReturnError(mCallbackProcessDescriptor != nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(GetAccumulatorLength());
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
    mUnalignmentNum = 0;
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTATlvProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
    mUnalignmentNum = 0;
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTATlvProcessor::Process(ByteSpan & block)
{
    CHIP_ERROR status     = CHIP_NO_ERROR;
    uint32_t bytes        = std::min(mLength - mProcessedLength, static_cast<uint32_t>(block.size()));
    ByteSpan relevantData = block.SubSpan(0, bytes);
    if (mProcessedLength + bytes >= mLength)
    {
        mLastBlock = true;
    }
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
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
    mIVOffset  = 0;
    mLastBlock = false;
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION
}

bool OTATlvProcessor::IsError(CHIP_ERROR & status)
{
    return status != CHIP_NO_ERROR && status != CHIP_ERROR_BUFFER_TOO_SMALL && status != CHIP_OTA_FETCH_ALREADY_SCHEDULED;
}

bool OTATlvProcessor::IsValidTag(OTAProcessorTag tag)
{
    auto value = chip::to_underlying(tag);

    return value >= chip::to_underlying(OTAProcessorTag::kApplicationProcessor) &&
        value <= chip::to_underlying(OTAProcessorTag::kMaxValue);
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
    uint32_t numBytes = std::min(mThreshold - mBufferOffset, static_cast<uint32_t>(block.size()));
    memcpy(&mBuffer[mBufferOffset], block.data(), numBytes);
    mBufferOffset += numBytes;
    block = block.SubSpan(numBytes);

    if (mBufferOffset < mThreshold)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return CHIP_NO_ERROR;
}

#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
CHIP_ERROR OTATlvProcessor::vOtaProcessInternalEncryption(MutableByteSpan & block)
{
#if defined(SL_MBEDTLS_USE_TINYCRYPT)
    Provision::Manager::GetInstance().GetStorage().DecryptUsingOtaTlvEncryptionKey(block, mIVOffset);
#else  // MBEDTLS_USE_PSA_CRYPTO
    uint32_t keyId;
    Provision::Manager::GetInstance().GetStorage().GetOtaTlvEncryptionKeyId(keyId);
    chip::DeviceLayer::Silabs::OtaTlvEncryptionKey key(keyId);

    key.Decrypt(block, mIVOffset);
#endif // SL_MBEDTLS_USE_TINYCRYPT

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTATlvProcessor::RemovePadding(MutableByteSpan & block)
{
    if (block.size() == 0)
    {
        ChipLogError(DeviceLayer, "Block size is zero, cannot unpad");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint8_t padLength = block.data()[block.size() - 1];
    if (padLength == 0 || padLength > block.size())
    {
        ChipLogError(DeviceLayer, "Invalid PKCS7 padding");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Verify padding bytes
    uint8_t mismatch = 0;
    for (size_t i = 0; i < padLength; ++i)
    {
        if (block.data()[block.size() - 1 - i] != padLength)
        {
            // not returning early here, we want to check all padding bytes
            // preventing timing side-channel attacks
            mismatch = 1;
        }
    }

    if (mismatch != 0)
    {
        ChipLogError(DeviceLayer, "PKCS7 padding verification failed");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    block.reduce_size(block.size() - padLength);
    return CHIP_NO_ERROR;
}
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION
} // namespace chip
