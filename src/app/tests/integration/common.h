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

#include <messaging/ExchangeMgr.h>

#define MAX_MESSAGE_SOURCE_STR_LENGTH (100)
#define NETWORK_SLEEP_TIME_MSECS (100 * 1000)

extern chip::Messaging::ExchangeManager gExchangeManager;

void InitializeChip(void);
void ShutdownChip(void);
void DriveIO(void);
void TLVPrettyPrinter(const char * aFormat, ...);
