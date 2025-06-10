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

#include "UDPEndPointImplNetworkFrameworkDebug.h"

#include <lib/support/CodeUtils.h>

#include <arpa/inet.h>

#define NETWORK_FRAMEWORK_DEBUG 0

#if NETWORK_FRAMEWORK_DEBUG
namespace {
constexpr const char * kNoAddress = "<no-address>";
constexpr size_t kMaxIPStrLen = INET6_ADDRSTRLEN + 1 /*%*/ + IFNAMSIZ + 1 /*:*/ + 5 /*65535*/ + 1 /*\0*/;

bool FormatIPv4Address(const struct sockaddr * sa, uint16_t port, char * out, socklen_t len)
{
    __auto_type in4 = reinterpret_cast<const struct sockaddr_in *>(sa);
    VerifyOrReturnValue(nullptr != inet_ntop(AF_INET, &in4->sin_addr, out, len), false);

    size_t offset = strlen(out);
    size_t available = len - offset;
    int written = snprintf(out + offset, available, ":%u", port);
    return (written >= 0 && static_cast<size_t>(written) < available);
}

bool FormatIPv6Address(const struct sockaddr * sa, uint16_t port, char * out, socklen_t len)
{
    __auto_type in6 = reinterpret_cast<const struct sockaddr_in6 *>(sa);
    VerifyOrReturnValue(nullptr != inet_ntop(AF_INET6, &in6->sin6_addr, out, len), false);

    size_t offset = strlen(out);
    size_t available = len - offset;
    int written = 0;

    uint32_t interfaceIndex = static_cast<uint32_t>(in6->sin6_scope_id);
    if (interfaceIndex == 0) {
        written = snprintf(out + offset, available, ":%u", port);
    } else {
        char interfaceName[IFNAMSIZ] = { 0 };
        if (nullptr != if_indextoname(interfaceIndex, interfaceName)) {
            written = snprintf(out + offset, available, "%%%s:%u", interfaceName, port);
        } else {
            written = snprintf(out + offset, available, "%%<%u>:%u", interfaceIndex, port);
        }
    }

    return (written >= 0 && static_cast<size_t>(written) < available);
}

bool FormatIPAddress(const struct sockaddr * sa, uint16_t port, char * out, socklen_t len)
{
    VerifyOrReturnValue(nullptr != sa, false);

    bool hasAddress = false;

    if (sa->sa_family == AF_INET) {
        hasAddress = FormatIPv4Address(sa, port, out, len);
    } else if (sa->sa_family == AF_INET6) {
        hasAddress = FormatIPv6Address(sa, port, out, len);
    }

    return hasAddress;
}

//
// We intentionally do not use nw_endpoint_copy_address_string() here because:
//   - For IPv6 endpoints created via nw_endpoint_create_host("<addr>%<ifname>", "<port>"),
//     nw_endpoint_copy_address_string() will return only the raw 128-bit address and port,
//     omitting the “%<ifname>” zone ID. This makes it impossible to see which interface
//     (sin6_scope_id) was actually encoded into the endpoint.
//   - To reliably log “IP%<interface>:port” for debugging, we must extract the sockaddr
//     directly via nw_endpoint_get_address() and inspect sin6_scope_id ourselves.
//
char * FormatEndpointAddressWithInterface(nw_endpoint_t endpoint)
{
    __auto_type * sa = nw_endpoint_get_address(endpoint);
    uint16_t port = nw_endpoint_get_port(endpoint);
    char addressStr[kMaxIPStrLen] = { 0 };

    bool hasAddress = FormatIPAddress(sa, port, addressStr, sizeof(addressStr));
    return strdup(hasAddress ? addressStr : kNoAddress);
}

}
#endif

namespace chip {
namespace Inet {
    namespace Darwin {
#if NETWORK_FRAMEWORK_DEBUG
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

        constexpr const char * kNilConnection = "The connection is nil.";
        constexpr const char * kNilPath = "The connection path is nil.";
        constexpr const char * kNilPathSourceEndPoint = "The connection path source endpoint is nil.";
        constexpr const char * kNilPathDestinationEndPoint = "The connection path destination endpoint is nil.";

        constexpr const char * kPathStatusInvalid = "This path is not valid.";
        constexpr const char * kPathStatusUnsatisfied = "The path is not available for use.";
        constexpr const char * kPathStatusSatisfied = "The path is available to establish connections and send data.";
        constexpr const char * kPathStatusSatisfiable = "The path is not currently available, but establishing a new connection may activate the path.";

        constexpr const char * kEndpointTypeInvalid = "Endpoint: Invalid";
        constexpr const char * kEndpointTypeAddress = "Endpoint: Address";
        constexpr const char * kEndpointTypeHost = "Endpoint: Host";
        constexpr const char * kEndpointTypeBonjourService = "Endpoint: Bonjour Service";
        constexpr const char * kEndpointTypeURL = "Endpoint: URL";

        void DebugPrintListenerState(nw_listener_state_t state, nw_error_t error)
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

            if (error) {
                CHIP_ERROR err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                ChipLogDetail(Inet, "%s - Error: %s", str, chip::ErrorStr(err));
            } else {
                ChipLogDetail(Inet, "%s", str);
            }
        }

        void DebugPrintConnectionGroupState(nw_connection_group_state_t state, nw_interface_t interface, nw_error_t error)
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

            const char * interfaceName = nw_interface_get_name(interface);
            if (error) {
                CHIP_ERROR err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                ChipLogDetail(Inet, "%s (%s) - Error: %s", str, interfaceName, chip::ErrorStr(err));
            } else {
                ChipLogDetail(Inet, "%s (%s)", str, interfaceName);
            }
        }

        void DebugPrintConnectionState(nw_connection_state_t state, nw_error_t error)
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

            if (error) {
                CHIP_ERROR err = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
                ChipLogDetail(Inet, "%s - Error: %s", str, chip::ErrorStr(err));
            } else {
                ChipLogDetail(Inet, "%s", str);
            }
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

        void DebugPrintConnection(nw_connection_t connection)
        {
            VerifyOrReturn(nil != connection, ChipLogError(Inet, "%s", kNilConnection));

            __auto_type path = nw_connection_copy_current_path(connection);
            VerifyOrReturn(nil != path, ChipLogError(Inet, "%s", kNilPath));
            DebugPrintConnectionPathStatus(path);

            __auto_type srcEndPoint = nw_path_copy_effective_local_endpoint(path);
            VerifyOrReturn(nil != srcEndPoint, ChipLogError(Inet, "%s", kNilPathSourceEndPoint));

            __auto_type dstEndPoint = nw_path_copy_effective_remote_endpoint(path);
            VerifyOrReturn(nil != dstEndPoint, ChipLogError(Inet, "%s", kNilPathDestinationEndPoint));

            __auto_type * srcAddress = FormatEndpointAddressWithInterface(srcEndPoint);
            __auto_type * dstAddress = FormatEndpointAddressWithInterface(dstEndPoint);

            ChipLogError(Inet, "Connection source: %s destination: %s", srcAddress, dstAddress);

            free(srcAddress);
            free(dstAddress);
        }

        void DebugPrintEndPoint(nw_endpoint_t endpoint)
        {
            const char * str = nullptr;

            switch (nw_endpoint_get_type(endpoint)) {
            case nw_endpoint_type_invalid:
                str = kEndpointTypeInvalid;
                break;
            case nw_endpoint_type_address:
                str = kEndpointTypeAddress;
                break;

            case nw_endpoint_type_host:
                str = kEndpointTypeHost;
                break;
            case nw_endpoint_type_bonjour_service:
                str = kEndpointTypeBonjourService;
                break;
            case nw_endpoint_type_url:
                str = kEndpointTypeURL;
                break;
            default:
                chipDie();
            }

            if (str == kEndpointTypeAddress) {
                char * addressStr = FormatEndpointAddressWithInterface(endpoint);
                ChipLogError(Inet, "%s (%s)", str, addressStr);
                free(addressStr);
            } else {
                ChipLogDetail(Inet, "%s", str);
            }
        }

        void DebugPrintPacketInfo(IPPacketInfo & packetInfo)
        {
            char srcAddrStr[IPAddress::kMaxStringLength + 1 /*null terminator */];
            char dstAddrStr[IPAddress::kMaxStringLength + 1 /*null terminator */];
            packetInfo.SrcAddress.ToString(srcAddrStr);
            packetInfo.DestAddress.ToString(dstAddrStr);
            ChipLogError(Inet, "Packet received from %s to %s", srcAddrStr, dstAddrStr);
        }
#else
        void DebugPrintListenerState(nw_listener_state_t state, nw_error_t error) {};
        void DebugPrintConnectionGroupState(nw_connection_group_state_t state, nw_interface_t interface, nw_error_t error) {};
        void DebugPrintConnectionState(nw_connection_state_t state, nw_error_t error) {};
        void DebugPrintConnection(const nw_connection_t connection) {};
        void DebugPrintEndPoint(nw_endpoint_t endpoint) {};
        void DebugPrintPacketInfo(IPPacketInfo & packetInfo) {};
#endif
    } // namespace Darwin
} // namespace Inet
} // namespace chip
