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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <controller/CommissioningDelegate.h>
#include <controller/jcm/AutoCommissioner.h>
#include <controller/jcm/DeviceCommissioner.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <pw_unit_test/framework.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionTable.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::Transport;
using namespace chip::Messaging;

namespace chip {
namespace Controller {
namespace JCM {

class MockTrustVerificationDelegate : public TrustVerificationDelegate
{
public:
    void OnProgressUpdate(DeviceCommissioner & commissioner, TrustVerificationStage stage, TrustVerificationInfo & info,
                          TrustVerificationError error) override
    {
        mProgressUpdates++;
        mLastStage = stage;
        mLastError = error;
    }

    void OnAskUserForConsent(DeviceCommissioner & commissioner, TrustVerificationInfo & info) override
    {
        mAskedForConsent = true;
        mLastVendorId    = info.adminVendorId;
        commissioner.ContinueAfterUserConsent(mShouldConsent);
    }

    void OnVerifyVendorId(DeviceCommissioner & commissioner, TrustVerificationInfo & info) override
    {
        mAskedForVendorIdVerification = true;
        commissioner.ContinueAfterVendorIDVerification(mShouldVerifyVendorId);
    }

    int mProgressUpdates               = 0;
    TrustVerificationStage mLastStage  = TrustVerificationStage::kIdle;
    TrustVerificationError mLastError  = TrustVerificationError::kSuccess;
    bool mAskedForConsent              = false;
    bool mShouldConsent                = true;
    bool mAskedForVendorIdVerification = false;
    bool mShouldVerifyVendorId         = true;
    VendorId mLastVendorId             = VendorId::Common;
};

class MockClusterStateCache : public ClusterStateCache
{
public:
    MockClusterStateCache() : ClusterStateCache(mClusterStateCacheCallback) {}

    class MockClusterStateCacheCallback : public ClusterStateCache::Callback
    {
        void OnDone(ReadClient *) override {}
        void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override {}
    };

    CHIP_ERROR SetUp()
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        // Setup JF Administrator cluster attributes
        ConcreteAttributePath adminFabricIndexPath(1, JointFabricAdministrator::Id,
                                                   JointFabricAdministrator::Attributes::AdministratorFabricIndex::Id);
        err = SetAttribute(adminFabricIndexPath, static_cast<FabricIndex>(1));
        ReturnErrorOnFailure(err);

        // TrustedRootCertificates attribute
        // static constexpr BitFlags<chip::TestCerts::TestCertLoadFlags> sNullLoadFlag;
        std::string rcacString =
            "153001010124020137032413032C080E6A662D616E63686F722D6963616318260480228127260580254D3A370624150124110126160100FFFF2616"
            "0100FEFF1824070124080130094104FC4878524D35ADD9BA150BCFE8CF1FDC294A60A2BAC0FB7BB8C5C9681CD9948823D4DD9E054DC464883311F9"
            "D12E6624B6C6410972256A58D3BA96431499473D370A350128011824020136030402040118300414E8B9760D5CB7F0500DDE598DC5FB26DAC9970A"
            "F4300514797001B5F2EEB658886340D3AAC9252B2BA4561518300B4079476C84B62BCC45D0BB6A5023F785A30B63F92E26D681E25175C5A95AF2D2"
            "D8A3B1BABDE90303F225827AF19970F39BEDBC14EF5C99ECB97A6440369886D96D18";
        auto rcacBytes = hexStringToBytes(rcacString);
        chip::ByteSpan rcac(rcacBytes.data(), rcacBytes.size());

        chip::ByteSpan rcacCertsData[] = { rcac };
        DataModel::List<chip::ByteSpan> rcacCerts;
        rcacCerts = rcacCertsData;

        ConcreteAttributePath trustedRootsPath(0, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::TrustedRootCertificates::Id);
        err = SetAttribute(trustedRootsPath, rcacCerts);
        ReturnErrorOnFailure(err);

        // Setup Operational Credentials cluster attributes
        // Fabrics attribute
        OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricDescriptor;
        fabricDescriptor.fabricIndex = static_cast<chip::FabricIndex>(1);
        fabricDescriptor.vendorID    = static_cast<chip::VendorId>(chip::VendorId::TestVendor1); // Example vendor ID
        fabricDescriptor.fabricID    = static_cast<chip::FabricId>(1234);
        fabricDescriptor.nodeID      = static_cast<chip::NodeId>(1);

        // Create a fake public key for testing
        Credentials::P256PublicKeySpan trustedCAPublicKeySpan;
        err = Credentials::ExtractPublicKeyFromChipCert(rcac, trustedCAPublicKeySpan);
        ReturnErrorOnFailure(err);

        Crypto::P256PublicKey trustedCAPublicKey{ trustedCAPublicKeySpan };
        fabricDescriptor.rootPublicKey = ByteSpan{ trustedCAPublicKey.ConstBytes(), trustedCAPublicKey.Length() };

        OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricListData[1] = { std::move(fabricDescriptor) };
        DataModel::List<const OperationalCredentials::Structs::FabricDescriptorStruct::Type> fabricsList;
        fabricsList = fabricListData;
        ConcreteAttributePath fabricsPath(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
        err = SetAttributeForWrite(fabricsPath, fabricsList);
        ReturnErrorOnFailure(err);

        // NOCs attribute
        OperationalCredentials::Structs::NOCStruct::Type nocStruct;
        nocStruct.fabricIndex = 1;

        std::string nocString =
            "153001010124020137032413032C080E6A662D616E63686F722D6963616318260480228127260580254D3A370624150124110B26160100FFFF2616"
            "0100FEFF1824070124080130094104A32EFB8E9D2BDFE01911600064D9B9CE7A4B3D24188EFF0942A3889261D4CEFCCC8109FBBF8C65F23B41C922"
            "0EBCF8CD5B162039524CA9263D90B6884A800A4F370A3501280118240201360304020401183004140E1347C63F35CCDA5382AE29D1E42B1C4BD340"
            "0B30051418B72CD295F75A805D5AC41B9A13F13C9DB74D0218300B40DAE0BC25977BC590359BAA15BDFB28C7DEE05C7F8221EBE174CF75BFFB7320"
            "F6CE5D1FE562287735C1879FEBC3598E48EBDD98A8F8DF58914C3EF5631B4DC03518";
        auto nocBytes = hexStringToBytes(nocString);
        chip::ByteSpan noc(nocBytes.data(), nocBytes.size());
        nocStruct.noc = noc;

        std::string icacString =
            "1530010101240201370324140118260480228127260580254D3A37062413032C080E6A662D616E63686F722D696361631824070124080130094104"
            "4192347068FE0999BDE90BC853DEC5AA7E45DAB387567AD165F539B1F36B3B1E5A56E14AD849EDBDD5FD7E42C89B85EF458D2643E2BFE5D8286F49"
            "397FC73E21370A350129011824026030041418B72CD295F75A805D5AC41B9A13F13C9DB74D02300514E564D5D4948410F8B108C5EA8E12B43ACF8D"
            "4A4918300B40DF0A62FF24ED10C91B754A14D712C04C4041CDD5963A5954BD542748A05B2B7F5E53E2FADE8F3D1F1CE3FCE3D1B2723E38698AB400"
            "E1AABAEF6456790651631118";
        auto icacBytes = hexStringToBytes(icacString);
        chip::ByteSpan icac(icacBytes.data(), icacBytes.size());
        nocStruct.icac = icac;

        OperationalCredentials::Structs::NOCStruct::Type nocListData[1] = { nocStruct };
        DataModel::List<OperationalCredentials::Structs::NOCStruct::Type> nocsList;
        nocsList = nocListData;

        ConcreteAttributePath nocsPath(0, OperationalCredentials::Id, OperationalCredentials::Attributes::NOCs::Id);
        err = SetAttributeForWrite(nocsPath, nocsList);
        ReturnErrorOnFailure(err);

        return err;
    }

    void TearDown() { ClearEventCache(); }

    template <typename AttrType>
    CHIP_ERROR SetAttribute(const ConcreteAttributePath & path, const AttrType data)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> handle;
        handle.Calloc(3000);
        TLV::ScopedBufferTLVWriter writer(std::move(handle), 3000);
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), data));
        uint32_t writtenLength = writer.GetLengthWritten();
        ReturnErrorOnFailure(writer.Finalize(handle));

        TLV::ScopedBufferTLVReader reader;
        StatusIB aStatus;
        reader.Init(std::move(handle), writtenLength);
        ReturnErrorOnFailure(reader.Next());
        ReadClient::Callback & callback = GetBufferedCallback();
        callback.OnAttributeData(path, &reader, aStatus);

        return CHIP_NO_ERROR;
    }

    template <typename AttrType>
    CHIP_ERROR SetAttributeForWrite(const ConcreteAttributePath & path, const AttrType data)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> handle;
        handle.Calloc(3000);
        TLV::ScopedBufferTLVWriter writer(std::move(handle), 3000);
        ReturnErrorOnFailure(DataModel::EncodeForRead(writer, TLV::AnonymousTag(), static_cast<chip::FabricIndex>(1), data));
        uint32_t writtenLength = writer.GetLengthWritten();
        ReturnErrorOnFailure(writer.Finalize(handle));

        TLV::ScopedBufferTLVReader reader;
        StatusIB aStatus;
        reader.Init(std::move(handle), writtenLength);
        ReturnErrorOnFailure(reader.Next());
        ReadClient::Callback & callback = GetBufferedCallback();
        callback.OnAttributeData(path, &reader, aStatus);

        return CHIP_NO_ERROR;
    }

    std::vector<uint8_t> hexStringToBytes(const std::string & hex_string)
    {
        std::vector<uint8_t> bytes;

        if (hex_string.length() % 2 != 0)
        {
            return bytes;
        }

        for (size_t i = 0; i < hex_string.length(); i += 2)
        {
            std::string byte_str = hex_string.substr(i, 2);
            uint8_t byte         = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

private:
    MockClusterStateCacheCallback mClusterStateCacheCallback;
};

class TestableDeviceCommissioner : public DeviceCommissioner
{
public:
    void OnTrustVerificationComplete(TrustVerificationError result) override
    {
        mResult = result;

        ChipLogProgress(Controller, "TestableDeviceCommissioner::OnTrustVerificationComplete called with result: %hu",
                        static_cast<uint16_t>(result));
    }

    TrustVerificationError mResult;
};

class TestCommissioner : public chip::Test::AppContext
{
public:
    TestCommissioner() { mInfo.attributes = &mClusterStateCache; }

    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

        chip::Test::AppContext::SetUpTestSuite();
    }

    // Performs shared teardown for all tests in the test suite.  Run once for the whole suite.
    static void TearDownTestSuite()
    {
        chip::Test::AppContext::TearDownTestSuite();

        chip::Platform::MemoryShutdown();
    }

    void TestTrustVerificationStageFinishedProgressesThroughStages();
    void TestTrustVerificationStageFinishedHandlesUserConsent();
    void TestTrustVerificationStageFinishedHandlesError();
    void TestParseExtraCommissioningInfo();
    void TestParseAdminFabricIndexAndEndpointId();
    void TestParseOperationalCredentials();
    void TestParseTrustedRoot();

protected:
    void SetUp() override
    {
        chip::Test::AppContext::SetUp();

        mDeviceCommissioner = new DeviceCommissioner();
        mDeviceCommissioner->RegisterTrustVerificationDelegate(&mTrustVerificationDelegate);

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        mCommissioningParams.SetUseJCM(true);
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        mAutoCommissioner.SetCommissioningParameters(mCommissioningParams);

        CHIP_ERROR err = mClusterStateCache.SetUp();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "MockClusterStateCache::SetUp failed with error: %s", ErrorStr(err));
        }
    }

    void TearDown() override
    {
        mClusterStateCache.TearDown();

        delete mDeviceCommissioner;
        mDeviceCommissioner = nullptr;

        chip::Test::AppContext::TearDown();
    }

private:
    AutoCommissioner mAutoCommissioner;
    DeviceCommissioner * mDeviceCommissioner = nullptr;
    MockTrustVerificationDelegate mTrustVerificationDelegate;
    MockClusterStateCache mClusterStateCache;
    ReadCommissioningInfo mInfo;
    CommissioningParameters mCommissioningParams;
};

TEST_F_FROM_FIXTURE(TestCommissioner, TestTrustVerificationStageFinishedProgressesThroughStages)
{
    TestableDeviceCommissioner commissioner;

    // Simulate user consenting
    mTrustVerificationDelegate.mShouldConsent = true;
    // Register the mock trust verification delegate
    commissioner.RegisterTrustVerificationDelegate(&mTrustVerificationDelegate);
    // Set up the mock ReadCommissioningInfo
    commissioner.ParseExtraCommissioningInfo(mInfo, mCommissioningParams);

    TrustVerificationStage stage = TrustVerificationStage::kIdle;
    TrustVerificationError error = TrustVerificationError::kSuccess;

    // Start at Started, advance through all stages

    // Advance to kAskUserForConsent (should trigger consent)
    commissioner.TrustVerificationStageFinished(stage, error);
    EXPECT_EQ(mTrustVerificationDelegate.mProgressUpdates, 5); // Progress not incremented for consent
    EXPECT_EQ(mTrustVerificationDelegate.mLastStage, TrustVerificationStage::kComplete);
    EXPECT_EQ(mTrustVerificationDelegate.mLastError, TrustVerificationError::kSuccess);
    EXPECT_TRUE(mTrustVerificationDelegate.mAskedForConsent);
    EXPECT_TRUE(mTrustVerificationDelegate.mAskedForVendorIdVerification);
    EXPECT_EQ(mTrustVerificationDelegate.mLastVendorId, chip::VendorId(65521));
}

TEST_F_FROM_FIXTURE(TestCommissioner, TestTrustVerificationStageFinishedHandlesUserConsent)
{
    TestableDeviceCommissioner commissioner;
    mTrustVerificationDelegate.mShouldConsent = false; // Simulate user rejecting consent
    commissioner.RegisterTrustVerificationDelegate(&mTrustVerificationDelegate);

    // Simulate reaching consent stage
    commissioner.mInfo.adminFabricIndex = 1;
    commissioner.mInfo.adminEndpointId  = 1;

    TrustVerificationStage stage = TrustVerificationStage::kPerformingVendorIDVerification;
    TrustVerificationError error = TrustVerificationError::kSuccess;

    commissioner.TrustVerificationStageFinished(stage, error);
    EXPECT_TRUE(mTrustVerificationDelegate.mAskedForConsent);
    EXPECT_EQ(mTrustVerificationDelegate.mProgressUpdates, 2); // Only OnProgressUpdate called for error
    EXPECT_EQ(mTrustVerificationDelegate.mLastStage, TrustVerificationStage::kAskingUserForConsent);
    EXPECT_EQ(mTrustVerificationDelegate.mLastError, TrustVerificationError::kUserDeniedConsent);
}

TEST_F_FROM_FIXTURE(TestCommissioner, TestTrustVerificationStageFinishedHandlesError)
{
    TestableDeviceCommissioner commissioner;
    mTrustVerificationDelegate.mShouldVerifyVendorId = false; // Simulate vendor id verification failure
    commissioner.RegisterTrustVerificationDelegate(&mTrustVerificationDelegate);

    TrustVerificationStage stage = TrustVerificationStage::kVerifyingAdministratorInformation;
    TrustVerificationError error = TrustVerificationError::kSuccess;

    // Simulate error at operational credentials stage
    commissioner.TrustVerificationStageFinished(stage, error);
    EXPECT_EQ(mTrustVerificationDelegate.mProgressUpdates, 2);
    EXPECT_EQ(mTrustVerificationDelegate.mLastStage, TrustVerificationStage::kPerformingVendorIDVerification);
    EXPECT_EQ(mTrustVerificationDelegate.mLastError, TrustVerificationError::kVendorIdVerificationFailed);
}

// Test getting admin fabric index and endpoint ID
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseAdminFabricIndexAndEndpointId)
{
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);
}

// Test getting operational credentials
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseOperationalCredentials)
{
    // Set up the prerequisites for ParseTrustedRoot
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));

    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseOperationalCredentials(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);

    // Verify the ParseOperationalCredentials results
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, static_cast<FabricId>(1234));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(270));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(248));
}

// Test getting trusted root
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseTrustedRoot)
{
    // Set up the prerequisites for ParseTrustedRoot
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseOperationalCredentials(mInfo));

    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseTrustedRoot(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);

    // Verify the ParseOperationalCredentials results
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, static_cast<FabricId>(1234));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(270));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(248));

    // Verify the ParseTrustedRoot results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminRCAC.AllocatedSize(), static_cast<size_t>(270));
}

// Test parsing administrator info
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseExtraCommissioningInfo)
{
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseExtraCommissioningInfo(mInfo, mCommissioningParams));

    // Verify the ParseExtraCommissioningInfo results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, static_cast<FabricId>(1234));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(270));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(248));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminRCAC.AllocatedSize(), static_cast<size_t>(270));
}
} // namespace JCM
} // namespace Controller
} // namespace chip
