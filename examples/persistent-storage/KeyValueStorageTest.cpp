/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "KeyValueStorageTest.h"

#include <cstring>
#include <string>

#include <platform/KeyValueStoreManager.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::DeviceLayer::PersistedStorage;

#define RUN_TEST(test_result)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        const CHIP_ERROR temp_test_result = test_result;                                                                           \
        if (temp_test_result != CHIP_NO_ERROR)                                                                                     \
        {                                                                                                                          \
            char error_str[255];                                                                                                   \
            chip::FormatCHIPError(error_str, sizeof(error_str), temp_test_result);                                                 \
            ChipLogError(NotSpecified, "%s: FAILED %" PRId32 " [%s]", #test_result, temp_test_result,                              \
                         chip::ErrorStr(temp_test_result));                                                                        \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            ChipLogProgress(NotSpecified, "%s: PASSED", #test_result);                                                             \
        }                                                                                                                          \
    } while (0)

namespace chip {
namespace {

CHIP_ERROR TestEmptyString()
{
    const char * kTestKey   = "str_key";
    const char kTestValue[] = "";
    char read_value[sizeof(kTestValue)];
    size_t read_size;
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, kTestValue));
    ReturnErrorOnFailure(KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size));
    ReturnErrorCodeIf(strcmp(kTestValue, read_value) != 0, CHIP_ERROR_INTERNAL);
    ReturnErrorCodeIf(read_size != sizeof(kTestValue), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestString()
{
    const char * kTestKey   = "str_key";
    const char kTestValue[] = "test_value";
    char read_value[sizeof(kTestValue)];
    size_t read_size;
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, kTestValue));
    ReturnErrorOnFailure(KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size));
    ReturnErrorCodeIf(strcmp(kTestValue, read_value) != 0, CHIP_ERROR_INTERNAL);
    ReturnErrorCodeIf(read_size != sizeof(kTestValue), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestUint32()
{
    const char * kTestKey = "uint32_key";
    uint32_t kTestValue   = 5;
    uint32_t read_value;
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, kTestValue));
    ReturnErrorOnFailure(KeyValueStoreMgr().Get(kTestKey, &read_value));
    ReturnErrorCodeIf(kTestValue != read_value, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestArray()
{
    const char * kTestKey  = "array_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    uint32_t read_value[5];
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, kTestValue));
    ReturnErrorOnFailure(KeyValueStoreMgr().Get(kTestKey, &read_value));
    ReturnErrorCodeIf(memcmp(kTestValue, read_value, sizeof(kTestValue)) != 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestStruct()
{
    struct SomeStruct
    {
        uint8_t value1;
        uint32_t value2;
    };
    const char * kTestKey = "struct_key";
    SomeStruct kTestValue{ value1 : 1, value2 : 2 };
    SomeStruct read_value;
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, kTestValue));
    ReturnErrorOnFailure(KeyValueStoreMgr().Get(kTestKey, &read_value));
    ReturnErrorCodeIf(kTestValue.value1 != read_value.value1, CHIP_ERROR_INTERNAL);
    ReturnErrorCodeIf(kTestValue.value2 != read_value.value2, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestUpdateValue()
{
    const char * kTestKey = "update_key";
    uint32_t read_value;
    for (size_t i = 0; i < 10; i++)
    {
        ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, i));
        ReturnErrorOnFailure(KeyValueStoreMgr().Get(kTestKey, &read_value));
        ReturnErrorCodeIf(i != read_value, CHIP_ERROR_INTERNAL);
    }
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestMultiRead()
{
    const char * kTestKey  = "multi_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kTestKey, kTestValue));
    for (size_t i = 0; i < 5; i++)
    {
        uint32_t read_value;
        size_t read_size;
        // Returns buffer too small for all but the last read.
        CHIP_ERROR error = KeyValueStoreMgr().Get(kTestKey, &read_value, sizeof(read_value), &read_size, i * sizeof(uint32_t));
        ReturnErrorCodeIf(error != (i < 4 ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR), error);
        ReturnErrorCodeIf(read_size != sizeof(read_value), CHIP_ERROR_INTERNAL);
        ReturnErrorCodeIf(kTestValue[i] != read_value, CHIP_ERROR_INTERNAL);
    }
    ReturnErrorOnFailure(KeyValueStoreMgr().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

} // namespace

void RunKvsTest(TestConfigurations test_config)
{
    RUN_TEST(TestEmptyString());
    RUN_TEST(TestString());
    RUN_TEST(TestUint32());
    RUN_TEST(TestArray());
    RUN_TEST(TestStruct());
    RUN_TEST(TestUpdateValue());
    if (test_config != SKIP_MULTI_READ_TEST)
    {
        RUN_TEST(TestMultiRead());
    }
}

} // namespace chip
