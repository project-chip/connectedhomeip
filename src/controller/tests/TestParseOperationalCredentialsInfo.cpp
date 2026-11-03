/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app/ClusterStateCache.h>
#include <app/ConcreteAttributePath.h>
#include <app/ReadClient.h>
#include <app/data-model/Encode.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <platform/CHIPDeviceLayer.h>

#include <clusters/OperationalCredentials/Attributes.h>
#include <clusters/OperationalCredentials/ClusterId.h>
#include <clusters/OperationalCredentials/Enums.h>
#include <clusters/OperationalCredentials/Structs.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Testing;

namespace {

class MockClusterStateCache : public ClusterStateCache
{
public:
    MockClusterStateCache() : ClusterStateCache(mCallback) {}

    template <typename AttrType>
    CHIP_ERROR SetAttribute(const ConcreteAttributePath & path, const AttrType & data)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> handle;
        handle.Calloc(3000);
        TLV::ScopedBufferTLVWriter writer(std::move(handle), 3000);
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), data));
        uint32_t writtenLength = writer.GetLengthWritten();
        ReturnErrorOnFailure(writer.Finalize(handle));

        TLV::ScopedBufferTLVReader reader;
        StatusIB status;
        reader.Init(std::move(handle), writtenLength);
        ReturnErrorOnFailure(reader.Next());
        ReadClient::Callback & cb = GetBufferedCallback();
        cb.OnAttributeData(path, &reader, status);

        return CHIP_NO_ERROR;
    }

private:
    class MockCallback : public ClusterStateCache::Callback
    {
        void OnDone(ReadClient *) override {}
        void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override {}
    };

    MockCallback mCallback;
};

class TestParseOperationalCredentialsInfo : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    DeviceCommissioner mCommissioner{};
};

TEST_F(TestParseOperationalCredentialsInfo, LegacyFeatureMapDefaultsToLegacyProfile)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    BitFlags<OperationalCredentials::Feature> featureMap;
    ConcreteAttributePath featureMapPath(kRootEndpointId, OperationalCredentials::Id,
                                         OperationalCredentials::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
    EXPECT_FALSE(info.supportsPqcDeviceAttestation);
    EXPECT_TRUE(info.paiSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
    EXPECT_TRUE(info.dacSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
}

TEST_F(TestParseOperationalCredentialsInfo, FeatureMapStatusErrorFallsBackToLegacy)
{
    using OperationalCredentials::AttestationCryptoProfileBitmap;

    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    // An unavailable FeatureMap must select pre-PQC attestation instead of failing commissioning.
    ConcreteDataAttributePath featureMapPath(kRootEndpointId, OperationalCredentials::Id,
                                             OperationalCredentials::Attributes::FeatureMap::Id);
    ReadClient::Callback & callback = cache->GetBufferedCallback();
    callback.OnAttributeData(featureMapPath, nullptr, StatusIB(Protocols::InteractionModel::Status::UnsupportedAttribute));

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    info.supportsPqcDeviceAttestation = true;
    info.paiSupportedAttestationProfiles.Set(AttestationCryptoProfileBitmap::kSupportsMlDsa65);
    info.dacSupportedAttestationProfiles.Set(AttestationCryptoProfileBitmap::kSupportsMlDsa44);

    ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
    EXPECT_FALSE(info.supportsPqcDeviceAttestation);
    EXPECT_EQ(info.paiSupportedAttestationProfiles.Raw(),
              BitMask<AttestationCryptoProfileBitmap>(AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy).Raw());
    EXPECT_EQ(info.dacSupportedAttestationProfiles.Raw(),
              BitMask<AttestationCryptoProfileBitmap>(AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy).Raw());
}

TEST_F(TestParseOperationalCredentialsInfo, PqcFeaturePreservesIndependentPaiAndDacProfiles)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    BitFlags<OperationalCredentials::Feature> featureMap(OperationalCredentials::Feature::kPQCDeviceAttestation);
    ConcreteAttributePath featureMapPath(kRootEndpointId, OperationalCredentials::Id,
                                         OperationalCredentials::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    OperationalCredentials::Structs::PQCDeviceAttestationProfileStruct::Type profileSupport;
    profileSupport.PAASupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44);
    profileSupport.PAISupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa65);
    profileSupport.DACSupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44);

    ConcreteAttributePath profilePath(kRootEndpointId, OperationalCredentials::Id,
                                      OperationalCredentials::Attributes::PQCDeviceAttestationProfile::Id);
    ASSERT_EQ(cache->SetAttribute(profilePath, profileSupport), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
    EXPECT_TRUE(info.supportsPqcDeviceAttestation);
    EXPECT_TRUE(info.paiSupportedAttestationProfiles.Has(OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa65));
    EXPECT_FALSE(
        info.paiSupportedAttestationProfiles.Has(OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44));
    EXPECT_TRUE(info.dacSupportedAttestationProfiles.Has(OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44));
    EXPECT_FALSE(
        info.dacSupportedAttestationProfiles.Has(OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa65));
}

TEST_F(TestParseOperationalCredentialsInfo, RetainsPaaBitmapAndResetsItOnLegacyFallback)
{
    using Profiles = OperationalCredentials::AttestationCryptoProfileBitmap;
    for (const uint16_t bitmap : { uint16_t(1), uint16_t(3), uint16_t(5), uint16_t(7), uint16_t(9) })
    {
        auto cache      = Platform::MakeUnique<MockClusterStateCache>();
        auto * cachePtr = cache.get();
        const ConcreteAttributePath featurePath(kRootEndpointId, OperationalCredentials::Id,
                                                OperationalCredentials::Attributes::FeatureMap::Id);
        ASSERT_EQ(cache->SetAttribute(featurePath, uint32_t(1)), CHIP_NO_ERROR);
        OperationalCredentials::Structs::PQCDeviceAttestationProfileStruct::Type profiles;
        profiles.PAASupportedProfiles = BitMask<Profiles>(bitmap);
        profiles.PAISupportedProfiles = BitMask<Profiles>(Profiles::kSupportsEcdsaMatterLegacy, Profiles::kSupportsMlDsa44);
        profiles.DACSupportedProfiles = BitMask<Profiles>(Profiles::kSupportsEcdsaMatterLegacy);
        ASSERT_EQ(cache->SetAttribute(ConcreteAttributePath(kRootEndpointId, OperationalCredentials::Id,
                                                            OperationalCredentials::Attributes::PQCDeviceAttestationProfile::Id),
                                      profiles),
                  CHIP_NO_ERROR);
        DeviceCommissionerTestAccess access(&mCommissioner);
        access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));
        ReadCommissioningInfo info;
        ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
        EXPECT_EQ(info.paaSupportedAttestationProfiles.Raw(), bitmap);
        ASSERT_EQ(cachePtr->SetAttribute(featurePath, uint32_t(0)), CHIP_NO_ERROR);
        ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
        EXPECT_EQ(info.paaSupportedAttestationProfiles.Raw(), 1u);
    }
}

TEST_F(TestParseOperationalCredentialsInfo, PqcFeatureFallsBackToLegacyWhenDacDoesNotAdvertiseLegacy)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    BitFlags<OperationalCredentials::Feature> featureMap(OperationalCredentials::Feature::kPQCDeviceAttestation);
    ConcreteAttributePath featureMapPath(kRootEndpointId, OperationalCredentials::Id,
                                         OperationalCredentials::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    OperationalCredentials::Structs::PQCDeviceAttestationProfileStruct::Type profileSupport;
    profileSupport.PAASupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44);
    profileSupport.PAISupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy,
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44);
    profileSupport.DACSupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsMlDsa44);

    ConcreteAttributePath profilePath(kRootEndpointId, OperationalCredentials::Id,
                                      OperationalCredentials::Attributes::PQCDeviceAttestationProfile::Id);
    ASSERT_EQ(cache->SetAttribute(profilePath, profileSupport), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
    EXPECT_FALSE(info.supportsPqcDeviceAttestation);
    EXPECT_TRUE(info.paiSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
    EXPECT_TRUE(info.dacSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
}

TEST_F(TestParseOperationalCredentialsInfo, PqcFeatureFallsBackToLegacyWhenNoPqcIssuerIsAdvertised)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    BitFlags<OperationalCredentials::Feature> featureMap(OperationalCredentials::Feature::kPQCDeviceAttestation);
    ConcreteAttributePath featureMapPath(kRootEndpointId, OperationalCredentials::Id,
                                         OperationalCredentials::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    OperationalCredentials::Structs::PQCDeviceAttestationProfileStruct::Type profileSupport;
    profileSupport.PAASupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy);
    profileSupport.PAISupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy);
    profileSupport.DACSupportedProfiles = BitMask<OperationalCredentials::AttestationCryptoProfileBitmap>(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy);

    ConcreteAttributePath profilePath(kRootEndpointId, OperationalCredentials::Id,
                                      OperationalCredentials::Attributes::PQCDeviceAttestationProfile::Id);
    ASSERT_EQ(cache->SetAttribute(profilePath, profileSupport), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
    EXPECT_FALSE(info.supportsPqcDeviceAttestation);
    EXPECT_TRUE(info.paiSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
    EXPECT_TRUE(info.dacSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
}

TEST_F(TestParseOperationalCredentialsInfo, PqcFeatureFallsBackToLegacyWhenProfileAttributeIsMissing)
{
    auto cache = Platform::MakeUnique<MockClusterStateCache>();
    ASSERT_NE(cache, nullptr);

    BitFlags<OperationalCredentials::Feature> featureMap(OperationalCredentials::Feature::kPQCDeviceAttestation);
    ConcreteAttributePath featureMapPath(kRootEndpointId, OperationalCredentials::Id,
                                         OperationalCredentials::Attributes::FeatureMap::Id);
    ASSERT_EQ(cache->SetAttribute(featureMapPath, *featureMap.RawStorage()), CHIP_NO_ERROR);

    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetAttributeCache(Platform::UniquePtr<ClusterStateCache>(cache.release()));

    ReadCommissioningInfo info{};
    ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
    EXPECT_FALSE(info.supportsPqcDeviceAttestation);
    EXPECT_TRUE(info.paiSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
    EXPECT_TRUE(info.dacSupportedAttestationProfiles.Has(
        OperationalCredentials::AttestationCryptoProfileBitmap::kSupportsEcdsaMatterLegacy));
}

TEST_F(TestParseOperationalCredentialsInfo, CertificateRequestsSerializeAndRetainTheCompleteChain)
{
    using Profile = OperationalCredentials::AttestationCryptoProfileEnum;
    const Credentials::DeviceAttestationChainProfile chain{ Profile::kMlDsa65, Profile::kMlDsa44, Profile::kEcdsaMatterLegacy };
    DeviceCommissionerTestAccess access(&mCommissioner);
    for (auto type : { Credentials::CertificateType::kPAI, Credentials::CertificateType::kDAC })
    {
        for (uint16_t segment = 0; segment < 3; ++segment)
        {
            OperationalCredentials::Commands::CertificateChainRequest::Type request;
            ASSERT_EQ(
                access.PrepareCertificateChainRequest(type, chain, segment == 0 ? NullOptional : MakeOptional(segment), request),
                CHIP_NO_ERROR);
            uint8_t buffer[128];
            TLV::TLVWriter writer;
            writer.Init(buffer);
            ASSERT_EQ(request.Encode(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);
            ASSERT_EQ(writer.Finalize(), CHIP_NO_ERROR);
            TLV::TLVReader reader;
            reader.Init(buffer, writer.GetLengthWritten());
            ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
            OperationalCredentials::Commands::CertificateChainRequest::DecodableType decoded;
            ASSERT_EQ(decoded.Decode(reader), CHIP_NO_ERROR);
            ASSERT_TRUE(decoded.cryptoProfile.HasValue());
            EXPECT_EQ(decoded.cryptoProfile.Value().PAACrypto, chain.PAACrypto);
            EXPECT_EQ(decoded.cryptoProfile.Value().PAICrypto, chain.PAICrypto);
            EXPECT_EQ(decoded.cryptoProfile.Value().DACCrypto, chain.DACCrypto);
            EXPECT_EQ(decoded.segmentID.HasValue(), segment != 0);
            EXPECT_EQ(decoded.segmentID.ValueOr(0), segment);
            ASSERT_TRUE(decoded.maxSegmentSize.HasValue());
            EXPECT_EQ(decoded.maxSegmentSize.Value(), CertificateChainRequestTracker::kDefaultSegmentSize);
        }
    }
    OperationalCredentials::Commands::CertificateChainRequest::Type request;
    auto differentChain      = chain;
    differentChain.PAACrypto = Profile::kMlDsa44;
    EXPECT_EQ(access.PrepareCertificateChainRequest(Credentials::CertificateType::kDAC, differentChain, MakeOptional(uint16_t{ 3 }),
                                                    request),
              CHIP_ERROR_INCORRECT_STATE);
    access.CancelCommissioningInteractions();
    EXPECT_EQ(
        access.PrepareCertificateChainRequest(Credentials::CertificateType::kDAC, chain, MakeOptional(uint16_t{ 3 }), request),
        CHIP_ERROR_INCORRECT_STATE);
    ASSERT_EQ(access.PrepareCertificateChainRequest(Credentials::CertificateType::kDAC, std::nullopt, NullOptional, request),
              CHIP_NO_ERROR);
    EXPECT_FALSE(request.cryptoProfile.HasValue());
    EXPECT_FALSE(request.segmentID.HasValue());
    EXPECT_FALSE(request.maxSegmentSize.HasValue());
    differentChain.PAICrypto = Profile::kMlDsa65;
    EXPECT_EQ(access.PrepareCertificateChainRequest(Credentials::CertificateType::kPAI, differentChain, NullOptional, request),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestParseOperationalCredentialsInfo, LegacyCertificateRequestsEncodeOnlyCertificateType)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    for (auto type : { Credentials::CertificateType::kPAI, Credentials::CertificateType::kDAC })
    {
        OperationalCredentials::Commands::CertificateChainRequest::Type request;
        ASSERT_EQ(access.PrepareCertificateChainRequest(type, std::nullopt, NullOptional, request), CHIP_NO_ERROR);
        uint8_t buffer[128];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        ASSERT_EQ(request.Encode(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);
        ASSERT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        TLV::TLVType container;
        ASSERT_EQ(reader.EnterContainer(container), CHIP_NO_ERROR);
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetTag(), TLV::ContextTag(0));
        uint8_t certificateType;
        ASSERT_EQ(reader.Get(certificateType), CHIP_NO_ERROR);
        EXPECT_EQ(certificateType, to_underlying(type));
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(container), CHIP_NO_ERROR);
    }
}

TEST_F(TestParseOperationalCredentialsInfo, CertificateRequestBoundsUseSelectedSubjectAndIssuer)
{
    using Profile = OperationalCredentials::AttestationCryptoProfileEnum;
    const Credentials::DeviceAttestationChainProfile chain{ Profile::kMlDsa65, Profile::kMlDsa44, Profile::kEcdsaMatterLegacy };
    DeviceCommissionerTestAccess access(&mCommissioner);
    OperationalCredentials::Commands::CertificateChainRequest::Type request;
    uint8_t bytes[600] = {};
    ASSERT_EQ(access.PrepareCertificateChainRequest(Credentials::CertificateType::kPAI, chain, NullOptional, request),
              CHIP_NO_ERROR);
    EXPECT_EQ(access.CertificateRequestTracker().HandleResponse(
                  ByteSpan(bytes), MakeOptional(static_cast<uint16_t>(Credentials::kMaxDERCertLengthMlDsa65)),
                  MakeOptional(uint16_t{ 1 })),
              CHIP_NO_ERROR);
    ASSERT_EQ(access.PrepareCertificateChainRequest(Credentials::CertificateType::kDAC, chain, NullOptional, request),
              CHIP_NO_ERROR);
    // The selected DAC has an ML-DSA-44 issuer; a document requiring the ML-DSA-65 bound is invalid.
    EXPECT_NE(access.CertificateRequestTracker().HandleResponse(
                  ByteSpan(bytes), MakeOptional(static_cast<uint16_t>(Credentials::kMaxDERCertLengthMlDsa65)),
                  MakeOptional(uint16_t{ 1 })),
              CHIP_NO_ERROR);
}

TEST_F(TestParseOperationalCredentialsInfo, CertificateResponseRejectsProfilesDifferentFromTheRequest)
{
    class CertificateDevice : public DeviceProxy
    {
    public:
        void Disconnect() override {}
        NodeId GetDeviceId() const override { return 0x1234; }
        Messaging::ExchangeManager * GetExchangeManager() const override { return nullptr; }
        Optional<SessionHandle> GetSecureSession() const override { return NullOptional; }

    private:
        bool IsSecureConnected() const override { return false; }
    } device;
    class CertificateDelegate : public AutoCommissioner
    {
    public:
        CHIP_ERROR CommissioningStepFinished(CHIP_ERROR error, CommissioningReport report) override
        {
            completions++;
            result              = error;
            receivedCertificate = report.Is<RequestedCertificate>();
            return CHIP_NO_ERROR;
        }
        unsigned completions     = 0;
        CHIP_ERROR result        = CHIP_ERROR_INTERNAL;
        bool receivedCertificate = false;
    } delegate;
    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetCommissioningDelegate(&delegate);
    using Profile = OperationalCredentials::AttestationCryptoProfileEnum;
    for (auto type : { Credentials::CertificateType::kPAI, Credentials::CertificateType::kDAC })
    {
        for (bool mismatch : { false, true })
        {
            Credentials::DeviceAttestationChainProfile chain{};
            if (mismatch)
            {
                chain.PAACrypto = Profile::kMlDsa65;
                chain.PAICrypto = type == Credentials::CertificateType::kPAI ? Profile::kEcdsaMatterLegacy : Profile::kMlDsa44;
            }
            OperationalCredentials::Commands::CertificateChainRequest::Type request;
            ASSERT_EQ(access.PrepareCertificateChainRequest(type, chain, NullOptional, request), CHIP_NO_ERROR);
            access.SetDeviceBeingCommissioned(&device);
            access.SetCommissioningStage(type == Credentials::CertificateType::kPAI ? kSendPAICertificateRequest
                                                                                    : kSendDACCertificateRequest);
            OperationalCredentials::Commands::CertificateChainResponse::DecodableType response;
            response.certificate  = type == Credentials::CertificateType::kPAI ? TestCerts::sTestCert_PAI_FFF1_8000_Cert
                                                                               : TestCerts::sTestCert_DAC_FFF1_8000_0000_Cert;
            const unsigned before = delegate.completions;
            access.OnCertificateChainResponse(response);
            EXPECT_EQ(delegate.completions, before + 1);
            EXPECT_EQ(delegate.result, mismatch ? CHIP_ERROR_FAILED_DEVICE_ATTESTATION : CHIP_NO_ERROR);
            EXPECT_EQ(delegate.receivedCertificate, !mismatch);
            EXPECT_EQ(access.PrepareCertificateChainRequest(type, chain, MakeOptional(uint16_t{ 1 }), request),
                      CHIP_ERROR_INCORRECT_STATE);
        }
    }
    access.SetCommissioningDelegate(nullptr);
}

TEST_F(TestParseOperationalCredentialsInfo, OnlyInitialProfiledPaiRejectionAllowsAnotherChain)
{
    class CertificateDevice : public DeviceProxy
    {
    public:
        void Disconnect() override {}
        NodeId GetDeviceId() const override { return 0x1234; }
        Messaging::ExchangeManager * GetExchangeManager() const override { return nullptr; }
        Optional<SessionHandle> GetSecureSession() const override { return NullOptional; }

    private:
        bool IsSecureConnected() const override { return false; }
    } device;
    class CertificateDelegate : public AutoCommissioner
    {
    public:
        CHIP_ERROR CommissioningStepFinished(CHIP_ERROR error, CommissioningReport report) override
        {
            result      = error;
            unavailable = report.Is<AttestationChainNotAvailable>();
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR result = CHIP_ERROR_INTERNAL;
        bool unavailable  = false;
    } delegate;
    DeviceCommissionerTestAccess access(&mCommissioner);
    access.SetCommissioningDelegate(&delegate);
    for (auto type : { Credentials::CertificateType::kPAI, Credentials::CertificateType::kDAC })
    {
        for (bool explicitChain : { false, true })
        {
            for (bool continuation : { false, true })
            {
                for (auto error : { CHIP_IM_GLOBAL_STATUS(InvalidCommand), CHIP_IM_GLOBAL_STATUS(Failure), CHIP_ERROR_TIMEOUT })
                {
                    std::optional<Credentials::DeviceAttestationChainProfile> chain;
                    if (explicitChain)
                    {
                        chain.emplace();
                    }
                    OperationalCredentials::Commands::CertificateChainRequest::Type request;
                    ASSERT_EQ(access.PrepareCertificateChainRequest(type, chain, NullOptional, request), CHIP_NO_ERROR);
                    if (continuation)
                    {
                        const uint8_t segment[] = { 1 };
                        ASSERT_EQ(access.CertificateRequestTracker().HandleResponse(ByteSpan(segment), MakeOptional(uint16_t{ 2 }),
                                                                                    MakeOptional(uint16_t{ 1 })),
                                  CHIP_NO_ERROR);
                    }
                    access.SetDeviceBeingCommissioned(&device);
                    access.SetCommissioningStage(type == Credentials::CertificateType::kPAI ? kSendPAICertificateRequest
                                                                                            : kSendDACCertificateRequest);
                    access.OnCertificateChainFailureResponse(error);
                    EXPECT_EQ(delegate.result, error);
                    EXPECT_EQ(delegate.unavailable,
                              type == Credentials::CertificateType::kPAI && explicitChain && !continuation &&
                                  error == CHIP_IM_GLOBAL_STATUS(InvalidCommand));
                    EXPECT_EQ(access.PrepareCertificateChainRequest(type, chain, MakeOptional(uint16_t{ 1 }), request),
                              CHIP_ERROR_INCORRECT_STATE);
                }
            }
        }
    }
    access.SetCommissioningDelegate(nullptr);
}

} // namespace
