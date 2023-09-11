/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/ConfigurationManager.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/bouffalolab/BL702/WiFiInterface.h>
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" {
#include <eth_bd.h>
}
#endif

namespace chip {
namespace DeviceLayer {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    wifiInterface_getMacAddress(buf);

    return CHIP_NO_ERROR;
}
#elif !CHIP_DEVICE_CONFIG_ENABLE_THREAD
CHIP_ERROR ConfigurationManagerImpl::GetPrimaryMACAddress(MutableByteSpan buf)
{
    if (buf.size() != ConfigurationManager::kPrimaryMACAddressLength)
        return CHIP_ERROR_INVALID_ARGUMENT;

    eth_get_mac(buf.data());

    return CHIP_NO_ERROR;
}
#endif

} // namespace DeviceLayer
} // namespace chip
