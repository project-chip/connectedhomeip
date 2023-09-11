/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <credentials/FabricTable.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SessionHolder.h>
#include <transport/SessionManager.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
#endif
constexpr chip::System::Clock::Timeout kResponseTimeOut = chip::System::Clock::Seconds16(1);

extern chip::FabricTable gFabricTable;
extern chip::secure_channel::MessageCounterManager gMessageCounterManager;
extern chip::Messaging::ExchangeManager gExchangeManager;
extern chip::SessionManager gSessionManager;
extern chip::Inet::IPAddress gDestAddr;
extern chip::SessionHolder gSession;
extern chip::TestPersistentStorageDelegate gStorage;

extern chip::FabricIndex gFabricIndex;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
extern chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
#endif
extern chip::TransportMgr<chip::Transport::UDP> gUDPManager;
