/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "blecent.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ChipDeviceScanner.h>

#define CHIPoBLE_SERVICE_UUID 0xFFF6

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace {

/// Retrieve CHIP device identification info from the device advertising data
bool NimbleGetChipDeviceInfo(const ble_hs_adv_fields & fields, chip::Ble::ChipBLEDeviceIdentificationInfo & deviceInfo)
{
    // Check for CHIP Service UUID

    if (fields.svc_data_uuid16 != NULL)
    {
        if (fields.svc_data_uuid16_len > 8 && fields.svc_data_uuid16[0] == 0xf6 && fields.svc_data_uuid16[1] == 0xff)
        {
            deviceInfo.OpCode                              = fields.svc_data_uuid16[2];
            deviceInfo.DeviceDiscriminatorAndAdvVersion[0] = fields.svc_data_uuid16[3];
            deviceInfo.DeviceDiscriminatorAndAdvVersion[1] = fields.svc_data_uuid16[4];
            // vendor and product Id from adv
            deviceInfo.DeviceVendorId[0]  = fields.svc_data_uuid16[5];
            deviceInfo.DeviceVendorId[1]  = fields.svc_data_uuid16[6];
            deviceInfo.DeviceProductId[0] = fields.svc_data_uuid16[7];
            deviceInfo.DeviceProductId[1] = fields.svc_data_uuid16[8];
            deviceInfo.AdditionalDataFlag = fields.svc_data_uuid16[9];
            return true;
        }
    }
    return false;
}

} // namespace

void ChipDeviceScanner::ReportDevice(const struct ble_hs_adv_fields & fields, const ble_addr_t & addr)
{
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;
    if (NimbleGetChipDeviceInfo(fields, deviceInfo) == false)
    {
        ChipLogDetail(Ble, "Device %s does not look like a CHIP device", addr_str(addr.val));
        return;
    }
    mDelegate->OnDeviceScanned(fields, addr, deviceInfo);
}

void ChipDeviceScanner::RemoveDevice()
{
    // TODO
}

int ChipDeviceScanner::OnBleCentralEvent(struct ble_gap_event * event, void * arg)
{
    ChipDeviceScanner * scanner = (ChipDeviceScanner *) arg;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC_COMPLETE: {
        scanner->mIsScanning = false;
        return 0;
    }

    case BLE_GAP_EVENT_DISC: {

        /* Try to connect to the advertiser if it looks interesting. */
        struct ble_hs_adv_fields fields;
        ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
        scanner->ReportDevice(fields, event->disc.addr);
        return 0;
    }
    }

    return 0;
}

CHIP_ERROR ChipDeviceScanner::StartScan(uint16_t timeout)
{
    ReturnErrorCodeIf(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    uint8_t ownAddrType;
    struct ble_gap_disc_params discParams;
    int rc;

    /* Figure out address to use while advertising. */
    rc = ble_hs_id_infer_auto(0, &ownAddrType);
    if (rc != 0)
    {
        ChipLogError(DeviceLayer, "ble_hs_id_infer_auto failed: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }

    /* Set up discovery parameters. */
    memset(&discParams, 0, sizeof(discParams));

    /* Tell the controller to filter the duplicates. */
    discParams.filter_duplicates = 1;
    /* Perform passive scanning. */
    discParams.passive = 1;
    /* Use defaults for the rest of the parameters. */
    discParams.itvl          = 0;
    discParams.window        = 0;
    discParams.filter_policy = 0;
    discParams.limited       = 0;

    /* Start the discovery process. */
    rc = ble_gap_disc(ownAddrType, (timeout * 1000), &discParams, OnBleCentralEvent, this);
    if (rc != 0)
    {
        ChipLogError(DeviceLayer, "ble_gap_disc failed: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    mIsScanning = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    ReturnErrorCodeIf(!mIsScanning, CHIP_NO_ERROR);

    int rc = ble_gap_disc_cancel();
    if (rc != 0)
    {
        ChipLogError(DeviceLayer, "ble_gap_disc_cancel failed: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    mIsScanning = false;
    mDelegate->OnScanComplete();
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
