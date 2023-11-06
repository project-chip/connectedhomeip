/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <inet/IPPacketInfo.h>
#include <lib/dnssd/minimal_mdns/Server.h>

namespace chip {
namespace Dnssd {

/// Generic receive delegate for a MDNS packet
class MdnsPacketDelegate
{
public:
    virtual ~MdnsPacketDelegate() {}
    virtual void OnMdnsPacketData(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) = 0;
};

/// A global mdns::Minimal::Server wrapper
/// used to share the same server between MDNS Advertiser and resolver
/// as advertiser responds to 'onquery' and resolver expects 'onresponse'
class GlobalMinimalMdnsServer : public mdns::Minimal::ServerDelegate
{
public:
    static constexpr size_t kMaxEndPoints = 30;

    using ServerType = mdns::Minimal::Server<kMaxEndPoints>;

    GlobalMinimalMdnsServer();

    static GlobalMinimalMdnsServer & Instance();
    static mdns::Minimal::ServerBase & Server()
    {
        if (Instance().mReplacementServer != nullptr)
        {
            return *Instance().mReplacementServer;
        }

        return Instance().mServer;
    }

    /// Calls Server().Listen() on all available interfaces
    CHIP_ERROR StartServer(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager, uint16_t port);
    void ShutdownServer();

    void SetQueryDelegate(MdnsPacketDelegate * delegate) { mQueryDelegate = delegate; }
    void SetResponseDelegate(MdnsPacketDelegate * delegate) { mResponseDelegate = delegate; }

    // ServerDelegate implementation
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        if (mQueryDelegate != nullptr)
        {
            mQueryDelegate->OnMdnsPacketData(data, info);
        }
    }

    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        if (mResponseDelegate != nullptr)
        {
            mResponseDelegate->OnMdnsPacketData(data, info);
        }
    }

    void SetReplacementServer(mdns::Minimal::ServerBase * server) { mReplacementServer = server; }

private:
    ServerType mServer;
    mdns::Minimal::ServerBase * mReplacementServer = nullptr;
    MdnsPacketDelegate * mQueryDelegate            = nullptr;
    MdnsPacketDelegate * mResponseDelegate         = nullptr;
};

} // namespace Dnssd
} // namespace chip
