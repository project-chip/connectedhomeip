/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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

/**
 * This file implements Inet::UDPEndPoint using Network Framework.
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include "UDPEndPointImplNetworkFramework.h"

#include "UDPEndPointImplNetworkFrameworkDebug.h"

#include <lib/support/CHIPMemString.h>

#define INET_PORTSTRLEN 6

using namespace chip::Inet::Darwin;

constexpr uint64_t kSendTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr const char * kSendCleaningQueueName = "inet_send_cleaning";

namespace chip {
namespace Inet {

    CHIP_ERROR UDPEndPointImplNetworkFramework::BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port,
        InterfaceId intfId)
    {
        __auto_type configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
        __auto_type parameters = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);
        VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_INVALID_ARGUMENT, ReleaseAll());

        nw_parameters_set_reuse_local_address(parameters, true);

        // Note: The ConfigureProtocol function uses nw_ip_options_set_version to set the IP version for this endpoint.
        //
        // This works as expected when the IPAddress is specified. However, when using a wildcard address (chip::Inet::IPAddressType::kAny)
        //  for an IPv6 socket, the specified IP version (nw_ip_version_6) may be ignored, allowing both IPv4 and IPv6 connections.
        CHIP_ERROR err = ConfigureProtocol(addressType, parameters);
        VerifyOrReturnError(CHIP_NO_ERROR == err, err, ReleaseAll());

        mAddrType = addressType;
        mWorkFlagStrong = Platform::MakeShared<WorkFlag>();
        mWorkFlagWeak = mWorkFlagStrong;

        err = UDPEndPointImplNetworkFrameworkListener::Configure(parameters, addressType, address, port, intfId);
        VerifyOrReturnError(CHIP_NO_ERROR == err, err, ReleaseAll());

        err = UDPEndPointImplNetworkFrameworkListenerGroup::Configure(parameters, addressType, intfId);
        VerifyOrReturnError(CHIP_NO_ERROR == err, err, ReleaseAll());

        err = UDPEndPointImplNetworkFrameworkConnection::Configure(parameters, addressType);
        VerifyOrReturnError(CHIP_NO_ERROR == err, err, ReleaseAll());

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
    {
        return INET_ERROR_UNKNOWN_INTERFACE;
    }

    InterfaceId UDPEndPointImplNetworkFramework::GetBoundInterface() const
    {
        return InterfaceId::Null();
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ListenImpl()
    {
        CHIP_ERROR error = UDPEndPointImplNetworkFrameworkListener::Listen();
        VerifyOrDo(CHIP_NO_ERROR == error, ReleaseAll());
        return error;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
    {
        // Ensure we have an actual message to send.
        VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

        // For now the entire message must fit within a single buffer.
        VerifyOrReturnError(msg->Next() == nullptr, CHIP_ERROR_MESSAGE_TOO_LONG);

        __auto_type cleaningQueue = dispatch_queue_create(kSendCleaningQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        VerifyOrReturnError(nullptr != cleaningQueue, CHIP_ERROR_INCORRECT_STATE);

        nw_connection_t connection = RetrieveOrStartConnection(*pktInfo);
        VerifyOrReturnError(nullptr != connection, CHIP_ERROR_INCORRECT_STATE);
        HandleDataReceived(connection);

        // Wrap the PacketBufferHandle in a shared_ptr so we can capture it by value
        __auto_type retainedHandle = std::make_shared<System::PacketBufferHandle>(std::move(msg));
        __auto_type content = dispatch_data_create(
            retainedHandle->operator->()->Start(),
            retainedHandle->operator->()->DataLength(),
            cleaningQueue,
            ^{
                static_cast<System::LayerDispatch &>(GetSystemLayer()).ScheduleWorkWithBlock(^{
                    // Keep retainedHandle alive until the dispatch_data is released
                    [[maybe_unused]] auto cleanup = retainedHandle;
                });
            });

        // Send a message, and wait for it to be dispatched.
        __auto_type group = dispatch_group_create();
        dispatch_group_enter(group);

        // If there is a current message pending and the state of the network connection changes (e.g switch to a
        // different network) the connection will enter a nw_connection_state_failed state and the completion handler
        // will never be called. In such cases a signal is sent from the connection state change handler to release
        // the semaphore. In this case the CHIP_ERROR will not update with the result of the completion handler.
        // To make sure our caller knows that sending a message has failed the following code assumes there is an error
        // _unless_ the completion handler says otherwise.
        __block CHIP_ERROR err = CHIP_ERROR_UNEXPECTED_EVENT;
        nw_connection_send(connection, content, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t error) {
            if (error) {
                err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
            } else {
                err = CHIP_NO_ERROR;
            }
            dispatch_group_leave(group);
        });

        __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kSendTimeoutInSeconds);
        dispatch_group_wait(group, timeout); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)
        return err;
    }

    void UDPEndPointImplNetworkFramework::CloseImpl()
    {
        if (mWorkFlagStrong) {
            mWorkFlagStrong->MarkDead();
            mWorkFlagStrong.reset();
        }
        ReleaseAll();
    }

    void UDPEndPointImplNetworkFramework::ReleaseAll()
    {
        OnMessageReceived = nullptr;
        OnReceiveError = nullptr;

        UDPEndPointImplNetworkFrameworkConnection::StopAll();
        UDPEndPointImplNetworkFrameworkListener::Unlisten();
        UDPEndPointImplNetworkFrameworkListenerGroup::Unlisten();
    }

    void UDPEndPointImplNetworkFramework::Free()
    {
        Close();
        Release();
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        __auto_type protocolStack = nw_parameters_copy_default_protocol_stack(aParameters);
        __auto_type ipOptions = nw_protocol_stack_copy_internet_protocol(protocolStack);

        switch (aAddressType) {

        case IPAddressType::kIPv6:
            nw_ip_options_set_version(ipOptions, nw_ip_version_6);
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPAddressType::kIPv4:
            nw_ip_options_set_version(ipOptions, nw_ip_version_4);
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            err = INET_ERROR_WRONG_ADDRESS_TYPE;
            break;
        }

        return err;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo)
    {
        nw_path_t path = nw_connection_copy_current_path(aConnection);
        VerifyOrReturnError(nullptr != path, CHIP_ERROR_INVALID_ARGUMENT);
        nw_endpoint_t dest_endpoint = nw_path_copy_effective_local_endpoint(path);
        nw_endpoint_t src_endpoint = nw_path_copy_effective_remote_endpoint(path);

        aPacketInfo.Clear();

        // TODO Handle return value of IPAddress::GetIPAddressFromSockAdd
        const auto * srcAddress = nw_endpoint_get_address(src_endpoint);
        IPAddress::GetIPAddressFromSockAddr(*srcAddress, aPacketInfo.SrcAddress);

        // TODO Handle return value of IPAddress::GetIPAddressFromSockAdd
        const auto * dstAddress = nw_endpoint_get_address(dest_endpoint);
        IPAddress::GetIPAddressFromSockAddr(*dstAddress, aPacketInfo.DestAddress);

        aPacketInfo.SrcPort = nw_endpoint_get_port(src_endpoint);
        aPacketInfo.DestPort = nw_endpoint_get_port(dest_endpoint);

        DebugPrintPacketInfo(aPacketInfo);
        return CHIP_NO_ERROR;
    }

    nw_endpoint_t UDPEndPointImplNetworkFramework::GetEndPoint(const IPAddressType aAddressType,
        const IPAddress & aAddress, uint16_t aPort, InterfaceId interfaceIndex)
    {
        char addrStr[IPAddress::kMaxStringLength + 1 /*%*/ + InterfaceId::kMaxIfNameLength + 1 /*null terminator */];

        // Note: aAddress.ToString will return the IPv6 Any address if the address type is Any, but that's not what
        // we want if the local endpoint is IPv4.
#if INET_CONFIG_ENABLE_IPV4
        if (aAddressType == IPAddressType::kIPv4 && aAddress.Type() == IPAddressType::kAny) {
            const IPAddress anyAddr = IPAddress(aAddress.ToIPv4());
            anyAddr.ToString(addrStr);
        } else
#endif // INET_CONFIG_ENABLE_IPV4
        {
            aAddress.ToString(addrStr);
            if (interfaceIndex != InterfaceId::Null() && (aAddress.IsIPv6LinkLocal() || aAddress.IsIPv6Multicast())) {
                char interface[InterfaceId::kMaxIfNameLength + 1] = {}; // +1 to prepend '%'
                interface[0] = '%';
                interface[1] = 0;
                CHIP_ERROR err = interfaceIndex.GetInterfaceName(interface + 1, sizeof(interface) - 1);
                if (err != CHIP_NO_ERROR) {
                    Platform::CopyString(interface, sizeof(interface), "%(err)");
                }
                strncat(addrStr, interface, sizeof(addrStr) - strlen(addrStr) - 1);
            }
        }

        char portStr[INET_PORTSTRLEN];
        snprintf(portStr, sizeof(portStr), "%u", aPort);

        __auto_type endpoint = nw_endpoint_create_host(addrStr, portStr);
        DebugPrintEndPoint(endpoint);
        return endpoint;
    }

    void UDPEndPointImplNetworkFramework::StartConnectionFromListener(nw_connection_t connection)
    {
        static_cast<System::LayerDispatch &>(GetSystemLayer()).ScheduleWorkWithBlock(^{
            CHIP_ERROR error = StartConnection(connection);
            LogErrorOnFailure(error);

            if (CHIP_NO_ERROR == error) {
                HandleDataReceived(connection);
            }
        });
    }

    void UDPEndPointImplNetworkFramework::HandleDataReceived(nw_connection_t aConnection)
    {
        nw_connection_receive_completion_t handler = ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t receive_error) {
            dispatch_block_t schedule_next_receive = ^{
                if (receive_error == nullptr) {
                    HandleDataReceived(aConnection);
                } else if (OnReceiveError != nullptr) {
                    __auto_type error_domain = nw_error_get_error_domain(receive_error);
                    errno = nw_error_get_error_code(receive_error);
                    if (!(error_domain == nw_error_domain_posix && errno == ECANCELED)) {
                        CHIP_ERROR error = CHIP_ERROR_POSIX(errno);
                        IPPacketInfo packetInfo;
                        if (CHIP_NO_ERROR == GetPacketInfo(aConnection, packetInfo)) {
                            OnReceiveError((UDPEndPoint *) this, error, &packetInfo);
                        } else {
                            OnReceiveError((UDPEndPoint *) this, error, nullptr);
                        }
                    }
                }
            };

            auto localWeakFlag = mWorkFlagWeak;
            static_cast<System::LayerDispatch &>(GetSystemLayer()).ScheduleWorkWithBlock(^{
                auto workFlag = localWeakFlag.lock();
                if (!workFlag || !workFlag->IsAlive()) {
                    return;
                }

                if (content != nullptr && OnMessageReceived != nullptr) {
                    VerifyOrReturn(RefreshConnectionTimeout(aConnection));
                    size_t count = dispatch_data_get_size(content);

                    __auto_type packetBufferHandle = System::PacketBufferHandle::New(count);
                    __auto_type * packetBuffer = std::move(packetBufferHandle).UnsafeRelease();
                    dispatch_data_apply(content, ^(dispatch_data_t data, size_t offset, const void * buffer, size_t size) {
                        memmove(packetBuffer->Start() + offset, buffer, size);
                        return true;
                    });
                    packetBuffer->SetDataLength(count);

                    IPPacketInfo packetInfo;
                    GetPacketInfo(aConnection, packetInfo);
                    auto handle = System::PacketBufferHandle::Adopt(packetBuffer);
                    OnMessageReceived((UDPEndPoint *) this, std::move(handle), &packetInfo);
                }

                schedule_next_receive();
            });
        };

        nw_connection_receive_message(aConnection, handler);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::SetMulticastLoopback(IPVersion ipVersion, bool loopback)
    {
        return UDPEndPointImplNetworkFrameworkListenerGroup::SetMulticastLoopback(ipVersion, loopback);
    }

#if INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR UDPEndPointImplNetworkFramework::IPv4JoinLeaveMulticastGroupImpl(InterfaceId interfaceId, const IPAddress & address, bool join)
    {
        return UDPEndPointImplNetworkFrameworkListenerGroup::IPv4JoinLeaveMulticastGroup(interfaceId, address, join);
    }
#endif // INET_CONFIG_ENABLE_IPV4

    CHIP_ERROR UDPEndPointImplNetworkFramework::IPv6JoinLeaveMulticastGroupImpl(InterfaceId interfaceId, const IPAddress & address, bool join)
    {
        return UDPEndPointImplNetworkFrameworkListenerGroup::IPv6JoinLeaveMulticastGroup(interfaceId, address, join);
    }
} // namespace Inet
} // namespace chip
