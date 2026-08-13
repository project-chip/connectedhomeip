/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app/ClusterStateCache.h>
#include <app/ConcreteAttributePath.h>
#include <app/ReadClient.h>
#include <app/data-model/Encode.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <platform/CHIPDeviceLayer.h>

#include <clusters/IcdManagement/Attributes.h>
#include <clusters/IcdManagement/ClusterId.h>
#include <clusters/IcdManagement/Enums.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Testing;

namespace {

class MockClusterStateCache : public ClusterStateCache
{
public:
    MockClusterStateCache() : ClusterStateCache(mCallback) {}

    template <typename AttrType>
    CHIP_ERROR SetAttribute(const ConcreteAttributePath & path, const AttrType data)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> handle;
        handle.Calloc(3000);
        TLV::ScopedBufferTLVWriter writer(std::move(handle), 3000);
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), data));
        uint32_t writtenLength = writer.GetLengthWritten();
        ReturnErrorOnFailure(writer.Finalize(handle));

        TLV::ScopedBufferTLVReader reader;
        StatusIB status;
        reader.Init(std::move(handle), writtenLength);
        ReturnErrorOnFailure(reader.Next());
        ReadClient::Callback & cb = GetBufferedCallback();
        cb.OnAttributeData(path, &reader, status);

        return CHIP_NO_ERROR;
    }

private:
    class MockCallback : public ClusterStateCache::Callback
    {
        void OnDone(ReadClient *) override {}
        void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override {}
    };

    MockCallback mCallback;
};

class TestParseICDInfo : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    DeviceCommissioner mCommissioner{};
};

TEST_F(TestParseICDInfo, LITDisabledForClusterRevisionLessThanOrEqual2)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    // Set FeatureMap with LIT bit set
    BitFlags<IcdManagement::Feature> featureMap;
    featureMap.Set(IcdManagement::Feature::kLongIdleTimeSupport);
    ConcreteAttributePath featureMapPath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    // Set ClusterRevision to 2 (Matter 1.3 — LIT broken)
    uint16_t clusterRevision = 2;
    ConcreteAttributePath clusterRevisionPath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::ClusterRevision::Id);
    ASSERT_EQ(cache->SetAttribute(clusterRevisionPath, clusterRevision), CHIP_NO_ERROR);

    // Populate required ICD attributes that ParseICDInfo reads when isICD is true
    uint32_t idleModeDuration = 300;
    ConcreteAttributePath idlePath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::IdleModeDuration::Id);
    ASSERT_EQ(cache->SetAttribute(idlePath, idleModeDuration), CHIP_NO_ERROR);

    uint32_t activeModeDuration = 1000;
    ConcreteAttributePath activePath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::ActiveModeDuration::Id);
    ASSERT_EQ(cache->SetAttribute(activePath, activeModeDuration), CHIP_NO_ERROR);

    uint16_t activeModeThreshold = 500;
    ConcreteAttributePath thresholdPath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::ActiveModeThreshold::Id);
    ASSERT_EQ(cache->SetAttribute(thresholdPath, activeModeThreshold), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    CHIP_ERROR err = access.ParseICDInfo(info);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(info.icd.isLIT);
}

TEST_F(TestParseICDInfo, LITEnabledForClusterRevisionGreaterThan2)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    // Set FeatureMap with LIT bit set
    BitFlags<IcdManagement::Feature> featureMap;
    featureMap.Set(IcdManagement::Feature::kLongIdleTimeSupport);
    ConcreteAttributePath featureMapPath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    // Set ClusterRevision to 3 (post-1.3 — LIT fixed)
    uint16_t clusterRevision = 3;
    ConcreteAttributePath clusterRevisionPath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::ClusterRevision::Id);
    ASSERT_EQ(cache->SetAttribute(clusterRevisionPath, clusterRevision), CHIP_NO_ERROR);

    // Populate required ICD attributes that ParseICDInfo reads when isICD is true
    uint32_t idleModeDuration = 300;
    ConcreteAttributePath idlePath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::IdleModeDuration::Id);
    ASSERT_EQ(cache->SetAttribute(idlePath, idleModeDuration), CHIP_NO_ERROR);

    uint32_t activeModeDuration = 1000;
    ConcreteAttributePath activePath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::ActiveModeDuration::Id);
    ASSERT_EQ(cache->SetAttribute(activePath, activeModeDuration), CHIP_NO_ERROR);

    uint16_t activeModeThreshold = 500;
    ConcreteAttributePath thresholdPath(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::ActiveModeThreshold::Id);
    ASSERT_EQ(cache->SetAttribute(thresholdPath, activeModeThreshold), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    CHIP_ERROR err = access.ParseICDInfo(info);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(info.icd.isLIT);
}

} // namespace
