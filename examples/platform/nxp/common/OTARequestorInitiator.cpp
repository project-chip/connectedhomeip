/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright 2023 NXP
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

#include "OTARequestorInitiator.h"

extern "C" {
#include "mflash_drv.h"
}

using namespace chip;

void OTARequestorInitiator::InitOTA(intptr_t context)
{
    auto * otaRequestorInit = reinterpret_cast<OTARequestorInitiator *>(context);
    // Set the global instance of the OTA requestor core component
    SetRequestorInstance(&otaRequestorInit->gRequestorCore);

    otaRequestorInit->gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    otaRequestorInit->gRequestorCore.Init(chip::Server::GetInstance(), otaRequestorInit->gRequestorStorage,
                                          otaRequestorInit->gRequestorUser, otaRequestorInit->gDownloader);
    otaRequestorInit->gRequestorUser.Init(&otaRequestorInit->gRequestorCore, &otaRequestorInit->gImageProcessor);
    otaRequestorInit->gImageProcessor.SetOTADownloader(&otaRequestorInit->gDownloader);

    // Set the image processor instance used for handling image being downloaded
    otaRequestorInit->gDownloader.SetImageProcessorDelegate(&otaRequestorInit->gImageProcessor);
}

void OTARequestorInitiator::HandleSelfTest()
{
    /* If application is in test mode after an OTA update
       mark image as "ok" to switch the update state to permanent
       (if we have arrived this far, the bootloader had validated the image) */

    mflash_drv_init();

    OtaImgState_t update_state;

    /* Retrieve current update state */
    update_state = OTA_GetImgState();

    if (update_state == OtaImgState_RunCandidate)
    {
        if (OTA_UpdateImgState(OtaImgState_Permanent) != gOtaSuccess_c)
        {
            ChipLogError(SoftwareUpdate, "Self-testing : Failed to switch update state to permanent");
            return;
        }

        ChipLogProgress(SoftwareUpdate, "Successful software update... applied permanently");
    }

    OTA_Initialize();

    /* If the image is not marked ok, the bootloader will automatically revert back to primary application at next reboot */
}
