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

#pragma once

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <cstring>
#include <device_lock.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

#if defined(CONFIG_PLATFORM_8710C)
#include "ota_8710c.h"
#include "sys.h"
#include "sys_api.h"
#elif defined(CONFIG_PLATFORM_8721D)
#include "rtl8721d_ota.h"
#endif

namespace chip {

class AmebaOTAImageProcessor : public OTAImageProcessorInterface
{
public:
    //////////// OTAImageProcessorInterface Implementation ///////////////
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;
    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }

private:
    //////////// Actual handlers for the OTAImageProcessorInterface ///////////////
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    static void HandleApply(intptr_t context);
    static void HandleRestart(chip::System::Layer * systemLayer, void * appState);

    CHIP_ERROR ProcessHeader(ByteSpan & block);

    /**
     * Called to allocate memory for mBlock if necessary and set it to block
     */
    CHIP_ERROR SetBlock(ByteSpan & block);

    /**
     * Called to release allocated memory for mBlock
     */
    CHIP_ERROR ReleaseBlock();

#if defined(CONFIG_PLATFORM_8721D)
    uint32_t ota_target_index = OTA_INDEX_2;
    update_ota_target_hdr * pOtaTgtHdr;
    uint32_t RemainBytes;
    uint8_t * signature;
#elif defined(CONFIG_PLATFORM_8710C)
    uint32_t ota_target_index;
    uint32_t NewFWBlkSize = 0;
    uint32_t block_len    = 0;
    uint8_t signature[32];
#endif
    MutableByteSpan mBlock;
    OTADownloader * mDownloader;
    uint32_t size           = 0;
    uint8_t RemainHeader    = 32;
    uint8_t AmebaHeader[32] = { 0 };
    uint32_t flash_addr;
    OTAImageHeaderParser mHeaderParser;
    uint32_t mSoftwareVersion;
};

} // namespace chip
