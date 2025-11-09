/*
 *   Copyright (c) 2020-2024 Project CHIP Authors
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

#include "PairingCommand.h"
#include "commissioner/error.hpp"
#include "core/CHIPError.h"
#include "dnssd/TxtFields.h"
#include "dnssd/Types.h"
#include "dnssd/minimal_mdns/Parser.h"
#include "dnssd/minimal_mdns/QueryBuilder.h"
#include "dnssd/minimal_mdns/RecordData.h"
#include "dnssd/minimal_mdns/core/Constants.h"
#include "dnssd/minimal_mdns/core/DnsHeader.h"
#include "dnssd/minimal_mdns/core/QNameString.h"
#include "inet/IPAddress.h"
#include "inet/InetInterface.h"
#include "platform/PlatformManager.h"
#include <commands/common/DeviceScanner.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <cstddef>
#include <future>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <protocols/secure_channel/PASESession.h>

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#include "../dcl/DCLClient.h"
#include "../dcl/DisplayTermsAndConditions.h"
#include "../third_party/ot-commissioner/repo/include/commissioner/commissioner.hpp"
#include "support/BytesToHex.h"
#include "support/CHIPMemString.h"
#include "support/CodeUtils.h"
#include "support/Span.h"
#include "support/ThreadOperationalDataset.h"

#include <iostream>
#include <string>
#include <sys/socket.h>

using namespace ::chip;
using namespace ::chip::Controller;

CHIP_ERROR PairingCommand::RunCommand()
{
    CurrentCommissioner().RegisterPairingDelegate(this);
    // Clear the CATs in OperationalCredentialsIssuer
    mCredIssuerCmds->SetCredentialIssuerCATValues(kUndefinedCATs);

    mDeviceIsICD = false;

    if (mCASEAuthTags.HasValue() && mCASEAuthTags.Value().size() <= kMaxSubjectCATAttributeCount)
    {
        CATValues cats = kUndefinedCATs;
        for (size_t index = 0; index < mCASEAuthTags.Value().size(); ++index)
        {
            cats.values[index] = mCASEAuthTags.Value()[index];
        }
        if (cats.AreValid())
        {
            mCredIssuerCmds->SetCredentialIssuerCATValues(cats);
        }
    }
    return RunInternal(mNodeId);
}

CHIP_ERROR PairingCommand::RunInternal(NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mPairingMode)
    {
    case PairingMode::None:
        err = Unpair(remoteId);
        break;
    case PairingMode::Code:
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        chip::DeviceLayer::ConnectivityMgr().WiFiPafSetApFreq(
            mApFreqStr.HasValue() ? static_cast<uint16_t>(std::stol(mApFreqStr.Value())) : 0);
#endif
        err = PairWithCode(remoteId);
        break;
    case PairingMode::CodePaseOnly:
        err = PaseWithCode(remoteId);
        break;
    case PairingMode::Ble:
        err = Pair(remoteId, PeerAddress::BLE());
        break;
    case PairingMode::Nfc:
        if (mDiscriminator.has_value())
        {
            err = Pair(remoteId, PeerAddress::NFC(mDiscriminator.value()));
        }
        else
        {
            // Discriminator is mandatory
            err = CHIP_ERROR_MESSAGE_INCOMPLETE;
        }
        break;
    case PairingMode::OnNetwork:
        err = PairWithMdns(remoteId);
        break;
    case PairingMode::SoftAP:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort, mRemoteAddr.interfaceId));
        break;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    case PairingMode::WiFiPAF:
        chip::DeviceLayer::ConnectivityMgr().WiFiPafSetApFreq(
            mApFreqStr.HasValue() ? static_cast<uint16_t>(std::stol(mApFreqStr.Value())) : 0);
        err = Pair(remoteId, PeerAddress::WiFiPAF(remoteId));
        break;
#endif
    case PairingMode::AlreadyDiscovered:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort, mRemoteAddr.interfaceId));
        break;
    case PairingMode::AlreadyDiscoveredByIndex:
        err = PairWithMdnsOrBleByIndex(remoteId, mIndex);
        break;
    case PairingMode::AlreadyDiscoveredByIndexWithCode:
        err = PairWithMdnsOrBleByIndexWithCode(remoteId, mIndex);
        break;
    }

    return err;
}

CommissioningParameters PairingCommand::GetCommissioningParameters()
{
    auto params = CommissioningParameters();
    params.SetSkipCommissioningComplete(mSkipCommissioningComplete.ValueOr(false));
    if (mBypassAttestationVerifier.ValueOr(false))
    {
        params.SetDeviceAttestationDelegate(this);
    }

    switch (mNetworkType)
    {
    case PairingNetworkType::WiFi:
        params.SetWiFiCredentials(Controller::WiFiCredentials(mSSID, mPassword));
        break;
    case PairingNetworkType::Thread:
        params.SetThreadOperationalDataset(mOperationalDataset);
        break;
    case PairingNetworkType::WiFiOrThread:
        params.SetWiFiCredentials(Controller::WiFiCredentials(mSSID, mPassword));
        params.SetThreadOperationalDataset(mOperationalDataset);
        break;
    case PairingNetworkType::None:
        if (mThreadDataset.HasValue())
        {
            params.SetThreadOperationalDataset(mThreadDataset.Value());
        }
        break;
    }

    if (mCountryCode.HasValue())
    {
        params.SetCountryCode(CharSpan::fromCharString(mCountryCode.Value()));
    }

    // mTCAcknowledgements and mTCAcknowledgementVersion are optional, but related. When one is missing, default the value to 0, to
    // increase the test tools ability to test the applications.
    if (mTCAcknowledgements.HasValue() || mTCAcknowledgementVersion.HasValue())
    {
        TermsAndConditionsAcknowledgement termsAndConditionsAcknowledgement = {
            .acceptedTermsAndConditions        = mTCAcknowledgements.ValueOr(0),
            .acceptedTermsAndConditionsVersion = mTCAcknowledgementVersion.ValueOr(0),
        };
        params.SetTermsAndConditionsAcknowledgement(termsAndConditionsAcknowledgement);
    }

    // mTimeZoneList is an optional argument managed by TypedComplexArgument mComplex_TimeZones.
    // Since optional Complex arguments are not currently supported via the <chip::Optional> class,
    // we will use mTimeZoneList.data() value to determine if the argument was provided.
    if (mTimeZoneList.data())
    {
        params.SetTimeZone(mTimeZoneList);
    }

    // miDSTOffsetList is an optional argument managed by TypedComplexArgument mComplex_DSTOffsets.
    // Since optional Complex arguments are not currently supported via the <chip::Optional> class,
    // we will use mTimeZoneList.data() value to determine if the argument was provided.
    if (mDSTOffsetList.data())
    {
        params.SetDSTOffsets(mDSTOffsetList);
    }

    if (mICDRegistration.ValueOr(false))
    {
        params.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);

        if (!mICDSymmetricKey.HasValue())
        {
            Crypto::DRBG_get_bytes(mRandomGeneratedICDSymmetricKey, sizeof(mRandomGeneratedICDSymmetricKey));
            mICDSymmetricKey.SetValue(ByteSpan(mRandomGeneratedICDSymmetricKey));
        }
        if (!mICDCheckInNodeId.HasValue())
        {
            mICDCheckInNodeId.SetValue(CurrentCommissioner().GetNodeId());
        }
        if (!mICDMonitoredSubject.HasValue())
        {
            mICDMonitoredSubject.SetValue(mICDCheckInNodeId.Value());
        }
        if (!mICDClientType.HasValue())
        {
            mICDClientType.SetValue(app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);
        }
        // These Optionals must have values now.
        // The commissioner will verify these values.
        params.SetICDSymmetricKey(mICDSymmetricKey.Value());
        if (mICDStayActiveDurationMsec.HasValue())
        {
            params.SetICDStayActiveDurationMsec(mICDStayActiveDurationMsec.Value());
        }
        params.SetICDCheckInNodeId(mICDCheckInNodeId.Value());
        params.SetICDMonitoredSubject(mICDMonitoredSubject.Value());
        params.SetICDClientType(mICDClientType.Value());
    }

    return params;
}

CHIP_ERROR PairingCommand::PaseWithCode(NodeId remoteId)
{
    auto discoveryType = DiscoveryType::kAll;
    if (mUseOnlyOnNetworkDiscovery.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnly;
    }

    if (mDiscoverOnce.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry;
    }

    return CurrentCommissioner().EstablishPASEConnection(remoteId, mOnboardingPayload, discoveryType);
}

CHIP_ERROR PairingCommand::PairWithCode(NodeId remoteId)
{
    CommissioningParameters commissioningParams = GetCommissioningParameters();

    // If no network discovery behavior and no network credentials are provided, assume that the pairing command is trying to pair
    // with an on-network device.
    if (!mUseOnlyOnNetworkDiscovery.HasValue())
    {
        auto threadCredentials = commissioningParams.GetThreadOperationalDataset();
        auto wiFiCredentials   = commissioningParams.GetWiFiCredentials();
        mUseOnlyOnNetworkDiscovery.SetValue(!threadCredentials.HasValue() && !wiFiCredentials.HasValue());
    }

    auto discoveryType = DiscoveryType::kAll;
    if (mUseOnlyOnNetworkDiscovery.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnly;
    }

    if (mDiscoverOnce.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry;
    }

    ReturnErrorOnFailure(MaybeDisplayTermsAndConditions(commissioningParams));
    return CurrentCommissioner().PairDevice(remoteId, mOnboardingPayload, commissioningParams, discoveryType);
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    VerifyOrDieWithMsg(mSetupPINCode.has_value(), chipTool, "Using mSetupPINCode in a mode when we have not gotten one");
    auto params = RendezvousParameters().SetSetupPINCode(mSetupPINCode.value()).SetPeerAddress(address);
    if (mDiscriminator.has_value())
    {
        params.SetDiscriminator(mDiscriminator.value());
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (mPaseOnly.ValueOr(false))
    {
        err = CurrentCommissioner().EstablishPASEConnection(remoteId, params);
    }
    else
    {
        auto commissioningParams = GetCommissioningParameters();
        err                      = CurrentCommissioner().PairDevice(remoteId, params, commissioningParams);
    }
    return err;
}

CHIP_ERROR PairingCommand::PairWithMdnsOrBleByIndex(NodeId remoteId, uint16_t index)
{
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    VerifyOrDieWithMsg(mSetupPINCode.has_value(), chipTool, "Using mSetupPINCode in a mode when we have not gotten one");

    RendezvousParameters params;
    ReturnErrorOnFailure(GetDeviceScanner().Get(index, params));
    params.SetSetupPINCode(mSetupPINCode.value());

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (mPaseOnly.ValueOr(false))
    {
        err = CurrentCommissioner().EstablishPASEConnection(remoteId, params);
    }
    else
    {
        auto commissioningParams = GetCommissioningParameters();
        err                      = CurrentCommissioner().PairDevice(remoteId, params, commissioningParams);
    }
    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
}

CHIP_ERROR PairingCommand::PairWithMdnsOrBleByIndexWithCode(NodeId remoteId, uint16_t index)
{
    // We might or might not have a setup code.  We don't know yet, but if we
    // do, we'll emplace it at that point.
    mSetupPINCode.reset();

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    Dnssd::CommonResolutionData resolutionData;
    auto err = GetDeviceScanner().Get(index, resolutionData);
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // There is no device with this index that has some resolution data. This could simply
        // be because the device is a ble device. In this case let's fall back to looking for
        // a device with this index and some RendezvousParameters.
        SetupPayload payload;
        bool isQRCode = strncmp(mOnboardingPayload, kQRCodePrefix, strlen(kQRCodePrefix)) == 0;
        if (isQRCode)
        {
            ReturnErrorOnFailure(QRCodeSetupPayloadParser(mOnboardingPayload).populatePayload(payload));
            VerifyOrReturnError(payload.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ReturnErrorOnFailure(ManualSetupPayloadParser(mOnboardingPayload).populatePayload(payload));
            VerifyOrReturnError(payload.isValidManualCode(), CHIP_ERROR_INVALID_ARGUMENT);
        }

        mSetupPINCode.emplace(payload.setUpPINCode);
        return PairWithMdnsOrBleByIndex(remoteId, index);
    }

    err = CHIP_NO_ERROR;
    if (mPaseOnly.ValueOr(false))
    {
        err = CurrentCommissioner().EstablishPASEConnection(remoteId, mOnboardingPayload, DiscoveryType::kDiscoveryNetworkOnly,
                                                            MakeOptional(resolutionData));
    }
    else
    {
        auto commissioningParams = GetCommissioningParameters();
        err                      = CurrentCommissioner().PairDevice(remoteId, mOnboardingPayload, commissioningParams,
                                                                    DiscoveryType::kDiscoveryNetworkOnly, MakeOptional(resolutionData));
    }
    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
}

class CommissionProxy : public ot::commissioner::CommissionerHandler,
                        public mdns::Minimal::ParserDelegate,
                        public mdns::Minimal::TxtRecordDelegate
{
    // DNS parser
    chip::Dnssd::DiscoveredNodeData nodeData;
    // struct sockaddr_in commissionerAddr;
    int proxyFd          = -1;
    uint16_t servicePort = 0;
    bool notified        = false;

    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override
    {
        ChipLogProgress(chipTool, "srp update=%d, msgId=%u, zoneCount=%u, prerequisiteCount=%u updateCount=%u additionalCount=%u",
                        header.GetFlags().IsUpdate(), header.GetMessageId(), header.GetZoneCount(), header.GetPrerequisiteCount(),
                        header.GetUpdateCount(), header.GetAdditionalCount());

        if (header.GetUpdateCount() == 0)
        {
            ChipLogProgress(chipTool, "srp no update");
        }

        mdns::Minimal::BitPackedFlags flags = header.GetFlags();

        flags.SetResponse();

        uint8_t buffer[mdns::Minimal::ConstHeaderRef::kSizeBytes];
        mdns::Minimal::HeaderRef response{ buffer };
        response.Clear();
        response.SetMessageId(header.GetMessageId());
        response.SetFlags(flags);

        commissioner->SendToJoiner(mJoinerId, mSrpClientPort, buffer, sizeof(buffer));
    }

    void OnQuery(const mdns::Minimal::QueryData & data) override
    {
        if (notified)
        {
            ChipLogProgress(chipTool, "already notified");
        }

        ChipLogProgress(chipTool, "srp zone: %s", mdns::Minimal::QNameString(data.GetName()).c_str());
        nodeData.Set<Dnssd::CommissionNodeData>();
    }

    void OnResource(mdns::Minimal::ResourceType section, const mdns::Minimal::ResourceData & data) override
    {
        if (notified)
        {
            ChipLogProgress(chipTool, "already notified");
            return;
        }

        auto name = mdns::Minimal::QNameString(data.GetName());

        auto & commissionData = nodeData.Get<Dnssd::CommissionNodeData>();

        switch (data.GetType())
        {
        case mdns::Minimal::QType::A:
        case mdns::Minimal::QType::AAAA:
            Platform::CopyString(commissionData.hostName, name.c_str());
            break;
        case mdns::Minimal::QType::SRV: {
            mdns::Minimal::SrvRecord srv;

            Platform::CopyString(commissionData.instanceName, name.c_str());

            if (!srv.Parse(data.GetData(), data.GetData()))
            {
                ChipLogProgress(chipTool, "failed to parse the SRV record");
                return;
            }

#define MATTERC_SERVICE_SUFFIX "_matterc._udp.default.service.arpa"
            if (!name.EndsWith(MATTERC_SERVICE_SUFFIX))
            {
                ChipLogProgress(chipTool, "unexpected service: %s", name.c_str());
                return;
            }

            servicePort = srv.GetPort();

            if (proxyFd != -1)
            {
                ChipLogProgress(chipTool, "already created");
                return;
            }

            proxyFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (proxyFd < 0)
            {
                ChipLogProgress(chipTool, "failed to create socket: %s", strerror(errno));
                return;
            }

            {
                sockaddr_in addr{
                    AF_INET,
                    0,
                    { htobe32(0x7f000001) },
                };

                if (bind(proxyFd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0)
                {
                    ChipLogProgress(chipTool, "failed to bind proxy: %s", strerror(errno));
                    close(proxyFd);
                    proxyFd = -1;
                    return;
                }

                socklen_t addr_len = sizeof(addr);
                if (getsockname(proxyFd, reinterpret_cast<struct sockaddr *>(&addr), &addr_len) == -1)
                {
                    ChipLogProgress(chipTool, "failed to getsockname: %s", strerror(errno));
                    close(proxyFd);
                    proxyFd = -1;
                    return;
                }

                commissionData.numIPs = 1;
                commissionData.port   = ntohs(addr.sin_port);

                ChipLogProgress(chipTool, "created proxy port=%u", commissionData.port);

                commissionData.ipAddress[0] = Inet::IPAddress::FromSockAddr(addr);
                commissionData.interfaceId  = Inet::InterfaceId::FromIPAddress(commissionData.ipAddress[0]);
            }
            break;
        }
        case mdns::Minimal::QType::TXT:
            mdns::Minimal::ParseTxtRecord(data.GetData(), this);
            break;
        default:
            break;
        }
    }
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value) override
    {
        ByteSpan key(name.Start(), name.Size());
        ByteSpan val(value.Start(), value.Size());

        Dnssd::FillNodeDataFromTxt(key, val, nodeData.Get<Dnssd::CommissionNodeData>());
    }

    // Commissioner Handler
    uint16_t mSrpClientPort = 0;
    uint64_t mJoinerId      = 0;
    std::promise<Dnssd::DiscoveredNodeData> mTask;

    void onJoinerMessage(uint64_t joinerId, uint16_t joinerPort, const uint8_t * buf, uint16_t len) override
    {
        ChipLogProgress(chipTool, "get message from joiner id=0x%" PRIx64 " port=%u", joinerId, joinerPort);

        if (mJoinerId == 0)
        {
            mJoinerId = joinerId;
        }

        if (mJoinerId != joinerId)
        {
            ChipLogProgress(chipTool, "Single Joiner is supported, ignoring different joiners");
            return;
        }

        if (mSrpClientPort == 0)
        {
            mSrpClientPort = joinerPort;
        }

        if (mSrpClientPort == joinerPort)
        {
            // Always parsing the packet to generate response.
            if (!mdns::Minimal::ParsePacket(mdns::Minimal::BytesRange(buf, buf + len), this))
            {
                ChipLogError(chipTool, "failed to parse parsing srp");
                return;
            }

            if (proxyFd != -1 && !notified)
            {
                mTask.set_value(nodeData);
                notified = true;
                std::thread([this, joinerId] {
                    struct sockaddr addr;
                    socklen_t addr_len = sizeof(addr);
                    uint8_t pkt[1280];
                    ssize_t rval = -1;

                    while ((rval = recvfrom(proxyFd, pkt, sizeof(pkt), 0, &addr, &addr_len)) > 0)
                    {
                        connect(proxyFd, &addr, addr_len);
                        commissioner->SendToJoiner(joinerId, servicePort, pkt, static_cast<uint16_t>(rval));
                    }
                }).detach();
            }
        }
        else if (proxyFd != -1)
        {
            auto sent = send(proxyFd, buf, len, 0);
            if (sent < 0)
            {
                ChipLogProgress(chipTool, "failed to send to commissioner: %s", strerror(errno));
            }
        }
    }

    std::shared_ptr<ot::commissioner::Commissioner> commissioner = ot::commissioner::Commissioner::Create(*this);

public:
    ~CommissionProxy()
    {
        if (proxyFd != -1)
        {
            close(proxyFd);
            proxyFd = -1;
        }
    }

    Dnssd::DiscoveredNodeData discover(uint8_t (&pskc)[Thread::kSizePSKc], const char * host, uint16_t port, uint64_t code)
    {
        struct Logger : public ot::commissioner::Logger
        {
            void Log(ot::commissioner::LogLevel level, const std::string & region, const std::string & message) override
            {
                ChipLogProgress(chipTool, "[ot-commissioner][%u][%s]%s", static_cast<unsigned>(level), region.c_str(),
                                message.c_str());
            }
        };
        ot::commissioner::Config config;

        config.mLogger    = std::make_shared<Logger>();
        config.mEnableCcm = false;
        config.mProxyMode = true;
        config.mPSKc      = std::vector<uint8_t>(&pskc[0], &pskc[Thread::kSizePSKc]);

        auto error = commissioner->Init(config);
        VerifyOrDieWithMsg(error == ot::commissioner::ErrorCode::kNone, chipTool, "failed to init commissioner: %s",
                           error.GetMessage().c_str());

        ChipLogProgress(chipTool, "connecting to border agent");
        std::string id;
        error = commissioner->Petition(id, std::string(host), port);
        VerifyOrDieWithMsg(error == ot::commissioner::ErrorCode::kNone, chipTool, "failed to connect: %s",
                           error.GetMessage().c_str());

        // TODO add single joiner
        error = commissioner->EnableAllJoiners();
        VerifyOrDieWithMsg(error == ot::commissioner::ErrorCode::kNone, chipTool, "failed enable joiners: %s",
                           error.GetMessage().c_str());

        return mTask.get_future().get();
    }
};

CHIP_ERROR PairingCommand::StartMeshcopCommissioning(uint8_t (&pskc)[Thread::kSizePSKc], const char * host, uint16_t port,
                                                     uint64_t code)
{
    static CommissionProxy proxy;

    {
        char pskc_hex[100];

        SuccessOrDie(Encoding::BytesToLowercaseHexString(pskc, sizeof(pskc), pskc_hex, sizeof(pskc_hex)));
        ChipLogProgress(chipTool, "starting meshcop commissioner host=%s port=%u pskc=%s", host, port, pskc_hex);
    }

    CurrentCommissioner().OnNodeDiscovered(proxy.discover(pskc, host, port, code));

    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingCommand::PairWithMdns(NodeId remoteId)
{
    Dnssd::DiscoveryFilter filter(mFilterType);
    switch (mFilterType)
    {
    case Dnssd::DiscoveryFilterType::kNone:
        break;
    case Dnssd::DiscoveryFilterType::kShortDiscriminator:
    case Dnssd::DiscoveryFilterType::kLongDiscriminator:
    case Dnssd::DiscoveryFilterType::kCompressedFabricId:
    case Dnssd::DiscoveryFilterType::kVendorId:
    case Dnssd::DiscoveryFilterType::kDeviceType:
        filter.code = mDiscoveryFilterCode;
        break;
    case Dnssd::DiscoveryFilterType::kCommissioningMode:
        break;
    case Dnssd::DiscoveryFilterType::kCommissioner:
        filter.code = 1;
        break;
    case Dnssd::DiscoveryFilterType::kInstanceName:
        filter.code         = 0;
        filter.instanceName = mDiscoveryFilterInstanceName;
        break;
    }

    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    if (mThreadBaHost.HasValue())
    {
        CHIP_ERROR error;
        Thread::OperationalDatasetView dataset;

        dataset.Init(mThreadDataset.Value());

        uint8_t pskc[Thread::kSizePSKc];

        error = dataset.GetPSKc(pskc);

        VerifyOrDieWithMsg(error == CHIP_NO_ERROR, chipTool, "Failed to retrieve PSKc from dataset");

        return StartMeshcopCommissioning(pskc, mThreadBaHost.Value(), mThreadBaPort.Value(), mDiscoveryFilterCode);
    }

    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR PairingCommand::Unpair(NodeId remoteId)
{
    mCurrentFabricRemover = Platform::MakeUnique<Controller::CurrentFabricRemover>(&CurrentCommissioner());
    return mCurrentFabricRemover->RemoveCurrentFabric(remoteId, &mCurrentFabricRemoveCallback);
}

void PairingCommand::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        ChipLogProgress(chipTool, "CASE establishment successful");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE);
        break;
    }
}

void PairingCommand::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Success");
        ChipLogProgress(chipTool, "PASE establishment successful");
        if (mPairingMode == PairingMode::CodePaseOnly || mPaseOnly.ValueOr(false))
        {
            SetCommandExitStatus(err);
        }
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Failure: %s", ErrorStr(err));
    }

    if (err != CHIP_NO_ERROR)
    {
        SetCommandExitStatus(err);
    }
}

void PairingCommand::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Deleted Success");
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Deleted Failure: %s", ErrorStr(err));
    }

    SetCommandExitStatus(err);
}

void PairingCommand::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Device commissioning completed with success");
    }
    else
    {
        // When ICD device commissioning fails, the ICDClientInfo stored in OnICDRegistrationComplete needs to be removed.
        if (mDeviceIsICD)
        {
            CHIP_ERROR deleteEntryError =
                CHIPCommand::sICDClientStorage.DeleteEntry(ScopedNodeId(mNodeId, CurrentCommissioner().GetFabricIndex()));
            if (deleteEntryError != CHIP_NO_ERROR)
            {
                ChipLogError(chipTool, "Failed to delete ICD entry: %s", ErrorStr(err));
            }
        }
        ChipLogProgress(chipTool, "Device commissioning Failure: %s", ErrorStr(err));
    }

    SetCommandExitStatus(err);
}

void PairingCommand::OnReadCommissioningInfo(const Controller::ReadCommissioningInfo & info)
{
    ChipLogProgress(AppServer, "OnReadCommissioningInfo - vendorId=0x%04X productId=0x%04X", info.basic.vendorId,
                    info.basic.productId);

    // The string in CharSpan received from the device is not null-terminated, we use std::string here for coping and
    // appending a numm-terminator at the end of the string.
    std::string userActiveModeTriggerInstruction;

    // Note: the callback doesn't own the buffer, should make a copy if it will be used it later.
    if (info.icd.userActiveModeTriggerInstruction.size() != 0)
    {
        userActiveModeTriggerInstruction =
            std::string(info.icd.userActiveModeTriggerInstruction.data(), info.icd.userActiveModeTriggerInstruction.size());
    }

    if (info.icd.userActiveModeTriggerHint.HasAny())
    {
        ChipLogProgress(AppServer, "OnReadCommissioningInfo - LIT UserActiveModeTriggerHint=0x%08x",
                        info.icd.userActiveModeTriggerHint.Raw());
        ChipLogProgress(AppServer, "OnReadCommissioningInfo - LIT UserActiveModeTriggerInstruction=%s",
                        userActiveModeTriggerInstruction.c_str());
    }
    ChipLogProgress(AppServer, "OnReadCommissioningInfo ICD - IdleModeDuration=%u activeModeDuration=%u activeModeThreshold=%u",
                    info.icd.idleModeDuration, info.icd.activeModeDuration, info.icd.activeModeThreshold);
}

void PairingCommand::OnICDRegistrationComplete(ScopedNodeId nodeId, uint32_t icdCounter)
{
    char icdSymmetricKeyHex[Crypto::kAES_CCM128_Key_Length * 2 + 1];

    Encoding::BytesToHex(mICDSymmetricKey.Value().data(), mICDSymmetricKey.Value().size(), icdSymmetricKeyHex,
                         sizeof(icdSymmetricKeyHex), Encoding::HexFlags::kNullTerminate);

    app::ICDClientInfo clientInfo;
    clientInfo.check_in_node     = ScopedNodeId(mICDCheckInNodeId.Value(), nodeId.GetFabricIndex());
    clientInfo.peer_node         = nodeId;
    clientInfo.monitored_subject = mICDMonitoredSubject.Value();
    clientInfo.start_icd_counter = icdCounter;

    CHIP_ERROR err = CHIPCommand::sICDClientStorage.SetKey(clientInfo, mICDSymmetricKey.Value());
    if (err == CHIP_NO_ERROR)
    {
        err = CHIPCommand::sICDClientStorage.StoreEntry(clientInfo);
    }

    if (err != CHIP_NO_ERROR)
    {
        CHIPCommand::sICDClientStorage.RemoveKey(clientInfo);
        ChipLogError(chipTool, "Failed to persist symmetric key for " ChipLogFormatX64 ": %s", ChipLogValueX64(nodeId.GetNodeId()),
                     err.AsString());
        SetCommandExitStatus(err);
        return;
    }

    mDeviceIsICD = true;

    ChipLogProgress(chipTool, "Saved ICD Symmetric key for " ChipLogFormatX64, ChipLogValueX64(nodeId.GetNodeId()));
    ChipLogProgress(chipTool,
                    "ICD Registration Complete for device " ChipLogFormatX64 " / Check-In NodeID: " ChipLogFormatX64
                    " / Monitored Subject: " ChipLogFormatX64 " / Symmetric Key: %s / ICDCounter %u",
                    ChipLogValueX64(nodeId.GetNodeId()), ChipLogValueX64(mICDCheckInNodeId.Value()),
                    ChipLogValueX64(mICDMonitoredSubject.Value()), icdSymmetricKeyHex, icdCounter);
}

void PairingCommand::OnICDStayActiveComplete(ScopedNodeId deviceId, uint32_t promisedActiveDuration)
{
    ChipLogProgress(chipTool, "ICD Stay Active Complete for device " ChipLogFormatX64 " / promisedActiveDuration: %u",
                    ChipLogValueX64(deviceId.GetNodeId()), promisedActiveDuration);
}

void PairingCommand::OnCommissioningStageStart(PeerId peerId, CommissioningStage stageStarting)
{
    ChipLogDetail(chipTool, "Starting commissioning stage '%s'", StageToString(stageStarting));
}

CHIP_ERROR PairingCommand::WiFiCredentialsNeeded(EndpointId endpoint)
{
    if (mNetworkType != PairingNetworkType::None)
    {
        // We only support prompting for credentials when no credentials were
        // provided up front, for now.
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    // We block while prompting for the information, and that does not seem to
    // work well if we do it synchronously: we seem to lose the BLE connection
    // to the commissionee.  So do all the rest of the work async.  The
    // outermost ScheduleLambda is only there to avoid the prompt interleaving
    // with logging that happens on the Matter thread after this function
    // returns.
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        mPrompterThread.emplace([this] {
            do
            {
                std::cout << "Enter the Wi-Fi SSID: ";
                std::getline(std::cin, mPromptedSSID);
                if (OctetStringFromCharString(mPromptedSSID.data(), &mSSID))
                {
                    break;
                }
                ChipLogError(chipTool, "Invalid value for SSID");
            } while (true);

            do
            {
                std::cout << "Enter the Wi-Fi password (empty for an open network): ";
                std::getline(std::cin, mPromptedPassword);
                if (OctetStringFromCharString(mPromptedPassword.data(), &mPassword))
                {
                    break;
                }
                ChipLogError(chipTool, "Invalid value for password");
            } while (true);

            DeviceLayer::SystemLayer().ScheduleLambda([this] {
                // Ensure that the background thread (and its writes to our members) is done.
                mPrompterThread->join();
                mPrompterThread.reset();

                auto & commissioner            = CurrentCommissioner();
                CommissioningParameters params = commissioner.GetCommissioningParameters();
                auto credentials               = Controller::WiFiCredentials(mSSID, mPassword);
                params.SetWiFiCredentials(credentials);
                commissioner.UpdateCommissioningParameters(params);

                commissioner.NetworkCredentialsReady();
            });
        });
    });

    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingCommand::ThreadCredentialsNeeded(EndpointId endpoint)
{
    if (mNetworkType != PairingNetworkType::None)
    {
        // We only support prompting for credentials when no credentials were
        // provided up front, for now.
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    // We block while prompting for the information, and that does not seem to
    // work well if we do it synchronously: we seem to lose the BLE connection
    // to the commissionee.  So do all the rest of the work async.  The
    // outermost ScheduleLambda is only there to avoid the prompt interleaving
    // with logging that happens on the Matter thread after this function
    // returns.
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        mPrompterThread.emplace([this] {
            do
            {
                std::cout << "Enter the operational dataset (probably as a hex string prefixed with \"hex:\"): ";
                std::getline(std::cin, mPromptedOperationalDataset);
                if (OctetStringFromCharString(mPromptedOperationalDataset.data(), &mOperationalDataset))
                {
                    break;
                }
                ChipLogError(chipTool, "Invalid value for operational dataset");
            } while (true);

            DeviceLayer::SystemLayer().ScheduleLambda([this] {
                // Ensure that the background thread (and its writes to our members) is done.
                mPrompterThread->join();
                mPrompterThread.reset();

                auto & commissioner            = CurrentCommissioner();
                CommissioningParameters params = commissioner.GetCommissioningParameters();
                params.SetThreadOperationalDataset(mOperationalDataset);
                commissioner.UpdateCommissioningParameters(params);

                commissioner.NetworkCredentialsReady();
            });
        });
    });

    return CHIP_NO_ERROR;
}

void PairingCommand::OnDiscoveredDevice(const Dnssd::CommissionNodeData & nodeData)
{
    // Ignore nodes with closed commissioning window
    VerifyOrReturn(nodeData.commissioningMode != 0);

    auto & resolutionData = nodeData;

    const uint16_t port = resolutionData.port;
    char buf[Inet::IPAddress::kMaxStringLength];
    resolutionData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery.
    auto err = CurrentCommissioner().StopCommissionableDiscovery();

    // Some platforms does not implement a mechanism to stop mdns browse, so
    // we just ignore CHIP_ERROR_NOT_IMPLEMENTED instead of bailing out.
    if (CHIP_NO_ERROR != err && CHIP_ERROR_NOT_IMPLEMENTED != err)
    {
        SetCommandExitStatus(err);
        return;
    }

    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(nullptr);

    auto interfaceId = resolutionData.ipAddress[0].IsIPv6LinkLocal() ? resolutionData.interfaceId : Inet::InterfaceId::Null();
    auto peerAddress = PeerAddress::UDP(resolutionData.ipAddress[0], port, interfaceId);
    err              = Pair(mNodeId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}

void PairingCommand::OnCurrentFabricRemove(void * context, NodeId nodeId, CHIP_ERROR err)
{
    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnCurrentFabricRemove: context is null"));

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Device unpair completed with success: " ChipLogFormatX64, ChipLogValueX64(nodeId));
    }
    else
    {
        ChipLogProgress(chipTool, "Device unpair Failure: " ChipLogFormatX64 " %s", ChipLogValueX64(nodeId), ErrorStr(err));
    }

    command->SetCommandExitStatus(err);
}

Optional<uint16_t> PairingCommand::FailSafeExpiryTimeoutSecs() const
{
    // We don't need to set additional failsafe timeout as we don't ask the final user if he wants to continue
    return Optional<uint16_t>();
}

void PairingCommand::OnDeviceAttestationCompleted(Controller::DeviceCommissioner * deviceCommissioner, DeviceProxy * device,
                                                  const Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                                  Credentials::AttestationVerificationResult attestationResult)
{
    // Bypass attestation verification, continue with success
    auto err = deviceCommissioner->ContinueCommissioningAfterDeviceAttestation(
        device, Credentials::AttestationVerificationResult::kSuccess);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}

CHIP_ERROR PairingCommand::MaybeDisplayTermsAndConditions(CommissioningParameters & params)
{
    VerifyOrReturnError(mUseDCL.ValueOr(false), CHIP_NO_ERROR);

    Json::Value tc;
    auto client = tool::dcl::DCLClient(mDCLHostName, mDCLPort);
    ReturnErrorOnFailure(client.TermsAndConditions(mOnboardingPayload, tc));
    if (tc != Json::nullValue)
    {
        uint16_t version      = 0;
        uint16_t userResponse = 0;
        ReturnErrorOnFailure(tool::dcl::DisplayTermsAndConditions(tc, version, userResponse, mCountryCode));

        TermsAndConditionsAcknowledgement termsAndConditionsAcknowledgement = {
            .acceptedTermsAndConditions        = userResponse,
            .acceptedTermsAndConditionsVersion = version,
        };
        params.SetTermsAndConditionsAcknowledgement(termsAndConditionsAcknowledgement);
    }

    return CHIP_NO_ERROR;
}
