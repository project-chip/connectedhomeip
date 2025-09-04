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
#include <cstring>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
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

TEST_F(AutoCommissionerTest, TrySecondaryNetwork_confirm_true)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    privateConfigCommissioner.AccessTrySecondaryNetwork();
    EXPECT_EQ(privateConfigCommissioner.AccessTryingSecondaryNetwork(), true);
}

TEST_F(AutoCommissionerTest, ResetTryingSecondaryNetwork_confirm_false)
{
    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    privateConfigCommissioner.AccessResetTryingSecondaryNetwork();
    EXPECT_EQ(privateConfigCommissioner.AccessTryingSecondaryNetwork(), false);
}
TEST_F(AutoCommissionerTest, IsScanNeededCombinations)
{
    struct Case
    {
        const char * name;
        bool attemptWiFi;
        bool attemptThread;
        EndpointId wifiEndpoint;
        EndpointId threadEndpoint;
        bool expected;
    };

    Case cases[] = {
        { "WiFiAndThreadSet", true, true, 0x000, 0x000, true },
        { "WiFiAndThreadNotSet", false, false, kInvalidEndpointId, kInvalidEndpointId, false },
        { "WiFiOnlySet", true, false, 0x000, kInvalidEndpointId, true },
        { "ThreadOnlySet", false, true, kInvalidEndpointId, 0x000, true },
    };

    for (const auto & c : cases)
    {
        CommissioningParameters params{};
        params.SetAttemptWiFiNetworkScan(c.attemptWiFi);
        params.SetAttemptThreadNetworkScan(c.attemptThread);
        EXPECT_EQ(mCommissioner.SetCommissioningParameters(params), CHIP_NO_ERROR);

        AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
        ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
        commissioningInfo.network.wifi.endpoint   = c.wifiEndpoint;
        commissioningInfo.network.thread.endpoint = c.threadEndpoint;

        bool result = privateConfigCommissioner.AccessIsScanNeeded();

        if (result != c.expected)
        {
            ChipLogError(
                Test,
                "%s failed: result=%d expected=%d, attemptWiFi=%d, attemptThread=%d, wifiEndpoint=0x%03X, threadEndpoint=0x%03X",
                c.name, result, c.expected, c.attemptWiFi, c.attemptThread, c.wifiEndpoint, c.threadEndpoint);
        }
        EXPECT_EQ(result, c.expected);
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
        bool expected;
    };

    Case cases[] = {
        { "AllConditionsTrue", true, true, true, 0x000, 0x000, true },
        { "NoConcurrentConnection", false, true, true, 0x000, 0x000, false },
        { "NoWiFiCredentials", true, false, true, 0x000, 0x000, false },
        { "NoThreadDataset", true, true, false, 0x000, 0x000, false },
        { "InvalidWiFiEndpoint", true, true, true, kInvalidEndpointId, 0x000, false },
        { "InvalidThreadEndpoint", true, true, true, 0x000, kInvalidEndpointId, false },
    };

    for (const auto & c : cases)
    {
        CommissioningParameters params{};
        params.SetSupportsConcurrentConnection(c.supportsConcurrent);

        if (c.hasWiFiCreds)
            params.SetWiFiCredentials(WiFiCredentials(ByteSpan(), ByteSpan()));
        if (c.hasThreadDataset)
            params.SetThreadOperationalDataset(ByteSpan());

        EXPECT_EQ(mCommissioner.SetCommissioningParameters(params), CHIP_NO_ERROR);

        AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
        ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
        commissioningInfo.network.wifi.endpoint   = c.wifiEndpoint;
        commissioningInfo.network.thread.endpoint = c.threadEndpoint;

        bool result = privateConfigCommissioner.AccessIsSecondaryNetworkSupported();
        if (result != c.expected)
        {
            ChipLogError(Test,
                         "%s failed: result=%d expected=%d, supportsConcurrent=%d, hasWiFiCreds=%d, hasThreadDataset=%d, "
                         "wifiEndpoint=0x%03X, threadEndpoint=0x%03X",
                         c.name, result, c.expected, c.supportsConcurrent, c.hasWiFiCreds, c.hasThreadDataset, c.wifiEndpoint,
                         c.threadEndpoint);
        }
        EXPECT_EQ(result, c.expected);
    }
}

TEST_F(AutoCommissionerTest, NetworkSetup_Cases)
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
        { "TrySecondaryWifiAtRootExpectThread", true, true, true, true, kRootEndpointId, kRootEndpointId,
          CommissioningStage::kThreadNetworkSetup, CHIP_NO_ERROR },

        { "TrySecondaryWifiNotRootExpectWiFi", true, true, true, true, kRootEndpointId + 1, kRootEndpointId,
          CommissioningStage::kWiFiNetworkSetup, CHIP_NO_ERROR },

        { "PrimaryWifiAtRootExpectWiFi", true, false, true, true, kRootEndpointId, kRootEndpointId,
          CommissioningStage::kWiFiNetworkSetup, CHIP_NO_ERROR },

        { "PrimaryWifiNotRootExpectThread", true, false, true, true, kRootEndpointId + 1, kRootEndpointId,
          CommissioningStage::kThreadNetworkSetup, CHIP_NO_ERROR },

        { "NoSecondarySupportWithWiFiOnlyExpectWiFi", false, false, true, false, kRootEndpointId, kInvalidEndpointId,
          CommissioningStage::kWiFiNetworkSetup, CHIP_NO_ERROR },

        { "NoSecondarySupportWithThreadOnlyExpectThread", false, false, false, true, kInvalidEndpointId, kRootEndpointId,
          CommissioningStage::kThreadNetworkSetup, CHIP_NO_ERROR },

        { "MissingAllParamsExpectCleanupError", false, false, false, false, kInvalidEndpointId, kInvalidEndpointId,
          CommissioningStage::kCleanup, CHIP_ERROR_INVALID_ARGUMENT },
    };

    for (const auto & c : cases)
    {
        CommissioningParameters params{};
        if (c.supportsConcurrent)
            params.SetSupportsConcurrentConnection(true);
        if (c.hasWiFiCreds)
            params.SetWiFiCredentials(WiFiCredentials(ByteSpan(), ByteSpan()));
        if (c.hasThreadDataset)
            params.SetThreadOperationalDataset(ByteSpan());

        ASSERT_EQ(mCommissioner.SetCommissioningParameters(params), CHIP_NO_ERROR);

        AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);

        if (c.trySecondary)
            privateConfigCommissioner.AccessTrySecondaryNetwork();
        else
            privateConfigCommissioner.AccessResetTryingSecondaryNetwork();

        ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
        commissioningInfo.network.wifi.endpoint   = c.wifiEndpoint;
        commissioningInfo.network.thread.endpoint = c.threadEndpoint;

        CHIP_ERROR err = CHIP_NO_ERROR;
        auto stage     = privateConfigCommissioner.GetNextCommissioningStageNetworkSetup(static_cast<CommissioningStage>(250), err);

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
TEST_F(AutoCommissionerTest, GetEndpoint_Cases)
{
    struct Case
    {
        const char * name;
        CommissioningStage stage;
        EndpointId expectedEndpoint;
    };

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();

    // Configure some dummy endpoints
    commissioningInfo.network.wifi.endpoint   = 0x100;
    commissioningInfo.network.thread.endpoint = 0x200;

    Case cases[] = {
        { "WiFiNetworkSetup", CommissioningStage::kWiFiNetworkSetup, commissioningInfo.network.wifi.endpoint },
        { "WiFiNetworkEnable", CommissioningStage::kWiFiNetworkEnable, commissioningInfo.network.wifi.endpoint },
        { "ThreadNetworkSetup", CommissioningStage::kThreadNetworkSetup, commissioningInfo.network.thread.endpoint },
        { "ThreadNetworkEnable", CommissioningStage::kThreadNetworkEnable, commissioningInfo.network.thread.endpoint },
        { "RemoveWiFiNetworkConfig", CommissioningStage::kRemoveWiFiNetworkConfig, kRootEndpointId },
        { "RemoveThreadNetworkConfig", CommissioningStage::kRemoveThreadNetworkConfig, kRootEndpointId },
        { "UnknownStageDefaultsToRoot", static_cast<CommissioningStage>(250), kRootEndpointId },
    };

    for (const auto & c : cases)
    {
        auto endpoint = privateConfigCommissioner.GetEndpoint(c.stage);

        if (endpoint != c.expectedEndpoint)
        {
            ChipLogError(Test, "Case %s FAILED: got endpoint=0x%03X, expected=0x%03X (stage=%d)", c.name, endpoint,
                         c.expectedEndpoint, static_cast<int>(c.stage));
        }

        EXPECT_EQ(endpoint, c.expectedEndpoint);
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

    // not asserting on  nested structs
}

inline void ExpectOperationalDeviceProxyEq(const chip::OperationalDeviceProxy & actual,
                                           const chip::OperationalDeviceProxy & expected)
{
    EXPECT_EQ(actual.GetDeviceId(), expected.GetDeviceId());
    EXPECT_EQ(actual.GetExchangeManager(), expected.GetExchangeManager());
    EXPECT_EQ(actual.GetSecureSession().HasValue(), expected.GetSecureSession().HasValue());
    EXPECT_EQ(actual.GetPeerScopedNodeId(), expected.GetPeerScopedNodeId());
}
} // namespace test_helpers

TEST_F(AutoCommissionerTest, CleanupCommissioning_SecondaryNetworkSupported_case1)
{
    mParams.SetSupportsConcurrentConnection(true);
    mParams.SetWiFiCredentials(WiFiCredentials(ByteSpan(), ByteSpan()));
    mParams.SetThreadOperationalDataset(ByteSpan());
    EXPECT_EQ(mCommissioner.SetCommissioningParameters(mParams), CHIP_NO_ERROR);

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
    commissioningInfo.network.wifi.endpoint   = kRootEndpointId;
    commissioningInfo.network.thread.endpoint = kRootEndpointId;

    privateConfigCommissioner.AccessTrySecondaryNetwork();

    privateConfigCommissioner.CleanupCommissioning();

    EXPECT_EQ(privateConfigCommissioner.AccessTryingSecondaryNetwork(), false);
    EXPECT_TRUE(privateConfigCommissioner.GetPAI().empty());
    EXPECT_TRUE(privateConfigCommissioner.GetDAC().empty());
    EXPECT_EQ(privateConfigCommissioner.GetCommissioneeDeviceProxy(), nullptr);
    test_helpers::ExpectOperationalDeviceProxyEq(privateConfigCommissioner.GetOperationalDeviceProxy(), OperationalDeviceProxy());
    test_helpers::ExpectReadCommissioningInfoEq(privateConfigCommissioner.GetDeviceCommissioningInfo(), ReadCommissioningInfo());
    EXPECT_EQ(privateConfigCommissioner.GetNeedsDST(), false);
}

TEST_F(AutoCommissionerTest, CleanupCommissioning_SecondaryNetworkSupported_case2)
{

    AutoCommissionerTestAccess privateConfigCommissioner(&mCommissioner);
    ReadCommissioningInfo & commissioningInfo = privateConfigCommissioner.GetDeviceCommissioningInfo();
    CommissioneeDeviceProxy * device          = new CommissioneeDeviceProxy();
    privateConfigCommissioner.SetDeviceCommissioneeProxy(device);
    struct Case
    {
        CommissioningStage stage;
        System::Clock::Timeout expectedTimeout;
    };
    commissioningInfo.network.wifi.minConnectionTime   = 40;
    commissioningInfo.network.thread.minConnectionTime = 40;
    Case cases[]                                       = {
        { kWiFiNetworkEnable, System::Clock::Seconds16(commissioningInfo.network.wifi.minConnectionTime) },
        { kThreadNetworkEnable, System::Clock::Seconds16(commissioningInfo.network.thread.minConnectionTime) },
        { kSendNOC, std::max(System::Clock::Timeout(System::Clock::Seconds16(7)), kMinimumCommissioningStepTimeout) },
        { kSendOpCertSigningRequest,
                                                std::max(System::Clock::Timeout(System::Clock::Seconds16(7)), kMinimumCommissioningStepTimeout) },
        { static_cast<CommissioningStage>(250), std::max(app::kExpectedIMProcessingTime, kMinimumCommissioningStepTimeout) },
    };

    for (auto & c : cases)
    {
        EXPECT_EQ(privateConfigCommissioner.GetCommandTimeout(privateConfigCommissioner.GetCommissioneeDeviceProxy(), c.stage),
                  MakeOptional(c.expectedTimeout));
    }
}

} // namespace
