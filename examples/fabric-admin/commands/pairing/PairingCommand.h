/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <commands/common/CHIPCommand.h>
#include <commands/common/CredentialIssuerCommands.h>
#include <controller/CommissioningDelegate.h>
#include <controller/CurrentFabricRemover.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

enum class PairingMode
{
    None,
    Code,
    CodePaseOnly,
    Ble,
    SoftAP,
    AlreadyDiscovered,
    AlreadyDiscoveredByIndex,
    AlreadyDiscoveredByIndexWithCode,
    OnNetwork,
};

enum class PairingNetworkType
{
    None,
    WiFi,
    Thread,
};

class CommissioningDelegate
{
public:
    virtual void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err) = 0;
    virtual ~CommissioningDelegate()                                            = default;
};

class PairingDelegate
{
public:
    virtual void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) = 0;
    virtual ~PairingDelegate()                                          = default;
};

class PairingCommand : public CHIPCommand,
                       public chip::Controller::DevicePairingDelegate,
                       public chip::Controller::DeviceDiscoveryDelegate,
                       public chip::Credentials::DeviceAttestationDelegate
{
public:
    PairingCommand(const char * commandName, PairingMode mode, PairingNetworkType networkType,
                   CredentialIssuerCommands * credIssuerCmds,
                   chip::Dnssd::DiscoveryFilterType filterType = chip::Dnssd::DiscoveryFilterType::kNone) :
        CHIPCommand(commandName, credIssuerCmds),
        mPairingMode(mode), mNetworkType(networkType), mFilterType(filterType),
        mRemoteAddr{ IPAddress::Any, chip::Inet::InterfaceId::Null() }, mComplex_TimeZones(&mTimeZoneList),
        mComplex_DSTOffsets(&mDSTOffsetList), mCurrentFabricRemoveCallback(OnCurrentFabricRemove, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("bypass-attestation-verifier", 0, 1, &mBypassAttestationVerifier,
                    "Bypass the attestation verifier. If not provided or false, the attestation verifier is not bypassed."
                    " If true, the commissioning will continue in case of attestation verification failure.");
        AddArgument("case-auth-tags", 1, UINT32_MAX, &mCASEAuthTags, "The CATs to be encoded in the NOC sent to the commissionee");
        AddArgument("icd-registration", 0, 1, &mICDRegistration,
                    "Whether to register for check-ins from ICDs during commissioning. Default: false");
        AddArgument("icd-check-in-nodeid", 0, UINT64_MAX, &mICDCheckInNodeId,
                    "The check-in node id for the ICD, default: node id of the commissioner.");
        AddArgument("icd-monitored-subject", 0, UINT64_MAX, &mICDMonitoredSubject,
                    "The monitored subject of the ICD, default: The node id used for icd-check-in-nodeid.");
        AddArgument("icd-symmetric-key", &mICDSymmetricKey, "The 16 bytes ICD symmetric key, default: randomly generated.");
        AddArgument("icd-stay-active-duration", 0, UINT32_MAX, &mICDStayActiveDurationMsec,
                    "If set, a LIT ICD that is commissioned will be requested to stay active for this many milliseconds");

        switch (networkType)
        {
        case PairingNetworkType::None:
            break;
        case PairingNetworkType::WiFi:
            AddArgument("ssid", &mSSID);
            AddArgument("password", &mPassword);
            break;
        case PairingNetworkType::Thread:
            AddArgument("operationalDataset", &mOperationalDataset);
            break;
        }

        switch (mode)
        {
        case PairingMode::None:
            break;
        case PairingMode::Code:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            FALLTHROUGH;
        case PairingMode::CodePaseOnly:
            AddArgument("payload", &mOnboardingPayload);
            AddArgument("discover-once", 0, 1, &mDiscoverOnce);
            AddArgument("use-only-onnetwork-discovery", 0, 1, &mUseOnlyOnNetworkDiscovery);
            break;
        case PairingMode::Ble:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            break;
        case PairingMode::OnNetwork:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("pase-only", 0, 1, &mPaseOnly);
            break;
        case PairingMode::SoftAP:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            AddArgument("pase-only", 0, 1, &mPaseOnly);
            break;
        case PairingMode::AlreadyDiscovered:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            AddArgument("pase-only", 0, 1, &mPaseOnly);
            break;
        case PairingMode::AlreadyDiscoveredByIndex:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("index", 0, UINT16_MAX, &mIndex);
            AddArgument("pase-only", 0, 1, &mPaseOnly);
            break;
        case PairingMode::AlreadyDiscoveredByIndexWithCode:
            AddArgument("skip-commissioning-complete", 0, 1, &mSkipCommissioningComplete);
            AddArgument("payload", &mOnboardingPayload);
            AddArgument("index", 0, UINT16_MAX, &mIndex);
            AddArgument("pase-only", 0, 1, &mPaseOnly);
            break;
        }

        switch (filterType)
        {
        case chip::Dnssd::DiscoveryFilterType::kNone:
            break;
        case chip::Dnssd::DiscoveryFilterType::kShortDiscriminator:
            AddArgument("discriminator", 0, 15, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kLongDiscriminator:
            AddArgument("discriminator", 0, 4096, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kVendorId:
            AddArgument("vendor-id", 0, UINT16_MAX, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
            AddArgument("fabric-id", 0, UINT64_MAX, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kCommissioningMode:
        case chip::Dnssd::DiscoveryFilterType::kCommissioner:
            break;
        case chip::Dnssd::DiscoveryFilterType::kDeviceType:
            AddArgument("device-type", 0, UINT16_MAX, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kInstanceName:
            AddArgument("name", &mDiscoveryFilterInstanceName);
            break;
        }

        if (mode != PairingMode::None)
        {
            AddArgument("country-code", &mCountryCode,
                        "Country code to use to set the Basic Information cluster's Location attribute");

            // mTimeZoneList is an optional argument managed by TypedComplexArgument mComplex_TimeZones.
            // Since optional Complex arguments are not currently supported via the <chip::Optional> class,
            // we explicitly set the kOptional flag.
            AddArgument("time-zone", &mComplex_TimeZones,
                        "TimeZone list to use when setting Time Synchronization cluster's TimeZone attribute", Argument::kOptional);

            // mDSTOffsetList is an optional argument managed by TypedComplexArgument mComplex_DSTOffsets.
            // Since optional Complex arguments are not currently supported via the <chip::Optional> class,
            // we explicitly set the kOptional flag.
            AddArgument("dst-offset", &mComplex_DSTOffsets,
                        "DSTOffset list to use when setting Time Synchronization cluster's DSTOffset attribute",
                        Argument::kOptional);
        }

        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(mTimeout.ValueOr(120)); }

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;
    void OnICDRegistrationComplete(chip::ScopedNodeId deviceId, uint32_t icdCounter) override;
    void OnICDStayActiveComplete(chip::ScopedNodeId deviceId, uint32_t promisedActiveDuration) override;

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::CommissionNodeData & nodeData) override;

    /////////// DeviceAttestationDelegate Interface /////////
    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override;
    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
                                      const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                      chip::Credentials::AttestationVerificationResult attestationResult) override;

    /////////// CommissioningDelegate /////////
    void RegisterCommissioningDelegate(CommissioningDelegate * delegate) { mCommissioningDelegate = delegate; }
    void UnregisterCommissioningDelegate() { mCommissioningDelegate = nullptr; }

    /////////// PairingDelegate /////////
    void RegisterPairingDelegate(PairingDelegate * delegate) { mPairingDelegate = delegate; }
    void UnregisterPairingDelegate() { mPairingDelegate = nullptr; }
    PairingDelegate * GetPairingDelegate() { return mPairingDelegate; }

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR Pair(NodeId remoteId, PeerAddress address);
    CHIP_ERROR PairWithMdns(NodeId remoteId);
    CHIP_ERROR PairWithCode(NodeId remoteId);
    CHIP_ERROR PaseWithCode(NodeId remoteId);
    CHIP_ERROR PairWithMdnsOrBleByIndex(NodeId remoteId, uint16_t index);
    CHIP_ERROR PairWithMdnsOrBleByIndexWithCode(NodeId remoteId, uint16_t index);
    CHIP_ERROR Unpair(NodeId remoteId);
    chip::Controller::CommissioningParameters GetCommissioningParameters();

    const PairingMode mPairingMode;
    const PairingNetworkType mNetworkType;
    const chip::Dnssd::DiscoveryFilterType mFilterType;
    Command::AddressWithInterface mRemoteAddr;
    NodeId mNodeId;
    chip::Optional<uint16_t> mTimeout;
    chip::Optional<bool> mDiscoverOnce;
    chip::Optional<bool> mUseOnlyOnNetworkDiscovery;
    chip::Optional<bool> mPaseOnly;
    chip::Optional<bool> mSkipCommissioningComplete;
    chip::Optional<bool> mBypassAttestationVerifier;
    chip::Optional<std::vector<uint32_t>> mCASEAuthTags;
    chip::Optional<char *> mCountryCode;
    chip::Optional<bool> mICDRegistration;
    chip::Optional<NodeId> mICDCheckInNodeId;
    chip::Optional<chip::ByteSpan> mICDSymmetricKey;
    chip::Optional<uint64_t> mICDMonitoredSubject;
    chip::Optional<uint32_t> mICDStayActiveDurationMsec;
    chip::app::DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type> mTimeZoneList;
    TypedComplexArgument<chip::app::DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type>>
        mComplex_TimeZones;
    chip::app::DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type> mDSTOffsetList;
    TypedComplexArgument<chip::app::DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type>>
        mComplex_DSTOffsets;

    uint16_t mRemotePort;
    uint16_t mDiscriminator;
    uint32_t mSetupPINCode;
    uint16_t mIndex;
    chip::ByteSpan mOperationalDataset;
    chip::ByteSpan mSSID;
    chip::ByteSpan mPassword;
    char * mOnboardingPayload;
    uint64_t mDiscoveryFilterCode;
    char * mDiscoveryFilterInstanceName;

    bool mDeviceIsICD;
    uint8_t mRandomGeneratedICDSymmetricKey[chip::Crypto::kAES_CCM128_Key_Length];

    // For unpair
    chip::Platform::UniquePtr<chip::Controller::CurrentFabricRemover> mCurrentFabricRemover;
    chip::Callback::Callback<chip::Controller::OnCurrentFabricRemove> mCurrentFabricRemoveCallback;

    CommissioningDelegate * mCommissioningDelegate = nullptr;
    PairingDelegate * mPairingDelegate             = nullptr;

    static void OnCurrentFabricRemove(void * context, NodeId remoteNodeId, CHIP_ERROR status);
    void PersistIcdInfo();
};
