#include <pw_unit_test/framework.h>

#include <controller/CommissioningDelegate.h>
#include <cstring>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::Controller;

namespace {

class CommissioningDelegateTest : public ::testing::Test
{
    // This test fixture is currently empty, but can be extended in the future.
};

struct TestDADelegate : public Credentials::DeviceAttestationDelegate
{
    // Return a fixed timeout to make behavior observable in tests
    Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override { return MakeOptional<uint16_t>(uint16_t{ 42 }); }

    // Required pure virtual. We won’t invoke it in this test, but it must exist.
    void OnDeviceAttestationCompleted(Controller::DeviceCommissioner * /*deviceCommissioner*/, DeviceProxy * /*device*/,
                                      const Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & /*info*/,
                                      Credentials::AttestationVerificationResult /*attestationResult*/) override
    {
        // no-op
    }

    // Override to return true so we can assert a meaningful value
    bool ShouldWaitAfterDeviceAttestation() override { return true; }
};

// ---------- StageToString (and MetricKey… if enabled) ----------
TEST_F(CommissioningDelegateTest, StageToString_AllCases_AndDefault)
{
    struct Case
    {
        CommissioningStage stage;
        const char * str;
    } cases[] = {
        { kError, "Error" },
        { kSecurePairing, "SecurePairing" },
        { kReadCommissioningInfo, "ReadCommissioningInfo" },
        { kArmFailsafe, "ArmFailSafe" },
        { kScanNetworks, "ScanNetworks" },
        { kConfigRegulatory, "ConfigRegulatory" },
        { kConfigureTCAcknowledgments, "ConfigureTCAcknowledgments" },
        { kConfigureUTCTime, "ConfigureUTCTime" },
        { kConfigureTimeZone, "ConfigureTimeZone" },
        { kConfigureDSTOffset, "ConfigureDSTOffset" },
        { kConfigureDefaultNTP, "ConfigureDefaultNTP" },
        { kSendPAICertificateRequest, "SendPAICertificateRequest" },
        { kSendDACCertificateRequest, "SendDACCertificateRequest" },
        { kSendAttestationRequest, "SendAttestationRequest" },
        { kAttestationVerification, "AttestationVerification" },
        { kAttestationRevocationCheck, "AttestationRevocationCheck" },
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        { kJCMTrustVerification, "JCMTrustVerification" },
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        { kSendOpCertSigningRequest, "SendOpCertSigningRequest" },
        { kValidateCSR, "ValidateCSR" },
        { kGenerateNOCChain, "GenerateNOCChain" },
        { kSendTrustedRootCert, "SendTrustedRootCert" },
        { kSendNOC, "SendNOC" },
        { kConfigureTrustedTimeSource, "ConfigureTrustedTimeSource" },
        { kICDGetRegistrationInfo, "ICDGetRegistrationInfo" },
        { kICDRegistration, "ICDRegistration" },
        { kWiFiNetworkSetup, "WiFiNetworkSetup" },
        { kThreadNetworkSetup, "ThreadNetworkSetup" },
        { kFailsafeBeforeWiFiEnable, "FailsafeBeforeWiFiEnable" },
        { kFailsafeBeforeThreadEnable, "FailsafeBeforeThreadEnable" },
        { kWiFiNetworkEnable, "WiFiNetworkEnable" },
        { kThreadNetworkEnable, "ThreadNetworkEnable" },
        { kEvictPreviousCaseSessions, "EvictPreviousCaseSessions" },
        { kFindOperationalForStayActive, "FindOperationalForStayActive" },
        { kFindOperationalForCommissioningComplete, "FindOperationalForCommissioningComplete" },
        { kICDSendStayActive, "ICDSendStayActive" },
        { kSendComplete, "SendComplete" },
        { kCleanup, "Cleanup" },
        { kNeedsNetworkCreds, "NeedsNetworkCreds" },
        { kPrimaryOperationalNetworkFailed, "PrimaryOperationalNetworkFailed" },
        { kRemoveWiFiNetworkConfig, "RemoveWiFiNetworkConfig" },
        { kRemoveThreadNetworkConfig, "RemoveThreadNetworkConfig" },
    };

    for (const auto & c : cases)
    {
        EXPECT_STREQ(StageToString(c.stage), c.str);
    }

    // default/unknown
    EXPECT_STREQ(StageToString(static_cast<CommissioningStage>(250)), "???");
}

#if MATTER_TRACING_ENABLED
TEST_F(CommissioningDelegateTest, MetricKeyForCommissioningStage_AllCases)
{
    struct Case
    {
        CommissioningStage stage;
        const char * key;
    } cases[] = {
        { kError, "core_commissioning_stage_error" },
        { kSecurePairing, "core_commissioning_stage_secure_pairing" },
        { kReadCommissioningInfo, "core_commissioning_stage_read_commissioning_info" },
        { kArmFailsafe, "core_commissioning_stage_arm_failsafe" },
        { kScanNetworks, "core_commissioning_stage_scan_networks" },
        { kConfigRegulatory, "core_commissioning_stage_config_regulatory" },
        { kConfigureUTCTime, "core_commissioning_stage_configure_utc_time" },
        { kConfigureTimeZone, "core_commissioning_stage_configure_timezone" },
        { kConfigureDSTOffset, "core_commissioning_stage_configure_dst_offset" },
        { kConfigureDefaultNTP, "core_commissioning_stage_configure_default_ntp" },
        { kSendPAICertificateRequest, "core_commissioning_stage_send_pai_certificate_request" },
        { kSendDACCertificateRequest, "core_commissioning_stage_send_dac_certificate_request" },
        { kSendAttestationRequest, "core_commissioning_stage_send_attestation_request" },
        { kAttestationVerification, "core_commissioning_stage_attestation_verification" },
        { kSendOpCertSigningRequest, "core_commissioning_stage_opcert_signing_request" },
        { kValidateCSR, "core_commissioning_stage_validate_csr" },
        { kGenerateNOCChain, "core_commissioning_stage_generate_noc_chain" },
        { kSendTrustedRootCert, "core_commissioning_stage_send_trusted_root_cert" },
        { kSendNOC, "core_commissioning_stage_send_noc" },
        { kConfigureTrustedTimeSource, "core_commissioning_stage_configure_trusted_time_source" },
        { kICDGetRegistrationInfo, "core_commissioning_stage_icd_get_registration_info" },
        { kICDRegistration, "core_commissioning_stage_icd_registration" },
        { kWiFiNetworkSetup, "core_commissioning_stage_wifi_network_setup" },
        { kThreadNetworkSetup, "core_commissioning_stage_thread_network_setup" },
        { kFailsafeBeforeWiFiEnable, "core_commissioning_stage_failsafe_before_wifi_enable" },
        { kFailsafeBeforeThreadEnable, "core_commissioning_stage_failsafe_before_thread_enable" },
        { kWiFiNetworkEnable, "core_commissioning_stage_wifi_network_enable" },
        { kThreadNetworkEnable, "core_commissioning_stage_thread_network_enable" },
        { kEvictPreviousCaseSessions, "core_commissioning_stage_evict_previous_case_sessions" },
        { kFindOperationalForStayActive, "core_commissioning_stage_find_operational_for_stay_active" },
        { kFindOperationalForCommissioningComplete, "core_commissioning_stage_find_operational_for_commissioning_complete" },
        { kICDSendStayActive, "core_commissioning_stage_icd_send_stay_active" },
        { kSendComplete, "core_commissioning_stage_send_complete" },
        { kCleanup, "core_commissioning_stage_cleanup" },
        { kNeedsNetworkCreds, "core_commissioning_stage_need_network_creds" },
    };

    for (const auto & c : cases)
    {
        EXPECT_STREQ(MetricKeyForCommissioningStage(c.stage), c.key);
    }

    // default/unknown
    EXPECT_STREQ(MetricKeyForCommissioningStage(static_cast<CommissioningStage>(250)), "core_commissioning_stage_unknown");
}
#endif

// ---------- CommissioningParameters getters/setters & buffer-clearing ----------
TEST_F(CommissioningDelegateTest, CommissioningParameters_DefaultsAndSettersExerciseUncovered)
{
    CommissioningParameters p;

    // Defaults (a few)
    EXPECT_FALSE(p.GetFailsafeTimerSeconds().HasValue());
    EXPECT_FALSE(p.GetDeviceRegulatoryLocation().HasValue());
    EXPECT_FALSE(p.GetSupportsConcurrentConnection().HasValue());
    EXPECT_FALSE(p.GetAttemptWiFiNetworkScan().HasValue());
    EXPECT_FALSE(p.GetAttemptThreadNetworkScan().HasValue());
    EXPECT_FALSE(p.GetSkipCommissioningComplete().HasValue());
    EXPECT_FALSE(p.GetICDCheckInNodeId().HasValue());
    EXPECT_FALSE(p.GetICDMonitoredSubject().HasValue());
    EXPECT_FALSE(p.GetICDSymmetricKey().HasValue());
    EXPECT_FALSE(p.GetICDClientType().HasValue());
    EXPECT_FALSE(p.GetICDStayActiveDurationMsec().HasValue());
    EXPECT_EQ(p.GetICDRegistrationStrategy(), ICDRegistrationStrategy::kIgnore);
    EXPECT_FALSE(p.GetDefaultNTP().HasValue());
    EXPECT_EQ(p.GetExtraReadPaths().size(), size_t{ 0 });

    // Set timers, regulatory, supports concurrent
    p.SetFailsafeTimerSeconds(45);
    p.SetCASEFailsafeTimerSeconds(30);
    p.SetDeviceRegulatoryLocation(app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoor);
    p.SetSupportsConcurrentConnection(true);
    EXPECT_EQ(p.GetFailsafeTimerSeconds().Value(), 45);
    EXPECT_EQ(p.GetCASEFailsafeTimerSeconds().Value(), 30);
    EXPECT_TRUE(p.GetSupportsConcurrentConnection().Value());

    // Country code + T&C ack
    p.SetCountryCode("US"_span);
    TermsAndConditionsAcknowledgement tca{ .acceptedTermsAndConditions = 1, .acceptedTermsAndConditionsVersion = 2 };
    p.SetTermsAndConditionsAcknowledgement(tca);
    ASSERT_TRUE(p.GetCountryCode().HasValue());
    EXPECT_TRUE(p.GetCountryCode().Value().data_equal("US"_span));
    ASSERT_TRUE(p.GetTermsAndConditionsAcknowledgement().HasValue());
    EXPECT_EQ(p.GetTermsAndConditionsAcknowledgement().Value().acceptedTermsAndConditions, 1);

    // TimeZone & DST lists (single item is fine)
    app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type tz{};
    tz.offset  = 1800;
    tz.validAt = 123;
    tz.name.SetValue("AMT"_span);
    app::DataModel::List<decltype(tz)> tzList(&tz, 1);
    p.SetTimeZone(tzList);
    ASSERT_TRUE(p.GetTimeZone().HasValue());
    EXPECT_EQ(p.GetTimeZone().Value().size(), size_t{ 1 });

    app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type dst{};
    dst.offset        = 3600;
    dst.validStarting = static_cast<uint64_t>(50);
    dst.validUntil    = static_cast<uint64_t>(200);
    app::DataModel::List<decltype(dst)> dstList(&dst, 1);
    p.SetDSTOffsets(dstList);
    ASSERT_TRUE(p.GetDSTOffsets().HasValue());
    EXPECT_EQ(p.GetDSTOffsets().Value().size(), size_t{ 1 });

    // Default NTP: set value then null
    p.SetDefaultNTP(app::DataModel::MakeNullable("pool.ntp.org"_span));
    ASSERT_TRUE(p.GetDefaultNTP().HasValue());
    EXPECT_FALSE(p.GetDefaultNTP().Value().IsNull());
    EXPECT_TRUE(p.GetDefaultNTP().Value().Value().data_equal("pool.ntp.org"_span));
    p.SetDefaultNTP(app::DataModel::Nullable<CharSpan>{}); // null
    ASSERT_TRUE(p.GetDefaultNTP().HasValue());
    EXPECT_TRUE(p.GetDefaultNTP().Value().IsNull());

    // Trusted time source: just set a null Nullable to exercise setter/getter
    app::DataModel::Nullable<app::Clusters::TimeSynchronization::Structs::FabricScopedTrustedTimeSourceStruct::Type> tts;
    p.SetTrustedTimeSource(tts);
    ASSERT_TRUE(p.GetTrustedTimeSource().HasValue());
    EXPECT_TRUE(p.GetTrustedTimeSource().Value().IsNull());

    // Nonces
    const uint8_t csr[5] = { 1, 2, 3, 4, 5 };
    const uint8_t att[3] = { 9, 9, 9 };
    p.SetCSRNonce(ByteSpan{ csr });
    p.SetAttestationNonce(ByteSpan{ att });
    EXPECT_TRUE(p.GetCSRNonce().HasValue());
    EXPECT_TRUE(p.GetAttestationNonce().HasValue());

    // WiFi + Thread datasets set scans to false
    const uint8_t ssid[] = { 'A', 'P' };
    const uint8_t pwd[]  = { 'p', 'w' };
    p.SetWiFiCredentials(WiFiCredentials{ ByteSpan{ ssid }, ByteSpan{ pwd } });
    EXPECT_TRUE(p.GetWiFiCredentials().HasValue());
    ASSERT_TRUE(p.GetAttemptWiFiNetworkScan().HasValue());
    EXPECT_FALSE(p.GetAttemptWiFiNetworkScan().Value());

    const uint8_t tds[] = { 7, 7, 7 };
    p.SetThreadOperationalDataset(ByteSpan{ tds });
    EXPECT_TRUE(p.GetThreadOperationalDataset().HasValue());
    ASSERT_TRUE(p.GetAttemptThreadNetworkScan().HasValue());
    EXPECT_FALSE(p.GetAttemptThreadNetworkScan().Value());

    // AttemptThreadNetworkScan should be ignored once dataset is present
    p.SetAttemptThreadNetworkScan(true);
    ASSERT_TRUE(p.GetAttemptThreadNetworkScan().HasValue());
    EXPECT_FALSE(p.GetAttemptThreadNetworkScan().Value());

    // NOCChain params + certs + IDs (exercise getters)
    NOCChainGenerationParameters ngp{ ByteSpan{ csr }, ByteSpan{ att } };
    p.SetNOCChainGenerationParameters(ngp);
    p.SetRootCert(ByteSpan{ csr });
    p.SetNoc(ByteSpan{ att });
    p.SetIcac(ByteSpan{ csr });
    // IdentityProtectionKey: provide 16 bytes (AES-128-sized)
    std::array<uint8_t, 16> ipkBytes{};
    p.SetIpk(ipkBytes);
    p.SetAdminSubject(NodeId{ 0x1111'2222'3333'4444ULL });

    EXPECT_TRUE(p.GetNOCChainGenerationParameters().HasValue());
    EXPECT_TRUE(p.GetRootCert().HasValue());
    EXPECT_TRUE(p.GetNoc().HasValue());
    EXPECT_TRUE(p.GetIcac().HasValue());
    EXPECT_TRUE(p.GetIpk().HasValue());
    EXPECT_TRUE(p.GetAdminSubject().HasValue());

    // Remote info + default regulatory/location capability
    p.SetRemoteVendorId(VendorId::Common);
    p.SetRemoteProductId(1234);
    p.SetDefaultRegulatoryLocation(app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoorOutdoor);
    p.SetLocationCapability(app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoorOutdoor);
    EXPECT_TRUE(p.GetRemoteVendorId().HasValue());
    EXPECT_TRUE(p.GetRemoteProductId().HasValue());
    EXPECT_TRUE(p.GetDefaultRegulatoryLocation().HasValue());
    EXPECT_TRUE(p.GetLocationCapability().HasValue());

    // Non-null path using our concrete no-op delegate
    TestDADelegate delegate;
    p.SetDeviceAttestationDelegate(&delegate);
    ASSERT_NE(p.GetDeviceAttestationDelegate(), nullptr);
    EXPECT_EQ(p.GetDeviceAttestationDelegate(), &delegate);

    // Call a couple of virtuals to ensure vtable is correct and behavior is observable
    auto * d = p.GetDeviceAttestationDelegate();
    ASSERT_NE(d, nullptr);
    EXPECT_TRUE(d->ShouldWaitAfterDeviceAttestation());

    const auto timeout = d->FailSafeExpiryTimeoutSecs();
    ASSERT_TRUE(timeout.HasValue());
    EXPECT_EQ(timeout.Value(), 42);

    // SkipCommissioningComplete + CheckForMatchingFabric flags
    p.SetSkipCommissioningComplete(true);
    p.SetCheckForMatchingFabric(true);
    ASSERT_TRUE(p.GetSkipCommissioningComplete().HasValue());
    EXPECT_TRUE(p.GetSkipCommissioningComplete().Value());
    EXPECT_TRUE(p.GetCheckForMatchingFabric());

    // ICD knobs
    p.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);
    p.SetICDCheckInNodeId(NodeId{ 55 });
    p.SetICDMonitoredSubject(static_cast<uint64_t>(99));
    const uint8_t sym[16] = {};
    p.SetICDSymmetricKey(ByteSpan{ sym });
    p.SetICDClientType(app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);
    p.SetICDStayActiveDurationMsec(123456);
    EXPECT_EQ(p.GetICDRegistrationStrategy(), ICDRegistrationStrategy::kBeforeComplete);
    EXPECT_TRUE(p.GetICDCheckInNodeId().HasValue());
    EXPECT_TRUE(p.GetICDMonitoredSubject().HasValue());
    EXPECT_TRUE(p.GetICDSymmetricKey().HasValue());
    EXPECT_TRUE(p.GetICDClientType().HasValue());
    EXPECT_TRUE(p.GetICDStayActiveDurationMsec().HasValue());
    p.ClearICDStayActiveDurationMsec();
    EXPECT_FALSE(p.GetICDStayActiveDurationMsec().HasValue());

    // Extra read paths
    app::AttributePathParams attrs[2];
    attrs[0] = app::AttributePathParams{ 1, 2, 3 };
    attrs[1] = app::AttributePathParams{ 4, 5, 6 };
    p.SetExtraReadPaths(Span<const app::AttributePathParams>(attrs, 2));
    EXPECT_EQ(p.GetExtraReadPaths().size(), size_t{ 2 });

    // Now clear all external buffer dependent values
    p.ClearExternalBufferDependentValues();
    EXPECT_FALSE(p.GetCSRNonce().HasValue());
    EXPECT_FALSE(p.GetAttestationNonce().HasValue());
    EXPECT_FALSE(p.GetWiFiCredentials().HasValue());
    EXPECT_FALSE(p.GetCountryCode().HasValue());
    EXPECT_FALSE(p.GetThreadOperationalDataset().HasValue());
    EXPECT_FALSE(p.GetNOCChainGenerationParameters().HasValue());
    EXPECT_FALSE(p.GetRootCert().HasValue());
    EXPECT_FALSE(p.GetNoc().HasValue());
    EXPECT_FALSE(p.GetIcac().HasValue());
    EXPECT_FALSE(p.GetIpk().HasValue());
    EXPECT_FALSE(p.GetAttestationElements().HasValue());
    EXPECT_FALSE(p.GetAttestationSignature().HasValue());
    EXPECT_FALSE(p.GetPAI().HasValue());
    EXPECT_FALSE(p.GetDAC().HasValue());
    EXPECT_FALSE(p.GetTimeZone().HasValue());
    EXPECT_FALSE(p.GetDSTOffsets().HasValue());
    EXPECT_FALSE(p.GetDefaultNTP().HasValue());
    EXPECT_FALSE(p.GetICDSymmetricKey().HasValue());
    EXPECT_EQ(p.GetExtraReadPaths().size(), size_t{ 0 });
}

// ---------- Small POD structs & CommissioningReport ----------

TEST_F(CommissioningDelegateTest, PODStructConstructorsCover)
{
    // WiFiCredentials
    const uint8_t s[] = { 'X', 'Y' };
    const uint8_t c[] = { 1, 2, 3 };
    WiFiCredentials w(ByteSpan{ s }, ByteSpan{ c });
    EXPECT_EQ(w.ssid.size(), sizeof(s));
    EXPECT_EQ(w.credentials.size(), sizeof(c));

    // RequestedCertificate / AttestationResponse / CSRResponse
    const uint8_t cert[] = { 0xAA, 0xBB };
    const uint8_t sig[]  = { 0xCC };
    RequestedCertificate rc(ByteSpan{ cert });
    EXPECT_EQ(rc.certificate.size(), sizeof(cert));
    AttestationResponse ar(ByteSpan{ cert }, ByteSpan{ sig });
    EXPECT_EQ(ar.attestationElements.size(), sizeof(cert));
    EXPECT_EQ(ar.signature.size(), sizeof(sig));
    CSRResponse cr(ByteSpan{ cert }, ByteSpan{ sig });
    EXPECT_EQ(cr.nocsrElements.size(), sizeof(cert));
    EXPECT_EQ(cr.signature.size(), sizeof(sig));

    // CommissioningReport default stage
    CommissioningDelegate::CommissioningReport report;
    EXPECT_EQ(report.stageCompleted, kError);
}

TEST_F(CommissioningDelegateTest, AttestationPayloadSettersStoreSpans)
{
    CommissioningParameters p;

    const uint8_t elems[] = { 0x01, 0x02, 0x03, 0x04 };
    const uint8_t sig[]   = { 0xA5, 0x5A };

    p.SetAttestationElements(ByteSpan{ elems });
    p.SetAttestationSignature(ByteSpan{ sig });

    ASSERT_TRUE(p.GetAttestationElements().HasValue());
    ASSERT_TRUE(p.GetAttestationSignature().HasValue());

    const ByteSpan gotElems = p.GetAttestationElements().Value();
    const ByteSpan gotSig   = p.GetAttestationSignature().Value();

    EXPECT_EQ(gotElems.size(), sizeof(elems));
    EXPECT_EQ(gotSig.size(), sizeof(sig));
    EXPECT_EQ(std::memcmp(gotElems.data(), elems, sizeof(elems)), 0);
    EXPECT_EQ(std::memcmp(gotSig.data(), sig, sizeof(sig)), 0);
}

TEST_F(CommissioningDelegateTest, CertSettersStoreSpans)
{
    CommissioningParameters p;

    const uint8_t pai[] = { 0x11, 0x22, 0x33 };
    const uint8_t dac[] = { 0x44, 0x55 };

    p.SetPAI(ByteSpan{ pai });
    p.SetDAC(ByteSpan{ dac });

    ASSERT_TRUE(p.GetPAI().HasValue());
    ASSERT_TRUE(p.GetDAC().HasValue());

    const ByteSpan gotPAI = p.GetPAI().Value();
    const ByteSpan gotDAC = p.GetDAC().Value();

    EXPECT_EQ(gotPAI.size(), sizeof(pai));
    EXPECT_EQ(gotDAC.size(), sizeof(dac));
    EXPECT_EQ(std::memcmp(gotPAI.data(), pai, sizeof(pai)), 0);
    EXPECT_EQ(std::memcmp(gotDAC.data(), dac, sizeof(dac)), 0);
}

TEST_F(CommissioningDelegateTest, RemoteNodeIdSetterStoresValue)
{
    CommissioningParameters p;

    const NodeId id = 0x1111222233334444ULL;
    p.SetRemoteNodeId(id);

    ASSERT_TRUE(p.GetRemoteNodeId().HasValue());
    EXPECT_EQ(p.GetRemoteNodeId().Value(), id);
}

TEST_F(CommissioningDelegateTest, CompletionStatus_DefaultAndRoundTrip)
{
    CommissioningParameters p;

    // Defaults
    const CompletionStatus & def = p.GetCompletionStatus();
    EXPECT_EQ(def.err, CHIP_NO_ERROR);
    EXPECT_FALSE(def.failedStage.HasValue());
    EXPECT_FALSE(def.attestationResult.HasValue());
    EXPECT_FALSE(def.commissioningError.HasValue());
    EXPECT_FALSE(def.networkCommissioningStatus.HasValue());

    // Populate and set
    CompletionStatus cs;
    cs.err                = CHIP_ERROR_INTERNAL;
    cs.failedStage        = MakeOptional(CommissioningStage::kAttestationVerification);
    cs.attestationResult  = MakeOptional(Credentials::AttestationVerificationResult::kPaiAuthorityNotFound);
    cs.commissioningError = MakeOptional(app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kValueOutsideRange);
    cs.networkCommissioningStatus =
        MakeOptional(app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum::kBoundsExceeded);

    p.SetCompletionStatus(cs);
    const CompletionStatus & got = p.GetCompletionStatus();

    EXPECT_EQ(got.err, CHIP_ERROR_INTERNAL);
    ASSERT_TRUE(got.failedStage.HasValue());
    EXPECT_EQ(got.failedStage.Value(), CommissioningStage::kAttestationVerification);
    ASSERT_TRUE(got.attestationResult.HasValue());
    EXPECT_EQ(got.attestationResult.Value(), Credentials::AttestationVerificationResult::kPaiAuthorityNotFound);
    ASSERT_TRUE(got.commissioningError.HasValue());
    EXPECT_EQ(got.commissioningError.Value(), app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kValueOutsideRange);
    ASSERT_TRUE(got.networkCommissioningStatus.HasValue());
    EXPECT_EQ(got.networkCommissioningStatus.Value(),
              app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum::kBoundsExceeded);
}

} // namespace
