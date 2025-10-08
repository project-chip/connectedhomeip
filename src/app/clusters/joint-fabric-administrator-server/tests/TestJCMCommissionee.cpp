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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <access/SubjectDescriptor.h>
#include <app/CommandHandler.h>
#include <app/server/Server.h>

#include <type_traits>
#include <utility>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::JointFabricAdministrator;
using namespace chip::Credentials::JCM;

namespace {

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
    TrustVerificationError mError;
    bool mReadShouldSucceed;

protected:
    void TrustVerificationStageFinished(const TrustVerificationStage & completedStage, const TrustVerificationError & error)
    {
        mError = error;
    }

    template <typename T>
    CHIP_ERROR ReadAttribute(EndpointId endpointId,
                             std::function<void(const ConcreteAttributePath &, const typename T::DecodableType &)> onSuccess,
                             std::function<void(const ConcreteAttributePath *, CHIP_ERROR err)> onError, const bool fabricFiltered)
    {
        (void) fabricFiltered;

        using AdminFabricIndexAttr = Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::TypeInfo;
        using FabricsAttr          = Clusters::OperationalCredentials::Attributes::Fabrics::TypeInfo;
        using CertsAttr            = Clusters::OperationalCredentials::Attributes::TrustedRootCertificates::TypeInfo;
        using NocsAttr             = Clusters::OperationalCredentials::Attributes::NOCs::TypeInfo;

        if (!mReadShouldSucceed)
        {
            onError(nullptr, CHIP_ERROR_INTERNAL);
            return CHIP_ERROR_INTERNAL;
        }

        ConcreteAttributePath path(endpointId, T::GetClusterId(), T::GetAttributeId());

        if constexpr (std::is_same_v<T, AdminFabricIndexAttr>)
        {
            typename T::DecodableType value;
            value.SetNonNull(static_cast<FabricIndex>(1));
            onSuccess(path, value);
            return CHIP_NO_ERROR;
        }
        else if constexpr (std::is_same_v<T, FabricsAttr>)
        {
            constexpr uint8_t kDummyRootKey[Crypto::kP256_PublicKey_Length] = {
                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
                0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
                0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
                0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41
            };
            FabricIndex fabricIndex =
                (mInfo.adminFabricIndex == kUndefinedFabricIndex) ? static_cast<FabricIndex>(1) : mInfo.adminFabricIndex;

            typename T::DecodableType value;

            uint8_t buffer[256];
            TLV::TLVWriter writer;
            writer.Init(buffer, sizeof(buffer));
            TLV::TLVType outerType;
            CHIP_ERROR err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVType structType;
            err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            err = writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                 Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kRootPublicKey)),
                             ByteSpan(kDummyRootKey, sizeof(kDummyRootKey)));
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                     Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kVendorID)),
                                 static_cast<uint16_t>(0x1234));
            }
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                     Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kFabricID)),
                                 static_cast<FabricId>(0x1122334455667788ULL));
            }
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                     Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Fields::kFabricIndex)),
                                 fabricIndex);
            }
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            err = writer.EndContainer(structType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            err = writer.EndContainer(outerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVReader reader;
            reader.Init(buffer, writer.GetLengthWritten());
            err = reader.Next();
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVType innerType;
            err = reader.EnterContainer(innerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            value.SetReader(reader);
            value.SetFabricIndex(fabricIndex);

            err = reader.ExitContainer(innerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            onSuccess(path, value);
            return CHIP_NO_ERROR;
        }
        else if constexpr (std::is_same_v<T, CertsAttr>)
        {
            constexpr uint8_t kDummyRootCert[] = { 0xA1, 0xB2, 0xC3, 0xD4 };

            typename T::DecodableType value;

            uint8_t buffer[64];
            TLV::TLVWriter writer;
            writer.Init(buffer, sizeof(buffer));
            TLV::TLVType outerType;
            CHIP_ERROR err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType);
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(TLV::AnonymousTag(), ByteSpan(kDummyRootCert, sizeof(kDummyRootCert)));
            }
            if (err == CHIP_NO_ERROR)
            {
                err = writer.EndContainer(outerType);
            }
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVReader reader;
            reader.Init(buffer, writer.GetLengthWritten());
            err = reader.Next();
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVType innerType;
            err = reader.EnterContainer(innerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            value.SetReader(reader);

            err = reader.ExitContainer(innerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            onSuccess(path, value);
            return CHIP_NO_ERROR;
        }
        else if constexpr (std::is_same_v<T, NocsAttr>)
        {
            constexpr uint8_t kDummyNoc[]  = { 0x0A, 0x0B, 0x0C };
            constexpr uint8_t kDummyIcac[] = { 0x1A, 0x1B, 0x1C, 0x1D };

            FabricIndex fabricIndex =
                (mInfo.adminFabricIndex == kUndefinedFabricIndex) ? static_cast<FabricIndex>(1) : mInfo.adminFabricIndex;

            typename T::DecodableType value;

            uint8_t buffer[128];
            TLV::TLVWriter writer;
            writer.Init(buffer, sizeof(buffer));
            TLV::TLVType outerType;
            CHIP_ERROR err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVType structType;
            err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType);
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(
                    TLV::ContextTag(static_cast<uint8_t>(Clusters::OperationalCredentials::Structs::NOCStruct::Fields::kNoc)),
                    ByteSpan(kDummyNoc, sizeof(kDummyNoc)));
            }
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(
                    TLV::ContextTag(static_cast<uint8_t>(Clusters::OperationalCredentials::Structs::NOCStruct::Fields::kIcac)),
                    ByteSpan(kDummyIcac, sizeof(kDummyIcac)));
            }
            if (err == CHIP_NO_ERROR)
            {
                err = writer.Put(TLV::ContextTag(static_cast<uint8_t>(
                                     Clusters::OperationalCredentials::Structs::NOCStruct::Fields::kFabricIndex)),
                                 fabricIndex);
            }
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            err = writer.EndContainer(structType);
            if (err == CHIP_NO_ERROR)
            {
                err = writer.EndContainer(outerType);
            }
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVReader reader;
            reader.Init(buffer, writer.GetLengthWritten());
            err = reader.Next();
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            TLV::TLVType innerType;
            err = reader.EnterContainer(innerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            value.SetReader(reader);
            value.SetFabricIndex(fabricIndex);

            err = reader.ExitContainer(innerType);
            if (err != CHIP_NO_ERROR)
            {
                onError(&path, err);
                return err;
            }

            onSuccess(path, value);
            return CHIP_NO_ERROR;
        }
        else
        {
            onError(&path, CHIP_ERROR_NOT_IMPLEMENTED);
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    }
};

class TestJCMCommissionee : public chip::Test::AppContext
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        chip::Test::AppContext::SetUpTestSuite();
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
    void TestStoreEndpointId();
    void TestReadCommissionerAdminFabricIndex();
};

} // namespace

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

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestStoreEndpointId)
{
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    class StoreEndpointIdOnlyCommissionee : public JCMCommissionee
    {
    public:
        StoreEndpointIdOnlyCommissionee(CommandHandler::Handle & handle, EndpointId endpointId, OnCompletionFunc onCompletion) :
            JCMCommissionee(handle, endpointId, std::move(onCompletion))
        {}

    protected:
        TrustVerificationStage GetNextTrustVerificationStage(const TrustVerificationStage & currentStage) override
        {
            switch (currentStage)
            {
            case TrustVerificationStage::kIdle:
                return TrustVerificationStage::kStoringEndpointID;
            case TrustVerificationStage::kStoringEndpointID:
                return TrustVerificationStage::kComplete;
            default:
                return TrustVerificationStage::kComplete;
            }
        }

        void PerformTrustVerificationStage(const TrustVerificationStage & nextStage) override
        {
            if (nextStage == TrustVerificationStage::kStoringEndpointID || nextStage == TrustVerificationStage::kComplete)
            {
                JCMCommissionee::PerformTrustVerificationStage(nextStage);
                return;
            }

            TrustVerificationStageFinished(nextStage, TrustVerificationError::kInternalError);
        }
    };

    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_ERROR_INTERNAL;

    constexpr EndpointId kExpectedEndpointId{ 55 };
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kInvalidEndpointId);

    StoreEndpointIdOnlyCommissionee commissionee(handle, kExpectedEndpointId, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    commissionee.VerifyTrustAgainstCommissionerAdmin();

    EXPECT_TRUE(completionCalled);
    EXPECT_EQ(completionError, CHIP_NO_ERROR);
    EXPECT_EQ(Server::GetInstance().GetJointFabricAdministrator().GetPeerJFAdminClusterEndpointId(), kExpectedEndpointId);

    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kInvalidEndpointId);
#else
    GTEST_SKIP() << "Joint Fabric Administrator feature disabled.";
#endif
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, TestReadCommissionerAdminFabricIndex)
{
    // TODO: implement
}
