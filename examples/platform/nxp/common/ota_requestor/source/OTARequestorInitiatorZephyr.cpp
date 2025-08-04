/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright 2023-2024 NXP
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

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>

using namespace chip;

void chip::NXP::App::OTARequestorInitiator::HandleSelfTest()
{
    if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT)
    {
        int img_confirmation = boot_write_img_confirmed();
        if (img_confirmation)
        {
            ChipLogError(SoftwareUpdate, "Self-testing : Failed to switch update state to permanent");
        }
        else
        {
            ChipLogProgress(SoftwareUpdate, "Successful software update... applied permanently");
        }
    }
}
