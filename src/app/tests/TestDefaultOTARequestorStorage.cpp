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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

TEST(TestDefaultOTARequestorStorage, TestDefaultProviders)
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
    EXPECT_EQ(CHIP_NO_ERROR, providers.Add(makeProvider(FabricIndex(1), NodeId(0x11111111), EndpointId(1))));
    EXPECT_EQ(CHIP_NO_ERROR, providers.Add(makeProvider(FabricIndex(2), NodeId(0x22222222), EndpointId(2))));
    EXPECT_EQ(CHIP_NO_ERROR, providers.Add(makeProvider(FabricIndex(3), NodeId(0x33333333), EndpointId(3))));
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.StoreDefaultProviders(providers));

    providers = {};
    EXPECT_FALSE(providers.Begin().Next());
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.LoadDefaultProviders(providers));

    auto provider = providers.Begin();
    bool hasNext;

    EXPECT_TRUE(hasNext = provider.Next());

    if (hasNext)
    {
        EXPECT_EQ(provider.GetValue().fabricIndex, 1);
        EXPECT_EQ(provider.GetValue().providerNodeID, 0x11111111u);
        EXPECT_EQ(provider.GetValue().endpoint, 1);
    }

    EXPECT_TRUE(hasNext = provider.Next());

    if (hasNext)
    {
        EXPECT_EQ(provider.GetValue().fabricIndex, 2);
        EXPECT_EQ(provider.GetValue().providerNodeID, 0x22222222u);
        EXPECT_EQ(provider.GetValue().endpoint, 2);
    }

    EXPECT_TRUE(hasNext = provider.Next());

    if (hasNext)
    {
        EXPECT_EQ(provider.GetValue().fabricIndex, 3);
        EXPECT_EQ(provider.GetValue().providerNodeID, 0x33333333u);
        EXPECT_EQ(provider.GetValue().endpoint, 3);
    }

    EXPECT_FALSE(provider.Next());
}

TEST(TestDefaultOTARequestorStorage, TestDefaultProvidersEmpty)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    ProviderLocationList providers = {};

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, otaStorage.LoadDefaultProviders(providers));
    EXPECT_FALSE(providers.Begin().Next());
}

TEST(TestDefaultOTARequestorStorage, TestDefaultProvidersDuplicated)
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

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, otaStorage.LoadDefaultProviders(providers));
    auto iterator = providers.Begin();
    EXPECT_EQ(false, iterator.Next());

    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_FABRICS; i++)
    {
        EXPECT_EQ(CHIP_NO_ERROR, providers.Add(makeProvider(FabricIndex(1), NodeId(0x11111111), EndpointId(1))));
    }

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.StoreDefaultProviders(providers));

    providers = {};
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.LoadDefaultProviders(providers));

    iterator = providers.Begin();

    // Check provider #1
    EXPECT_EQ(true, iterator.Next());
    OTARequestorStorage::ProviderLocationType provider1 = iterator.GetValue();
    EXPECT_EQ(FabricIndex(1), provider1.fabricIndex);
    EXPECT_EQ(NodeId(0x11111111), provider1.providerNodeID);
    EXPECT_EQ(EndpointId(1), provider1.endpoint);

    EXPECT_EQ(false, iterator.Next());
}

TEST(TestDefaultOTARequestorStorage, TestCurrentProviderLocation)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    OTARequestorStorage::ProviderLocationType provider;
    provider.fabricIndex    = 1;
    provider.providerNodeID = 0x12344321;
    provider.endpoint       = 10;

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.StoreCurrentProviderLocation(provider));

    provider = {};

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.LoadCurrentProviderLocation(provider));
    EXPECT_EQ(provider.fabricIndex, 1);
    EXPECT_EQ(provider.providerNodeID, 0x12344321u);
    EXPECT_EQ(provider.endpoint, 10);
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.ClearCurrentProviderLocation());
    EXPECT_NE(CHIP_NO_ERROR, otaStorage.LoadCurrentProviderLocation(provider));
}

TEST(TestDefaultOTARequestorStorage, TestUpdateToken)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    constexpr size_t updateTokenLength = 32;
    uint8_t updateTokenBuffer[updateTokenLength];
    ByteSpan updateToken(updateTokenBuffer);

    for (uint8_t i = 0; i < updateTokenLength; ++i)
        updateTokenBuffer[i] = i;

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.StoreUpdateToken(updateToken));

    uint8_t readBuffer[updateTokenLength + 10];
    MutableByteSpan readUpdateToken(readBuffer);
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.LoadUpdateToken(readUpdateToken));
    EXPECT_EQ(readUpdateToken.size(), updateTokenLength);

    for (uint8_t i = 0; i < updateTokenLength; ++i)
        EXPECT_EQ(readBuffer[i], i);

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.ClearUpdateToken());
    EXPECT_NE(CHIP_NO_ERROR, otaStorage.LoadUpdateToken(readUpdateToken));
}

TEST(TestDefaultOTARequestorStorage, TestCurrentUpdateState)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    OTARequestorStorage::OTAUpdateStateEnum updateState = OTARequestorStorage::OTAUpdateStateEnum::kApplying;

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.StoreCurrentUpdateState(updateState));

    updateState = OTARequestorStorage::OTAUpdateStateEnum::kUnknown;

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.LoadCurrentUpdateState(updateState));
    EXPECT_EQ(updateState, OTARequestorStorage::OTAUpdateStateEnum::kApplying);
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.ClearCurrentUpdateState());
    EXPECT_NE(CHIP_NO_ERROR, otaStorage.LoadCurrentUpdateState(updateState));
}

TEST(TestDefaultOTARequestorStorage, TestTargetVersion)
{
    TestPersistentStorageDelegate persistentStorage;
    DefaultOTARequestorStorage otaStorage;
    otaStorage.Init(persistentStorage);

    uint32_t targetVersion = 2;

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.StoreTargetVersion(targetVersion));

    targetVersion = 0;

    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.LoadTargetVersion(targetVersion));
    EXPECT_EQ(targetVersion, 2u);
    EXPECT_EQ(CHIP_NO_ERROR, otaStorage.ClearTargetVersion());
    EXPECT_NE(CHIP_NO_ERROR, otaStorage.LoadTargetVersion(targetVersion));
}

} // namespace
