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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/BufferReader.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>

#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>

using namespace chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

static chip::OTAMultiImageProcessorImpl gImageProcessor;

#if SL_WIFI
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#endif // SL_WIFI

extern "C" {
#include "btl_interface.h"
#include "sl_core.h"
}

namespace chip {

CHIP_ERROR OTAMultiImageProcessorImpl::Init(OTADownloader * downloader)
{
    VerifyOrReturnError(downloader != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    gImageProcessor.SetOTADownloader(downloader);

    OtaHookInit();

    return CHIP_NO_ERROR;
}

void OTAMultiImageProcessorImpl::Clear()
{
    mHeaderParser.Clear();
    mAccumulator.Clear();
    mParams.totalFileBytes  = 0;
    mParams.downloadedBytes = 0;
    mCurrentProcessor       = nullptr;

    ReleaseBlock();
}

CHIP_ERROR OTAMultiImageProcessorImpl::PrepareDownload()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::Apply()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::Abort()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::ProcessBlock(ByteSpan & block)
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

void OTAMultiImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAMultiImageProcessorImpl *>(context);

    VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));

    VerifyOrReturn(imageProcessor->mDownloader != nullptr, ChipLogError(SoftwareUpdate, "mDownloader is null"));

    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload: started");

    CORE_CRITICAL_SECTION(bootloader_init();)

    imageProcessor->mParams.downloadedBytes = 0;

    imageProcessor->mHeaderParser.Init();
    imageProcessor->mAccumulator.Init(sizeof(OTATlvHeader));
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

CHIP_ERROR OTAMultiImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    OTAImageHeader header;
    ReturnErrorOnFailure(mHeaderParser.AccumulateAndDecode(block, header));

    mParams.totalFileBytes = header.mPayloadSize;
    mHeaderParser.Clear();
    ChipLogError(SoftwareUpdate, "Processed header successfully");

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::ProcessPayload(ByteSpan & block)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    while (true)
    {
        if (!mCurrentProcessor)
        {
            ReturnErrorOnFailure(mAccumulator.Accumulate(block));
            ByteSpan tlvHeader{ mAccumulator.data(), sizeof(OTATlvHeader) };
            ReturnErrorOnFailure(SelectProcessor(tlvHeader));
            ReturnErrorOnFailure(mCurrentProcessor->Init());
        }

        status = mCurrentProcessor->Process(block);
        if (status == CHIP_OTA_CHANGE_PROCESSOR)
        {
            mAccumulator.Clear();
            mAccumulator.Init(sizeof(OTATlvHeader));

            mCurrentProcessor = nullptr;

            // If the block size is 0, it means that the processed data was a multiple of
            // received BDX block size (e.g. 8 blocks of 1024 bytes were transferred).
            // After state for selecting next processor is reset, a request for fetching next
            // data must be sent.
            if (block.size() == 0)
            {
                status = CHIP_NO_ERROR;
                break;
            }
        }
        else
        {
            break;
        }
    }

    return status;
}

CHIP_ERROR OTAMultiImageProcessorImpl::SelectProcessor(ByteSpan & block)
{
    OTATlvHeader header;
    Encoding::LittleEndian::Reader reader(block.data(), sizeof(header));

    ReturnErrorOnFailure(reader.Read32(&header.tag).StatusCode());
    ReturnErrorOnFailure(reader.Read32(&header.length).StatusCode());

    auto pair = mProcessorMap.find(header.tag);
    if (pair == mProcessorMap.end())
    {
        ChipLogError(SoftwareUpdate, "There is no registered processor for tag: %lu", header.tag);
        return CHIP_OTA_PROCESSOR_NOT_REGISTERED;
    }

    ChipLogDetail(SoftwareUpdate, "Selected processor with tag: %lu", pair->first);
    mCurrentProcessor = pair->second;
    mCurrentProcessor->SetLength(header.length);
    mCurrentProcessor->SetWasSelected(true);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::RegisterProcessor(uint32_t tag, OTATlvProcessor * processor)
{
    auto pair = mProcessorMap.find(tag);
    if (pair != mProcessorMap.end())
    {
        ChipLogError(SoftwareUpdate, "A processor for tag %lu is already registered.", tag);
        return CHIP_OTA_PROCESSOR_ALREADY_REGISTERED;
    }

    mProcessorMap.insert({ tag, processor });

    return CHIP_NO_ERROR;
}

void OTAMultiImageProcessorImpl::HandleAbort(intptr_t context)
{
    ChipLogError(SoftwareUpdate, "OTA was aborted");
    auto * imageProcessor = reinterpret_cast<OTAMultiImageProcessorImpl *>(context);
    if (imageProcessor != nullptr)
    {
        imageProcessor->AbortAllProcessors();
    }
    imageProcessor->Clear();
}

void OTAMultiImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAMultiImageProcessorImpl *>(context);

    VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));

    VerifyOrReturn(imageProcessor->mDownloader != nullptr, ChipLogError(SoftwareUpdate, "mDownloader is null"));

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

void OTAMultiImageProcessorImpl::HandleStatus(CHIP_ERROR status)
{
    if (status == CHIP_NO_ERROR || status == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        mParams.downloadedBytes += mBlock.size();
        FetchNextData(0);
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

void OTAMultiImageProcessorImpl::AbortAllProcessors()
{
    ChipLogError(SoftwareUpdate, "All selected processors will call abort action");

    for (auto const & pair : mProcessorMap)
    {
        if (pair.second->WasSelected())
        {
            pair.second->Clear();
            pair.second->SetWasSelected(false);
        }
    }
}

bool OTAMultiImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR OTAMultiImageProcessorImpl::ConfirmCurrentImage()
{
    uint32_t currentVersion;
    uint32_t targetVersion;

    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    VerifyOrReturnError(requestor != nullptr, CHIP_ERROR_INTERNAL);

    targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != targetVersion)
    {
        ChipLogError(SoftwareUpdate, "Current sw version %lu is different than the expected sw version = %lu", currentVersion,
                     targetVersion);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAMultiImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (block.empty())
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

void OTAMultiImageProcessorImpl::HandleFinalize(intptr_t context)
{
    ChipLogError(SoftwareUpdate, "HandleFinalize begin");
    CHIP_ERROR error      = CHIP_NO_ERROR;
    auto * imageProcessor = reinterpret_cast<OTAMultiImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    error = imageProcessor->ProcessFinalize();

    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();

    imageProcessor->ReleaseBlock();

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "ProcessFinalize() error");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully");
}

CHIP_ERROR OTAMultiImageProcessorImpl::ProcessFinalize()
{
    for (auto const & pair : this->mProcessorMap)
    {
        pair.second->FinalizeAction();
    }
    return CHIP_NO_ERROR;
}

void OTAMultiImageProcessorImpl::HandleApply(intptr_t context)
{
    CHIP_ERROR error      = CHIP_NO_ERROR;
    auto * imageProcessor = reinterpret_cast<OTAMultiImageProcessorImpl *>(context);

    ChipLogProgress(SoftwareUpdate, "HandleApply: started");

    // Force KVS to store pending keys such as data from StoreCurrentUpdateInfo()
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().ForceKeyMapSave();

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
                // Revert all previously applied actions if current apply action fails.
                // Reset image processor and requestor states.
                imageProcessor->AbortAllProcessors();
                imageProcessor->Clear();
                GetRequestorInstance()->Reset();

                return;
            }
        }
    }

    for (auto const & pair : imageProcessor->mProcessorMap)
    {
        pair.second->Clear();
        pair.second->SetWasSelected(false);
    }

    imageProcessor->mAccumulator.Clear();

    ChipLogProgress(SoftwareUpdate, "HandleApply: Finished");

    // This reboots the device
    CORE_CRITICAL_SECTION(bootloader_rebootAndInstall();)
}

CHIP_ERROR OTAMultiImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

void OTAMultiImageProcessorImpl::FetchNextData(uint32_t context)
{
    auto * imageProcessor = &OTAMultiImageProcessorImpl::GetDefaultInstance();
    SystemLayer().ScheduleLambda([imageProcessor] {
        if (imageProcessor->mDownloader)
        {
            imageProcessor->mDownloader->FetchNextData();
        }
    });
}

OTAMultiImageProcessorImpl & OTAMultiImageProcessorImpl::GetDefaultInstance()
{
    return gImageProcessor;
}

} // namespace chip
