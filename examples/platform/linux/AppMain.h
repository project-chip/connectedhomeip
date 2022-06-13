/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <controller/CHIPDeviceController.h>
#include <controller/CommissionerDiscoveryController.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <transport/TransportMgr.h>

#include "Options.h"

int ChipLinuxAppInit(int argc, char * const argv[], chip::ArgParser::OptionSet * customOptions = nullptr);
void ChipLinuxAppMainLoop();

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

using chip::Controller::DeviceCommissioner;
using chip::Transport::PeerAddress;

CHIP_ERROR CommissionerPairOnNetwork(uint32_t pincode, uint16_t disc, PeerAddress address);
CHIP_ERROR CommissionerPairUDC(uint32_t pincode, size_t index);

DeviceCommissioner * GetDeviceCommissioner();
CommissionerDiscoveryController * GetCommissionerDiscoveryController();

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

// For extra init calls, the function will be called right before running Matter main loop.
void ApplicationInit();
