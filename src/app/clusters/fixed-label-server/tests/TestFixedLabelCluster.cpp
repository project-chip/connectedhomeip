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

#include <pw_unit_test/framework.h>

#include <app/clusters/fixed-label-server/fixed-label-cluster.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/FixedLabel/Attributes.h>
#include <clusters/FixedLabel/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FixedLabel;
using namespace chip::app::Clusters::FixedLabel::Attributes;

namespace {

struct TestFixedLabelCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

constexpr size_t kFixedLabelFixedClusterCount = 2;

class FixedLabelClusterTest
{
    FixedLabelCluster fixedLabel;

public:
    template <typename... Args>
    FixedLabelClusterTest(Args &&... args) : fixedLabel(std::forward<Args>(args)...)
    {}

    template <typename F>
    void Check(F check)
    {
        check(fixedLabel);
    }
};

class ReadAttribute
{
    FixedLabelCluster & mCluster;
    DataModel::ActionReturnStatus mStatus;

public:
    ReadAttribute(FixedLabelCluster & cluster) : mCluster(cluster), mStatus(CHIP_NO_ERROR) {}

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

TEST_F(TestFixedLabelCluster, AttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kFixedLabelFixedClusterCount; ++endpoint)
    {
        FixedLabelClusterTest(endpoint).Check([&](FixedLabelCluster & fixedLabel) {
            ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
            ASSERT_EQ(fixedLabel.Attributes(ConcreteClusterPath(endpoint, FixedLabel::Id), attributes), CHIP_NO_ERROR);

            ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
            AttributeListBuilder listBuilder(expected);
            ASSERT_EQ(listBuilder.Append(Span(FixedLabel::Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
            ASSERT_TRUE(Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
        });
    }
}

TEST_F(TestFixedLabelCluster, ReadAttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kFixedLabelFixedClusterCount; ++endpoint)
    {
        FixedLabelClusterTest(endpoint).Check([&](FixedLabelCluster & fixedLabel) {
            DataModel::ReadAttributeRequest request;
            request.path.mEndpointId  = endpoint;
            request.path.mClusterId   = FixedLabel::Id;
            request.path.mAttributeId = Globals::Attributes::ClusterRevision::Id;

            ReadAttribute readAttribute(fixedLabel);
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = FeatureMap::Id;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = LabelList::Id;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = 0xFFFF;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsError());
        });
    }
}
