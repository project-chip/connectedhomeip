/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>

#include <dfu/dfu_target.h>
#include <dfu/dfu_target_mcuboot.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return DeviceLayer::SystemLayer().ScheduleLambda([this] { mDownloader->OnPreparedForDownload(PrepareDownloadImpl()); });
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownloadImpl()
{
    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_target_mcuboot_set_buf(mBuffer, sizeof(mBuffer))));
    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_target_reset()));
    return System::MapErrorZephyr(dfu_target_init(DFU_TARGET_IMAGE_TYPE_MCUBOOT, /* size */ 0, nullptr));
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    return System::MapErrorZephyr(dfu_target_reset());
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    return System::MapErrorZephyr(dfu_target_done(true));
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // DFU target library buffers data internally, so do not clone the block data.
    CHIP_ERROR error = System::MapErrorZephyr(dfu_target_write(block.data(), block.size()));

    // Report the result back to the downloader asynchronously.
    return DeviceLayer::SystemLayer().ScheduleLambda([this, error] {
        if (error == CHIP_NO_ERROR)
            mDownloader->FetchNextData();
        else
            mDownloader->EndDownload(error);
    });
}

} // namespace DeviceLayer
} // namespace chip
