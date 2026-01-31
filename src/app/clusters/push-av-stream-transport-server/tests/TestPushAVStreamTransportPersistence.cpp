/**
 *
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportLogic.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>
#include <app/server-cluster/testing/ClusterTester.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;

namespace {

class TestPushAVStreamTransportPersistence : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);
        mLogic.Init();
    }

    void TearDown() override { app::SetSafeAttributePersistenceProvider(nullptr); }

protected:
    ClusterTester mClusterTester;
    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
    PushAvStreamTransportServerLogic mLogic{ 1, BitFlags<Feature>() };

    CHIP_ERROR TestStore()
    {
        // This is a bit awkward, ideally Load/Store would be public or tested through public APIs
        return mLogic.StoreCurrentConnections();
    }

    CHIP_ERROR TestLoad()
    {
        mLogic.mCurrentConnections.clear(); // Clear before load
        return mLogic.LoadCurrentConnections();
    }
};

TEST_F(TestPushAVStreamTransportPersistence, TestStoreCurrentConnections)
{
    // Add some test connections
    TransportConfigurationStorage conn1;
    conn1.connectionID    = 1;
    conn1.transportStatus = TransportStatusEnum::kActive;
    mLogic.mCurrentConnections.push_back(conn1);

    TransportConfigurationStorage conn2;
    conn2.connectionID    = 2;
    conn2.transportStatus = TransportStatusEnum::kInactive;
    mLogic.mCurrentConnections.push_back(conn2);

    ASSERT_EQ(TestStore(), CHIP_NO_ERROR);

    // Verify that data was written to the persistence provider
    auto path = ConcreteAttributePath(1, PushAvStreamTransport::Id, Attributes::CurrentConnections::Id);
    uint8_t buffer[10000];
    MutableByteSpan span(buffer);
    ASSERT_EQ(mPersistenceProvider.ReadValue(path, span), CHIP_NO_ERROR);

    // Basic check on size
    ASSERT_TRUE(span.size() > 0);

    // Decode the TLV data and verify contents
    TLV::TLVReader reader;
    reader.Init(span);

    ASSERT_EQ(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()), CHIP_NO_ERROR);
    TLV::TLVType arrayType;
    ASSERT_EQ(reader.EnterContainer(arrayType), CHIP_NO_ERROR);

    int count = 0;
    TransportConfigurationStorage decodedConn;
    CHIP_ERROR err;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        ASSERT_EQ(decodedConn.Decode(reader), CHIP_NO_ERROR);
        if (count == 0)
        {
            EXPECT_EQ(decodedConn.connectionID, 1);
            EXPECT_EQ(decodedConn.transportStatus, TransportStatusEnum::kActive);
        }
        else if (count == 1)
        {
            EXPECT_EQ(decodedConn.connectionID, 2);
            EXPECT_EQ(decodedConn.transportStatus, TransportStatusEnum::kInactive);
        }
        count++;
    }
    EXPECT_EQ(err, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(count, 2);
    ASSERT_EQ(reader.ExitContainer(arrayType), CHIP_NO_ERROR);
    ASSERT_EQ(reader.VerifyEndOfContainer(), CHIP_NO_ERROR);
}

TEST_F(TestPushAVStreamTransportPersistence, TestLoadCurrentConnections)
{
    // Prepare some TLV data to be loaded
    uint8_t buffer[10000];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType arrayType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType), CHIP_NO_ERROR);

    TransportConfigurationStorage conn1;
    conn1.connectionID    = 10;
    conn1.transportStatus = TransportStatusEnum::kActive;
    ASSERT_EQ(conn1.EncodeForWrite(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);

    TransportConfigurationStorage conn2;
    conn2.connectionID    = 20;
    conn2.transportStatus = TransportStatusEnum::kInactive;
    ASSERT_EQ(conn2.EncodeForWrite(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);

    ASSERT_EQ(writer.EndContainer(arrayType), CHIP_NO_ERROR);

    size_t len = writer.GetLengthWritten();
    auto path  = ConcreteAttributePath(1, PushAvStreamTransport::Id, Attributes::CurrentConnections::Id);
    ASSERT_EQ(mPersistenceProvider.WriteValue(path, ByteSpan(buffer, len)), CHIP_NO_ERROR);

    // Load the data
    ASSERT_EQ(TestLoad(), CHIP_NO_ERROR);

    ASSERT_EQ(mLogic.mCurrentConnections.size(), 2);

    bool found10 = false;
    bool found20 = false;
    for (const auto & conn : mLogic.mCurrentConnections)
    {
        if (conn.connectionID == 10)
        {
            found10 = true;
            EXPECT_EQ(conn.transportStatus, TransportStatusEnum::kActive);
        }
        else if (conn.connectionID == 20)
        {
            found20 = true;
            EXPECT_EQ(conn.transportStatus, TransportStatusEnum::kInactive);
        }
    }
    EXPECT_TRUE(found10);
    EXPECT_TRUE(found20);
}

} // namespace
