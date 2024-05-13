/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <lib/dnssd/minimal_mdns/Logging.h>
#include <lib/dnssd/minimal_mdns/core/QNameString.h>
#include <lib/support/logging/CHIPLogging.h>

namespace mdns {
namespace Minimal {
namespace Logging {

namespace {

#if CHIP_PROGRESS_LOGGING

const char * QueryTypeToString(mdns::Minimal::QType type)
{
    // NOTE: not all values are handled, only things that matter
    // and minmdns really cares about
    switch (type)
    {
    case QType::A:
        return "A";
    case QType::PTR:
        return "PTR";
    case QType::TXT:
        return "TXT";
    case QType::AAAA:
        return "AAAA";
    case QType::SRV:
        return "SRV";
    case QType::ANY:
        return "ANY";
    default:
        // Not reentrant, however our logging is in the chip thread so seems ok.
        static char buff[16];
        snprintf(buff, sizeof(buff), "(%d)", static_cast<int>(type));
        return buff;
    }
}

#endif // CHIP_PROGRESS_LOGGING
} // namespace

void LogSendingQuery(const mdns::Minimal::Query & query)
{
    QNameString name(query.GetName());

    ChipLogProgress(Discovery, "MINMDNS: Sending query %s/%s for %s%s", QueryTypeToString(query.GetType()),
                    query.IsAnswerViaUnicast() ? "UNICAST" : "MULTICAST", name.c_str(), name.Fit() ? "" : "...");
}

void LogReceivedResource(const mdns::Minimal::ResourceData & data)
{
    QNameString name(data.GetName());

    ChipLogProgress(Discovery, "MINMDNS: received %s record for %s%s", QueryTypeToString(data.GetType()), name.c_str(),
                    name.Fit() ? "" : "...");
}

void LogFoundOperationalSrvRecord(const chip::PeerId & peerId, const mdns::Minimal::SerializedQNameIterator & targetHost)
{
    QNameString host(targetHost);

    ChipLogProgress(Discovery, "MINMDNS:     Operational SRV for " ChipLogFormatX64 "-" ChipLogFormatX64 ": %s",
                    ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()), host.c_str());
}

void LogFoundCommissionSrvRecord(const char * instance, const mdns::Minimal::SerializedQNameIterator & targetHost)
{
    QNameString host(targetHost);

    ChipLogProgress(Discovery, "MINMDNS:     Commission SRV for instance %s: %s", instance, host.c_str());
}

void LogFoundIPAddress(const mdns::Minimal::SerializedQNameIterator & targetHost, const chip::Inet::IPAddress & addr)
{
    QNameString host(targetHost);
    char ipBuff[chip::Inet::IPAddress::kMaxStringLength];

    addr.ToString(ipBuff);

    ChipLogProgress(Discovery, "MINMDNS:     IP address %s found for %s%s", ipBuff, host.c_str(), host.Fit() ? "" : "...");
}

} // namespace Logging
} // namespace Minimal
} // namespace mdns
