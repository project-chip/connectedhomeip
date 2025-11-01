/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <atomic>
#include <lib/core/CHIPError.h>

namespace chip {
namespace Dnssd {
class DnssdServer
{
public:
    virtual ~DnssdServer() = default;

    /**
     * @brief Advertise the operational state of the device.
     *
     * @return CHIP_ERROR Returns CHIP_NO_ERROR on success, or an error code on failure.
     */
    virtual CHIP_ERROR AdvertiseOperational() = 0;

    /**
     * @brief Start the DNS-SD server.
     */
    virtual void StartServer() = 0;

    /**
     * @brief Stop the DNS-SD server.
     */
    virtual void StopServer() = 0;
};
} // namespace Dnssd
} // namespace chip
