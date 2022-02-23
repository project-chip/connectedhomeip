/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/GenericOTARequestorStorage.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

void TestProviderLocation(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    GenericOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    OTARequestorStorage::ProviderLocationType provider;
    provider.fabricIndex    = 1;
    provider.providerNodeID = 0x12344321;
    provider.endpoint       = 10;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreProviderLocation(provider));

    provider = {};
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadProviderLocation(provider));
    NL_TEST_ASSERT(inSuite, provider.fabricIndex == 1);
    NL_TEST_ASSERT(inSuite, provider.providerNodeID == 0x12344321);
    NL_TEST_ASSERT(inSuite, provider.endpoint == 10);
}

void TestUpdateToken(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    GenericOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    constexpr size_t updateTokenLength = 32;
    uint8_t updateToken[updateTokenLength];
    ByteSpan updateTokenSpan(updateToken);

    for (uint8_t i = 0; i < updateTokenLength; ++i)
        updateToken[i] = i;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreUpdateToken(updateTokenSpan));

    uint8_t readBuffer[updateTokenLength + 10];
    MutableByteSpan readUpdateTokenSpan(readBuffer);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadUpdateToken(readUpdateTokenSpan));
    NL_TEST_ASSERT(inSuite, readUpdateTokenSpan.size() == updateTokenLength);

    for (uint8_t i = 0; i < updateTokenLength; ++i)
        NL_TEST_ASSERT(inSuite, readBuffer[i] == i);
}

const nlTest sTests[] = {

    NL_TEST_DEF("Test provider location", TestProviderLocation), NL_TEST_DEF("Test update token", TestUpdateToken),
    NL_TEST_SENTINEL()
};

int TestSetup(void * inContext)
{
    return SUCCESS;
}

int TestTearDown(void * inContext)
{
    return SUCCESS;
}

} // namespace

int TestGenericOTARequestorStorage()
{
    nlTestSuite theSuite = { "OTA Storage tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestGenericOTARequestorStorage)
