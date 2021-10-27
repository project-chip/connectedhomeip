/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <app/util/basic-types.h>
#include <controller/CHIPDeviceController.h>
#include <inipp/inipp.h>
#include <lib/support/logging/CHIPLogging.h>

class PersistentStorage : public chip::PersistentStorageDelegate
{
public:
    CHIP_ERROR Init();

    /////////// PersistentStorageDelegate Interface /////////
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    uint16_t GetListenPort();
    chip::Logging::LogCategory GetLoggingLevel();

    // Return the stored node ids, or the default ones if nothing is stored.
    chip::NodeId GetLocalNodeId();
    chip::NodeId GetRemoteNodeId();

    // Store node ids.
    CHIP_ERROR SetLocalNodeId(chip::NodeId nodeId);
    CHIP_ERROR SetRemoteNodeId(chip::NodeId nodeId);

private:
    // Helpers for node ids.
    chip::NodeId GetNodeId(const char * key, chip::NodeId defaultVal);
    CHIP_ERROR SetNodeId(const char * key, chip::NodeId value);

    CHIP_ERROR CommitConfig();
    inipp::Ini<char> mConfig;
};
