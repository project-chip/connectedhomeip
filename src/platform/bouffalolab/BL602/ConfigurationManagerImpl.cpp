/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/ConfigurationManager.h>

extern "C" {
#include <bl_efuse.h>
}

namespace chip {
namespace DeviceLayer {

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    bl_efuse_read_mac(buf);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
