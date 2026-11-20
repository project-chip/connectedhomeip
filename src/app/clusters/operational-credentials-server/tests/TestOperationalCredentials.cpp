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
#include <clusters/OperationalCredentials/Metadata.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <messaging/ExchangeMgr.h>
#include <transport/SessionManager.h>
#include <transport/raw/MessageHeader.h>

#include <algorithm>
#include <limits>

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

// Mirrors the segment size the cluster serves and the floor the spec puts on MaxSegmentSize.
constexpr uint16_t kDefaultCertificateSegmentSize = 600;
constexpr uint16_t kMaxCertificateSegmentSize     = 900;

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
            .PAASupportedProfiles = BitMask<Credentials::DeviceAttestationCertProfileBitmap>(
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy,
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsMlDsa44),
            .PAISupportedProfiles = BitMask<Credentials::DeviceAttestationCertProfileBitmap>(
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy),
            .DACSupportedProfiles = BitMask<Credentials::DeviceAttestationCertProfileBitmap>(
                Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy),
        };
    }

    Credentials::DeviceAttestationCertProfile GetPreferredDeviceAttestationChainProfile() const override
    {
        return Credentials::DeviceAttestationCertProfile::kMlDsa44;
    }

    CHIP_ERROR GetDeviceAttestationDocumentSegmentForChain(Credentials::DeviceAttestationDocumentType documentType,
                                                           const Credentials::DeviceAttestationChainProfile & chain, size_t offset,
                                                           MutableByteSpan & buffer, size_t & totalSize) override
    {
        VerifyOrReturnError(chain.PAICrypto == AttestationCryptoProfileEnum::kEcdsaMatterLegacy &&
                                chain.DACCrypto == AttestationCryptoProfileEnum::kEcdsaMatterLegacy &&
                                chain.PAACrypto <= AttestationCryptoProfileEnum::kMlDsa44,
                            CHIP_ERROR_NOT_FOUND);
        return GetDeviceAttestationDocumentSegment(documentType, chain.PAACrypto, offset, buffer, totalSize);
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
        case Credentials::DeviceAttestationCertProfile::kUnknownEnumValue:
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

    OperationalCredentialsCluster::Context MakeContext(BitFlags<Feature> featureMap                                 = {},
                                                       Credentials::DeviceAttestationCredentialsProvider * provider = nullptr)
    {
        mCommissioningWindowManager.Init({
            .fabricTable = mFabricTable, .sessionManager = mSessionManager, .exchangeManager = mExchangeManager,
            .failSafeContext = mFailSafeContext,
#if CONFIG_NETWORK_LAYER_BLE
            .bleLayer = mBleLayer,
#endif
        });
        return {
            .fabricTable                = mFabricTable,
            .failSafeContext            = mFailSafeContext,
            .sessionManager             = mSessionManager,
            .dnssdServer                = app::DnssdServer::Instance(),
            .commissioningWindowManager = mCommissioningWindowManager,
            .dacProvider                = provider != nullptr ? *provider : mDacProvider,
            .groupDataProvider          = mGroupDataProvider,
            .accessControl              = Access::GetAccessControl(),
            .platformManager            = DeviceLayer::PlatformMgr(),
            .eventManagement            = EventManagement::GetInstance(),
            .featureMap                 = featureMap,
        };
    }

    FabricTable mFabricTable;
    FailSafeContext mFailSafeContext;
    SessionManager mSessionManager;
    Messaging::ExchangeManager mExchangeManager;
    CommissioningWindowManager mCommissioningWindowManager;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    TestDACProvider mDacProvider;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer mBleLayer;
#endif
};

TEST_F(TestOperationalCredentials, TestPQCProviderRequirements)
{
    using Profile = Credentials::DeviceAttestationCertProfileBitmap;
    class ProfileProvider : public TestDACProvider
    {
    public:
        Credentials::DeviceAttestationProfileSupport profiles{
            BitMask<Profile>(Profile::kSupportsEcdsaMatterLegacy),
            BitMask<Profile>(Profile::kSupportsEcdsaMatterLegacy),
            BitMask<Profile>(Profile::kSupportsEcdsaMatterLegacy),
        };

        Credentials::DeviceAttestationProfileSupport GetDeviceAttestationProfileSupport() const override { return profiles; }
    } provider;

    // The default legacy provider cannot enable PQC, but remains usable without the feature.
    EXPECT_FALSE(provider.HasRequiredPqcCredentials());
    auto context       = MakeContext();
    auto legacyContext = OperationalCredentialsCluster::Context{ context.fabricTable,
                                                                 context.failSafeContext,
                                                                 context.sessionManager,
                                                                 context.dnssdServer,
                                                                 context.commissioningWindowManager,
                                                                 provider,
                                                                 context.groupDataProvider,
                                                                 context.accessControl,
                                                                 context.platformManager,
                                                                 context.eventManagement,
                                                                 context.featureMap };
    OperationalCredentialsCluster cluster(kRootEndpointId, legacyContext);
    ClusterTester tester(cluster);
    Attributes::FeatureMap::TypeInfo::DecodableType featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    // PQC DAC support alone is insufficient: a PQC issuer is required.
    provider.profiles.DACSupportedProfiles.Set(Profile::kSupportsMlDsa44);
    EXPECT_FALSE(provider.HasRequiredPqcCredentials());
    provider.profiles.DACSupportedProfiles.Clear(Profile::kSupportsMlDsa44);

    for (auto pqcProfile : { Profile::kSupportsMlDsa44, Profile::kSupportsMlDsa65 })
    {
        provider.profiles.PAASupportedProfiles.Set(pqcProfile);
        EXPECT_TRUE(provider.HasRequiredPqcCredentials());
        provider.profiles.PAASupportedProfiles.Clear(pqcProfile);
        provider.profiles.PAISupportedProfiles.Set(pqcProfile);
        EXPECT_TRUE(provider.HasRequiredPqcCredentials());

        // Every chain element must retain legacy support even when a PQC issuer is available.
        for (auto * profiles : { &provider.profiles.PAASupportedProfiles, &provider.profiles.PAISupportedProfiles,
                                 &provider.profiles.DACSupportedProfiles })
        {
            profiles->Clear(Profile::kSupportsEcdsaMatterLegacy);
            EXPECT_FALSE(provider.HasRequiredPqcCredentials());
            profiles->Set(Profile::kSupportsEcdsaMatterLegacy);
        }
        provider.profiles.PAISupportedProfiles.Clear(pqcProfile);
    }
}

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
    EXPECT_EQ(profile.PAISupportedProfiles.Raw(),
              BitMask<AttestationCryptoProfileBitmap>(AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy).Raw());
    EXPECT_EQ(profile.DACSupportedProfiles.Raw(), profile.PAISupportedProfiles.Raw());
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
    request.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
    request.cryptoProfile.Emplace().DACCrypto = AttestationCryptoProfileEnum::kMlDsa44;
    request.segmentID.SetValue(3);
    request.maxSegmentSize.SetValue(700);

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (!result.response.has_value())
    {
        return;
    }
    const auto & response = result.response.value();
    EXPECT_EQ(response.certificate.size(), 32u);
    EXPECT_FALSE(response.totalDocumentSize.HasValue());
    EXPECT_FALSE(response.nextSegmentID.HasValue());
}

TEST_F(TestOperationalCredentials, TestCertificateChainDocumentSizeLimit)
{
    class CertificateSizeProvider : public TestDACProvider
    {
    public:
        size_t documentSize = Credentials::kMaxDERCertLengthMlDsa65;

        CHIP_ERROR GetDeviceAttestationDocumentSegment(Credentials::DeviceAttestationDocumentType documentType,
                                                       Credentials::DeviceAttestationCertProfile profile, size_t offset,
                                                       MutableByteSpan & buffer, size_t & totalSize) override
        {
            totalSize = documentSize;
            memset(buffer.data(), 0, buffer.size());
            return CHIP_NO_ERROR;
        }
    } provider;
    auto context     = MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation));
    auto sizeContext = OperationalCredentialsCluster::Context{ context.fabricTable,
                                                               context.failSafeContext,
                                                               context.sessionManager,
                                                               context.dnssdServer,
                                                               context.commissioningWindowManager,
                                                               provider,
                                                               context.groupDataProvider,
                                                               context.accessControl,
                                                               context.platformManager,
                                                               context.eventManagement,
                                                               context.featureMap };
    OperationalCredentialsCluster cluster(kRootEndpointId, sizeContext);
    ClusterTester tester(cluster);
    Commands::CertificateChainRequest::Type request;
    request.certificateType                   = CertificateChainTypeEnum::kPAICertificate;
    request.cryptoProfile.Emplace().PAICrypto = AttestationCryptoProfileEnum::kEcdsaMatterLegacy;

    auto accepted = tester.Invoke(request);
    ASSERT_TRUE(accepted.IsSuccess());
    ASSERT_TRUE(accepted.response.has_value());
    if (!accepted.response.has_value())
    {
        return;
    }
    const auto & acceptedResponse = accepted.response.value();
    ASSERT_TRUE(acceptedResponse.totalDocumentSize.HasValue());
    EXPECT_EQ(acceptedResponse.totalDocumentSize.Value(), Credentials::kMaxDERCertLengthMlDsa65);

    // Provider metadata larger than the supported DER certificate limit must be rejected.
    ++provider.documentSize;
    auto rejected = tester.Invoke(request);
    EXPECT_FALSE(rejected.IsSuccess());
    EXPECT_FALSE(rejected.response.has_value());
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
    if (!dacResult.response.has_value())
    {
        return;
    }
    const auto & dacResponse = dacResult.response.value();
    EXPECT_EQ(dacResponse.certificate.size(), 32u);
    EXPECT_EQ(dacResponse.certificate.data()[0], 0u);
    EXPECT_FALSE(dacResponse.totalDocumentSize.HasValue());
    EXPECT_FALSE(dacResponse.nextSegmentID.HasValue());

    Commands::CertificateChainRequest::Type paiRequest;
    paiRequest.certificateType = CertificateChainTypeEnum::kPAICertificate;

    auto paiResult = tester.Invoke(paiRequest);
    ASSERT_TRUE(paiResult.IsSuccess());
    ASSERT_TRUE(paiResult.response.has_value());
    if (!paiResult.response.has_value())
    {
        return;
    }
    const auto & paiResponse = paiResult.response.value();
    EXPECT_EQ(paiResponse.certificate.size(), 48u);
    EXPECT_EQ(paiResponse.certificate.data()[0], 0x80u);
    EXPECT_FALSE(paiResponse.totalDocumentSize.HasValue());
    EXPECT_FALSE(paiResponse.nextSegmentID.HasValue());
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestEcdsaProfilesSelectMixedPQCChain)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    Commands::CertificateChainRequest::Type dacRequest;
    dacRequest.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
    dacRequest.cryptoProfile.Emplace().PAACrypto = AttestationCryptoProfileEnum::kMlDsa44;

    auto dacResult = tester.Invoke(dacRequest);
    ASSERT_TRUE(dacResult.IsSuccess());
    ASSERT_TRUE(dacResult.response.has_value());
    if (!dacResult.response.has_value())
    {
        return;
    }
    const auto & dacResponse = dacResult.response.value();
    EXPECT_EQ(dacResponse.certificate.size(), 96u);
    EXPECT_EQ(dacResponse.certificate.data()[0], 0x20u);
    ASSERT_TRUE(dacResponse.totalDocumentSize.HasValue());
    EXPECT_EQ(dacResponse.totalDocumentSize.Value(), 96u);
    EXPECT_FALSE(dacResponse.nextSegmentID.HasValue());

    Commands::CertificateChainRequest::Type paiRequest;
    paiRequest.certificateType                   = CertificateChainTypeEnum::kPAICertificate;
    paiRequest.cryptoProfile.Emplace().PAACrypto = AttestationCryptoProfileEnum::kMlDsa44;

    auto paiResult = tester.Invoke(paiRequest);
    ASSERT_TRUE(paiResult.IsSuccess());
    ASSERT_TRUE(paiResult.response.has_value());
    if (!paiResult.response.has_value())
    {
        return;
    }
    const auto & paiResponse = paiResult.response.value();
    EXPECT_EQ(paiResponse.certificate.size(), 128u);
    EXPECT_EQ(paiResponse.certificate.data()[0], 0x40u);
    ASSERT_TRUE(paiResponse.totalDocumentSize.HasValue());
    EXPECT_EQ(paiResponse.totalDocumentSize.Value(), 128u);
    EXPECT_FALSE(paiResponse.nextSegmentID.HasValue());
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestPQCModeRejectsUnsupportedRequests)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    // The PAA supports ML-DSA-44, but neither the PAI nor DAC has an ML-DSA key.
    for (auto type : { CertificateChainTypeEnum::kDACCertificate, CertificateChainTypeEnum::kPAICertificate })
    {
        Commands::CertificateChainRequest::Type request;
        request.certificateType = type;
        auto & profiles         = request.cryptoProfile.Emplace();
        if (type == CertificateChainTypeEnum::kDACCertificate)
        {
            profiles.DACCrypto = AttestationCryptoProfileEnum::kMlDsa44;
        }
        else
        {
            profiles.PAICrypto = AttestationCryptoProfileEnum::kMlDsa44;
        }
        auto result = tester.Invoke(request);
        ASSERT_TRUE(result.status.has_value());
        if (!result.status.has_value())
        {
            return;
        }
        const auto & status = result.status.value();
        EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }

    // PAA support must be checked even though this command retrieves a DAC.
    Commands::CertificateChainRequest::Type unsupportedPaa;
    unsupportedPaa.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
    unsupportedPaa.cryptoProfile.Emplace().PAACrypto = AttestationCryptoProfileEnum::kMlDsa65;
    auto unsupportedPaaResult                        = tester.Invoke(unsupportedPaa);
    ASSERT_TRUE(unsupportedPaaResult.status.has_value());
    if (!unsupportedPaaResult.status.has_value())
    {
        return;
    }
    const auto & unsupportedPaaStatus = unsupportedPaaResult.status.value();
    EXPECT_EQ(unsupportedPaaStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type unsupportedProfile;
    unsupportedProfile.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
    unsupportedProfile.cryptoProfile.Emplace().DACCrypto = AttestationCryptoProfileEnum::kMlDsa65;

    auto unsupportedProfileResult = tester.Invoke(unsupportedProfile);
    ASSERT_TRUE(unsupportedProfileResult.status.has_value());
    if (!unsupportedProfileResult.status.has_value())
    {
        return;
    }
    const auto & unsupportedProfileStatus = unsupportedProfileResult.status.value();
    EXPECT_EQ(unsupportedProfileStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type outOfBoundsSegment;
    outOfBoundsSegment.certificateType = CertificateChainTypeEnum::kDACCertificate;
    outOfBoundsSegment.segmentID.SetValue(0);

    auto outOfBoundsResult = tester.Invoke(outOfBoundsSegment);
    ASSERT_TRUE(outOfBoundsResult.status.has_value());
    if (!outOfBoundsResult.status.has_value())
    {
        return;
    }
    const auto & outOfBoundsStatus = outOfBoundsResult.status.value();
    EXPECT_EQ(outOfBoundsStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    // Unknown generated enum values must be rejected before profile-to-bitmap conversion.
    Commands::CertificateChainRequest::Type unknownProfile;
    unknownProfile.certificateType = CertificateChainTypeEnum::kDACCertificate;
    // The encoder rejects the sentinel itself; decoding 0xff maps it to kUnknownEnumValue.
    unknownProfile.cryptoProfile.Emplace().DACCrypto = static_cast<AttestationCryptoProfileEnum>(0xff);
    auto unknownProfileResult                        = tester.Invoke(unknownProfile);
    ASSERT_TRUE(unknownProfileResult.status.has_value());
    if (!unknownProfileResult.status.has_value())
    {
        return;
    }
    const auto & unknownProfileStatus = unknownProfileResult.status.value();
    EXPECT_EQ(unknownProfileStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type unsupportedPqcProfile;
    unsupportedPqcProfile.certificateType                   = CertificateChainTypeEnum::kPAICertificate;
    unsupportedPqcProfile.cryptoProfile.Emplace().PAICrypto = AttestationCryptoProfileEnum::kMlDsa65;

    auto unsupportedPqcProfileResult = tester.Invoke(unsupportedPqcProfile);
    ASSERT_TRUE(unsupportedPqcProfileResult.status.has_value());
    if (!unsupportedPqcProfileResult.status.has_value())
    {
        return;
    }
    const auto & unsupportedPqcProfileStatus = unsupportedPqcProfileResult.status.value();
    EXPECT_EQ(unsupportedPqcProfileStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    // A SegmentID whose offset lands past the end of the document is a client error, not an
    // internal failure, so it has to surface as INVALID_COMMAND rather than the provider's
    // CHIP_ERROR_INVALID_ARGUMENT.
    Commands::CertificateChainRequest::Type outOfRangeSegment;
    outOfRangeSegment.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
    outOfRangeSegment.cryptoProfile.Emplace().DACCrypto = AttestationCryptoProfileEnum::kEcdsaMatterLegacy;
    outOfRangeSegment.segmentID.SetValue(3);

    auto outOfRangeSegmentResult = tester.Invoke(outOfRangeSegment);
    ASSERT_TRUE(outOfRangeSegmentResult.status.has_value());
    if (!outOfRangeSegmentResult.status.has_value())
    {
        return;
    }
    const auto & outOfRangeSegmentStatus = outOfRangeSegmentResult.status.value();
    EXPECT_EQ(outOfRangeSegmentStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    Commands::CertificateChainRequest::Type undersizedSegmentSize;
    undersizedSegmentSize.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
    undersizedSegmentSize.cryptoProfile.Emplace().DACCrypto = AttestationCryptoProfileEnum::kEcdsaMatterLegacy;
    undersizedSegmentSize.maxSegmentSize.SetValue(kDefaultCertificateSegmentSize - 1);

    auto undersizedSegmentSizeResult = tester.Invoke(undersizedSegmentSize);
    ASSERT_TRUE(undersizedSegmentSizeResult.status.has_value());
    if (!undersizedSegmentSizeResult.status.has_value())
    {
        return;
    }
    const auto & undersizedSegmentStatus = undersizedSegmentSizeResult.status.value();
    EXPECT_EQ(undersizedSegmentStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    // TC-OPCREDS-3.9 step 8 requires INVALID_COMMAND above RESP_MAX, even if
    // the actual document would fit in a normal 600-byte segment.
    for (uint16_t maxSegmentSize : { static_cast<uint16_t>(kMaxCertificateSegmentSize + 1), std::numeric_limits<uint16_t>::max() })
    {
        Commands::CertificateChainRequest::Type oversizedSegmentSize;
        oversizedSegmentSize.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
        oversizedSegmentSize.cryptoProfile.Emplace().DACCrypto = AttestationCryptoProfileEnum::kEcdsaMatterLegacy;
        oversizedSegmentSize.maxSegmentSize.SetValue(maxSegmentSize);

        auto result = tester.Invoke(oversizedSegmentSize);
        ASSERT_TRUE(result.status.has_value());
        if (!result.status.has_value())
        {
            return;
        }
        const auto & status = result.status.value();
        EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }
}

TEST_F(TestOperationalCredentials, TestCertificateChainRequestPQCModeAcceptsSegmentSizeRange)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation)));
    ClusterTester tester(cluster);

    // Both ends of the permitted MaxSegmentSize range are served, and the response is capped at the
    // default segment size regardless of how much the client is willing to receive.
    for (uint16_t maxSegmentSize : { kDefaultCertificateSegmentSize, kMaxCertificateSegmentSize })
    {
        Commands::CertificateChainRequest::Type request;
        request.certificateType                   = CertificateChainTypeEnum::kDACCertificate;
        request.cryptoProfile.Emplace().PAACrypto = AttestationCryptoProfileEnum::kMlDsa44;
        request.maxSegmentSize.SetValue(maxSegmentSize);

        auto result = tester.Invoke(request);
        ASSERT_TRUE(result.IsSuccess()) << "MaxSegmentSize " << maxSegmentSize << " was rejected";
        ASSERT_TRUE(result.response.has_value());
        if (!result.response.has_value())
        {
            return;
        }
        const auto & response = result.response.value();
        EXPECT_EQ(response.certificate.size(), 96u);
        ASSERT_TRUE(response.totalDocumentSize.HasValue());
        EXPECT_EQ(response.totalDocumentSize.Value(), 96u);
        EXPECT_FALSE(response.nextSegmentID.HasValue());
    }
}

class ChainDACProvider : public TestDACProvider
{
public:
    size_t calls     = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;

    Credentials::DeviceAttestationProfileSupport GetDeviceAttestationProfileSupport() const override
    {
        const BitMask<AttestationCryptoProfileBitmap> issuers(AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
                                                              AttestationCryptoProfileBitmap::kSupportsMlDsa44,
                                                              AttestationCryptoProfileBitmap::kSupportsMlDsa65);
        return { issuers, issuers,
                 BitMask<AttestationCryptoProfileBitmap>(AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy) };
    }

    CHIP_ERROR GetDeviceAttestationDocumentSegmentForChain(Credentials::DeviceAttestationDocumentType documentType,
                                                           const Credentials::DeviceAttestationChainProfile & chain, size_t offset,
                                                           MutableByteSpan & buffer, size_t & totalSize) override
    {
        ++calls;
        ReturnErrorOnFailure(error);
        VerifyOrReturnError(offset < 601, CHIP_ERROR_INVALID_ARGUMENT);
        totalSize = 601;
        buffer.reduce_size(std::min(buffer.size(), totalSize - offset));
        // Distinguish every tuple, document, and segment without allocating certificate fixtures.
        memset(buffer.data(),
               32 * to_underlying(chain.PAACrypto) + 8 * to_underlying(chain.PAICrypto) + 2 * to_underlying(documentType) +
                   static_cast<int>(offset / 600),
               buffer.size());
        return CHIP_NO_ERROR;
    }
};

TEST_F(TestOperationalCredentials, CertificateRequestsAreIndependentAndUseTheEntireTuple)
{
    ChainDACProvider provider;
    OperationalCredentialsCluster cluster(kRootEndpointId,
                                          MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation), &provider));
    ClusterTester tester(cluster);
    using Profile = AttestationCryptoProfileEnum;
    // Request later segments first and interleave chains and document types.
    for (uint16_t segment : std::array<uint16_t, 3>{ 1, 0, 1 })
    {
        for (auto paa : { Profile::kMlDsa65, Profile::kEcdsaMatterLegacy, Profile::kMlDsa44 })
        {
            for (auto pai : { Profile::kMlDsa65, Profile::kEcdsaMatterLegacy, Profile::kMlDsa44 })
            {
                for (auto type : { CertificateChainTypeEnum::kDACCertificate, CertificateChainTypeEnum::kPAICertificate })
                {
                    Commands::CertificateChainRequest::Type request;
                    request.certificateType = type;
                    auto & chain            = request.cryptoProfile.Emplace();
                    chain.PAACrypto         = paa;
                    chain.PAICrypto         = pai;
                    request.segmentID.SetValue(segment);
                    const auto calls = provider.calls;
                    auto result      = tester.Invoke(request);
                    if (pai > paa)
                    {
                        // An issuer cannot use a weaker profile than the certificate it signs.
                        ASSERT_TRUE(result.status.has_value());
                        if (!result.status.has_value())
                        {
                            return;
                        }
                        const auto & status = result.status.value();
                        EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
                        EXPECT_EQ(provider.calls, calls);
                        continue;
                    }
                    ASSERT_TRUE(result.IsSuccess());
                    ASSERT_TRUE(result.response.has_value());
                    if (!result.response.has_value())
                    {
                        return;
                    }
                    const auto & response = result.response.value();
                    EXPECT_EQ(provider.calls, calls + 1);
                    EXPECT_EQ(response.certificate.size(), segment == 0 ? 600u : 1u);
                    EXPECT_EQ(response.certificate[0],
                              32 * to_underlying(paa) + 8 * to_underlying(pai) + 2 * to_underlying(type) + segment);
                    ASSERT_TRUE(response.totalDocumentSize.HasValue());
                    EXPECT_EQ(response.totalDocumentSize.Value(), 601u);
                    EXPECT_EQ(response.nextSegmentID.HasValue(), segment == 0);
                    if (segment == 0)
                    {
                        EXPECT_EQ(response.nextSegmentID.Value(), 1u);
                    }
                }
            }
        }
    }
}

TEST_F(TestOperationalCredentials, CertificateRequestRejectsInvalidFieldsAndMissingChains)
{
    ChainDACProvider provider;
    OperationalCredentialsCluster cluster(kRootEndpointId,
                                          MakeContext(BitFlags<Feature>(Feature::kPQCDeviceAttestation), &provider));
    ClusterTester tester(cluster);
    using Profile = AttestationCryptoProfileEnum;
    for (unsigned field = 0; field < 3; ++field)
    {
        Commands::CertificateChainRequest::Type request;
        request.certificateType = CertificateChainTypeEnum::kDACCertificate;
        auto & chain            = request.cryptoProfile.Emplace();
        Profile * fields[]      = { &chain.PAACrypto, &chain.PAICrypto, &chain.DACCrypto };
        *fields[field]          = static_cast<Profile>(0xff);
        auto result             = tester.Invoke(request);
        ASSERT_TRUE(result.status.has_value());
        if (!result.status.has_value())
        {
            return;
        }
        const auto & status = result.status.value();
        EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
        EXPECT_EQ(provider.calls, 0u);
    }
    Commands::CertificateChainRequest::Type request;
    request.certificateType = CertificateChainTypeEnum::kPAICertificate;
    auto & chain            = request.cryptoProfile.Emplace();
    chain.PAACrypto         = Profile::kMlDsa65;
    chain.PAICrypto         = Profile::kMlDsa44;
    // SegmentID is constrained to at most 100, independent of provider document bounds.
    request.segmentID.SetValue(101);
    auto invalidSegment = tester.Invoke(request);
    ASSERT_TRUE(invalidSegment.status.has_value());
    if (!invalidSegment.status.has_value())
    {
        return;
    }
    const auto & invalidSegmentStatus = invalidSegment.status.value();
    EXPECT_EQ(invalidSegmentStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    EXPECT_EQ(provider.calls, 0u);
    request.segmentID.ClearValue();
    provider.error = CHIP_ERROR_NOT_FOUND;
    auto missing   = tester.Invoke(request);
    ASSERT_TRUE(missing.status.has_value());
    if (!missing.status.has_value())
    {
        return;
    }
    const auto & missingStatus = missing.status.value();
    EXPECT_EQ(missingStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    EXPECT_FALSE(missing.response.has_value());
    EXPECT_EQ(provider.calls, 1u);
    provider.error = CHIP_ERROR_INTERNAL;
    auto failed    = tester.Invoke(request);
    ASSERT_TRUE(failed.status.has_value());
    if (!failed.status.has_value())
    {
        return;
    }
    const auto & failedStatus = failed.status.value();
    EXPECT_EQ(failedStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Failure);
}

TEST_F(TestOperationalCredentials, TestSetCSRVendorReserved)
{
    OperationalCredentialsCluster cluster(kRootEndpointId, MakeContext());

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
