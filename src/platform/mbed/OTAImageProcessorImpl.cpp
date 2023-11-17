/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "OTAImageProcessorImpl.h"
#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef BOOT_ENABLED
#include "bootutil/bootutil.h"
#include "flash_map_backend/secondary_bd.h"
#include "platform/mbed_power_mgmt.h"
#include "rtos/ThisThread.h"
#endif

using namespace ::chip::DeviceLayer::Internal;

namespace chip {

OTAImageProcessorImpl::OTAImageProcessorImpl()
{
#ifdef BOOT_ENABLED
    // Set block device - memory for image update
    mBlockDevice = get_secondary_bd();
#endif
}

int OTAImageProcessorImpl::MemoryTest()
{
    int ret = 0;
    bd_size_t read_size;
    bd_size_t program_size;
    bd_size_t erase_size;
    bd_size_t full_size;
    size_t buffer_size;
    char * buffer = nullptr;

    if (!mBlockDevice)
    {
        ChipLogError(SoftwareUpdate, "Block device not set");
        return 1;
    }

    // Initialize the block device
    ret = mBlockDevice->init();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device initialization failed [%d]", ret);
        goto exit;
    }

    // Get the block device type
    ChipLogProgress(SoftwareUpdate, "Block device type: %s", mBlockDevice->get_type());

    // Get device geometry
    read_size    = mBlockDevice->get_read_size();
    program_size = mBlockDevice->get_program_size();
    erase_size   = mBlockDevice->get_erase_size();
    full_size    = mBlockDevice->size();

    ChipLogProgress(SoftwareUpdate, "--- Block device geometry ---");
    ChipLogProgress(SoftwareUpdate, "read_size:    %lld B", read_size);
    ChipLogProgress(SoftwareUpdate, "program_size: %lld B", program_size);
    ChipLogProgress(SoftwareUpdate, "erase_size:   %lld B", erase_size);
    ChipLogProgress(SoftwareUpdate, "size:         %lld B", full_size);
    ChipLogProgress(SoftwareUpdate, "---\n");

    // Allocate a block with enough space for our data, aligned to the
    // nearest program_size. This is the minimum size necessary to write
    // data to a block.
    buffer_size = sizeof("Hello Storage!") + program_size - 1;
    buffer_size = buffer_size - (buffer_size % program_size);
    buffer      = new char[buffer_size];

    // Read what is currently stored on the block device. We haven't written
    // yet so this may be garbage
    ret = mBlockDevice->read(buffer, 0, buffer_size);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device read failed [%d]", ret);
        goto exit;
    }

    ChipLogProgress(SoftwareUpdate, "--- Currently stored data ---");
    for (size_t i = 0; i < buffer_size; i += 16)
    {
        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < buffer_size)
            {
                ChipLogProgress(SoftwareUpdate, "%02x ", buffer[i + j]);
            }
            else
            {
                ChipLogProgress(SoftwareUpdate, "   ");
            }
        }
        ChipLogProgress(SoftwareUpdate, " ");
    }
    ChipLogProgress(SoftwareUpdate, "---\n");

    // Write data to first block, write occurs in two parts,
    // an erase followed by a program
    ret = mBlockDevice->erase(0, erase_size);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device erase failed [%d]", ret);
        goto exit;
    }

    // Clear the buffer so we don't get old data
    memset(buffer, 0x0, buffer_size);

    // Read the data from the first block
    ret = mBlockDevice->read(buffer, 0, buffer_size);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device read failed [%d]", ret);
        goto exit;
    }

    ChipLogProgress(SoftwareUpdate, "--- Stored data after erase ---");
    for (size_t i = 0; i < buffer_size; i += 16)
    {
        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < buffer_size)
            {
                ChipLogProgress(SoftwareUpdate, "%02x ", buffer[i + j]);
            }
            else
            {
                ChipLogProgress(SoftwareUpdate, "   ");
            }
            ChipLogProgress(SoftwareUpdate, " ");
        }
    }
    ChipLogProgress(SoftwareUpdate, "---\n");

    // Clear the buffer so we don't get old data
    memset(buffer, 0x0, buffer_size);
    // Update buffer with our string we want to store
    Platform::CopyString(buffer, buffer_size, "Hello Storage!");

    ret = mBlockDevice->program(buffer, 0, buffer_size);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device program failed [%d]", ret);
        goto exit;
    }

    // Clear the buffer so we don't get old data
    memset(buffer, 0x0, buffer_size);

    // Read the data from the first block, note that the program_size must be
    // a multiple of the read_size, so we don't have to check for alignment
    ret = mBlockDevice->read(buffer, 0, buffer_size);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device read failed [%d]", ret);
        goto exit;
    }

    ChipLogProgress(SoftwareUpdate, "--- Stored data after write ---");
    for (size_t i = 0; i < buffer_size; i += 16)
    {
        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < buffer_size)
            {
                ChipLogProgress(SoftwareUpdate, "%02x ", buffer[i + j]);
            }
            else
            {
                ChipLogProgress(SoftwareUpdate, "   ");
            }
        }

        ChipLogProgress(SoftwareUpdate, " %.*s", buffer_size - i, &buffer[i]);
    }
    ChipLogProgress(SoftwareUpdate, "---\n");

    ret = strcmp(buffer, "Hello Storage!");
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Data compare failed");
    }
    else
    {
        ChipLogProgress(SoftwareUpdate, "--- MEMORY TEST PASS ---");
    }

exit:
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "--- MEMORY TEST FAILED ---");
    }

    if (buffer)
    {
        delete buffer;
    }
    // Deinitialize the block device
    ret = mBlockDevice->deinit();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block deinitialization read failed [%d]", ret);
        goto exit;
    }

    return ret;
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    ChipLogProgress(SoftwareUpdate, "Prepare download");
    ClearDownloadParams();
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    ChipLogProgress(SoftwareUpdate, "Finalize");
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    ChipLogProgress(SoftwareUpdate, "Apply");
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    ChipLogProgress(SoftwareUpdate, "Abort");
    ClearDownloadParams();
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
};

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    ChipLogProgress(SoftwareUpdate, "Process block");
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
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

    auto ret = imageProcessor->PrepareMemory();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Prepare download memory failed");
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_INTERNAL);
        return;
    }

    ret = imageProcessor->ClearMemory();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Clear download memory failed");
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_INTERNAL);
        return;
    }
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    imageProcessor->mParams.totalFileBytes = imageProcessor->mParams.downloadedBytes;
    imageProcessor->ReleaseBlock();
    auto ret = imageProcessor->CloseMemory();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Close download memory failed");
    }
    ChipLogProgress(SoftwareUpdate, "OTA image downloaded size %lldB", imageProcessor->mParams.totalFileBytes);
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    auto ret = imageProcessor->ClearMemory();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Clear download memory failed");
    }

    ret = imageProcessor->CloseMemory();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Close download memory failed");
    }

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
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

    auto ret = imageProcessor->ProgramMemory();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Program download memory failed");
    }
    imageProcessor->mDownloader->FetchNextData();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

#ifdef BOOT_ENABLED
    ChipLogProgress(SoftwareUpdate, "Set secondary image pending");
    auto ret = boot_set_pending_multi(/*image index*/ 0, /*permanent*/ 0);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Setting the update candidate as pending failed: %d", ret);
    }

    // Restart the device
    ChipLogProgress(SoftwareUpdate, "Device restarting....");
    rtos::ThisThread::sleep_for(3000);
    system_reset();
#endif
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (block.empty())
    {
        ReleaseBlock();
        return CHIP_NO_ERROR;
    }
    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            ReleaseBlock();
        }

        size_t buffer_size = 0;
        if (mBlockDevice)
        {
            // Aligned to the nearest program_size. This is the minimum size necessary to write data to a block.
            bd_size_t program_size = mBlockDevice->get_program_size();
            buffer_size            = block.size() + program_size - 1;
            buffer_size            = buffer_size - (buffer_size % program_size);
        }
        else
        {
            buffer_size = block.size();
        }

        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(buffer_size));
        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, buffer_size);
    }
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
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

void OTAImageProcessorImpl::ClearDownloadParams()
{
    mParams.downloadedBytes = 0;
    mParams.totalFileBytes  = 0;
}

int OTAImageProcessorImpl::PrepareMemory()
{
    int ret = 0;

    if (!mBlockDevice)
    {
        ChipLogError(SoftwareUpdate, "Block device not set");
        return 1;
    }

    // Initialize the block device
    ret = mBlockDevice->init();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device initialization failed [%d]", ret);
        return ret;
    }
    ChipLogProgress(SoftwareUpdate, "Block device initialize");

    // Initialization read from the block device
    bd_size_t read_size = mBlockDevice->get_read_size();
    char buff[read_size];
    ret = mBlockDevice->read(&buff, 0, read_size);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device read failed [%d]", ret);
        return ret;
    }

    return ret;
}

int OTAImageProcessorImpl::CloseMemory()
{
    int ret = 0;

    if (!mBlockDevice)
    {
        ChipLogError(SoftwareUpdate, "Block device not set");
        return 1;
    }

    // Deinitialize the block device
    ret = mBlockDevice->deinit();
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device deinitialization failed [%d]", ret);
        return ret;
    }
    ChipLogProgress(SoftwareUpdate, "Block device deinitialization");

    return ret;
}

int OTAImageProcessorImpl::ClearMemory()
{
    int ret = 0;

    if (!mBlockDevice)
    {
        ChipLogError(SoftwareUpdate, "Block device not set");
        return 1;
    }

    // Erase memory of block device
    ret = mBlockDevice->erase(0, mBlockDevice->size());
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Erase block device failed [%d]", ret);
        return ret;
    }

    return ret;
}

int OTAImageProcessorImpl::ProgramMemory()
{
    int ret = 0;

    if (!mBlockDevice)
    {
        ChipLogError(SoftwareUpdate, "Block device not set");
        return 1;
    }

    // Write data to memory
    ret = mBlockDevice->program(mBlock.data(), mParams.downloadedBytes, mBlock.size());
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Block device program failed [%d]", ret);
        return ret;
    }
    ChipLogProgress(SoftwareUpdate,
                    "Secondary slot program with offset: "
                    "0x" ChipLogFormatX64,
                    ChipLogValueX64(mParams.downloadedBytes));
    mParams.downloadedBytes += mBlock.size();

    return ret;
}

} // namespace chip
