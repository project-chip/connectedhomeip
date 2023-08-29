/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTAUtil.h"

#if CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/nrfconnect/OTAImageProcessorImpl.h>
#include <zephyr/dfu/mcuboot.h>
#endif

using namespace chip;
using namespace chip::DeviceLayer;

#if CONFIG_CHIP_OTA_REQUESTOR

namespace {

DefaultOTARequestorStorage sOTARequestorStorage;
DefaultOTARequestorDriver sOTARequestorDriver;
chip::BDXDownloader sBDXDownloader;
chip::DefaultOTARequestor sOTARequestor;
} // namespace

// compile-time factory method
OTAImageProcessorImpl & GetOTAImageProcessor()
{
#if CONFIG_PM_DEVICE && CONFIG_NORDIC_QSPI_NOR
    static OTAImageProcessorImpl sOTAImageProcessor(&GetFlashHandler());
#else
    static OTAImageProcessorImpl sOTAImageProcessor;
#endif
    return sOTAImageProcessor;
}

void InitBasicOTARequestor()
{
    VerifyOrReturn(GetRequestorInstance() == nullptr);

    OTAImageProcessorImpl & imageProcessor = GetOTAImageProcessor();
    imageProcessor.SetOTADownloader(&sBDXDownloader);
    sBDXDownloader.SetImageProcessorDelegate(&imageProcessor);
    sOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    sOTARequestor.Init(Server::GetInstance(), sOTARequestorStorage, sOTARequestorDriver, sBDXDownloader);
    chip::SetRequestorInstance(&sOTARequestor);
    sOTARequestorDriver.Init(&sOTARequestor, &imageProcessor);
    imageProcessor.TriggerFlashAction(ExternalFlashManager::Action::SLEEP);
}

CHIP_ERROR OtaConfirmNewImage()
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    OTAImageProcessorImpl & imageProcessor = GetOTAImageProcessor();
    if (imageProcessor.IsFirstImageRun())
    {
        CHIP_ERROR err = System::MapErrorZephyr(boot_write_img_confirmed());
        if (CHIP_NO_ERROR == err)
        {
            imageProcessor.SetImageConfirmed();
        }
    }
    ChipLogError(SoftwareUpdate, "Failed to confirm firmware image, it will be reverted on the next boot");
    return err;
}

#endif

ExternalFlashManager & GetFlashHandler()
{
    static ExternalFlashManager sFlashHandler;
    return sFlashHandler;
}
