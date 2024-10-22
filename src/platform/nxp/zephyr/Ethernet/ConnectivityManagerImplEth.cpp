/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "EthManager.h"

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "ConnectivityManagerImplEth.h"

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
using namespace ::chip;
namespace chip {
namespace DeviceLayer {
CHIP_ERROR ConnectivityManagerImplEth::InitEth()
{
    return EthManager::Instance().Init();
}
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
