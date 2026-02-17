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

#pragma once

#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/Types.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/support/SetupDiscriminator.h>
#include <lib/support/ThreadDiscoveryCode.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <transport/raw/PeerAddress.h>

#include <commissioner/commissioner.hpp>

namespace chip {
namespace Controller {
/**
 * CommissionProxy acts as a bridge between the OpenThread Commissioner and Matter commissioning.
 * It handles Thread-specific commissioning (MeshCoP) and proxies mDNS discovery data
 * to facilitate the transition into Matter's operational commissioning flow.
 */
class ThreadMeshcopCommissionProxy : public ot::commissioner::CommissionerHandler,
                                     public mdns::Minimal::ParserDelegate,
                                     public mdns::Minimal::TxtRecordDelegate
{
public:
    enum class State
    {
        kConnecting,    // Establishing connection to Thread Border Agent
        kDiscovering,   // Waiting for mDNS announcements from the joiner
        kDiscovered,    // Discovered a matching joiner
        kCommissioning, // Proxying packets between local socket and joiner
        kAborted,       // Error or user cancellation
    };

    ThreadMeshcopCommissionProxy();
    ~ThreadMeshcopCommissionProxy() override;

    /**
     * Entry point to start the Thread commissioning and discover the device.
     */
    CHIP_ERROR Discover(ByteSpan & pskc, const Transport::PeerAddress & peerAddr, const Thread::DiscoveryCode code,
                        chip::SetupDiscriminator expectedDiscriminator, Dnssd::DiscoveredNodeData & nodeData, uint16_t timeout);

    // ot::commissioner::CommissionerHandler
    void OnJoinerMessage(const std::vector<uint8_t> & joinerIdBytes, uint16_t joinerPort,
                         const std::vector<uint8_t> & payload) override;

    // mdns::Minimal::ParserDelegate
    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override;
    void OnQuery(const mdns::Minimal::QueryData & data) override;
    void OnResource(mdns::Minimal::ResourceType section, const mdns::Minimal::ResourceData & data) override;

    // mdns::Minimal::TxtRecordDelegate
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value) override;

private:
    // Internal Helper Methods
    CHIP_ERROR InitializeCommissioner(ByteSpan & pskc);
    CHIP_ERROR CreateProxySocket(Dnssd::CommissionNodeData & commissionData);
    void ProcessAnnouncement(const std::vector<uint8_t> & joinerIdBytes, uint16_t joinerPort, const std::vector<uint8_t> & payload);
    void SetState(State state);

    ot::commissioner::CommissionerDataset MakeCommissionerDataset(Thread::DiscoveryCode code);

    // Member Variables
    chip::Dnssd::DiscoveredNodeData mNodeData;
    mdns::Minimal::BytesRange mDnsPacket;

    int mProxyFd          = -1;
    uint16_t mServicePort = 0;
    std::atomic<State> mState;
    chip::SetupDiscriminator mExpectedDiscriminator;

    uint64_t mJoinerId = 0;

    std::recursive_mutex mMutex;
    bool mPromiseFulfilled = false;
    std::promise<Dnssd::DiscoveredNodeData> mDiscoveredNodePromise;

    std::shared_ptr<ot::commissioner::Commissioner> mCommissioner;
    std::thread mProxyThread;
};

} // namespace Controller
} // namespace chip
