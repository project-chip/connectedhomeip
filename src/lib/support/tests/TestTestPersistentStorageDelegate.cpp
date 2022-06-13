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

#include <array>
#include <cstring>
#include <set>
#include <string>

#include <nlunit-test.h>

using namespace chip;

namespace {

template <class T, size_t N>
bool SetMatches(const std::set<T> & set, const std::array<T, N> expectedContents)
{
    if (set.size() != N)
    {
        return false;
    }

    for (auto item : expectedContents)
    {
        if (set.find(item) == set.cend())
        {
            return false;
        }
    }

    return true;
}

void TestBasicApi(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storage;

    uint8_t buf[16];
    uint16_t size = sizeof(buf);

    // Key not there
    CHIP_ERROR err;
    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));

    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);

    err = storage.SyncDeleteKeyValue("roboto");
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Add basic key, read it back, erase it
    const char * kStringValue1 = "abcd";
    err                        = storage.SyncSetKeyValue("roboto", kStringValue1, static_cast<uint16_t>(strlen(kStringValue1)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue1));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue1, strlen(kStringValue1)));

    err = storage.SyncDeleteKeyValue("roboto");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));

    // Validate adding 2 different keys
    const char * kStringValue2 = "0123abcd";
    const char * kStringValue3 = "cdef89";
    err                        = storage.SyncSetKeyValue("key2", kStringValue2, static_cast<uint16_t>(strlen(kStringValue2)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = storage.SyncSetKeyValue("key3", kStringValue3, static_cast<uint16_t>(strlen(kStringValue3)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 2);
    auto keys = storage.GetKeys();
    std::array<std::string, 2> kExpectedKeys{ "key2", "key3" };
    NL_TEST_ASSERT(inSuite, SetMatches(keys, kExpectedKeys) == true);

    // Read them back

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("key3", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue3));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue3, strlen(kStringValue3)));

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));

    // Pre-clear buffer to make sure next operations don't change contents
    uint8_t all_zeroes[sizeof(buf)];
    memset(&buf[0], 0, sizeof(buf));
    memset(&all_zeroes[0], 0, sizeof(all_zeroes));

    // Read in too small a buffer: no data read, but correct size given
    memset(&buf[0], 0, sizeof(buf));
    size = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Read in too small a buffer, which is nullptr and size == 0: check correct size given
    size = 0;
    err  = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Read in too small a buffer, which is nullptr and size != 0: error
    size = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    err  = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Read in zero size buffer, which is also nullptr (i.e. just try to find if key exists without
    // using a buffer).
    size = 0;
    err  = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // When key not found, size is not touched.
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, sizeof(buf) == size);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    size = 0;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, 0 == size);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Even when key not found, cannot pass nullptr with size != 0.
    size = static_cast<uint16_t>(sizeof(buf));
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, sizeof(buf) == size);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], size));

    // Attempt an empty key write with either nullptr or zero size works
    err = storage.SyncSetKeyValue("key2", kStringValue2, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    size = 0;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    err = storage.SyncSetKeyValue("key2", nullptr, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    size = 0;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    // Failure to set key if buffer is nullptr and size != 0
    size = 10;
    err  = storage.SyncSetKeyValue("key4", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    // Can delete empty key
    err = storage.SyncDeleteKeyValue("key2");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    size = static_cast<uint16_t>(sizeof(buf));
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));
}

// ClearStorage is not a PersistentStorageDelegate base class method, it only
// appears in the TestPersistentStorageDelegate.
void TestClearStorage(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storage;

    uint8_t buf[16];
    uint16_t size = sizeof(buf);

    // Key not there
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);

    CHIP_ERROR err;
    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));

    // Add basic key, read it back
    const char * kStringValue1 = "abcd";
    err                        = storage.SyncSetKeyValue("roboto", kStringValue1, static_cast<uint16_t>(strlen(kStringValue1)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 1);

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue1));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue1, strlen(kStringValue1)));

    // Clear storage, make sure it's gone
    storage.ClearStorage();

    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);
    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == sizeof(buf));
}

const nlTest sTests[] = { NL_TEST_DEF("Test basic API", TestBasicApi),
                          NL_TEST_DEF("Test ClearStorage method of TestPersistentStorageDelegate", TestClearStorage),
                          NL_TEST_SENTINEL() };

} // namespace

int TestTestPersistentStorageDelegate(void)
{
    nlTestSuite theSuite = { "TestPersistentStorageDelegate tests", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTestPersistentStorageDelegate);
