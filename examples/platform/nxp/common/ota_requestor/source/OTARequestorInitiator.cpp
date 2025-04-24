/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright 2023-2025 NXP
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
#include "OtaSupport.h"

extern "C" {
#include "mflash_drv.h"
}
using namespace chip;

void chip::NXP::App::OTARequestorInitiator::HandleSelfTest()
{
    /*
     * If the application running is in test state after an OTA update,
     * and since we have arrived this far, the image can be marked as valid
     * and the update state can be switched to permanent.
     */

    otaResult_t status;
    status = OTA_Initialize();

    if (status != gOtaSuccess_c)
    {
        ChipLogError(SoftwareUpdate, "Self-testing : Failed to transition the image to permanent state");
    }

    /* If the image is not marked ok, the bootloader will automatically revert back to primary application at next reboot */
}
