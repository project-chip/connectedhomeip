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

#pragma once

#include <commissioner/commissioner.hpp>
#include <lib/dnssd/Types.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>

#include <future>
#include <memory>

namespace chip {
namespace Controller {

class ThreadCommissionerProxy : public ot::commissioner::CommissionerHandler,
                                public mdns::Minimal::ParserDelegate,
                                public mdns::Minimal::TxtRecordDelegate
{
public:
    ~ThreadCommissionerProxy();

    Dnssd::DiscoveredNodeData discover(const ByteSpan & pskc, const char * host, uint16_t port, uint64_t code);

private:
    // DNS parser
    chip::Dnssd::DiscoveredNodeData nodeData;
    // struct sockaddr_in commissionerAddr;
    int proxyFd          = -1;
    uint16_t servicePort = 0;
    bool notified        = false;

    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override;

    void OnQuery(const mdns::Minimal::QueryData & data) override;

    void OnResource(mdns::Minimal::ResourceType section, const mdns::Minimal::ResourceData & data) override;

    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value) override;

    // Commissioner Handler
    uint16_t mSrpClientPort                                      = 0;
    uint64_t mJoinerId                                           = 0;
    std::shared_ptr<ot::commissioner::Commissioner> commissioner = ot::commissioner::Commissioner::Create(*this);
    std::promise<Dnssd::DiscoveredNodeData> mTask;

    void onJoinerMessage(uint64_t joinerId, uint16_t joinerPort, const uint8_t * buf, uint16_t len) override;
};

} // namespace Controller
} // namespace chip
