/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/FailSafeContext.h>
#include <app/clusters/thread-border-router-management-server/thread-border-router-management-server.h>
#include <cstdint>
#include <cstring>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVReader.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {
namespace Clusters {

namespace GeneralCommissioning {
// Mock function
void SetBreadcrumb(Attributes::Breadcrumb::TypeInfo::Type breadcrumb) {}
} // namespace GeneralCommissioning

namespace ThreadBorderRouterManagement {

class TestDelegate : public Delegate
{
public:
    TestDelegate()  = default;
    ~TestDelegate() = default;

    CHIP_ERROR Init(AttributeChangeCallback * callback) override { return CHIP_NO_ERROR; }

    bool GetPanChangeSupported() override { return mPanChangeSupported; }

    void GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        size_t nameIndex = mUseInvalidBorderRouterName ? 1 : 0;
        if (borderRouterName.size() >= strlen(kTestName[nameIndex]))
        {
            CopyCharSpanToMutableCharSpan(CharSpan(kTestName[nameIndex], strlen(kTestName[nameIndex])), borderRouterName);
        }
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override
    {
        if (borderAgentId.size() >= mTestBorderAgentIdLen)
        {
            CopySpanToMutableSpan(ByteSpan(kTestBorderAgentId, mTestBorderAgentIdLen), borderAgentId);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    uint16_t GetThreadVersion() override { return kTestThreadVersion; }

    bool GetInterfaceEnabled() override { return mInterfaceEnabled; }

    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override
    {
        if (type == DatasetType::kActive && mStoredActiveDatasetLen)
        {
            dataset.Init(ByteSpan(mStoredActiveDataset, mStoredActiveDatasetLen));
            return CHIP_NO_ERROR;
        }
        if (type == DatasetType::kPending && mPendingDatasetLen)
        {
            dataset.Init(ByteSpan(mPendingDataset, mPendingDatasetLen));
            return CHIP_NO_ERROR;
        }
        return CHIP_IM_GLOBAL_STATUS(NotFound);
    }

    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNumber,
                          ActivateDatasetCallback * callback) override
    {
        memcpy(mActiveDataset, activeDataset.AsByteSpan().data(), activeDataset.AsByteSpan().size());
        mActiveDatasetLen                   = activeDataset.AsByteSpan().size();
        mCallback                           = callback;
        mSetActiveDatasetCommandSequenceNum = sequenceNumber;
    }

    CHIP_ERROR CommitActiveDataset() override { return CHIP_NO_ERROR; }

    CHIP_ERROR RevertActiveDataset() override
    {
        mStoredActiveDatasetLen = 0;
        mInterfaceEnabled       = false;
        mCallback               = nullptr;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override
    {
        memcpy(mPendingDataset, pendingDataset.AsByteSpan().data(), pendingDataset.AsByteSpan().size());
        mPendingDatasetLen = pendingDataset.AsByteSpan().size();
        return CHIP_NO_ERROR;
    }

    void ActivateActiveDataset()
    {
        memcpy(mStoredActiveDataset, mActiveDataset, Thread::kSizeOperationalDataset);
        mStoredActiveDatasetLen = mActiveDatasetLen;
        mInterfaceEnabled       = true;
        if (mCallback)
        {
            mCallback->OnActivateDatasetComplete(mSetActiveDatasetCommandSequenceNum, CHIP_NO_ERROR);
        }
        mCallback = nullptr;
    }

    bool mPanChangeSupported  = true;
    const char * kTestName[2] = { "TestName", "TestNameLength64________________________________________________" };
    const uint8_t kTestBorderAgentId[kBorderAgentIdLength]        = { 0x0,  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    const uint16_t kTestThreadVersion                             = 4;
    uint8_t mActiveDataset[Thread::kSizeOperationalDataset]       = { 0 };
    size_t mActiveDatasetLen                                      = 0;
    uint8_t mStoredActiveDataset[Thread::kSizeOperationalDataset] = { 0 };
    size_t mStoredActiveDatasetLen                                = 0;
    uint8_t mPendingDataset[Thread::kSizeOperationalDataset]      = { 0 };
    size_t mPendingDatasetLen                                     = 0;
    bool mUseInvalidBorderRouterName                              = true;
    size_t mTestBorderAgentIdLen                                  = kBorderAgentIdLength - 1;
    bool mInterfaceEnabled                                        = false;
    uint32_t mSetActiveDatasetCommandSequenceNum                  = 0;
    ActivateDatasetCallback * mCallback                           = nullptr;
};

constexpr EndpointId kTestEndpointId            = 1;
constexpr FabricIndex kTestAccessingFabricIndex = 1;
static FailSafeContext sTestFailsafeContext;
static TestDelegate sTestDelegate;
static ServerInstance sTestSeverInstance(kTestEndpointId, &sTestDelegate, sTestFailsafeContext);

class TestCommandHandler : public CommandHandler
{
public:
    TestCommandHandler() : mClusterStatus(Protocols::InteractionModel::Status::Success) {}
    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus, const char * context = nullptr)
    {
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr)
    {
        mClusterStatus = aStatus;
    }

    FabricIndex GetAccessingFabricIndex() const { return kTestAccessingFabricIndex; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable)
    {
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable)
    {}

    bool IsTimedInvoke() const { return false; }

    void FlushAcksRightAwayOnSlowCommand() {}

    Access::SubjectDescriptor GetSubjectDescriptor() const
    {
        Access::SubjectDescriptor subjectDescriptor = { kUndefinedFabricIndex, Access::AuthMode::kNone, kUndefinedNodeId,
                                                        kUndefinedCATs };
        return subjectDescriptor;
    }

    Messaging::ExchangeContext * GetExchangeContext() const { return nullptr; }

    Protocols::InteractionModel::ClusterStatusCode mClusterStatus;
};

TestCommandHandler sTestCommandHandler;

class TestThreadBorderRouterManagementCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void TestAttributeRead();
    void TestCommandHandle();
};

// Test ReadXX functions in ThreadBorderRouterManagement ServerInstance
TEST_F_FROM_FIXTURE(TestThreadBorderRouterManagementCluster, TestAttributeRead)
{
    // FeatureMap attribute
    BitFlags<Feature> featureMap = BitFlags<Feature>();
    // Make the PAN change feature supported in Test delegate.
    sTestDelegate.mPanChangeSupported = true;
    sTestSeverInstance.ReadFeatureMap(featureMap);
    EXPECT_TRUE(featureMap.Has(Feature::kPANChange));
    // Make the PAN change feature unsupported in Test delegate.
    sTestDelegate.mPanChangeSupported = false;
    featureMap.ClearAll();
    sTestSeverInstance.ReadFeatureMap(featureMap);
    EXPECT_FALSE(featureMap.Has(Feature::kPANChange));
    // BorderRouterName attribute
    // Use invalid BR name
    sTestDelegate.mUseInvalidBorderRouterName              = true;
    char borderRouterName[kBorderRouterNameMaxLength + 10] = { 0 };
    MutableCharSpan nameSpan(borderRouterName);
    EXPECT_EQ(sTestSeverInstance.ReadBorderRouterName(nameSpan), CHIP_IM_GLOBAL_STATUS(Failure));
    nameSpan = MutableCharSpan(borderRouterName);
    // Use valid BR name
    sTestDelegate.mUseInvalidBorderRouterName = false;
    EXPECT_EQ(sTestSeverInstance.ReadBorderRouterName(nameSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(nameSpan.data_equal(CharSpan("TestName", strlen("TestName"))));
    // BorderAgentId attribute
    uint8_t borderAgentId[kBorderAgentIdLength] = { 0 };
    MutableByteSpan agentIdSpan(borderAgentId);
    // Use invalid border agent id
    sTestDelegate.mTestBorderAgentIdLen = kBorderAgentIdLength - 1;
    EXPECT_EQ(sTestSeverInstance.ReadBorderAgentID(agentIdSpan), CHIP_IM_GLOBAL_STATUS(Failure));
    agentIdSpan = MutableByteSpan(borderAgentId);
    // Use valid border agent id
    sTestDelegate.mTestBorderAgentIdLen = kBorderAgentIdLength;
    EXPECT_EQ(sTestSeverInstance.ReadBorderAgentID(agentIdSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(agentIdSpan.data_equal(ByteSpan(sTestDelegate.kTestBorderAgentId)));
    // ActiveDatasetTimestamp attribute
    // The active dataset timestamp should be null when no active dataset is configured
    std::optional<uint64_t> timestamp = sTestSeverInstance.ReadActiveDatasetTimestamp();
    EXPECT_FALSE(timestamp.has_value());
}

TEST_F_FROM_FIXTURE(TestThreadBorderRouterManagementCluster, TestCommandHandle)
{
    // Test GetActiveDatasetRequest and GetPendingDatasetRequest commands
    Thread::OperationalDataset dataset;
    ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::DecodableType req1;
    Commands::SetPendingDatasetRequest::DecodableType req2;
    using DatasetType = Delegate::DatasetType;
    using Status      = Protocols::InteractionModel::Status;
    ConcreteCommandPath testPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    TLV::TLVReader testTLVReader;
    CommandHandlerInterface::HandlerContext ctx(sTestCommandHandler, testPath, testTLVReader);
    // All the command should be over CASE session.
    EXPECT_EQ(sTestSeverInstance.HandleGetDatasetRequest(ctx, DatasetType::kActive, dataset), Status::UnsupportedAccess);
    EXPECT_EQ(sTestSeverInstance.HandleGetDatasetRequest(ctx, DatasetType::kPending, dataset), Status::UnsupportedAccess);
    EXPECT_EQ(sTestSeverInstance.HandleSetActiveDatasetRequest(ctx, req1), Status::UnsupportedAccess);
    EXPECT_EQ(sTestSeverInstance.HandleSetPendingDatasetRequest(ctx, req2), Status::UnsupportedAccess);
    sTestSeverInstance.SetSkipCASESessionCheck(true);
    // The GetDataset should return NotFound when no dataset is configured.
    EXPECT_EQ(sTestSeverInstance.HandleGetDatasetRequest(ctx, DatasetType::kActive, dataset), Status::NotFound);
    EXPECT_EQ(sTestSeverInstance.HandleGetDatasetRequest(ctx, DatasetType::kPending, dataset), Status::NotFound);
    // Test SetActiveDatasetRequest
    uint8_t invalidDataset[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
    uint8_t validDataset[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0b, 0x35, 0x06,
                               0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0xde, 0xaa, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xef, 0x07,
                               0x08, 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00, 0x05, 0x10, 0xb7, 0x28, 0x08, 0x04, 0x85, 0xcf,
                               0xc5, 0x25, 0x7f, 0x68, 0x4c, 0x54, 0x9d, 0x6a, 0x57, 0x5e, 0x03, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x54,
                               0x68, 0x72, 0x65, 0x61, 0x64, 0x01, 0x02, 0xc1, 0x15, 0x04, 0x10, 0xcb, 0x13, 0x47, 0xeb, 0x0c, 0xd4,
                               0xb3, 0x5c, 0xd1, 0x42, 0xda, 0x5e, 0x6d, 0xf1, 0x8b, 0x88, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8 };
    std::optional<uint64_t> activeDatasetTimestamp = std::nullopt;
    activeDatasetTimestamp                         = sTestSeverInstance.ReadActiveDatasetTimestamp();
    EXPECT_FALSE(activeDatasetTimestamp.has_value());
    req1.activeDataset = ByteSpan(invalidDataset);
    // SetActiveDatasetRequest is FailsafeRequired.
    EXPECT_EQ(sTestSeverInstance.HandleSetActiveDatasetRequest(ctx, req1), Status::FailsafeRequired);
    EXPECT_EQ(sTestFailsafeContext.ArmFailSafe(kTestAccessingFabricIndex, System::Clock::Seconds16(1)), CHIP_NO_ERROR);
    // SetActiveDatasetRequest should return InvalidCommand when dataset is invalid.
    EXPECT_EQ(sTestSeverInstance.HandleSetActiveDatasetRequest(ctx, req1), Status::InvalidCommand);
    req1.activeDataset = ByteSpan(validDataset);
    EXPECT_EQ(sTestSeverInstance.HandleSetActiveDatasetRequest(ctx, req1), Status::Success);
    // When the Server is handling a SetActiveDatasetRequest command, it should return Busy after receiving another one.
    EXPECT_EQ(sTestSeverInstance.HandleSetActiveDatasetRequest(ctx, req1), Status::Busy);
    EXPECT_FALSE(sTestDelegate.mInterfaceEnabled);
    EXPECT_EQ(sTestDelegate.mSetActiveDatasetCommandSequenceNum, static_cast<unsigned int>(1));
    // Activate the dataset.
    sTestDelegate.ActivateActiveDataset();
    EXPECT_EQ(sTestCommandHandler.mClusterStatus,
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::Success));
    sTestFailsafeContext.DisarmFailSafe();
    // The Dataset should be updated.
    EXPECT_EQ(sTestSeverInstance.HandleGetDatasetRequest(ctx, DatasetType::kActive, dataset), Status::Success);
    EXPECT_TRUE(dataset.AsByteSpan().data_equal(ByteSpan(validDataset)));
    EXPECT_TRUE(sTestDelegate.mInterfaceEnabled);
    activeDatasetTimestamp = sTestSeverInstance.ReadActiveDatasetTimestamp();
    // activeDatasetTimestamp should have value.
    EXPECT_TRUE(activeDatasetTimestamp.has_value());
    EXPECT_EQ(sTestFailsafeContext.ArmFailSafe(kTestAccessingFabricIndex, System::Clock::Seconds16(1)), CHIP_NO_ERROR);
    // When ActiveDatasetTimestamp is not null, the set active dataset request should return InvalidInState.
    EXPECT_EQ(sTestSeverInstance.HandleSetActiveDatasetRequest(ctx, req1), Status::InvalidInState);
    sTestFailsafeContext.DisarmFailSafe();
    // Test SetPendingDatasetRequest command
    sTestDelegate.mPanChangeSupported = false;
    req2.pendingDataset               = ByteSpan(validDataset);
    // SetPendingDatasetRequest is supported when PANChange feature is enabled.
    EXPECT_EQ(sTestSeverInstance.HandleSetPendingDatasetRequest(ctx, req2), Status::UnsupportedCommand);
    sTestDelegate.mPanChangeSupported = true;
    req2.pendingDataset               = ByteSpan(invalidDataset);
    // SetPendingDatasetRequest should return InvalidCommand when dataset is invalid.
    EXPECT_EQ(sTestSeverInstance.HandleSetPendingDatasetRequest(ctx, req2), Status::InvalidCommand);
    req2.pendingDataset = ByteSpan(validDataset);
    // Success SetPendingDatasetRequest
    EXPECT_EQ(sTestSeverInstance.HandleSetPendingDatasetRequest(ctx, req2), Status::Success);
    EXPECT_EQ(sTestSeverInstance.HandleGetDatasetRequest(ctx, DatasetType::kPending, dataset), Status::Success);
    EXPECT_TRUE(dataset.AsByteSpan().data_equal(ByteSpan(validDataset)));
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
