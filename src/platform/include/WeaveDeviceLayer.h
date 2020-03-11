/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#ifndef WEAVE_DEVICE_H
#define WEAVE_DEVICE_H

#include <Weave/Core/WeaveCore.h>
#include <Weave/Core/WeaveGlobals.h>
#include <Weave/DeviceLayer/WeaveDeviceConfig.h>
#include <Weave/DeviceLayer/WeaveDeviceError.h>
#include <Weave/DeviceLayer/GeneralUtils.h>
#include <BleLayer/BleLayer.h>
#include <Weave/DeviceLayer/PlatformManager.h>
#include <Weave/DeviceLayer/ConfigurationManager.h>
#include <Weave/DeviceLayer/ConnectivityManager.h>
#if WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#include <Weave/DeviceLayer/SoftwareUpdateManager.h>
#endif // WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#if WEAVE_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
#include <Weave/DeviceLayer/TraitManager.h>
#endif // WEAVE_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
#include <Weave/DeviceLayer/TimeSyncManager.h>
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
#include <Weave/DeviceLayer/ThreadStackManager.h>
#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

namespace nl {
namespace Weave {
namespace DeviceLayer {

struct WeaveDeviceEvent;

using nl::Weave::FabricState;
using nl::Weave::MessageLayer;
using nl::Weave::ExchangeMgr;
using nl::Weave::SecurityMgr;

extern nl::Weave::System::Layer SystemLayer;
extern nl::Inet::InetLayer InetLayer;

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_H
