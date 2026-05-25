/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>

#include <clusters/ThreadNetworkDirectory/Attributes.h>
#include <clusters/ThreadNetworkDirectory/Commands.h>
#include <clusters/ThreadNetworkDirectory/Metadata.h>

#include "FakeThreadNetworkDirectoryStorage.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDirectory;
using namespace chip::Testing;
using IMStatus = chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// A valid Thread operational dataset containing ALL required sub-TLVs:
// Active Timestamp, Channel, Channel Mask, Extended PAN ID, Network Key,
// Mesh-Local Prefix, Network Name, PAN ID, PKSc, and Security Policy.
//
// ExtPanId = 0x39758ec8144b07fb, ActiveTimestamp = 1
// Source: kValidExampleDataset from src/lib/support/tests/TestThreadOperationalDataset.cpp
constexpr uint8_t kDataset1[] = {
    0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // Active Timestamp 1
    0x00, 0x03, 0x00, 0x00, 0x0f,                               // Channel 15
    0x35, 0x04, 0x07, 0xff, 0xf8, 0x00,                         // Channel Mask 0x07fff800
    0x02, 0x08, 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb, // Ext PAN ID 0x39758ec8144b07fb
    0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, // Mesh Local Prefix
    0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23, // Network Key
    0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38,       // "OpenThread-5938"
    0x01, 0x02, 0x59, 0x38,                                                                                     // PAN ID 0x5938
    0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, // PKSc
    0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,                                                                         // Security Policy
};

// Same ExtPanId as kDataset1 but with ActiveTimestamp = 2 (for update testing).
constexpr uint8_t kDataset1Updated[] = {
    0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // Active Timestamp 2
    0x00, 0x03, 0x00, 0x00, 0x0f, 0x35, 0x04, 0x07, 0xff, 0xf8, 0x00, 0x02, 0x08, 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b,
    0x07, 0xfb, // Same ExtPanId
    0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, 0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba,
    0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23, 0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65,
    0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38, 0x01, 0x02, 0x59, 0x38, 0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb,
    0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,
};

// Different ExtPanId (0xAABBCCDDEEFF1122) for adding a second network.
constexpr uint8_t kDataset2[] = {
    0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // Active Timestamp 1
    0x00, 0x03, 0x00, 0x00, 0x0f, 0x35, 0x04, 0x07, 0xff, 0xf8, 0x00, 0x02, 0x08, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x11, 0x22, // Ext PAN ID 0xAABBCCDDEEFF1122
    0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, 0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba,
    0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23, 0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65,
    0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38, 0x01, 0x02, 0x59, 0x38, 0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb,
    0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,
};

// A third valid dataset with yet another ExtPanId (0x3333333333333333), for capacity overflow tests.
constexpr uint8_t kDataset3[] = {
    0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x0f, 0x35, 0x04, 0x07, 0xff,
    0xf8, 0x00, 0x02, 0x08, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // Ext PAN ID 0x3333333333333333
    0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, 0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba,
    0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23, 0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65,
    0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38, 0x01, 0x02, 0x59, 0x38, 0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb,
    0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,
};

// ExtPanId byte arrays for use with RemoveNetwork and GetOperationalDataset commands.
constexpr uint8_t kExPanId1Bytes[] = { 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb };
constexpr uint8_t kExPanId2Bytes[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22 };
constexpr uint8_t kExPanId3Bytes[] = { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33 };

app::ThreadNetworkDirectoryStorage::ExtendedPanId MakeExPanId1()
{
    return app::ThreadNetworkDirectoryStorage::ExtendedPanId(ByteSpan(kExPanId1Bytes));
}

app::ThreadNetworkDirectoryStorage::ExtendedPanId MakeExPanId2()
{
    return app::ThreadNetworkDirectoryStorage::ExtendedPanId(ByteSpan(kExPanId2Bytes));
}

app::ThreadNetworkDirectoryStorage::ExtendedPanId MakeExPanId3()
{
    return app::ThreadNetworkDirectoryStorage::ExtendedPanId(ByteSpan(kExPanId3Bytes));
}

/// RAII helper that sets the global SafeAttributePersistenceProvider for tests that need
/// attribute persistence (e.g. PreferredExtendedPanID reads/writes, RemoveNetwork checks).
/// The previous global is restored on destruction.
class ScopedSafeAttributePersistence
{
public:
    explicit ScopedSafeAttributePersistence(chip::Testing::TestServerClusterContext & context) :
        mOldPersistence(app::GetSafeAttributePersistenceProvider())
    {
        VerifyOrDie(mPersistence.Init(&context.StorageDelegate()) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistence);
    }
    ~ScopedSafeAttributePersistence() { app::SetSafeAttributePersistenceProvider(mOldPersistence); }

private:
    app::SafeAttributePersistenceProvider * mOldPersistence;
    app::DefaultSafeAttributePersistenceProvider mPersistence;
};

struct TestThreadNetworkDirectoryCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// ---------------------------------------------------------------------------
// TestMetadata: verify the cluster's attribute list and command lists.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestMetadata)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);

    // IsAttributesListEqualTo automatically includes mandatory global attributes; we
    // only need to list the cluster-specific ones here.
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::PreferredExtendedPanID::kMetadataEntry,
                                            Attributes::ThreadNetworks::kMetadataEntry,
                                            Attributes::ThreadNetworkTableSize::kMetadataEntry,
                                        }));

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::AddNetwork::kMetadataEntry,
                                                  Commands::RemoveNetwork::kMetadataEntry,
                                                  Commands::GetOperationalDataset::kMetadataEntry,
                                              }));

    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { Commands::OperationalDatasetResponse::Id }));
}

// ---------------------------------------------------------------------------
// TestReadTableSize: ThreadNetworkTableSize reflects storage capacity.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestReadTableSize)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage(3);
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);
    ClusterTester tester(cluster);

    uint8_t tableSize = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ThreadNetworkTableSize::Id, tableSize), CHIP_NO_ERROR);
    EXPECT_EQ(tableSize, 3u);
}

// ---------------------------------------------------------------------------
// TestPreferredExtendedPanId: null initially, write/read round-trip,
// ConstraintError when PAN ID is not in the stored list, clear with null.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestPreferredExtendedPanId)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);
    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ScopedSafeAttributePersistence scopedPersistence(context);
    ClusterTester tester(cluster);

    // Initially null.
    {
        Attributes::PreferredExtendedPanID::TypeInfo::DecodableType value;
        EXPECT_EQ(tester.ReadAttribute(Attributes::PreferredExtendedPanID::Id, value), CHIP_NO_ERROR);
        EXPECT_TRUE(value.IsNull());
    }

    // ConstraintError: PAN ID not in the stored network list.
    {
        DataModel::Nullable<ByteSpan> toWrite;
        toWrite.SetNonNull(ByteSpan(kExPanId1Bytes));
        EXPECT_EQ(tester.WriteAttribute(Attributes::PreferredExtendedPanID::Id, toWrite), IMStatus::ConstraintError);
    }

    // Add the network so the write can succeed.
    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId1(), ByteSpan(kDataset1)), CHIP_NO_ERROR);

    {
        DataModel::Nullable<ByteSpan> toWrite;
        toWrite.SetNonNull(ByteSpan(kExPanId1Bytes));
        EXPECT_EQ(tester.WriteAttribute(Attributes::PreferredExtendedPanID::Id, toWrite), IMStatus::Success);
    }

    // Read back and verify the value.
    {
        Attributes::PreferredExtendedPanID::TypeInfo::DecodableType value;
        EXPECT_EQ(tester.ReadAttribute(Attributes::PreferredExtendedPanID::Id, value), CHIP_NO_ERROR);
        ASSERT_FALSE(value.IsNull());
        EXPECT_TRUE(value.Value().data_equal(ByteSpan(kExPanId1Bytes)));
    }

    // Clear with null.
    {
        DataModel::Nullable<ByteSpan> nullValue;
        nullValue.SetNull();
        EXPECT_EQ(tester.WriteAttribute(Attributes::PreferredExtendedPanID::Id, nullValue), IMStatus::Success);
    }

    {
        Attributes::PreferredExtendedPanID::TypeInfo::DecodableType value;
        EXPECT_EQ(tester.ReadAttribute(Attributes::PreferredExtendedPanID::Id, value), CHIP_NO_ERROR);
        EXPECT_TRUE(value.IsNull());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// TestThreadNetworksList: list is initially empty; after directly adding
// networks to the storage the attribute encodes correct field values.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestThreadNetworksList)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);
    ClusterTester tester(cluster);

    // Initially empty.
    {
        Attributes::ThreadNetworks::TypeInfo::DecodableType networksList;
        EXPECT_EQ(tester.ReadAttribute(Attributes::ThreadNetworks::Id, networksList), CHIP_NO_ERROR);
        auto it = networksList.begin();
        EXPECT_FALSE(it.Next());
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    }

    // Add two networks directly to storage (bypassing the cluster command path).
    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId1(), ByteSpan(kDataset1)), CHIP_NO_ERROR);
    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId2(), ByteSpan(kDataset2)), CHIP_NO_ERROR);

    // List should contain two entries with correct fields.
    {
        Attributes::ThreadNetworks::TypeInfo::DecodableType networksList;
        EXPECT_EQ(tester.ReadAttribute(Attributes::ThreadNetworks::Id, networksList), CHIP_NO_ERROR);

        size_t count = 0;
        auto it      = networksList.begin();
        while (it.Next())
        {
            const auto & network = it.GetValue();
            EXPECT_EQ(network.extendedPanID.size(), 8u);
            EXPECT_GT(network.networkName.size(), 0u);
            EXPECT_GT(network.channel, 0u);
            ++count;
        }
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(count, 2u);
    }
}

// ---------------------------------------------------------------------------
// TestAddNetworkCommand: cover success, ConstraintError (empty dataset),
// InvalidInState (old timestamp), success on update, ResourceExhausted.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestAddNetworkCommand)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);
    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ScopedSafeAttributePersistence scopedPersistence(context);
    ClusterTester tester(cluster);

    // ConstraintError: empty dataset is structurally invalid.
    {
        Commands::AddNetwork::Type req;
        req.operationalDataset = ByteSpan();
        auto result            = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::ConstraintError);
        }
    }

    // Success: add a valid dataset.
    {
        Commands::AddNetwork::Type req;
        req.operationalDataset = ByteSpan(kDataset1);
        auto result            = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::Success);
        }
        EXPECT_TRUE(storage.ContainsNetwork(MakeExPanId1()));
    }

    // InvalidInState: re-adding with the same (not strictly newer) active timestamp.
    {
        Commands::AddNetwork::Type req;
        req.operationalDataset = ByteSpan(kDataset1);
        auto result            = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::InvalidInState);
        }
    }

    // Success: update accepted when active timestamp is strictly newer.
    {
        Commands::AddNetwork::Type req;
        req.operationalDataset = ByteSpan(kDataset1Updated);
        auto result            = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::Success);
        }
    }

    // ResourceExhausted: fill remaining capacity then add one more new network.
    // Default storage capacity is 4; kDataset1 (exPanId1) is already present = 1/4 full.
    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId2(), ByteSpan(kDataset2)), CHIP_NO_ERROR); // 2/4
    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId3(), ByteSpan(kDataset3)), CHIP_NO_ERROR); // 3/4
    {
        // Fill the last slot: 4/4.
        app::ThreadNetworkDirectoryStorage::ExtendedPanId exPan4(UINT64_C(0x4444444444444444));
        EXPECT_EQ(storage.AddOrUpdateNetwork(exPan4, ByteSpan(kDataset2)), CHIP_NO_ERROR);

        // Now storage is full (4/4). Invoking AddNetwork with a new PAN ID must fail.
        // Build a dataset that has a unique ExtPanId (0x5555555555555555).
        constexpr uint8_t kDataset5[] = {
            0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x0f, 0x35, 0x04, 0x07,
            0xff, 0xf8, 0x00, 0x02, 0x08, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x07, 0x08, 0xfd, 0xf1, 0xf1,
            0xad, 0xd0, 0x79, 0x7d, 0xc0, 0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, 0x78, 0xd9, 0x0d,
            0x27, 0xab, 0xe3, 0x8f, 0x23, 0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d,
            0x35, 0x39, 0x33, 0x38, 0x01, 0x02, 0x59, 0x38, 0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c,
            0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,
        };
        Commands::AddNetwork::Type req;
        req.operationalDataset = ByteSpan(kDataset5);
        auto result            = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::ResourceExhausted);
        }
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// TestRemoveNetworkCommand: NotFound, ConstraintError (bad size),
// ConstraintError (preferred network), and success.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestRemoveNetworkCommand)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);
    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ScopedSafeAttributePersistence scopedPersistence(context);
    ClusterTester tester(cluster);

    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId1(), ByteSpan(kDataset1)), CHIP_NO_ERROR);

    // ConstraintError: PAN ID with wrong byte length.
    {
        constexpr uint8_t kBadPanId[] = { 0x01, 0x02, 0x03 };
        Commands::RemoveNetwork::Type req;
        req.extendedPanID = ByteSpan(kBadPanId);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::ConstraintError);
        }
    }

    // NotFound: PAN ID not in storage.
    {
        Commands::RemoveNetwork::Type req;
        req.extendedPanID = ByteSpan(kExPanId2Bytes);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::NotFound);
        }
    }

    // ConstraintError: cannot remove the currently preferred network.
    {
        DataModel::Nullable<ByteSpan> preferred;
        preferred.SetNonNull(ByteSpan(kExPanId1Bytes));
        EXPECT_EQ(tester.WriteAttribute(Attributes::PreferredExtendedPanID::Id, preferred), IMStatus::Success);

        Commands::RemoveNetwork::Type req;
        req.extendedPanID = ByteSpan(kExPanId1Bytes);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::ConstraintError);
        }

        // Clear preferred so the subsequent remove can succeed.
        DataModel::Nullable<ByteSpan> nullValue;
        nullValue.SetNull();
        EXPECT_EQ(tester.WriteAttribute(Attributes::PreferredExtendedPanID::Id, nullValue), IMStatus::Success);
    }

    // Success: remove an existing network.
    {
        Commands::RemoveNetwork::Type req;
        req.extendedPanID = ByteSpan(kExPanId1Bytes);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::Success);
        }
        EXPECT_FALSE(storage.ContainsNetwork(MakeExPanId1()));
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// TestGetOperationalDatasetCommand: UnsupportedAccess for non-CASE sessions,
// ConstraintError for wrong PAN ID size, NotFound, and success with
// round-trip verification of the returned dataset blob.
// ---------------------------------------------------------------------------
TEST_F(TestThreadNetworkDirectoryCluster, TestGetOperationalDatasetCommand)
{
    app::Testing::FakeThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryCluster cluster(kTestEndpointId, storage);
    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ScopedSafeAttributePersistence scopedPersistence(context);
    ClusterTester tester(cluster);

    EXPECT_EQ(storage.AddOrUpdateNetwork(MakeExPanId1(), ByteSpan(kDataset1)), CHIP_NO_ERROR);

    // UnsupportedAccess: group-addressed sessions (non-CASE) must be rejected.
    {
        Access::SubjectDescriptor groupDescriptor;
        groupDescriptor.authMode    = Access::AuthMode::kGroup;
        groupDescriptor.fabricIndex = 1;
        groupDescriptor.subject     = 0;
        tester.SetSubjectDescriptor(groupDescriptor);

        Commands::GetOperationalDataset::Type req;
        req.extendedPanID = ByteSpan(kExPanId1Bytes);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::UnsupportedAccess);
        }

        // Restore CASE subject descriptor for subsequent tests.
        tester.SetSubjectDescriptor(kAdminSubjectDescriptor);
    }

    // ConstraintError: PAN ID with wrong byte length.
    {
        constexpr uint8_t kShortPanId[] = { 0x01, 0x02 };
        Commands::GetOperationalDataset::Type req;
        req.extendedPanID = ByteSpan(kShortPanId);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::ConstraintError);
        }
    }

    // NotFound: valid-size PAN ID not in storage.
    {
        Commands::GetOperationalDataset::Type req;
        req.extendedPanID = ByteSpan(kExPanId2Bytes);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), IMStatus::NotFound);
        }
    }

    // Success: OperationalDatasetResponse must contain the exact stored bytes.
    {
        Commands::GetOperationalDataset::Type req;
        req.extendedPanID = ByteSpan(kExPanId1Bytes);
        auto result       = tester.Invoke(req);
        ASSERT_TRUE(result.IsSuccess());
        ASSERT_TRUE(result.response.has_value());
        if (result.response.has_value())
        {
            EXPECT_TRUE(result.response->operationalDataset.data_equal(ByteSpan(kDataset1)));
        }
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
