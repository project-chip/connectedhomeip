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

#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportLogic.h>
#include <app/persistence/tests/TestPersistence.h>
#include <lib/support/tests/CHIPFaultInjection.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;

namespace {

class TestPushAVStreamTransportLogic : public PushAvStreamTransportServerLogic
{
public:
    TestPushAVStreamTransportLogic(EndpointId aEndpoint, BitFlags<Feature> aFeatures) :
        PushAvStreamTransportServerLogic(aEndpoint, aFeatures)
    {}

    // Expose protected methods for testing
    using PushAvStreamTransportServerLogic::StoreCurrentConnections;
    using PushAvStreamTransportServerLogic::LoadCurrentConnections;
    using PushAvStreamTransportServerLogic::mCurrentConnections;

    CHIP_ERROR TestStore() { return StoreCurrentConnections(); }
    CHIP_ERROR TestLoad() { return LoadCurrentConnections(); }
};

struct TestContext
{
    TestPersistence sTestPersistence;
    TestPushAVStreamTransportLogic mLogic{ 1, BitFlags<Feature>() };

    TestContext()
    {
        SetAttributePersistenceProvider(&sTestPersistence);
    }

    ~TestContext()
    {
        SetAttributePersistenceProvider(nullptr);
    }
};

TEST_F(TestPushAVStreamTransportLogic, TestStoreCurrentConnections)
{
    TestContext testContext;
    TestPushAVStreamTransportLogic & logic = testContext.mLogic;

    // Add some test connections
    TransportConfigurationStorage conn1;
    conn1.connectionID = 1;
    conn1.transportStatus = TransportStatusEnum::kActive;
    logic.mCurrentConnections.push_back(conn1);

    TransportConfigurationStorage conn2;
    conn2.connectionID = 2;
    conn2.transportStatus = TransportStatusEnum::kInactive;
    logic.mCurrentConnections.push_back(conn2);

    REQUIRE(logic.TestStore() == CHIP_NO_ERROR);

    // Verify that data was written to the persistence provider
    auto path = ConcreteAttributePath(1, PushAvStreamTransport::Id, Attributes::CurrentConnections::Id);
    uint8_t buffer[10000];
    MutableByteSpan span(buffer);
    REQUIRE(testContext.sTestPersistence.ReadValue(path, span) == CHIP_NO_ERROR);

    // Basic check on size
    REQUIRE(span.size() > 0);

    // Decode the TLV data and verify contents
    TLV::TLVReader reader;
    reader.Init(span);

    REQUIRE(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()) == CHIP_NO_ERROR);
    TLV::TLVType arrayType;
    REQUIRE(reader.EnterContainer(arrayType) == CHIP_NO_ERROR);

    int count = 0;
    TransportConfigurationStorage decodedConn;
    CHIP_ERROR err;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        REQUIRE(decodedConn.Decode(reader) == CHIP_NO_ERROR);
        if (count == 0)
        {
            REQUIRE(decodedConn.connectionID == 1);
            REQUIRE(decodedConn.transportStatus == TransportStatusEnum::kActive);
        }
        else if (count == 1)
        {
            REQUIRE(decodedConn.connectionID == 2);
            REQUIRE(decodedConn.transportStatus == TransportStatusEnum::kInactive);
        }
        count++;
    }
    REQUIRE(err == CHIP_ERROR_END_OF_TLV);
    REQUIRE(count == 2);
    REQUIRE(reader.ExitContainer(arrayType) == CHIP_NO_ERROR);
    REQUIRE(reader.VerifyEndOfContainer() == CHIP_NO_ERROR);
}

TEST_F(TestPushAVStreamTransportLogic, TestLoadCurrentConnectionsNoData)
{
    TestContext testContext;
    TestPushAVStreamTransportLogic & logic = testContext.mLogic;

    // Persistence is empty by default
    REQUIRE(logic.TestLoad() == CHIP_NO_ERROR);
    REQUIRE(logic.mCurrentConnections.empty());
}

TEST_F(TestPushAVStreamTransportLogic, TestLoadCurrentConnectionsWithData)
{
    TestContext testContext;
    TestPushAVStreamTransportLogic & logic = testContext.mLogic;

    // Prepare some TLV data to be loaded
    uint8_t buffer[10000];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType arrayType;
    REQUIRE(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType) == CHIP_NO_ERROR);

    TransportConfigurationStorage conn1;
    conn1.connectionID = 10;
    conn1.transportStatus = TransportStatusEnum::kActive;
    REQUIRE(conn1.EncodeForWrite(writer, TLV::AnonymousTag()) == CHIP_NO_ERROR);

    TransportConfigurationStorage conn2;
    conn2.connectionID = 20;
    conn2.transportStatus = TransportStatusEnum::kInactive;
    REQUIRE(conn2.EncodeForWrite(writer, TLV::AnonymousTag()) == CHIP_NO_ERROR);

    REQUIRE(writer.EndContainer(arrayType) == CHIP_NO_ERROR);

    size_t len = writer.GetLengthWritten();
    auto path = ConcreteAttributePath(1, PushAvStreamTransport::Id, Attributes::CurrentConnections::Id);
    REQUIRE(testContext.sTestPersistence.WriteValue(path, ByteSpan(buffer, len)) == CHIP_NO_ERROR);

    // Load the data
    REQUIRE(logic.TestLoad() == CHIP_NO_ERROR);

    REQUIRE(logic.mCurrentConnections.size() == 2);

    bool found10 = false;
    bool found20 = false;
    for (const auto & conn : logic.mCurrentConnections)
    {
        if (conn.connectionID == 10)
        {
            found10 = true;
            REQUIRE(conn.transportStatus == TransportStatusEnum::kActive);
        }
        else if (conn.connectionID == 20)
        {
            found20 = true;
            REQUIRE(conn.transportStatus == TransportStatusEnum::kInactive);
        }
    }
    REQUIRE(found10);
    REQUIRE(found20);
}

} // namespace
