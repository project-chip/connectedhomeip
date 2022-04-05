/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *         This file defines a class for managing client application
 *         user-editable settings. CHIP settings are partitioned into two
 *         distinct areas:
 *
 *         1. immutable / durable: factory parameters (CHIP_DEFAULT_FACTORY_PATH)
 *         2. mutable / ephemeral: user parameters (CHIP_DEFAULT_CONFIG_PATH/CHIP_DEFAULT_DATA_PATH)
 *
 *         The ephemeral partition should be erased during factory reset.
 *
 *         ChipLinuxStorage wraps the storage class ChipLinuxStorageIni with mutex.
 *
 */

#pragma once

#include <mutex>
#include <platform/Linux/CHIPLinuxStorageIni.h>

#ifndef FATCONFDIR
#define FATCONFDIR "/tmp"
#endif

#ifndef SYSCONFDIR
#define SYSCONFDIR "/tmp"
#endif

#ifndef LOCALSTATEDIR
#define LOCALSTATEDIR "/tmp"
#endif

#define CHIP_DEFAULT_FACTORY_PATH                                                                                                  \
    FATCONFDIR "/"                                                                                                                 \
               "chip_factory.ini"
#define CHIP_DEFAULT_CONFIG_PATH                                                                                                   \
    SYSCONFDIR "/"                                                                                                                 \
               "chip_config.ini"
#define CHIP_DEFAULT_DATA_PATH                                                                                                     \
    LOCALSTATEDIR "/"                                                                                                              \
                  "chip_counters.ini"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ChipLinuxStorage : private ChipLinuxStorageIni
{
public:
    ChipLinuxStorage();
    ~ChipLinuxStorage();

    CHIP_ERROR Init(const char * configFile);
    CHIP_ERROR ReadValue(const char * key, bool & val);
    CHIP_ERROR ReadValue(const char * key, uint16_t & val);
    CHIP_ERROR ReadValue(const char * key, uint32_t & val);
    CHIP_ERROR ReadValue(const char * key, uint64_t & val);
    CHIP_ERROR ReadValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen);
    CHIP_ERROR ReadValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen);
    CHIP_ERROR WriteValue(const char * key, bool val);
    CHIP_ERROR WriteValue(const char * key, uint16_t val);
    CHIP_ERROR WriteValue(const char * key, uint32_t val);
    CHIP_ERROR WriteValue(const char * key, uint64_t val);
    CHIP_ERROR WriteValueStr(const char * key, const char * val);
    CHIP_ERROR WriteValueBin(const char * key, const uint8_t * data, size_t dataLen);
    CHIP_ERROR ClearValue(const char * key);
    CHIP_ERROR ClearAll();
    CHIP_ERROR Commit();
    bool HasValue(const char * key);

private:
    std::mutex mLock;
    bool mDirty;
    std::string mConfigPath;
    bool mInitialized = false;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
