/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <lib/support/BufferReader.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <src/app/clusters/ota-requestor/OTADownloader.h>
#include <src/app/clusters/ota-requestor/OTARequestorInterface.h>

#include <platform/nxp/k32w/common/OTAImageProcessorImpl.h>

using namespace chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {

CHIP_ERROR OTAImageProcessorImpl::Init(OTADownloader * downloader)
{
    ReturnErrorCodeIf(downloader == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mDownloader = downloader;

    OtaHookInit();

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::Clear()
{
    mHeaderParser.Clear();
    mAccumulator.Clear();
    mParams.totalFileBytes  = 0;
    mParams.downloadedBytes = 0;
    mCurrentProcessor       = nullptr;

    ReleaseBlock();
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    imageProcessor->mHeaderParser.Init();
    imageProcessor->mAccumulator.Init(sizeof(OTATlvHeader));
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    OTAImageHeader header;
    ReturnErrorOnFailure(mHeaderParser.AccumulateAndDecode(block, header));

    mParams.totalFileBytes = header.mPayloadSize;
    mHeaderParser.Clear();
    ChipLogError(SoftwareUpdate, "Processed header successfully");

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessPayload(ByteSpan & block)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    while (true)
    {
        if (!mCurrentProcessor)
        {
            ReturnErrorOnFailure(mAccumulator.Accumulate(block));
            ByteSpan tlvHeader{ mAccumulator.data(), sizeof(OTATlvHeader) };
            ReturnErrorOnFailure(SelectProcessor(tlvHeader));
            mCurrentProcessor->Init();
        }

        status = mCurrentProcessor->Process(block);
        if (status == CHIP_OTA_CHANGE_PROCESSOR)
        {
            mAccumulator.Clear();
            mAccumulator.Init(sizeof(OTATlvHeader));
            mCurrentProcessor = nullptr;
        }
        else
        {
            break;
        }
    }

    return status;
}

CHIP_ERROR OTAImageProcessorImpl::SelectProcessor(ByteSpan & block)
{
    OTATlvHeader header;
    Encoding::LittleEndian::Reader reader(block.data(), sizeof(header));

    ReturnErrorOnFailure(reader.Read32(&header.tag).StatusCode());
    ReturnErrorOnFailure(reader.Read32(&header.length).StatusCode());

    auto pair = mProcessorMap.find(header.tag);
    if (pair == mProcessorMap.end())
    {
        ChipLogError(SoftwareUpdate, "There is no registered processor for tag: %" PRIu32, header.tag);
        return CHIP_OTA_PROCESSOR_NOT_REGISTERED;
    }

    mCurrentProcessor = pair->second;
    mCurrentProcessor->SetLength(header.length);
    mCurrentProcessor->SetWasSelected(true);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::RegisterProcessor(uint32_t tag, OTATlvProcessor * processor)
{
    auto pair = mProcessorMap.find(tag);
    if (pair != mProcessorMap.end())
    {
        ChipLogError(SoftwareUpdate, "A processor for tag %" PRIu32 " is already registered.", tag);
        return CHIP_OTA_PROCESSOR_ALREADY_REGISTERED;
    }

    mProcessorMap.insert({ tag, processor });

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    ChipLogError(SoftwareUpdate, "OTA was aborted");
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor != nullptr)
    {
        for (auto const & pair : imageProcessor->mProcessorMap)
        {
            if (pair.second->WasSelected())
            {
                pair.second->AbortAction();
                pair.second->Clear();
            }
        }
    }
    imageProcessor->Clear();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    CHIP_ERROR status;
    auto block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

    if (imageProcessor->mHeaderParser.IsInitialized())
    {
        status = imageProcessor->ProcessHeader(block);
        if (status != CHIP_NO_ERROR)
        {
            imageProcessor->HandleStatus(status);
        }
    }

    status = imageProcessor->ProcessPayload(block);
    imageProcessor->HandleStatus(status);
}

void OTAImageProcessorImpl::HandleStatus(CHIP_ERROR status)
{
    if (status == CHIP_NO_ERROR || status == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        mParams.downloadedBytes += mBlock.size();
        FetchNextData(reinterpret_cast<uint32_t>(this));
    }
    else if (status == CHIP_OTA_FETCH_ALREADY_SCHEDULED)
    {
        mParams.downloadedBytes += mBlock.size();
    }
    else
    {
        ChipLogError(SoftwareUpdate, "Image update canceled. Failed to process OTA block: %s", ErrorStr(status));
        GetRequestorInstance()->CancelImageUpdate();
    }
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    uint32_t currentVersion;
    uint32_t targetVersion;

    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    ReturnErrorCodeIf(requestor == nullptr, CHIP_ERROR_INTERNAL);

    targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != targetVersion)
    {
        ChipLogError(SoftwareUpdate, "Current sw version %" PRIu32 " is different than the expected sw version = %" PRIu32,
                     currentVersion, targetVersion);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (!IsSpanUsable(block))
    {
        return CHIP_NO_ERROR;
    }

    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            ReleaseBlock();
        }
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));
        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }

    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    CHIP_ERROR error      = CHIP_NO_ERROR;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    for (auto const & pair : imageProcessor->mProcessorMap)
    {
        if (pair.second->WasSelected())
        {
            error = pair.second->ApplyAction();
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Apply action for tag %d processor failed.", (uint8_t) pair.first);
                imageProcessor->Clear();
                GetRequestorInstance()->Reset();
                return;
            }
            pair.second->Clear();
            pair.second->SetWasSelected(false);
        }
    }

    imageProcessor->mAccumulator.Clear();

    // Set the necessary information to inform the SSBL that a new image is available
    // and trigger the actual device reboot after some time, to take into account
    // queued actions, e.g. sending events to a subscription
    SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(CHIP_DEVICE_LAYER_OTA_REBOOT_DELAY),
        [](chip::System::Layer *, void *) { OtaHookReset(); }, nullptr);
}

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::FetchNextData(uint32_t context)
{
    CHIP_ERROR error      = CHIP_NO_ERROR;
    auto * imageProcessor = &OTAImageProcessorImpl::GetDefaultInstance();
    SystemLayer().ScheduleLambda([imageProcessor] {
        if (imageProcessor->mDownloader)
        {
            imageProcessor->mDownloader->FetchNextData();
        }
    });
}

OTAImageProcessorImpl & OTAImageProcessorImpl::GetDefaultInstance()
{
    static OTAImageProcessorImpl imageProcessor;
    return imageProcessor;
}

} // namespace chip
