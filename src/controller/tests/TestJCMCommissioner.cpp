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

#include <controller/JCMCommissioner.h>
#include <controller/CommissioningDelegate.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionTable.h>

#include <credentials/tests/CHIPCert_test_vectors.h>

#include <pw_unit_test/framework.h>

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

class MockCommissioneeDeviceProxy : public CommissioneeDeviceProxy
{
public:
    MockCommissioneeDeviceProxy() {}
    CHIP_ERROR SendCommands(app::CommandSender * commandObj, Optional<System::Clock::Timeout> timeout) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetAttestationChallenge(ByteSpan & attestationChallenge) { return CHIP_NO_ERROR; }
    bool IsSecureConnected() const override { return true; }
    Messaging::ExchangeManager * GetExchangeManager() const override { return app::InteractionModelEngine::GetInstance()->GetExchangeManager(); }

    void SetUpDeviceProxy()
    {
        // auto exchangeManager = GetExchangeManager();
        // Init(ControllerDeviceInitParams{ .sessionManager = exchangeManager->GetSessionManager(), .exchangeMgr = exchangeManager },
        //      kLocalNodeId, Transport::PeerAddress::UDP(Inet::IPAddress::Any));
        // auto optionalSession = mConnections.CreateNewSecureSessionForTest(SecureSession::Type::kPASE, 2, kLocalNodeId, kCasePeerNodeId,
        //                                                              kPeerCATs, 1, kFabricIndex, GetDefaultMRPConfig());
        // SetConnected(optionalSession.Value());
    }

private:
    const NodeId kLocalNodeId      = 0xC439A991071292DB;
    const NodeId kCasePeerNodeId  = 1;
    const FabricIndex kFabricIndex = 1;
    const CATValues kPeerCATs = { { 0xABCD0001, 0xABCE0100, 0xABCD0020 } };
    SecureSessionTable mConnections;
};

class MockJCMTrustVerificationDelegate : public JCMTrustVerificationDelegate
{
public:
    void OnProgressUpdate(JCMDeviceCommissioner & commissioner, 
                          JCMTrustVerificationStage stage,
                          JCMTrustVerificationError error) override
    {
        mProgressUpdates++;
        mLastStage = stage;
        mLastError = error;
    }

    void OnAskUserForConsent(JCMDeviceCommissioner & commissioner, VendorId vendorId) override
    {
        mAskedForConsent = true;
        mLastVendorId = vendorId;
        commissioner.ContinueAfterUserConsent(mShouldConsent);
    }

    int mProgressUpdates = 0;
    JCMTrustVerificationStage mLastStage = JCMTrustVerificationStage::kIdle;
    JCMTrustVerificationError mLastError = JCMTrustVerificationResult::kSuccess;
    bool mAskedForConsent = false;
    bool mShouldConsent = true;
    VendorId mLastVendorId = VendorId::Common;
};

// TODO: May be able to remove; not currently in use
class MockClusterStateCacheCallback : public ClusterStateCache::Callback
{
    void OnDone(ReadClient *) override {}
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override {}
};

class TestJCMCommissioner : public chip::Test::AppContext
{
public:
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

    void TestParseExtraCommissioningInfo();
    void TestParseAdminFabricIndexAndEndpointId();
    void TestParseOperationalCredentials();
    void TestParseTrustedRoot();

protected:
    void SetUp() override
    {
        chip::Test::AppContext::SetUp();

        ASSERT_EQ(CHIP_NO_ERROR, SetupDeviceProxy());
        ASSERT_EQ(CHIP_NO_ERROR, SetupCommissioner());
        ASSERT_EQ(CHIP_NO_ERROR, SetupClusterStateCache());
    }

    void TearDown() override
    {
        mClusterStateCache->ClearEventCache();
        Platform::Delete(mClusterStateCache);
        delete mDeviceCommissioner;
        mTrustVerificationDelegate.reset();

        chip::Test::AppContext::TearDown(); 
    }

private:
    CHIP_ERROR SetupCommissioner()
    {
        mDeviceCommissioner = new JCMDeviceCommissioner();
        mTrustVerificationDelegate = std::make_shared<chip::Controller::MockJCMTrustVerificationDelegate>();
        mDeviceCommissioner->RegisterTrustVerificationDelegate(mTrustVerificationDelegate);

        mCommissioningParams.SetUseJCM(true);
        mAutoCommissioner.SetCommissioningParameters(mCommissioningParams);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetupDeviceProxy()
    {
        mDeviceProxy.SetUpDeviceProxy();

        return CHIP_NO_ERROR;
    }

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
        ReadClient::Callback & callback = mClusterStateCache->GetBufferedCallback();
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
        ReadClient::Callback & callback = mClusterStateCache->GetBufferedCallback();
        callback.OnAttributeData(path, &reader, aStatus);

        return CHIP_NO_ERROR;
    }

    std::vector<uint8_t> hexStringToBytes(const std::string& hex_string) {
        std::vector<uint8_t> bytes;

        if (hex_string.length() % 2 != 0) {
            return bytes;
        }

        for (size_t i = 0; i < hex_string.length(); i += 2) {
            std::string byte_str = hex_string.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    CHIP_ERROR SetupClusterStateCache()
    {
        MockClusterStateCacheCallback client;
        mClusterStateCache = Platform::New<ClusterStateCache>(client);
        mInfo.attributes = mClusterStateCache;

        // Setup JF Administrator cluster attributes
        ConcreteAttributePath adminFabricIndexPath(1, JointFabricAdministrator::Id, 
                                                 JointFabricAdministrator::Attributes::AdministratorFabricIndex::Id);
        ReturnErrorOnFailure(SetAttribute(adminFabricIndexPath, static_cast<FabricIndex>(1)));

        // TrustedRootCertificates attribute
        //static constexpr BitFlags<chip::TestCerts::TestCertLoadFlags> sNullLoadFlag;
        std::string rootString = "153001010124020137032413032C080E6A662D616E63686F722D6963616318260480228127260580254D3A370624150124110126160100FFFF26160100FEFF1824070124080130094104FC4878524D35ADD9BA150BCFE8CF1FDC294A60A2BAC0FB7BB8C5C9681CD9948823D4DD9E054DC464883311F9D12E6624B6C6410972256A58D3BA96431499473D370A350128011824020136030402040118300414E8B9760D5CB7F0500DDE598DC5FB26DAC9970AF4300514797001B5F2EEB658886340D3AAC9252B2BA4561518300B4079476C84B62BCC45D0BB6A5023F785A30B63F92E26D681E25175C5A95AF2D2D8A3B1BABDE90303F225827AF19970F39BEDBC14EF5C99ECB97A6440369886D96D18";
        auto rootBytes = hexStringToBytes(rootString);
        chip::ByteSpan rootCert(rootBytes.data(), rootBytes.size());

        //ReturnErrorOnFailure(GetTestCert(chip::TestCerts::TestCert::kRoot01, sNullLoadFlag, rootCert));
        chip::ByteSpan rootCertsData[] = { rootCert };
        DataModel::List<chip::ByteSpan> rootCerts;
        rootCerts = rootCertsData;

        ConcreteAttributePath trustedRootsPath(0, OperationalCredentials::Id, 
                                             OperationalCredentials::Attributes::TrustedRootCertificates::Id);
        ReturnErrorOnFailure(SetAttribute(trustedRootsPath, rootCerts));

        // Setup Operational Credentials cluster attributes
        // Fabrics attribute
        OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricDescriptor;
        fabricDescriptor.fabricIndex = static_cast<chip::FabricIndex>(1);
        fabricDescriptor.vendorID = static_cast<chip::VendorId>(chip::VendorId::TestVendor1); // Example vendor ID
        fabricDescriptor.fabricID = static_cast<chip::FabricId>(1234);
        fabricDescriptor.nodeID = static_cast<chip::NodeId>(1);
        
        // Create a fake public key for testing
        Credentials::P256PublicKeySpan trustedCAPublicKeySpan;
        ReturnErrorOnFailure(Credentials::ExtractPublicKeyFromChipCert(rootCert, trustedCAPublicKeySpan));
        Crypto::P256PublicKey trustedCAPublicKey{ trustedCAPublicKeySpan };
        fabricDescriptor.rootPublicKey = ByteSpan{ trustedCAPublicKey.ConstBytes(), trustedCAPublicKey.Length() };

        OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricListData[1] = { std::move(fabricDescriptor) };
        DataModel::List<const OperationalCredentials::Structs::FabricDescriptorStruct::Type> fabricsList;
        fabricsList = fabricListData;
        ConcreteAttributePath fabricsPath(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
        ReturnErrorOnFailure(SetAttributeForWrite(fabricsPath, fabricsList));

        // NOCs attribute
        OperationalCredentials::Structs::NOCStruct::Type nocStruct;
        nocStruct.fabricIndex = 1;
        
        uint8_t nocBuffer[128] = {0};
        nocStruct.noc = ByteSpan(nocBuffer, sizeof(nocBuffer));
        
        uint8_t icacBuffer[128] = {0};
        nocStruct.icac = app::DataModel::MakeNullable(ByteSpan(icacBuffer, sizeof(icacBuffer)));

        OperationalCredentials::Structs::NOCStruct::Type nocListData[1] = { nocStruct };
        DataModel::List<OperationalCredentials::Structs::NOCStruct::Type> nocsList;
        nocsList = nocListData;

        ConcreteAttributePath nocsPath(0, OperationalCredentials::Id, OperationalCredentials::Attributes::NOCs::Id);
        ReturnErrorOnFailure(SetAttributeForWrite(nocsPath, nocsList));

        return CHIP_NO_ERROR;
    }

    JCMAutoCommissioner mAutoCommissioner;
    JCMDeviceCommissioner * mDeviceCommissioner = nullptr;
    std::shared_ptr<MockJCMTrustVerificationDelegate> mTrustVerificationDelegate = nullptr;
    ClusterStateCache * mClusterStateCache = nullptr;
    ReadCommissioningInfo mInfo;
    MockCommissioneeDeviceProxy mDeviceProxy;
    CommissioningParameters mCommissioningParams;
};

// Test getting admin fabric index and endpoint ID
TEST_F_FROM_FIXTURE(TestJCMCommissioner, TestParseAdminFabricIndexAndEndpointId)
{
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);
}

// Test getting operational credentials
TEST_F_FROM_FIXTURE(TestJCMCommissioner, TestParseOperationalCredentials)
{
    // Set up the prerequisites for ParseTrustedRoot
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));

    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseOperationalCredentials(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);

    // Verify the ParseOperationalCredentials results
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootKeySpan.size(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, static_cast<FabricId>(1234));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(128));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(128));
}

// Test getting trusted root
TEST_F_FROM_FIXTURE(TestJCMCommissioner, TestParseTrustedRoot) {
    // Set up the prerequisites for ParseTrustedRoot
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseOperationalCredentials(mInfo));
    
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseTrustedRoot(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);

    // Verify the ParseOperationalCredentials results
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootKeySpan.size(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, static_cast<FabricId>(1234));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(128));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(128));

    // Verify the ParseTrustedRoot results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminRCAC.AllocatedSize(), static_cast<size_t>(270));
}

// Test parsing administrator info
TEST_F_FROM_FIXTURE(TestJCMCommissioner, TestParseExtraCommissioningInfo)
{
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseExtraCommissioningInfo(mInfo));
    
    // Verify the ParseExtraCommissioningInfo results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootKeySpan.size(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, static_cast<FabricId>(1234));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(128));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(128));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminRCAC.AllocatedSize(), static_cast<size_t>(270));
}

} // namespace Controller
} // namespace chip
