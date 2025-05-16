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

#include "InterfacesMonitor.h"

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
                    if (interfaceId == inetInterface.first) {
                        char addr[INET_ADDRSTRLEN] = {};
                        inet_ntop(AF_INET, &inetInterface.second, addr, sizeof(addr));
                        ChipLogProgress(Inet, "\t\t* ipv4: %s", addr);
                    }
                }

                for (auto & inet6Interface : inet6Interfaces) {
                    if (interfaceId == inet6Interface.first) {
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

            void LogDetails(InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces)
            {
                std::set<uint32_t> interfaceIds;
                for (auto & inetInterface : inetInterfaces) {
                    interfaceIds.insert(inetInterface.first);
                }

                for (auto & inet6Interface : inet6Interfaces) {
                    interfaceIds.insert(inet6Interface.first);
                }

                for (auto interfaceId : interfaceIds) {
                    char interfaceName[Inet::InterfaceId::kMaxIfNameLength] = {};
                    if_indextoname(interfaceId, interfaceName);
                    ChipLogProgress(Inet, "\t%s (%u)", interfaceName, interfaceId);
                    LogDetails(interfaceId, inetInterfaces, inet6Interfaces);
                }
            }

            void LogDetails(nw_interface_t interface, InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces)
            {
                auto interfaceId = nw_interface_get_index(interface);
                auto interfaceName = nw_interface_get_name(interface);
                auto interfaceType = nw_interface_get_type(interface);
                ChipLogProgress(Inet, "\t%s (%u / %s)", interfaceName, interfaceId, GetInterfaceTypeString(interfaceType));
                LogDetails(interfaceId, inetInterfaces, inet6Interfaces);
            }
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
                return interfaceType == nw_interface_type_wifi || interfaceType == nw_interface_type_wired || interfaceType == nw_interface_type_other;
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

            void GetInterfaceAddresses(uint32_t interfaceId, chip::Inet::IPAddressType addressType, InetInterfacesVector & inetInterfaces,
                Inet6InterfacesVector & inet6Interfaces, bool searchLoopbackOnly = false)
            {
                bool shouldUseIPv4, shouldUseIPv6;
                ShouldUseVersion(addressType, shouldUseIPv4, shouldUseIPv6);

                ifaddrs * ifap;
                VerifyOrReturn(getifaddrs(&ifap) >= 0);

                for (struct ifaddrs * ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
                    auto interfaceAddress = ifa->ifa_addr;
                    if (interfaceAddress == nullptr) {
                        continue;
                    }

                    if (!HasValidFlags(ifa->ifa_flags, searchLoopbackOnly)) {
                        continue;
                    }

                    auto currentInterfaceId = if_nametoindex(ifa->ifa_name);
                    if (interfaceId != kDNSServiceInterfaceIndexAny && interfaceId != currentInterfaceId) {
                        continue;
                    }

                    if (shouldUseIPv4 && (AF_INET == interfaceAddress->sa_family)) {
                        auto inetAddress = reinterpret_cast<struct sockaddr_in *>(interfaceAddress)->sin_addr;
                        inetInterfaces.push_back(Inet::Darwin::InetInterface(currentInterfaceId, inetAddress));
                    } else if (shouldUseIPv6 && (AF_INET6 == interfaceAddress->sa_family)) {
                        auto inet6Address = reinterpret_cast<struct sockaddr_in6 *>(interfaceAddress)->sin6_addr;
                        inet6Interfaces.push_back(Inet::Darwin::Inet6Interface(currentInterfaceId, inet6Address));
                    }
                }

                freeifaddrs(ifap);
            }
        } // namespace

        InterfacesMonitor::~InterfacesMonitor()
        {
            if (mLivenessTracker != nullptr) {
                *mLivenessTracker = false;
            }
        }

        CHIP_ERROR InterfacesMonitor::Init(dispatch_queue_t workQueue, Inet::IPAddressType addressType, uint32_t interfaceId)
        {
            mInterfaceId = interfaceId;
            mAddressType = addressType;
            mWorkQueue = workQueue;
            mInterfaceMonitor = nullptr;

            mLivenessTracker = std::make_shared<bool>(true);
            if (mLivenessTracker == nullptr) {
                return CHIP_ERROR_NO_MEMORY;
            }

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR InterfacesMonitor::StartMonitorInterfaces(OnInterfaceChanges interfaceChangesBlock)
        {
            mInterfaceMonitor = nw_path_monitor_create();
            VerifyOrReturnError(mInterfaceMonitor != nullptr, CHIP_ERROR_NO_MEMORY);

            nw_path_monitor_set_queue(mInterfaceMonitor, mWorkQueue);

            // Our update handler closes over "this", but can't keep us alive (because we
            // are not refcounted).  Make sure it closes over a shared ref to our
            // liveness tracker, which it _can_ keep alive, so it can bail out if we
            // have been destroyed between when the task was queued and when it ran.
            std::shared_ptr<bool> livenessTracker = mLivenessTracker;
            nw_path_monitor_set_update_handler(mInterfaceMonitor, ^(nw_path_t path) {
                if (!*livenessTracker) {
                    // The InterfacesMonitor has been destroyed; just bail out.
                    return;
                }

#if CHIP_PROGRESS_LOGGING
                LogDetails(path);
#endif // CHIP_PROGRESS_LOGGING

                __block InetInterfacesVector inet;
                __block Inet6InterfacesVector inet6;

                // The loopback interfaces needs to be manually added. While lo0 is usually 1, this is not guaranteed. So search for a
                // loopback interface with the specified interface id. If the specified interface id is kDNSServiceInterfaceIndexAny, it
                // will look for all available loopback interfaces.
                GetInterfaceAddresses(mInterfaceId, mAddressType, inet, inet6, true /* searchLoopbackOnly */);
#if CHIP_PROGRESS_LOGGING
                LogDetails(inet, inet6);
#endif // CHIP_PROGRESS_LOGGING

                auto status = nw_path_get_status(path);
                if (status == nw_path_status_satisfied) {
                    nw_path_enumerate_interfaces(path, ^(nw_interface_t interface) {
                        VerifyOrReturnValue(HasValidNetworkType(interface), true);
                        VerifyOrReturnValue(IsValidInterfaceId(mInterfaceId, interface), true);

                        auto targetInterfaceId = nw_interface_get_index(interface);
                        GetInterfaceAddresses(targetInterfaceId, mAddressType, inet, inet6);
#if CHIP_PROGRESS_LOGGING
                        LogDetails(interface, inet, inet6);
#endif // CHIP_PROGRESS_LOGGING
                        return true;
                    });
                }

                interfaceChangesBlock(inet, inet6);
            });

            nw_path_monitor_start(mInterfaceMonitor);

            return CHIP_NO_ERROR;
        }

        void InterfacesMonitor::StopMonitorInterfaces()
        {
            if (mInterfaceMonitor != nullptr) {
                nw_path_monitor_cancel(mInterfaceMonitor);
                mInterfaceMonitor = nullptr;
            }
        }

    } // namespace Darwin
} // namespace Inet
} // namespace chip
