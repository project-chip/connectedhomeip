/*
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/telink/OTAImageProcessorImpl.h>
#endif

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif

using namespace chip;
using namespace chip::DeviceLayer;

#ifdef CONFIG_CHIP_OTA_REQUESTOR

namespace {

DefaultOTARequestorStorage sOTARequestorStorage;
DefaultOTARequestorDriver sOTARequestorDriver;
chip::BDXDownloader sBDXDownloader;
chip::DefaultOTARequestor sOTARequestor;
chip::DeviceLayer::OTAImageProcessorImpl sOTAImageProcessor;
} // namespace

void InitBasicOTARequestor()
{
    VerifyOrReturn(GetRequestorInstance() == nullptr);

    sOTAImageProcessor.SetOTADownloader(&sBDXDownloader);
    sBDXDownloader.SetImageProcessorDelegate(&sOTAImageProcessor);
    sOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    sOTARequestor.Init(Server::GetInstance(), sOTARequestorStorage, sOTARequestorDriver, sBDXDownloader);
    chip::SetRequestorInstance(&sOTARequestor);
    sOTARequestorDriver.Init(&sOTARequestor, &sOTAImageProcessor);
}

#endif

void OtaConfirmNewImage()
{
    if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT)
    {
        CHIP_ERROR err = System::MapErrorZephyr(boot_write_img_confirmed());
        if (CHIP_NO_ERROR == err)
        {
            ChipLogProgress(SoftwareUpdate, "New firmware image confirmed");
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Failed to confirm firmware image, it will be reverted on the next boot");
        }
    }
}
