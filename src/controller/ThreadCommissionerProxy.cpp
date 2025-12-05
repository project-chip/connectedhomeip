/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <controller/ThreadCommissionerProxy.h>
#include <lib/dnssd/minimal_mdns/core/QNameString.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/ThreadOperationalDataset.h>

#include <unistd.h>
#include <sys/socket.h>

namespace chip {
namespace Controller {

ThreadCommissionerProxy::~ThreadCommissionerProxy()
{
    if (proxyFd != -1)
    {
        close(proxyFd);
        proxyFd = -1;
    }
}

void ThreadCommissionerProxy::OnHeader(mdns::Minimal::ConstHeaderRef & header)
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

void ThreadCommissionerProxy::OnQuery(const mdns::Minimal::QueryData & data)
{
    if (notified)
    {
        ChipLogProgress(chipTool, "already notified");
    }

    ChipLogProgress(chipTool, "srp zone: %s", mdns::Minimal::QNameString(data.GetName()).c_str());
    nodeData.Set<Dnssd::CommissionNodeData>();
}

void ThreadCommissionerProxy::OnResource(mdns::Minimal::ResourceType section, const mdns::Minimal::ResourceData & data)
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
void ThreadCommissionerProxy::OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value)
{
    ByteSpan key(name.Start(), name.Size());
    ByteSpan val(value.Start(), value.Size());

    Dnssd::FillNodeDataFromTxt(key, val, nodeData.Get<Dnssd::CommissionNodeData>());
}

void ThreadCommissionerProxy::onJoinerMessage(uint64_t joinerId, uint16_t joinerPort, const uint8_t * buf, uint16_t len)
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

Dnssd::DiscoveredNodeData ThreadCommissionerProxy::discover(const ByteSpan &pskc, const char * host, uint16_t port,
                                                            uint64_t code)
{
    struct Logger : public ot::commissioner::Logger
    {
        void Log(ot::commissioner::LogLevel level, const std::string & region, const std::string & message) override
        {
            ChipLogProgress(chipTool, "[ot-commissioner][%u][%s]%s", static_cast<unsigned>(level), region.c_str(), message.c_str());
        }
    };
    ot::commissioner::Config config;

    config.mLogger    = std::make_shared<Logger>();
    config.mEnableCcm = false;
    config.mProxyMode = true;
    config.mPSKc      = std::vector<uint8_t>(&pskc[0], &pskc[Thread::kSizePSKc - 1] + 1);

    auto error = commissioner->Init(config);
    VerifyOrDieWithMsg(error == ot::commissioner::ErrorCode::kNone, chipTool, "failed to init commissioner: %s",
                       error.GetMessage().c_str());

    ChipLogProgress(chipTool, "connecting to border agent");
    std::string id;
    error = commissioner->Petition(id, std::string(host), port);
    VerifyOrDieWithMsg(error == ot::commissioner::ErrorCode::kNone, chipTool, "failed to connect: %s", error.GetMessage().c_str());

    // TODO add single joiner
    error = commissioner->EnableAllJoiners();
    VerifyOrDieWithMsg(error == ot::commissioner::ErrorCode::kNone, chipTool, "failed enable joiners: %s",
                       error.GetMessage().c_str());

    return mTask.get_future().get();
}

} // namespace Controller
} // namespace chip
