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

        typedef std::pair<uint32_t, in_addr> InetInterface;
        typedef std::pair<uint32_t, in6_addr> Inet6Interface;
        typedef std::vector<InetInterface> InetInterfacesVector;
        typedef std::vector<std::pair<uint32_t, in6_addr>> Inet6InterfacesVector;
        typedef void (^OnInterfaceChanges)(InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces);

        class InterfacesMonitor {
        public:
            ~InterfacesMonitor();

            CHIP_ERROR Init(dispatch_queue_t workQueue, IPAddressType addressType, uint32_t interfaceId);

            bool IsLocalOnly() const { return mInterfaceId == kDNSServiceInterfaceIndexLocalOnly; };

            CHIP_ERROR StartMonitorInterfaces(OnInterfaceChanges interfaceChangesBlock);
            void StopMonitorInterfaces();

        private:
            nw_path_monitor_t mInterfaceMonitor = nullptr;

            // Default to kDNSServiceInterfaceIndexLocalOnly so we don't mess around
            // with un-registration if we never get Init() called.
            uint32_t mInterfaceId = kDNSServiceInterfaceIndexLocalOnly;
            IPAddressType mAddressType;

            // We use mLivenessTracker to indicate to blocks that close over us that
            // we've been destroyed.  This is needed because we're not a refcounted
            // object, so the blocks can't keep us alive; they just close over the
            // raw pointer to "this".
            std::shared_ptr<bool> mLivenessTracker;

            dispatch_queue_t mWorkQueue = nullptr;
        };

    } // namespace Darwin
} // namespace Inet
} // namespace chip
