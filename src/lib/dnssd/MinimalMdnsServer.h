/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
