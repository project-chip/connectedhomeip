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

#include "UDPEndPointImplNetworkFrameworkConnection.h"

#include "UDPEndPointImplNetworkFrameworkDebug.h"

#include <lib/support/CodeUtils.h>

constexpr uint64_t kConnectTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kConnectionCancelledTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kAllConnectionCancelledTimeoutInSeconds = 30 * NSEC_PER_SEC;
constexpr uint64_t kConnectionAliveTimeoutInSeconds = 60 * NSEC_PER_SEC;
constexpr uint64_t kSetInterfaceTimeoutInSeconds = 5 * NSEC_PER_SEC;
constexpr const char * kConnectionQueueName = "inet_connection";

namespace chip {
namespace Inet {
    namespace Darwin {
        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::Configure(nw_parameters_t parameters, IPAddressType addressType)
        {
            VerifyOrReturnError(nullptr == mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mConnectionQueue, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mConnections, CHIP_ERROR_INCORRECT_STATE);

            mConnectionQueue = dispatch_queue_create(kConnectionQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            VerifyOrReturnError(nil != mConnectionQueue, CHIP_ERROR_NO_MEMORY);

            mLocalParameters = parameters;
            mAddressType = addressType;
            PrepareConnections();

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::StartConnection(nw_connection_t connection)
        {
            if (HasConnection(connection)) {
                RefreshConnectionTimeout(connection);
                return CHIP_NO_ERROR;
            }

            ReturnErrorOnFailure(AddConnectionWrapper(connection));
            nw_connection_set_queue(connection, mConnectionQueue);

            CHIP_ERROR error = WaitForConnectionStateReady(connection);
            VerifyOrReturnError(CHIP_NO_ERROR == error, error, RemoveConnectionWrapper(connection));

            DebugPrintConnection(connection);
            return CHIP_NO_ERROR;
        }

        nw_connection_t UDPEndPointImplNetworkFrameworkConnection::RetrieveOrStartConnection(const IPPacketInfo & pktInfo)
        {
            __auto_type connection = FindConnection(pktInfo);

            if (!connection) {
                LogErrorOnFailure(GetConnection(pktInfo));
                connection = FindConnection(pktInfo);
            }

            return connection;
        }

        bool UDPEndPointImplNetworkFrameworkConnection::RefreshConnectionTimeout(nw_connection_t connection)
        {
            __auto_type wrapper = const_cast<ConnectionWrapper *>(static_cast<const ConnectionWrapper *>(CFDictionaryGetValue(mConnections, (__bridge const void *) connection)));
            VerifyOrReturnValue(nullptr != wrapper, false);

            wrapper->RefreshTimeout();
            return true;
        }

        void UDPEndPointImplNetworkFrameworkConnection::StopAll()
        {
            ReleaseConnections();

            mConnectionQueue = nullptr;
            mLocalParameters = nullptr;
            mAddressType = IPAddressType::kAny;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::GetConnection(const IPPacketInfo & pktInfo)
        {
            VerifyOrReturnError(nullptr != mLocalParameters, CHIP_ERROR_INCORRECT_STATE);

            // Ensure the destination address type is compatible with the endpoint address type.
            VerifyOrReturnError(mAddressType == pktInfo.DestAddress.Type() || mAddressType == IPAddressType::kAny, CHIP_ERROR_INVALID_ARGUMENT);

            __auto_type endpoint = GetEndPoint(mAddressType, pktInfo.DestAddress, pktInfo.DestPort, pktInfo.Interface);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INCORRECT_STATE);

            __auto_type parameters = nw_parameters_copy(mLocalParameters);
            VerifyOrReturnError(parameters != nullptr, CHIP_ERROR_NO_MEMORY);

            __auto_type localEndpoint = GetEndPoint(mAddressType, pktInfo.SrcAddress, pktInfo.SrcPort, pktInfo.Interface);
            nw_parameters_set_local_endpoint(parameters, localEndpoint);
            SetConnectionInterface(parameters, pktInfo.Interface);

            __auto_type connection = nw_connection_create(endpoint, parameters); // Let system pick ephemeral port
            VerifyOrReturnError(nullptr != connection, CHIP_ERROR_INCORRECT_STATE);

            return StartConnection(connection);
        }

        bool UDPEndPointImplNetworkFrameworkConnection::HasConnection(nw_connection_t connection)
        {
            return CFDictionaryContainsKey(mConnections, (__bridge const void *) connection);
        }

        nw_connection_t UDPEndPointImplNetworkFrameworkConnection::FindConnection(const IPPacketInfo & pktInfo)
        {
            CFIndex count = CFDictionaryGetCount(mConnections);
            __auto_type ** keys = static_cast<const void **>(malloc(sizeof(void *) * static_cast<size_t>(count)));
            CFDictionaryGetKeysAndValues(mConnections, keys, nullptr);

            nw_connection_t match = nullptr;
            for (CFIndex i = 0; i < count; ++i) {
                __auto_type * wrapper = const_cast<ConnectionWrapper *>(static_cast<const ConnectionWrapper *>(CFDictionaryGetValue(mConnections, keys[i])));
                if (wrapper && wrapper->Matches(pktInfo)) {
                    match = wrapper->mConnection;
                    break;
                }
            }

            free(keys);
            return match;
        }

        void UDPEndPointImplNetworkFrameworkConnection::PrepareConnections()
        {
            CFDictionaryValueCallBacks valueCallbacks = {
                0, // version
                nullptr, // retain callback
                ReleaseConnectionWrapperCallback,
                nullptr, // copyDescription callback
                nullptr // equal callback
            };

            mConnections = CFDictionaryCreateMutable(
                kCFAllocatorDefault,
                0,
                &kCFTypeDictionaryKeyCallBacks,
                &valueCallbacks);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::AddConnectionWrapper(nw_connection_t connection)
        {
            __auto_type onTimeout = ^{
                ChipLogDetail(Inet, "Connection: Timeout");
                nw_connection_cancel(connection);
                RemoveConnectionWrapper(connection);
            };
            __auto_type wrapper = new ConnectionWrapper(connection, mConnectionQueue, onTimeout);
            VerifyOrReturnError(nullptr != wrapper, CHIP_ERROR_NO_MEMORY);

            CFDictionarySetValue(mConnections, (__bridge const void *) connection, wrapper);
            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkConnection::RemoveConnectionWrapper(nw_connection_t connection)
        {
            CFDictionaryRemoveValue(mConnections, (__bridge const void *) connection);
        }

        void UDPEndPointImplNetworkFrameworkConnection::ReleaseConnections()
        {
            VerifyOrReturn(nullptr != mConnectionQueue);
            VerifyOrReturn(nullptr != mConnections);

            WaitForAllConnectionStateCancelled();

            CFRelease(mConnections);
            mConnections = nullptr;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::WaitForConnectionStateReady(nw_connection_t connection)
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

            __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
            nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
                DebugPrintConnectionState(state, error);

                switch (state) {
                case nw_connection_state_invalid:
                case nw_connection_state_waiting:
                    err = CHIP_ERROR_INCORRECT_STATE;
                    nw_connection_cancel(connection);
                    break;

                case nw_connection_state_preparing:
                    // Nothing to do.
                    break;

                case nw_connection_state_failed:
                    err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                    break;

                case nw_connection_state_ready:
                    err = CHIP_NO_ERROR;
                    dispatch_semaphore_signal(semaphore);
                    break;

                case nw_connection_state_cancelled:
                    if (err == CHIP_NO_ERROR) {
                        err = CHIP_ERROR_CONNECTION_ABORTED;
                    }
                    dispatch_semaphore_signal(semaphore);
                    break;
                }
            });

            nw_connection_start(connection);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)
            nw_connection_set_state_changed_handler(connection, nullptr);

            return err;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::WaitForConnectionStateCancelled(nw_connection_t connection)
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

            nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
                DebugPrintConnectionState(state, error);

                if (state == nw_connection_state_cancelled) {
                    dispatch_semaphore_signal(semaphore);
                }
            });

            nw_connection_cancel(connection);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionCancelledTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)
            nw_connection_set_state_changed_handler(connection, nullptr);

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkConnection::WaitForAllConnectionStateCancelled()
        {
            CFIndex count = CFDictionaryGetCount(mConnections);
            const void ** keys = static_cast<const void **>(malloc(sizeof(void *) * static_cast<size_t>(count)));
            CFDictionaryGetKeysAndValues(mConnections, keys, nullptr);

            __auto_type group = dispatch_group_create();
            VerifyOrReturnError(nullptr != group, CHIP_ERROR_NO_MEMORY);

            for (CFIndex i = 0; i < count; ++i) {
                __auto_type connection = (__bridge nw_connection_t)(const_cast<void *>(keys[i]));

                dispatch_group_enter(group);
                nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
                    DebugPrintConnectionState(state, error);

                    if (state == nw_connection_state_cancelled) {
                        dispatch_group_leave(group);
                    }
                });

                nw_connection_cancel(connection);
            }
            free(keys);

            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kAllConnectionCancelledTimeoutInSeconds);
            dispatch_group_wait(group, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)

            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkConnection::SetConnectionInterface(nw_parameters_t parameters, InterfaceId interfaceId)
        {
            VerifyOrReturn(InterfaceId::Null() != interfaceId);

            __auto_type workQueue = dispatch_queue_create(kConnectionQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            __auto_type monitor = nw_path_monitor_create();
            nw_path_monitor_set_queue(monitor, workQueue);

            __auto_type semaphore = dispatch_semaphore_create(0);

            uint32_t interfaceIndex = interfaceId.GetPlatformInterface();
            nw_path_monitor_set_update_handler(monitor, ^(nw_path_t path) {
                nw_path_enumerate_interfaces(path, ^(nw_interface_t interface) {
                    if (interfaceIndex == nw_interface_get_index(interface)) {
                        nw_parameters_require_interface(parameters, interface);
                        return false;
                    }
                    return true;
                });
                nw_path_monitor_cancel(monitor);
                dispatch_semaphore_signal(semaphore);
            });

            nw_path_monitor_start(monitor);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kSetInterfaceTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)
        }

        UDPEndPointImplNetworkFrameworkConnection::ConnectionWrapper::ConnectionWrapper(nw_connection_t connection, dispatch_queue_t timeoutQueue, dispatch_block_t onTimeout)
            : mConnection(connection)
            , mTimeoutQueue(timeoutQueue)
            , mTimeoutBlock(onTimeout)
        {
            RefreshTimeout();
        }

        bool UDPEndPointImplNetworkFrameworkConnection::ConnectionWrapper::Matches(const IPPacketInfo & pktInfo) const
        {
            __auto_type path = nw_connection_copy_current_path(mConnection);
            VerifyOrReturnValue(nullptr != path, false);

            __auto_type remoteEndpoint = nw_path_copy_effective_remote_endpoint(path);
            VerifyOrReturnValue(nullptr != remoteEndpoint, false);

            IPAddress remoteAddress;
            IPAddress::GetIPAddressFromSockAddr(*nw_endpoint_get_address(remoteEndpoint), remoteAddress);
            VerifyOrReturnValue(pktInfo.DestAddress == remoteAddress, false);

            const uint16_t remotePort = nw_endpoint_get_port(remoteEndpoint);
            VerifyOrReturnValue(pktInfo.DestPort == remotePort, false);

            if (pktInfo.Interface != InterfaceId::Null()) {
                uint32_t interfaceIndex = 0;
                __auto_type * sa = nw_endpoint_get_address(remoteEndpoint);
                if (sa && sa->sa_family == AF_INET6) {
                    __auto_type in6 = reinterpret_cast<const struct sockaddr_in6 *>(sa);
                    interfaceIndex = static_cast<uint32_t>(in6->sin6_scope_id);
                    VerifyOrReturnValue(interfaceIndex == pktInfo.Interface.GetPlatformInterface(), false);
                }
            }

            return true;
        }

        void UDPEndPointImplNetworkFrameworkConnection::ConnectionWrapper::RefreshTimeout()
        {
            mTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, mTimeoutQueue);
            dispatch_source_set_event_handler(mTimer, mTimeoutBlock);

            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionAliveTimeoutInSeconds);
            uint64_t interval = DISPATCH_TIME_FOREVER;
            uint64_t leeway = 1 * NSEC_PER_SEC;
            dispatch_source_set_timer(mTimer, timeout, interval, leeway);

            dispatch_resume(mTimer);
        }

        void UDPEndPointImplNetworkFrameworkConnection::ReleaseConnectionWrapperCallback(CFAllocatorRef allocator, const void * value)
        {
            __auto_type * wrapper = static_cast<ConnectionWrapper *>(const_cast<void *>(value));
            if (wrapper->mTimer) {
                dispatch_source_cancel(wrapper->mTimer);
                wrapper->mTimer = nullptr;
            }
            wrapper->mConnection = nullptr;
            wrapper->mTimeoutQueue = nullptr;
            wrapper->mTimeoutBlock = nullptr;
            delete wrapper;
        }

    } // namespace Darwin
} // namespace Inet
} // namespace chip
