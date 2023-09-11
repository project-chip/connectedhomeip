/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines some of the common constants, globals and interfaces
 *      that are common to and used by CHIP example applications.
 *
 */

#pragma once

#include <credentials/FabricTable.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SessionManager.h>

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr size_t kNetworkSleepTimeMsecs       = (100 * 1000);

extern chip::FabricTable gFabricTable;
extern chip::SessionManager gSessionManager;
extern chip::Messaging::ExchangeManager gExchangeManager;
extern chip::secure_channel::MessageCounterManager gMessageCounterManager;
extern chip::TestPersistentStorageDelegate gStorage;
extern chip::Crypto::DefaultSessionKeystore gSessionKeystore;

void InitializeChip(void);
void ShutdownChip(void);
