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

#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

void TestDefaultProviders(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    const auto makeProvider = [](FabricIndex fabric, NodeId nodeId, EndpointId endpointId) {
        OTARequestorStorage::ProviderLocationType provider;
        provider.fabricIndex    = fabric;
        provider.providerNodeID = nodeId;
        provider.endpoint       = endpointId;
        return provider;
    };

    ProviderLocationList providers = {};
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == providers.Add(makeProvider(FabricIndex(1), NodeId(0x11111111), EndpointId(1))));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == providers.Add(makeProvider(FabricIndex(2), NodeId(0x22222222), EndpointId(2))));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == providers.Add(makeProvider(FabricIndex(3), NodeId(0x33333333), EndpointId(3))));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreDefaultProviders(providers));

    providers = {};
    NL_TEST_ASSERT(inSuite, !providers.Begin().Next());
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadDefaultProviders(providers));

    auto provider = providers.Begin();
    bool hasNext;

    NL_TEST_ASSERT(inSuite, hasNext = provider.Next());

    if (hasNext)
    {
        NL_TEST_ASSERT(inSuite, provider.GetValue().fabricIndex == 1);
        NL_TEST_ASSERT(inSuite, provider.GetValue().providerNodeID == 0x11111111);
        NL_TEST_ASSERT(inSuite, provider.GetValue().endpoint == 1);
    }

    NL_TEST_ASSERT(inSuite, hasNext = provider.Next());

    if (hasNext)
    {
        NL_TEST_ASSERT(inSuite, provider.GetValue().fabricIndex == 2);
        NL_TEST_ASSERT(inSuite, provider.GetValue().providerNodeID == 0x22222222);
        NL_TEST_ASSERT(inSuite, provider.GetValue().endpoint == 2);
    }

    NL_TEST_ASSERT(inSuite, hasNext = provider.Next());

    if (hasNext)
    {
        NL_TEST_ASSERT(inSuite, provider.GetValue().fabricIndex == 3);
        NL_TEST_ASSERT(inSuite, provider.GetValue().providerNodeID == 0x33333333);
        NL_TEST_ASSERT(inSuite, provider.GetValue().endpoint == 3);
    }

    NL_TEST_ASSERT(inSuite, !provider.Next());
}

void TestDefaultProvidersEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    ProviderLocationList providers = {};

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == otaStorage.LoadDefaultProviders(providers));
    NL_TEST_ASSERT(inSuite, !providers.Begin().Next());
}

void TestCurrentProviderLocation(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    OTARequestorStorage::ProviderLocationType provider;
    provider.fabricIndex    = 1;
    provider.providerNodeID = 0x12344321;
    provider.endpoint       = 10;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreCurrentProviderLocation(provider));

    provider = {};

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadCurrentProviderLocation(provider));
    NL_TEST_ASSERT(inSuite, provider.fabricIndex == 1);
    NL_TEST_ASSERT(inSuite, provider.providerNodeID == 0x12344321);
    NL_TEST_ASSERT(inSuite, provider.endpoint == 10);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.ClearCurrentProviderLocation());
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR != otaStorage.LoadCurrentProviderLocation(provider));
}

void TestUpdateToken(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    constexpr size_t updateTokenLength = 32;
    uint8_t updateTokenBuffer[updateTokenLength];
    ByteSpan updateToken(updateTokenBuffer);

    for (uint8_t i = 0; i < updateTokenLength; ++i)
        updateTokenBuffer[i] = i;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreUpdateToken(updateToken));

    uint8_t readBuffer[updateTokenLength + 10];
    MutableByteSpan readUpdateToken(readBuffer);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadUpdateToken(readUpdateToken));
    NL_TEST_ASSERT(inSuite, readUpdateToken.size() == updateTokenLength);

    for (uint8_t i = 0; i < updateTokenLength; ++i)
        NL_TEST_ASSERT(inSuite, readBuffer[i] == i);

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.ClearUpdateToken());
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR != otaStorage.LoadUpdateToken(readUpdateToken));
}

void TestCurrentUpdateState(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    OTARequestorStorage::OTAUpdateStateEnum updateState = OTARequestorStorage::OTAUpdateStateEnum::kApplying;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreCurrentUpdateState(updateState));

    updateState = OTARequestorStorage::OTAUpdateStateEnum::kUnknown;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadCurrentUpdateState(updateState));
    NL_TEST_ASSERT(inSuite, updateState == OTARequestorStorage::OTAUpdateStateEnum::kApplying);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.ClearCurrentUpdateState());
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR != otaStorage.LoadCurrentUpdateState(updateState));
}

void TestTargetVersion(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    uint32_t targetVersion = 2;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.StoreTargetVersion(targetVersion));

    targetVersion = 0;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.LoadTargetVersion(targetVersion));
    NL_TEST_ASSERT(inSuite, targetVersion == 2);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == otaStorage.ClearTargetVersion());
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR != otaStorage.LoadTargetVersion(targetVersion));
}

const nlTest sTests[] = { NL_TEST_DEF("Test default providers", TestDefaultProviders),
                          NL_TEST_DEF("Test default providers (empty list)", TestDefaultProvidersEmpty),
                          NL_TEST_DEF("Test current provider location", TestCurrentProviderLocation),
                          NL_TEST_DEF("Test update token", TestUpdateToken),
                          NL_TEST_DEF("Test current update state", TestCurrentUpdateState),
                          NL_TEST_DEF("Test target version", TestTargetVersion),
                          NL_TEST_SENTINEL() };

int TestSetup(void * inContext)
{
    return SUCCESS;
}

int TestTearDown(void * inContext)
{
    return SUCCESS;
}

} // namespace

int TestDefaultOTARequestorStorage()
{
    nlTestSuite theSuite = { "OTA Storage tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDefaultOTARequestorStorage)
