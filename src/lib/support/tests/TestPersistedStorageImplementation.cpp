/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Implementation of a simple std::map based persisted store.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <platform/PersistedStorage.h>
#include <support/Base64.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>

#include "TestPersistedStorageImplementation.h"

using namespace chip::ArgParser;

std::map<std::string, std::string> sPersistentStore;

FILE * sPersistentStoreFile = nullptr;

namespace chip {
namespace Platform {
namespace PersistedStorage {

static void RemoveEndOfLineSymbol(char * str)
{
    size_t len = strlen(str) - 1;
    if (str[len] == '\n')
        str[len] = '\0';
}

static CHIP_ERROR GetCounterValueFromFile(const char * aKey, uint32_t & aValue)
{
    char key[CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH];
    char value[CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH];

    rewind(sPersistentStoreFile);

    while (fgets(key, sizeof(key), sPersistentStoreFile) != nullptr)
    {
        RemoveEndOfLineSymbol(key);

        if (strcmp(key, aKey) == 0)
        {
            if (fgets(value, sizeof(value), sPersistentStoreFile) == nullptr)
            {
                return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
            }
            RemoveEndOfLineSymbol(value);

            if (!ParseInt(value, aValue, 0))
                return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

static CHIP_ERROR SaveCounterValueToFile(const char * aKey, uint32_t aValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int res;
    char key[CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH];
    char value[CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH];

    snprintf(value, sizeof(value), "0x%08X\n", aValue);

    rewind(sPersistentStoreFile);

    // Find the stored counter value location in the file.
    while (fgets(key, sizeof(key), sPersistentStoreFile) != nullptr)
    {
        RemoveEndOfLineSymbol(key);

        // If value is found in the file then override it.
        if (strcmp(key, aKey) == 0)
        {
            res = fputs(value, sPersistentStoreFile);
            VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

            ExitNow();
        }
    }

    // If value not found in the file then write the counter key and
    // the counter value to the end of the file.
    res = fputs(aKey, sPersistentStoreFile);
    VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    res = fputs("\n", sPersistentStoreFile);
    VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    res = fputs(value, sPersistentStoreFile);
    VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

exit:
    fflush(sPersistentStoreFile);
    return err;
}

CHIP_ERROR Read(const char * aKey, uint32_t & aValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::map<std::string, std::string>::iterator it;

    VerifyOrReturnError(aKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(aKey) <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    if (sPersistentStoreFile)
    {
        err = GetCounterValueFromFile(aKey, aValue);
    }
    else
    {
        it = sPersistentStore.find(aKey);
        VerifyOrReturnError(it != sPersistentStore.end(), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        size_t aValueLength =
            Base64Decode(it->second.c_str(), static_cast<uint16_t>(it->second.length()), reinterpret_cast<uint8_t *>(&aValue));
        VerifyOrReturnError(aValueLength == sizeof(uint32_t), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    }

    return err;
}

CHIP_ERROR Write(const char * aKey, uint32_t aValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(aKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(aKey) <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    if (sPersistentStoreFile)
    {
        err = SaveCounterValueToFile(aKey, aValue);
    }
    else
    {
        char encodedValue[BASE64_ENCODED_LEN(sizeof(uint32_t)) + 1];

        memset(encodedValue, 0, sizeof(encodedValue));
        Base64Encode(reinterpret_cast<uint8_t *>(&aValue), sizeof(aValue), encodedValue);

        sPersistentStore[aKey] = encodedValue;
    }

    return err;
}

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip
