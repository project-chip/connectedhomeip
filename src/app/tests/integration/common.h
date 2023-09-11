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

#include <app/util/basic-types.h>
#include <credentials/FabricTable.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SessionHolder.h>

#define MAX_MESSAGE_SOURCE_STR_LENGTH (100)
#define NETWORK_SLEEP_TIME_MSECS (100 * 1000)

extern chip::FabricTable gFabricTable;
extern chip::Messaging::ExchangeManager gExchangeManager;
extern chip::SessionManager gSessionManager;
extern chip::secure_channel::MessageCounterManager gMessageCounterManager;
extern chip::SessionHolder gSession;
extern chip::TestPersistentStorageDelegate gStorage;
extern chip::Crypto::DefaultSessionKeystore gSessionKeystore;

constexpr chip::NodeId kTestNodeId         = 0x1ULL;
constexpr chip::NodeId kTestNodeId1        = 0x2ULL;
constexpr chip::ClusterId kTestClusterId   = 6;
constexpr chip::CommandId kTestCommandId   = 40;
constexpr chip::EndpointId kTestEndpointId = 1;
constexpr chip::GroupId kTestGroupId       = 0;
constexpr chip::FieldId kTestFieldId1      = 1;
constexpr chip::FieldId kTestFieldId2      = 2;
constexpr uint8_t kTestFieldValue1         = 1;
constexpr uint8_t kTestFieldValue2         = 2;
constexpr chip::EventId kTestChangeEvent1  = 1;
constexpr chip::EventId kTestChangeEvent2  = 2;
void InitializeChip(void);
void ShutdownChip(void);
void TLVPrettyPrinter(const char * aFormat, ...);
