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
bool NimbleGetChipDeviceInfo(const ble_hs_adv_field * field, chip::Ble::ChipBLEDeviceIdentificationInfo & deviceInfo)
{
    // Check for CHIP Service UUID
    if (field && field->type == BLE_HS_ADV_TYPE_SVC_DATA_UUID16 && field->length > 10 && field->value[0] == 0xf6 &&
        field->value[1] == 0xff)
    {
        deviceInfo.OpCode                              = field->value[2];
        deviceInfo.DeviceDiscriminatorAndAdvVersion[0] = field->value[3];
        deviceInfo.DeviceDiscriminatorAndAdvVersion[1] = field->value[4];
        // vendor and product Id from adv
        deviceInfo.DeviceVendorId[0]  = field->value[5];
        deviceInfo.DeviceVendorId[1]  = field->value[6];
        deviceInfo.DeviceProductId[0] = field->value[7];
        deviceInfo.DeviceProductId[1] = field->value[8];
        deviceInfo.AdditionalDataFlag = field->value[9];
        return true;
    }
    return false;
}

} // namespace

void ChipDeviceScanner::ReportDevice(const uint8_t * adv_data, size_t adv_data_len, const ble_addr_t & addr)
{
    const struct ble_hs_adv_field * field = reinterpret_cast<const struct ble_hs_adv_field *>(adv_data);
    while (field && field->length > 0 && field->length + 1 <= adv_data_len)
    {
        chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;
        if (NimbleGetChipDeviceInfo(field, deviceInfo))
        {
            mDelegate->OnDeviceScanned(addr, deviceInfo);
            return;
        }
        adv_data     = adv_data + 1 + field->length;
        adv_data_len = adv_data_len - 1 - field->length;
        field        = adv_data_len >= 2 ? reinterpret_cast<const struct ble_hs_adv_field *>(adv_data) : nullptr;
    }
    ChipLogDetail(Ble, "Device %s does not look like a CHIP device", addr_str(addr.val));
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
        ESP_LOG_BUFFER_HEX("BLE Scanner", event->disc.data, event->disc.length_data);
        scanner->ReportDevice(event->disc.data, event->disc.length_data, event->disc.addr);
        return 0;
    }
    }

    return 0;
}

CHIP_ERROR ChipDeviceScanner::StartScan(uint16_t timeout)
{
    VerifyOrReturnError(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

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
    VerifyOrReturnError(mIsScanning, CHIP_NO_ERROR);

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
