/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <controller/AutoCommissioner.h>
#include <controller/CommissioningDelegate.h>
#include <controller/tests/AutoCommissionerTestAccess.h>
#include <crypto/CHIPCryptoPAL.h>
#include <cstring>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <memory>

using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Controller;
using namespace chip::Testing;

namespace {

constexpr uint64_t epochJanFirst2000 = 946695600; // Monday, January 1, 2000 12:00 AM
constexpr uint64_t epochJanFirst2001 = 978318000; // Monday, January 1, 2001 12:00 AM

class AutoCommissionerTest : public ::testing::Test
{
protected:
    AutoCommissioner mCommissioner{};
    CommissioningParameters mParams{};
};

TEST_F(AutoCommissionerTest, DetectsThreadOperationalDatasetExceedsBuffer)
{
    auto up = std::make_unique<uint8_t[]>(CommissioningParameters::kMaxThreadDatasetLen + 1);

    ASSERT_TRUE(up);

    std::memset(up.get(), 0x00, CommissioningParameters::kMaxThreadDatasetLen + 1);

    mParams.SetThreadOperationalDataset(ByteSpan{ up.get(), CommissioningParameters::kMaxThreadDatasetLen + 1 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsWifiCredentialsExceedBuffer)
{
    auto ssid_buffer_up = std::make_unique<uint8_t[]>(CommissioningParameters::kMaxSsidLen + 1);

    auto creds_buffer_up = std::make_unique<uint8_t[]>(CommissioningParameters::kMaxCredentialsLen + 1);

    mParams.SetWiFiCredentials(WiFiCredentials{
        ByteSpan{ ssid_buffer_up.get(), CommissioningParameters::kMaxSsidLen + 1 },
        ByteSpan{ creds_buffer_up.get(), CommissioningParameters::kMaxCredentialsLen + 1 },
    });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsCountryCodeExceedsBuffer)
{
    mParams.SetCountryCode("012"_span);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsAttestationNonceExceedsBuffer)
{
    auto attestation_nonce_buffer_up = std::make_unique<uint8_t[]>(kAttestationNonceLength + 1);

    mParams.SetAttestationNonce(ByteSpan{ attestation_nonce_buffer_up.get(), kAttestationNonceLength + 1 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsCSRNonceExceedsBuffer)
{
    auto csr_nonce_buffer_up = std::make_unique<uint8_t[]>(kCSRNonceLength + 1);

    mParams.SetCSRNonce(ByteSpan{ csr_nonce_buffer_up.get(), kCSRNonceLength + 1 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, FeaturesPassedDSTOffsetsValue)
{
    app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type sDSTBuf;

    sDSTBuf.offset        = int32_t{ 10 };
    sDSTBuf.validStarting = epochJanFirst2000;
    sDSTBuf.validUntil    = epochJanFirst2001;
    app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type> list(&sDSTBuf, 1);

    mParams.SetDSTOffsets(list);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetDSTOffsets().HasValue());
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value().size(), size_t{ 1 });
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value()[0].offset, 10);
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value()[0].validStarting, epochJanFirst2000);
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value()[0].validUntil, epochJanFirst2001);
}

TEST_F(AutoCommissionerTest, FeaturesPassedTimeZoneValue)
{
    app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type sTimeZoneBuf;

    constexpr CharSpan countryName = "ARG"_span;

    sTimeZoneBuf.offset  = int32_t{ 10 };
    sTimeZoneBuf.validAt = epochJanFirst2000; // Monday, January 1, 2000 12:00 AM
    sTimeZoneBuf.name.SetValue(chip::CharSpan{ countryName });

    app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type> list(&sTimeZoneBuf, 1);
    mParams.SetTimeZone(list);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetTimeZone().HasValue());
    ASSERT_EQ(commissioning_params.GetTimeZone().Value().size(), size_t{ 1 });
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].offset, 10);
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].validAt, epochJanFirst2000);
    ASSERT_TRUE(commissioning_params.GetTimeZone().Value()[0].name.HasValue());
    ASSERT_TRUE(commissioning_params.GetTimeZone().Value()[0].name.Value().data_equal("ARG"_span));
}

TEST_F(AutoCommissionerTest, FeaturesPassedNTPValue)
{
    constexpr CharSpan defaultNTPBuffer = "default"_span;

    mParams.SetDefaultNTP(chip::app::DataModel::MakeNullable(defaultNTPBuffer));

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetDefaultNTP().HasValue());
    ASSERT_TRUE(commissioning_params.GetDefaultNTP().Value().Value().data_equal("default"_span));
}

TEST_F(AutoCommissionerTest, FeaturesPassedICDRegistrationKey)
{
    mParams.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);

    uint8_t symmetric_key_buffer[Crypto::kAES_CCM128_Key_Length];

    std::memset(symmetric_key_buffer, 0x00, Crypto::kAES_CCM128_Key_Length);

    mParams.SetICDSymmetricKey(ByteSpan{ symmetric_key_buffer, Crypto::kAES_CCM128_Key_Length });
    mParams.SetICDCheckInNodeId(NodeId{ 10000 });
    mParams.SetICDMonitoredSubject(uint64_t{ 9999 });
    mParams.SetICDClientType(app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetICDSymmetricKey().HasValue());
    ASSERT_EQ(commissioning_params.GetICDRegistrationStrategy(), ICDRegistrationStrategy::kBeforeComplete);
    ASSERT_TRUE(commissioning_params.GetICDSymmetricKey().HasValue());
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value().size(), Crypto::kAES_CCM128_Key_Length);
    ASSERT_TRUE(commissioning_params.GetICDSymmetricKey().Value().data_equal(ByteSpan{ symmetric_key_buffer }));
    ASSERT_TRUE(commissioning_params.GetICDCheckInNodeId().HasValue());
    ASSERT_EQ(commissioning_params.GetICDCheckInNodeId().Value(), NodeId{ 10000 });
    ASSERT_TRUE(commissioning_params.GetICDMonitoredSubject().HasValue());
    ASSERT_EQ(commissioning_params.GetICDMonitoredSubject().Value(), uint64_t{ 9999 });
    ASSERT_TRUE(commissioning_params.GetICDClientType().HasValue());
    ASSERT_EQ(commissioning_params.GetICDClientType().Value(), app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);
}

TEST_F(AutoCommissionerTest, FeaturesPassedExtraReadPaths)
{
    chip::app::AttributePathParams attributes[1];

    constexpr uint32_t endpointId  = 1;
    constexpr uint32_t clusterId   = 2;
    constexpr uint32_t attributeId = 3;

    attributes[0] = chip::app::AttributePathParams{ endpointId, clusterId, attributeId };

    mParams.SetExtraReadPaths(Span<const app::AttributePathParams>{ attributes, 1 });

    auto pathParams = mParams.GetExtraReadPaths();

    ASSERT_EQ(pathParams.size(), size_t{ 1 });
    ASSERT_EQ(pathParams[0].mEndpointId, endpointId);
    ASSERT_EQ(pathParams[0].mClusterId, clusterId);
    ASSERT_EQ(pathParams[0].mAttributeId, attributeId);
}

// kStages are enumerators from enum type name CommissioningStage
struct StageTransition
{
    CommissioningStage currentStage;
    CommissioningStage nextStage;
};

const std::vector<StageTransition> kStagePairs = {
    // Only linear transitions are tested here;
    // Branching cases (like kReadCommissioningInfo, kConfigureTCAcknowledgments, etc.) are tested separately
    { kSecurePairing, kReadCommissioningInfo },
    { kArmFailsafe, kConfigRegulatory },
    { kConfigRegulatory, kConfigureTCAcknowledgments },
    { kConfigureDefaultNTP, kSendPAICertificateRequest },
    { kSendPAICertificateRequest, kSendDACCertificateRequest },
    { kSendDACCertificateRequest, kSendAttestationRequest },
    { kSendAttestationRequest, kAttestationVerification },
    { kAttestationVerification, kAttestationRevocationCheck },
    { kJCMTrustVerification, kSendOpCertSigningRequest },
    { kSendOpCertSigningRequest, kValidateCSR },
    { kValidateCSR, kGenerateNOCChain },
    { kGenerateNOCChain, kSendTrustedRootCert },
    { kSendTrustedRootCert, kSendNOC },
    { kICDGetRegistrationInfo, kICDRegistration },
    { kScanNetworks, kNeedsNetworkCreds },
    { kWiFiNetworkSetup, kFailsafeBeforeWiFiEnable },
    { kThreadNetworkSetup, kFailsafeBeforeThreadEnable },
    { kFailsafeBeforeWiFiEnable, kWiFiNetworkEnable },
    { kFailsafeBeforeThreadEnable, kThreadNetworkEnable },
    { kEvictPreviousCaseSessions, kFindOperationalForStayActive },
    { kFindOperationalForStayActive, kICDSendStayActive },
    { kICDSendStayActive, kFindOperationalForCommissioningComplete },
    { kFindOperationalForCommissioningComplete, kSendComplete },
    { kSendComplete, kCleanup },
    { kCleanup, kError },
    { kError, kError },
    { static_cast<CommissioningStage>(250), kError }, // triggers default case in switch statement

};

// Test each case pair for the next commissioning stage
TEST_F(AutoCommissionerTest, NextCommissioningStage)
{
    // Accessor class used due to private/protected members.
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (const auto & stagePair : kStagePairs)
    {
        CommissioningStage nextStage =
            privateConfigCommissioner.AccessGetNextCommissioningStageInternal(stagePair.currentStage, err);
        EXPECT_EQ(nextStage, stagePair.nextStage);
    }
}

// if commissioning is manually stopped, the next stage should be kCleanup
TEST_F(AutoCommissionerTest, NextStageStopCommissioning)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    mCommissioner.StopCommissioning();

    CHIP_ERROR err           = CHIP_ERROR_INTERNAL;
    CommissioningStage stage = privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kSecurePairing, err);
    EXPECT_EQ(stage, kCleanup);
}

// if commissioning failed, then the next stage should be cleanup
TEST_F(AutoCommissionerTest, NextCommissioningStageAfterError)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    CHIP_ERROR err           = CHIP_ERROR_INTERNAL;
    CommissioningStage stage = privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kSecurePairing, err);
    EXPECT_EQ(stage, kCleanup);
}

// Verifies that the commissioner proceeds to ConfigureTCAcknowledgments under the correct conditions.
TEST_F(AutoCommissionerTest, NextStageReadCommissioningInfo)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err = CHIP_NO_ERROR;

    privateConfigCommissioner.SetBreadcrumb(0);
    CommissioningStage nextStage = privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kReadCommissioningInfo, err);

    EXPECT_EQ(nextStage, kArmFailsafe);

    // if breadcrumb > 0, the stage changes to kSendNOC; subsequent stages progress accordingly.
    privateConfigCommissioner.SetBreadcrumb(1);

    CommissioningStage nextStageReadCommissioningInfo =
        privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kReadCommissioningInfo, err);
    CommissioningStage nextStageSendNOC = privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kSendNOC, err);

    EXPECT_EQ(nextStageReadCommissioningInfo, nextStageSendNOC);
}

// Ensures TCAcknowledgment stage is triggered only under expected commissioning conditions.
TEST_F(AutoCommissionerTest, NextStageConfigureTCAcknowledgments)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err = CHIP_NO_ERROR;

    privateConfigCommissioner.SetUTCRequirements(true);

    CommissioningStage nextStage =
        privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kConfigureTCAcknowledgments, err);

    EXPECT_EQ(nextStage, kConfigureUTCTime);

    privateConfigCommissioner.SetUTCRequirements(false);

    nextStage = privateConfigCommissioner.AccessGetNextCommissioningStageInternal(kConfigureTCAcknowledgments, err);

    EXPECT_EQ(nextStage, kSendPAICertificateRequest);
}

namespace TestCerts {
/*
 * Test certificates and keys used for AutoCommissioner tests only.
 * These are “real” certificates (not arbitrary numbers) to ensure that
 * NOCChainGenerated calls ConvertX509CertToChipCert correctly.
 *
 * These certificates were generated using the CHIP cert tool.
 *
 * Example command:
 *   ./chip-cert gen-cert \
 *       --type -- \
 *       --subject-chip-id -- \
 *       --valid-from -- \
 *       --lifetime -- \
 *       --out-key -- \
 *       --out -- \
 *       --out-format --
 *
 *   # Converted PEM to DER
 *   openssl x509 -in out.pem -outform DER -out out.der
 *
 *   # Convert DER to C array for inclusion in test
 *   xxd -i out.der > out.cpp
 *
 * Arrays below (kTestRCAC, kTestICAC, kTestIcacNOC) represent the DER bytes
 * of the certificates. kTestIPK is a symmetric key used in tests.
 *
 * See more in Matter documentation: tools/matter/chip/certificate-tool
 */

constexpr uint8_t kTestRCAC[] = {
    0x30, 0x82, 0x01, 0x9d, 0x30, 0x82, 0x01, 0x43, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x03, 0x98, 0x46, 0xb6, 0xd0, 0x8f,
    0x11, 0x88, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x22, 0x31, 0x20, 0x30, 0x1e, 0x06,
    0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x04, 0x0c, 0x10, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x32,
    0x33, 0x34, 0x33, 0x5a, 0x17, 0x0d, 0x34, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x32, 0x33, 0x34, 0x32, 0x5a, 0x30, 0x22,
    0x31, 0x20, 0x30, 0x1e, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x04, 0x0c, 0x10, 0x43, 0x41, 0x43,
    0x41, 0x43, 0x41, 0x43, 0x41, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
    0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x1b, 0x71,
    0x65, 0x37, 0xe3, 0xa1, 0xe7, 0xdb, 0x11, 0x5f, 0xd0, 0xd3, 0x4b, 0xda, 0xf7, 0xd2, 0x85, 0x7c, 0xdc, 0xfd, 0x68, 0xcc, 0x6d,
    0xfa, 0x91, 0xab, 0xa8, 0x7c, 0xfd, 0x70, 0x5a, 0x4a, 0x23, 0x7b, 0x2c, 0x54, 0x43, 0xa0, 0x9a, 0x00, 0xb6, 0xdd, 0xbb, 0xc4,
    0xdc, 0xcd, 0x2b, 0x0c, 0x01, 0xfe, 0xa6, 0x01, 0x2e, 0xc6, 0x39, 0x21, 0x89, 0x28, 0xfa, 0x14, 0x65, 0x53, 0xd7, 0x85, 0xa3,
    0x63, 0x30, 0x61, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
    0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d,
    0x0e, 0x04, 0x16, 0x04, 0x14, 0xa4, 0xd4, 0xe6, 0x34, 0x79, 0x07, 0xa0, 0x6f, 0xab, 0xe7, 0x2a, 0xd0, 0x51, 0x26, 0xe0, 0x59,
    0x85, 0xb7, 0x08, 0x0d, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xa4, 0xd4, 0xe6, 0x34,
    0x79, 0x07, 0xa0, 0x6f, 0xab, 0xe7, 0x2a, 0xd0, 0x51, 0x26, 0xe0, 0x59, 0x85, 0xb7, 0x08, 0x0d, 0x30, 0x0a, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x21, 0x00, 0xff, 0x03, 0x62, 0x9f, 0xd7, 0x49,
    0xbf, 0x17, 0xf1, 0x45, 0x97, 0x33, 0x0d, 0x4b, 0xb0, 0x9a, 0xe9, 0xc8, 0x80, 0x25, 0xcc, 0x6c, 0x9b, 0x50, 0x1b, 0xbe, 0xd9,
    0xd0, 0x75, 0xb7, 0x45, 0x02, 0x02, 0x20, 0x57, 0xe5, 0xe2, 0x79, 0x40, 0xc7, 0x0b, 0xb9, 0x78, 0x9e, 0x9a, 0xdf, 0x51, 0xa7,
    0x44, 0x92, 0x6a, 0x2b, 0xf8, 0x67, 0x01, 0x86, 0x1e, 0xa3, 0xc3, 0xeb, 0xfc, 0x13, 0x7b, 0x2f, 0x75, 0x42
};

const uint8_t kTestICAC[] = {
    0x30, 0x82, 0x01, 0x9d, 0x30, 0x82, 0x01, 0x43, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x75, 0x57, 0x30, 0xd4, 0xc5, 0xf0,
    0x65, 0xb6, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x22, 0x31, 0x20, 0x30, 0x1e, 0x06,
    0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x04, 0x0c, 0x10, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x32,
    0x33, 0x34, 0x33, 0x5a, 0x17, 0x0d, 0x34, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x32, 0x33, 0x34, 0x32, 0x5a, 0x30, 0x22,
    0x31, 0x20, 0x30, 0x1e, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x03, 0x0c, 0x10, 0x43, 0x41, 0x43,
    0x41, 0x43, 0x41, 0x43, 0x41, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
    0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x76, 0xd9,
    0xb1, 0xd3, 0xed, 0x44, 0xba, 0x51, 0x86, 0x83, 0x1e, 0xf7, 0x41, 0xba, 0x70, 0x7f, 0xfe, 0x1c, 0x1b, 0xbb, 0x94, 0x53, 0x4b,
    0x4c, 0x65, 0x6e, 0x1c, 0x39, 0xdd, 0xe7, 0xa8, 0xf7, 0x3e, 0x9c, 0xc9, 0xb5, 0x82, 0x86, 0x14, 0xb0, 0xc0, 0xb3, 0x40, 0x2a,
    0xf9, 0x2d, 0x62, 0x03, 0x65, 0x4f, 0x15, 0xa9, 0x53, 0x0a, 0x79, 0xaa, 0xe5, 0xa9, 0x3c, 0x3a, 0xbf, 0xed, 0x6e, 0xd2, 0xa3,
    0x63, 0x30, 0x61, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
    0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d,
    0x0e, 0x04, 0x16, 0x04, 0x14, 0x74, 0xa6, 0x08, 0xd0, 0x78, 0xc2, 0x5c, 0x3c, 0x31, 0xf1, 0x78, 0x08, 0x3f, 0x9d, 0x37, 0x7e,
    0xf5, 0x73, 0x79, 0x61, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xa4, 0xd4, 0xe6, 0x34,
    0x79, 0x07, 0xa0, 0x6f, 0xab, 0xe7, 0x2a, 0xd0, 0x51, 0x26, 0xe0, 0x59, 0x85, 0xb7, 0x08, 0x0d, 0x30, 0x0a, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x21, 0x00, 0xe4, 0x44, 0xed, 0x33, 0x8a, 0x6c,
    0xf5, 0x90, 0xfa, 0xd1, 0xbe, 0x6e, 0x79, 0xfe, 0x2f, 0x5d, 0x4d, 0x09, 0x39, 0xae, 0x63, 0x0d, 0xe8, 0xd1, 0x41, 0x3b, 0x64,
    0x3c, 0x8a, 0x05, 0x76, 0xaa, 0x02, 0x20, 0x17, 0x34, 0xbd, 0x91, 0xef, 0xcd, 0x16, 0x60, 0x48, 0xac, 0xb0, 0x0a, 0xcf, 0x2b,
    0xea, 0x0c, 0x91, 0xe9, 0xeb, 0xc6, 0xcd, 0xe7, 0x4b, 0x1c, 0x24, 0x2c, 0xff, 0xd8, 0x3a, 0x86, 0x32, 0xbc
};

const uint8_t kTestIcacNOC[] = {
    0x30, 0x82, 0x01, 0xe1, 0x30, 0x82, 0x01, 0x86, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x2b, 0x15, 0x4b, 0x87, 0x6a, 0x57,
    0xf7, 0x50, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x22, 0x31, 0x20, 0x30, 0x1e, 0x06,
    0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x03, 0x0c, 0x10, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x32,
    0x33, 0x34, 0x33, 0x5a, 0x17, 0x0d, 0x34, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x32, 0x33, 0x34, 0x32, 0x5a, 0x30, 0x44,
    0x31, 0x20, 0x30, 0x1e, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x01, 0x0c, 0x10, 0x44, 0x45, 0x44,
    0x45, 0x44, 0x45, 0x44, 0x45, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x44, 0x31, 0x20, 0x30, 0x1e, 0x06, 0x0a, 0x2b, 0x06,
    0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x01, 0x05, 0x0c, 0x10, 0x46, 0x41, 0x42, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x31, 0x44, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xe9, 0x00, 0x43, 0x2f, 0x2d, 0x50, 0x42, 0xab, 0xdc, 0xac,
    0x21, 0xec, 0x59, 0xc6, 0x81, 0xfe, 0x2e, 0x3e, 0x68, 0x0b, 0xf9, 0x07, 0x02, 0x56, 0x34, 0x1c, 0x13, 0x6e, 0x8f, 0x89, 0x80,
    0xe4, 0xdd, 0xa1, 0x85, 0x74, 0x57, 0x7b, 0xeb, 0x34, 0x2e, 0x33, 0x2b, 0xed, 0xe1, 0x75, 0xa3, 0x48, 0x54, 0x2e, 0x91, 0x43,
    0xb2, 0xa5, 0x5e, 0x3e, 0x44, 0x15, 0xa9, 0x1c, 0x6b, 0x3c, 0x14, 0x31, 0xa3, 0x81, 0x83, 0x30, 0x81, 0x80, 0x30, 0x0c, 0x06,
    0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff,
    0x04, 0x04, 0x03, 0x02, 0x07, 0x80, 0x30, 0x20, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x01, 0x01, 0xff, 0x04, 0x16, 0x30, 0x14, 0x06,
    0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01, 0x30, 0x1d,
    0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x7c, 0x8d, 0xf2, 0x93, 0x8b, 0x96, 0xac, 0xc9, 0xab, 0x49, 0x92, 0xc5,
    0x66, 0xef, 0xf1, 0xfe, 0x09, 0x3e, 0x57, 0x4b, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14,
    0x74, 0xa6, 0x08, 0xd0, 0x78, 0xc2, 0x5c, 0x3c, 0x31, 0xf1, 0x78, 0x08, 0x3f, 0x9d, 0x37, 0x7e, 0xf5, 0x73, 0x79, 0x61, 0x30,
    0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x49, 0x00, 0x30, 0x46, 0x02, 0x21, 0x00, 0xb7, 0x35,
    0x48, 0x18, 0x05, 0xed, 0xe5, 0x3a, 0xea, 0x64, 0x9b, 0x30, 0x26, 0x8e, 0xc4, 0x8c, 0x0b, 0x55, 0x92, 0xaa, 0xc0, 0xc4, 0xac,
    0x0b, 0xa7, 0xf3, 0x85, 0x4a, 0x14, 0x92, 0x03, 0xa4, 0x02, 0x21, 0x00, 0xde, 0x84, 0xa6, 0x86, 0x3c, 0x2f, 0x2b, 0x74, 0xf6,
    0xc6, 0xf2, 0x75, 0x87, 0xe2, 0x64, 0x1f, 0x41, 0xd9, 0x2f, 0xf9, 0x1e, 0xa7, 0x15, 0x3a, 0x0e, 0xf2, 0xb6, 0x61, 0xec, 0x62,
    0xc2, 0x32
};

const uint8_t kTestIPK[chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                                                                 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00 };

} // namespace TestCerts

namespace {
const ByteSpan kValidRCAC(TestCerts::kTestRCAC);
const ByteSpan kValidNOC(TestCerts::kTestICAC);
const ByteSpan kValidICAC(TestCerts::kTestIcacNOC);
const Crypto::IdentityProtectionKeySpan kValidIpk(TestCerts::kTestIPK);
const NodeId kValidNodeId = 0x000;
// Invalid buffers
const ByteSpan kInvalidEmptyRCAC;
const ByteSpan kInvalidEmptyNOC;

uint8_t buffer[10];
ByteSpan kCoruptedBufferInvalidRCAC(buffer, static_cast<size_t>(std::numeric_limits<uint32_t>::max()) + 1);
ByteSpan kCoruptedBufferInvalidNOC(buffer, static_cast<size_t>(std::numeric_limits<uint32_t>::max()) + 1);
} // namespace

// Happy paths and valid arguments are thoroughly checked with the Python integration test in
// src/controller/python/OpCredsBinding.cpp

// Ensures empty RCAC certificates aren't admitted
TEST_F(AutoCommissionerTest, NOCChainGenerated_EmptyRCACReturnsInvalidArgument)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    CHIP_ERROR err = privateConfigCommissioner.NOCChainGenerated(kValidNOC, kValidICAC, kInvalidEmptyRCAC, kValidIpk, kValidNodeId);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

// On 32-bit systems, NOCChainGenerated cannot fail due to size_t being 32 bits and never exceeding uint32_t,
// therefore we skip some tests.
#if SIZE_MAX > UINT32_MAX

// Ensures extra checks are done with RCAC buffer size
TEST_F(AutoCommissionerTest, NOCChainGenerated_CorruptedRCACLengthReturnsError)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err =
        privateConfigCommissioner.NOCChainGenerated(kValidNOC, kValidICAC, kCoruptedBufferInvalidRCAC, kValidIpk, kValidNodeId);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

// Ensures extra checks are done with NOC buffer size
TEST_F(AutoCommissionerTest, NOCChainGenerated_CorruptedNOCLengthReturnsError)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err =
        privateConfigCommissioner.NOCChainGenerated(kCoruptedBufferInvalidNOC, kValidICAC, kValidRCAC, kValidIpk, kValidNodeId);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}
#endif

// Ensures empty NOC certificates aren't admitted
TEST_F(AutoCommissionerTest, NOCChainGenerated_EmptyNOCReturnsInvalidArgument)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err = privateConfigCommissioner.NOCChainGenerated(kInvalidEmptyNOC, kValidICAC, kValidRCAC, kValidIpk, kValidNodeId);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, TrySecondaryNetwork_confirm_true)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    privateConfigCommissioner.TrySecondaryNetwork();
    EXPECT_EQ(privateConfigCommissioner.TryingSecondaryNetwork(), true);
}

TEST_F(AutoCommissionerTest, ResetTryingSecondaryNetwork_confirm_false)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    privateConfigCommissioner.ResetNetworkAttemptType();
    EXPECT_EQ(privateConfigCommissioner.TryingSecondaryNetwork(), false);
}

TEST_F(AutoCommissionerTest, IsScanNeededCombinations)
{
    struct Case
    {
        const char * name;
        bool attemptWiFi;
        bool attemptThread;
        bool scanExpected;
    };

    const Case cases[] = {
        {
            .name          = "WiFiAndThreadSet",
            .attemptWiFi   = true,
            .attemptThread = true,
            .scanExpected  = true,
        },
        {
            .name          = "WiFiAndThreadNotSet",
            .attemptWiFi   = false,
            .attemptThread = false,
            .scanExpected  = false,
        },
        {
            .name          = "WiFiOnlySet",
            .attemptWiFi   = true,
            .attemptThread = false,
            .scanExpected  = true,
        },
        {
            .name          = "ThreadOnlySet",
            .attemptWiFi   = false,
            .attemptThread = true,
            .scanExpected  = true,
        },
    };

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    for (const auto & c : cases)
    {
        CommissioningParameters params{};
        params.SetAttemptWiFiNetworkScan(c.attemptWiFi);
        params.SetAttemptThreadNetworkScan(c.attemptThread);
        EXPECT_EQ(mCommissioner.SetCommissioningParameters(params), CHIP_NO_ERROR);

        ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
        commissioningInfo.network.wifi.endpoint   = c.attemptWiFi ? kRootEndpointId : kInvalidEndpointId;
        commissioningInfo.network.thread.endpoint = c.attemptThread ? kRootEndpointId : kInvalidEndpointId;

        bool result = privateConfigCommissioner.IsScanNeeded();

        if (result != c.scanExpected)
        {
            ChipLogError(Test, "%s failed: result=%d scanExpected=%d, attemptWiFi=%d, attemptThread=%d", c.name, result,
                         c.scanExpected, c.attemptWiFi, c.attemptThread);
        }
        EXPECT_EQ(result, c.scanExpected);
    }
}
TEST_F(AutoCommissionerTest, IsSecondaryNetworkSupportedCombinations)
{
    struct Case
    {
        const char * name;
        bool supportsConcurrent;
        bool hasWiFiCreds;
        bool hasThreadDataset;
        EndpointId wifiEndpoint;
        EndpointId threadEndpoint;
        bool isSecondaryNetworkSupported;
    };

    const Case cases[] = {
        {
            .name                        = "AllConditionsTrue",
            .supportsConcurrent          = true,
            .hasWiFiCreds                = true,
            .hasThreadDataset            = true,
            .wifiEndpoint                = kRootEndpointId,
            .threadEndpoint              = kRootEndpointId,
            .isSecondaryNetworkSupported = true,
        },
        {
            .name                        = "NoConcurrentConnection",
            .supportsConcurrent          = false,
            .hasWiFiCreds                = true,
            .hasThreadDataset            = true,
            .wifiEndpoint                = kRootEndpointId,
            .threadEndpoint              = kRootEndpointId,
            .isSecondaryNetworkSupported = false,
        },
        {
            .name                        = "NoWiFiCredentials",
            .supportsConcurrent          = true,
            .hasWiFiCreds                = false,
            .hasThreadDataset            = true,
            .wifiEndpoint                = kRootEndpointId,
            .threadEndpoint              = kRootEndpointId,
            .isSecondaryNetworkSupported = false,
        },
        {
            .name                        = "NoThreadDataset",
            .supportsConcurrent          = true,
            .hasWiFiCreds                = true,
            .hasThreadDataset            = false,
            .wifiEndpoint                = kRootEndpointId,
            .threadEndpoint              = kRootEndpointId,
            .isSecondaryNetworkSupported = false,
        },
        {
            .name                        = "InvalidWiFiEndpoint",
            .supportsConcurrent          = true,
            .hasWiFiCreds                = true,
            .hasThreadDataset            = true,
            .wifiEndpoint                = kInvalidEndpointId,
            .threadEndpoint              = kRootEndpointId,
            .isSecondaryNetworkSupported = false,
        },
        {
            .name                        = "InvalidThreadEndpoint",
            .supportsConcurrent          = true,
            .hasWiFiCreds                = true,
            .hasThreadDataset            = true,
            .wifiEndpoint                = kRootEndpointId,
            .threadEndpoint              = kInvalidEndpointId,
            .isSecondaryNetworkSupported = false,
        },
    };

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    for (const auto & c : cases)
    {
        CommissioningParameters params{};
        params.SetSupportsConcurrentConnection(c.supportsConcurrent);

        if (c.hasWiFiCreds)
        {
            params.SetWiFiCredentials(WiFiCredentials(ByteSpan(), ByteSpan()));
        }
        if (c.hasThreadDataset)
        {
            params.SetThreadOperationalDataset(ByteSpan());
        }

        EXPECT_EQ(mCommissioner.SetCommissioningParameters(params), CHIP_NO_ERROR);

        ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
        commissioningInfo.network.wifi.endpoint   = c.wifiEndpoint;
        commissioningInfo.network.thread.endpoint = c.threadEndpoint;

        bool result = privateConfigCommissioner.IsSecondaryNetworkSupported();
        if (result != c.isSecondaryNetworkSupported)
        {
            ChipLogError(Test,
                         "%s failed: result=%d expected=%d, supportsConcurrent=%d, hasWiFiCreds=%d, hasThreadDataset=%d, "
                         "wifiEndpoint=0x%03X, threadEndpoint=0x%03X",
                         c.name, result, c.isSecondaryNetworkSupported, c.supportsConcurrent, c.hasWiFiCreds, c.hasThreadDataset,
                         c.wifiEndpoint, c.threadEndpoint);
        }
        EXPECT_EQ(result, c.isSecondaryNetworkSupported);
    }
}
} // namespace
