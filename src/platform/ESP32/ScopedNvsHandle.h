/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include <platform/ESP32/ESP32Utils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "nvs.h"
#include "nvs_flash.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ScopedNvsHandle
{
public:
    ScopedNvsHandle() : mIsOpen(false) {}
    ~ScopedNvsHandle() { Close(); }
    CHIP_ERROR Open(const char * name, nvs_open_mode_t open_mode, const char * partition_label = NVS_DEFAULT_PART_NAME)
    {
        esp_err_t err = nvs_open_from_partition(partition_label, name, open_mode, &mHandle);
        if (err == ESP_OK)
        {
            mIsOpen = true;
            return CHIP_NO_ERROR;
        }
        return ESP32Utils::MapError(err);
    }
    void Close()
    {
        if (mIsOpen)
        {
            nvs_close(mHandle);
            mIsOpen = false;
        }
    }
    bool IsOpen() const { return mIsOpen; }

    operator nvs_handle_t() const { return mHandle; }

private:
    bool mIsOpen;
    nvs_handle_t mHandle;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
