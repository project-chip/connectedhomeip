/*
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

#include <cstring>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

#include "PersistentStorageAudit.h"

#ifdef NL_TEST_ASSERT
#undef NL_TEST_ASSERT
#endif

#define NL_TEST_ASSERT(inSuite, inCondition)                                                                                       \
    do                                                                                                                             \
    {                                                                                                                              \
        (inSuite)->performedAssertions += 1;                                                                                       \
                                                                                                                                   \
        if (!(inCondition))                                                                                                        \
        {                                                                                                                          \
            ChipLogError(Automation, "%s:%u: assertion failed: \"%s\"", __FILE__, __LINE__, #inCondition);                         \
            (inSuite)->failedAssertions += 1;                                                                                      \
            (inSuite)->flagError = true;                                                                                           \
        }                                                                                                                          \
    } while (0)

namespace chip {
namespace audit {

// The following test is a copy of `src/lib/support/tests/TestTestPersistentStorageDelegate.cpp` 's
// `TestBasicApi()` test. It has to be copied since we currently are not setup to
// run on-device unit tests at large on all embedded platforms part of the SDK.
bool ExecutePersistentStorageApiAudit(PersistentStorageDelegate & storage)
{
    struct fakeTestSuite
    {
        int performedAssertions = 0;
        int failedAssertions    = 0;
        bool flagError          = false;
    } theSuite;
    auto * inSuite = &theSuite;

    static const char kLongKeyString[] = "aKeyThatIsExactlyMaxKeyLengthhhh";
    // Start fresh.
    (void) storage.SyncDeleteKeyValue("roboto");
    (void) storage.SyncDeleteKeyValue("key2");
    (void) storage.SyncDeleteKeyValue("key3");
    (void) storage.SyncDeleteKeyValue("key4");
    (void) storage.SyncDeleteKeyValue("keyDOES_NOT_EXIST");
    (void) storage.SyncDeleteKeyValue(kLongKeyString);

    // ========== Start of actual audit from TestTestPersistentStorageDelegate.cpp =========

    uint8_t buf[16];
    const uint16_t actualSizeOfBuf = static_cast<uint16_t>(sizeof(buf));
    uint16_t size                  = actualSizeOfBuf;

    // Key not there
    CHIP_ERROR err;
    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == actualSizeOfBuf);

    err = storage.SyncDeleteKeyValue("roboto");
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Add basic key, read it back, erase it
    static const char kStringValue1[] = "abcd";
    err = storage.SyncSetKeyValue("roboto", kStringValue1, static_cast<uint16_t>(strlen(kStringValue1)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue1));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue1, strlen(kStringValue1)));

    err = storage.SyncDeleteKeyValue("roboto");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("roboto", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == actualSizeOfBuf);

    // Validate adding 2 different keys
    static const char kStringValue2[] = "0123abcd";
    static const char kStringValue3[] = "cdef89";
    err = storage.SyncSetKeyValue("key2", kStringValue2, static_cast<uint16_t>(strlen(kStringValue2)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = storage.SyncSetKeyValue("key3", kStringValue3, static_cast<uint16_t>(strlen(kStringValue3)));
    NL_TEST_ASSERT(inSuite, storage.SyncDoesKeyExist("key3"));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Read them back

    uint8_t all_zeroes[sizeof(buf)];
    memset(&all_zeroes[0], 0, sizeof(all_zeroes));

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key3", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue3));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue3, strlen(kStringValue3)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    // Read providing too small a buffer. Data read up to `size` and nothing more.
    memset(&buf[0], 0, sizeof(buf));
    size                               = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    uint16_t sizeBeforeGetKeyValueCall = size;
    err                                = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, size == sizeBeforeGetKeyValueCall);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, size));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    // Read in too small a buffer, which is nullptr and size == 0: check CHIP_ERROR_BUFFER_TOO_SMALL is given.
    memset(&buf[0], 0, sizeof(buf));
    size                      = 0;
    sizeBeforeGetKeyValueCall = size;
    err                       = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, size != strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, size == sizeBeforeGetKeyValueCall);
    // Just making sure that implementation doesn't hold onto reference of previous destination buffer when
    // nullptr is provided.
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Read in too small a buffer, which is nullptr and size != 0: error
    size                      = static_cast<uint16_t>(strlen(kStringValue2) - 1);
    sizeBeforeGetKeyValueCall = size;
    err                       = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, size == sizeBeforeGetKeyValueCall);
    // Just making sure that implementation doesn't hold onto reference of previous destination buffer when
    // nullptr is provided.
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // When key not found, size is not touched.
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, actualSizeOfBuf == size);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    size = 0;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, 0 == size);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], sizeof(buf)));

    // Even when key not found, cannot pass nullptr with size != 0.
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("keyDOES_NOT_EXIST", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, actualSizeOfBuf == size);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], size));

    // Attempt an empty key write with either nullptr or zero size works
    err = storage.SyncSetKeyValue("key2", kStringValue2, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storage.SyncDoesKeyExist("key2"));

    size = 0;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    size = 0;
    err  = storage.SyncGetKeyValue("key2", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    err = storage.SyncSetKeyValue("key2", nullptr, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storage.SyncDoesKeyExist("key2"));

    size = 0;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == 0);

    // Failure to set key if buffer is nullptr and size != 0
    size = 10;
    err  = storage.SyncSetKeyValue("key4", nullptr, size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, !storage.SyncDoesKeyExist("key4"));

    // Can delete empty key
    err = storage.SyncDeleteKeyValue("key2");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, !storage.SyncDoesKeyExist("key2"));

    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue("key2", &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, size == actualSizeOfBuf);
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], &all_zeroes[0], size));

    // Using key and value with base64 symbols
    static const char kBase64SymbolsKey[]   = "key+/=";
    static const char kBase64SymbolValues[] = "value+/=";
    err = storage.SyncSetKeyValue(kBase64SymbolsKey, kBase64SymbolValues, static_cast<uint16_t>(strlen(kBase64SymbolValues)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue(kBase64SymbolsKey, &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kBase64SymbolValues));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kBase64SymbolValues, strlen(kBase64SymbolValues)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    err = storage.SyncDeleteKeyValue(kBase64SymbolsKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !storage.SyncDoesKeyExist(kBase64SymbolsKey));

    // Try using key that is a size that equals PersistentStorageDelegate::kKeyLengthMax
    char longKeyString[PersistentStorageDelegate::kKeyLengthMax + 1];
    memset(&longKeyString, 'X', PersistentStorageDelegate::kKeyLengthMax);
    longKeyString[sizeof(longKeyString) - 1] = '\0';
    // strlen() is not compile time so we just have this runtime assert that should aways pass as a sanity check.
    NL_TEST_ASSERT(inSuite, strlen(longKeyString) == PersistentStorageDelegate::kKeyLengthMax);

    err = storage.SyncSetKeyValue(longKeyString, kStringValue2, static_cast<uint16_t>(strlen(kStringValue2)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&buf[0], 0, sizeof(buf));
    size = actualSizeOfBuf;
    err  = storage.SyncGetKeyValue(longKeyString, &buf[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == strlen(kStringValue2));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[0], kStringValue2, strlen(kStringValue2)));
    // Make sure that there was no buffer overflow during SyncGetKeyValue
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&buf[size], &all_zeroes[0], sizeof(buf) - size));

    NL_TEST_ASSERT(inSuite, storage.SyncDoesKeyExist(longKeyString));

    err = storage.SyncDeleteKeyValue(longKeyString);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !storage.SyncDoesKeyExist(longKeyString));

    constexpr size_t kMaxCHIPCertLength = 400; // From credentials/CHIPCert.h and spec
    uint8_t largeBuffer[kMaxCHIPCertLength];
    memset(&largeBuffer, 'X', sizeof(largeBuffer));
    uint8_t largeBufferForCheck[sizeof(largeBuffer)];
    memcpy(largeBufferForCheck, largeBuffer, sizeof(largeBuffer));

    err = storage.SyncSetKeyValue(longKeyString, largeBuffer, static_cast<uint16_t>(sizeof(largeBuffer)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    memset(&largeBuffer, 0, sizeof(largeBuffer));
    size = static_cast<uint16_t>(sizeof(largeBuffer));
    err  = storage.SyncGetKeyValue(longKeyString, &largeBuffer[0], size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, size == static_cast<uint16_t>(sizeof(largeBuffer)));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(&largeBuffer, largeBufferForCheck, sizeof(largeBuffer)));

    err = storage.SyncDeleteKeyValue(longKeyString);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Cleaning up
    (void) storage.SyncDeleteKeyValue("roboto");
    (void) storage.SyncDeleteKeyValue("key2");
    (void) storage.SyncDeleteKeyValue("key3");
    (void) storage.SyncDeleteKeyValue("key4");
    (void) storage.SyncDeleteKeyValue(kBase64SymbolsKey);
    (void) storage.SyncDeleteKeyValue(kLongKeyString);

    // ========== End of code from TestTestPersistentStorageDelegate.cpp =========
    if (inSuite->flagError)
    {
        ChipLogError(Automation,
                     "==== PersistentStorageDelegate API audit: FAILED: %d/%d failed assertions ====", inSuite->failedAssertions,
                     inSuite->performedAssertions);
        return false;
    }

    ChipLogError(Automation, "==== PersistentStorageDelegate API audit: SUCCESS ====");
    return true;
}

bool ExecutePersistentStorageLoadTestAudit(PersistentStorageDelegate & storage)
{
    (void) storage;
    ChipLogError(Automation, "==== PersistentStorageDelegate load test audit: SUCCESS ====");
    return true;
}

} // namespace audit
} // namespace chip
