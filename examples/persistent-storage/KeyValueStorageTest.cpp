/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <string>

#include "KeyValueStorage.h"
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Platform::PersistedStorage;

#define RETURN_CHIP_ERROR_IF(cond, chip_error)                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        if (cond)                                                                                                                  \
        {                                                                                                                          \
            return (chip_error);                                                                                                   \
        }                                                                                                                          \
    } while (0)

#define RETURN_IF_CHIP_ERROR(expr)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        const CHIP_ERROR temp_chip_error = (expr);                                                                                 \
        RETURN_CHIP_ERROR_IF(temp_chip_error != CHIP_NO_ERROR, temp_chip_error);                                                   \
    } while (0)

#define RUN_TEST(test_result)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        const CHIP_ERROR temp_test_result = test_result;                                                                           \
        if (temp_test_result != CHIP_NO_ERROR)                                                                                     \
        {                                                                                                                          \
            char error_str[255];                                                                                                   \
            chip::FormatCHIPError(error_str, sizeof(error_str), temp_test_result);                                                 \
            ChipLogError(NotSpecified, "%s: FAILED %d [%s]", #test_result, temp_test_result, chip::ErrorStr(temp_test_result));    \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            ChipLogProgress(NotSpecified, "%s: PASSED", #test_result);                                                             \
        }                                                                                                                          \
    } while (0)

namespace chip {
namespace {

CHIP_ERROR TestString()
{
    const char * kTestKey   = "str_key";
    const char kTestValue[] = "test_value";
    char read_value[sizeof(kTestValue)];
    size_t read_size;
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Put(kTestKey, kTestValue));
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Get(kTestKey, read_value, sizeof(read_value), &read_size));
    RETURN_CHIP_ERROR_IF(strcmp(kTestValue, read_value) != 0, CHIP_ERROR_INTERNAL);
    RETURN_CHIP_ERROR_IF(read_size != sizeof(kTestValue), CHIP_ERROR_INTERNAL);
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestUint32()
{
    const char * kTestKey = "uint32_key";
    uint32_t kTestValue   = 5;
    uint32_t read_value;
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Put(kTestKey, kTestValue));
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Get(kTestKey, &read_value));
    RETURN_CHIP_ERROR_IF(kTestValue != read_value, CHIP_ERROR_INTERNAL);
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestArray()
{
    const char * kTestKey  = "array_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    uint32_t read_value[5];
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Put(kTestKey, kTestValue));
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Get(kTestKey, &read_value));
    RETURN_CHIP_ERROR_IF(memcmp(kTestValue, read_value, sizeof(kTestValue)) != 0, CHIP_ERROR_INTERNAL);
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Delete(kTestKey));
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
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Put(kTestKey, kTestValue));
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Get(kTestKey, &read_value));
    RETURN_CHIP_ERROR_IF(kTestValue.value1 != read_value.value1, CHIP_ERROR_INTERNAL);
    RETURN_CHIP_ERROR_IF(kTestValue.value2 != read_value.value2, CHIP_ERROR_INTERNAL);
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestUpdateValue()
{
    const char * kTestKey = "update_key";
    uint32_t read_value;
    for (size_t i = 0; i < 10; i++)
    {
        RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Put(kTestKey, i));
        RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Get(kTestKey, &read_value));
        RETURN_CHIP_ERROR_IF(i != read_value, CHIP_ERROR_INTERNAL);
    }
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestMultiRead()
{
    const char * kTestKey  = "multi_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Put(kTestKey, kTestValue));
    for (size_t i = 0; i < 5; i++)
    {
        uint32_t read_value;
        size_t read_size;
        // Returns buffer too small for all but the last read.
        CHIP_ERROR error =
            ChipKeyValueStoreInstance().Get(kTestKey, &read_value, sizeof(read_value), &read_size, i * sizeof(uint32_t));
        RETURN_CHIP_ERROR_IF(error != (i < 4 ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR), error);
        RETURN_CHIP_ERROR_IF(read_size != sizeof(read_value), CHIP_ERROR_INTERNAL);
        RETURN_CHIP_ERROR_IF(kTestValue[i] != read_value, CHIP_ERROR_INTERNAL);
    }
    RETURN_IF_CHIP_ERROR(ChipKeyValueStoreInstance().Delete(kTestKey));
    return CHIP_NO_ERROR;
}

} // namespace

void RunKvsTest()
{
    RUN_TEST(TestString());
    RUN_TEST(TestUint32());
    RUN_TEST(TestArray());
    RUN_TEST(TestStruct());
    RUN_TEST(TestUpdateValue());
    RUN_TEST(TestMultiRead());
}

} // namespace chip
