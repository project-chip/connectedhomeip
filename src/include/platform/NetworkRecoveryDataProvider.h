/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/CHIPError.h>

namespace chip {

namespace DeviceLayer {

class NetworkRecoveryDataProvider
{
public:
    NetworkRecoveryDataProvider()          = default;
    virtual ~NetworkRecoveryDataProvider() = default;

    // Not copyable
    NetworkRecoveryDataProvider(const NetworkRecoveryDataProvider &)             = delete;
    NetworkRecoveryDataProvider & operator=(const NetworkRecoveryDataProvider &) = delete;

    virtual bool ShouldAdvertise()                                                 = 0;
    virtual CHIP_ERROR GetNetworkRecoveryIdentifier(uint64_t & recoveryIdentifier) = 0;
    virtual CHIP_ERROR GetNetworkRecoveryReason(uint8_t & reason)                  = 0;
};

/**
 * Instance getter for the global NetworkRecoveryDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global NetworkRecoveryDataProvider. Assume never null.
 */
NetworkRecoveryDataProvider * GetNetworkRecoveryDataProvider();

/**
 * Instance setter for the global NetworkRecoveryDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the NetworkRecoveryDataProvider to start returning with the getter
 */
void SetNetworkRecoveryDataProvider(NetworkRecoveryDataProvider * provider);

} // namespace DeviceLayer
} // namespace chip
