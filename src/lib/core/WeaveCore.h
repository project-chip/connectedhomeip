/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file is the master "umbrella" include file for the core
 *      Weave library.
 *
 */

#ifndef WEAVECORE_H_
#define WEAVECORE_H_

#include <Weave/Core/WeaveConfig.h>

#include <SystemLayer/SystemLayer.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <BleLayer/BleLayer.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#include <InetLayer/InetLayer.h>

//Currently only used on Sapphire
#define NL_WEAVE_CORE_IDENTITY           "weave-core"
#define NL_WEAVE_CORE_PREFIX             NL_WEAVE_CORE_IDENTITY ": "

namespace nl {
namespace Weave {

#if CONFIG_NETWORK_LAYER_BLE
using namespace ::nl::Ble;
#endif // CONFIG_NETWORK_LAYER_BLE

using namespace ::nl::Inet;

}
}

#include <Weave/Core/WeaveError.h>
#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Core/WeaveFabricState.h>
#include <Weave/Core/WeaveMessageLayer.h>
#include <Weave/Core/WeaveBinding.h>
#include <Weave/Core/WeaveExchangeMgr.h>
#include <Weave/Core/WeaveSecurityMgr.h>
#include <Weave/Core/WeaveGlobals.h>

#endif /* WEAVECORE_H_ */
