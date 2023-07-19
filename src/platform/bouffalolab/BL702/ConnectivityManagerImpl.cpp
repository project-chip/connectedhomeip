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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>

#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <eth_bd.h>
#include <platform/bouffalolab/BL702/EthernetInterface.h>
#endif

using namespace ::chip;

namespace chip {
namespace DeviceLayer {

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_ENABLE_WIFI
extern "C" void ethernetInterface_eventGotIP(struct netif * interface)
{
    ChipLogProgress(DeviceLayer, "ethernetInterface_eventGotIP");
    ConnectivityMgrImpl().OnConnectivityChanged(interface);
}
#endif

} // namespace DeviceLayer
} // namespace chip
