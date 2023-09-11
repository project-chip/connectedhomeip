/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Globals.h>

chip::FabricTable gFabricTable;
chip::secure_channel::MessageCounterManager gMessageCounterManager;
chip::Messaging::ExchangeManager gExchangeManager;
chip::SessionManager gSessionManager;
chip::Inet::IPAddress gDestAddr;
chip::SessionHolder gSession;
chip::TestPersistentStorageDelegate gStorage;

chip::FabricIndex gFabricIndex = 0;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
#endif
chip::TransportMgr<chip::Transport::UDP> gUDPManager;
