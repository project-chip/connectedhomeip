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

#include "Test_cert_data.h"
#include <controller/AutoCommissioner.h>
#include <controller/CommissioningDelegate.h>
#include <controller/tests/AutoCommissionerTestAccess.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>

#include <cstring>
#include <memory>


using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Controller;
using namespace chip::Test;

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

// Happy paths and valid arguments are thoroughly checked with the Python integration test in
// src/controller/python/OpCredsBinding.cpp Ensures empty RCAC certificates arent admitted
TEST_F(AutoCommissionerTest, NOCChainGenerated_EmptyRCACReturnsInvalidArgument)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    CHIP_ERROR err =
        privateConfigCommissioner.CallNOCChainGenerated(ByteSpan(), ByteSpan(), ByteSpan(), Crypto::IdentityProtectionKeySpan(), 0);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

// Ensures extra checks are done with RCAC buffer size
TEST_F(AutoCommissionerTest, NOCChainGenerated_CorruptedRCACLengthReturnsError)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    uint8_t buffer[10];
    ByteSpan corrupted_size_rcac(buffer, std::numeric_limits<size_t>::max());

    CHIP_ERROR err = privateConfigCommissioner.CallNOCChainGenerated(ByteSpan(), ByteSpan(), corrupted_size_rcac,
                                                                     Crypto::IdentityProtectionKeySpan(), 0);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

// Ensures extra checks are done with NOC buffer size
TEST_F(AutoCommissionerTest, NOCChainGenerated_CorruptedNOCLengthReturnsError)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    uint8_t buffer[10];
    ByteSpan corrupted_size_noc(buffer, std::numeric_limits<size_t>::max());

    CHIP_ERROR err = privateConfigCommissioner.CallNOCChainGenerated(
        corrupted_size_noc, ByteSpan(), ByteSpan(chip::TestCerts::kTestRCAC, chip::TestCerts::kTestRCAC_size),
        Crypto::IdentityProtectionKeySpan(), 0);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

// Ensures empty NOC certificates arent admitted
TEST_F(AutoCommissionerTest, NOCChainGenerated_EmptyNOCReturnsInvalidArgument)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    CHIP_ERROR err = privateConfigCommissioner.CallNOCChainGenerated(
        ByteSpan(), ByteSpan(), ByteSpan(chip::TestCerts::kTestRCAC, chip::TestCerts::kTestRCAC_size),
        Crypto::IdentityProtectionKeySpan(), 0);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
