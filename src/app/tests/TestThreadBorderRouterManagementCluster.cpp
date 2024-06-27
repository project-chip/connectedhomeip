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
#include <app/clusters/thread-border-router-management-server/thread-br-mgmt-server.h>
#include <app/server/Server.h>
#include <cstdint>
#include <cstring>
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
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

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetPanChangeSupported(bool & panChangeSupported) override
    {
        panChangeSupported = mPanChangeSupported;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        size_t nameIndex = mUseInvalidBorderRouterName ? 1 : 0;
        if (borderRouterName.size() > strlen(kTestName[nameIndex]))
        {
            memcpy(borderRouterName.data(), kTestName[nameIndex], strlen(kTestName[nameIndex]));
            borderRouterName.reduce_size(strlen(kTestName[nameIndex]));
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override
    {
        if (borderAgentId.size() >= mTestBorderAgentIdLen)
        {
            memcpy(borderAgentId.data(), kTestBorderAgentId, mTestBorderAgentIdLen);
            borderAgentId.reduce_size(mTestBorderAgentIdLen);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR GetThreadVersion(uint16_t & threadVersion) override
    {
        threadVersion = kTestThreadVersion;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetInterfaceEnabled(bool & interfaceEnabled) override
    {
        interfaceEnabled = mInterfaceEnabled;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override
    {
        if (type == DatasetType::kActive && mActiveDatasetLen)
        {
            dataset.Init(ByteSpan(mActiveDataset, mActiveDatasetLen));
            return CHIP_NO_ERROR;
        }
        else if (type == DatasetType::kPending && mPendingDatasetLen)
        {
            dataset.Init(ByteSpan(mPendingDataset, mPendingDatasetLen));
            return CHIP_NO_ERROR;
        }
        return CHIP_IM_GLOBAL_STATUS(NotFound);
    }

    CHIP_ERROR SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t randomNumber,
                                ActivateDatasetCallback * callback) override
    {
        memcpy(mActiveDataset, activeDataset.AsByteSpan().data(), activeDataset.AsByteSpan().size());
        mActiveDatasetLen = activeDataset.AsByteSpan().size();
        mInterfaceEnabled = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommitActiveDataset() override { return CHIP_NO_ERROR; }

    CHIP_ERROR RevertActiveDataset() override { return CHIP_NO_ERROR; }

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override
    {
        memcpy(mPendingDataset, pendingDataset.AsByteSpan().data(), pendingDataset.AsByteSpan().size());
        mPendingDatasetLen = pendingDataset.AsByteSpan().size();
        return CHIP_NO_ERROR;
    }

    bool mPanChangeSupported  = true;
    const char * kTestName[2] = { "TestName", "TestNameLength64________________________________________________" };
    const uint8_t kTestBorderAgentId[kBorderAgentIdLength] = { 0x0,  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                               0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    const uint16_t kTestThreadVersion                      = 4;
    uint8_t mActiveDataset[Thread::kSizeOperationalDataset];
    size_t mActiveDatasetLen = 0;
    uint8_t mPendingDataset[Thread::kSizeOperationalDataset];
    size_t mPendingDatasetLen        = 0;
    bool mUseInvalidBorderRouterName = true;
    size_t mTestBorderAgentIdLen     = kBorderAgentIdLength - 1;
    bool mInterfaceEnabled           = false;
};

constexpr chip::EndpointId sTestEndpointId    = 1;
static TestDelegate * sTestDelegatePtr        = nullptr;
static ServerInstance * sTestSeverInstancePtr = nullptr;

class TestThreadBorderRouterManagementCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        sTestDelegatePtr = new ThreadBorderRouterManagement::TestDelegate();
        ASSERT_TRUE(sTestDelegatePtr);
        sTestSeverInstancePtr = new ThreadBorderRouterManagement::ServerInstance(sTestEndpointId, sTestDelegatePtr);
        ASSERT_TRUE(sTestSeverInstancePtr);
    }

    static void TearDownTestSuite()
    {
        delete sTestDelegatePtr;
        delete sTestSeverInstancePtr;
    }

    void TestAttributeRead();
    void TestCommandHandle();
};

// Test ReadXX functions in ThreadBorderRouterManagement ServerInstance
TEST_F_FROM_FIXTURE(TestThreadBorderRouterManagementCluster, TestAttributeRead)
{
    BitFlags<Feature> featureMap;
    EXPECT_EQ(sTestSeverInstancePtr->ReadFeatureMap(featureMap), CHIP_NO_ERROR);
    EXPECT_TRUE(featureMap.Has(Feature::kPANChange));
    sTestDelegatePtr->mPanChangeSupported = false;
    featureMap.ClearAll();
    EXPECT_EQ(sTestSeverInstancePtr->ReadFeatureMap(featureMap), CHIP_NO_ERROR);
    EXPECT_FALSE(featureMap.Has(Feature::kPANChange));
    char borderRouterName[kBorderRouterNameMaxLength + 10];
    MutableCharSpan nameSpan = MutableCharSpan(borderRouterName);
    EXPECT_EQ(sTestSeverInstancePtr->ReadBorderRouterName(nameSpan), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    nameSpan                                      = MutableCharSpan(borderRouterName);
    sTestDelegatePtr->mUseInvalidBorderRouterName = false;
    EXPECT_EQ(sTestSeverInstancePtr->ReadBorderRouterName(nameSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(nameSpan.data_equal(CharSpan("TestName", strlen("TestName"))));
    uint8_t borderAgentId[kBorderAgentIdLength];
    MutableByteSpan agentIdSpan = MutableByteSpan(borderAgentId);
    EXPECT_EQ(sTestSeverInstancePtr->ReadBorderAgentID(agentIdSpan), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    agentIdSpan                             = MutableByteSpan(borderAgentId);
    sTestDelegatePtr->mTestBorderAgentIdLen = kBorderAgentIdLength;
    EXPECT_EQ(sTestSeverInstancePtr->ReadBorderAgentID(agentIdSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(agentIdSpan.data_equal(ByteSpan(sTestDelegatePtr->kTestBorderAgentId)));
    uint16_t threadVersion;
    EXPECT_EQ(sTestSeverInstancePtr->ReadThreadVersion(threadVersion), CHIP_NO_ERROR);
    EXPECT_EQ(threadVersion, sTestDelegatePtr->kTestThreadVersion);
    bool interfaceEnabled;
    EXPECT_EQ(sTestSeverInstancePtr->ReadInterfaceEnabled(interfaceEnabled), CHIP_NO_ERROR);
    EXPECT_FALSE(interfaceEnabled);
    Optional<uint64_t> timestamp;
    EXPECT_EQ(sTestSeverInstancePtr->ReadActiveDatasetTimestamp(timestamp), CHIP_NO_ERROR);
    EXPECT_FALSE(timestamp.HasValue());
}

TEST_F_FROM_FIXTURE(TestThreadBorderRouterManagementCluster, TestCommandHandle)
{
    Thread::OperationalDataset dataset;
    using DatasetType = Delegate::DatasetType;
    using Status      = Protocols::InteractionModel::Status;
    EXPECT_EQ(sTestSeverInstancePtr->HandleGetDatasetRequest(false, DatasetType::kActive, dataset), Status::UnsupportedAccess);
    EXPECT_EQ(sTestSeverInstancePtr->HandleGetDatasetRequest(false, DatasetType::kPending, dataset), Status::UnsupportedAccess);
    EXPECT_EQ(sTestSeverInstancePtr->HandleGetDatasetRequest(true, DatasetType::kActive, dataset), Status::NotFound);
    EXPECT_EQ(sTestSeverInstancePtr->HandleGetDatasetRequest(true, DatasetType::kPending, dataset), Status::NotFound);
    ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::DecodableType req1;
    uint8_t invalidDataset[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
    uint8_t validDataset[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0b, 0x35, 0x06,
                               0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0xde, 0xaa, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xef, 0x07,
                               0x08, 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00, 0x05, 0x10, 0xb7, 0x28, 0x08, 0x04, 0x85, 0xcf,
                               0xc5, 0x25, 0x7f, 0x68, 0x4c, 0x54, 0x9d, 0x6a, 0x57, 0x5e, 0x03, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x54,
                               0x68, 0x72, 0x65, 0x61, 0x64, 0x01, 0x02, 0xc1, 0x15, 0x04, 0x10, 0xcb, 0x13, 0x47, 0xeb, 0x0c, 0xd4,
                               0xb3, 0x5c, 0xd1, 0x42, 0xda, 0x5e, 0x6d, 0xf1, 0x8b, 0x88, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8 };
    req1.activeDataset     = ByteSpan(invalidDataset);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetActiveDatasetRequest(false, req1), Status::FailsafeRequired);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetActiveDatasetRequest(true, req1), Status::InvalidCommand);
    req1.activeDataset = ByteSpan(validDataset);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetActiveDatasetRequest(true, req1), Status::Success);
    EXPECT_EQ(sTestSeverInstancePtr->HandleGetDatasetRequest(true, DatasetType::kActive, dataset), Status::Success);
    EXPECT_TRUE(dataset.AsByteSpan().data_equal(ByteSpan(validDataset)));
    bool interfaceEnabled;
    EXPECT_EQ(sTestSeverInstancePtr->ReadInterfaceEnabled(interfaceEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(interfaceEnabled);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetActiveDatasetRequest(true, req1), Status::InvalidInState);
    Commands::SetPendingDatasetRequest::DecodableType req2;
    req2.pendingDataset = ByteSpan(validDataset);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetPendingDatasetRequest(req2), Status::UnsupportedCommand);
    sTestDelegatePtr->mPanChangeSupported = true;
    req2.pendingDataset                   = ByteSpan(invalidDataset);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetPendingDatasetRequest(req2), Status::InvalidCommand);
    req2.pendingDataset = ByteSpan(validDataset);
    EXPECT_EQ(sTestSeverInstancePtr->HandleSetPendingDatasetRequest(req2), Status::Success);
    EXPECT_EQ(sTestSeverInstancePtr->HandleGetDatasetRequest(true, DatasetType::kPending, dataset), Status::Success);
    EXPECT_TRUE(dataset.AsByteSpan().data_equal(ByteSpan(validDataset)));
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
