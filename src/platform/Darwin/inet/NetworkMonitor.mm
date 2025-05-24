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

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include "NetworkMonitor.h"

#include <lib/support/CodeUtils.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if_dl.h>
#include <netdb.h>

#include <set>

namespace chip {
namespace Inet {
    namespace Darwin {

        namespace {

#if CHIP_PROGRESS_LOGGING
            constexpr char kPathStatusInvalid[] = "Invalid";
            constexpr char kPathStatusUnsatisfied[] = "Unsatisfied";
            constexpr char kPathStatusSatisfied[] = "Satisfied";
            constexpr char kPathStatusSatisfiable[] = "Satisfiable";
            constexpr char kPathStatusUnknown[] = "Unknown";

            constexpr char kInterfaceTypeCellular[] = "Cellular";
            constexpr char kInterfaceTypeWiFi[] = "WiFi";
            constexpr char kInterfaceTypeWired[] = "Wired";
            constexpr char kInterfaceTypeLoopback[] = "Loopback";
            constexpr char kInterfaceTypeOther[] = "Other";
            constexpr char kInterfaceTypeUnknown[] = "Unknown";

            const char * GetPathStatusString(nw_path_status_t status)
            {
                const char * str = nullptr;

                if (status == nw_path_status_invalid) {
                    str = kPathStatusInvalid;
                } else if (status == nw_path_status_unsatisfied) {
                    str = kPathStatusUnsatisfied;
                } else if (status == nw_path_status_satisfied) {
                    str = kPathStatusSatisfied;
                } else if (status == nw_path_status_satisfiable) {
                    str = kPathStatusSatisfiable;
                } else {
                    str = kPathStatusUnknown;
                }

                return str;
            }

            const char * GetInterfaceTypeString(nw_interface_type_t type)
            {
                const char * str = nullptr;

                if (type == nw_interface_type_cellular) {
                    str = kInterfaceTypeCellular;
                } else if (type == nw_interface_type_wifi) {
                    str = kInterfaceTypeWiFi;
                } else if (type == nw_interface_type_wired) {
                    str = kInterfaceTypeWired;
                } else if (type == nw_interface_type_loopback) {
                    str = kInterfaceTypeLoopback;
                } else if (type == nw_interface_type_other) {
                    str = kInterfaceTypeOther;
                } else {
                    str = kInterfaceTypeUnknown;
                }

                return str;
            }

            void LogDetails(uint32_t interfaceId, InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces)
            {
                for (auto & inetInterface : inetInterfaces) {
                    if (interfaceId == nw_interface_get_index(inetInterface.first)) {
                        char addr[INET_ADDRSTRLEN] = {};
                        inet_ntop(AF_INET, &inetInterface.second, addr, sizeof(addr));
                        ChipLogProgress(Inet, "\t\t* ipv4: %s", addr);
                    }
                }

                for (auto & inet6Interface : inet6Interfaces) {
                    if (interfaceId == nw_interface_get_index(inet6Interface.first)) {
                        char addr[INET6_ADDRSTRLEN] = {};
                        inet_ntop(AF_INET6, &inet6Interface.second, addr, sizeof(addr));
                        ChipLogProgress(Inet, "\t\t* ipv6: %s", addr);
                    }
                }
            }

            void LogDetails(nw_path_t path)
            {
                auto status = nw_path_get_status(path);
                ChipLogProgress(Inet, "Status: %s", GetPathStatusString(status));
            }

            void LogDetails(nw_interface_t interface, InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces)
            {
                auto interfaceId = nw_interface_get_index(interface);
                auto interfaceName = nw_interface_get_name(interface);
                auto interfaceType = nw_interface_get_type(interface);
                ChipLogProgress(Inet, "\t%s (%u / %s)", interfaceName, interfaceId, GetInterfaceTypeString(interfaceType));
                LogDetails(interfaceId, inetInterfaces, inet6Interfaces);
            }
#else
            void LogDetails(uint32_t interfaceId, InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces) {};
            void LogDetails(nw_path_t path) {};
            void LogDetails(InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces) {};
            void LogDetails(nw_interface_t interface, InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces) {};
#endif // CHIP_PROGRESS_LOGGING

            bool HasValidFlags(unsigned int flags, bool allowLoopbackOnly)
            {
                VerifyOrReturnValue(!allowLoopbackOnly || (flags & IFF_LOOPBACK), false);
                VerifyOrReturnValue((flags & IFF_RUNNING), false);
                VerifyOrReturnValue((flags & IFF_MULTICAST), false);
                return true;
            }

            bool HasValidNetworkType(nw_interface_t interface)
            {
                auto interfaceType = nw_interface_get_type(interface);
                return interfaceType == nw_interface_type_wifi || interfaceType == nw_interface_type_wired || interfaceType == nw_interface_type_other || interfaceType == nw_interface_type_loopback;
            }

            bool IsValidInterfaceId(uint32_t targetInterfaceId, nw_interface_t interface)
            {
                auto currentInterfaceId = nw_interface_get_index(interface);
                return targetInterfaceId == kDNSServiceInterfaceIndexAny || targetInterfaceId == currentInterfaceId;
            }

            void ShouldUseVersion(chip::Inet::IPAddressType addressType, bool & shouldUseIPv4, bool & shouldUseIPv6)
            {
#if INET_CONFIG_ENABLE_IPV4
                shouldUseIPv4 = addressType == Inet::IPAddressType::kIPv4 || addressType == Inet::IPAddressType::kAny;
#else
                shouldUseIPv4 = false;
#endif // INET_CONFIG_ENABLE_IPV4
                shouldUseIPv6 = addressType == Inet::IPAddressType::kIPv6 || addressType == Inet::IPAddressType::kAny;
            }

            void GetInterfaceAddresses(nw_interface_t interface, chip::Inet::IPAddressType addressType, InetInterfacesVector & inetInterfaces,
                Inet6InterfacesVector & inet6Interfaces, bool searchLoopbackOnly = false)
            {
                bool shouldUseIPv4, shouldUseIPv6;
                ShouldUseVersion(addressType, shouldUseIPv4, shouldUseIPv6);

                ifaddrs * ifap;
                VerifyOrReturn(getifaddrs(&ifap) >= 0);

                uint32_t interfaceId = nw_interface_get_index(interface);
                for (struct ifaddrs * ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
                    auto interfaceAddress = ifa->ifa_addr;
                    if (interfaceAddress == nullptr) {
                        continue;
                    }

                    if (!HasValidFlags(ifa->ifa_flags, searchLoopbackOnly)) {
                        continue;
                    }

                    auto currentInterfaceId = if_nametoindex(ifa->ifa_name);
                    if (interfaceId != currentInterfaceId) {
                        continue;
                    }

                    if (shouldUseIPv4 && (AF_INET == interfaceAddress->sa_family)) {
                        auto inetAddress = reinterpret_cast<struct sockaddr_in *>(interfaceAddress)->sin_addr;
                        inetInterfaces.push_back(Inet::Darwin::InetInterface(interface, inetAddress));
                    } else if (shouldUseIPv6 && (AF_INET6 == interfaceAddress->sa_family)) {
                        auto inet6Address = reinterpret_cast<struct sockaddr_in6 *>(interfaceAddress)->sin6_addr;
                        inet6Interfaces.push_back(Inet::Darwin::Inet6Interface(interface, inet6Address));
                    }
                }

                freeifaddrs(ifap);
            }
        } // namespace

        NetworkMonitor::~NetworkMonitor()
        {
            if (mLivenessTracker != nullptr) {
                *mLivenessTracker = false;
            }
        }

        CHIP_ERROR NetworkMonitor::Init(dispatch_queue_t workQueue, Inet::IPAddressType addressType, uint32_t interfaceId)
        {
            mInterfaceId = interfaceId;
            mAddressType = addressType;
            mWorkQueue = workQueue;
            mMonitor = nullptr;

            mLivenessTracker = std::make_shared<bool>(true);
            if (mLivenessTracker == nullptr) {
                return CHIP_ERROR_NO_MEMORY;
            }

            return CHIP_NO_ERROR;
        }

        nw_path_monitor_t NetworkMonitor::CreatePathMonitor(nw_interface_type_t type, nw_path_monitor_update_handler_t handler, bool once)
        {
            __auto_type monitor = nw_path_monitor_create_with_type(type);
            VerifyOrReturnValue(nullptr != monitor, nullptr);

            nw_path_monitor_set_queue(monitor, mWorkQueue);

            // Our update handler closes over "this", but can't keep us alive (because we
            // are not refcounted).  Make sure it closes over a shared ref to our
            // liveness tracker, which it _can_ keep alive, so it can bail out if we
            // have been destroyed between when the task was queued and when it ran.
            std::shared_ptr<bool> livenessTracker = mLivenessTracker;

            nw_path_monitor_set_update_handler(monitor, ^(nw_path_t path) {
                VerifyOrReturn(*livenessTracker); // The NetworkMonitor has been destroyed; just bail out.

                LogDetails(path);

                auto status = nw_path_get_status(path);
                if (status == nw_path_status_satisfied) {
                    handler(path);
                }

                if (once) {
                    nw_path_monitor_cancel(monitor);
                }
            });
            return monitor;
        }

        void NetworkMonitor::EnumeratePathInterfaces(nw_path_t path, InetInterfacesVector & out4, Inet6InterfacesVector & out6, bool searchLoopbackOnly)
        {
            nw_path_enumerate_interfaces(path, ^(nw_interface_t interface) {
                VerifyOrReturnValue(HasValidNetworkType(interface), true);
                VerifyOrReturnValue(IsValidInterfaceId(mInterfaceId, interface), true);

                GetInterfaceAddresses(interface, mAddressType, out4, out6, searchLoopbackOnly);
                LogDetails(interface, out4, out6);
                return true;
            });
        }

        CHIP_ERROR NetworkMonitor::StartMonitorPaths(OnPathChange pathChangeBlock)
        {
            VerifyOrReturnError(nullptr == mMonitor, CHIP_ERROR_INCORRECT_STATE);

            __auto_type pathMonitorHandler = ^(nw_path_t path) {
                pathChangeBlock(path);
            };

            mMonitor = CreatePathMonitor(nw_interface_type_other, pathMonitorHandler, false /* once */);
            VerifyOrReturnError(nullptr != mMonitor, CHIP_ERROR_NO_MEMORY);

            nw_path_monitor_start(mMonitor);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR NetworkMonitor::StartMonitorInterfaces(OnInterfaceChanges interfaceChangesBlock)
        {
            VerifyOrReturnError(nullptr == mMonitor, CHIP_ERROR_INCORRECT_STATE);

            __block InetInterfacesVector inetLoopback;
            __block Inet6InterfacesVector inet6Loopback;

            __auto_type loopbackMonitorHandler = ^(nw_path_t path) {
                EnumeratePathInterfaces(path, inetLoopback, inet6Loopback, true /* searchLoopbackOnly */);
                nw_path_monitor_start(mMonitor);
            };

            __auto_type otherMonitorHandler = ^(nw_path_t path) {
                __block InetInterfacesVector inet = inetLoopback;
                __block Inet6InterfacesVector inet6 = inet6Loopback;
                EnumeratePathInterfaces(path, inet, inet6, false /* searchLoopbackOnly */);
                interfaceChangesBlock(inet, inet6);
            };

            __auto_type loopbackMonitor = CreatePathMonitor(nw_interface_type_loopback, loopbackMonitorHandler, true /* once */);
            VerifyOrReturnError(nullptr != loopbackMonitor, CHIP_ERROR_NO_MEMORY);

            mMonitor = CreatePathMonitor(nw_interface_type_other, otherMonitorHandler, false /* once */);
            VerifyOrReturnError(nullptr != mMonitor, CHIP_ERROR_NO_MEMORY);

            nw_path_monitor_start(loopbackMonitor);
            return CHIP_NO_ERROR;
        }

        void NetworkMonitor::Stop()
        {
            if (mMonitor != nullptr) {
                nw_path_monitor_cancel(mMonitor);
                mMonitor = nullptr;
            }
        }

    } // namespace Darwin
} // namespace Inet
} // namespace chip
