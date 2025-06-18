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

#include "UDPEndPointImplNetworkFrameworkListener.h"

#include "UDPEndPointImplNetworkFrameworkDebug.h"

constexpr uint64_t kInterfacesLookupTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kListenerReadyTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kListenerCancelledTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr const char * kInterfaceMonitorQueueName = "inet_interfaces_monitor";
constexpr const char * kListenersQueueName = "inet_listeners";

namespace chip {
namespace Inet {
    namespace Darwin {
        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::Configure(nw_parameters_t parameters, const IPAddressType addressType, const IPAddress & address, uint16_t port, InterfaceId interfaceId)
        {
            VerifyOrReturnError(nullptr == mListeners, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mListenerQueue, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(nullptr == mLocalParameters, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(IPAddress::Any == mLocalAddress, CHIP_ERROR_INCORRECT_STATE);
            VerifyOrReturnError(0 == mLocalPort, CHIP_ERROR_INCORRECT_STATE);

            __auto_type monitorQueue = dispatch_queue_create(kInterfaceMonitorQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            VerifyOrReturnError(nil != monitorQueue, CHIP_ERROR_NO_MEMORY);
            ReturnErrorOnFailure(NetworkMonitor::Init(monitorQueue, addressType, interfaceId));

            mListenerQueue = dispatch_queue_create(kListenersQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            VerifyOrReturnError(nullptr != mListenerQueue, CHIP_ERROR_NO_MEMORY);

            mLocalParameters = parameters;
            mLocalAddress = address;
            mLocalPort = port;

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::Listen()
        {
            VerifyOrReturnError(nullptr == mListeners, CHIP_ERROR_INCORRECT_STATE);

            mListeners = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
            VerifyOrReturnError(nullptr != mListeners, CHIP_ERROR_NO_MEMORY);

            VerifyOrReturnError(!IsLocalOnly(), ListenLoopback());
            VerifyOrReturnError(IsAnyAddress(), ListenAddress());
            return ListenInterfaces();
        }

        void UDPEndPointImplNetworkFrameworkListener::Unlisten()
        {
            NetworkMonitor::Stop();
            StopListeners();
            mListenerQueue = nullptr;
            mLocalParameters = nullptr;
            mLocalAddress = IPAddress::Any;
            mLocalPort = 0;

            if (mListeners) {
                CFRelease(mListeners);
                mListeners = nullptr;
            }
        }

        uint16_t UDPEndPointImplNetworkFrameworkListener::GetBoundPort() const
        {
            // If no listeners have been created yet, return the initially requested port.
            VerifyOrReturnValue(nullptr != mListeners, mLocalPort);
            VerifyOrReturnValue(CFArrayGetCount(mListeners) != 0, mLocalPort);

            // Otherwise, return the actual port assigned to the first listener.
            __auto_type listener = static_cast<nw_listener_t>(CFArrayGetValueAtIndex(mListeners, 0));
            return nw_listener_get_port(listener);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::ListenLoopback()
        {
#if INET_CONFIG_ENABLE_IPV4
            if (IsIPv4()) {
                return ListenAddress(IPAddress::Loopback(IPAddressType::kIPv4));
            }
#endif
            return ListenAddress(IPAddress::Loopback(IPAddressType::kIPv6));
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::ListenAddress()
        {
            return ListenAddress(mLocalAddress, GetInterfaceId());
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::ListenAddress(const IPAddress & address, InterfaceId intfId)
        {
            __auto_type parameters = nw_parameters_copy(mLocalParameters);
            VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_NO_MEMORY);

            __auto_type endpoint = GetEndPoint(address.Type(), address, GetBoundPort(), intfId);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

            nw_parameters_set_local_endpoint(parameters, endpoint);

            __auto_type listener = nw_listener_create(parameters);
            VerifyOrReturnError(nullptr != listener, CHIP_ERROR_INTERNAL);

            nw_listener_set_queue(listener, mListenerQueue);
            nw_listener_set_new_connection_handler(listener, ^(nw_connection_t connection) {
                StartConnectionFromListener(connection);
            });

            ReturnErrorOnFailure(WaitForListenerReadyState(listener));

            CFArrayAppendValue(mListeners, (__bridge const void *) listener);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::ListenAddress(const IPAddress & address, nw_interface_t interface)
        {
            __auto_type parameters = nw_parameters_copy(mLocalParameters);
            VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_NO_MEMORY);

            __auto_type intfId = static_cast<InterfaceId>(nw_interface_get_index(interface));
            __auto_type endpoint = GetEndPoint(address.Type(), address, GetBoundPort(), intfId);
            VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

            nw_parameters_set_local_endpoint(parameters, endpoint);
            nw_parameters_require_interface(parameters, interface);

            __auto_type listener = nw_listener_create(parameters);
            VerifyOrReturnError(nullptr != listener, CHIP_ERROR_INTERNAL);

            nw_listener_set_queue(listener, mListenerQueue);
            nw_listener_set_new_connection_handler(listener, ^(nw_connection_t connection) {
                StartConnectionFromListener(connection);
            });

            ReturnErrorOnFailure(WaitForListenerReadyState(listener));

            CFArrayAppendValue(mListeners, (__bridge const void *) listener);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::ListenInterfaces()
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(semaphore != nullptr, CHIP_ERROR_NO_MEMORY);

            CHIP_ERROR err = StartMonitorInterfaces(^(InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces) {
                StopListeners();
#if INET_CONFIG_ENABLE_IPV4
                ListenInterfaces(inetInterfaces);
#endif
                ListenInterfaces(inet6Interfaces);
                dispatch_semaphore_signal(semaphore);
            });
            VerifyOrReturnError(err == CHIP_NO_ERROR, err);

            // Wait until the first list of interfaces has been handled
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kInterfacesLookupTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)

            return CHIP_NO_ERROR;
        }

        void UDPEndPointImplNetworkFrameworkListener::StopListeners()
        {
            VerifyOrReturn(nullptr != mListeners);

            CFIndex count = CFArrayGetCount(mListeners);
            for (CFIndex i = 0; i < count; ++i) {
                __auto_type listener = static_cast<nw_listener_t>(CFArrayGetValueAtIndex(mListeners, i));
                LogErrorOnFailure(WaitForListenerCancelledState(listener));
            }

            CFArrayRemoveAllValues(mListeners);
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::WaitForListenerReadyState(nw_listener_t listener)
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

            __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
            nw_listener_set_state_changed_handler(listener, ^(nw_listener_state_t state, nw_error_t error) {
                DebugPrintListenerState(state, error);

                switch (state) {
                case nw_listener_state_invalid:
                    err = CHIP_ERROR_INCORRECT_STATE;
                    nw_listener_cancel(listener);
                    break;

                case nw_listener_state_waiting:
                    // Nothing to do.
                    break;

                case nw_listener_state_failed:
                    err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                    nw_listener_cancel(listener);
                    break;

                case nw_listener_state_ready:
                    err = CHIP_NO_ERROR;
                    dispatch_semaphore_signal(semaphore);
                    break;

                case nw_listener_state_cancelled:
                    if (err == CHIP_NO_ERROR) {
                        err = CHIP_ERROR_CONNECTION_ABORTED;
                    }

                    dispatch_semaphore_signal(semaphore);
                    break;
                }
            });

            nw_listener_start(listener);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kListenerReadyTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)
            nw_listener_set_state_changed_handler(listener, nullptr);

            return err;
        }

        CHIP_ERROR UDPEndPointImplNetworkFrameworkListener::WaitForListenerCancelledState(nw_listener_t listener)
        {
            __auto_type semaphore = dispatch_semaphore_create(0);
            VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

            nw_listener_set_state_changed_handler(listener, ^(nw_listener_state_t state, nw_error_t error) {
                DebugPrintListenerState(state, error);
                if (state == nw_listener_state_cancelled) {
                    dispatch_semaphore_signal(semaphore);
                }
            });

            nw_listener_cancel(listener);
            __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kListenerCancelledTimeoutInSeconds);
            dispatch_semaphore_wait(semaphore, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)

            return CHIP_NO_ERROR;
        }

    } // namespace Darwin
} // namespace Inet
} // namespace chip
