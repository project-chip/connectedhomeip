/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "../common/CHIPCommand.h"
#include <controller/CommissioningDelegate.h>
#include <controller/CurrentFabricRemover.h>
#include <zap-generated/CHIPClusters.h>

#include <commands/common/CredentialIssuerCommands.h>
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
    OnNetwork,
};

enum class PairingNetworkType
{
    None,
    WiFi,
    Thread,
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
        mPairingMode(mode), mNetworkType(networkType),
        mFilterType(filterType), mRemoteAddr{ IPAddress::Any, chip::Inet::InterfaceId::Null() },
        mCurrentFabricRemoveCallback(OnCurrentFabricRemove, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("bypass-attestation-verifier", 0, 1, &mBypassAttestationVerifier,
                    "Bypass the attestation verifier. If not provided or false, the attestation verifier is not bypassed."
                    " If true, the commissioning will continue in case of attestation verification failure.");
        AddArgument("case-auth-tags", 1, UINT32_MAX, &mCASEAuthTags, "The CATs to be encoded in the NOC sent to the commissionee");

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

        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(mTimeout.ValueOr(120)); }

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

    /////////// DeviceAttestationDelegate /////////
    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override;
    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
                                      const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                      chip::Credentials::AttestationVerificationResult attestationResult) override;

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR Pair(NodeId remoteId, PeerAddress address);
    CHIP_ERROR PairWithMdns(NodeId remoteId);
    CHIP_ERROR PairWithCode(NodeId remoteId);
    CHIP_ERROR PaseWithCode(NodeId remoteId);
    CHIP_ERROR PairWithMdnsOrBleByIndex(NodeId remoteId, uint16_t index);
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

    // For unpair
    chip::Platform::UniquePtr<chip::Controller::CurrentFabricRemover> mCurrentFabricRemover;
    chip::Callback::Callback<chip::Controller::OnCurrentFabricRemove> mCurrentFabricRemoveCallback;

    static void OnCurrentFabricRemove(void * context, NodeId remoteNodeId, CHIP_ERROR status);
};
