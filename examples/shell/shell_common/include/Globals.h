/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPCore.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
#endif
constexpr size_t kResponseTimeOut = 1000;

extern chip::secure_channel::MessageCounterManager gMessageCounterManager;
extern chip::Messaging::ExchangeManager gExchangeManager;
extern chip::SecureSessionMgr gSessionManager;
extern chip::Inet::IPAddress gDestAddr;

extern chip::FabricIndex gFabricIndex;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
extern chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
#endif
extern chip::TransportMgr<chip::Transport::UDP> gUDPManager;
