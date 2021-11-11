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

    // Return the stored local node id, or the default one if nothing is stored.
    chip::NodeId GetLocalNodeId();

    // Store local node id.
    CHIP_ERROR SetLocalNodeId(chip::NodeId nodeId);

    /**
     * @brief
     *  Configure the fabric used for pairing and sending commands.
     *
     * @param[in] fabricName  The name of the fabric. It must be one of the following strings:
     *                         - alpha
     *                         - beta
     *                         - gamma
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR SetFabric(const char * fabricName);

    /**
     * @brief
     *  Return the stored fabric id, or the one for the "alpha" fabric if nothing is stored.
     */
    chip::FabricId GetFabricId();

private:
    CHIP_ERROR CommitConfig();
    inipp::Ini<char> mConfig;
};
