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

TEST_F(TestParseOperationalCredentialsInfo, RetainsLargestPaaProfileAndClearsItOnLegacyFallback)
{
    using Profile  = OperationalCredentials::AttestationCryptoProfileEnum;
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
        if (bitmap == 9)
        {
            EXPECT_FALSE(access.PaaAttestationIssuerProfile().HasValue());
        }
        else
        {
            ASSERT_TRUE(access.PaaAttestationIssuerProfile().HasValue());
            EXPECT_EQ(access.PaaAttestationIssuerProfile().Value(),
                      bitmap >= 5       ? Profile::kMlDsa65
                          : bitmap == 3 ? Profile::kMlDsa44
                                        : Profile::kEcdsaMatterLegacy);
        }
        ASSERT_EQ(cachePtr->SetAttribute(featurePath, uint32_t(0)), CHIP_NO_ERROR);
        ASSERT_EQ(access.ParseOperationalCredentialsInfo(info), CHIP_NO_ERROR);
        EXPECT_FALSE(access.PaaAttestationIssuerProfile().HasValue());
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

} // namespace
