/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AppRpc.h"

#include "Rpc.h"

#include <lib/support/logging/CHIPLogging.h>

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
#include <platform/CHIPDeviceLayer.h>
#endif

namespace chip::NXP::App::Rpc {

CHIP_ERROR Init()
{
    return chip::rpc::Init();
}

} // namespace chip::NXP::App::Rpc
