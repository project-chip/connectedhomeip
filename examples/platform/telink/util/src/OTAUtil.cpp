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

using namespace chip;
using namespace chip::DeviceLayer;
#endif

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

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
        int img_confirmation = boot_write_img_confirmed();
        if (img_confirmation)
        {
            LOG_ERR("Image not confirmed %d. Will be reverted!", img_confirmation);
        }
        else
        {
            LOG_INF("Image confirmed");
        }
    }
}
