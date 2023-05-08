/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
