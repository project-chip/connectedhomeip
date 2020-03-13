/*
 *
 *    <COPYRIGHT>
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

#ifndef CHIP_DEVICE_H
#define CHIP_DEVICE_H

#include <core/CHIPCore.h>
// #include <core/CHIPGlobals.h>
#include <CHIPDeviceConfig.h>
#include <CHIPDeviceError.h>
#include <GeneralUtils.h>
#include <ble/BleLayer.h>
#include <PlatformManager.h>
#include <ConfigurationManager.h>
#include <ConnectivityManager.h>
#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#include <SoftwareUpdateManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#include <TimeSyncManager.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

namespace chip {
namespace DeviceLayer {

struct chipDeviceEvent;
/*
using chip::FabricState;
using chip::MessageLayer;
using chip::ExchangeMgr;
using chip::SecurityMgr;
*/
extern chip::System::Layer SystemLayer;
extern Inet::InetLayer InetLayer;

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_H
