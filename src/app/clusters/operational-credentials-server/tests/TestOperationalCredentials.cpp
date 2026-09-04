/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/operational-credentials-server/OperationalCredentialsCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/server/Server.h>
#include <clusters/OperationalCredentials/Metadata.h>
#include <credentials/CertificationDeclaration.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include <algorithm>

namespace {

using namespace chip;
using namespace chip::app;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalCredentials;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::ClusterTester;

class TestDACProvider : public Credentials::DeviceAttestationCredentialsProvider
{
public:
    static constexpr size_t kLongCdSize = Credentials::kMaxCMSSignedCDMessage;

    size_t GetLastSegmentOffset() const { return mLastSegmentOffset; }
    size_t GetLastSegmentCapacity() const { return mLastSegmentCapacity; }

    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override
    {
        return CopySpanToMutableSpan(ByteSpan(mCertificationDeclaration.data(), mCertificationDeclaration.size()), out_cd_buffer);
    }

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override
    {
        out_firmware_info_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override
    {
        return GetDeviceAttestationCertForProfile(Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy, out_dac_buffer);
    }

    CHIP_ERROR GetDeviceAttestationCertForProfile(Credentials::DeviceAttestationCertProfile profile,
                                                  MutableByteSpan & out_dac_buffer) override
    {
        return CopySpanToMutableSpan(GetDocumentForProfile(profile, mDac, mPqcDac), out_dac_buffer);
    }

    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override
    {
        return GetProductAttestationIntermediateCertForProfile(Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy,
                                                               out_pai_buffer);
    }

    CHIP_ERROR GetProductAttestationIntermediateCertForProfile(Credentials::DeviceAttestationCertProfile profile,
                                                               MutableByteSpan & out_pai_buffer) override
    {
        return CopySpanToMutableSpan(GetDocumentForProfile(profile, mPai, mPqcPai), out_pai_buffer);
    }

    Credentials::DeviceAttestationProfileSupport GetDeviceAttestationProfileSupport() const override
    {
        return {
            .paaSupportedProfiles = BitMask<Credentials::DeviceAttestationCertProfileBitmap>(
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy,
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsMlDsa44),
            .paiSupportedProfiles = BitMask<Credentials::DeviceAttestationCertProfileBitmap>(
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy,
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsMlDsa44),
            .dacSupportedProfiles = BitMask<Credentials::DeviceAttestationCertProfileBitmap>(
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy,
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsMlDsa44),
        };
    }

    CHIP_ERROR GetDeviceAttestationDocumentSegment(Credentials::DeviceAttestationDocumentType documentType,
                                                   Credentials::DeviceAttestationCertProfile profile, size_t offset,
                                                   MutableByteSpan & out_document_buffer, size_t & out_document_size) override
    {
        mLastSegmentOffset   = offset;
        mLastSegmentCapacity = out_document_buffer.size();

        ByteSpan document;
        switch (documentType)
        {
        case Credentials::DeviceAttestationDocumentType::kDACCertificate:
            document = GetDocumentForProfile(profile, mDac, mPqcDac);
            break;
        case Credentials::DeviceAttestationDocumentType::kPAICertificate:
            document = GetDocumentForProfile(profile, mPai, mPqcPai);
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        VerifyOrReturnError(!document.empty(), CHIP_ERROR_NOT_IMPLEMENTED);
        VerifyOrReturnError(offset < document.size(), CHIP_ERROR_INVALID_ARGUMENT);
        const size_t segmentSize = std::min(out_document_buffer.size(), document.size() - offset);
        out_document_size        = document.size();
        return CopySpanToMutableSpan(document.SubSpan(offset, segmentSize), out_document_buffer);
    }

    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override
    {
        out_signature_buffer.reduce_size(0);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    template <size_t kLegacySize, size_t kPqcSize>
    ByteSpan GetDocumentForProfile(Credentials::DeviceAttestationCertProfile profile,
                                   const std::array<uint8_t, kLegacySize> & legacy, const std::array<uint8_t, kPqcSize> & pqc) const
    {
        switch (profile)
        {
        case Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy:
            return ByteSpan(legacy.data(), legacy.size());
        case Credentials::DeviceAttestationCertProfile::kMlDsa44:
            return ByteSpan(pqc.data(), pqc.size());
        case Credentials::DeviceAttestationCertProfile::kMlDsa65:
            return ByteSpan();
        }

        return ByteSpan();
    }

    size_t mLastSegmentOffset   = 0;
    size_t mLastSegmentCapacity = 0;

    std::array<uint8_t, 32> mDac = [] {
        std::array<uint8_t, 32> data{};
        for (size_t i = 0; i < data.size(); ++i)
        {
            data[i] = static_cast<uint8_t>(i);
        }
        return data;
    }();

    std::array<uint8_t, 96> mPqcDac = [] {
        std::array<uint8_t, 96> data{};
        for (size_t i = 0; i < data.size(); ++i)
        {
            data[i] = static_cast<uint8_t>(0x20 + i);
        }
        return data;
    }();

    std::array<uint8_t, 48> mPai = [] {
        std::array<uint8_t, 48> data{};
        for (size_t i = 0; i < data.size(); ++i)
        {
            data[i] = static_cast<uint8_t>(0x80 + i);
        }
        return data;
    }();

    std::array<uint8_t, 128> mPqcPai = [] {
        std::array<uint8_t, 128> data{};
        for (size_t i = 0; i < data.size(); ++i)
        {
            data[i] = static_cast<uint8_t>(0x40 + i);
        }
        return data;
    }();

    std::array<uint8_t, kLongCdSize> mCertificationDeclaration = [] {
        std::array<uint8_t, kLongCdSize> data{};
        for (size_t i = 0; i < data.size(); ++i)
        {
            data[i] = static_cast<uint8_t>(i & 0xFF);
        }
        return data;
    }();
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestOperationalCredentials : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    OperationalCredentialsCluster::Context MakeContext(BitFlags<Feature> featureMap = {})
    {
        return {
            .fabricTable                = Server::GetInstance().GetFabricTable(),
            .failSafeContext            = Server::GetInstance().GetFailSafeContext(),
            .sessionManager             = Server::GetInstance().GetSecureSessionManager(),
            .dnssdServer                = app::DnssdServer::Instance(),
            .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(),
            .dacProvider                = mDacProvider,
            .groupDataProvider          = *Server::GetInstance().GetGroupDataProvider(),
            .accessControl              = Access::GetAccessControl(),
            .platformManager            = DeviceLayer::PlatformMgr(),
            .eventManagement            = EventManagement::GetInstance(),
            .featureMap                 = featureMap,
        };
    }

    TestDACProvider mDacProvider;
};

TEST_F(TestOperationalCredentials, TestAttributes)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext());

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            OperationalCredentials::Attributes::NOCs::kMetadataEntry,
                                            OperationalCredentials::Attributes::Fabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::SupportedFabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::CommissionedFabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::TrustedRootCertificates::kMetadataEntry,
                                            OperationalCredentials::Attributes::CurrentFabricIndex::kMetadataEntry,
                                        }));
}

TEST_F(TestOperationalCredentials, TestAttributesWithPQCFeature)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            OperationalCredentials::Attributes::NOCs::kMetadataEntry,
                                            OperationalCredentials::Attributes::Fabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::SupportedFabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::CommissionedFabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::TrustedRootCertificates::kMetadataEntry,
                                            OperationalCredentials::Attributes::CurrentFabricIndex::kMetadataEntry,
                                            OperationalCredentials::Attributes::PQCDeviceAttestationProfile::kMetadataEntry,
                                        }));

    Attributes::FeatureMap::TypeInfo::DecodableType featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitFlags<Feature>(Feature::kPQCDeviceAttestation).Raw());

    Attributes::PQCDeviceAttestationProfile::TypeInfo::DecodableType profile;
    ASSERT_EQ(tester.ReadAttribute(Attributes::PQCDeviceAttestationProfile::Id, profile), CHIP_NO_ERROR);
    EXPECT_EQ(profile.PAASupportedProfiles.Raw(),
              BitMask<AttestationCryptoProfileBitmap>(AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
                                                      AttestationCryptoProfileBitmap::kSupportsMlDsa44)
                  .Raw());
    EXPECT_EQ(profile.PAISupportedProfiles.Raw(), profile.PAASupportedProfiles.Raw());
    EXPECT_EQ(profile.DACSupportedProfiles.Raw(), profile.PAASupportedProfiles.Raw());
}

TEST_F(TestOperationalCredentials, TestCommands)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext());

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  OperationalCredentials::Commands::AttestationRequest::kMetadataEntry,
                                                  OperationalCredentials::Commands::CertificateChainRequest::kMetadataEntry,
                                                  OperationalCredentials::Commands::CSRRequest::kMetadataEntry,
                                                  OperationalCredentials::Commands::AddNOC::kMetadataEntry,
                                                  OperationalCredentials::Commands::UpdateNOC::kMetadataEntry,
                                                  OperationalCredentials::Commands::UpdateFabricLabel::kMetadataEntry,
                                                  OperationalCredentials::Commands::RemoveFabric::kMetadataEntry,
                                                  OperationalCredentials::Commands::AddTrustedRootCertificate::kMetadataEntry,
                                                  OperationalCredentials::Commands::SetVIDVerificationStatement::kMetadataEntry,
                                                  OperationalCredentials::Commands::SignVIDVerificationRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestLegacyModeIgnoresPQCFields)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext());
    ClusterTester tester(cluster);

    Commands::CertificateChainRequest::Type request;
    request.certificateType = CertificateChainTypeEnum::kDACCertificate;
    request.cryptoProfile.SetValue(AttestationCryptoProfileEnum::kMlDsa44);
    request.segmentID.SetValue(3);
    request.maxSegmentSize.SetValue(700);

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->certificate.size(), 32u);
    EXPECT_FALSE(result.response->totalDocumentSize.HasValue());
    EXPECT_FALSE(result.response->nextSegmentID.HasValue());
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestPQCFeatureServesLegacyRequest)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    Commands::CertificateChainRequest::Type dacRequest;
    dacRequest.certificateType = CertificateChainTypeEnum::kDACCertificate;

    auto dacResult = tester.Invoke(dacRequest);
    ASSERT_TRUE(dacResult.IsSuccess());
    ASSERT_TRUE(dacResult.response.has_value());
    EXPECT_EQ(dacResult.response->certificate.size(), 32u);
    EXPECT_EQ(dacResult.response->certificate.data()[0], 0u);
    EXPECT_FALSE(dacResult.response->totalDocumentSize.HasValue());
    EXPECT_FALSE(dacResult.response->nextSegmentID.HasValue());

    Commands::CertificateChainRequest::Type paiRequest;
    paiRequest.certificateType = CertificateChainTypeEnum::kPAICertificate;

    auto paiResult = tester.Invoke(paiRequest);
    ASSERT_TRUE(paiResult.IsSuccess());
    ASSERT_TRUE(paiResult.response.has_value());
    EXPECT_EQ(paiResult.response->certificate.size(), 48u);
    EXPECT_EQ(paiResult.response->certificate.data()[0], 0x80u);
    EXPECT_FALSE(paiResult.response->totalDocumentSize.HasValue());
    EXPECT_FALSE(paiResult.response->nextSegmentID.HasValue());
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestPQCFeatureServesRequestedPQCChain)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    Commands::CertificateChainRequest::Type dacRequest;
    dacRequest.certificateType = CertificateChainTypeEnum::kDACCertificate;
    dacRequest.cryptoProfile.SetValue(AttestationCryptoProfileEnum::kMlDsa44);

    auto dacResult = tester.Invoke(dacRequest);
    ASSERT_TRUE(dacResult.IsSuccess());
    ASSERT_TRUE(dacResult.response.has_value());
    EXPECT_EQ(dacResult.response->certificate.size(), 96u);
    EXPECT_EQ(dacResult.response->certificate.data()[0], 0x20u);
    ASSERT_TRUE(dacResult.response->totalDocumentSize.HasValue());
    EXPECT_EQ(dacResult.response->totalDocumentSize.Value(), 96u);
    EXPECT_FALSE(dacResult.response->nextSegmentID.HasValue());

    Commands::CertificateChainRequest::Type paiRequest;
    paiRequest.certificateType = CertificateChainTypeEnum::kPAICertificate;
    paiRequest.cryptoProfile.SetValue(AttestationCryptoProfileEnum::kMlDsa44);

    auto paiResult = tester.Invoke(paiRequest);
    ASSERT_TRUE(paiResult.IsSuccess());
    ASSERT_TRUE(paiResult.response.has_value());
    EXPECT_EQ(paiResult.response->certificate.size(), 128u);
    EXPECT_EQ(paiResult.response->certificate.data()[0], 0x40u);
    ASSERT_TRUE(paiResult.response->totalDocumentSize.HasValue());
    EXPECT_EQ(paiResult.response->totalDocumentSize.Value(), 128u);
    EXPECT_FALSE(paiResult.response->nextSegmentID.HasValue());
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestPQCModeRejectsUnsupportedRequests)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    Commands::CertificateChainRequest::Type unsupportedProfile;
    unsupportedProfile.certificateType = CertificateChainTypeEnum::kDACCertificate;
    unsupportedProfile.cryptoProfile.SetValue(AttestationCryptoProfileEnum::kMlDsa65);

    auto unsupportedProfileResult = tester.Invoke(unsupportedProfile);
    ASSERT_TRUE(unsupportedProfileResult.status.has_value());
    EXPECT_EQ(unsupportedProfileResult.GetStatusCode().value().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type outOfBoundsSegment;
    outOfBoundsSegment.certificateType = CertificateChainTypeEnum::kDACCertificate;
    outOfBoundsSegment.segmentID.SetValue(0);

    auto outOfBoundsResult = tester.Invoke(outOfBoundsSegment);
    ASSERT_TRUE(outOfBoundsResult.status.has_value());
    EXPECT_EQ(outOfBoundsResult.GetStatusCode().value().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type unsupportedPqcProfile;
    unsupportedPqcProfile.certificateType = CertificateChainTypeEnum::kPAICertificate;
    unsupportedPqcProfile.cryptoProfile.SetValue(AttestationCryptoProfileEnum::kMlDsa65);

    auto unsupportedPqcProfileResult = tester.Invoke(unsupportedPqcProfile);
    ASSERT_TRUE(unsupportedPqcProfileResult.status.has_value());
    EXPECT_EQ(unsupportedPqcProfileResult.GetStatusCode().value().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type outOfRangeSegment;
    outOfRangeSegment.certificateType = CertificateChainTypeEnum::kDACCertificate;
    outOfRangeSegment.cryptoProfile.SetValue(AttestationCryptoProfileEnum::kMlDsa44);
    outOfRangeSegment.segmentID.SetValue(3);

    auto outOfRangeSegmentResult = tester.Invoke(outOfRangeSegment);
    ASSERT_TRUE(outOfRangeSegmentResult.status.has_value());
    EXPECT_EQ(outOfRangeSegmentResult.status->GetUnderlyingError(), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestOperationalCredentials, TestSetCSRVendorReserved)
{
    OperationalCredentialsCluster::Context context = {
        .fabricTable                = Server::GetInstance().GetFabricTable(),
        .failSafeContext            = Server::GetInstance().GetFailSafeContext(),
        .sessionManager             = Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                = app::DnssdServer::Instance(),
        .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(),
        .dacProvider                = *Credentials::GetDeviceAttestationCredentialsProvider(),
        .groupDataProvider          = *Server::GetInstance().GetGroupDataProvider(),
        .accessControl              = Access::GetAccessControl(),
        .platformManager            = DeviceLayer::PlatformMgr(),
        .eventManagement            = EventManagement::GetInstance(),
    };
    OperationalCredentialsCluster cluster(kRootEndpointId, context);

    using Field = OperationalCredentialsCluster::CSRVendorReservedField;
    const uint8_t payload[]{ 0xAA, 0xBB, 0xCC };

    // Each of the three valid fields can be set, and a field can be cleared with an empty span.
    EXPECT_EQ(cluster.SetCSRVendorReserved(Field::kVendorReserved1, ByteSpan(payload)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetCSRVendorReserved(Field::kVendorReserved2, ByteSpan(payload)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetCSRVendorReserved(Field::kVendorReserved3, ByteSpan(payload)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetCSRVendorReserved(Field::kVendorReserved1, ByteSpan()), CHIP_NO_ERROR);

    // An out-of-range field index is rejected.
    EXPECT_EQ(cluster.SetCSRVendorReserved(static_cast<Field>(OperationalCredentialsCluster::kMaxCSRVendorReservedFields),
                                           ByteSpan(payload)),
              CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
