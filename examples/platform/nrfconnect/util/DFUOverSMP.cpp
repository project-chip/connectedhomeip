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

#include "DFUOverSMP.h"

#if !defined(CONFIG_MCUMGR_SMP_BT) || !defined(CONFIG_MCUMGR_CMD_IMG_MGMT) || !defined(CONFIG_MCUMGR_CMD_OS_MGMT)
#error "DFUOverSMP requires MCUMGR module configs enabled"
#endif

#include "OTAUtil.h"

#include <platform/CHIPDeviceLayer.h>

#include <lib/support/logging/CHIPLogging.h>

#include <img_mgmt/img_mgmt.h>
#include <os_mgmt/os_mgmt.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/mgmt/mcumgr/smp_bt.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

constexpr uint8_t kAdvertisingPriority     = UINT8_MAX;
constexpr uint32_t kAdvertisingOptions     = BT_LE_ADV_OPT_CONNECTABLE;
constexpr uint16_t kAdvertisingIntervalMin = 400;
constexpr uint16_t kAdvertisingIntervalMax = 500;
constexpr uint8_t kAdvertisingFlags        = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;

DFUOverSMP DFUOverSMP::sDFUOverSMP;

void DFUOverSMP::Init()
{
    const char * name = bt_get_name();

    mAdvertisingItems[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
    mAdvertisingItems[1] = BT_DATA(BT_DATA_NAME_COMPLETE, name, static_cast<uint8_t>(strlen(name)));

    mAdvertisingRequest.priority        = kAdvertisingPriority;
    mAdvertisingRequest.options         = kAdvertisingOptions;
    mAdvertisingRequest.minInterval     = kAdvertisingIntervalMin;
    mAdvertisingRequest.maxInterval     = kAdvertisingIntervalMax;
    mAdvertisingRequest.advertisingData = Span<bt_data>(mAdvertisingItems);

    mAdvertisingRequest.onStarted = [](int rc) {
        if (rc == 0)
        {
            ChipLogProgress(SoftwareUpdate, "SMP BLE advertising started");
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Failed to start SMP BLE advertising: %d", rc);
        }
    };

    os_mgmt_register_group();
    img_mgmt_register_group();

    img_mgmt_set_upload_cb([](const img_mgmt_upload_req req, const img_mgmt_upload_action action) {
        ChipLogProgress(SoftwareUpdate, "DFU over SMP progress: %u/%u B of image %u", static_cast<unsigned>(req.off),
                        static_cast<unsigned>(action.size), static_cast<unsigned>(req.image));
        return 0;
    });

    mgmt_register_evt_cb([](uint8_t opcode, uint16_t group, uint8_t id, void * arg) {
        switch (opcode)
        {
        case MGMT_EVT_OP_CMD_RECV:
            GetFlashHandler().DoAction(ExternalFlashManager::Action::WAKE_UP);
            break;
        case MGMT_EVT_OP_CMD_DONE:
            GetFlashHandler().DoAction(ExternalFlashManager::Action::SLEEP);
            break;
        default:
            break;
        }
    });
}

void DFUOverSMP::ConfirmNewImage()
{
    // Check if the image is run in the REVERT mode and eventually
    // confirm it to prevent reverting on the next boot.
    VerifyOrReturn(mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT);

    if (boot_write_img_confirmed())
    {
        ChipLogError(SoftwareUpdate, "Confirming firmware image failed, it will be reverted on the next boot");
    }
    else
    {
        ChipLogProgress(SoftwareUpdate, "New firmware image confirmed");
    }
}

void DFUOverSMP::StartServer()
{
    VerifyOrReturn(!mIsStarted, ChipLogProgress(SoftwareUpdate, "DFU over SMP was already started"));
    smp_bt_register();

    // Synchronize access to the advertising arbiter that normally runs on the CHIP thread.
    PlatformMgr().LockChipStack();
    BLEAdvertisingArbiter::InsertRequest(mAdvertisingRequest);
    PlatformMgr().UnlockChipStack();

    mIsStarted = true;
    ChipLogProgress(DeviceLayer, "DFU over SMP started");
}
