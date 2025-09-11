/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/boolean-state-server/boolean-state-cluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/BooleanState/Attributes.h>
#include <clusters/BooleanState/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanState;
using namespace chip::app::Clusters::BooleanState::Attributes;

namespace {

struct TestBooleanStateCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

constexpr size_t kBooleanStateFixedClusterCount = 2;

class BooleanStateClusterTest
{
    BooleanStateCluster booleanState;

public:
    template <typename... Args>
    BooleanStateClusterTest(Args &&... args) : booleanState(std::forward<Args>(args)...)
    {}

    template <typename F>
    void Check(F check)
    {
        check(booleanState);
    }
};

class ReadAttribute
{
    BooleanStateCluster & mCluster;
    DataModel::ActionReturnStatus mStatus;

public:
    ReadAttribute(BooleanStateCluster & cluster) : mCluster(cluster), mStatus(CHIP_NO_ERROR) {}

    DataModel::ActionReturnStatus GetStatus() const { return mStatus; }

    void operator()(DataModel::ReadAttributeRequest & request)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
        ASSERT_NE(buffer.Alloc(1024).Get(), nullptr);

        AttributeReportIBs::Builder attributeReportIBsBuilder;
        TLV::TLVWriter reportWriter;
        reportWriter.Init(buffer.Get(), buffer.AllocatedSize());
        ASSERT_EQ(attributeReportIBsBuilder.Init(&reportWriter), CHIP_NO_ERROR);

        AttributeValueEncoder encoder(attributeReportIBsBuilder, Access::SubjectDescriptor{}, request.path, 0 /* dataVersion */);

        mStatus = mCluster.ReadAttribute(request, encoder);
    }
};

} // namespace

TEST_F(TestBooleanStateCluster, AttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([&](BooleanStateCluster & booleanState) {
            ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
            ASSERT_EQ(booleanState.Attributes(ConcreteClusterPath(endpoint, BooleanState::Id), attributes), CHIP_NO_ERROR);

            ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
            AttributeListBuilder listBuilder(expected);
            ASSERT_EQ(listBuilder.Append(Span(BooleanState::Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
            ASSERT_TRUE(Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
        });
    }
}

TEST_F(TestBooleanStateCluster, ReadAttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([&](BooleanStateCluster & booleanState) {
            DataModel::ReadAttributeRequest request;
            request.path.mEndpointId  = endpoint;
            request.path.mClusterId   = BooleanState::Id;
            request.path.mAttributeId = Globals::Attributes::ClusterRevision::Id;

            ReadAttribute readAttribute(booleanState);
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = FeatureMap::Id;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = StateValue::Id;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = 0xFFFF;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsError());
        });
    }
}

TEST_F(TestBooleanStateCluster, StateValue)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([](BooleanStateCluster & booleanState) {
            bool stateValue = false;
            booleanState.SetStateValue(stateValue);
            auto stateVal = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);

            stateValue = true;
            booleanState.SetStateValue(stateValue);
            stateVal = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);

            stateValue = false;
            booleanState.SetStateValue(stateValue);
            stateVal = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);
        });
    }
}
