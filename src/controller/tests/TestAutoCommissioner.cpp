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
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <cstring>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <memory>
#include <system/SystemClock.h>

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

namespace {

/**
 * @brief RAII helper to ensure DetachSecureSession is always called.
 *
 * This class guarantees that the device's secure session is detached when the
 * object goes out of scope, preventing crashes or memory corruption if a
 * test fails before the end of the function.
 */
class ScopedSessionDetacher
{
public:
    explicit ScopedSessionDetacher(chip::CommissioneeDeviceProxy & device) : mDevice(device) {}
    ~ScopedSessionDetacher() { mDevice.DetachSecureSession(); }

private:
    chip::CommissioneeDeviceProxy & mDevice;
};
} // namespace

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

// TODO : Remove redundancies,  optimize code and debug
TEST_F(AutoCommissionerTest, GetNextCommissioningStageNetworkSetup_ReturnsCorrectStageAndError)
{
    struct Case
    {
        const char * name;
        bool supportsConcurrent;
        bool trySecondary;
        bool hasWiFiCreds;
        bool hasThreadDataset;
        EndpointId wifiEndpoint;
        EndpointId threadEndpoint;
        CommissioningStage expectedStage;
        CHIP_ERROR expectedError;
    };

    Case cases[] = {
        { .name               = "TrySecondaryWifiAtRootExpectThread",
          .supportsConcurrent = true,
          .trySecondary       = true,
          .hasWiFiCreds       = true,
          .hasThreadDataset   = true,
          .wifiEndpoint       = kRootEndpointId,
          .threadEndpoint     = kRootEndpointId,
          .expectedStage      = CommissioningStage::kThreadNetworkSetup,
          .expectedError      = CHIP_NO_ERROR },
        { .name               = "TrySecondaryWifiNotRootExpectWiFi",
          .supportsConcurrent = true,
          .trySecondary       = true,
          .hasWiFiCreds       = true,
          .hasThreadDataset   = true,
          .wifiEndpoint       = kRootEndpointId + 1,
          .threadEndpoint     = kRootEndpointId,
          .expectedStage      = CommissioningStage::kWiFiNetworkSetup,
          .expectedError      = CHIP_NO_ERROR },
        { .name               = "PrimaryWifiAtRootExpectWiFi",
          .supportsConcurrent = true,
          .trySecondary       = false,
          .hasWiFiCreds       = true,
          .hasThreadDataset   = true,
          .wifiEndpoint       = kRootEndpointId,
          .threadEndpoint     = kRootEndpointId,
          .expectedStage      = CommissioningStage::kWiFiNetworkSetup,
          .expectedError      = CHIP_NO_ERROR },
        { .name               = "PrimaryWifiNotRootExpectThread",
          .supportsConcurrent = true,
          .trySecondary       = false,
          .hasWiFiCreds       = true,
          .hasThreadDataset   = true,
          .wifiEndpoint       = kRootEndpointId + 1,
          .threadEndpoint     = kRootEndpointId,
          .expectedStage      = CommissioningStage::kThreadNetworkSetup,
          .expectedError      = CHIP_NO_ERROR },
        { .name               = "NoSecondarySupportWithWiFiOnlyExpectWiFi",
          .supportsConcurrent = false,
          .trySecondary       = false,
          .hasWiFiCreds       = true,
          .hasThreadDataset   = false,
          .wifiEndpoint       = kRootEndpointId,
          .threadEndpoint     = kInvalidEndpointId,
          .expectedStage      = CommissioningStage::kWiFiNetworkSetup,
          .expectedError      = CHIP_NO_ERROR },
        { .name               = "NoSecondarySupportWithThreadOnlyExpectThread",
          .supportsConcurrent = false,
          .trySecondary       = false,
          .hasWiFiCreds       = false,
          .hasThreadDataset   = true,
          .wifiEndpoint       = kInvalidEndpointId,
          .threadEndpoint     = kRootEndpointId,
          .expectedStage      = CommissioningStage::kThreadNetworkSetup,
          .expectedError      = CHIP_NO_ERROR },
        { .name               = "MissingAllParamsExpectCleanupError",
          .supportsConcurrent = false,
          .trySecondary       = false,
          .hasWiFiCreds       = false,
          .hasThreadDataset   = false,
          .wifiEndpoint       = kInvalidEndpointId,
          .threadEndpoint     = kInvalidEndpointId,
          .expectedStage      = CommissioningStage::kCleanup,
          .expectedError      = CHIP_ERROR_INCORRECT_STATE },
    };

    for (const auto & c : cases)
    {
        CommissioningParameters params{};
        if (c.supportsConcurrent)
        {
            params.SetSupportsConcurrentConnection(true);
        }

        if (c.hasWiFiCreds)
        {
            params.SetWiFiCredentials(WiFiCredentials(ByteSpan(), ByteSpan()));
        }
        if (c.hasThreadDataset)
        {
            params.SetThreadOperationalDataset(ByteSpan());
        }

        ASSERT_EQ(mCommissioner.SetCommissioningParameters(params), CHIP_NO_ERROR);

        AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

        if (c.trySecondary)
        {
            privateConfigCommissioner.TrySecondaryNetwork();
        }
        else
        {
            privateConfigCommissioner.ResetNetworkAttemptType();
        }

        ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
        commissioningInfo.network.wifi.endpoint   = c.wifiEndpoint;
        commissioningInfo.network.thread.endpoint = c.threadEndpoint;

        CHIP_ERROR err                             = CHIP_NO_ERROR;
        constexpr CommissioningStage kInvalidStage = static_cast<CommissioningStage>(250);
        auto stage = privateConfigCommissioner.GetNextCommissioningStageNetworkSetup(kInvalidStage, err);

        if (stage != c.expectedStage || err != c.expectedError)
        {
            ChipLogError(Test,
                         "Case %s FAILED: got stage=%d err=%s, expected stage=%d err=%s "
                         "(supportsConcurrent=%d trySecondary=%d hasWiFi=%d hasThread=%d "
                         "wifiEndpoint=0x%03X threadEndpoint=0x%03X)",
                         c.name, static_cast<int>(stage), ErrorStr(err), static_cast<int>(c.expectedStage),
                         ErrorStr(c.expectedError), c.supportsConcurrent, c.trySecondary, c.hasWiFiCreds, c.hasThreadDataset,
                         c.wifiEndpoint, c.threadEndpoint);
        }

        EXPECT_EQ(stage, c.expectedStage);
        EXPECT_EQ(err, c.expectedError);
    }
}

namespace test_helpers {

inline void ExpectReadCommissioningInfoEq(const ReadCommissioningInfo & actual,
                                          const ReadCommissioningInfo & expected = ReadCommissioningInfo())
{
#if CHIP_CONFIG_ENABLE_READ_CLIENT
    EXPECT_EQ(actual.attributes, expected.attributes);
#endif
    EXPECT_EQ(actual.requiresUTC, expected.requiresUTC);
    EXPECT_EQ(actual.requiresTimeZone, expected.requiresTimeZone);
    EXPECT_EQ(actual.requiresDefaultNTP, expected.requiresDefaultNTP);
    EXPECT_EQ(actual.requiresTrustedTimeSource, expected.requiresTrustedTimeSource);
    EXPECT_EQ(actual.maxTimeZoneSize, expected.maxTimeZoneSize);
    EXPECT_EQ(actual.maxDSTSize, expected.maxDSTSize);
    EXPECT_EQ(actual.remoteNodeId, expected.remoteNodeId);
    EXPECT_EQ(actual.supportsConcurrentConnection, expected.supportsConcurrentConnection);

    // not asserting on nested structs
}

inline void ExpectOperationalDeviceProxyEq(const chip::OperationalDeviceProxy & actual,
                                           const chip::OperationalDeviceProxy & expected)
{
    EXPECT_EQ(actual.GetDeviceId(), expected.GetDeviceId());
    EXPECT_EQ(actual.GetExchangeManager(), expected.GetExchangeManager());
    EXPECT_EQ(actual.GetSecureSession().HasValue(), expected.GetSecureSession().HasValue());
    EXPECT_EQ(actual.GetPeerScopedNodeId(), expected.GetPeerScopedNodeId());
}
struct ActiveSessionState
{
    CommissioneeDeviceProxy device;
    Transport::SecureSessionTable sessionTable;
    // std::optional is perfect for holding an object that is constructed after its container
    std::optional<Transport::SecureSession> session;

    // The RAII guard is part of the state!
    std::optional<ScopedSessionDetacher> detacher;
};

auto CreateAndConnectTestDevice(AutoCommissionerTestAccess & accessor)
{
    auto state = std::make_unique<ActiveSessionState>();

    state->device.Init(ControllerDeviceInitParams(), 0x00, Transport::Type::kUdp);
    accessor.SetDeviceCommissioneeProxy(&state->device);

    state->sessionTable.Init();

    // Construct the session in-place inside the struct
    state->session.emplace(state->sessionTable, Transport::SecureSession::Type::kPASE, 1234, 0x1111, 0x2222, CATValues(), 5678, 1,
                           GetDefaultMRPConfig());

    SessionHandle handle(state->session.value());
    EXPECT_EQ(state->device.SetConnected(handle), CHIP_NO_ERROR);

    // Construct the RAII guard in-place
    state->detacher.emplace(state->device);

    return state;
}

} // namespace test_helpers

TEST_F(AutoCommissionerTest, CleanupCommissioning_ResetsStateAfterTryingSecondaryNetwork)
{

    mParams.SetSupportsConcurrentConnection(true);
    mParams.SetWiFiCredentials(WiFiCredentials(ByteSpan(), ByteSpan()));
    mParams.SetThreadOperationalDataset(ByteSpan());

    mParams.SetDAC(TestCerts::sTestCert_DAC_FFF1_8000_0000_Cert);
    mParams.SetPAI(TestCerts::sTestCert_PAI_FFF1_8000_Cert);
    EXPECT_EQ(mCommissioner.SetCommissioningParameters(mParams), CHIP_NO_ERROR);

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    CommissioneeDeviceProxy device;
    ControllerDeviceInitParams initParams; // Use the one from the test fixture if available
    device.Init(initParams, 0x00, Transport::Type::kUdp);
    privateConfigCommissioner.SetDeviceCommissioneeProxy(&device);

    Messaging::ExchangeManager exchangeMgr;

    Transport::SecureSessionTable sessionTable;
    sessionTable.Init();
    Transport::SecureSession testSession(sessionTable, Transport::SecureSession::Type::kPASE, 1234, 0x1111, 0x2222, CATValues(),
                                         5678, 1, GetDefaultMRPConfig());
    SessionHandle sessionHandle(testSession);
    OperationalDeviceProxy operationalProxy(&exchangeMgr, sessionHandle);
    // Use std::move because the test accessor's setter is designed to move.
    privateConfigCommissioner.SetOperationalDeviceProxy(operationalProxy);

    chip::Controller::ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
    commissioningInfo.requiresUTC                               = true;
    commissioningInfo.requiresTimeZone                          = true;
    commissioningInfo.maxTimeZoneSize                           = 128;
    commissioningInfo.remoteNodeId                              = 0xDEADBEEF;
    commissioningInfo.supportsConcurrentConnection              = false;
    commissioningInfo.network.wifi.endpoint                     = 0x000;
    commissioningInfo.network.thread.endpoint                   = 0x000;

    privateConfigCommissioner.SetNeedsDST();
    privateConfigCommissioner.TrySecondaryNetwork();

    privateConfigCommissioner.CleanupCommissioning();

    EXPECT_EQ(privateConfigCommissioner.TryingSecondaryNetwork(), false);
    EXPECT_TRUE(privateConfigCommissioner.GetPAI().empty());
    EXPECT_TRUE(privateConfigCommissioner.GetDAC().empty());
    EXPECT_EQ(privateConfigCommissioner.GetCommissioneeDeviceProxy(), nullptr);
    test_helpers::ExpectOperationalDeviceProxyEq(privateConfigCommissioner.GetOperationalDeviceProxy(), OperationalDeviceProxy());
    test_helpers::ExpectReadCommissioningInfoEq(privateConfigCommissioner.GetDeviceCommissioningInfo(), ReadCommissioningInfo());
    EXPECT_EQ(privateConfigCommissioner.GetNeedsDST(), false);
}

TEST_F(AutoCommissionerTest, GetCommandTimeout_CalculatesCorrectlyWithoutActiveSession)
{

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    Controller::ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
    CommissioneeDeviceProxy device;
    privateConfigCommissioner.SetDeviceCommissioneeProxy(&device);
    commissioningInfo.network.wifi.minConnectionTime = 20;
    CommissioningStage stage                         = kWiFiNetworkEnable;
    System::Clock::Timeout expectedTimeout           = kMinimumCommissioningStepTimeout;

    // no need to test semi-hardcoded pathways

    EXPECT_EQ(privateConfigCommissioner.GetCommandTimeout(privateConfigCommissioner.GetCommissioneeDeviceProxy(), stage),
              MakeOptional(expectedTimeout));
}

TEST_F(AutoCommissionerTest, GetCommandTimeout_CalculatesCorrectlyWithActiveSession)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    Controller::ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();

    // The returned 'state' object now correctly manages all lifetimes.
    auto state = test_helpers::CreateAndConnectTestDevice(privateConfigCommissioner);

    CommissioningStage stage                           = kThreadNetworkEnable;
    commissioningInfo.network.thread.minConnectionTime = 40;
    System::Clock::Timeout expectedTimeout = System::Clock::Seconds16(commissioningInfo.network.thread.minConnectionTime);

    // We use the device from our state object. It's guaranteed to be valid.
    EXPECT_EQ(privateConfigCommissioner.GetCommandTimeout(&state->device, stage), MakeOptional(expectedTimeout));

    // When 'state' goes out of scope at the end of the function, the ScopedSessionDetacher
    // inside it is automatically destroyed, safely calling DetachSecureSession().
}

TEST_F(AutoCommissionerTest, GetDeviceProxyForStep_ReturnsCorrectProxy)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

    enum class ExpectedProxyType
    {
        kCommissionee,
        kOperational
    };

    struct Case
    {
        const char * name;
        bool setOperationalProxy;
        CommissioningStage nextStage;
        ExpectedProxyType expectedProxyType;
    };

    Case cases[] = {
        { .name                = "OperationalProxyForSendComplete",
          .setOperationalProxy = false,
          .nextStage           = CommissioningStage::kSendComplete,
          .expectedProxyType   = ExpectedProxyType::kOperational },
        { .name                = "OperationalProxyForICDSendStayActive",
          .setOperationalProxy = false,
          .nextStage           = CommissioningStage::kICDSendStayActive,
          .expectedProxyType   = ExpectedProxyType::kOperational },
        { .name                = "CommissioneeProxyForCleanup",
          .setOperationalProxy = false,
          .nextStage           = CommissioningStage::kCleanup,
          .expectedProxyType   = ExpectedProxyType::kCommissionee },
        { .name                = "OperationalProxyForCleanupWhenAvailable",
          .setOperationalProxy = true,
          .nextStage           = CommissioningStage::kCleanup,
          .expectedProxyType   = ExpectedProxyType::kOperational },
    };

    for (const auto & c : cases)
    {
        // The 'state' object manages the lifetimes of the device, session,
        // session table, and the RAII detacher.
        auto state = test_helpers::CreateAndConnectTestDevice(privateConfigCommissioner);

        if (c.setOperationalProxy)
        {
            Messaging::ExchangeManager exchangeMgr;
            // The operational proxy is created using the now-guaranteed-to-be-valid session.
            OperationalDeviceProxy operationalProxy(&exchangeMgr, state->device.GetSecureSession().Value());
            privateConfigCommissioner.SetOperationalDeviceProxy(operationalProxy);
        }

        DeviceProxy * expectedProxy = nullptr;
        switch (c.expectedProxyType)
        {
        case ExpectedProxyType::kCommissionee:
            expectedProxy = privateConfigCommissioner.GetCommissioneeDeviceProxy();
            break;
        case ExpectedProxyType::kOperational:
            expectedProxy = &privateConfigCommissioner.GetOperationalDeviceProxy();
            break;
        }

        DeviceProxy * actualProxy = privateConfigCommissioner.GetDeviceProxyForStep(c.nextStage);
        EXPECT_EQ(actualProxy, expectedProxy) << "Test case failed: " << c.name;

        // Cleanup state inside the commissioner for the next iteration of the loop.
        // The 'state' object's resources are cleaned up automatically when it goes out of scope.
        privateConfigCommissioner.CleanupCommissioning();
    }
}

} // namespace
