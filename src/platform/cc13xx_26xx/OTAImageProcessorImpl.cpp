/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/DefaultStorageKeyAllocator.h>

#include "OTAImageProcessorImpl.h"

#include <ti/common/cc26xx/flash_interface/flash_interface.h>
#include <ti/common/cc26xx/oad/ext_flash_layout.h>

#include <ti_drivers_config.h>

// clang-format off
/* driverlib header for resetting the SoC */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
// clang-format on

using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::PersistedStorage;

namespace chip {

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if (nullptr == mNvsHandle)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if ((nullptr == block.data()) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor;
    uint32_t runningSwVer;

    if (CHIP_NO_ERROR != ConfigurationMgr().GetSoftwareVersion(runningSwVer))
    {
        return false;
    }

    requestor = GetRequestorInstance();

    return (requestor->GetTargetVersion() == runningSwVer) &&
        (requestor->GetCurrentUpdateState() == chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kApplying);
}

/* DESIGN NOTE: The Boot Image Manager will search external flash for an
 * `ExtImageInfo_t` structure every 4K for 1M. This structure points to where
 * the executable image is in external flash with a uint32_t. It is possible to
 * have multiple images ready to be programmed into the internal flash of the
 * device. This design is only concerned with managing 1 image in external
 * flash starting at `IMG_START` and being defined by a meta header at address
 * 0. Future designs may be able to take advantage of other images for rollback
 * functionality, however this will require a larger external flash chip.
 */

#define IMG_START (4 * EFL_SIZE_META)

static bool readExtFlashImgHeader(NVS_Handle handle, imgFixedHdr_t * header, size_t otaHeaderLen)
{
    int_fast16_t status;
    status = NVS_read(handle, IMG_START + otaHeaderLen, header, sizeof(imgFixedHdr_t));
    return (status == NVS_STATUS_SUCCESS);
}

/* makes room for the new block if needed */
static bool writeExtFlashImgPages(NVS_Handle handle, size_t bytesWritten, MutableByteSpan block)
{
    int_fast16_t status;
    NVS_Attrs regionAttrs;
    unsigned int erasedSectors;
    unsigned int neededSectors;
    size_t sectorSize;

    NVS_getAttrs(handle, &regionAttrs);
    sectorSize    = regionAttrs.sectorSize;
    erasedSectors = (bytesWritten + (sectorSize - 1)) / sectorSize;
    neededSectors = ((bytesWritten + block.size()) + (sectorSize - 1)) / sectorSize;
    if (neededSectors != erasedSectors)
    {
        status = NVS_erase(handle, IMG_START + (erasedSectors * sectorSize), (neededSectors - erasedSectors) * sectorSize);
        if (status != NVS_STATUS_SUCCESS)
        {
            ChipLogError(SoftwareUpdate, "NVS_erase failed status: %d", status);
            return false;
        }
    }
    status = NVS_write(handle, IMG_START + bytesWritten, block.data(), block.size(), NVS_WRITE_POST_VERIFY);
    if (status != NVS_STATUS_SUCCESS)
    {
        ChipLogError(SoftwareUpdate, "NVS_write failed status: %d", status);
        return false;
    }
    return true;
}

static bool readExtFlashMetaHeader(NVS_Handle handle, ExtImageInfo_t * header)
{
    int_fast16_t status;
    status = NVS_read(handle, EFL_ADDR_META, header, sizeof(ExtImageInfo_t));
    return (status == NVS_STATUS_SUCCESS);
}

static bool eraseExtFlashMetaHeader(NVS_Handle handle)
{
    int_fast16_t status;
    NVS_Attrs regionAttrs;
    unsigned int sectors;

    NVS_getAttrs(handle, &regionAttrs);
    /* calculate the number of sectors to erase */
    sectors = (sizeof(ExtImageInfo_t) + (regionAttrs.sectorSize - 1)) / regionAttrs.sectorSize;
    status  = NVS_erase(handle, EFL_ADDR_META, sectors * regionAttrs.sectorSize);

    return (status == NVS_STATUS_SUCCESS);
}

static bool writeExtFlashMetaHeader(NVS_Handle handle, ExtImageInfo_t * header)
{
    int_fast16_t status;
    if (!eraseExtFlashMetaHeader(handle))
    {
        return false;
    }
    status = NVS_write(handle, EFL_ADDR_META, header, sizeof(ExtImageInfo_t), NVS_WRITE_POST_VERIFY);
    return (status == NVS_STATUS_SUCCESS);
}

/**
 * Generated on by pycrc v0.9.2, https://pycrc.org using the configuration:
 *  - Width         = 32
 *  - Poly          = 0x04c11db7
 *  - XorIn         = 0xffffffff
 *  - ReflectIn     = True
 *  - XorOut        = 0xffffffff
 *  - ReflectOut    = True
 *  - Algorithm     = bit-by-bit-fast
 *
 * Modified to take uint32_t as the CRC type
 */
uint32_t crc_reflect(uint32_t data, size_t data_len)
{
    unsigned int i;
    uint32_t ret;

    ret = data & 0x01;
    for (i = 1; i < data_len; i++)
    {
        data >>= 1;
        ret = (ret << 1) | (data & 0x01);
    }
    return ret;
}

uint32_t crc_update(uint32_t crc, const void * data, size_t data_len)
{
    const unsigned char * d = (const unsigned char *) data;
    unsigned int i;
    bool bit;
    unsigned char c;

    while (data_len--)
    {
        c = *d++;
        for (i = 0x01; i & 0xff; i <<= 1)
        {
            bit = crc & 0x80000000;
            if (c & i)
            {
                bit = !bit;
            }
            crc <<= 1;
            if (bit)
            {
                crc ^= 0x04c11db7;
            }
        }
        crc &= 0xffffffff;
    }
    return crc & 0xffffffff;
}

static bool validateExtFlashImage(NVS_Handle handle, size_t otaHeaderLen)
{
    uint32_t crc;
    imgFixedHdr_t header;
    size_t addr, endAddr;

    if (!readExtFlashImgHeader(handle, &header, otaHeaderLen))
    {
        return false;
    }

    /* CRC is calculated after the CRC element of the image header */
    addr    = IMG_START + otaHeaderLen + IMG_DATA_OFFSET;
    endAddr = IMG_START + otaHeaderLen + header.len;

    crc = 0xFFFFFFFF;

    while (addr < endAddr)
    {
        uint8_t buffer[32];
        size_t bytesLeft = endAddr - addr;
        size_t toRead    = (sizeof(buffer) < bytesLeft) ? sizeof(buffer) : bytesLeft;

        if (NVS_STATUS_SUCCESS != NVS_read(handle, addr, buffer, toRead))
        {
            return false;
        }

        crc = crc_update(crc, buffer, toRead);

        addr += toRead;
    }

    crc = crc_reflect(crc, 32) ^ 0xffffffff;

    return (crc == header.crc32);
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    NVS_Handle handle;
    NVS_Params nvsParams;
    NVS_Params_init(&nvsParams);
    handle = NVS_open(CONFIG_NVSEXTERNAL, &nvsParams);

    if (NULL != handle)
    {
        eraseExtFlashMetaHeader(handle);
        NVS_close(handle);
        return CHIP_NO_ERROR;
    }
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

    if (NULL == imageProcessor->mNvsHandle)
    {
        NVS_Params nvsParams;
        NVS_Params_init(&nvsParams);
        imageProcessor->mNvsHandle = NVS_open(CONFIG_NVSEXTERNAL, &nvsParams);

        if (NULL == imageProcessor->mNvsHandle)
        {
            imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_OPEN_FAILED);
            return;
        }
    }

    if (!eraseExtFlashMetaHeader(imageProcessor->mNvsHandle))
    {
        NVS_close(imageProcessor->mNvsHandle);
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    ExtImageInfo_t header;
    auto * imageProcessor    = reinterpret_cast<OTAImageProcessorImpl *>(context);
    const uint8_t extImgId[] = OAD_EFL_MAGIC;

    if (imageProcessor == nullptr)
    {
        return;
    }

    if (!readExtFlashImgHeader(imageProcessor->mNvsHandle, &(header.fixedHdr),
                               imageProcessor->mFixedOtaHeader.headerSize + sizeof(imageProcessor->mFixedOtaHeader)))
    {
        return;
    }
    // offset in the size of the fixed and variable OTA image headers
    memcpy(&(header.fixedHdr.imgID), extImgId, sizeof(header.fixedHdr.imgID));
    header.extFlAddr = IMG_START + imageProcessor->mFixedOtaHeader.headerSize + sizeof(imageProcessor->mFixedOtaHeader);
    header.counter   = 0x0;

    if (validateExtFlashImage(imageProcessor->mNvsHandle,
                              imageProcessor->mFixedOtaHeader.headerSize + sizeof(imageProcessor->mFixedOtaHeader)))
    {
        // only write the meta header if the crc check passes
        writeExtFlashMetaHeader(imageProcessor->mNvsHandle, &header);
    }
    else
    {
        // ensure the external image is not mistaken for a valid image
        eraseExtFlashMetaHeader(imageProcessor->mNvsHandle);
    }

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded");
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    ExtImageInfo_t header;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    if (!readExtFlashMetaHeader(imageProcessor->mNvsHandle, &header))
    {
        return;
    }
    header.fixedHdr.imgCpStat = NEED_COPY;

    writeExtFlashMetaHeader(imageProcessor->mNvsHandle, &header);

    // reset SoC to kick BIM
    SysCtrlSystemReset();
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    if (!eraseExtFlashMetaHeader(imageProcessor->mNvsHandle))
    {
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_WRITE_FAILED);
    }

    NVS_close(imageProcessor->mNvsHandle);
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

    /* Save the fixed size header */
    if (imageProcessor->mParams.downloadedBytes < sizeof(imageProcessor->mFixedOtaHeader))
    {
        uint8_t * header = reinterpret_cast<uint8_t *>(&(imageProcessor->mFixedOtaHeader));
        if (imageProcessor->mBlock.size() + imageProcessor->mParams.downloadedBytes < sizeof(imageProcessor->mFixedOtaHeader))
        {
            // the first block is smaller than the header, somehow
            memcpy(header + imageProcessor->mParams.downloadedBytes, imageProcessor->mBlock.data(), imageProcessor->mBlock.size());
        }
        else
        {
            // we have received the whole header, fill it up
            memcpy(header + imageProcessor->mParams.downloadedBytes, imageProcessor->mBlock.data(),
                   sizeof(imageProcessor->mFixedOtaHeader) - imageProcessor->mParams.downloadedBytes);

            // update the total size for download tracking
            imageProcessor->mParams.totalFileBytes = imageProcessor->mFixedOtaHeader.totalSize;
        }
    }

    /* chip::OTAImageHeaderParser can be used for processing the variable size header */

    ChipLogDetail(SoftwareUpdate, "Write block %d, %d", (size_t) imageProcessor->mParams.downloadedBytes,
                  imageProcessor->mBlock.size());
    if (!writeExtFlashImgPages(imageProcessor->mNvsHandle, imageProcessor->mParams.downloadedBytes, imageProcessor->mBlock))
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
    imageProcessor->mDownloader->FetchNextData();
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

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

} // namespace chip
