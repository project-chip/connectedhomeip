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

#include <vector>

#include <Network/Network.h>

#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace Inet {
    namespace Darwin {

        typedef std::pair<nw_interface_t, in_addr> InetInterface;
        typedef std::pair<nw_interface_t, in6_addr> Inet6Interface;
        typedef std::vector<InetInterface> InetInterfacesVector;
        typedef std::vector<Inet6Interface> Inet6InterfacesVector;
        typedef void (^OnInterfaceChanges)(InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces);
        typedef void (^OnPathChange)(nw_path_t path);

        class NetworkMonitor {
        public:
            ~NetworkMonitor();

            CHIP_ERROR Init(dispatch_queue_t workQueue, IPAddressType addressType, InterfaceId interfaceId)
            {
                return Init(workQueue, addressType, interfaceId.GetPlatformInterface());
            }

            CHIP_ERROR Init(dispatch_queue_t workQueue, IPAddressType addressType, uint32_t interfaceId);

            bool IsLocalOnly() const { return mInterfaceId == kDNSServiceInterfaceIndexLocalOnly; };

            InterfaceId GetInterfaceId() const { return InterfaceId(static_cast<InterfaceId::PlatformType>(mInterfaceId)); };

            bool IsIPv6() const { return mAddressType == IPAddressType::kIPv6; }

#if INET_CONFIG_ENABLE_IPV4
            bool IsIPv4() const
            {
                return mAddressType == IPAddressType::kIPv4;
            }
#endif

            CHIP_ERROR StartMonitorInterfaces(OnInterfaceChanges interfaceChangesBlock);
            CHIP_ERROR StartMonitorPaths(OnPathChange pathChangeBlock);
            void Stop();

        protected:
            // We use mLivenessTracker to indicate to blocks that close over us that
            // we've been destroyed.  This is needed because we're not a refcounted
            // object, so the blocks can't keep us alive; they just close over the
            // raw pointer to "this".
            std::shared_ptr<bool> mLivenessTracker;

        private:
            nw_path_monitor_t CreatePathMonitor(nw_interface_type_t type, nw_path_monitor_update_handler_t handler, bool once);
            void EnumeratePathInterfaces(nw_path_t path, InetInterfacesVector & out4, Inet6InterfacesVector & out6, bool searchLoopBackOnly);

            nw_path_monitor_t mMonitor = nullptr;

            // Default to kDNSServiceInterfaceIndexLocalOnly so we don't mess around
            // with un-registration if we never get Init() called.
            uint32_t mInterfaceId = kDNSServiceInterfaceIndexLocalOnly;
            IPAddressType mAddressType;

            dispatch_queue_t mWorkQueue = nullptr;
        };

    } // namespace Darwin
} // namespace Inet
} // namespace chip
