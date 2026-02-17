/*
 *   Copyright (c) 2026 Project CHIP Authors
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

#include "ThreadMeshcopCommissionProxy.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/dnssd/minimal_mdns/core/QNameString.h> // nogncheck
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <thread>

using namespace chip;

namespace {
/**
 * Internal OT Commissioner Logger implementation.
 */
class CommissionerLogger : public ot::commissioner::Logger
{
public:
    void Log(ot::commissioner::LogLevel level, const std::string & region, const std::string & message) override
    {
        ChipLogProgress(Controller, "[ot-commissioner][%u][%s] %s", static_cast<unsigned>(level), region.c_str(), message.c_str());
    }
};

constexpr char kMatterCServiceSuffix[] = "_matterc._udp.local";

uint64_t JoinerIdFromBytes(const std::vector<uint8_t> & bytes)
{
    const uint8_t * buffer = bytes.data();
    return Encoding::BigEndian::Read64(buffer);
}

std::vector<uint8_t> DiscoveryCodeToVector(Thread::DiscoveryCode code)
{
    uint8_t bytes[sizeof(uint64_t)];
    Encoding::BigEndian::Put64(bytes, code.AsUInt64());
    return std::vector<uint8_t>(bytes, bytes + sizeof(bytes));
}
} // namespace

namespace chip {
namespace Controller {

ThreadMeshcopCommissionProxy::ThreadMeshcopCommissionProxy() : mState(State::kConnecting), mPromiseFulfilled(false)
{
    mCommissioner = ot::commissioner::Commissioner::Create(*this);
}

ThreadMeshcopCommissionProxy::~ThreadMeshcopCommissionProxy()
{
    if (mProxyFd != -1)
    {
        close(mProxyFd);
        mProxyFd = -1;
    }

    if (mProxyThread.joinable())
    {
        mProxyThread.join();
    }
}

void ThreadMeshcopCommissionProxy::SetState(State state)
{
    mState = state;
}

void ThreadMeshcopCommissionProxy::OnHeader(mdns::Minimal::ConstHeaderRef & header)
{
    ChipLogDetail(Controller, "mDNS Response: ID=%u, Answers=%u, Additional=%u", header.GetMessageId(), header.GetAnswerCount(),
                  header.GetAdditionalCount());
}

void ThreadMeshcopCommissionProxy::OnQuery(const mdns::Minimal::QueryData & data)
{
    if (mState != State::kDiscovering)
    {
        ChipLogProgress(Controller, "Received mDNS query but proxy is not in discovery state");
    }

    ChipLogDetail(Controller, "mDNS query: %s", mdns::Minimal::QNameString(data.GetName()).c_str());
    mNodeData.Set<Dnssd::CommissionNodeData>();
}

void ThreadMeshcopCommissionProxy::OnResource(mdns::Minimal::ResourceType section, const mdns::Minimal::ResourceData & data)
{
    if (mState != State::kDiscovering)
    {
        return;
    }

    auto name             = mdns::Minimal::QNameString(data.GetName());
    auto & commissionData = mNodeData.Get<Dnssd::CommissionNodeData>();

    commissionData.threadMeshcop = true;

    switch (data.GetType())
    {
    case mdns::Minimal::QType::A:
    case mdns::Minimal::QType::AAAA:
        Platform::CopyString(commissionData.hostName, name.c_str());
        break;

    case mdns::Minimal::QType::SRV: {
        mdns::Minimal::SrvRecord srv;
        if (!srv.Parse(data.GetData(), mDnsPacket))
        {
            ChipLogError(Controller, "Failed to parse mDNS SRV record");
            return;
        }

        if (!name.EndsWith(kMatterCServiceSuffix))
        {
            ChipLogDetail(Controller, "Ignoring non-Matter service: %s", name.c_str());
            return;
        }

        // Extract the instance label (portion before "._matterc._udp.local") for CommissionNodeData::instanceName.
        std::string fullName(name.c_str());
        constexpr size_t kMatterCServiceSuffixLen = sizeof(kMatterCServiceSuffix) - 1; // exclude null terminator
        if (fullName.length() >= kMatterCServiceSuffixLen)
        {
            fullName.erase(fullName.length() - kMatterCServiceSuffixLen);
        }
        Platform::CopyString(commissionData.instanceName, fullName.c_str());

        mServicePort = srv.GetPort();

        if (mProxyFd == -1)
        {
            CHIP_ERROR err = CreateProxySocket(commissionData);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "Failed to setup proxy socket: %" CHIP_ERROR_FORMAT, err.Format());
                SetState(State::kAborted);
            }
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

CHIP_ERROR ThreadMeshcopCommissionProxy::CreateProxySocket(chip::Dnssd::CommissionNodeData & commissionData)
{
    mProxyFd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    VerifyOrReturnError(mProxyFd >= 0, CHIP_ERROR_POSIX(errno));

    sockaddr_in6 addr = {};
    addr.sin6_family  = AF_INET6;
    addr.sin6_port    = 0;
    addr.sin6_addr    = in6addr_loopback;

    if (bind(mProxyFd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        close(mProxyFd);
        mProxyFd = -1;
        return CHIP_ERROR_POSIX(errno);
    }

    socklen_t addr_len = sizeof(addr);
    if (getsockname(mProxyFd, reinterpret_cast<struct sockaddr *>(&addr), &addr_len) == -1)
    {
        close(mProxyFd);
        mProxyFd = -1;
        return CHIP_ERROR_POSIX(errno);
    }

    commissionData.numIPs       = 1;
    commissionData.port         = ntohs(addr.sin6_port);
    commissionData.ipAddress[0] = Inet::IPAddress::FromSockAddr(addr);
    commissionData.interfaceId  = Inet::InterfaceId::FromIPAddress(commissionData.ipAddress[0]);

    ChipLogProgress(Controller, "Proxy socket created on port %u", commissionData.port);
    return CHIP_NO_ERROR;
}

void ThreadMeshcopCommissionProxy::OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value)
{
    ByteSpan key(name.Start(), name.Size());
    ByteSpan val(value.Start(), value.Size());

    Dnssd::FillNodeDataFromTxt(key, val, mNodeData.Get<Dnssd::CommissionNodeData>());
}

void ThreadMeshcopCommissionProxy::ProcessAnnouncement(const std::vector<uint8_t> & joinerIdBytes, uint16_t joinerPort,
                                                       const std::vector<uint8_t> & payload)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);

    if (mPromiseFulfilled)
    {
        return;
    }

    mNodeData.Set<Dnssd::CommissionNodeData>();
    mDnsPacket = mdns::Minimal::BytesRange(payload.data(), payload.data() + payload.size());

    if (!mdns::Minimal::ParsePacket(mDnsPacket, this))
    {
        ChipLogError(Controller, "Failed to parse joiner mDNS announcement");
        return;
    }

    uint32_t discoveredDiscriminator = mNodeData.Get<Dnssd::CommissionNodeData>().longDiscriminator;
    ChipLogProgress(Controller, "Discovered joiner with discriminator: %u", discoveredDiscriminator);

    if (!mExpectedDiscriminator.MatchesLongDiscriminator(static_cast<uint16_t>(discoveredDiscriminator)))
    {
        ChipLogProgress(Controller, "Discriminator mismatch (Expected %u, Got %u). Ignoring announcement.",
                        mExpectedDiscriminator.GetLongValue(), discoveredDiscriminator);
        return;
    }

    mDiscoveredNodePromise.set_value(mNodeData);
    mPromiseFulfilled = true;

    SetState(State::kDiscovered);

    if (mProxyThread.joinable())
    {
        mProxyThread.join();
    }

    mProxyThread = std::thread([id = joinerIdBytes, this]() {
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        uint8_t buf[chip::detail::kMaxIPPacketSizeBytes];
        ssize_t received;

        while ((received = recvfrom(mProxyFd, buf, sizeof(buf), 0, reinterpret_cast<struct sockaddr *>(&addr), &len)) > 0)
        {
            switch (mState)
            {
            case State::kDiscovered: {
                int rval = connect(mProxyFd, reinterpret_cast<struct sockaddr *>(&addr), len);
                if (rval < 0)
                {
                    ChipLogError(Controller, "Failed to connect to Matter Commissioner: %s", strerror(errno));
                    continue;
                }
                SetState(State::kCommissioning);
                FALLTHROUGH;
            }

            case State::kCommissioning: {
                std::vector<uint8_t> pkt(buf, buf + received);

                auto error = mCommissioner->SendToJoiner(id, mServicePort, pkt);
                if (error != ot::commissioner::ErrorCode::kNone)
                {
                    ChipLogError(Controller, "Failed to send packet to joiner: %s", error.GetMessage().c_str());
                    return;
                }
                break;
            }
            default:
                ChipLogError(Controller, "Invalid CommissionProxy state: %d", static_cast<int>(mState.load()));
                return;
            }
        }
    });
}

void ThreadMeshcopCommissionProxy::OnJoinerMessage(const std::vector<uint8_t> & joinerIdBytes, uint16_t joinerPort,
                                                   const std::vector<uint8_t> & payload)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);

    if (joinerIdBytes.size() != sizeof(uint64_t) || mState == State::kAborted)
    {
        return;
    }

    uint64_t joinerId = JoinerIdFromBytes(joinerIdBytes);
    ChipLogDetail(Controller, "Message from joiner 0x%" PRIx64 " on port %u", joinerId, joinerPort);

    if (mJoinerId == 0)
    {
        mJoinerId = joinerId;
    }
    else if (mJoinerId != joinerId)
    {
        ChipLogProgress(Controller, "Ignoring message from unexpected joiner 0x%" PRIx64, joinerId);
        return;
    }

    switch (mState)
    {
    case State::kCommissioning:
        if (mProxyFd != -1)
        {
            if (send(mProxyFd, payload.data(), payload.size(), 0) < 0)
            {
                ChipLogError(Controller, "Failed to forward packet to local proxy: %s", strerror(errno));
                SetState(State::kAborted);
            }
        }
        break;
    case State::kAborted:
        break;
    case State::kConnecting:
        // First message from joiner is usually the mDNS announcement
        SetState(State::kDiscovering);
        FALLTHROUGH;
    case State::kDiscovering:
        ProcessAnnouncement(joinerIdBytes, joinerPort, payload);
        break;
    case State::kDiscovered:
        ChipLogProgress(Controller, "WARNING ignore unsolicited messages after joiner is already discovered");
        break;
    }
}

ot::commissioner::CommissionerDataset ThreadMeshcopCommissionProxy::MakeCommissionerDataset(Thread::DiscoveryCode code)
{
    ot::commissioner::CommissionerDataset dataset;

    dataset.mJoinerUdpPort = ot::commissioner::kDefaultJoinerUdpPort;
    dataset.mPresentFlags |= ot::commissioner::CommissionerDataset::kJoinerUdpPortBit;
    dataset.mPresentFlags &=
        ~(ot::commissioner::CommissionerDataset::kSessionIdBit | ot::commissioner::CommissionerDataset::kBorderAgentLocatorBit);

    if (code.IsAny())
    {
        dataset.mSteeringData = std::vector<uint8_t>{ 0xff };
    }
    else
    {
        std::vector<uint8_t> steeringData(ot::commissioner::kMaxSteeringDataLength);
        ot::commissioner::Commissioner::AddJoiner(steeringData, DiscoveryCodeToVector(code));
        dataset.mSteeringData = steeringData;
    }

    dataset.mPresentFlags |= ot::commissioner::CommissionerDataset::kSteeringDataBit;
    return dataset;
}
CHIP_ERROR ThreadMeshcopCommissionProxy::InitializeCommissioner(ByteSpan & pskc)
{
    VerifyOrReturnError(pskc.size() == Thread::kSizePSKc, CHIP_ERROR_INVALID_ARGUMENT);
    ot::commissioner::Config config;
    config.mLogger    = std::make_shared<CommissionerLogger>();
    config.mEnableCcm = false;
    config.mProxyMode = true;
    config.mPSKc      = std::vector<uint8_t>(pskc.begin(), pskc.end());

    auto error = mCommissioner->Init(config);
    if (error != ot::commissioner::ErrorCode::kNone)
    {
        ChipLogError(Controller, "OT Commissioner Init failed: %s", error.GetMessage().c_str());
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadMeshcopCommissionProxy::Discover(ByteSpan & pskc, const Transport::PeerAddress & peerAddr,
                                                  const Thread::DiscoveryCode code, SetupDiscriminator expectedDiscriminator,
                                                  Dnssd::DiscoveredNodeData & nodeData, uint16_t timeout)
{
    using ot::commissioner::Error;

    Error error;

    // Reset the promise and state for a new discovery session
    std::future<Dnssd::DiscoveredNodeData> future;
    {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        mExpectedDiscriminator = expectedDiscriminator;
        SetState(State::kConnecting);
        mDiscoveredNodePromise = std::promise<Dnssd::DiscoveredNodeData>();
        future                 = mDiscoveredNodePromise.get_future();
        mPromiseFulfilled      = false;
        mJoinerId              = 0;
    }

    ReturnErrorOnFailure(InitializeCommissioner(pskc));

    {
        std::string id;
        char host[Inet::IPAddress::kMaxStringLength];
        peerAddr.GetIPAddress().ToString(host);

        ChipLogProgress(Controller, "Petitioning Thread Border Agent at %s:%u", host, peerAddr.GetPort());
        error = mCommissioner->Petition(id, std::string(host), peerAddr.GetPort());
        if (error != ot::commissioner::ErrorCode::kNone)
        {
            ChipLogError(Controller, "Petition failed: %s", error.GetMessage().c_str());
            SetState(State::kAborted);
            return CHIP_ERROR_INTERNAL;
        }

        ChipLogProgress(Controller, "Thread Commissioner active with ID: %s", id.c_str());
    }

    error = mCommissioner->SetCommissionerDataset(MakeCommissionerDataset(code));
    if (error != ot::commissioner::ErrorCode::kNone)
    {
        ChipLogError(Controller, "Failed to set Steering Data: %s", error.GetMessage().c_str());
        SetState(State::kAborted);
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(Controller, "Waiting for mDNS announcement from joiner...");
    auto waitDuration = std::chrono::seconds(timeout);
    if (future.wait_for(waitDuration) == std::future_status::timeout)
    {
        ChipLogError(Controller, "Timed out waiting for joiner mDNS announcement after %u seconds", timeout);
        SetState(State::kAborted);
        return CHIP_ERROR_TIMEOUT;
    }
    nodeData = future.get();
    return CHIP_NO_ERROR;
}
} // namespace Controller
} // namespace chip
