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
#include <lib/support/SafeInt.h>

#define INET_PORTSTRLEN 6

#define NETWORK_FRAMEWORK_DEBUG 1

#if NETWORK_FRAMEWORK_DEBUG
#include <arpa/inet.h>
#endif // NETWORK_FRAMEWORK_DEBUG

namespace {
constexpr uint64_t kSendTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kConnectTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kConnectionAliveTimeoutInSeconds = 60 * NSEC_PER_SEC;
constexpr uint64_t kListenerTimeoutInSeconds = 10 * NSEC_PER_SEC;
constexpr uint64_t kConnectionGroupTimeoutInSeconds = 10 * NSEC_PER_SEC;

class WorkFlag {
public:
    void MarkDead() { mAlive = false; }
    bool IsAlive() const { return mAlive; }

private:
    std::atomic<bool> mAlive { true };
};

class ConnectionWrapper {
public:
    nw_connection_t connection = nullptr;
    dispatch_source_t timer = nullptr;

    bool Matches(const chip::Inet::IPPacketInfo & pktInfo) const
    {
        __auto_type path = nw_connection_copy_current_path(connection);
        VerifyOrReturnValue(nullptr != path, false);

        __auto_type remoteEndpoint = nw_path_copy_effective_remote_endpoint(path);
        VerifyOrReturnValue(nullptr != remoteEndpoint, false);

        chip::Inet::IPAddress remoteAddress;
        chip::Inet::IPAddress::GetIPAddressFromSockAddr(*nw_endpoint_get_address(remoteEndpoint), remoteAddress);
        const uint16_t remotePort = nw_endpoint_get_port(remoteEndpoint);

        return pktInfo.DestAddress == remoteAddress && pktInfo.DestPort == remotePort;
    }
};

static void ReleaseConnectionWrapperCallback(CFAllocatorRef allocator, const void * value)
{
    __auto_type * wrapper = static_cast<ConnectionWrapper *>(const_cast<void *>(value));
    if (wrapper->timer) {
        dispatch_source_cancel(wrapper->timer);
        wrapper->timer = nullptr;
    }
    wrapper->connection = nullptr;
    delete wrapper;
}

bool CompareEndPoints(nw_endpoint_t a, nw_endpoint_t b)
{
    if (a == nullptr || b == nullptr) {
        return false;
    }

    nw_endpoint_type_t typeA = nw_endpoint_get_type(a);
    nw_endpoint_type_t typeB = nw_endpoint_get_type(b);

    if (typeA != nw_endpoint_type_address || typeB != nw_endpoint_type_address) {
        return false;
    }

    const sockaddr * addrA = nw_endpoint_get_address(a);
    const sockaddr * addrB = nw_endpoint_get_address(b);
    if (addrA == nullptr || addrB == nullptr) {
        return false;
    }

    if (addrA->sa_family != addrB->sa_family) {
        return false;
    }

    uint16_t portA = nw_endpoint_get_port(a);
    uint16_t portB = nw_endpoint_get_port(b);

    if (portA != portB) {
        return false;
    }

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

#if !NETWORK_FRAMEWORK_DEBUG
void DebugPrintListenerState(nw_listener_state_t state) {};
void DebugPrintConnectionState(nw_connection_state_t state) {};
void DebugPrintConnection(const nw_connection_t aConnection) {};
void DebugPrintConnectionGroupState(nw_connection_group_state_t state) {};
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

constexpr const char * kConnectionGroupStateInvalid = "Connection Group: Invalid";
constexpr const char * kConnectionGroupStateWaiting = "Connection Group: Waiting";
constexpr const char * kConnectionGroupStateFailed = "Connection Group: Failed";
constexpr const char * kConnectionGroupStateReady = "Connection Group: Ready";
constexpr const char * kConnectionGroupStateCancelled = "Connection Group: Cancelled";

constexpr const char * kConnectionStateInvalid = "Connection: Invalid";
constexpr const char * kConnectionStateWaiting = "Connection: Waiting";
constexpr const char * kConnectionStatePreparing = "Connection: Preparing";
constexpr const char * kConnectionStateFailed = "Connection: Failed";
constexpr const char * kConnectionStateReady = "Connection: Ready";
constexpr const char * kConnectionStateCancelled = "Connection: Cancelled";

void DebugPrintConnectionGroupState(nw_connection_group_state_t state)
{
    const char * str = nullptr;

    switch (state) {
    case nw_connection_group_state_invalid:
        str = kConnectionGroupStateInvalid;
        break;
    case nw_connection_group_state_waiting:
        str = kConnectionGroupStateWaiting;
        break;
    case nw_connection_group_state_ready:
        str = kConnectionGroupStateReady;
        break;
    case nw_connection_group_state_failed:
        str = kConnectionGroupStateFailed;
        break;
    case nw_connection_group_state_cancelled:
        str = kConnectionGroupStateCancelled;
        break;
    default:
        chipDie();
    }

    ChipLogDetail(Inet, "%s", str);
}

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

    __auto_type * srcAddress = nw_endpoint_copy_address_string(srcEndPoint);
    __auto_type srcPort = nw_endpoint_get_port(srcEndPoint);
    __auto_type * dstAddress = nw_endpoint_copy_address_string(dstEndPoint);
    __auto_type dstPort = nw_endpoint_get_port(dstEndPoint);

    ChipLogError(Inet, "Connection source: %s:%u destination: %s:%u", srcAddress, srcPort, dstAddress, dstPort);

    free(srcAddress);
    free(dstAddress);
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

        IPAddress targetAddress = address;
        if (intfId.IsPresent()) {
            auto allowedInterfaceId = InterfaceId(InterfaceId::PlatformType(kDNSServiceInterfaceIndexLocalOnly));
            if (allowedInterfaceId != intfId) {
                return CHIP_ERROR_NOT_IMPLEMENTED;
            }
            Inet::IPAddress::FromString("::1", targetAddress);
        }

        __auto_type configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
        mParameters = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);
        VerifyOrReturnError(nullptr != mParameters, CHIP_ERROR_INVALID_ARGUMENT, ReleaseAll());

        nw_parameters_set_reuse_local_address(mParameters, true);

        // Note: The ConfigureProtocol function uses nw_ip_options_set_version to set the IP version for this endpoint.
        //
        // This works as expected when the IPAddress is specified. However, when using a wildcard address (chip::Inet::IPAddressType::kAny)
        //  for an IPv6 socket, the specified IP version (nw_ip_version_6) may be ignored, allowing both IPv4 and IPv6 connections.
        CHIP_ERROR err = ConfigureProtocol(addressType, mParameters);
        VerifyOrReturnError(CHIP_NO_ERROR == err, err, ReleaseAll());

#if INET_CONFIG_ENABLE_IPV4
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
#endif // INET_CONFIG_ENABLE_IPV4

        __auto_type endpoint = GetEndPoint(addressType, targetAddress, port);
        VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INTERNAL, ReleaseAll());
        nw_parameters_set_local_endpoint(mParameters, endpoint);

        mConnectionQueue = dispatch_queue_create("inet_dispatch_global", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        VerifyOrReturnError(nullptr != mConnectionQueue, CHIP_ERROR_NO_MEMORY, ReleaseAll());

        mSystemQueue = static_cast<System::LayerDispatch &>(GetSystemLayer()).GetDispatchQueue();
        mAddrType = addressType;
        mWorkFlagStrong = Platform::MakeShared<WorkFlag>();
        mWorkFlagWeak = mWorkFlagStrong;

        PrepareConnections();
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
        return nw_listener_get_port(mListener);
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

        nw_connection_t connection = FindConnection(*pktInfo);
        if (!connection) {
            ReturnErrorOnFailure(GetConnection(pktInfo));
            connection = FindConnection(*pktInfo);
            VerifyOrReturnError(nullptr != connection, CHIP_ERROR_INCORRECT_STATE);
        }

        // Wrap the PacketBufferHandle in a shared_ptr so we can capture it by value
        __auto_type retainedHandle = std::make_shared<System::PacketBufferHandle>(std::move(msg));
        __auto_type content = dispatch_data_create(
            retainedHandle->operator->()->Start(),
            retainedHandle->operator->()->DataLength(),
            mSystemQueue,
            ^{
                // Keep retainedHandle alive until the dispatch_data is released
                [[maybe_unused]] auto cleanup = retainedHandle;
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

        ReleaseConnections();
        ReleaseConnectionGroup();
        ReleaseListener();

        mParameters = nullptr;

        mConnectionQueue = nullptr;

        mListenerQueue = nullptr;
        mListenerSemaphore = nullptr;

        mConnectionGroupQueue = nullptr;
        mConnectionGroupSemaphore = nullptr;

        mSystemQueue = nullptr;
    }

    void UDPEndPointImplNetworkFramework::Free()
    {
        Close();
        Release();
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::SetMulticastLoopback(IPVersion ipVersion, bool loopback)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::IPAnyJoinLeaveMulticastGroup(nw_endpoint_t endpoint, bool join)
    {
        ChipLogError(Inet, "%s", __func__);

        if (nullptr == mConnectionGroup) {
            VerifyOrReturnError(join, CHIP_ERROR_NOT_FOUND); // Nothing ever joined, can't leave!

            __auto_type groupDescriptor = nw_group_descriptor_create_multicast(endpoint);
            VerifyOrReturnError(nullptr != groupDescriptor, CHIP_ERROR_INCORRECT_STATE);

            return StartConnectionGroup(groupDescriptor);
        }

        __auto_type previousGroupDescriptor = nw_connection_group_copy_descriptor(mConnectionGroup);
        ReleaseConnectionGroup();

        __block nw_group_descriptor_t groupDescriptor = nullptr;
        __block bool found = false;
        __block size_t endpointCount = 0;
        __block CHIP_ERROR error = CHIP_NO_ERROR;
        nw_group_descriptor_enumerate_endpoints(previousGroupDescriptor, ^(nw_endpoint_t previousEndpoint) {
            if (CompareEndPoints(previousEndpoint, endpoint)) {
                found = true;
                if (!join) {
                    // Leaving -> skip adding this one
                    return true;
                }
            }

            if (nullptr == groupDescriptor) {
                groupDescriptor = nw_group_descriptor_create_multicast(previousEndpoint);
                if (nullptr == groupDescriptor) {
                    error = CHIP_ERROR_INCORRECT_STATE;
                    return false;
                }
            } else {
                bool added = nw_group_descriptor_add_endpoint(groupDescriptor, previousEndpoint);
                if (false == added) {
                    error = CHIP_ERROR_INCORRECT_STATE;
                    return false;
                }
            }
            endpointCount++;
            return true;
        });
        ReturnErrorOnFailure(error);

        if (!join && endpointCount == 0) {
            // After removal, if no endpoints remain, fully remove the connection group
            mConnectionGroup = nullptr;
            mConnectionGroupSemaphore = nullptr;
            mConnectionGroupQueue = nullptr;
            return CHIP_NO_ERROR;
        }

        if (join && !found) {
            // If joining and the endpoint wasn't already present, add it
            VerifyOrReturnError(nw_group_descriptor_add_endpoint(groupDescriptor, endpoint), CHIP_ERROR_INVALID_ARGUMENT);
        }

        return StartConnectionGroup(groupDescriptor);
    }

#if INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR UDPEndPointImplNetworkFramework::IPv4JoinLeaveMulticastGroupImpl(InterfaceId intfId, const IPAddress & address, bool join)
    {
        VerifyOrReturnError(!intfId.IsPresent(), CHIP_ERROR_NOT_IMPLEMENTED);

        const uint16_t port = GetBoundPort();
        __auto_type endpoint = GetEndPoint(IPAddressType::kIPv4, address, port);
        VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

        return IPAnyJoinLeaveMulticastGroup(endpoint, join);
    }
#endif // INET_CONFIG_ENABLE_IPV4

    CHIP_ERROR UDPEndPointImplNetworkFramework::IPv6JoinLeaveMulticastGroupImpl(InterfaceId intfId, const IPAddress & address, bool join)
    {
        VerifyOrReturnError(!intfId.IsPresent(), CHIP_ERROR_NOT_IMPLEMENTED);

        const uint16_t port = GetBoundPort();
        __auto_type endpoint = GetEndPoint(IPAddressType::kIPv6, address, port);
        VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

        return IPAnyJoinLeaveMulticastGroup(endpoint, join);
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

#if NETWORK_FRAMEWORK_DEBUG
        char srcAddrStr[IPAddress::kMaxStringLength + 1 /*null terminator */];
        char dstAddrStr[IPAddress::kMaxStringLength + 1 /*null terminator */];
        aPacketInfo.SrcAddress.ToString(srcAddrStr);
        aPacketInfo.DestAddress.ToString(dstAddrStr);
        ChipLogError(Inet, "Packet received from %s to %s", srcAddrStr, dstAddrStr);
#endif

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

        char portStr[INET_PORTSTRLEN];
        snprintf(portStr, sizeof(portStr), "%u", aPort);

#if NETWORK_FRAMEWORK_DEBUG
        ChipLogError(Inet, "Create endpoint for ip(%s) port(%s)", addrStr, portStr);
#endif

        return nw_endpoint_create_host(addrStr, portStr);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::StartListener()
    {
        VerifyOrReturnError(nullptr == mListener, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr == mListenerSemaphore, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr == mListenerQueue, CHIP_ERROR_INCORRECT_STATE);

        mListener = nw_listener_create(mParameters);
        VerifyOrReturnError(nullptr != mListener, CHIP_ERROR_INCORRECT_STATE, ReleaseAll());

        mListenerSemaphore = dispatch_semaphore_create(0);
        VerifyOrReturnError(nullptr != mListenerSemaphore, CHIP_ERROR_NO_MEMORY, ReleaseAll());

        mListenerQueue = dispatch_queue_create("inet_dispatch_listener", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        VerifyOrReturnError(nullptr != mListenerQueue, CHIP_ERROR_NO_MEMORY, ReleaseAll());

        nw_listener_set_queue(mListener, mListenerQueue);
        nw_listener_set_new_connection_handler(mListener, ^(nw_connection_t connection) {
            StartConnectionFromListener(connection);
        });

        __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
        nw_listener_set_state_changed_handler(mListener, ^(nw_listener_state_t state, nw_error_t error) {
            DebugPrintListenerState(state);

            switch (state) {
            case nw_listener_state_invalid:
                err = CHIP_ERROR_INCORRECT_STATE;
                nw_listener_cancel(mListener);
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

        nw_listener_start(mListener);
        __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kListenerTimeoutInSeconds);
        dispatch_semaphore_wait(mListenerSemaphore, timeout);
        nw_listener_set_state_changed_handler(mListener, nil);

        if (CHIP_NO_ERROR != err) {
            mListener = nullptr;
            mListenerSemaphore = nullptr;
            mListenerQueue = nullptr;
        }

        return err;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::GetConnection(const IPPacketInfo * aPktInfo)
    {
        VerifyOrReturnError(nullptr != mParameters, CHIP_ERROR_INCORRECT_STATE);

        __auto_type endpoint = GetEndPoint(mAddrType, aPktInfo->DestAddress, aPktInfo->DestPort, aPktInfo->Interface);
        VerifyOrReturnError(nullptr != endpoint, CHIP_ERROR_INCORRECT_STATE);

        __auto_type parameters = nw_parameters_copy(mParameters);
        VerifyOrReturnError(parameters != nullptr, CHIP_ERROR_NO_MEMORY);

        nw_parameters_set_local_endpoint(parameters, nullptr);
        __auto_type connection = nw_connection_create(endpoint, parameters); // Let system pick ephemeral port
        VerifyOrReturnError(nullptr != connection, CHIP_ERROR_INCORRECT_STATE);

        return StartConnection(connection);
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::StartConnectionFromListener(nw_connection_t connection)
    {
        DebugPrintConnection(connection);

        VerifyOrReturnError(CreateConnectionWrapper(connection), CHIP_ERROR_NO_MEMORY);
        nw_connection_set_queue(connection, mConnectionQueue);
        nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
            DebugPrintConnectionState(state);
            switch (state) {
            case nw_connection_state_ready:
                HandleDataReceived(connection);
                break;

            case nw_connection_state_preparing:
                // Nothing to do.
                break;

            case nw_connection_state_invalid:
            case nw_connection_state_waiting:
                nw_connection_cancel(connection);
                break;

            case nw_connection_state_failed: {
                CHIP_ERROR err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                ChipLogError(Inet, "Error: %s", chip::ErrorStr(err));
                break;
            }

            case nw_connection_state_cancelled:
                static_cast<System::LayerDispatch &>(GetSystemLayer()).ScheduleWorkWithBlock(^{
                    ClearConnectionWrapper(connection);
                });
                break;
            }
        });

        nw_connection_start(connection);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::StartConnection(nw_connection_t connection)
    {
        __auto_type semaphore = dispatch_semaphore_create(0);
        VerifyOrReturnError(nullptr != semaphore, CHIP_ERROR_NO_MEMORY);

        VerifyOrReturnError(CreateConnectionWrapper(connection), CHIP_ERROR_NO_MEMORY);
        nw_connection_set_queue(connection, mConnectionQueue);

        __block CHIP_ERROR err = CHIP_NO_ERROR;
        nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
            DebugPrintConnectionState(state);

            switch (state) {
            case nw_connection_state_invalid:
                err = CHIP_ERROR_INCORRECT_STATE;
                nw_connection_cancel(connection);
                break;

            case nw_connection_state_preparing:
                err = CHIP_ERROR_INCORRECT_STATE;
                break;

            case nw_connection_state_waiting:
                nw_connection_cancel(connection);
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
        dispatch_semaphore_wait(semaphore, timeout);

        if (err != CHIP_NO_ERROR) {
            ClearConnectionWrapper(connection);
            return err;
        }

        DebugPrintConnection(connection);
        HandleDataReceived(connection);
        return err;
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
                            OnReceiveError((UDPEndPoint *) this, error, nullptr);
                        } else {
                            OnReceiveError((UDPEndPoint *) this, error, &packetInfo);
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

    CHIP_ERROR UDPEndPointImplNetworkFramework::StartConnectionGroup(nw_group_descriptor_t groupDescriptor)
    {
        __auto_type parameters = nw_parameters_copy(mParameters);
        VerifyOrReturnError(nullptr != parameters, CHIP_ERROR_NO_MEMORY);
        nw_parameters_set_local_endpoint(parameters, nil);

        mConnectionGroup = nw_connection_group_create(groupDescriptor, parameters);
        VerifyOrReturnError(mConnectionGroup != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        if (mConnectionGroupQueue == nullptr) {
            mConnectionGroupQueue = dispatch_queue_create("inet_dispatch_group", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        }

        nw_connection_group_set_queue(mConnectionGroup, mConnectionGroupQueue);

        mConnectionGroupSemaphore = dispatch_semaphore_create(0);
        VerifyOrReturnError(mConnectionGroupSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);

        __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
        nw_connection_group_set_state_changed_handler(mConnectionGroup, ^(nw_connection_group_state_t state, nw_error_t error) {
            DebugPrintConnectionGroupState(state);
            switch (state) {
            case nw_connection_group_state_invalid:
                err = CHIP_ERROR_INCORRECT_STATE;
                nw_connection_group_cancel(mConnectionGroup);
                break;
            case nw_connection_group_state_ready:
                err = CHIP_NO_ERROR;
                dispatch_semaphore_signal(mConnectionGroupSemaphore);
                break;
            case nw_connection_group_state_failed:
                err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                ChipLogError(Inet, "Error: %s", chip::ErrorStr(err));
                break;
            case nw_connection_group_state_cancelled:
                if (err == CHIP_NO_ERROR) {
                    err = CHIP_ERROR_CONNECTION_ABORTED;
                }
                dispatch_semaphore_signal(mConnectionGroupSemaphore);
                break;
            default:
                break;
            }
        });

        nw_connection_group_set_receive_handler(mConnectionGroup, UINT32_MAX, YES, ^(dispatch_data_t content, nw_content_context_t context, bool is_complete) {
            ChipLogError(Inet, "%s", __func__);
        });

        nw_connection_group_start(mConnectionGroup);

        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionGroupTimeoutInSeconds);
        dispatch_semaphore_wait(mConnectionGroupSemaphore, timeout);

        nw_connection_group_set_state_changed_handler(mConnectionGroup, nil);

        return err;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseConnectionGroup()
    {
        VerifyOrReturnError(nullptr != mConnectionGroup, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnectionGroupSemaphore, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnectionGroupQueue, CHIP_ERROR_INCORRECT_STATE);

        nw_connection_group_set_state_changed_handler(mConnectionGroup, ^(nw_connection_group_state_t state, nw_error_t error) {
            if (state == nw_connection_group_state_cancelled) {
                dispatch_semaphore_signal(mConnectionGroupSemaphore);
            }
        });

        nw_connection_group_cancel(mConnectionGroup);
        dispatch_semaphore_wait(mConnectionGroupSemaphore, DISPATCH_TIME_FOREVER);
        mConnectionGroup = nullptr;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseListener()
    {
        VerifyOrReturnError(nullptr != mListener, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mListenerSemaphore, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mListenerQueue, CHIP_ERROR_INCORRECT_STATE);

        nw_listener_set_state_changed_handler(mListener, ^(nw_listener_state_t state, nw_error_t error) {
            if (state == nw_listener_state_cancelled) {
                dispatch_semaphore_signal(mListenerSemaphore);
            }
        });

        nw_listener_cancel(mListener);
        dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
        mListener = nullptr;

        return CHIP_NO_ERROR;
    }

    void UDPEndPointImplNetworkFramework::PrepareConnections()
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

    CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseConnections()
    {
        VerifyOrReturnError(nullptr != mConnectionQueue, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(nullptr != mConnections, CHIP_NO_ERROR);

        CFIndex count = CFDictionaryGetCount(mConnections);
        const void ** keys = static_cast<const void **>(malloc(sizeof(void *) * static_cast<size_t>(count)));
        CFDictionaryGetKeysAndValues(mConnections, keys, nullptr);

        __auto_type group = dispatch_group_create();

        for (CFIndex i = 0; i < count; ++i) {
            __auto_type connection = (__bridge nw_connection_t)(const_cast<void *>(keys[i]));

            dispatch_group_enter(group);
            nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
                if (state == nw_connection_state_cancelled) {
                    dispatch_group_leave(group);
                }
            });

            nw_connection_cancel(connection);
        }

        free(keys);
        dispatch_group_wait(group, DISPATCH_TIME_FOREVER); // NOLINT(clang-analyzer-optin.performance.GCDAntipattern)
        CFRelease(mConnections);
        mConnections = nullptr;

        return CHIP_NO_ERROR;
    }

    bool UDPEndPointImplNetworkFramework::RefreshConnectionTimeout(nw_connection_t connection)
    {
        __auto_type wrapper = const_cast<ConnectionWrapper *>(static_cast<const ConnectionWrapper *>(CFDictionaryGetValue(mConnections, (__bridge const void *) connection)));
        if (nullptr == wrapper) {
            wrapper = new ConnectionWrapper;
        }
        VerifyOrReturnValue(nullptr != wrapper, false);

        __auto_type timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, mSystemQueue);
        dispatch_source_set_event_handler(timer, ^{
            ClearConnectionWrapper(connection);
        });

        __auto_type timeout = dispatch_time(DISPATCH_TIME_NOW, kConnectionAliveTimeoutInSeconds);
        uint64_t interval = DISPATCH_TIME_FOREVER;
        uint64_t leeway = 1 * NSEC_PER_SEC;
        dispatch_source_set_timer(timer, timeout, interval, leeway);

        if (nullptr == wrapper->connection) {
            wrapper->connection = connection;
            wrapper->timer = timer;
            CFDictionarySetValue(mConnections, (__bridge const void *) connection, wrapper);
        } else {
            wrapper->timer = timer;
        }
        dispatch_resume(timer);

        return true;
    }

    bool UDPEndPointImplNetworkFramework::CreateConnectionWrapper(nw_connection_t connection)
    {
        return RefreshConnectionTimeout(connection);
    }

    bool UDPEndPointImplNetworkFramework::ClearConnectionWrapper(nw_connection_t connection)
    {
        CFDictionaryRemoveValue(mConnections, (__bridge const void *) connection);
        return true;
    }

    nw_connection_t UDPEndPointImplNetworkFramework::FindConnection(const IPPacketInfo & pktInfo)
    {
        CFIndex count = CFDictionaryGetCount(mConnections);
        __auto_type ** keys = static_cast<const void **>(malloc(sizeof(void *) * static_cast<size_t>(count)));
        CFDictionaryGetKeysAndValues(mConnections, keys, nullptr);

        nw_connection_t match = nullptr;
        for (CFIndex i = 0; i < count; ++i) {
            __auto_type * wrapper = const_cast<ConnectionWrapper *>(static_cast<const ConnectionWrapper *>(CFDictionaryGetValue(mConnections, keys[i])));
            if (wrapper && wrapper->Matches(pktInfo)) {
                match = wrapper->connection;
                break;
            }
        }

        free(keys);
        return match;
    }

} // namespace Inet
} // namespace chip
