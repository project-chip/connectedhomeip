#pragma once
#include <app/server/Dnssd.h>
#include <lib/dnssd/IDnssdServer.h>

namespace chip {
namespace app {
class PlatformDnssdServer : public chip::Dnssd::IDnssdServer
{
    CHIP_ERROR AdvertiseOperational() override { return DnssdServer::Instance().AdvertiseOperational(); }
    virtual void Start() override { DnssdServer::Instance().StartServer(); }
    virtual void Stop() override { DnssdServer::Instance().StopServer(); }
    bool IsAdvertisingEnabled() override
    {
        return false; // placeholder
    }
};
} // namespace app
} // namespace chip
