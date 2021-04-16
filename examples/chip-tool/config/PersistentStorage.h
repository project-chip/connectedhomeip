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

#include <controller/CHIPDeviceController.h>
#include <inipp/inipp.h>
#include <support/logging/CHIPLogging.h>

class PersistentStorage : public chip::PersistentStorageDelegate
{
public:
    CHIP_ERROR Init();

    /////////// PersistentStorageDelegate Interface /////////
    void SetStorageDelegate(chip::PersistentStorageResultDelegate * delegate) override;
    CHIP_ERROR SyncGetKeyValue(const char * key, char * value, uint16_t & size) override;
    void AsyncSetKeyValue(const char * key, const char * value) override;
    void AsyncDeleteKeyValue(const char * key) override;

    uint16_t GetListenPort();
    chip::Logging::LogCategory GetLoggingLevel();

private:
    CHIP_ERROR CommitConfig();
    inipp::Ini<char> mConfig;
};
