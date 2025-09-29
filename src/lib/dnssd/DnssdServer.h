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
#include <lib/core/CHIPError.h>

namespace chip {
namespace Dnssd {
class DnssdServer
{
public:
    virtual ~DnssdServer()                   = default;

    /**
     * @brief Advertise the operational state of the device.
     *
     * This method is responsible for advertising the operational parameters
     * of the device, such as its Peer ID, MAC address, and other relevant
     * information, to the network using DNS-SD.
     *
     * @return CHIP_ERROR Returns CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    virtual CHIP_ERROR AdvertiseOperational() = 0;

    /**
     * @brief Start the DNS-SD server.
     *
     * This method initializes and starts the DNS-SD server, enabling the device
     * to advertise its services and respond to discovery requests on the network.
    */
    virtual void StartServer()                = 0;

    /**
     * @brief Stop the DNS-SD server.
     *
     * This method stops the DNS-SD server, removing any advertised services
     * and preventing the device from responding to discovery requests.
     */
    virtual void StopServer()                 = 0;

    /**
     * @brief Check if advertising is enabled.
     *
     * This method checks whether the DNS-SD server is currently advertising
     * services on the network.
     */
    virtual bool IsAdvertisingEnabled()       = 0;
};
} // namespace Dnssd
} // namespace chip
