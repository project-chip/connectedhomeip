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

/**
 *    @file
 *      This file is the master "umbrella" include file for the core
 *      chip library.
 *
 */

#ifndef CHIPCORE_H_
#define CHIPCORE_H_

#include "core/CHIPConfig.h"

#include <SystemLayer/SystemLayer.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <BleLayer/BleLayer.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#include <InetLayer/InetLayer.h>

//Currently only used on Sapphire
#define CHIP_CORE_IDENTITY           "chip-core"
#define CHIP_CORE_PREFIX             CHIP_CORE_IDENTITY ": "

namespace chip {

#if CONFIG_NETWORK_LAYER_BLE
using namespace ::Ble;
#endif // CONFIG_NETWORK_LAYER_BLE

using namespace ::Inet;

}

#include "core/CHIPError.h"
#include "core/CHIPKeyIds.h"
#include "core/CHIPFabricState.h"
#include "core/CHIPMessageLayer.h"
#include "core/CHIPBinding.h"
#include "core/CHIPExchangeMgr.h"
#include "core/CHIPSecurityMgr.h"
#include "core/CHIPGlobals.h"

#endif /* CHIPCORE_H_ */
