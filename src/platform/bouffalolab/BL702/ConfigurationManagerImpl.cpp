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

#include <platform/ConfigurationManager.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/bouffalolab/BL702/wifi_mgmr_portable.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#include "EthernetInterface.h"
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

namespace chip {
namespace DeviceLayer {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    wifiInterface_getMacAddress(buf);

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
CHIP_ERROR ConfigurationManagerImpl::GetPrimaryMACAddress(MutableByteSpan & buf)
{
    if (buf.size() != ConfigurationManager::kPrimaryMACAddressLength)
        return CHIP_ERROR_INVALID_ARGUMENT;

    memcpy(buf.data(), deviceInterface_getNetif()->hwaddr, ConfigurationManager::kPrimaryMACAddressLength);

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

} // namespace DeviceLayer
} // namespace chip
