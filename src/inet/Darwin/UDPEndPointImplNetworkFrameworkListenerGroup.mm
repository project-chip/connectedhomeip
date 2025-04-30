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

#include <inet/Darwin/UDPEndPointImplNetworkFrameworkDebug.h>

#include <lib/support/CodeUtils.h>

constexpr uint64_t kConnectionGroupReadyTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kConnectionGroupCancelledTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr const char * kConnectionGroupQueueName = "inet_connection_group";

namespace chip {
namespace Inet {
    namespace Darwin {
        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::Configure(nw_parameters_t parameters)
        {
            VerifyOrReturnError(nullptr == mConnectionGroup, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mCancelledSemaphore, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);

            mConnectionGroupQueue = dispatch_queue_create(kConnectionGroupQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            VerifyOrReturnError(nil != mConnectionGroupQueue, CHIP_ERROR_NO_MEMORY);

            mLocalParameters = parameters;

            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkListenerGroup::Unlisten()
        {
            StopListeners();
            mConnectionGroupQueue = nullptr;
            mLocalParameters = nullptr;
        }

#if INET_CONFIG_ENABLE_IPV4
        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::IPv4JoinLeaveMulticastGroup(InterfaceId interfaceId, const IPAddress & address, bool join)
        {
            VerifyOrReturnError(nullptr != mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr != mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);

            __auto_type endpoint = GetEndPoint(IPAddressType::kIPv4, address, GetBoundPort(), interfaceId);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

            return JoinLeaveMulticastGroup(endpoint, join);
        }
#endif // INET_CONFIG_ENABLE_IPV4

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::IPv6JoinLeaveMulticastGroup(InterfaceId interfaceId, const IPAddress & address, bool join)
        {
            VerifyOrReturnError(nullptr != mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr != mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);

            __auto_type endpoint = GetEndPoint(IPAddressType::kIPv6, address, GetBoundPort(), interfaceId);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

            return JoinLeaveMulticastGroup(endpoint, join);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::JoinLeaveMulticastGroup(nw_endpoint_t endpoint, bool join)
        {
            return (join ? JoinMulticastGroup(endpoint) : LeaveMulticastGroup(endpoint));
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::JoinMulticastGroup(nw_endpoint_t endpoint)
        {
            if (nullptr == mConnectionGroup) {
                __auto_type groupDescriptor = nw_group_descriptor_create_multicast(endpoint);
                VerifyOrReturnError(nullptr != groupDescriptor, CHIP_ERROR_INCORRECT_STATE);

                return StartListeners(groupDescriptor);
            }

            __auto_type oldGroupDescriptor = nw_connection_group_copy_descriptor(mConnectionGroup);
            StopListeners();

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

            return StartListeners(groupDescriptor);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::LeaveMulticastGroup(nw_endpoint_t endpoint)
        {
            VerifyOrReturnError(nullptr != mConnectionGroup, CHIP_ERROR_NOT_FOUND); // Nothing ever joined, can't leave!

            __auto_type oldGroupDescriptor = nw_connection_group_copy_descriptor(mConnectionGroup);
            StopListeners();

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

            VerifyOrReturnError(endpointCount, CHIP_NO_ERROR);

            return StartListeners(groupDescriptor);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::StartListeners(nw_group_descriptor_t groupDescriptor)
        {
            __auto_type parameters = nw_parameters_copy(mLocalParameters);
            VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_NO_MEMORY);
            nw_parameters_set_local_endpoint(parameters, nil);

            __auto_type connectionGroup = nw_connection_group_create(groupDescriptor, parameters);
            VerifyOrReturnError(nullptr != connectionGroup, CHIP_ERROR_INVALID_ARGUMENT);

            nw_connection_group_set_queue(connectionGroup, mConnectionGroupQueue);
            nw_connection_group_set_receive_handler(connectionGroup, UINT32_MAX, YES, ^(dispatch_data_t content, nw_content_context_t context, bool complete) {
                ChipLogError(Inet, "%s", __func__);
            });

            ReturnErrorOnFailure(WaitForConnectionGroupReadyState(connectionGroup));

            mConnectionGroup = connectionGroup;
            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkListenerGroup::StopListeners()
        {
            VerifyOrReturn(nullptr != mConnectionGroup);
            LogErrorOnFailure(WaitForConnectionGroupCancelledState(mConnectionGroup));
            mConnectionGroup = nullptr;
            mCancelledSemaphore = nullptr;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::WaitForConnectionGroupReadyState(nw_connection_group_t connectionGroup)
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

            mCancelledSemaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != mCancelledSemaphore, CHIP_ERROR_NO_MEMORY);

            __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
            nw_connection_group_set_state_changed_handler(connectionGroup, ^(nw_connection_group_state_t state, nw_error_t error) {
                DebugPrintConnectionGroupState(state, error);

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
                    break;
                case nw_connection_group_state_cancelled:
                    if (err == CHIP_NO_ERROR) {
                        err = CHIP_ERROR_CONNECTION_ABORTED;
                    }
                    dispatch_semaphore_signal(semaphore);
                    dispatch_semaphore_signal(mCancelledSemaphore);
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

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListenerGroup::WaitForConnectionGroupCancelledState(nw_connection_group_t connectionGroup)
        {
            nw_connection_group_cancel(connectionGroup);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionGroupCancelledTimeoutInSeconds);
            dispatch_semaphore_wait(mCancelledSemaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)

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

    } // namespace Darwin
} // namespace Inet
} // namespace chip
