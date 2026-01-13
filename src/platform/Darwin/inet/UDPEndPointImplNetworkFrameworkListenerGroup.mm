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

#include "UDPEndPointImplNetworkFrameworkListenerGroup.h"

#include "UDPEndPointImplNetworkFrameworkDebug.h"

#include <lib/support/CodeUtils.h>

constexpr uint64_t kStartUpInterfaceMonitorTimeoutInSeconds = 3 * NSEC_PER_SEC;
constexpr uint64_t kConnectionGroupReadyTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kConnectionGroupCancelledTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr const char * kInterfaceMonitorQueueName = "inet_interfaces_monitor";
constexpr const char * kConnectionGroupQueueName = "inet_connection_group";

namespace chip {
namespace Inet {
    namespace Darwin {
        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::Configure(nw_parameters_t parameters, const IPAddressType addressType, InterfaceId interfaceId)
        {
            VerifyOrReturnError(nullptr == mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);

            __auto_type monitorQueue = dispatch_queue_create(kInterfaceMonitorQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            VerifyOrReturnError(nil != monitorQueue, CHIP_ERROR_NO_MEMORY);
            ReturnErrorOnFailure(NetworkMonitor::Init(monitorQueue, addressType, interfaceId));

            __auto_type semaphore = dispatch_semaphore_create(0);
            __auto_type pathChangeBlock = ^(nw_path_t path) {
                mLastPath = path;
                dispatch_semaphore_signal(semaphore);
            };
            ReturnErrorOnFailure(StartMonitorPaths(pathChangeBlock));
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kStartUpInterfaceMonitorTimeoutInSeconds);
            VerifyOrReturnError(0 == dispatch_semaphore_wait(semaphore, timeout), CHIP_ERROR_INCORRECT_STATE);

            mConnectionGroupQueue = dispatch_queue_create(kConnectionGroupQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            VerifyOrReturnError(nil != mConnectionGroupQueue, CHIP_ERROR_NO_MEMORY);

            CFDictionaryValueCallBacks valueCallbacks = {
                0, // version
                nullptr, // retain callback
                ReleaseInterfaceGroupCallback,
                nullptr, // copyDescription callback
                nullptr // equal callback
            };

            mInterfaceGroups = CFDictionaryCreateMutable(
                kCFAllocatorDefault,
                0,
                &kCFTypeDictionaryKeyCallBacks,
                &valueCallbacks);

            mLocalParameters = parameters;

            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkListenerGroup::Unlisten()
        {
            NetworkMonitor::Stop();
            StopListeners();

            if (mInterfaceGroups != nullptr) {
                CFRelease(mInterfaceGroups);
                mInterfaceGroups = nullptr;
            }

            mConnectionGroupQueue = nullptr;
            mLocalParameters = nullptr;
            mLastPath = nullptr;
        }

#if INET_CONFIG_ENABLE_IPV4
        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::IPv4JoinLeaveMulticastGroup(InterfaceId interfaceId, const IPAddress & address, bool join)
        {
            VerifyOrReturnError(nullptr != mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr != mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr != mLastPath, CHIP_ERROR_INCORRECT_STATE);

            if (!interfaceId.IsPresent()) {
                // Do it on all the viable interfaces.
                __block bool interfaceFound = false;

                nw_path_enumerate_interfaces(mLastPath, ^bool(nw_interface_t interface) {
                    InterfaceId::PlatformType interfaceIndex = nw_interface_get_index(interface);
                    InterfaceId infId = static_cast<InterfaceId>(interfaceIndex);

                    IPAddress ifAddr;
                    if (infId.GetLinkLocalAddr(&ifAddr) != CHIP_NO_ERROR) {
                        return true;
                    }

                    if (ifAddr.Type() != IPAddressType::kIPv4) {
                        // Not the right sort of interface.
                        return true;
                    }

                    interfaceFound = true;

                    const char * ifName = nw_interface_get_name(interface);

                    // Ignore errors here, except for logging, because we expect some of
                    // these interfaces to not work, and some (e.g. loopback) to always
                    // work.
                    CHIP_ERROR err = IPv4JoinLeaveMulticastGroup(infId, address, join);
                    if (err == CHIP_NO_ERROR) {
                        ChipLogDetail(Inet, "  %s multicast group on interface %s", (join ? "Joined" : "Left"), ifName);
                    } else {
                        ChipLogError(Inet, "  Failed to %s multicast group on interface %s", (join ? "join" : "leave"), ifName);
                    }

                    return true; // In order to continue the enumeration
                });

                if (interfaceFound) {
                    // Assume we're good.
                    return CHIP_NO_ERROR;
                }

                // Else go ahead and try to work with the default interface.
                ChipLogError(Inet, "No valid IPv4 multicast interface found");
            }

            __auto_type endpoint = GetEndPoint(IPAddressType::kIPv4, address, GetBoundPort(), interfaceId);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

            return JoinLeaveMulticastGroup(interfaceId, endpoint, join);
        }
#endif // INET_CONFIG_ENABLE_IPV4

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::IPv6JoinLeaveMulticastGroup(InterfaceId interfaceId, const IPAddress & address, bool join)
        {
            VerifyOrReturnError(nullptr != mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr != mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr != mLastPath, CHIP_ERROR_INCORRECT_STATE);

            if (!interfaceId.IsPresent()) {
                // Do it on all the viable interfaces.
                __block bool interfaceFound = false;

                nw_path_enumerate_interfaces(mLastPath, ^(nw_interface_t interface) {
                    InterfaceId::PlatformType interfaceIndex = nw_interface_get_index(interface);
                    InterfaceId infId = static_cast<InterfaceId>(interfaceIndex);

                    IPAddress ifAddr;
                    if (infId.GetLinkLocalAddr(&ifAddr) != CHIP_NO_ERROR) {
                        return true;
                    }

                    if (ifAddr.Type() != IPAddressType::kIPv6) {
                        // Not the right sort of interface.
                        return true;
                    }

                    interfaceFound = true;

                    const char * ifName = nw_interface_get_name(interface);

                    // Ignore errors here, except for logging, because we expect some of
                    // these interfaces to not work, and some (e.g. loopback) to always
                    // work.
                    CHIP_ERROR err = IPv6JoinLeaveMulticastGroup(infId, address, join);
                    if (err == CHIP_NO_ERROR) {
                        ChipLogDetail(Inet, "  %s multicast group on interface %s", (join ? "Joined" : "Left"), ifName);
                    } else {
                        ChipLogError(Inet, "  Failed to %s multicast group on interface %s", (join ? "join" : "leave"), ifName);
                    }

                    return true; // In order to continue the enumeration
                });

                if (interfaceFound) {
                    // Assume we're good.
                    return CHIP_NO_ERROR;
                }

                // Else go ahead and try to work with the default interface.
                ChipLogError(Inet, "No valid IPv6 multicast interface found");
            }

            __auto_type endpoint = GetEndPoint(IPAddressType::kIPv6, address, GetBoundPort(), interfaceId);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

            return JoinLeaveMulticastGroup(interfaceId, endpoint, join);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::JoinLeaveMulticastGroup(InterfaceId interfaceId, nw_endpoint_t endpoint, bool join)
        {
            __block nw_interface_t targetInterface = nullptr;
            nw_path_enumerate_interfaces(mLastPath, ^(nw_interface_t interface) {
                if (interfaceId.GetPlatformInterface() == nw_interface_get_index(interface)) {
                    targetInterface = interface;
                    return false;
                }

                return true;
            });

            return (join ? JoinMulticastGroup(targetInterface, endpoint) : LeaveMulticastGroup(targetInterface, endpoint));
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::JoinMulticastGroup(nw_interface_t interface, nw_endpoint_t endpoint)
        {
            uint32_t ifIndex = interface ? nw_interface_get_index(interface) : 0;
            CFNumberRef key = CFNumberCreate(nullptr, kCFNumberIntType, &ifIndex);
            __auto_type * interfaceGroup = (InterfaceGroup *) CFDictionaryGetValue(mInterfaceGroups, key);
            CFRelease(key);

            if (nullptr == interfaceGroup) {
                __auto_type groupDescriptor = nw_group_descriptor_create_multicast(endpoint);
                VerifyOrReturnError(nullptr != groupDescriptor, CHIP_ERROR_INCORRECT_STATE);

                interfaceGroup = (InterfaceGroup *) calloc(1, sizeof(InterfaceGroup));
                interfaceGroup->interface = interface;

                CHIP_ERROR error = StartListeners(interfaceGroup, groupDescriptor);
                VerifyOrReturnError(CHIP_NO_ERROR == error, error, free(interfaceGroup));

                key = CFNumberCreate(nullptr, kCFNumberIntType, &ifIndex);
                CFDictionarySetValue(mInterfaceGroups, key, interfaceGroup);
                CFRelease(key);
                return CHIP_NO_ERROR;
            }

            VerifyOrReturnError(nullptr != interfaceGroup->connectionGroup, CHIP_ERROR_INCORRECT_STATE);
            __auto_type oldGroupDescriptor = nw_connection_group_copy_descriptor(interfaceGroup->connectionGroup);
            StopListeners(interfaceGroup);

            __block nw_group_descriptor_t groupDescriptor = nullptr;
            __block bool alreadyPresent = false;
            __block CHIP_ERROR error = CHIP_NO_ERROR;
            nw_group_descriptor_enumerate_endpoints(oldGroupDescriptor, ^(nw_endpoint_t oldEndpoint) {
                if (IsSameEndPoints(oldEndpoint, endpoint)) {
                    alreadyPresent = true;
                }

                if (nullptr == groupDescriptor) {
                    groupDescriptor = nw_group_descriptor_create_multicast(oldEndpoint);
                    VerifyOrReturnValue(nullptr != groupDescriptor, false, error = CHIP_ERROR_INCORRECT_STATE);
                } else {
                    bool joined = nw_group_descriptor_add_endpoint(groupDescriptor, oldEndpoint);
                    VerifyOrReturnValue(joined, false, error = CHIP_ERROR_INCORRECT_STATE);
                }
                return true;
            });
            ReturnErrorOnFailure(error);

            if (!alreadyPresent) {
                bool joined = nw_group_descriptor_add_endpoint(groupDescriptor, endpoint);
                VerifyOrReturnError(joined, CHIP_ERROR_INCORRECT_STATE);
            }

            return StartListeners(interfaceGroup, groupDescriptor);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::LeaveMulticastGroup(nw_interface_t interface, nw_endpoint_t endpoint)
        {
            uint32_t ifIndex = interface ? nw_interface_get_index(interface) : 0;
            CFNumberRef key = CFNumberCreate(nullptr, kCFNumberIntType, &ifIndex);
            __auto_type * interfaceGroup = (InterfaceGroup *) CFDictionaryGetValue(mInterfaceGroups, key);
            CFRelease(key);

            VerifyOrReturnError(nullptr != interfaceGroup, CHIP_ERROR_NOT_FOUND); // Nothing ever joined, can't leave!
            VerifyOrReturnError(nullptr != interfaceGroup->connectionGroup, CHIP_ERROR_NOT_FOUND); // Nothing ever joined, can't leave!

            __auto_type oldGroupDescriptor = nw_connection_group_copy_descriptor(interfaceGroup->connectionGroup);
            StopListeners(interfaceGroup);

            __block nw_group_descriptor_t groupDescriptor = nullptr;
            __block size_t endpointCount = 0;
            __block CHIP_ERROR error = CHIP_NO_ERROR;
            nw_group_descriptor_enumerate_endpoints(oldGroupDescriptor, ^(nw_endpoint_t oldEndpoint) {
                VerifyOrReturnValue(!IsSameEndPoints(oldEndpoint, endpoint), true); // Leaving -> skip adding this one

                if (nullptr == groupDescriptor) {
                    groupDescriptor = nw_group_descriptor_create_multicast(oldEndpoint);
                    VerifyOrReturnValue(nullptr != groupDescriptor, false, error = CHIP_ERROR_INCORRECT_STATE);
                } else {
                    bool joined = nw_group_descriptor_add_endpoint(groupDescriptor, oldEndpoint);
                    VerifyOrReturnValue(joined, false, error = CHIP_ERROR_INCORRECT_STATE);
                }

                endpointCount++;
                return true;
            });
            ReturnErrorOnFailure(error);

            if (endpointCount == 0) {
                key = CFNumberCreate(nullptr, kCFNumberIntType, &ifIndex);
                CFDictionaryRemoveValue(mInterfaceGroups, key);
                CFRelease(key);
                return CHIP_NO_ERROR;
            }

            return StartListeners(interfaceGroup, groupDescriptor);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::StartListeners(InterfaceGroup * group, nw_group_descriptor_t groupDescriptor)
        {
            __auto_type parameters = nw_parameters_copy(mLocalParameters);
            VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_NO_MEMORY);
            nw_parameters_set_local_endpoint(parameters, nil);
            nw_parameters_require_interface(parameters, group->interface);

            __auto_type connectionGroup = nw_connection_group_create(groupDescriptor, parameters);
            VerifyOrReturnError(nullptr != connectionGroup, CHIP_ERROR_INVALID_ARGUMENT);

            nw_connection_group_set_queue(connectionGroup, mConnectionGroupQueue);
            nw_connection_group_set_receive_handler(connectionGroup, UINT32_MAX, YES, ^(dispatch_data_t content, nw_content_context_t context, bool complete) {
                ChipLogError(Inet, "%s", __func__);
            });

            ReturnErrorOnFailure(WaitForConnectionGroupReadyState(group, connectionGroup));

            group->connectionGroup = connectionGroup;
            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkListenerGroup::StopListeners(InterfaceGroup * group)
        {
            VerifyOrReturn(nullptr != mInterfaceGroups);

            if (nullptr == group) {
                CFDictionaryApplyFunction(
                    mInterfaceGroups, [](const void * /*key*/, const void * value, void * context) {
                    __auto_type * interfaceGroup = (InterfaceGroup *) value;
                    __auto_type * self = static_cast<UDPEndPointImplNetworkFrameworkListenerGroup *>(context);
                    self->StopListeners(interfaceGroup); }, this);

                return;
            }

            VerifyOrReturn(nullptr != group->connectionGroup);
            LogErrorOnFailure(WaitForConnectionGroupCancelledState(group));
            group->connectionGroup = nullptr;
            group->cancelSemaphore = nullptr;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::WaitForConnectionGroupReadyState(InterfaceGroup * group, nw_connection_group_t connectionGroup)
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

            __auto_type cancelSemaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != cancelSemaphore, CHIP_ERROR_NO_MEMORY);
            group->cancelSemaphore = cancelSemaphore;

            __auto_type interface = group->interface;
            __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
            nw_connection_group_set_state_changed_handler(connectionGroup, ^(nw_connection_group_state_t state, nw_error_t error) {
                DebugPrintConnectionGroupState(state, interface, error);

                switch (state) {
                case nw_connection_group_state_waiting:
                case nw_connection_group_state_invalid:
                    err = CHIP_ERROR_INCORRECT_STATE;
                    nw_connection_group_cancel(connectionGroup);
                    break;
                case nw_connection_group_state_ready:
                    err = CHIP_NO_ERROR;
                    dispatch_semaphore_signal(semaphore);
                    break;
                case nw_connection_group_state_failed:
                    err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                    nw_connection_group_cancel(connectionGroup);
                    break;
                case nw_connection_group_state_cancelled:
                    if (err == CHIP_NO_ERROR) {
                        err = CHIP_ERROR_CONNECTION_ABORTED;
                    }
                    dispatch_semaphore_signal(semaphore);
                    dispatch_semaphore_signal(cancelSemaphore);
                    break;
                default:
                    break;
                }
            });

            nw_connection_group_start(connectionGroup);
            dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionGroupReadyTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)

            return err;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::WaitForConnectionGroupCancelledState(InterfaceGroup * group)
        {
            nw_connection_group_cancel(group->connectionGroup);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionGroupCancelledTimeoutInSeconds);
            dispatch_semaphore_wait(group->cancelSemaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)

            return CHIP_NO_ERROR;
        }

        bool UDPEndPointImplNetworkFrameworkListenerGroup::IsSameEndPoints(nw_endpoint_t a, nw_endpoint_t b)
        {
            const sockaddr * addrA = nw_endpoint_get_address(a);
            const sockaddr * addrB = nw_endpoint_get_address(b);

            VerifyOrReturnValue(addrA->sa_family == addrB->sa_family, false);

            uint16_t portA = nw_endpoint_get_port(a);
            uint16_t portB = nw_endpoint_get_port(b);

            VerifyOrReturnValue(portA == portB, false);

            if (addrA->sa_family == AF_INET) {
                const sockaddr_in * inA = reinterpret_cast<const sockaddr_in *>(addrA);
                const sockaddr_in * inB = reinterpret_cast<const sockaddr_in *>(addrB);
                return (inA->sin_addr.s_addr == inB->sin_addr.s_addr);
            }

            if (addrA->sa_family == AF_INET6) {
                const sockaddr_in6 * in6A = reinterpret_cast<const sockaddr_in6 *>(addrA);
                const sockaddr_in6 * in6B = reinterpret_cast<const sockaddr_in6 *>(addrB);
                return (memcmp(&in6A->sin6_addr, &in6B->sin6_addr, sizeof(in6A->sin6_addr)) == 0);
            }

            return false; // Unknown family
        }

        void UDPEndPointImplNetworkFrameworkListenerGroup::ReleaseInterfaceGroupCallback(CFAllocatorRef allocator, const void * value)
        {
            __auto_type * group = static_cast<InterfaceGroup *>(const_cast<void *>(value));
            free(group);
        }

    } // namespace Darwin
} // namespace Inet
} // namespace chip
