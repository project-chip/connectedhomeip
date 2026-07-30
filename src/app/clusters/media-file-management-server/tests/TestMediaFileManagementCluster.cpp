/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/media-file-management-server/MediaFileManagementCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/MediaFileManagement/Commands.h>
#include <clusters/MediaFileManagement/Metadata.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaFileManagement;
using namespace chip::Testing;
using Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

class MockDelegate : public Delegate
{
public:
    uint64_t GetTotalStorage() override { return mTotalStorage; }
    uint64_t GetAvailableStorage() override { return mAvailableStorage; }

    CHIP_ERROR GetFileAtIndex(size_t index, Structs::FileDescriptionStruct::Type & file) override
    {
        if (index >= mFileCount)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        file.fileID   = static_cast<uint64_t>(index + 1);
        file.name     = CharSpan::fromCharString("file");
        file.size     = 1000;
        file.mimeType = CharSpan::fromCharString("video/mp4");
        file.imageUri = CharSpan::fromCharString("bdx://node/thumb.jpg");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSupportedMimeTypeAtIndex(size_t index, MutableCharSpan & mimeType) override
    {
        if (index >= mMimeTypeCount)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        return CopyCharSpanToMutableCharSpan(CharSpan::fromCharString("video/mp4"), mimeType);
    }

    Status HandleAddFile(const CharSpan & name, uint64_t size, const CharSpan & mimeType, const CharSpan & imageUri,
                         Commands::AddFileResponse::Type & response) override
    {
        mAddFileCalled  = true;
        response.status = FileStatusEnum::kSuccess;
        response.fileID.SetNonNull(static_cast<uint64_t>(42));
        return Status::Success;
    }

    Status HandleDeleteFile(uint64_t fileID) override
    {
        mDeletedFileID = fileID;
        return Status::Success;
    }

    Status
    HandleRequestSharedFiles(const CharSpan & clientName, uint16_t requestID,
                             const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes) override
    {
        mRequestSharedFilesCalled = true;
        return Status::Success;
    }

    Status HandleGetSharedFile(uint16_t responseID, Commands::GetSharedFileResponse::Type & response) override
    {
        response.status = FileStatusEnum::kSuccess;
        return Status::Success;
    }

    Status HandleOfferFile(const CharSpan & clientName, const CharSpan & name, uint64_t size, const CharSpan & mimeType,
                           const CharSpan & imageUri) override
    {
        mOfferFileCalled = true;
        return Status::Success;
    }

    uint64_t mTotalStorage         = 1000000;
    uint64_t mAvailableStorage     = 500000;
    size_t mFileCount              = 2;
    size_t mMimeTypeCount          = 3;
    bool mAddFileCalled            = false;
    bool mRequestSharedFilesCalled = false;
    bool mOfferFileCalled          = false;
    uint64_t mDeletedFileID        = 0;
};

struct TestMediaFileManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
    MockDelegate delegate;
};

TEST_F(TestMediaFileManagementCluster, AttributeList)
{
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>(Feature::kMediaSharing));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attrs;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, MediaFileManagement::Id), attrs), CHIP_NO_ERROR);
    // 4 mandatory attributes + global attributes.
    EXPECT_GE(attrs.TakeBuffer().size(), 4u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMediaFileManagementCluster, AcceptedCommandsWithSharing)
{
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>(Feature::kMediaSharing));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> cmds;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, MediaFileManagement::Id), cmds), CHIP_NO_ERROR);
    // AddFile, DeleteFile, RequestSharedFiles, GetSharedFile, OfferFile
    EXPECT_EQ(cmds.TakeBuffer().size(), 5u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMediaFileManagementCluster, AcceptedCommandsWithoutSharing)
{
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> cmds;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, MediaFileManagement::Id), cmds), CHIP_NO_ERROR);
    // Only AddFile, DeleteFile
    EXPECT_EQ(cmds.TakeBuffer().size(), 2u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMediaFileManagementCluster, ReadStorageAttributes)
{
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>(Feature::kMediaSharing));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t total{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::TotalStorage::Id, total), CHIP_NO_ERROR);
    EXPECT_EQ(total, 1000000u);

    uint64_t available{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::AvailableStorage::Id, available), CHIP_NO_ERROR);
    EXPECT_EQ(available, 500000u);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kMediaSharing));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMediaFileManagementCluster, InvokeAddFile)
{
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>(Feature::kMediaSharing));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::AddFile::Type request;
    request.name     = CharSpan::fromCharString("Song.mp3");
    request.size     = 3500;
    request.mimeType = CharSpan::fromCharString("audio/mpeg");
    request.imageUri = CharSpan::fromCharString("bdx://node/thumb.jpg");

    auto result = tester.Invoke<Commands::AddFile::Type, Commands::AddFileResponse::DecodableType>(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(delegate.mAddFileCalled);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMediaFileManagementCluster, InvokeDeleteFile)
{
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>(Feature::kMediaSharing));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::DeleteFile::Type request;
    request.fileID = 12345;

    auto result = tester.Invoke<Commands::DeleteFile::Type, DataModel::NullObjectType>(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(delegate.mDeletedFileID, 12345u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMediaFileManagementCluster, InvokeSharingCommandWithoutFeatureFails)
{
    // Without the MediaSharing feature, OfferFile is not in AcceptedCommands,
    // so the framework rejects it with UnsupportedCommand before it reaches the delegate.
    MediaFileManagementCluster cluster(kTestEndpointId, delegate, BitFlags<Feature>());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::OfferFile::Type request;
    request.clientName = CharSpan::fromCharString("Phone");
    request.name       = CharSpan::fromCharString("vid.mp4");
    request.size       = 100;
    request.mimeType   = CharSpan::fromCharString("video/mp4");
    request.imageUri   = CharSpan::fromCharString("bdx://node/p.jpg");

    auto result = tester.Invoke<Commands::OfferFile::Type, DataModel::NullObjectType>(request);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::UnsupportedCommand));
    EXPECT_FALSE(delegate.mOfferFileCalled);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
