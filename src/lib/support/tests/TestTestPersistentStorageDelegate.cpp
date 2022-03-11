/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>

#include <cstring>
#include <nlunit-test.h>

using namespace chip;

namespace {

void TestBasicApi(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storage;

    uint8_t buf[16];
    uint16_t size = sizeof(buf);

    // Key not there
    CHIP_ERROR err;
    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));

    err = storage.SyncDeleteKeyValue("roboto");
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Add basic key, read it back, erase it
    const char* kStringValue1 = "abcd";
    err = storage.SyncSetKeyValue("roboto", kStringValue1, static_cast<uint16_t>(strlen(kStringValue1)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue1));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue1, strlen(kStringValue1)));

    err = storage.SyncDeleteKeyValue("roboto");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));

    // Validate adding 2 different keys
    const char* kStringValue2 = "0123abcd";
    const char* kStringValue3 = "cdef89";
    err = storage.SyncSetKeyValue("key2", kStringValue2, static_cast<uint16_t>(strlen(kStringValue2)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = storage.SyncSetKeyValue("key3", kStringValue3, static_cast<uint16_t>(strlen(kStringValue3)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Read them back

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err = storage.SyncGetKeyValue("key3", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue3));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue3, strlen(kStringValue3)));

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));

    // Read in too small a buffer
    memset(&buf[0], 0, sizeof(buf));
    size = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Make sure we get told of the right amount to read
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));

    // Read in too small a buffer, which is nullptr (i.e. just try to find if key exists)
    size = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    err = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));

    // Read in too small a buffer, which is not nullptr (i.e. just try to find if key exists)
    size = 0;
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t all_zeroes[sizeof(buf)];
    memset(&buf[0], 0, sizeof(buf));
    memset(&all_zeroes[0], 0, sizeof(all_zeroes));

    // Key not found never cares about output args
    size = 0;
    err = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    err = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    size = static_cast<uint16_t>(sizeof(buf));
    err = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], size));

    // Attempt an empty key write with either nullptr or zero size works
    err = storage.SyncSetKeyValue("key2", kStringValue2, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    size = 0;
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    err = storage.SyncSetKeyValue("key2", nullptr, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    size = 0;
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    // Can delete empty key
    err = storage.SyncDeleteKeyValue("key2");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    size = static_cast<uint16_t>(sizeof(buf));
    err = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));
}

const nlTest sTests[] = { NL_TEST_DEF("Test basic API", TestBasicApi), NL_TEST_SENTINEL() };

} // namespace

int TestTestPersistentStorageDelegate(void)
{
    nlTestSuite theSuite = { "TestPersistentStorageDelegate tests", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTestPersistentStorageDelegate);
