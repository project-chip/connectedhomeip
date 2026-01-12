/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/joint-fabric-administrator-server/JCMCommissionee.h>
#include <app/tests/AppTestContext.h>
#include <credentials/jcm/TrustVerification.h>
#include <credentials/tests/CHIPCert_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <access/SubjectDescriptor.h>
#include <app/CommandHandler.h>
#include <app/server/Server.h>

#include <cstring>

#include <type_traits>
#include <utility>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::JointFabricAdministrator;
using namespace chip::Credentials::JCM;

namespace chip {
namespace app {
namespace Clusters {
namespace JointFabricAdministrator {

class FakeCommandHandler : public CommandHandler
{
public:
    explicit FakeCommandHandler(FabricIndex fabricIndex = FabricIndex{ 1 }) : mFabricIndex(fabricIndex)
    {
        mSubjectDescriptor.fabricIndex = fabricIndex;
    }

    void SetExchangeContext(Messaging::ExchangeContext * exchange) { mExchangeContext = exchange; }
    void SetAccessingFabricIndex(FabricIndex fabricIndex)
    {
        mFabricIndex                   = fabricIndex;
        mSubjectDescriptor.fabricIndex = fabricIndex;
    }

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath &, const Protocols::InteractionModel::ClusterStatusCode &,
                                 const char *) override
    {
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath &, const Protocols::InteractionModel::ClusterStatusCode &, const char *) override {}

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath &, ClusterStatus) override { return CHIP_NO_ERROR; }

    CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath &, ClusterStatus) override { return CHIP_NO_ERROR; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath &, CommandId, const DataModel::EncodableToTLV &) override
    {
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath &, CommandId, const DataModel::EncodableToTLV &) override {}

    bool IsTimedInvoke() const override { return false; }

    void FlushAcksRightAwayOnSlowCommand() override {}

    FabricIndex GetAccessingFabricIndex() const override { return mFabricIndex; }

    Access::SubjectDescriptor GetSubjectDescriptor() const override { return mSubjectDescriptor; }

    Messaging::ExchangeContext * GetExchangeContext() const override { return mExchangeContext; }

private:
    FabricIndex mFabricIndex;
    Messaging::ExchangeContext * mExchangeContext = nullptr;
    Access::SubjectDescriptor mSubjectDescriptor;
};

/**
 * Intended for testing JCMCommissionee's progression through the stages of JCM
 */
class ProgressionJCMCommissionee : public JCMCommissionee
{
public:
    using Base             = JCMCommissionee;
    using OnCompletionFunc = Base::OnCompletionFunc;

    ProgressionJCMCommissionee(CommandHandler::Handle & handle, EndpointId endpointId, OnCompletionFunc onCompletion) :
        Base(handle, endpointId, std::move(onCompletion))
    {}

    TrustVerificationStage NextStage(TrustVerificationStage currentStage) { return GetNextTrustVerificationStage(currentStage); }

    void BeginVerification() { StartTrustVerification(); }

    void FinishStage(TrustVerificationStage stage, TrustVerificationError error) { TrustVerificationStageFinished(stage, error); }

    const std::vector<TrustVerificationStage> & ObservedStages() const { return mObservedStages; }

    const std::vector<TrustVerificationError> & CompletionEvents() const { return mCompletionEvents; }

protected:
    void PerformTrustVerificationStage(const TrustVerificationStage & nextStage) override { mObservedStages.push_back(nextStage); }

    void OnTrustVerificationComplete(TrustVerificationError error) override
    {
        mCompletionEvents.push_back(error);
        Base::OnTrustVerificationComplete(error);
    }

private:
    std::vector<TrustVerificationStage> mObservedStages;
    std::vector<TrustVerificationError> mCompletionEvents;
};

/**
 * Intended for testing the outcomes of a single stage of trust verification.
 */
class SingleStageJCMCommissionee : public JCMCommissionee
{
public:
    using JCMCommissionee::JCMCommissionee;

    /**
     * Set to false and ReadAttribute will invoke onError, then return CHIP_ERROR_INTERNAL.
     */
    bool mReadShouldSucceed = true;
    /**
     * Designates which stage should be invoked when Trust Verification is launched.
     */
    TrustVerificationStage mStageToRun = TrustVerificationStage::kError;
    /**
     * Error message returned by the invoked Trust Verification stage.
     */
    TrustVerificationError mError = TrustVerificationError::kInternalError;

protected:
    void OnTrustVerificationComplete(TrustVerificationError error) override { mError = error; }
    TrustVerificationStage GetNextTrustVerificationStage(const TrustVerificationStage & currentStage) override
    {
        if (currentStage == mStageToRun)
        {
            return TrustVerificationStage::kComplete;
        }

        return mStageToRun;
    }

    CHIP_ERROR ReadAdminFabricIndexAttribute(
        std::function<void(const ConcreteAttributePath &, const FabricIndexAttr::DecodableType &)> onSuccess,
        ReadErrorHandler onError) override
    {
        if (!mReadShouldSucceed)
        {
            onError(nullptr, CHIP_ERROR_INTERNAL);
            return CHIP_ERROR_INTERNAL;
        }

        ConcreteAttributePath path(mInfo.adminEndpointId, FabricIndexAttr::GetClusterId(), FabricIndexAttr::GetAttributeId());

        FabricIndexAttr::DecodableType value;
        value.SetNonNull(static_cast<FabricIndex>(1));
        onSuccess(path, value);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    ReadAdminFabricsAttribute(std::function<void(const ConcreteAttributePath &, const FabricsAttr::DecodableType &)> onSuccess,
                              ReadErrorHandler onError) override
    {
        if (!mReadShouldSucceed)
        {
            onError(nullptr, CHIP_ERROR_INTERNAL);
            return CHIP_ERROR_INTERNAL;
        }

        constexpr uint8_t kDummyRootKey[Crypto::kP256_PublicKey_Length] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
            0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
            0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
            0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41
        };

        FabricIndex fabricIndex =
            (mInfo.adminFabricIndex == kUndefinedFabricIndex) ? static_cast<FabricIndex>(1) : mInfo.adminFabricIndex;

        FabricsAttr::DecodableType value;

        uint8_t buffer[256];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

        TLV::TLVType structType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));

        ReturnErrorOnFailure(
            writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                           Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kRootPublicKey)),
                       ByteSpan(kDummyRootKey, sizeof(kDummyRootKey))));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                            Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kVendorID)),
                                        static_cast<uint16_t>(0x1234)));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                            Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kFabricID)),
                                        static_cast<FabricId>(0x1122334455667788ULL)));
        ReturnErrorOnFailure(
            writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                           Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kFabricIndex)),
                       fabricIndex));

        ReturnErrorOnFailure(writer.EndContainer(structType));
        ReturnErrorOnFailure(writer.EndContainer(outerType));

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        TLV::TLVType innerType;
        ReturnErrorOnFailure(reader.EnterContainer(innerType));

        value.SetReader(reader);
        value.SetFabricIndex(fabricIndex);

        ReturnErrorOnFailure(reader.ExitContainer(innerType));

        ConcreteAttributePath path(kRootEndpointId, FabricsAttr::GetClusterId(), FabricsAttr::GetAttributeId());
        onSuccess(path, value);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    ReadAdminCertsAttribute(std::function<void(const ConcreteAttributePath &, const CertsAttr::DecodableType &)> onSuccess,
                            ReadErrorHandler onError) override
    {
        if (!mReadShouldSucceed)
        {
            onError(nullptr, CHIP_ERROR_INTERNAL);
            return CHIP_ERROR_INTERNAL;
        }

        constexpr uint8_t kDummyRootCert[] = { 0xA1, 0xB2, 0xC3, 0xD4 };

        CertsAttr::DecodableType value;

        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), ByteSpan(kDummyRootCert, sizeof(kDummyRootCert))));
        ReturnErrorOnFailure(writer.EndContainer(outerType));

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        TLV::TLVType innerType;
        ReturnErrorOnFailure(reader.EnterContainer(innerType));

        value.SetReader(reader);

        ReturnErrorOnFailure(reader.ExitContainer(innerType));

        ConcreteAttributePath path(kRootEndpointId, CertsAttr::GetClusterId(), CertsAttr::GetAttributeId());
        onSuccess(path, value);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    ReadAdminNOCsAttribute(std::function<void(const ConcreteAttributePath &, const NOCsAttr::DecodableType &)> onSuccess,
                           ReadErrorHandler onError) override
    {
        if (!mReadShouldSucceed)
        {
            onError(nullptr, CHIP_ERROR_INTERNAL);
            return CHIP_ERROR_INTERNAL;
        }

        constexpr uint8_t kDummyNoc[]  = { 0x0A, 0x0B, 0x0C };
        constexpr uint8_t kDummyIcac[] = { 0x1A, 0x1B, 0x1C, 0x1D };

        FabricIndex fabricIndex =
            (mInfo.adminFabricIndex == kUndefinedFabricIndex) ? static_cast<FabricIndex>(1) : mInfo.adminFabricIndex;

        NOCsAttr::DecodableType value;

        uint8_t buffer[128];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

        TLV::TLVType structType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));
        ReturnErrorOnFailure(
            writer.Put(TLV::ContextTag(static_cast<uint8_t>(Clusters::OperationalCredentials::Structs::NOCStruct::Fields::kNoc)),
                       ByteSpan(kDummyNoc, sizeof(kDummyNoc))));
        ReturnErrorOnFailure(
            writer.Put(TLV::ContextTag(static_cast<uint8_t>(Clusters::OperationalCredentials::Structs::NOCStruct::Fields::kIcac)),
                       ByteSpan(kDummyIcac, sizeof(kDummyIcac))));
        ReturnErrorOnFailure(writer.Put(
            TLV::ContextTag(static_cast<uint8_t>(Clusters::OperationalCredentials::Structs::NOCStruct::Fields::kFabricIndex)),
            fabricIndex));

        ReturnErrorOnFailure(writer.EndContainer(structType));
        ReturnErrorOnFailure(writer.EndContainer(outerType));

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        TLV::TLVType innerType;
        ReturnErrorOnFailure(reader.EnterContainer(innerType));

        value.SetReader(reader);
        value.SetFabricIndex(fabricIndex);

        ReturnErrorOnFailure(reader.ExitContainer(innerType));

        ConcreteAttributePath path(kRootEndpointId, NOCsAttr::GetClusterId(), NOCsAttr::GetAttributeId());
        onSuccess(path, value);
        return CHIP_NO_ERROR;
    }
};

class TestJCMCommissionee : public chip::Testing::AppContext
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        chip::Testing::AppContext::SetUpTestSuite();
    }

    static void TearDownTestSuite()
    {
        AppContext::TearDownTestSuite();
        Platform::MemoryShutdown();
    }

protected:
    void SetUp() override { AppContext::SetUp(); }
    void TearDown() override { AppContext::TearDown(); }

    void TestNextStageFollowsExpectedOrder();
    void TestSuccessfulProgressionAdvancesAllStages();
    void TestErrorDuringStagePropagatesToCompletion();
    void TestStoreEndpointIdSuccess();
    void TestStoreEndpointIdError();
    void TestReadCommissionerAdminFabricIndexSuccess();
    void TestReadCommissionerAdminFabricIndexError();
    void TestValidateAdministratorIdsMatch();
    void TestReadAdminFabricsPopulatesCommissionerInfo();
    void TestReadAdminCertsPopulatesCommissionerRcac();
    void TestReadAdminNOCsPopulatesCommissionerCerts();
};

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestNextStageFollowsExpectedOrder)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_ERROR_INTERNAL;

    ProgressionJCMCommissionee commissionee(handle, EndpointId{ 1 }, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kIdle), TrustVerificationStage::kStoringEndpointID);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kStoringEndpointID),
              TrustVerificationStage::kReadingCommissionerAdminFabricIndex);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kReadingCommissionerAdminFabricIndex),
              TrustVerificationStage::kPerformingVendorIDVerification);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kPerformingVendorIDVerification),
              TrustVerificationStage::kCrossCheckingAdministratorIds);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kCrossCheckingAdministratorIds), TrustVerificationStage::kComplete);

    EXPECT_FALSE(completionCalled);
    EXPECT_EQ(completionError, CHIP_ERROR_INTERNAL);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestSuccessfulProgressionAdvancesAllStages)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_ERROR_INTERNAL;

    ProgressionJCMCommissionee commissionee(handle, EndpointId{ 2 }, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    commissionee.BeginVerification();

    ASSERT_EQ(commissionee.ObservedStages().size(), 1u);
    EXPECT_EQ(commissionee.ObservedStages()[0], TrustVerificationStage::kStoringEndpointID);

    commissionee.FinishStage(TrustVerificationStage::kStoringEndpointID, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 2u);
    EXPECT_EQ(commissionee.ObservedStages()[1], TrustVerificationStage::kReadingCommissionerAdminFabricIndex);

    commissionee.FinishStage(TrustVerificationStage::kReadingCommissionerAdminFabricIndex, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 3u);
    EXPECT_EQ(commissionee.ObservedStages()[2], TrustVerificationStage::kPerformingVendorIDVerification);

    commissionee.FinishStage(TrustVerificationStage::kPerformingVendorIDVerification, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 4u);
    EXPECT_EQ(commissionee.ObservedStages()[3], TrustVerificationStage::kCrossCheckingAdministratorIds);

    commissionee.FinishStage(TrustVerificationStage::kCrossCheckingAdministratorIds, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 5u);
    EXPECT_EQ(commissionee.ObservedStages()[4], TrustVerificationStage::kComplete);

    commissionee.FinishStage(TrustVerificationStage::kComplete, TrustVerificationError::kSuccess);

    ASSERT_EQ(commissionee.CompletionEvents().size(), 1u);
    EXPECT_EQ(commissionee.CompletionEvents()[0], TrustVerificationError::kSuccess);
    EXPECT_TRUE(completionCalled);
    EXPECT_EQ(completionError, CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestErrorDuringStagePropagatesToCompletion)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_NO_ERROR;

    ProgressionJCMCommissionee commissionee(handle, EndpointId{ 3 }, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    commissionee.BeginVerification();
    ASSERT_EQ(commissionee.ObservedStages().size(), 1u);

    commissionee.FinishStage(TrustVerificationStage::kStoringEndpointID, TrustVerificationError::kInternalError);

    ASSERT_EQ(commissionee.CompletionEvents().size(), 1u);
    EXPECT_EQ(commissionee.CompletionEvents()[0], TrustVerificationError::kInternalError);

    EXPECT_TRUE(completionCalled);
    EXPECT_EQ(completionError, CHIP_ERROR_INTERNAL);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestStoreEndpointIdSuccess)
{
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    constexpr EndpointId kExpectedEndpointId{ 55 };
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kInvalidEndpointId);

    SingleStageJCMCommissionee commissionee(handle, kExpectedEndpointId, [](CHIP_ERROR) {});
    commissionee.mStageToRun = TrustVerificationStage::kStoringEndpointID;

    commissionee.VerifyTrustAgainstCommissionerAdmin();

    EXPECT_EQ(commissionee.mError, TrustVerificationError::kSuccess);
    EXPECT_EQ(Server::GetInstance().GetJointFabricAdministrator().GetPeerJFAdminClusterEndpointId(), kExpectedEndpointId);

    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kInvalidEndpointId);
#else
    GTEST_SKIP() << "Joint Fabric Administrator feature disabled.";
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestStoreEndpointIdError)
{
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    constexpr EndpointId kExpectedEndpointId{ 55 };
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kExpectedEndpointId);

    SingleStageJCMCommissionee commissionee(handle, kInvalidEndpointId, [](CHIP_ERROR) {});
    commissionee.mStageToRun = TrustVerificationStage::kStoringEndpointID;

    commissionee.VerifyTrustAgainstCommissionerAdmin();

    EXPECT_EQ(commissionee.mError, TrustVerificationError::kInvalidAdministratorEndpointId);
    EXPECT_EQ(Server::GetInstance().GetJointFabricAdministrator().GetPeerJFAdminClusterEndpointId(), kExpectedEndpointId);

    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kInvalidEndpointId);
#else
    GTEST_SKIP() << "Joint Fabric Administrator feature disabled.";
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestReadCommissionerAdminFabricIndexSuccess)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    SingleStageJCMCommissionee commissionee(handle, EndpointId{ 77 }, [](CHIP_ERROR) {});
    commissionee.mStageToRun = TrustVerificationStage::kReadingCommissionerAdminFabricIndex;

    commissionee.VerifyTrustAgainstCommissionerAdmin();

    EXPECT_EQ(commissionee.mError, TrustVerificationError::kSuccess);
    EXPECT_EQ(commissionee.mInfo.adminFabricIndex, FabricIndex{ 1 });
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestReadAdminFabricsPopulatesCommissionerInfo)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    SingleStageJCMCommissionee commissionee(handle, EndpointId{ 78 }, [](CHIP_ERROR) {});
    commissionee.mInfo.adminFabricIndex = FabricIndex{ 1 };

    bool callbackCalled      = false;
    CHIP_ERROR callbackError = CHIP_ERROR_INTERNAL;

    CHIP_ERROR readError = commissionee.ReadAdminFabrics([&](CHIP_ERROR err) {
        callbackCalled = true;
        callbackError  = err;
    });

    EXPECT_EQ(readError, CHIP_NO_ERROR);
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackError, CHIP_NO_ERROR);

    EXPECT_EQ(commissionee.mInfo.adminFabricId, static_cast<FabricId>(0x1122334455667788ULL));
    EXPECT_EQ(commissionee.mInfo.adminVendorId, VendorId{ 0x1234 });

    ASSERT_EQ(commissionee.mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    ASSERT_NE(commissionee.mInfo.rootPublicKey.Get(), nullptr);
    for (size_t idx = 0; idx < Crypto::kP256_PublicKey_Length; ++idx)
    {
        EXPECT_EQ(commissionee.mInfo.rootPublicKey.Get()[idx], static_cast<uint8_t>(idx + 1));
    }
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestReadAdminCertsPopulatesCommissionerRcac)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    SingleStageJCMCommissionee commissionee(handle, EndpointId{ 79 }, [](CHIP_ERROR) {});

    bool callbackCalled      = false;
    CHIP_ERROR callbackError = CHIP_ERROR_INTERNAL;

    CHIP_ERROR readError = commissionee.ReadAdminCerts([&](CHIP_ERROR resultErr) {
        callbackCalled = true;
        callbackError  = resultErr;
    });

    EXPECT_EQ(readError, CHIP_NO_ERROR);
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackError, CHIP_NO_ERROR);

    constexpr uint8_t kExpectedRootCert[] = { 0xA1, 0xB2, 0xC3, 0xD4 };

    ASSERT_EQ(commissionee.mInfo.adminRCAC.AllocatedSize(), sizeof(kExpectedRootCert));
    ASSERT_NE(commissionee.mInfo.adminRCAC.Get(), nullptr);
    EXPECT_EQ(memcmp(commissionee.mInfo.adminRCAC.Get(), kExpectedRootCert, sizeof(kExpectedRootCert)), 0);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestReadAdminNOCsPopulatesCommissionerCerts)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    SingleStageJCMCommissionee commissionee(handle, EndpointId{ 80 }, [](CHIP_ERROR) {});
    commissionee.mInfo.adminFabricIndex = FabricIndex{ 1 };

    bool callbackCalled      = false;
    CHIP_ERROR callbackError = CHIP_ERROR_INTERNAL;

    CHIP_ERROR readError = commissionee.ReadAdminNOCs([&](CHIP_ERROR err) {
        callbackCalled = true;
        callbackError  = err;
    });

    EXPECT_EQ(readError, CHIP_NO_ERROR);
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackError, CHIP_NO_ERROR);

    constexpr uint8_t kExpectedNoc[]  = { 0x0A, 0x0B, 0x0C };
    constexpr uint8_t kExpectedIcac[] = { 0x1A, 0x1B, 0x1C, 0x1D };

    ASSERT_EQ(commissionee.mInfo.adminNOC.AllocatedSize(), sizeof(kExpectedNoc));
    ASSERT_NE(commissionee.mInfo.adminNOC.Get(), nullptr);
    EXPECT_EQ(memcmp(commissionee.mInfo.adminNOC.Get(), kExpectedNoc, sizeof(kExpectedNoc)), 0);

    ASSERT_EQ(commissionee.mInfo.adminICAC.AllocatedSize(), sizeof(kExpectedIcac));
    ASSERT_NE(commissionee.mInfo.adminICAC.Get(), nullptr);
    EXPECT_EQ(memcmp(commissionee.mInfo.adminICAC.Get(), kExpectedIcac, sizeof(kExpectedIcac)), 0);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestValidateAdministratorIdsMatch)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    SingleStageJCMCommissionee commissionee(handle, EndpointId{ 92 }, [](CHIP_ERROR) {});

    constexpr FabricId kAdminFabricId = static_cast<FabricId>(0x0123456789ABCDEFULL);
    commissionee.mInfo.adminFabricId  = kAdminFabricId;

    uint8_t adminRootKey[Crypto::kP256_PublicKey_Length];
    adminRootKey[0] = 0x04;
    for (size_t idx = 1; idx < Crypto::kP256_PublicKey_Length; ++idx)
    {
        adminRootKey[idx] = static_cast<uint8_t>(idx);
    }

    commissionee.mInfo.rootPublicKey.CopyFromSpan(ByteSpan(adminRootKey, sizeof(adminRootKey)));
    ASSERT_EQ(commissionee.mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);

    Crypto::P256PublicKey matchingKey;
    memcpy(matchingKey.Bytes(), adminRootKey, sizeof(adminRootKey));

    EXPECT_EQ(commissionee.ValidateAdministratorIdsMatch(kAdminFabricId, matchingKey), TrustVerificationError::kSuccess);

    Crypto::P256PublicKey mismatchingKey = matchingKey;
    mismatchingKey.Bytes()[Crypto::kP256_PublicKey_Length - 1] ^= 0xFF;
    EXPECT_EQ(commissionee.ValidateAdministratorIdsMatch(kAdminFabricId, mismatchingKey),
              TrustVerificationError::kAdministratorIdMismatched);

    constexpr FabricId kDifferentFabricId = static_cast<FabricId>(0xFEDCBA9876543210ULL);
    EXPECT_EQ(commissionee.ValidateAdministratorIdsMatch(kDifferentFabricId, matchingKey),
              TrustVerificationError::kAdministratorIdMismatched);

    commissionee.mInfo.rootPublicKey.CopyFromSpan(ByteSpan(adminRootKey, Crypto::kP256_PublicKey_Length - 1));
    ASSERT_EQ(commissionee.mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length - 1);
    EXPECT_EQ(commissionee.ValidateAdministratorIdsMatch(kAdminFabricId, matchingKey), TrustVerificationError::kInternalError);
}

} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
