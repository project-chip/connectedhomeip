/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include "DiagnosticLogsDemo.h"
#include "DiagnosticLogsProviderDelegateImpl.h"

#include <app/server/Server.h>

CHIP_ERROR chip::NXP::App::DiagnosticLogsDemo::DisplayUsage()
{
    char diagLog[CHIP_DEVICE_CONFIG_MAX_DIAG_LOG_SIZE];
    uint16_t diagLogSize = CHIP_DEVICE_CONFIG_MAX_DIAG_LOG_SIZE;

    chip::StorageKeyName keyUser  = chip::app::Clusters::DiagnosticLogs::LogProvider::GetKeyDiagUserSupport();
    chip::StorageKeyName keyNwk   = chip::app::Clusters::DiagnosticLogs::LogProvider::GetKeyDiagNetwork();
    chip::StorageKeyName keyCrash = chip::app::Clusters::DiagnosticLogs::LogProvider::GetKeyDiagCrashLog();

    auto & persistentStorage = chip::Server::GetInstance().GetPersistentStorage();
    /* The KVS wear stats are stored in the user diagnostic log key hence only
     * initialize this key if the KVS wear stats are not enabled. */
#if (CHIP_DEVICE_CONFIG_KVS_WEAR_STATS != 1)
    memset(diagLog, 0, diagLogSize);
    persistentStorage.SyncSetKeyValue(keyUser.KeyName(), diagLog, diagLogSize);
#endif

    memset(diagLog, 1, diagLogSize);
    persistentStorage.SyncSetKeyValue(keyNwk.KeyName(), diagLog, diagLogSize);

    memset(diagLog, 2, diagLogSize);
    persistentStorage.SyncSetKeyValue(keyCrash.KeyName(), diagLog, diagLogSize);

    return CHIP_NO_ERROR;
}
