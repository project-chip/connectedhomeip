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

#include <inet/UDPEndPoint.h>
#include <inet/UDPEndPointImplNetworkFramework.h>

#include <lib/support/CHIPMemString.h>

#define INET_PORTSTRLEN 6

#define NETWORK_FRAMEWORK_DEBUG 0

namespace {
#if !NETWORK_FRAMEWORK_DEBUG
void DebugPrintListenerState(nw_listener_state_t state) {};
void DebugPrintConnectionState(nw_connection_state_t state) {};
void DebugPrintConnection(const nw_connection_t aConnection) {};
#else
constexpr const char * kNilConnection = "The connection is nil.";
constexpr const char * kNilPath = "The connection path is nil.";
constexpr const char * kNilPathSourceEndPoint = "The connection path source endpoint is nil.";
constexpr const char * kNilPathDestinationEndPoint = "The connection path destination endpoint is nil.";
constexpr const char * kPathStatusInvalid = "This path is not valid.";
constexpr const char * kPathStatusUnsatisfied = "The path is not available for use.";
constexpr const char * kPathStatusSatisfied = "The path is available to establish connections and send data.";
constexpr const char * kPathStatusSatisfiable = "The path is not currently available, but establishing a new connection may activate the path.";

constexpr const char * kListenerStateInvalid = "Listener: Invalid";
constexpr const char * kListenerStateWaiting = "Listener: Waiting";
constexpr const char * kListenerStateFailed = "Listener: Failed";
constexpr const char * kListenerStateReady = "Listener: Ready";
constexpr const char * kListenerStateCancelled = "Listener: Cancelled";

constexpr const char * kConnectionStateInvalid = "Connection: Invalid";
constexpr const char * kConnectionStateWaiting = "Connection: Waiting";
constexpr const char * kConnectionStatePreparing = "Connection: Preparing";
constexpr const char * kConnectionStateFailed = "Connection: Failed";
constexpr const char * kConnectionStateReady = "Connection: Ready";
constexpr const char * kConnectionStateCancelled = "Connection: Cancelled";

void DebugPrintConnectionState(nw_connection_state_t state)
{
    const char * str = nullptr;

    switch (state) {
    case nw_connection_state_invalid:
        str = kConnectionStateInvalid;
        break;
    case nw_connection_state_preparing:
        str = kConnectionStatePreparing;
        break;
    case nw_connection_state_waiting:
        str = kConnectionStateWaiting;
        break;
    case nw_connection_state_failed:
        str = kConnectionStateFailed;
        break;
    case nw_connection_state_ready:
        str = kConnectionStateReady;
        break;
    case nw_connection_state_cancelled:
        str = kConnectionStateCancelled;
        break;
    default:
        chipDie();
    }

    ChipLogDetail(Inet, "%s", str);
}

void DebugPrintListenerState(nw_listener_state_t state)
{
    const char * str = nullptr;

    switch (state) {
    case nw_listener_state_invalid:
        str = kListenerStateInvalid;
        break;
    case nw_listener_state_waiting:
        str = kListenerStateWaiting;
        break;
    case nw_listener_state_failed:
        str = kListenerStateFailed;
        break;
    case nw_listener_state_ready:
        str = kListenerStateReady;
        break;
    case nw_listener_state_cancelled:
        str = kListenerStateCancelled;
        break;
    default:
        chipDie();
    }

    ChipLogDetail(Inet, "%s", str);
}

void DebugPrintConnectionPathStatus(nw_path_t path)
{
    const char * str = nullptr;

    __auto_type status = nw_path_get_status(path);
    switch (status) {
    case nw_path_status_invalid:
        str = kPathStatusInvalid;
        break;
    case nw_path_status_unsatisfied:
        str = kPathStatusUnsatisfied;
        break;
    case nw_path_status_satisfied:
        str = kPathStatusSatisfied;
        break;
    case nw_path_status_satisfiable:
        str = kPathStatusSatisfiable;
        break;
    default:
        chipDie();
    }

    ChipLogError(Inet, "%s", str);
}

void DebugPrintConnection(const nw_connection_t aConnection)
{
    VerifyOrReturn(nil != aConnection, ChipLogError(Inet, "%s", kNilConnection));

    __auto_type path = nw_connection_copy_current_path(aConnection);
    VerifyOrReturn(nil != path, ChipLogError(Inet, "%s", kNilPath));
    DebugPrintConnectionPathStatus(path);

    __auto_type srcEndPoint = nw_path_copy_effective_local_endpoint(path);
    VerifyOrReturn(nil != srcEndPoint, ChipLogError(Inet, "%s", kNilPathSourceEndPoint));

    __auto_type dstEndPoint = nw_path_copy_effective_remote_endpoint(path);
    VerifyOrReturn(nil != dstEndPoint, ChipLogError(Inet, "%s", kNilPathDestinationEndPoint));

    const __auto_type * srcAddress = nw_endpoint_copy_address_string(srcEndPoint);
    const __auto_type srcPort = nw_endpoint_get_port(srcEndPoint);
    const __auto_type * dstAddress = nw_endpoint_copy_address_string(dstEndPoint);
    const __auto_type dstPort = nw_endpoint_get_port(dstEndPoint);

    ChipLogError(Inet, "Connection source: %s:%u destination: %s:%u", srcAddress, srcPort, dstAddress, dstPort);
}
#endif
}

namespace chip {
namespace Inet {

    CHIP_ERROR UDPEndPointImplNetworkFramework::BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port,
        InterfaceId intfId)
    {
#if NETWORK_FRAMEWORK_DEBUG
        ChipLogError(Inet, "%s (%p)", __func__, this);
#endif

        VerifyOrReturnError(!intfId.IsPresent(), CHIP_ERROR_NOT_IMPLEMENTED);

        __auto_type configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
        __auto_type parameters = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);
        VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_INVALID_ARGUMENT);

        // Note: The ConfigureProtocol function uses nw_ip_options_set_version to set the IP version for this endpoint.
        //
        // This works as expected when the IPAddress is specified. However, when using a wildcard address (chip::Inet::IPAddressType::kAny)
        //  for an IPv6 socket, the specified IP version (nw_ip_version_6) may be ignored, allowing both IPv4 and IPv6 connections.
        ReturnErrorOnFailure(ConfigureProtocol(addressType, parameters));

        // Note: Network.framework does not provide an API to set the SO_REUSEPORT socket option.
        // This limitation is not an issue when the port is set to 0, as the platform will choose a random port.
        //
        // However, when both INET_CONFIG_ENABLE_IPV4 and CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY are enabled,
        // the system attempts to create two endpoints—one over IPv6 and another over IPv4—both using the same port
        // specified by CHIP_UDC_PORT.
        //
        // This results in a binding failure due to "Address already in use" since both IPv4 and IPv6 endpoints
        // try to use the same port.
        //
        // A potential solution would be to define separate ports for IPv4 and IPv6 (e.g., CHIP_UDC_PORT_IPv4 and CHIP_UDC_PORT_IPv6).
        // For now, as a workaround, we set the port to 0 for IPv4 when a specific port is needed, allowing the platform to
        // auto-assign an available port and avoid the conflict.
        if (IPAddressType::kIPv4 == addressType && port != 0) {
            port = 0;
        }

        __auto_type endpoint = GetEndPoint(addressType, address, port);
        VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INTERNAL);
        nw_parameters_set_local_endpoint(parameters, endpoint);

        mConnectionQueue = dispatch_queue_create("inet_dispatch_global", DISPATCH_QUEUE_SERIAL);
        VerifyOrReturnError(nullptr != mConnectionQueue, CHIP_ERROR_NO_MEMORY);

        mConnectionSemaphore = dispatch_semaphore_create(0);
        VerifyOrReturnError(nullptr != mConnectionSemaphore, CHIP_ERROR_NO_MEMORY);

        mSendSemaphore = dispatch_semaphore_create(0);
        VerifyOrReturnError(nullptr != mSendSemaphore, CHIP_ERROR_NO_MEMORY);

        mSystemQueue = static_cast<System::LayerSocketsLoop &>(GetSystemLayer()).GetDispatchQueue();
        mAddrType = addressType;
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
        __auto_type endpoint = nw_parameters_copy_local_endpoint(mParameters);
        return nw_endpoint_get_port(endpoint);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ListenImpl()
    {
        return StartListener();
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
    {
#if NETWORK_FRAMEWORK_DEBUG
        ChipLogError(Inet, "%s (%p)", __func__, this);
#endif

        // Ensure we have an actual message to send.
        VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

        // Ensure the destination address type is compatible with the endpoint address type.
        VerifyOrReturnError(mAddrType == pktInfo->DestAddress.Type(), CHIP_ERROR_INVALID_ARGUMENT);

        // For now the entire message must fit within a single buffer.
        VerifyOrReturnError(msg->Next() == nullptr, CHIP_ERROR_MESSAGE_TOO_LONG);

        ReturnErrorOnFailure(GetConnection(pktInfo));

        // Send a message, and wait for it to be dispatched.
        __auto_type content = dispatch_data_create(msg->Start(), msg->DataLength(), mSystemQueue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

        // If there is a current message pending and the state of the network connection changes (e.g switch to a
        // different network) the connection will enter a nw_connection_state_failed state and the completion handler
        // will never be called. In such cases a signal is sent from the connection state change handler to release
        // the semaphore. In this case the CHIP_ERROR will not update with the result of the completion handler.
        // To make sure our caller knows that sending a message has failed the following code assumes there is an error
        // _unless_ the completion handler says otherwise.
        __block CHIP_ERROR err = CHIP_ERROR_UNEXPECTED_EVENT;
        nw_connection_send(mConnection, content, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t error) {
            if (error) {
                err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
            } else {
                err = CHIP_NO_ERROR;
            }
            dispatch_semaphore_signal(mSendSemaphore);
        });

        dispatch_semaphore_wait(mSendSemaphore, DISPATCH_TIME_FOREVER);

        return err;
    }

    void UDPEndPointImplNetworkFramework::CloseImpl()
    {
        ReleaseAll();
    }

    void UDPEndPointImplNetworkFramework::ReleaseAll()
    {

        OnMessageReceived = nullptr;
        OnReceiveError = nullptr;

        ReleaseConnection();
        ReleaseListener();

        mParameters = nullptr;

        mConnectionQueue = nullptr;
        mConnectionSemaphore = nullptr;

        mListenerQueue = nullptr;
        mListenerSemaphore = nullptr;

        mSendSemaphore = nullptr;
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

    void UDPEndPointImplNetworkFramework::GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo)
    {
        nw_path_t path = nw_connection_copy_current_path(aConnection);
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

#if NETWORK_FRAMEWORK_DEBUG
        char srcAddrStr[IPAddress::kMaxStringLength + 1 /*null terminator */];
        char dstAddrStr[IPAddress::kMaxStringLength + 1 /*null terminator */];
        aPacketInfo.SrcAddress.ToString(srcAddrStr);
        aPacketInfo.DestAddress.ToString(dstAddrStr);
        ChipLogError(Inet, "Packet received from %s to %s", srcAddrStr, dstAddrStr);
#endif
    }

    nw_endpoint_t UDPEndPointImplNetworkFramework::GetEndPoint(const IPAddressType aAddressType,
        const IPAddress & aAddress, uint16_t aPort, InterfaceId interfaceIndex)
    {
        char addrStr[IPAddress::kMaxStringLength + 1 /*%*/ + InterfaceId::kMaxIfNameLength + 1 /*null terminator */];
        char portStr[INET_PORTSTRLEN];

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
            if (interfaceIndex != InterfaceId::Null()) {
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

        snprintf(portStr, sizeof(portStr), "%u", aPort);

        char * target = addrStr;

#if NETWORK_FRAMEWORK_DEBUG
        ChipLogError(Inet, "Create endpoint for ip(%s) port(%s)", target, portStr);
#endif
        return nw_endpoint_create_host(target, portStr);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::GetConnection(const IPPacketInfo * aPktInfo)
    {
        VerifyOrReturnError(nullptr != mParameters, CHIP_ERROR_INCORRECT_STATE);

        if (mConnection) {
            __auto_type path = nw_connection_copy_current_path(mConnection);
            __auto_type remote_endpoint = nw_path_copy_effective_remote_endpoint(path);
            // TODO Handle return value of IPAddress::GetIPAddressFromSockAdd
            IPAddress remote_address;
            IPAddress::GetIPAddressFromSockAddr(*nw_endpoint_get_address(remote_endpoint), remote_address);

            const uint16_t remote_port = nw_endpoint_get_port(remote_endpoint);
            const bool isDifferentEndPoint = aPktInfo->DestPort != remote_port || aPktInfo->DestAddress != remote_address;
            // Return without doing anything if we are not changing our endpoint.
            VerifyOrReturnError(isDifferentEndPoint, CHIP_NO_ERROR);

            ReturnErrorOnFailure(ReleaseConnection());
        }

        __auto_type endpoint = GetEndPoint(mAddrType, aPktInfo->DestAddress, aPktInfo->DestPort, aPktInfo->Interface);
        VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INCORRECT_STATE);

        __auto_type connection = nw_connection_create(endpoint, mParameters);
        VerifyOrReturnError(nullptr != connection, CHIP_ERROR_INCORRECT_STATE);

        return StartConnection(connection);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::StartListener()
    {
        VerifyOrReturnError(nullptr == mListener, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr == mListenerSemaphore, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr == mListenerQueue, CHIP_ERROR_INCORRECT_STATE);

        __auto_type listener = nw_listener_create(mParameters);
        VerifyOrReturnError(nullptr != listener, CHIP_ERROR_INCORRECT_STATE);

        mListenerSemaphore = dispatch_semaphore_create(0);
        VerifyOrReturnError(nullptr != mListenerSemaphore, CHIP_ERROR_NO_MEMORY);

        mListenerQueue = dispatch_queue_create("inet_dispatch_listener", DISPATCH_QUEUE_CONCURRENT);
        VerifyOrReturnError(nullptr != mListenerQueue, CHIP_ERROR_NO_MEMORY);

        nw_listener_set_queue(listener, mListenerQueue);

        nw_listener_set_new_connection_handler(listener, ^(nw_connection_t connection) {
            ReleaseConnection();
            StartConnection(connection);
        });

        __block CHIP_ERROR err = CHIP_NO_ERROR;
        nw_listener_set_state_changed_handler(listener, ^(nw_listener_state_t state, nw_error_t error) {
            DebugPrintListenerState(state);

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
                ChipLogError(Inet, "Error: %s", chip::ErrorStr(err));
                break;

            case nw_listener_state_ready:
                err = CHIP_NO_ERROR;
                dispatch_semaphore_signal(mListenerSemaphore);
                break;

            case nw_listener_state_cancelled:
                if (err == CHIP_NO_ERROR) {
                    err = CHIP_ERROR_CONNECTION_ABORTED;
                }

                dispatch_semaphore_signal(mListenerSemaphore);
                break;
            }
        });

        nw_listener_start(listener);
        dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);

        if (CHIP_NO_ERROR == err) {
            mListener = listener;
        }

        return err;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::StartConnection(nw_connection_t aConnection)
    {
        __block CHIP_ERROR err = CHIP_NO_ERROR;

        nw_connection_set_queue(aConnection, mConnectionQueue);

        nw_connection_set_state_changed_handler(aConnection, ^(nw_connection_state_t state, nw_error_t error) {
            DebugPrintConnectionState(state);

            switch (state) {
            case nw_connection_state_invalid:
                err = CHIP_ERROR_INCORRECT_STATE;
                nw_connection_cancel(aConnection);
                break;

            case nw_connection_state_preparing:
                err = CHIP_ERROR_INCORRECT_STATE;
                break;

            case nw_connection_state_waiting:
                nw_connection_cancel(aConnection);
                break;

            case nw_connection_state_failed:
                err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                break;

            case nw_connection_state_ready:
                err = CHIP_NO_ERROR;
                dispatch_semaphore_signal(mConnectionSemaphore);
                break;

            case nw_connection_state_cancelled:
                if (err == CHIP_NO_ERROR) {
                    err = CHIP_ERROR_CONNECTION_ABORTED;
                }

                dispatch_semaphore_signal(mConnectionSemaphore);
                break;
            }
        });

        nw_connection_start(aConnection);
        dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);

        if (CHIP_NO_ERROR == err) {
            DebugPrintConnection(aConnection);

            mConnection = aConnection;
            HandleDataReceived(mConnection);
        }
        return err;
    }

    void UDPEndPointImplNetworkFramework::HandleDataReceived(const nw_connection_t & aConnection)
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
                        GetPacketInfo(aConnection, packetInfo);
                        dispatch_async(mSystemQueue, ^{
                            OnReceiveError((UDPEndPoint *) this, error, &packetInfo);
                        });
                    }
                }
            };

            if (content != nullptr && OnMessageReceived != nullptr) {
                size_t count = dispatch_data_get_size(content);
                auto packetBufferHandle = System::PacketBufferHandle::New(count);
                auto * packetBuffer = std::move(packetBufferHandle).UnsafeRelease();
                dispatch_data_apply(content, ^(dispatch_data_t data, size_t offset, const void * buffer, size_t size) {
                    memmove(packetBuffer->Start() + offset, buffer, size);
                    return true;
                });
                packetBuffer->SetDataLength(count);

                IPPacketInfo packetInfo;
                GetPacketInfo(aConnection, packetInfo);
                dispatch_async(mSystemQueue, ^{
                    auto handle = System::PacketBufferHandle::Adopt(packetBuffer);
                    OnMessageReceived((UDPEndPoint *) this, std::move(handle), &packetInfo);
                });
            }

            schedule_next_receive();
        };

        nw_connection_receive_message(aConnection, handler);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseListener()
    {
        VerifyOrReturnError(nullptr != mListener, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnectionQueue, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

        nw_listener_cancel(mListener);
        dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
        mListener = nullptr;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseConnection()
    {
        VerifyOrReturnError(nullptr != mConnection, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnectionQueue, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

        nw_connection_cancel(mConnection);
        dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
        mConnection = nullptr;

        return CHIP_NO_ERROR;
    }

} // namespace Inet
} // namespace chip
