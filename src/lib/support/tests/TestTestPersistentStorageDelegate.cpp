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

#include <array>
#include <cstring>
#include <set>
#include <string>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using namespace chip;

namespace {

template <class T, size_t N>
bool SetMatches(const std::set<T> & set, const std::array<T, N> expectedContents)
{
    if (set.size() != N)
    {
        return false;
    }

    for (const auto & item : expectedContents)
    {
        if (set.find(item) == set.cend())
        {
            return false;
        }
    }

    return true;
}

TEST(TestTestPersistentStorageDelegate, TestBasicApi)
{
    TestPersistentStorageDelegate storage;

    uint8_t buf[16];
    const uint16_t actualSizeOfBuf = static_cast<uint16_t>(sizeof(buf));
    uint16_t size                  = actualSizeOfBuf;

    // Key not there
    CHIP_ERROR err;
    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(size, actualSizeOfBuf);

    EXPECT_EQ(storage.GetNumKeys(), 0u);

    err = storage.SyncDeleteKeyValue("roboto");
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Add basic key, read it back, erase it
    static const char kStringValue1[] = "abcd";
    err = storage.SyncSetKeyValue("roboto", kStringValue1, static_cast<uint16_t>(strlen(kStringValue1)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, strlen(kStringValue1));
    EXPECT_EQ(0, memcmp(&buf[0], kStringValue1, strlen(kStringValue1)));

    err = storage.SyncDeleteKeyValue("roboto");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(size, actualSizeOfBuf);

    // Validate adding 2 different keys
    static const char kStringValue2[] = "0123abcd";
    static const char kStringValue3[] = "cdef89";
    err = storage.SyncSetKeyValue("key2", kStringValue2, static_cast<uint16_t>(strlen(kStringValue2)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = storage.SyncSetKeyValue("key3", kStringValue3, static_cast<uint16_t>(strlen(kStringValue3)));
    EXPECT_TRUE(storage.SyncDoesKeyExist("key3"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(storage.GetNumKeys(), 2u);
    auto keys = storage.GetKeys();
    std::array<std::string, 2> kExpectedKeys{ "key2", "key3" };
    EXPECT_EQ(SetMatches(keys, kExpectedKeys), true);

    // Read them back

    uint8_t all_zeroes[sizeof(buf)];
    memset(&all_zeroes[0], 0, sizeof(all_zeroes));

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, strlen(kStringValue2));
    EXPECT_EQ(0, memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    EXPECT_EQ(0, memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key3", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, strlen(kStringValue3));
    EXPECT_EQ(0, memcmp(&buf[0], kStringValue3, strlen(kStringValue3)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    EXPECT_EQ(0, memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    // Read providing too small a buffer. Data read up to `size` and nothing more.
    memset(&buf[0], 0, sizeof(buf));
    size                               = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    uint16_t sizeBeforeGetKeyValueCall = size;
    err                                = storage.SyncGetKeyValue("key2", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_EQ(size, sizeBeforeGetKeyValueCall);
    EXPECT_EQ(0, memcmp(&buf[0], kStringValue2, size));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    EXPECT_EQ(0, memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    // Read in too small a buffer, which is nullptr and size == 0: check CHIP_ERROR_BUFFER_TOO_SMALL is given.
    memset(&buf[0], 0, sizeof(buf));
    size                      = 0;
    sizeBeforeGetKeyValueCall = size;
    err                       = storage.SyncGetKeyValue("key2", nullptr, size);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_NE(size, strlen(kStringValue2));
    EXPECT_EQ(size, sizeBeforeGetKeyValueCall);
    // Just making sure that implementation doesn't hold onto reference of previous destination buffer when
    // nullptr is provided.
    EXPECT_EQ(0, memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Read in too small a buffer, which is nullptr and size != 0: error
    size                      = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    sizeBeforeGetKeyValueCall = size;
    err                       = storage.SyncGetKeyValue("key2", nullptr, size);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(size, sizeBeforeGetKeyValueCall);
    // Just making sure that implementation doesn't hold onto reference of previous destination buffer when
    // nullptr is provided.
    EXPECT_EQ(0, memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // When key not found, size is not touched.
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(actualSizeOfBuf, size);
    EXPECT_EQ(0, memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    size = 0;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(0, size);
    EXPECT_EQ(0, memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Even when key not found, cannot pass nullptr with size != 0.
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(actualSizeOfBuf, size);
    EXPECT_EQ(0, memcmp(&buf[0], &all_zeroes[0], size));

    // Attempt an empty key write with either nullptr or zero size works
    err = storage.SyncSetKeyValue("key2", kStringValue2, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(storage.SyncDoesKeyExist("key2"));

    size = 0;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, 0u);

    size = 0;
    err  = storage.SyncGetKeyValue("key2", nullptr, size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, 0u);

    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, 0u);

    err = storage.SyncSetKeyValue("key2", nullptr, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(storage.SyncDoesKeyExist("key2"));

    size = 0;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, 0u);

    // Failure to set key if buffer is nullptr and size != 0
    size = 10;
    err  = storage.SyncSetKeyValue("key4", nullptr, size);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(storage.SyncDoesKeyExist("key4"));

    // Can delete empty key
    err = storage.SyncDeleteKeyValue("key2");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_FALSE(storage.SyncDoesKeyExist("key2"));

    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(size, actualSizeOfBuf);
    EXPECT_EQ(0, memcmp(&buf[0], &all_zeroes[0], size));

    // Using key and value with base64 symbols
    static const char kBase64SymbolsKey[]   = "key+/=";
    static const char kBase64SymbolValues[] = "value+/=";
    err = storage.SyncSetKeyValue(kBase64SymbolsKey, kBase64SymbolValues, static_cast<uint16_t>(strlen(kBase64SymbolValues)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue(kBase64SymbolsKey, &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, strlen(kBase64SymbolValues));
    EXPECT_EQ(0, memcmp(&buf[0], kBase64SymbolValues, strlen(kBase64SymbolValues)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    EXPECT_EQ(0, memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    err = storage.SyncDeleteKeyValue(kBase64SymbolsKey);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(storage.SyncDoesKeyExist(kBase64SymbolsKey));

    // Try using key that is a size that equals PersistentStorageDelegate::kKeyLengthMax
    char longKeyString[PersistentStorageDelegate::kKeyLengthMax + 1];
    memset(&longKeyString, 'X', PersistentStorageDelegate::kKeyLengthMax);
    longKeyString[sizeof(longKeyString) - 1] = '\0';
    // strlen() is not compile time so we just have this runtime assert that should aways pass as a sanity check.
    EXPECT_EQ(strlen(longKeyString), PersistentStorageDelegate::kKeyLengthMax);

    err = storage.SyncSetKeyValue(longKeyString, kStringValue2, static_cast<uint16_t>(strlen(kStringValue2)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue(longKeyString, &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, strlen(kStringValue2));
    EXPECT_EQ(0, memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    EXPECT_EQ(0, memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    EXPECT_TRUE(storage.SyncDoesKeyExist(longKeyString));

    err = storage.SyncDeleteKeyValue(longKeyString);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(storage.SyncDoesKeyExist(longKeyString));

    constexpr size_t kMaxCHIPCertLength = 400; // From credentials/CHIPCert.h and spec
    uint8_t largeBuffer[kMaxCHIPCertLength];
    memset(&largeBuffer, 'X', sizeof(largeBuffer));
    uint8_t largeBufferForCheck[sizeof(largeBuffer)];
    memcpy(largeBufferForCheck, largeBuffer, sizeof(largeBuffer));

    err = storage.SyncSetKeyValue(longKeyString, largeBuffer, static_cast<uint16_t>(sizeof(largeBuffer)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    memset(&largeBuffer, 0, sizeof(largeBuffer));
    size = static_cast<uint16_t>(sizeof(largeBuffer));
    err  = storage.SyncGetKeyValue(longKeyString, &largeBuffer[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, static_cast<uint16_t>(sizeof(largeBuffer)));
    EXPECT_EQ(0, memcmp(&largeBuffer, largeBufferForCheck, sizeof(largeBuffer)));

    err = storage.SyncDeleteKeyValue(longKeyString);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Cleaning up
    err = storage.SyncDeleteKeyValue("key3");
    EXPECT_EQ(storage.GetNumKeys(), 0u);
}

// ClearStorage is not a PersistentStorageDelegate base class method, it only
// appears in the TestPersistentStorageDelegate.
TEST(TestTestPersistentStorageDelegate, TestClearStorage)
{
    TestPersistentStorageDelegate storage;

    uint8_t buf[16];
    uint16_t size = sizeof(buf);

    // Key not there
    EXPECT_EQ(storage.GetNumKeys(), 0u);

    CHIP_ERROR err;
    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(size, sizeof(buf));

    // Add basic key, read it back
    static const char kStringValue1[] = "abcd";
    err = storage.SyncSetKeyValue("roboto", kStringValue1, static_cast<uint16_t>(strlen(kStringValue1)));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storage.GetNumKeys(), 1u);

    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(size, strlen(kStringValue1));
    EXPECT_EQ(0, memcmp(&buf[0], kStringValue1, strlen(kStringValue1)));

    // Clear storage, make sure it's gone
    storage.ClearStorage();

    EXPECT_EQ(storage.GetNumKeys(), 0u);
    memset(&buf[0], 0, sizeof(buf));
    size = sizeof(buf);
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    EXPECT_EQ(size, sizeof(buf));
}

} // namespace
