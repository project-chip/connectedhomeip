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

#include <Network/Network.h>

#include <inet/IPPacketInfo.h>

namespace chip {
namespace Inet {
namespace Darwin {
void DebugPrintListenerState(nw_listener_state_t state, nw_error_t error);
void DebugPrintConnectionGroupState(nw_connection_group_state_t state, nw_interface_t interface, nw_error_t error);
void DebugPrintConnectionState(nw_connection_state_t state, nw_error_t error);
void DebugPrintConnection(nw_connection_t connection);
void DebugPrintEndPoint(nw_endpoint_t endpoint);
void DebugPrintPacketInfo(IPPacketInfo & packetInfo);
} // namespace Darwin
} // namespace Inet
} // namespace chip
