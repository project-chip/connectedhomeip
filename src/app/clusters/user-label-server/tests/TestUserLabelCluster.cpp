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

#include <app/clusters/user-label-server/user-label-cluster.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/UserLabel/Attributes.h>
#include <clusters/UserLabel/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UserLabel;
using namespace chip::app::Clusters::UserLabel::Attributes;

namespace {

struct TestUserLabelCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

constexpr size_t kUserLabelFixedClusterCount = 2;

class UserLabelClusterTest
{
    UserLabelCluster userLabel;

public:
    template <typename... Args>
    UserLabelClusterTest(Args &&... args) : userLabel(std::forward<Args>(args)...)
    {}

    template <typename F>
    void Check(F check)
    {
        check(userLabel);
    }
};

class ReadAttribute
{
    UserLabelCluster & mCluster;
    DataModel::ActionReturnStatus mStatus;

public:
    ReadAttribute(UserLabelCluster & cluster) : mCluster(cluster), mStatus(CHIP_NO_ERROR) {}

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

class WriteAttribute
{
    UserLabelCluster & mCluster;
    DataModel::ActionReturnStatus mStatus;

public:
    WriteAttribute(UserLabelCluster & cluster) : mCluster(cluster), mStatus(CHIP_NO_ERROR) {}

    DataModel::ActionReturnStatus GetStatus() const { return mStatus; }

    void operator()(DataModel::WriteAttributeRequest & request)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
        ASSERT_NE(buffer.Alloc(1024).Get(), nullptr);

        TLV::TLVReader reportReader;
        reportReader.Init(buffer.Get(), buffer.AllocatedSize());

        AttributeValueDecoder decoder(reportReader, Access::SubjectDescriptor{});

        mStatus = mCluster.WriteAttribute(request, decoder);
    }
};

} // namespace

TEST_F(TestUserLabelCluster, AttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kUserLabelFixedClusterCount; ++endpoint)
    {
        UserLabelClusterTest(endpoint).Check([&](UserLabelCluster & userLabel) {
            ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
            ASSERT_EQ(userLabel.Attributes(ConcreteClusterPath(endpoint, UserLabel::Id), attributes), CHIP_NO_ERROR);

            ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
            AttributeListBuilder listBuilder(expected);
            ASSERT_EQ(listBuilder.Append(Span(UserLabel::Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
            ASSERT_TRUE(Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
        });
    }
}

TEST_F(TestUserLabelCluster, ReadAttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kUserLabelFixedClusterCount; ++endpoint)
    {
        UserLabelClusterTest(endpoint).Check([&](UserLabelCluster & userLabel) {
            chip::Test::TestServerClusterContext context;
            EXPECT_EQ(userLabel.Startup(context.Get()), CHIP_NO_ERROR);

            DataModel::ReadAttributeRequest request;
            request.path.mEndpointId  = endpoint;
            request.path.mClusterId   = UserLabel::Id;
            request.path.mAttributeId = Globals::Attributes::ClusterRevision::Id;

            ReadAttribute readAttribute(userLabel);
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

TEST_F(TestUserLabelCluster, WriteAttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kUserLabelFixedClusterCount; ++endpoint)
    {
        UserLabelClusterTest(endpoint).Check([&](UserLabelCluster & userLabel) {
            chip::Test::TestServerClusterContext context;
            EXPECT_EQ(userLabel.Startup(context.Get()), CHIP_NO_ERROR);

            DataModel::WriteAttributeRequest request;
            request.path.mEndpointId  = endpoint;
            request.path.mClusterId   = UserLabel::Id;
            request.path.mAttributeId = Globals::Attributes::ClusterRevision::Id;

            WriteAttribute writeAttribute(userLabel);
            writeAttribute(request);
            EXPECT_TRUE(writeAttribute.GetStatus().IsError());

            request.path.mAttributeId = FeatureMap::Id;
            writeAttribute(request);
            EXPECT_TRUE(writeAttribute.GetStatus().IsError());

            request.path.mAttributeId = 0xFFFF;
            writeAttribute(request);
            EXPECT_TRUE(writeAttribute.GetStatus().IsError());
        });
    }
}
