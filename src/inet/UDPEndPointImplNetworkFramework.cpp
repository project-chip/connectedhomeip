/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#define __APPLE_USE_RFC_3542
#include <inet/UDPEndPoint.h>
#include <inet/UDPEndPointImplNetworkFramework.h>

#include <inet/IPPacketInfo.h>
#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <cstring>
#include <utility>

#define INET_PORTSTRLEN 6

namespace chip {
namespace Inet {

CHIP_ERROR UDPEndPointImplNetworkFramework::BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port,
                                                     InterfaceId intfId)
{
    nw_parameters_configure_protocol_block_t configure_tls;
    nw_parameters_t parameters;

    if (intfId.IsPresent())
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
    parameters    = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);
    VerifyOrReturnError(parameters != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(ConfigureProtocol(addressType, parameters));

    nw_endpoint_t endpoint = nullptr;
    CHIP_ERROR res         = GetEndPoint(endpoint, addressType, address, port);
    nw_parameters_set_local_endpoint(parameters, endpoint);
    nw_release(endpoint);
    ReturnErrorOnFailure(res);

    mDispatchQueue = dispatch_queue_create("inet_dispatch_global", DISPATCH_QUEUE_CONCURRENT);
    VerifyOrReturnError(mDispatchQueue != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mDispatchQueue);

    mConnectionSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mConnectionSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mConnectionSemaphore);

    mSendSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mSendSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mSendSemaphore);

    mAddrType   = addressType;
    mConnection = nullptr;
    mParameters = parameters;

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

uint16_t UDPEndPointImplNetworkFramework::GetBoundPort() const
{
    nw_endpoint_t endpoint = nw_parameters_copy_local_endpoint(mParameters);
    return nw_endpoint_get_port(endpoint);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ListenImpl()
{
    return StartListener();
}

CHIP_ERROR UDPEndPointImplNetworkFramework::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    dispatch_data_t content;

    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrReturnError(mAddrType == pktInfo->DestAddress.Type(), CHIP_ERROR_INVALID_ARGUMENT);

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(msg->Next() == nullptr, CHIP_ERROR_MESSAGE_TOO_LONG);

    ReturnErrorOnFailure(GetConnection(pktInfo));

    // Send a message, and wait for it to be dispatched.
    content = dispatch_data_create(msg->Start(), msg->DataLength(), mDispatchQueue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    // If there is a current message pending and the state of the network connection change (e.g switch to a
    // different network) the connection will enter a nw_connection_state_failed state and the completion handler
    // will never be called. In such cases a signal is sent from the connection state change handler to release
    // the semaphore. In this case the CHIP_ERROR will not update with the result of the completion handler.
    // To make sure caller knows that sending a message has failed the following code consider there is an error
    // _unless_ the completion handler says otherwise.
    __block CHIP_ERROR res = CHIP_ERROR_UNEXPECTED_EVENT;
    nw_connection_send(mConnection, content, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t error) {
        if (error)
        {
            res = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
        }
        else
        {
            res = CHIP_NO_ERROR;
        }
        dispatch_semaphore_signal(mSendSemaphore);
    });
    dispatch_release(content);

    dispatch_semaphore_wait(mSendSemaphore, DISPATCH_TIME_FOREVER);

    return res;
}

void UDPEndPointImplNetworkFramework::CloseImpl()
{
    ReleaseAll();
}

void UDPEndPointImplNetworkFramework::ReleaseAll()
{

    OnMessageReceived = nullptr;
    OnReceiveError    = nullptr;

    ReleaseConnection();
    ReleaseListener();

    if (mParameters)
    {
        nw_release(mParameters);
        mParameters = nullptr;
    }

    if (mDispatchQueue)
    {
        dispatch_suspend(mDispatchQueue);
        dispatch_release(mDispatchQueue);
        mDispatchQueue = nullptr;
    }

    if (mConnectionSemaphore)
    {
        dispatch_release(mConnectionSemaphore);
        mConnectionSemaphore = nullptr;
    }

    if (mListenerQueue)
    {
        dispatch_suspend(mListenerQueue);
        dispatch_release(mListenerQueue);
        mListenerQueue = nullptr;
    }

    if (mListenerSemaphore)
    {
        dispatch_release(mListenerSemaphore);
        mListenerSemaphore = nullptr;
    }

    if (mSendSemaphore)
    {
        dispatch_release(mSendSemaphore);
        mSendSemaphore = nullptr;
    }
}

void UDPEndPointImplNetworkFramework::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPointImplNetworkFramework::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR UDPEndPointImplNetworkFramework::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                                            bool join)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplNetworkFramework::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                                            bool join)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    nw_protocol_stack_t protocolStack = nw_parameters_copy_default_protocol_stack(aParameters);
    nw_protocol_options_t ipOptions   = nw_protocol_stack_copy_internet_protocol(protocolStack);

    switch (aAddressType)
    {

    case IPAddressType::kIPv6:
        nw_ip_options_set_version(ipOptions, nw_ip_version_6);
        break;

#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        nw_ip_options_set_version(ipOptions, nw_ip_version_4);
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    default:
        res = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }
    nw_release(ipOptions);
    nw_release(protocolStack);

    return res;
}

void UDPEndPointImplNetworkFramework::GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo)
{
    nw_path_t path              = nw_connection_copy_current_path(aConnection);
    nw_endpoint_t dest_endpoint = nw_path_copy_effective_local_endpoint(path);
    nw_endpoint_t src_endpoint  = nw_path_copy_effective_remote_endpoint(path);

    aPacketInfo.Clear();
    aPacketInfo.SrcAddress  = IPAddress::FromSockAddr(*nw_endpoint_get_address(src_endpoint));
    aPacketInfo.DestAddress = IPAddress::FromSockAddr(*nw_endpoint_get_address(dest_endpoint));
    aPacketInfo.SrcPort     = nw_endpoint_get_port(src_endpoint);
    aPacketInfo.DestPort    = nw_endpoint_get_port(dest_endpoint);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::GetEndPoint(nw_endpoint_t & aEndPoint, const IPAddressType aAddressType,
                                                        const IPAddress & aAddress, uint16_t aPort)
{
    char addrStr[INET6_ADDRSTRLEN];
    char portStr[INET_PORTSTRLEN];

    // Note: aAddress.ToString will return the IPv6 Any address if the address type is Any, but that's not what
    // we want if the locale endpoint is IPv4.
    if (aAddressType == IPAddressType::kIPv4 && aAddress.Type() == IPAddressType::kAny)
    {
        const IPAddress anyAddr = IPAddress(aAddress.ToIPv4());
        anyAddr.ToString(addrStr, sizeof(addrStr));
    }
    else
    {
        aAddress.ToString(addrStr, sizeof(addrStr));
    }

    snprintf(portStr, sizeof(portStr), "%u", aPort);

    aEndPoint = nw_endpoint_create_host(addrStr, portStr);
    VerifyOrReturnError(aEndPoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::GetConnection(const IPPacketInfo * aPktInfo)
{
    VerifyOrReturnError(mParameters != nullptr, CHIP_ERROR_INCORRECT_STATE);

    nw_endpoint_t endpoint     = nullptr;
    nw_connection_t connection = nullptr;

    if (mConnection)
    {
        nw_path_t path                 = nw_connection_copy_current_path(mConnection);
        nw_endpoint_t remote_endpoint  = nw_path_copy_effective_remote_endpoint(path);
        const IPAddress remote_address = IPAddress::FromSockAddr(*nw_endpoint_get_address(remote_endpoint));
        const uint16_t remote_port     = nw_endpoint_get_port(remote_endpoint);
        const bool isDifferentEndPoint = aPktInfo->DestPort != remote_port || aPktInfo->DestAddress != remote_address;
        VerifyOrReturnError(isDifferentEndPoint, CHIP_NO_ERROR);

        ReturnErrorOnFailure(ReleaseConnection());
    }

    ReturnErrorOnFailure(GetEndPoint(endpoint, mAddrType, aPktInfo->DestAddress, aPktInfo->DestPort));

    connection = nw_connection_create(endpoint, mParameters);
    nw_release(endpoint);

    VerifyOrReturnError(connection != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return StartConnection(connection);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::StartListener()
{
    __block CHIP_ERROR res = CHIP_NO_ERROR;
    nw_listener_t listener;

    VerifyOrReturnError(mListener == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mListenerSemaphore == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mListenerQueue == nullptr, CHIP_ERROR_INCORRECT_STATE);

    listener = nw_listener_create(mParameters);
    VerifyOrReturnError(listener != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mListenerSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mListenerSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mListenerSemaphore);

    mListenerQueue = dispatch_queue_create("inet_dispatch_listener", DISPATCH_QUEUE_CONCURRENT);
    VerifyOrReturnError(mListenerQueue != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mListenerQueue);

    nw_listener_set_queue(listener, mListenerQueue);

    nw_listener_set_new_connection_handler(listener, ^(nw_connection_t connection) {
        ReleaseConnection();
        StartConnection(connection);
    });

    nw_listener_set_state_changed_handler(listener, ^(nw_listener_state_t state, nw_error_t error) {
        switch (state)
        {

        case nw_listener_state_invalid:
            ChipLogDetail(Inet, "Listener: Invalid");
            res = CHIP_ERROR_INCORRECT_STATE;
            nw_listener_cancel(listener);
            break;

        case nw_listener_state_waiting:
            ChipLogDetail(Inet, "Listener: Waiting");
            break;

        case nw_listener_state_failed:
            ChipLogDetail(Inet, "Listener: Failed");
            res = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
            break;

        case nw_listener_state_ready:
            ChipLogDetail(Inet, "Listener: Ready");
            res = CHIP_NO_ERROR;
            dispatch_semaphore_signal(mListenerSemaphore);
            break;

        case nw_listener_state_cancelled:
            ChipLogDetail(Inet, "Listener: Cancelled");
            if (res == CHIP_NO_ERROR)
            {
                res = CHIP_ERROR_CONNECTION_ABORTED;
            }

            dispatch_semaphore_signal(mListenerSemaphore);
            break;
        }
    });

    nw_listener_start(listener);
    dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
    ReturnErrorOnFailure(res);

    mListener = listener;
    nw_retain(mListener);
    return res;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::StartConnection(nw_connection_t & aConnection)
{
    __block CHIP_ERROR res = CHIP_NO_ERROR;

    nw_connection_set_queue(aConnection, mDispatchQueue);

    nw_connection_set_state_changed_handler(aConnection, ^(nw_connection_state_t state, nw_error_t error) {
        switch (state)
        {

        case nw_connection_state_invalid:
            ChipLogDetail(Inet, "Connection: Invalid");
            res = CHIP_ERROR_INCORRECT_STATE;
            nw_connection_cancel(aConnection);
            break;

        case nw_connection_state_preparing:
            ChipLogDetail(Inet, "Connection: Preparing");
            res = CHIP_ERROR_INCORRECT_STATE;
            break;

        case nw_connection_state_waiting:
            ChipLogDetail(Inet, "Connection: Waiting");
            nw_connection_cancel(aConnection);
            break;

        case nw_connection_state_failed:
            ChipLogDetail(Inet, "Connection: Failed");
            res = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
            break;

        case nw_connection_state_ready:
            ChipLogDetail(Inet, "Connection: Ready");
            res = CHIP_NO_ERROR;
            dispatch_semaphore_signal(mConnectionSemaphore);
            break;

        case nw_connection_state_cancelled:
            ChipLogDetail(Inet, "Connection: Cancelled");
            if (res == CHIP_NO_ERROR)
            {
                res = CHIP_ERROR_CONNECTION_ABORTED;
            }

            dispatch_semaphore_signal(mConnectionSemaphore);
            break;
        }
    });

    nw_connection_start(aConnection);
    dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
    SuccessOrExit(res);

    mConnection = aConnection;
    nw_retain(mConnection);
    HandleDataReceived(mConnection);

    return res;
}

void UDPEndPointImplNetworkFramework::HandleDataReceived(const nw_connection_t & aConnection)
{
    nw_connection_receive_completion_t handler =
        ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t receive_error) {
            dispatch_block_t schedule_next_receive = ^{
                if (receive_error == nullptr)
                {
                    HandleDataReceived(aConnection);
                }
                else if (OnReceiveError != nullptr)
                {
                    nw_error_domain_t error_domain = nw_error_get_error_domain(receive_error);
                    errno                          = nw_error_get_error_code(receive_error);
                    if (!(error_domain == nw_error_domain_posix && errno == ECANCELED))
                    {
                        CHIP_ERROR error = CHIP_ERROR_POSIX(errno);
                        IPPacketInfo packetInfo;
                        GetPacketInfo(aConnection, packetInfo);
                        dispatch_async(mDispatchQueue, ^{
                            OnReceiveError((UDPEndPoint *) this, error, &packetInfo);
                        });
                    }
                }
            };

            if (content != nullptr && OnMessageReceived != nullptr)
            {
                size_t count                              = dispatch_data_get_size(content);
                System::PacketBufferHandle * packetBuffer = System::PacketBufferHandle::New(count);
                dispatch_data_apply(content, ^(dispatch_data_t data, size_t offset, const void * buffer, size_t size) {
                    memmove(packetBuffer->Start() + offset, buffer, size);
                    return true;
                });
                packetBuffer->SetDataLength(count);

                IPPacketInfo packetInfo;
                GetPacketInfo(aConnection, packetInfo);
                dispatch_async(mDispatchQueue, ^{
                    OnMessageReceived((UDPEndPoint *) this, packetBuffer, &packetInfo);
                });
            }

            schedule_next_receive();
        };

    nw_connection_receive_message(aConnection, handler);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseListener()
{
    VerifyOrReturnError(mListener, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDispatchQueue, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

    nw_listener_cancel(mListener);
    dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
    nw_release(mListener);
    mListener = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseConnection()
{
    VerifyOrReturnError(mConnection, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDispatchQueue, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

    nw_connection_cancel(mConnection);
    dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
    nw_release(mConnection);
    mConnection = nullptr;

    return CHIP_NO_ERROR;
}

} // namespace Inet
} // namespace chip
