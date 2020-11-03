/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "core/CHIPError.h"
#include "inet/InetInterface.h"

namespace chip {
namespace Protocols {
namespace Mdns {

class Publisher
{
public:
    Publisher() = default;

    /**
     * This method initializes the publisher.
     *
     */
    CHIP_ERROR Init();

    /**
     * This method publishes the device on mDNS.
     *
     * This function will fetch device name and other information and publish them
     * via mDNS. If device meta data has changed, you can call this function again
     * to update the information.
     *
     * @param[in] interface   The interface to send mDNS multicast.
     *
     */
    CHIP_ERROR StartPublishDevice(chip::Inet::InterfaceId interface = INET_NULL_INTERFACEID);

    /**
     * This function stops publishing the device on mDNS.
     *
     */
    CHIP_ERROR StopPublishDevice();

private:
    Publisher(const Publisher &) = delete;
    Publisher & operator=(const Publisher &) = delete;

    CHIP_ERROR PublishUnprovisionedDevice(chip::Inet::InterfaceId interface);
    CHIP_ERROR PublishProvisionedDevice(chip::Inet::InterfaceId interface);
    CHIP_ERROR SetupHostname();

    static void HandleMdnsInit(void * context, CHIP_ERROR initError);
    static void HandleMdnsError(void * context, CHIP_ERROR initError);

    uint64_t mUnprovisionedInstanceName;
    bool mInitialized                   = false;
    bool mIsPublishingProvisionedDevice = false;
};

} // namespace Mdns
} // namespace Protocols
} // namespace chip
